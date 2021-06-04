#ifndef ANDROID_PDX_CLIENT_CHANNEL_FACTORY_H_
#define ANDROID_PDX_CLIENT_CHANNEL_FACTORY_H_

#include <pdx/client_channel.h>
#include <pdx/status.h>

namespace android {
namespace pdx {

class ClientChannelFactory {
 public:
  virtual ~ClientChannelFactory() = default;

  virtual Status<std::unique_ptr<ClientChannel>> Connect(
      int64_t timeout_ms) const = 0;
};

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_CLIENT_CHANNEL_FACTORY_H_
