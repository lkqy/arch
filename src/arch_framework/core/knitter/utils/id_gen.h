#pragma once
#include <inttypes.h>
#include <string>

namespace knitter {
namespace utils {

uint64_t increased_unique_id();

// 带时间标识的unique id
std::string ts_unique_id();

// client唯一标志
std::string ts_unique_client_id();

// server唯一标志
std::string ts_unique_server_id();

uint64_t random_id();

uint64_t random_id_greater(uint64_t min);

uint64_t random_id_less(uint64_t max);

uint64_t random_id_between(uint64_t min, uint64_t max);

std::pair<std::string, uint64_t> ts_unique_pair();

}  // namespace utils
}  // namespace knitter
