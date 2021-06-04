#ifndef ANDROID_DVR_DISPLAY_MANAGER_CLIENT_H_
#define ANDROID_DVR_DISPLAY_MANAGER_CLIENT_H_

#include <string>
#include <vector>

#include <pdx/client.h>
#include <pdx/status.h>
#include <private/dvr/display_protocol.h>

namespace android {
namespace dvr {

class IonBuffer;
class ConsumerQueue;

namespace display {

class DisplayManagerClient : public pdx::ClientBase<DisplayManagerClient> {
 public:
  ~DisplayManagerClient() override;

  pdx::Status<std::vector<SurfaceState>> GetSurfaceState();
  pdx::Status<std::unique_ptr<ConsumerQueue>> GetSurfaceQueue(int surface_id,
                                                              int queue_id);

  using Client::event_fd;

  pdx::Status<int> GetEventMask(int events) {
    if (auto* client_channel = GetChannel())
      return client_channel->GetEventMask(events);
    else
      return pdx::ErrorStatus(EINVAL);
  }

 private:
  friend BASE;

  DisplayManagerClient();

  DisplayManagerClient(const DisplayManagerClient&) = delete;
  void operator=(const DisplayManagerClient&) = delete;
};

}  // namespace display
}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_DISPLAY_MANAGER_CLIENT_H_
