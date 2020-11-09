#pragma once
#include <string>

namespace knitter {
namespace utils {

std::string md5_new(const std::string& input);

// results 是一个MD5后的字符串，取前4字节作为哈希值
inline uint32_t md5_hash_value(const char* results) {
    return ((uint32_t)(results[3] & 0xFF) << 24) | ((uint32_t)(results[2] & 0xFF) << 16) |
           ((uint32_t)(results[1] & 0xFF) << 8) | (results[0] & 0xFF);
}

inline uint32_t md5_gen_key(const std::string& key) {
    std::string md5_key = md5_new(key);
    return md5_hash_value(md5_key.c_str());
}

// 返回至多4个哈希值
inline void md5_gen_key(const std::string& key, uint32_t* output, size_t size) {
    std::string md5_key = md5_new(key);
    char* buffs = (char*)&md5_key[0];  // 16位定长
    for (size_t j = 0; j < 4 && j < size; j++) {
        char* buf = buffs + j * 4;
        uint32_t v = md5_hash_value(buf);
        output[j] = v;
    }
}

}  // namespace utils
}  // namespace knitter