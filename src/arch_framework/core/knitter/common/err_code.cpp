#include "knitter/common/err_code.h"

namespace knitter {
namespace common {

const char* ErrCode::message(int code) {
    switch (code) {
        case Success:
            return "success";

        case UnknownError:
            return "unknown error";

        default:
            return "unknown or other undescribable exception";
    }
}

}  // namespace common
}  // namespace knitter
