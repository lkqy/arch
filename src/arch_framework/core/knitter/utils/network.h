#pragma once
#include <string>

namespace knitter {
namespace utils {

// 获取可用的端口, 返回值为true时port为可用的端口号
bool get_avaliable_port(int& port);

int get_local_network_info(std::string& host_name, std::string& ip_address, unsigned long& ip_integer);

}  // namespace utils
}  // namespace knitter