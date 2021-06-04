#ifndef ANDROID_DVR_SERVICES_VRFLINGER_DISPLAY_MANAGER_SERVICE_H_
#define ANDROID_DVR_SERVICES_VRFLINGER_DISPLAY_MANAGER_SERVICE_H_

#include <pdx/service.h>
#include <pdx/status.h>
#include <private/dvr/display_protocol.h>

#include "display_service.h"

namespace android {
namespace dvr {

class DisplayManagerService;

// The display manager is a client of the display manager service. This class
// represents the connected client that the display manager service sends
// notifications to.
class DisplayManager : public pdx::Channel {
 public:
  DisplayManager(DisplayManagerService* service, int channel_id)
      : service_(service), channel_id_(channel_id) {}

  int channel_id() const { return channel_id_; }

  // Sets or clears the channel event mask to indicate pending events that the
  // display manager on the other end of the channel should read and handle.
  // When |pending| is true the POLLIN bit is set in the event mask; when
  // |pending| is false the POLLIN bit is cleared in the event mask.
  void SetNotificationsPending(bool pending);

 private:
  DisplayManager(const DisplayManager&) = delete;
  void operator=(const DisplayManager&) = delete;

  DisplayManagerService* service_;
  int channel_id_;
};

// The display manager service marshalls state and events from the display
// service to the display manager.
class DisplayManagerService : public pdx::ServiceBase<DisplayManagerService> {
 public:
  std::shared_ptr<pdx::Channel> OnChannelOpen(pdx::Message& message) override;
  void OnChannelClose(pdx::Message& message,
                      const std::shared_ptr<pdx::Channel>& channel) override;
  pdx::Status<void> HandleMessage(pdx::Message& message) override;

 private:
  friend BASE;

  explicit DisplayManagerService(
      const std::shared_ptr<DisplayService>& display_service);

  pdx::Status<std::vector<display::SurfaceState>> OnGetSurfaceState(
      pdx::Message& message);
  pdx::Status<pdx::LocalChannelHandle> OnGetSurfaceQueue(pdx::Message& message,
                                                         int surface_id,
                                                         int queue_id);

  // Called by the display service to indicate changes to display surfaces that
  // the display manager should evaluate.
  void OnDisplaySurfaceChange();

  DisplayManagerService(const DisplayManagerService&) = delete;
  void operator=(const DisplayManagerService&) = delete;

  std::shared_ptr<DisplayService> display_service_;
  std::shared_ptr<DisplayManager> display_manager_;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_SERVICES_VRFLINGER_DISPLAY_MANAGER_SERVICE_H_
