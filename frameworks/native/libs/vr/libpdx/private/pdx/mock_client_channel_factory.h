#ifndef ANDROID_PDX_MOCK_CLIENT_CHANNEL_FACTORY_H_
#define ANDROID_PDX_MOCK_CLIENT_CHANNEL_FACTORY_H_

#include <gmock/gmock.h>
#include <pdx/client_channel_factory.h>

namespace android {
namespace pdx {

class MockClientChannelFactory : public ClientChannelFactory {
 public:
  MOCK_CONST_METHOD1(
      Connect, Status<std::unique_ptr<ClientChannel>>(int64_t timeout_ms));
};

}  // namespace pdx
}  // namespace android

#endif  // ANDROID_PDX_MOCK_CLIENT_CHANNEL_FACTORY_H_
