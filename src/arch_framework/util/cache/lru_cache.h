#pragma once
#include <unordered_map>
#include <list>
#include <vector>
#include <memory>
#include <functional>
#include <boost/thread/shared_mutex.hpp>
#include "cpputil/common/timer/time_utils.h"

namespace cpputil {

template <typename K, typename V>
struct KeyValue {
    K key;
    V value;

    KeyValue(const K& k, const V& v) : key(k), value(v) {
    }
};

template <typename K, typename V, typename Hash = std::hash<K>>
class LruCache {
public:
    typedef std::list<KeyValue<K, V>> ListType;
    typedef std::unordered_map<K, typename ListType::iterator, Hash> MapType;

    /**
     * 显式构造函数，指定最大容量
     */
    explicit LruCache(size_t max_size) : max_size_(max_size) {
    }

    LruCache(const LruCache&) = delete;
    const LruCache& operator=(const LruCache&) = delete;
    virtual ~LruCache() {
    }

    size_t size() {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        return cache_.size();
    }

    size_t max_size() const {
        return max_size_;
    }

    bool empty() {
        return size() == 0;
    }

    /**
     * 增加一条记录，如果 cache 容量满了，会淘汰掉最旧的
     */
    void add(const K& k, const V& v) {
        std::vector<KeyValue<K, V>> drops;
        {
            boost::unique_lock<boost::shared_mutex> lock(mutex_);
            _add_no_lock(k, v, drops);
        }
    }

    void multi_add(const std::unordered_map<K, V>& key_values) {
        std::vector<KeyValue<K, V>> drops;
        {
            boost::unique_lock<boost::shared_mutex> lock(mutex_);
            for (auto& kv : key_values) {
                _add_no_lock(kv.first, kv.second, drops);
            }
        }
    }

    /**
     * 获取 k 对应的 value，存储到 result 中
     * 如果不存在，返回 false，否则返回 true
     *
     * NOTE: 这个访问会将该记录挪到最前，当成最近的记录
     */
    bool get(const K& k, V& result) {
        boost::unique_lock<boost::shared_mutex> lock(mutex_);
        return _get_no_lock<true>(k, result);
    }

    void multi_get(const std::vector<K>& keys, std::unordered_map<K, V, Hash>& result) {
        boost::unique_lock<boost::shared_mutex> lock(mutex_);
        for (const auto& key : keys) {
            V v;
            if (_get_no_lock<true>(key, v)) {
                result[key] = v;
            }
        }
    }

    void multi_get(const std::vector<K>& keys, std::unordered_map<K, V, Hash>& result, std::vector<K>& missed_keys) {
        boost::unique_lock<boost::shared_mutex> lock(mutex_);
        for (const auto& key : keys) {
            V v;
            if (_get_no_lock<true>(key, v)) {
                result[key] = v;
            } else {
                missed_keys.push_back(key);
            }
        }
    }

    /**
     * 取 k 对应的 value，存储到 result 中
     * 如果不存在，返回 false，否则返回 true
     *
     * NOTE: 和 get 的区别是: 这里并不将记录挪动，因此只需要挂读锁，
     *       并发性和性能会更好
     */
    bool peek(const K& k, V& result) {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        return _get_no_lock<false>(k, result);
    }

    void multi_peek(const std::vector<K>& keys, std::unordered_map<K, V, Hash>& result) {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        for (const auto& key : keys) {
            V v;
            if (_get_no_lock<false>(key, v)) {
                result[key] = v;
            }
        }
    }

    /**
     * 尝试添加，如果已存在，返回false，并赋值result，否则返回true，成功添加
     */
    bool try_add(const K& k, const V& v, V& result) {
        std::vector<KeyValue<K, V>> drops;
        {
            boost::unique_lock<boost::shared_mutex> lock(mutex_);
            if (_get_no_lock<true>(k, result)) {
                return false;
            }
            _add_no_lock(k, v, drops);
        }
        return true;
    }

    /**
     * 删除一条记录，存在，则返回 true，否则返回 false
     */
    bool remove(const K& k) {
        std::vector<KeyValue<K, V>> drops;
        {
            boost::unique_lock<boost::shared_mutex> lock(mutex_);
            const auto& iter = cache_.find(k);
            if (iter == cache_.end()) {
                return false;
            }
            drops.emplace_back(*(iter->second));
            list_.erase(iter->second);
            cache_.erase(iter);
        }
        return true;
    }

    /**
     * 返回是否存在某个 key
     */
    bool contains(const K& k) {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        return cache_.find(k) != cache_.end();
    }

    /**
     * 批量查询某个 key 是否存在
     */
    void multi_contains(const std::vector<K>& keys, std::unordered_map<K, bool, Hash>& result) {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        for (const auto& k : keys) {
            result[k] = cache_.find(k) != cache_.end();
        }
    }

    void clear() {
        ListType drops;
        {
            boost::unique_lock<boost::shared_mutex> lock(mutex_);
            cache_.clear();
            drops.swap(list_);
        }
    }

    /**
     * 访问 cache 中所有的 key value 对
     * f 是 void (const K&, const V&) 类型的
     */
    void iter_all(std::function<void(const K&, const V&)> f) {
        boost::shared_lock<boost::shared_mutex> lock(mutex_);
        for (auto& kv : list_) {
            f(kv.key, kv.value);
        }
    }

private:
    template <bool move_to_front = false>
    bool _get_no_lock(const K& k, V& result) {
        const auto& iter = cache_.find(k);
        if (iter == cache_.end())
            return false;
        if (move_to_front) {
            list_.splice(list_.begin(), list_, iter->second);
        }
        result = iter->second->value;
        return true;
    }

    void _add_no_lock(const K& k, const V& v) {
        const auto& iter = cache_.find(k);
        if (iter != cache_.end()) {
            iter->second->value = v;
            list_.splice(list_.begin(), list_, iter->second);
        } else {
            list_.emplace_front(KeyValue<K, V>(k, v));
            cache_[k] = list_.begin();
        }

        while (cache_.size() > max_size_) {
            cache_.erase(list_.back().key);
            list_.pop_back();
        }
    }

    void _add_no_lock(const K& k, const V& v, std::vector<KeyValue<K, V>>& drops) {
        const auto& iter = cache_.find(k);
        if (iter != cache_.end()) {
            iter->second->value = v;
            list_.splice(list_.begin(), list_, iter->second);
        } else {
            list_.emplace_front(KeyValue<K, V>(k, v));
            cache_[k] = list_.begin();
        }

        while (cache_.size() > max_size_) {
            drops.emplace_back(list_.back());
            cache_.erase(list_.back().key);
            list_.pop_back();
        }
    }

private:
    MapType cache_;
    ListType list_;
    size_t max_size_;
    boost::shared_mutex mutex_;
};

template <typename K, typename V, typename Hash = std::hash<K>>
class ExpiringLruCache {
public:
    typedef std::pair<V, int64_t> ValType;
    typedef LruCache<K, ValType, Hash> CacheType;

    ExpiringLruCache(const ExpiringLruCache&) = delete;
    const ExpiringLruCache& operator=(const ExpiringLruCache&) = delete;

    /**
     * @param max_size     lru size
     * @param timeout_sec  timeout seconds
     */
    ExpiringLruCache(size_t max_size, int timeout_sec) : cache_(max_size), timeout_sec_(timeout_sec) {
    }
    virtual ~ExpiringLruCache() {
    }

    /**
     * get current cache size
     */
    size_t size() {
        return cache_.size();
    }

    /**
     * get max size(capacity) of cache
     */
    size_t max_size() const {
        return cache_.max_size();
    }

    /**
     * check if cache is empty
     */
    bool empty() {
        return size() == 0;
    }

    /**
     * 增加一条记录，如果 cache 容量满了，会淘汰掉最旧的
     */
    void add(const K& k, const V& v) {
        cache_.add(k, std::make_pair(v, get_current_time_sec()));
    }

    void multi_add(const std::unordered_map<K, V>& key_values) {
        auto cur = get_current_time_sec();
        std::unordered_map<K, std::pair<V, int64_t>> timed_map{key_values.size()};
        for (auto& p : key_values) {
            timed_map.emplace(p.first, std::make_pair(p.second, cur));
        }
        cache_.multi_add(timed_map);
    }

    /**
     * 获取 k 对应的 value，存储到 result 中
     * 如果不存在，返回 false，否则返回 true
     *
     * NOTE: 这个访问会将该记录挪到最前，当成最近的记录
     */
    bool get(const K& k, V& result) {
        return _get(&CacheType::get, k, result);
    }

    void multi_get(const std::vector<K>& keys, std::unordered_map<K, V, Hash>& result) {
        _multi_get(&CacheType::multi_get, keys, result);
    }

    /**
     * 取 k 对应的 value，存储到 result 中
     * 如果不存在，返回 false，否则返回 true
     *
     * NOTE: 和 get 的区别是: 这里并不将记录挪动，因此只需要挂读锁，
     *       并发性和性能会更好
     */
    bool peek(const K& k, V& result) {
        return _get(&CacheType::peek, k, result);
    }

    void multi_peek(const std::vector<K>& keys, std::unordered_map<K, V, Hash>& result) {
        _multi_get(&CacheType::multi_peek, keys, result);
    }

    /**
     * 删除一条记录，存在，则返回 true，否则返回 false
     */
    bool remove(const K& k) {
        return cache_.remove(k);
    }

    /**
     * 返回是否存在某个 key
     */
    bool contains(const K& k) {
        V v;
        return peek(k, v);
    }

    void multi_contains(const std::vector<K>& keys, std::unordered_map<K, bool, Hash>& result) {
        std::unordered_map<K, V, Hash> tmp;
        multi_peek(keys, tmp);
        for (const auto& k : keys) {
            result[k] = tmp.find(k) != tmp.end();
        }
    }

    /**
     * 访问 cache 中所有的 key value 对
     * f 是 void (const K&, const V&) 类型的
     */
    void iter_all(std::function<void(const K&, const V&)> f) {
        auto cur = get_current_time_sec();
        auto timeout = timeout_sec_;
        auto newf = [&f, &cur, timeout](const K& k, const ValType& val) {
            if (cur - val.second > timeout)
                return;
            f(k, val.first);
        };
        cache_.iter_all(newf);
    }

private:
    typedef bool (CacheType::*GetFunc)(const K& k, ValType& val);
    bool _get(GetFunc f, const K& k, V& result) {
        ValType val;
        bool rt = (cache_.*f)(k, val);
        if (rt) {
            auto cur = get_current_time_sec();
            if (cur - val.second > timeout_sec_) {
                return false;
            }
            result = val.first;
            return true;
        }
        return false;
    }

    typedef void (CacheType::*MGetFunc)(const std::vector<K>&, std::unordered_map<K, ValType, Hash>&);
    void _multi_get(MGetFunc f, const std::vector<K>& keys, std::unordered_map<K, V, Hash>& result) {
        std::unordered_map<K, ValType, Hash> tmp;
        (cache_.*f)(keys, tmp);
        auto thres = get_current_time_sec() - timeout_sec_;
        for (auto p : tmp) {
            if (p.second.second < thres)
                continue;
            result[p.first] = p.second.first;
        }
    }

private:
    CacheType cache_;
    int timeout_sec_;
};

}  // namespace cpputil
