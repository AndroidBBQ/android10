#ifndef ANDROID_DVR_SERVICES_DISPLAYD_DISPLAY_SURFACE_H_
#define ANDROID_DVR_SERVICES_DISPLAYD_DISPLAY_SURFACE_H_

#include <pdx/file_handle.h>
#include <pdx/service.h>
#include <private/dvr/buffer_hub_queue_client.h>
#include <private/dvr/display_protocol.h>
#include <private/dvr/ring_buffer.h>

#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "acquired_buffer.h"

namespace android {
namespace dvr {

class DisplayService;

enum class SurfaceType {
  Direct,
  Application,
};

class DisplaySurface : public pdx::Channel {
 public:
  static pdx::Status<std::shared_ptr<DisplaySurface>> Create(
      DisplayService* service, int surface_id, int process_id, int user_id,
      const display::SurfaceAttributes& attributes);

  ~DisplaySurface() override;

  DisplayService* service() const { return service_; }
  SurfaceType surface_type() const { return surface_type_; }
  int surface_id() const { return surface_id_; }
  int process_id() const { return process_id_; }
  int user_id() const { return user_id_; }

  bool visible() const { return visible_; }
  int z_order() const { return z_order_; }

  const display::SurfaceAttributes& attributes() const { return attributes_; }
  display::SurfaceUpdateFlags update_flags() const { return update_flags_; }

  virtual std::vector<int32_t> GetQueueIds() const { return {}; }

  bool IsUpdatePending() const {
    return update_flags_.value() != display::SurfaceUpdateFlags::None;
  }

 protected:
  DisplaySurface(DisplayService* service, SurfaceType surface_type,
                 int surface_id, int process_id, int user_id);

  // Utility to retrieve a shared pointer to this channel as the desired derived
  // type.
  template <
      typename T = DisplaySurface,
      typename = std::enable_if_t<std::is_base_of<DisplaySurface, T>::value>>
  std::shared_ptr<T> Self() {
    return std::static_pointer_cast<T>(shared_from_this());
  }

  virtual pdx::Status<pdx::LocalChannelHandle> OnCreateQueue(
      pdx::Message& message, const ProducerQueueConfig& config) = 0;

  // Registers a consumer queue with the event dispatcher in DisplayService. The
  // OnQueueEvent callback below is called to handle queue events.
  pdx::Status<void> RegisterQueue(
      const std::shared_ptr<ConsumerQueue>& consumer_queue);
  pdx::Status<void> UnregisterQueue(
      const std::shared_ptr<ConsumerQueue>& consumer_queue);

  // Called by the event dispatcher in DisplayService when a registered queue
  // event triggers. Executes on the event dispatcher thread.
  virtual void OnQueueEvent(
      const std::shared_ptr<ConsumerQueue>& consumer_queue, int events);

  void SurfaceUpdated(display::SurfaceUpdateFlags update_flags);
  void ClearUpdate();

  // Synchronizes access to mutable state below between message dispatch thread
  // and frame post thread.
  mutable std::mutex lock_;

 private:
  friend class DisplayService;
  friend class DisplayManagerService;

  // Dispatches display surface messages to the appropriate handlers. This
  // handler runs on the VrFlinger message dispatch thread.
  pdx::Status<void> HandleMessage(pdx::Message& message);

  pdx::Status<void> OnSetAttributes(
      pdx::Message& message, const display::SurfaceAttributes& attributes);
  pdx::Status<display::SurfaceInfo> OnGetSurfaceInfo(pdx::Message& message);

  DisplayService* service_;
  SurfaceType surface_type_;
  int surface_id_;
  int process_id_;
  int user_id_;

  display::SurfaceAttributes attributes_;
  display::SurfaceUpdateFlags update_flags_ = display::SurfaceUpdateFlags::None;

  // Subset of attributes that may be interpreted by the display service.
  bool visible_ = false;
  int z_order_ = 0;

  DisplaySurface(const DisplaySurface&) = delete;
  void operator=(const DisplaySurface&) = delete;
};

class ApplicationDisplaySurface : public DisplaySurface {
 public:
  ApplicationDisplaySurface(DisplayService* service, int surface_id,
                            int process_id, int user_id)
      : DisplaySurface(service, SurfaceType::Application, surface_id,
                       process_id, user_id) {}

  std::shared_ptr<ConsumerQueue> GetQueue(int32_t queue_id);
  std::vector<int32_t> GetQueueIds() const override;

 private:
  pdx::Status<pdx::LocalChannelHandle> OnCreateQueue(
      pdx::Message& message, const ProducerQueueConfig& config) override;
  void OnQueueEvent(const std::shared_ptr<ConsumerQueue>& consumer_queue,
                    int events) override;

  // Accessed by both message dispatch thread and epoll event thread.
  std::unordered_map<int32_t, std::shared_ptr<ConsumerQueue>> consumer_queues_;
};

class DirectDisplaySurface : public DisplaySurface {
 public:
  DirectDisplaySurface(DisplayService* service, int surface_id, int process_id,
                       int user_id)
      : DisplaySurface(service, SurfaceType::Direct, surface_id, process_id,
                       user_id),
        acquired_buffers_(kMaxPostedBuffers),
        metadata_(nullptr) {}
  std::vector<int32_t> GetQueueIds() const override;
  bool IsBufferAvailable();
  bool IsBufferPosted();
  AcquiredBuffer AcquireCurrentBuffer();

  // Get the newest buffer. Up to one buffer will be skipped. If a buffer is
  // skipped, it will be stored in skipped_buffer if non null.
  AcquiredBuffer AcquireNewestAvailableBuffer(AcquiredBuffer* skipped_buffer);

 private:
  pdx::Status<pdx::LocalChannelHandle> OnCreateQueue(
      pdx::Message& message, const ProducerQueueConfig& config) override;
  void OnQueueEvent(const std::shared_ptr<ConsumerQueue>& consumer_queue,
                    int events) override;

  // The capacity of the pending buffer queue. Should be enough to hold all the
  // buffers of this DisplaySurface, although in practice only 1 or 2 frames
  // will be pending at a time.
  static constexpr int kSurfaceBufferMaxCount = 4;
  static constexpr int kSurfaceViewMaxCount = 4;
  static constexpr int kMaxPostedBuffers =
      kSurfaceBufferMaxCount * kSurfaceViewMaxCount;

  // Returns whether a frame is available without locking the mutex.
  bool IsFrameAvailableNoLock() const;

  // Dequeue all available buffers from the consumer queue.
  void DequeueBuffersLocked();

  // In a triple-buffered surface, up to kMaxPostedBuffers buffers may be
  // posted and pending.
  RingBuffer<AcquiredBuffer> acquired_buffers_;

  std::shared_ptr<ConsumerQueue> direct_queue_;

  // Stores metadata when it dequeue buffers from consumer queue.
  std::unique_ptr<uint8_t[]> metadata_;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_SERVICES_DISPLAYD_DISPLAY_SURFACE_H_
