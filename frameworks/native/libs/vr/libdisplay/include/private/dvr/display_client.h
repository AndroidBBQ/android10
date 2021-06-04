#ifndef ANDROID_DVR_DISPLAY_CLIENT_H_
#define ANDROID_DVR_DISPLAY_CLIENT_H_

#include <dvr/dvr_api.h>
#include <hardware/hwcomposer.h>
#include <pdx/client.h>
#include <pdx/file_handle.h>
#include <private/dvr/buffer_hub_queue_client.h>
#include <private/dvr/display_protocol.h>

namespace android {
namespace dvr {
namespace display {

class Surface : public pdx::ClientBase<Surface> {
 public:
  // Utility named constructor. This can be removed once ClientBase::Create is
  // refactored to return Status<T> types.
  static pdx::Status<std::unique_ptr<Surface>> CreateSurface(
      const SurfaceAttributes& attributes) {
    int error;
    pdx::Status<std::unique_ptr<Surface>> status;
    if (auto surface = Create(attributes, &error))
      status.SetValue(std::move(surface));
    else
      status.SetError(error);
    return status;
  }

  int surface_id() const { return surface_id_; }
  int z_order() const { return z_order_; }
  bool visible() const { return visible_; }

  pdx::Status<void> SetVisible(bool visible);
  pdx::Status<void> SetZOrder(int z_order);
  pdx::Status<void> SetAttributes(const SurfaceAttributes& attributes);

  // Creates an empty queue.
  pdx::Status<std::unique_ptr<ProducerQueue>> CreateQueue(uint32_t width,
                                                          uint32_t height,
                                                          uint32_t format,
                                                          size_t metadata_size);

  // Creates a queue and populates it with |capacity| buffers of the specified
  // parameters.
  pdx::Status<std::unique_ptr<ProducerQueue>> CreateQueue(uint32_t width,
                                                          uint32_t height,
                                                          uint32_t layer_count,
                                                          uint32_t format,
                                                          uint64_t usage,
                                                          size_t capacity,
                                                          size_t metadata_size);

 private:
  friend BASE;

  int surface_id_ = -1;
  int z_order_ = 0;
  bool visible_ = false;

  // TODO(eieio,avakulenko): Remove error param once pdx::ClientBase::Create()
  // returns Status<T>.
  explicit Surface(const SurfaceAttributes& attributes, int* error = nullptr);
  explicit Surface(pdx::LocalChannelHandle channel_handle,
                   int* error = nullptr);

  Surface(const Surface&) = delete;
  void operator=(const Surface&) = delete;
};

class DisplayClient : public pdx::ClientBase<DisplayClient> {
 public:
  pdx::Status<Metrics> GetDisplayMetrics();
  pdx::Status<std::string> GetConfigurationData(ConfigFileType config_type);
  pdx::Status<std::unique_ptr<IonBuffer>> SetupGlobalBuffer(
      DvrGlobalBufferKey key, size_t size, uint64_t usage);
  pdx::Status<void> DeleteGlobalBuffer(DvrGlobalBufferKey key);
  pdx::Status<std::unique_ptr<IonBuffer>> GetGlobalBuffer(
      DvrGlobalBufferKey key);
  pdx::Status<std::unique_ptr<Surface>> CreateSurface(
      const SurfaceAttributes& attributes);

  // Temporary query for current VR status. Will be removed later.
  pdx::Status<bool> IsVrAppRunning();

 private:
  friend BASE;

  explicit DisplayClient(int* error = nullptr);

  DisplayClient(const DisplayClient&) = delete;
  void operator=(const DisplayClient&) = delete;
};

}  // namespace display
}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_DISPLAY_CLIENT_H_
