#include "pdx/status.h"

#include <pdx/rpc/serialization.h>
#include <string.h>

namespace android {
namespace pdx {

std::string ErrorStatus::ErrorToString(int error_code) {
  char message[1024] = {};
  return strerror_r(error_code, message, sizeof(message));
}

}  // namespace pdx
}  // namespace android
