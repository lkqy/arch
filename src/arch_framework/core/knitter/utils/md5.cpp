#include "knitter/utils/md5.h"
#include <openssl/md5.h>

namespace knitter {
namespace utils {

std::string md5_new(const std::string& input) {
    std::string output;
    output.resize(16);
    MD5((unsigned char*)input.c_str(), input.size(), (unsigned char*)output.c_str());
    return output;
}

}  // namespace utils
}  // namespace knitter