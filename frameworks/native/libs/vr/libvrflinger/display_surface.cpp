#include "display_surface.h"

#include <private/android_filesystem_config.h>
#include <utils/Trace.h>

#include <private/dvr/trusted_uids.h>

#include "display_service.h"
#include "hardware_composer.h"

#define LOCAL_TRACE 1

using android::dvr::display::DisplayProtocol;
using android::pdx::BorrowedChannelHandle;
using android::pdx::ErrorStatus;
using android::pdx::LocalChannelHandle;
using android::pdx::LocalHandle;
using android::pdx::Message;
using android::pdx::RemoteChannelHandle;
using android::pdx::Status;
using android::pdx::rpc::DispatchRemoteMethod;
using android::pdx::rpc::IfAnyOf;

namespace android {
namespace dvr {

DisplaySurface::DisplaySurface(DisplayService* service,
                               SurfaceType surface_type, int surface_id,
                               int process_id, int user_id)
    : service_(service),
      surface_type_(surface_type),
      surface_id_(surface_id),
      process_id_(process_id),
      user_id_(user_id),
      update_flags_(display::SurfaceUpdateFlags::NewSurface) {}

DisplaySurface::~DisplaySurface() {
  ALOGD_IF(LOCAL_TRACE,
           "DisplaySurface::~DisplaySurface: surface_id=%d process_id=%d",
           surface_id(), process_id());
}

Status<void> DisplaySurface::HandleMessage(pdx::Message& message) {
  switch (message.GetOp()) {
    case DisplayProtocol::SetAttributes::Opcode:
      DispatchRemoteMethod<DisplayProtocol::SetAttributes>(
          *this, &DisplaySurface::OnSetAttributes, message);
      break;

    case DisplayProtocol::GetSurfaceInfo::Opcode:
      DispatchRemoteMethod<DisplayProtocol::GetSurfaceInfo>(
          *this, &DisplaySurface::OnGetSurfaceInfo, message);
      break;

    case DisplayProtocol::CreateQueue::Opcode:
      DispatchRemoteMethod<DisplayProtocol::CreateQueue>(
          *this, &DisplaySurface::OnCreateQueue, message);
      break;
  }

  return {};
}

Status<void> DisplaySurface::OnSetAttributes(
    pdx::Message& /*message*/, const display::SurfaceAttributes& attributes) {
  display::SurfaceUpdateFlags update_flags;

  for (const auto& attribute : attributes) {
    const auto key = attribute.first;
    const auto* variant = &attribute.second;
    bool invalid_value = false;
    bool visibility_changed = false;

    // Catch attributes that have significance to the display service.
    switch (key) {
      case display::SurfaceAttribute::ZOrder:
        invalid_value = !IfAnyOf<int32_t, int64_t, float>::Call(
            variant, [&](const auto& value) {
              if (z_order_ != value) {
                visibility_changed = true;
                z_order_ = value;
              }
            });
        break;
      case display::SurfaceAttribute::Visible:
        invalid_value = !IfAnyOf<int32_t, int64_t, bool>::Call(
            variant, [&](const auto& value) {
              if (visible_ != value) {
                visibility_changed = true;
                visible_ = value;
              }
            });
        break;
    }

    // Only update the attribute map with valid values. This check also has the
    // effect of preventing special attributes handled above from being deleted
    // by an empty value.
    if (invalid_value) {
      ALOGW(
          "DisplaySurface::OnClientSetAttributes: Failed to set display "
          "surface attribute '%d' because of incompatible type: %d",
          key, variant->index());
    } else {
      // An empty value indicates the attribute should be deleted.
      if (variant->empty()) {
        auto search = attributes_.find(key);
        if (search != attributes_.end())
          attributes_.erase(search);
      } else {
        attributes_[key] = *variant;
      }

      // All attribute changes generate a notification, even if the value
      // doesn't change. Visibility attributes set a flag only if the value
      // changes.
      update_flags.Set(display::SurfaceUpdateFlags::AttributesChanged);
      if (visibility_changed)
        update_flags.Set(display::SurfaceUpdateFlags::VisibilityChanged);
    }
  }

  SurfaceUpdated(update_flags);
  return {};
}

void DisplaySurface::SurfaceUpdated(display::SurfaceUpdateFlags update_flags) {
  ALOGD_IF(TRACE,
           "DisplaySurface::SurfaceUpdated: surface_id=%d update_flags=0x%x",
           surface_id(), update_flags.value());

  update_flags_.Set(update_flags);
  service()->SurfaceUpdated(surface_type(), update_flags_);
}

void DisplaySurface::ClearUpdate() {
  ALOGD_IF(TRACE > 1, "DisplaySurface::ClearUpdate: surface_id=%d",
           surface_id());
  update_flags_ = display::SurfaceUpdateFlags::None;
}

Status<display::SurfaceInfo> DisplaySurface::OnGetSurfaceInfo(
    Message& /*message*/) {
  ALOGD_IF(
      TRACE,
      "DisplaySurface::OnGetSurfaceInfo: surface_id=%d visible=%d z_order=%d",
      surface_id(), visible(), z_order());
  return {{surface_id(), visible(), z_order()}};
}

Status<void> DisplaySurface::RegisterQueue(
    const std::shared_ptr<ConsumerQueue>& consumer_queue) {
  ALOGD_IF(TRACE, "DisplaySurface::RegisterQueue: surface_id=%d queue_id=%d",
           surface_id(), consumer_queue->id());
  // Capture references for the lambda to work around apparent clang bug.
  // TODO(eieio): Figure out if there is a clang bug or C++11 ambiguity when
  // capturing self and consumer_queue by copy in the following case:
  //    auto self = Self();
  //    [self, consumer_queue](int events) {
  //        self->OnQueueEvent(consuemr_queue, events); }
  //
  struct State {
    std::shared_ptr<DisplaySurface> surface;
    std::shared_ptr<ConsumerQueue> queue;
  };
  State state{Self(), consumer_queue};

  return service()->AddEventHandler(
      consumer_queue->queue_fd(), EPOLLIN | EPOLLHUP | EPOLLET,
      [state](int events) {
        state.surface->OnQueueEvent(state.queue, events);
      });
}

Status<void> DisplaySurface::UnregisterQueue(
    const std::shared_ptr<ConsumerQueue>& consumer_queue) {
  ALOGD_IF(TRACE, "DisplaySurface::UnregisterQueue: surface_id=%d queue_id=%d",
           surface_id(), consumer_queue->id());
  return service()->RemoveEventHandler(consumer_queue->queue_fd());
}

void DisplaySurface::OnQueueEvent(
    const std::shared_ptr<ConsumerQueue>& /*consumer_queue*/, int /*events*/) {
  ALOGE(
      "DisplaySurface::OnQueueEvent: ERROR base virtual method should not be "
      "called!!!");
}

std::shared_ptr<ConsumerQueue> ApplicationDisplaySurface::GetQueue(
    int32_t queue_id) {
  ALOGD_IF(TRACE,
           "ApplicationDisplaySurface::GetQueue: surface_id=%d queue_id=%d",
           surface_id(), queue_id);

  std::lock_guard<std::mutex> autolock(lock_);
  auto search = consumer_queues_.find(queue_id);
  if (search != consumer_queues_.end())
    return search->second;
  else
    return nullptr;
}

std::vector<int32_t> ApplicationDisplaySurface::GetQueueIds() const {
  std::lock_guard<std::mutex> autolock(lock_);
  std::vector<int32_t> queue_ids;
  for (const auto& entry : consumer_queues_)
    queue_ids.push_back(entry.first);
  return queue_ids;
}

Status<LocalChannelHandle> ApplicationDisplaySurface::OnCreateQueue(
    Message& /*message*/, const ProducerQueueConfig& config) {
  ATRACE_NAME("ApplicationDisplaySurface::OnCreateQueue");
  ALOGD_IF(TRACE,
           "ApplicationDisplaySurface::OnCreateQueue: surface_id=%d, "
           "user_metadata_size=%zu",
           surface_id(), config.user_metadata_size);

  std::lock_guard<std::mutex> autolock(lock_);
  auto producer = ProducerQueue::Create(config, UsagePolicy{});
  if (!producer) {
    ALOGE(
        "ApplicationDisplaySurface::OnCreateQueue: Failed to create producer "
        "queue!");
    return ErrorStatus(ENOMEM);
  }

  std::shared_ptr<ConsumerQueue> consumer =
      producer->CreateSilentConsumerQueue();
  auto status = RegisterQueue(consumer);
  if (!status) {
    ALOGE(
        "ApplicationDisplaySurface::OnCreateQueue: Failed to register consumer "
        "queue: %s",
        status.GetErrorMessage().c_str());
    return status.error_status();
  }

  consumer_queues_[consumer->id()] = std::move(consumer);

  SurfaceUpdated(display::SurfaceUpdateFlags::BuffersChanged);
  return std::move(producer->GetChannelHandle());
}

void ApplicationDisplaySurface::OnQueueEvent(
    const std::shared_ptr<ConsumerQueue>& consumer_queue, int events) {
  ALOGD_IF(TRACE,
           "ApplicationDisplaySurface::OnQueueEvent: queue_id=%d events=%x",
           consumer_queue->id(), events);

  std::lock_guard<std::mutex> autolock(lock_);

  // Always give the queue a chance to handle its internal bookkeeping.
  consumer_queue->HandleQueueEvents();

  // Check for hangup and remove a queue that is no longer needed.
  if (consumer_queue->hung_up()) {
    ALOGD_IF(TRACE, "ApplicationDisplaySurface::OnQueueEvent: Removing queue.");
    UnregisterQueue(consumer_queue);
    auto search = consumer_queues_.find(consumer_queue->id());
    if (search != consumer_queues_.end()) {
      consumer_queues_.erase(search);
    } else {
      ALOGE(
          "ApplicationDisplaySurface::OnQueueEvent: Failed to find queue_id=%d",
          consumer_queue->id());
    }
    SurfaceUpdated(display::SurfaceUpdateFlags::BuffersChanged);
  }
}

std::vector<int32_t> DirectDisplaySurface::GetQueueIds() const {
  std::lock_guard<std::mutex> autolock(lock_);
  std::vector<int32_t> queue_ids;
  if (direct_queue_)
    queue_ids.push_back(direct_queue_->id());
  return queue_ids;
}

Status<LocalChannelHandle> DirectDisplaySurface::OnCreateQueue(
    Message& /*message*/, const ProducerQueueConfig& config) {
  ATRACE_NAME("DirectDisplaySurface::OnCreateQueue");
  ALOGD_IF(TRACE,
           "DirectDisplaySurface::OnCreateQueue: surface_id=%d "
           "user_metadata_size=%zu",
           surface_id(), config.user_metadata_size);

  std::lock_guard<std::mutex> autolock(lock_);
  if (!direct_queue_) {
    // Inject the hw composer usage flag to enable the display to read the
    // buffers.
    auto producer = ProducerQueue::Create(
        config, UsagePolicy{GraphicBuffer::USAGE_HW_COMPOSER, 0, 0, 0});
    if (!producer) {
      ALOGE(
          "DirectDisplaySurface::OnCreateQueue: Failed to create producer "
          "queue!");
      return ErrorStatus(ENOMEM);
    }

    direct_queue_ = producer->CreateConsumerQueue();
    if (direct_queue_->metadata_size() > 0) {
      metadata_.reset(new uint8_t[direct_queue_->metadata_size()]);
    }
    auto status = RegisterQueue(direct_queue_);
    if (!status) {
      ALOGE(
          "DirectDisplaySurface::OnCreateQueue: Failed to register consumer "
          "queue: %s",
          status.GetErrorMessage().c_str());
      return status.error_status();
    }

    return std::move(producer->GetChannelHandle());
  } else {
    return ErrorStatus(EALREADY);
  }
}

void DirectDisplaySurface::OnQueueEvent(
    const std::shared_ptr<ConsumerQueue>& consumer_queue, int events) {
  ALOGD_IF(TRACE, "DirectDisplaySurface::OnQueueEvent: queue_id=%d events=%x",
           consumer_queue->id(), events);

  std::lock_guard<std::mutex> autolock(lock_);

  // Always give the queue a chance to handle its internal bookkeeping.
  consumer_queue->HandleQueueEvents();

  // Check for hangup and remove a queue that is no longer needed.
  if (consumer_queue->hung_up()) {
    ALOGD_IF(TRACE, "DirectDisplaySurface::OnQueueEvent: Removing queue.");
    UnregisterQueue(consumer_queue);
    direct_queue_ = nullptr;
  }
}

void DirectDisplaySurface::DequeueBuffersLocked() {
  if (direct_queue_ == nullptr) {
    ALOGE(
        "DirectDisplaySurface::DequeueBuffersLocked: Consumer queue is not "
        "initialized.");
    return;
  }

  while (true) {
    LocalHandle acquire_fence;
    size_t slot;
    auto buffer_status = direct_queue_->Dequeue(
        0, &slot, metadata_.get(),
        direct_queue_->metadata_size(), &acquire_fence);
    ALOGD_IF(TRACE,
             "DirectDisplaySurface::DequeueBuffersLocked: Dequeue with metadata_size: %zu",
             direct_queue_->metadata_size());
    if (!buffer_status) {
      ALOGD_IF(
          TRACE > 1 && buffer_status.error() == ETIMEDOUT,
          "DirectDisplaySurface::DequeueBuffersLocked: All buffers dequeued.");
      ALOGE_IF(buffer_status.error() != ETIMEDOUT,
               "DirectDisplaySurface::DequeueBuffersLocked: Failed to dequeue "
               "buffer: %s",
               buffer_status.GetErrorMessage().c_str());
      return;
    }
    auto buffer_consumer = buffer_status.take();

    if (!visible()) {
      ATRACE_NAME("DropFrameOnInvisibleSurface");
      ALOGD_IF(TRACE,
               "DirectDisplaySurface::DequeueBuffersLocked: Discarding "
               "buffer_id=%d on invisible surface.",
               buffer_consumer->id());
      buffer_consumer->Discard();
      continue;
    }

    if (acquired_buffers_.IsFull()) {
      ALOGE(
          "DirectDisplaySurface::DequeueBuffersLocked: Posted buffers full, "
          "overwriting.");
      acquired_buffers_.PopBack();
    }

    acquired_buffers_.Append(
        AcquiredBuffer(buffer_consumer, std::move(acquire_fence), slot));
  }
}

AcquiredBuffer DirectDisplaySurface::AcquireCurrentBuffer() {
  std::lock_guard<std::mutex> autolock(lock_);
  DequeueBuffersLocked();

  if (acquired_buffers_.IsEmpty()) {
    ALOGE(
        "DirectDisplaySurface::AcquireCurrentBuffer: attempt to acquire buffer "
        "when none are posted.");
    return AcquiredBuffer();
  }
  AcquiredBuffer buffer = std::move(acquired_buffers_.Front());
  acquired_buffers_.PopFront();
  ALOGD_IF(TRACE, "DirectDisplaySurface::AcquireCurrentBuffer: buffer_id=%d",
           buffer.buffer()->id());
  return buffer;
}

AcquiredBuffer DirectDisplaySurface::AcquireNewestAvailableBuffer(
    AcquiredBuffer* skipped_buffer) {
  std::lock_guard<std::mutex> autolock(lock_);
  DequeueBuffersLocked();

  AcquiredBuffer buffer;
  int frames = 0;
  // Basic latency stopgap for when the application misses a frame:
  // If the application recovers on the 2nd or 3rd (etc) frame after
  // missing, this code will skip frames to catch up by checking if
  // the next frame is also available.
  while (!acquired_buffers_.IsEmpty() &&
         acquired_buffers_.Front().IsAvailable()) {
    // Capture the skipped buffer into the result parameter.
    // Note that this API only supports skipping one buffer per vsync.
    if (frames > 0 && skipped_buffer)
      *skipped_buffer = std::move(buffer);
    ++frames;
    buffer = std::move(acquired_buffers_.Front());
    acquired_buffers_.PopFront();
    if (frames == 2)
      break;
  }
  ALOGD_IF(TRACE,
           "DirectDisplaySurface::AcquireNewestAvailableBuffer: buffer_id=%d",
           buffer.buffer()->id());
  return buffer;
}

bool DirectDisplaySurface::IsBufferAvailable() {
  std::lock_guard<std::mutex> autolock(lock_);
  DequeueBuffersLocked();

  return !acquired_buffers_.IsEmpty() &&
         acquired_buffers_.Front().IsAvailable();
}

bool DirectDisplaySurface::IsBufferPosted() {
  std::lock_guard<std::mutex> autolock(lock_);
  DequeueBuffersLocked();

  return !acquired_buffers_.IsEmpty();
}

Status<std::shared_ptr<DisplaySurface>> DisplaySurface::Create(
    DisplayService* service, int surface_id, int process_id, int user_id,
    const display::SurfaceAttributes& attributes) {
  bool direct = false;
  auto search = attributes.find(display::SurfaceAttribute::Direct);
  if (search != attributes.end()) {
    if (!IfAnyOf<int32_t, int64_t, bool, float>::Get(&search->second,
                                                     &direct)) {
      ALOGE(
          "DisplaySurface::Create: Invalid type for SurfaceAttribute::Direct!");
      return ErrorStatus(EINVAL);
    }
  }

  ALOGD_IF(TRACE,
           "DisplaySurface::Create: surface_id=%d process_id=%d user_id=%d "
           "direct=%d",
           surface_id, process_id, user_id, direct);

  if (direct) {
    const bool trusted = user_id == AID_ROOT || IsTrustedUid(user_id);
    if (trusted) {
      return {std::shared_ptr<DisplaySurface>{
          new DirectDisplaySurface(service, surface_id, process_id, user_id)}};
    } else {
      ALOGE(
          "DisplaySurface::Create: Direct surfaces may only be created by "
          "trusted UIDs: user_id=%d",
          user_id);
      return ErrorStatus(EPERM);
    }
  } else {
    return {std::shared_ptr<DisplaySurface>{new ApplicationDisplaySurface(
        service, surface_id, process_id, user_id)}};
  }
}

}  // namespace dvr
}  // namespace android
