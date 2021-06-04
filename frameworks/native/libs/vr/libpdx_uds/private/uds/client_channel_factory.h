#ifndef ANDROID_PDX_UDS_CLIENT_CHANNEL_FACTORY_H_
#define ANDROID_PDX_UDS_CLIENT_CHANNEL_FACTORY_H_

#include <string>

#include <pdx/client_channel_factory.h>

namespace android {
namespace pdx {
namespace uds {

class ClientChannelFactory : public pdx::ClientChannelFactory {
 public:
  static std::unique_ptr<pdx::ClientChannelFactory> Create(
      const std::string& endpoint_path);
  static std::unique_ptr<pdx::ClientChannelFactory> Create(LocalHandle socket);

  Status<std::unique_ptr<pdx::ClientChannel>> Connect(
      int64_t timeout_ms) const override;

  static std::string GetRootEndpointPath();
  static std::string GetEndpointPath(const std::string& endpoint_path);

 private:
  explicit ClientChannelFactory(const std::string& endpoint_path);
  explicit ClientChannelFactory(LocalHandle socket);

  mutable LocalHandle socket_;
  std::string endpoint_path_;
};

}  // namespace uds
}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_UDS_CLIENT_CHANNEL_FACTORY_H_
