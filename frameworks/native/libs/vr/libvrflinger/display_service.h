#ifndef ANDROID_DVR_SERVICES_DISPLAYD_DISPLAY_SERVICE_H_
#define ANDROID_DVR_SERVICES_DISPLAYD_DISPLAY_SERVICE_H_

#include <dvr/dvr_api.h>
#include <pdx/service.h>
#include <pdx/status.h>
#include <private/dvr/bufferhub_rpc.h>
#include <private/dvr/display_protocol.h>

#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "acquired_buffer.h"
#include "display_surface.h"
#include "epoll_event_dispatcher.h"
#include "hardware_composer.h"

namespace android {
namespace dvr {

// DisplayService implements the display service component of VrFlinger.
class DisplayService : public pdx::ServiceBase<DisplayService> {
 public:
  bool IsInitialized() const override;
  std::string DumpState(size_t max_length) override;

  void OnChannelClose(pdx::Message& message,
                      const std::shared_ptr<pdx::Channel>& channel) override;
  pdx::Status<void> HandleMessage(pdx::Message& message) override;

  std::shared_ptr<DisplaySurface> GetDisplaySurface(int surface_id) const;
  std::vector<std::shared_ptr<DisplaySurface>> GetDisplaySurfaces() const;
  std::vector<std::shared_ptr<DirectDisplaySurface>> GetVisibleDisplaySurfaces()
      const;

  // Updates the list of actively displayed surfaces. This must be called after
  // any change to client/manager attributes that affect visibility or z order.
  void UpdateActiveDisplaySurfaces();

  pdx::Status<BorrowedNativeBufferHandle> SetupGlobalBuffer(
      DvrGlobalBufferKey key, size_t size, uint64_t usage);

  pdx::Status<void> DeleteGlobalBuffer(DvrGlobalBufferKey key);

  template <class A>
  void ForEachDisplaySurface(SurfaceType surface_type, A action) const {
    ForEachChannel([surface_type,
                    action](const ChannelIterator::value_type& pair) mutable {
      auto surface = std::static_pointer_cast<DisplaySurface>(pair.second);
      if (surface->surface_type() == surface_type)
        action(surface);
    });
  }

  using DisplayConfigurationUpdateNotifier = std::function<void(void)>;
  void SetDisplayConfigurationUpdateNotifier(
      DisplayConfigurationUpdateNotifier notifier);

  void GrantDisplayOwnership() { hardware_composer_.Enable(); }
  void SeizeDisplayOwnership() { hardware_composer_.Disable(); }
  void OnBootFinished() { hardware_composer_.OnBootFinished(); }

 private:
  friend BASE;
  friend DisplaySurface;

  friend class VrDisplayStateService;

  using RequestDisplayCallback = std::function<void(bool)>;

  DisplayService(android::Hwc2::Composer* hidl,
                 hwc2_display_t primary_display_id,
                 RequestDisplayCallback request_display_callback);

  pdx::Status<BorrowedNativeBufferHandle> OnGetGlobalBuffer(
      pdx::Message& message, DvrGlobalBufferKey key);
  pdx::Status<display::Metrics> OnGetMetrics(pdx::Message& message);
  pdx::Status<std::string> OnGetConfigurationData(
      pdx::Message& message, display::ConfigFileType config_type);
  pdx::Status<display::SurfaceInfo> OnCreateSurface(
      pdx::Message& message, const display::SurfaceAttributes& attributes);
  pdx::Status<BorrowedNativeBufferHandle> OnSetupGlobalBuffer(
      pdx::Message& message, DvrGlobalBufferKey key, size_t size,
      uint64_t usage);
  pdx::Status<void> OnDeleteGlobalBuffer(pdx::Message& message,
                                         DvrGlobalBufferKey key);

  // Temporary query for current VR status. Will be removed later.
  pdx::Status<bool> IsVrAppRunning(pdx::Message& message);

  pdx::Status<void> AddEventHandler(int fd, int events,
                                    EpollEventDispatcher::Handler handler) {
    return dispatcher_.AddEventHandler(fd, events, handler);
  }
  pdx::Status<void> RemoveEventHandler(int fd) {
    return dispatcher_.RemoveEventHandler(fd);
  }

  void SurfaceUpdated(SurfaceType surface_type,
                      display::SurfaceUpdateFlags update_flags);

  // Called by DisplaySurface to signal that a surface property has changed and
  // the display manager should be notified.
  void NotifyDisplayConfigurationUpdate();

  pdx::Status<void> HandleSurfaceMessage(pdx::Message& message);

  HardwareComposer hardware_composer_;
  EpollEventDispatcher dispatcher_;
  DisplayConfigurationUpdateNotifier update_notifier_;

  std::unordered_map<DvrGlobalBufferKey, std::unique_ptr<IonBuffer>>
      global_buffers_;

  DisplayService(const DisplayService&) = delete;
  void operator=(const DisplayService&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_SERVICES_DISPLAYD_DISPLAY_SERVICE_H_
