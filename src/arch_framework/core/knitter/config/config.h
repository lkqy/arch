#pragma once
#include <cpputil/program/conf.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <unordered_map>
#include "knitter/utils/log.h"

namespace knitter {
namespace common {

class Config {
public:
    using SPtr = std::shared_ptr<Config>;

    explicit Config(const std::string& conf_file) : conf_file_(conf_file), conf_(conf_file) {
    }

    ~Config() = default;

private:
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

public:
    template <typename Arg>
    static void init_instance(Arg&& arg) {
        instance_ = std::make_shared<Config>(std::forward<Arg>(arg));
    }

    static Config& instance() {
        if (instance_) {
            return *instance_;
        } else {
            PANIC("config instance is not inited");
        }
    }

    std::string get(const std::string& key, const std::string& default_value = "") const {
        auto search = inner_data_.find(key);
        if (search != inner_data_.end()) {
            return search->second;
        } else {
            return find_from_conf(key, default_value);
        }
    }

    template <typename T>
    T cast_get(const std::string& key, const std::string& default_value = "") const {
        try {
            auto val = get(key, default_value);
            return boost::lexical_cast<T>(val);
        } catch (std::exception& e) {
            ERROR("cast_get config[%s] exception: %s", key.c_str(), e.what());
            exit(-1);
        }
    }

    template <typename T>
    std::vector<T> cast_get_values(const std::string& key) const {
        try {
            auto vals = conf_.get_values(key, true);
            std::vector<T> ret;
            ret.reserve(vals.size());
            for (auto val : vals)
                ret.push_back(boost::lexical_cast<T>(val));
            return ret;
        } catch (std::exception& e) {
            ERROR("cast_get_values config[%s] exception: %s", key.c_str(), e.what());
            exit(-1);
        }
    }

    cpputil::program::Conf& get_conf() {
        return conf_;
    }

    bool set(const std::string& key, const std::string& value) {
        auto search = inner_data_.find(key);
        if (search != inner_data_.end()) {
            WARN("config[%s] was overrided, old value is[%s]", search->first.c_str(), search->second.c_str());
        }

        inner_data_[key] = value;
        return true;
    }

    bool get_bool(const std::string& key, bool default_value) const {
        auto val = get(key, default_value ? "1" : "0");
        if (default_value) {
            // 默认值是true，假如配置格式错误，也返回true
            return (val == "0" || val == "false") ? false : true;
        } else {
            // 默认值是false，假如配置格式错误，也返回false
            return (val == "1" || val == "true") ? true : false;
        }
    }

private:
    std::string find_from_conf(const std::string& key, const std::string& default_value) const {
        try {
            std::string val = conf_.get(key, default_value);
            if (val.empty()) {
                ERROR("config[%s] is empty", key.c_str());
            }
            return val;
        } catch (std::exception& e) {
            ERROR("config[%s] get exception: %s", key.c_str(), e.what());
        }

        return default_value;
    }

private:
    std::string conf_file_;
    cpputil::program::Conf conf_;
    std::unordered_map<std::string, std::string> inner_data_;
    static std::shared_ptr<Config> instance_;
};

}  // namespace common
}  // namespace knitter
