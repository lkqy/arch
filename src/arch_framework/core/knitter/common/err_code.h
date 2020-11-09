#pragma once

namespace knitter {
namespace common {

class ErrCode {
public:
    enum Code : int {
        Success = 0,

        InvalidGraph = -101,

        UnknownError = -100,
    };

    static const char* message(int code);
};

}  // namespace common
}  // namespace knitter
