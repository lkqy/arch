#include "knitter/utils/network.h"
#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace knitter {
namespace utils {

// 获取可用的端口, 返回值为true时port为可用的端口号
bool get_avaliable_port(int& port) {
    bool result = true;

    // 1. 创建一个socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // 2. 创建一个sockaddr，并将它的端口号设为0
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = 0;  // 若port指定为0,则调用bind时，系统会为其指定一个可用的端口号

    // 3. 绑定
    int ret = bind(sock, (sockaddr*)&addr, sizeof addr);

    if (0 != ret) {
        result = false;
        close(sock);
        return result;
    }

    // 4. 利用getsockname获取
    struct sockaddr_in connAddr;
    unsigned int len = sizeof(connAddr);
    ret = getsockname(sock, (struct sockaddr*)&connAddr, &len);

    if (0 != ret) {
        result = false;
        close(sock);
        return result;
    }

    port = ntohs(connAddr.sin_port);  // 获取端口号
    close(sock);
    return result;
}

int get_local_network_info(std::string& host_name, std::string& ip_address, unsigned long& ip_integer) {
    char host[256];
    if (gethostname(host, sizeof(host)) != 0) {
        return -1;
    }
    host_name = host;

    struct hostent* he;
    if ((he = gethostbyname(host)) == NULL) {
        return -2;
    }

    char ip[64];
    unsigned long ip_int;
    struct in_addr** addr_list = (struct in_addr**)he->h_addr_list;
    int i;
    for (i = 0; addr_list[i] != NULL; i++) {
        // Return the first one;
        strcpy(ip, inet_ntoa(*addr_list[i]));
        uint8_t addr[4];
        memcpy(addr, addr_list[i], 4);
        ip_int = addr[0] * 1000000000UL + addr[1] * 1000000UL + addr[2] * 1000UL + addr[3];
        break;
    }
    if (!addr_list[i]) {
        return -3;
    }

    ip_address = ip;
    ip_integer = ip_int;

    return 0;
}

}  // namespace utils
}  // namespace knitter