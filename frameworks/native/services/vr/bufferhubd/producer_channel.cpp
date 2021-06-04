#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/poll.h>

#include <algorithm>
#include <atomic>
#include <thread>

#include <log/log.h>
#include <private/dvr/bufferhub_rpc.h>
#include <private/dvr/consumer_channel.h>
#include <private/dvr/producer_channel.h>
#include <sync/sync.h>
#include <utils/Trace.h>

using android::pdx::BorrowedHandle;
using android::pdx::ErrorStatus;
using android::pdx::Message;
using android::pdx::RemoteChannelHandle;
using android::pdx::Status;
using android::pdx::rpc::BufferWrapper;
using android::pdx::rpc::DispatchRemoteMethod;
using android::pdx::rpc::WrapBuffer;

namespace android {
namespace dvr {

ProducerChannel::ProducerChannel(BufferHubService* service, int buffer_id,
                                 int channel_id, IonBuffer buffer,
                                 IonBuffer metadata_buffer,
                                 size_t user_metadata_size, int* error)
    : BufferHubChannel(service, buffer_id, channel_id, kProducerType),
      buffer_(std::move(buffer)),
      metadata_buffer_(std::move(metadata_buffer)),
      user_metadata_size_(user_metadata_size),
      metadata_buf_size_(BufferHubDefs::kMetadataHeaderSize +
                         user_metadata_size) {
  if (!buffer_.IsValid()) {
    ALOGE("ProducerChannel::ProducerChannel: Invalid buffer.");
    *error = -EINVAL;
    return;
  }
  if (!metadata_buffer_.IsValid()) {
    ALOGE("ProducerChannel::ProducerChannel: Invalid metadata buffer.");
    *error = -EINVAL;
    return;
  }

  *error = InitializeBuffer();
}

ProducerChannel::ProducerChannel(BufferHubService* service, int channel_id,
                                 uint32_t width, uint32_t height,
                                 uint32_t layer_count, uint32_t format,
                                 uint64_t usage, size_t user_metadata_size,
                                 int* error)
    : BufferHubChannel(service, channel_id, channel_id, kProducerType),
      user_metadata_size_(user_metadata_size),
      metadata_buf_size_(BufferHubDefs::kMetadataHeaderSize +
                         user_metadata_size) {
  if (int ret = buffer_.Alloc(width, height, layer_count, format, usage)) {
    ALOGE("ProducerChannel::ProducerChannel: Failed to allocate buffer: %s",
          strerror(-ret));
    *error = ret;
    return;
  }

  if (int ret = metadata_buffer_.Alloc(metadata_buf_size_, /*height=*/1,
                                       /*layer_count=*/1,
                                       BufferHubDefs::kMetadataFormat,
                                       BufferHubDefs::kMetadataUsage)) {
    ALOGE("ProducerChannel::ProducerChannel: Failed to allocate metadata: %s",
          strerror(-ret));
    *error = ret;
    return;
  }

  *error = InitializeBuffer();
}

int ProducerChannel::InitializeBuffer() {
  void* metadata_ptr = nullptr;
  if (int ret = metadata_buffer_.Lock(BufferHubDefs::kMetadataUsage, /*x=*/0,
                                      /*y=*/0, metadata_buf_size_,
                                      /*height=*/1, &metadata_ptr)) {
    ALOGE("ProducerChannel::ProducerChannel: Failed to lock metadata.");
    return ret;
  }
  metadata_header_ =
      reinterpret_cast<BufferHubDefs::MetadataHeader*>(metadata_ptr);

  // Using placement new here to reuse shared memory instead of new allocation
  // and also initialize the value to zero.
  buffer_state_ = new (&metadata_header_->bufferState) std::atomic<uint32_t>(0);
  fence_state_ = new (&metadata_header_->fenceState) std::atomic<uint32_t>(0);
  active_clients_bit_mask_ =
      new (&metadata_header_->activeClientsBitMask) std::atomic<uint32_t>(0);

  // Producer channel is never created after consumer channel, and one buffer
  // only have one fixed producer for now. Thus, it is correct to assume
  // producer state bit is kFirstClientBitMask for now.
  active_clients_bit_mask_->store(BufferHubDefs::kFirstClientBitMask,
                                  std::memory_order_release);

  acquire_fence_fd_.Reset(epoll_create1(EPOLL_CLOEXEC));
  release_fence_fd_.Reset(epoll_create1(EPOLL_CLOEXEC));
  if (!acquire_fence_fd_ || !release_fence_fd_) {
    ALOGE("ProducerChannel::ProducerChannel: Failed to create shared fences.");
    return -EIO;
  }

  dummy_fence_fd_.Reset(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK));
  if (!dummy_fence_fd_) {
    ALOGE("ProducerChannel::ProducerChannel: Failed to create dummy fences.");
    return EIO;
  }

  epoll_event event;
  event.events = 0;
  event.data.u32 = 0U;
  if (epoll_ctl(release_fence_fd_.Get(), EPOLL_CTL_ADD, dummy_fence_fd_.Get(),
                &event) < 0) {
    ALOGE(
        "ProducerChannel::ProducerChannel: Failed to modify the shared "
        "release fence to include the dummy fence: %s",
        strerror(errno));
    return -EIO;
  }

  // Success.
  return 0;
}

std::unique_ptr<ProducerChannel> ProducerChannel::Create(
    BufferHubService* service, int buffer_id, int channel_id, IonBuffer buffer,
    IonBuffer metadata_buffer, size_t user_metadata_size) {
  int error = 0;
  std::unique_ptr<ProducerChannel> producer(new ProducerChannel(
      service, buffer_id, channel_id, std::move(buffer),
      std::move(metadata_buffer), user_metadata_size, &error));

  if (error < 0)
    return nullptr;
  else
    return producer;
}

Status<std::shared_ptr<ProducerChannel>> ProducerChannel::Create(
    BufferHubService* service, int channel_id, uint32_t width, uint32_t height,
    uint32_t layer_count, uint32_t format, uint64_t usage,
    size_t user_metadata_size) {
  int error;
  std::shared_ptr<ProducerChannel> producer(
      new ProducerChannel(service, channel_id, width, height, layer_count,
                          format, usage, user_metadata_size, &error));
  if (error < 0)
    return ErrorStatus(-error);
  else
    return {std::move(producer)};
}

ProducerChannel::~ProducerChannel() {
  ALOGD_IF(TRACE,
           "ProducerChannel::~ProducerChannel: channel_id=%d buffer_id=%d "
           "state=%" PRIx32 ".",
           channel_id(), buffer_id(),
           buffer_state_->load(std::memory_order_acquire));
  for (auto consumer : consumer_channels_) {
    consumer->OnProducerClosed();
  }
  Hangup();
}

BufferHubChannel::BufferInfo ProducerChannel::GetBufferInfo() const {
  // Derive the mask of signaled buffers in this producer / consumer set.
  uint32_t signaled_mask = signaled() ? BufferHubDefs::kFirstClientBitMask : 0;
  for (const ConsumerChannel* consumer : consumer_channels_) {
    signaled_mask |= consumer->signaled() ? consumer->client_state_mask() : 0;
  }

  return BufferInfo(buffer_id(), consumer_channels_.size(), buffer_.width(),
                    buffer_.height(), buffer_.layer_count(), buffer_.format(),
                    buffer_.usage(),
                    buffer_state_->load(std::memory_order_acquire),
                    signaled_mask, metadata_header_->queueIndex);
}

void ProducerChannel::HandleImpulse(Message& message) {
  ATRACE_NAME("ProducerChannel::HandleImpulse");
  switch (message.GetOp()) {
    case BufferHubRPC::ProducerGain::Opcode:
      OnProducerGain(message);
      break;
    case BufferHubRPC::ProducerPost::Opcode:
      OnProducerPost(message, {});
      break;
  }
}

bool ProducerChannel::HandleMessage(Message& message) {
  ATRACE_NAME("ProducerChannel::HandleMessage");
  switch (message.GetOp()) {
    case BufferHubRPC::GetBuffer::Opcode:
      DispatchRemoteMethod<BufferHubRPC::GetBuffer>(
          *this, &ProducerChannel::OnGetBuffer, message);
      return true;

    case BufferHubRPC::NewConsumer::Opcode:
      DispatchRemoteMethod<BufferHubRPC::NewConsumer>(
          *this, &ProducerChannel::OnNewConsumer, message);
      return true;

    case BufferHubRPC::ProducerPost::Opcode:
      DispatchRemoteMethod<BufferHubRPC::ProducerPost>(
          *this, &ProducerChannel::OnProducerPost, message);
      return true;

    case BufferHubRPC::ProducerGain::Opcode:
      DispatchRemoteMethod<BufferHubRPC::ProducerGain>(
          *this, &ProducerChannel::OnProducerGain, message);
      return true;

    default:
      return false;
  }
}

BufferDescription<BorrowedHandle> ProducerChannel::GetBuffer(
    uint32_t client_state_mask) {
  return {buffer_,
          metadata_buffer_,
          buffer_id(),
          channel_id(),
          client_state_mask,
          acquire_fence_fd_.Borrow(),
          release_fence_fd_.Borrow()};
}

Status<BufferDescription<BorrowedHandle>> ProducerChannel::OnGetBuffer(
    Message& /*message*/) {
  ATRACE_NAME("ProducerChannel::OnGetBuffer");
  ALOGD_IF(TRACE, "ProducerChannel::OnGetBuffer: buffer=%d, state=%" PRIx32 ".",
           buffer_id(), buffer_state_->load(std::memory_order_acquire));
  return {GetBuffer(BufferHubDefs::kFirstClientBitMask)};
}

Status<uint32_t> ProducerChannel::CreateConsumerStateMask() {
  // Try find the next consumer state bit which has not been claimed by any
  // consumer yet.
  // memory_order_acquire is chosen here because all writes in other threads
  // that release active_clients_bit_mask_ need to be visible here.
  uint32_t current_active_clients_bit_mask =
      active_clients_bit_mask_->load(std::memory_order_acquire);
  uint32_t consumer_state_mask =
      BufferHubDefs::findNextAvailableClientStateMask(
          current_active_clients_bit_mask | orphaned_consumer_bit_mask_);
  if (consumer_state_mask == 0U) {
    ALOGE("%s: reached the maximum mumber of consumers per producer: 63.",
          __FUNCTION__);
    return ErrorStatus(E2BIG);
  }
  uint32_t updated_active_clients_bit_mask =
      current_active_clients_bit_mask | consumer_state_mask;
  // Set the updated value only if the current value stays the same as what was
  // read before. If the comparison succeeds, update the value without
  // reordering anything before or after this read-modify-write in the current
  // thread, and the modification will be visible in other threads that acquire
  // active_clients_bit_mask_. If the comparison fails, load the result of
  // all writes from all threads to updated_active_clients_bit_mask.
  // Keep on finding the next available slient state mask until succeed or out
  // of memory.
  while (!active_clients_bit_mask_->compare_exchange_weak(
      current_active_clients_bit_mask, updated_active_clients_bit_mask,
      std::memory_order_acq_rel, std::memory_order_acquire)) {
    ALOGE("%s: Current active clients bit mask is changed to %" PRIx32
          ", which was expected to be %" PRIx32
          ". Trying to generate a new client state mask to resolve race "
          "condition.",
          __FUNCTION__, updated_active_clients_bit_mask,
          current_active_clients_bit_mask);
    consumer_state_mask = BufferHubDefs::findNextAvailableClientStateMask(
        current_active_clients_bit_mask | orphaned_consumer_bit_mask_);
    if (consumer_state_mask == 0U) {
      ALOGE("%s: reached the maximum mumber of consumers per producer: %d.",
            __FUNCTION__, (BufferHubDefs::kMaxNumberOfClients - 1));
      return ErrorStatus(E2BIG);
    }
    updated_active_clients_bit_mask =
        current_active_clients_bit_mask | consumer_state_mask;
  }

  return {consumer_state_mask};
}

void ProducerChannel::RemoveConsumerClientMask(uint32_t consumer_state_mask) {
  // Clear up the buffer state and fence state in case there is already
  // something there due to possible race condition between producer post and
  // consumer failed to create channel.
  buffer_state_->fetch_and(~consumer_state_mask, std::memory_order_release);
  fence_state_->fetch_and(~consumer_state_mask, std::memory_order_release);

  // Restore the consumer state bit and make it visible in other threads that
  // acquire the active_clients_bit_mask_.
  active_clients_bit_mask_->fetch_and(~consumer_state_mask,
                                      std::memory_order_release);
}

Status<RemoteChannelHandle> ProducerChannel::CreateConsumer(
    Message& message, uint32_t consumer_state_mask) {
  ATRACE_NAME(__FUNCTION__);
  ALOGD("%s: buffer_id=%d", __FUNCTION__, buffer_id());

  int channel_id;
  auto status = message.PushChannel(0, nullptr, &channel_id);
  if (!status) {
    ALOGE("%s: Failed to push consumer channel: %s", __FUNCTION__,
          status.GetErrorMessage().c_str());
    RemoveConsumerClientMask(consumer_state_mask);
    return ErrorStatus(ENOMEM);
  }

  auto consumer = std::make_shared<ConsumerChannel>(
      service(), buffer_id(), channel_id, consumer_state_mask,
      shared_from_this());
  const auto channel_status = service()->SetChannel(channel_id, consumer);
  if (!channel_status) {
    ALOGE("%s: failed to set new consumer channel: %s.", __FUNCTION__,
          channel_status.GetErrorMessage().c_str());
    RemoveConsumerClientMask(consumer_state_mask);
    return ErrorStatus(ENOMEM);
  }

  uint32_t current_buffer_state =
      buffer_state_->load(std::memory_order_acquire);
  // Return the consumer channel handle without signal when adding the new
  // consumer to a buffer that is available to producer (a.k.a a fully-released
  // buffer) or a gained buffer.
  if (current_buffer_state == 0U ||
      BufferHubDefs::isAnyClientGained(current_buffer_state)) {
    return {status.take()};
  }

  // Signal the new consumer when adding it to a posted producer.
  bool update_buffer_state = true;
  if (!BufferHubDefs::isClientPosted(current_buffer_state,
                                     consumer_state_mask)) {
    uint32_t updated_buffer_state =
        current_buffer_state ^
        (consumer_state_mask & BufferHubDefs::kHighBitsMask);
    while (!buffer_state_->compare_exchange_weak(
        current_buffer_state, updated_buffer_state, std::memory_order_acq_rel,
        std::memory_order_acquire)) {
      ALOGI(
          "%s: Failed to post to the new consumer. "
          "Current buffer state was changed to %" PRIx32
          " when trying to acquire the buffer and modify the buffer state to "
          "%" PRIx32
          ". About to try again if the buffer is still not gained nor fully "
          "released.",
          __FUNCTION__, current_buffer_state, updated_buffer_state);
      if (current_buffer_state == 0U ||
          BufferHubDefs::isAnyClientGained(current_buffer_state)) {
        ALOGI("%s: buffer is gained or fully released, state=%" PRIx32 ".",
              __FUNCTION__, current_buffer_state);
        update_buffer_state = false;
        break;
      }
      updated_buffer_state =
          current_buffer_state ^
          (consumer_state_mask & BufferHubDefs::kHighBitsMask);
    }
  }
  if (update_buffer_state || BufferHubDefs::isClientPosted(
                                 buffer_state_->load(std::memory_order_acquire),
                                 consumer_state_mask)) {
    consumer->OnProducerPosted();
  }

  return {status.take()};
}

Status<RemoteChannelHandle> ProducerChannel::OnNewConsumer(Message& message) {
  ATRACE_NAME("ProducerChannel::OnNewConsumer");
  ALOGD_IF(TRACE, "ProducerChannel::OnNewConsumer: buffer_id=%d", buffer_id());
  auto status = CreateConsumerStateMask();
  if (!status.ok()) {
    return status.error_status();
  }
  return CreateConsumer(message, /*consumer_state_mask=*/status.get());
}

Status<void> ProducerChannel::OnProducerPost(Message&,
                                             LocalFence acquire_fence) {
  ATRACE_NAME("ProducerChannel::OnProducerPost");
  ALOGD_IF(TRACE, "%s: buffer_id=%d, state=0x%x", __FUNCTION__, buffer_id(),
           buffer_state_->load(std::memory_order_acquire));

  epoll_event event;
  event.events = 0;
  event.data.u32 = 0U;
  int ret = epoll_ctl(release_fence_fd_.Get(), EPOLL_CTL_MOD,
                      dummy_fence_fd_.Get(), &event);
  ALOGE_IF(ret < 0,
           "ProducerChannel::OnProducerPost: Failed to modify the shared "
           "release fence to include the dummy fence: %s",
           strerror(errno));

  eventfd_t dummy_fence_count = 0U;
  if (eventfd_read(dummy_fence_fd_.Get(), &dummy_fence_count) < 0) {
    const int error = errno;
    if (error != EAGAIN) {
      ALOGE(
          "ProducerChannel::ProducerChannel: Failed to read dummy fence, "
          "error: %s",
          strerror(error));
      return ErrorStatus(error);
    }
  }

  ALOGW_IF(dummy_fence_count > 0,
           "ProducerChannel::ProducerChannel: %" PRIu64
           " dummy fence(s) was signaled during last release/gain cycle "
           "buffer_id=%d.",
           dummy_fence_count, buffer_id());

  post_fence_ = std::move(acquire_fence);

  // Signal any interested consumers. If there are none, the buffer will stay
  // in posted state until a consumer comes online. This behavior guarantees
  // that no frame is silently dropped.
  for (auto& consumer : consumer_channels_) {
    consumer->OnProducerPosted();
  }

  return {};
}

Status<LocalFence> ProducerChannel::OnProducerGain(Message& /*message*/) {
  ATRACE_NAME("ProducerChannel::OnGain");
  ALOGD_IF(TRACE, "%s: buffer_id=%d", __FUNCTION__, buffer_id());

  ClearAvailable();
  post_fence_.close();
  for (auto& consumer : consumer_channels_) {
    consumer->OnProducerGained();
  }
  return {std::move(returned_fence_)};
}

// TODO(b/112338294) Keep here for reference. Remove it after new logic is
// written.
/* Status<RemoteChannelHandle> ProducerChannel::OnProducerDetach(
    Message& message) {
  ATRACE_NAME("ProducerChannel::OnProducerDetach");
  ALOGD_IF(TRACE, "ProducerChannel::OnProducerDetach: buffer_id=%d",
           buffer_id());

  uint32_t buffer_state = buffer_state_->load(std::memory_order_acquire);
  if (!BufferHubDefs::isClientGained(
      buffer_state, BufferHubDefs::kFirstClientStateMask)) {
    // Can only detach a ProducerBuffer when it's in gained state.
    ALOGW(
        "ProducerChannel::OnProducerDetach: The buffer (id=%d, state=%"
        PRIx32
        ") is not in gained state.",
        buffer_id(), buffer_state);
    return {};
  }

  int channel_id;
  auto status = message.PushChannel(0, nullptr, &channel_id);
  if (!status) {
    ALOGE(
        "ProducerChannel::OnProducerDetach: Failed to push detached buffer "
        "channel: %s",
        status.GetErrorMessage().c_str());
    return ErrorStatus(ENOMEM);
  }

  // Make sure we unlock the buffer.
  if (int ret = metadata_buffer_.Unlock()) {
    ALOGE("ProducerChannel::OnProducerDetach: Failed to unlock metadata.");
    return ErrorStatus(-ret);
  };

  std::unique_ptr<BufferChannel> channel =
      BufferChannel::Create(service(), buffer_id(), channel_id,
                            std::move(buffer_), user_metadata_size_);
  if (!channel) {
    ALOGE("ProducerChannel::OnProducerDetach: Invalid buffer.");
    return ErrorStatus(EINVAL);
  }

  const auto channel_status =
      service()->SetChannel(channel_id, std::move(channel));
  if (!channel_status) {
    // Technically, this should never fail, as we just pushed the channel.
    // Note that LOG_FATAL will be stripped out in non-debug build.
    LOG_FATAL(
        "ProducerChannel::OnProducerDetach: Failed to set new detached "
        "buffer channel: %s.", channel_status.GetErrorMessage().c_str());
  }

  return status;
} */

Status<LocalFence> ProducerChannel::OnConsumerAcquire(Message& /*message*/) {
  ATRACE_NAME("ProducerChannel::OnConsumerAcquire");
  ALOGD_IF(TRACE, "ProducerChannel::OnConsumerAcquire: buffer_id=%d",
           buffer_id());

  // Return a borrowed fd to avoid unnecessary duplication of the underlying
  // fd. Serialization just needs to read the handle.
  return {std::move(post_fence_)};
}

Status<void> ProducerChannel::OnConsumerRelease(Message&,
                                                LocalFence release_fence) {
  ATRACE_NAME("ProducerChannel::OnConsumerRelease");
  ALOGD_IF(TRACE, "ProducerChannel::OnConsumerRelease: buffer_id=%d",
           buffer_id());

  // Attempt to merge the fences if necessary.
  if (release_fence) {
    if (returned_fence_) {
      LocalFence merged_fence(sync_merge("bufferhub_merged",
                                         returned_fence_.get_fd(),
                                         release_fence.get_fd()));
      const int error = errno;
      if (!merged_fence) {
        ALOGE("ProducerChannel::OnConsumerRelease: Failed to merge fences: %s",
              strerror(error));
        return ErrorStatus(error);
      }
      returned_fence_ = std::move(merged_fence);
    } else {
      returned_fence_ = std::move(release_fence);
    }
  }

  if (IsBufferReleasedByAllActiveClientsExceptForOrphans()) {
    buffer_state_->store(0U);
    SignalAvailable();
    if (orphaned_consumer_bit_mask_) {
      ALOGW(
          "%s: orphaned buffer detected during the this acquire/release cycle: "
          "id=%d orphaned=0x%" PRIx32 " queue_index=%" PRId64 ".",
          __FUNCTION__, buffer_id(), orphaned_consumer_bit_mask_,
          metadata_header_->queueIndex);
      orphaned_consumer_bit_mask_ = 0;
    }
  }

  return {};
}

void ProducerChannel::OnConsumerOrphaned(const uint32_t& consumer_state_mask) {
  // Remember the ignored consumer so that newly added consumer won't be
  // taking the same state mask as this orphaned consumer.
  ALOGE_IF(orphaned_consumer_bit_mask_ & consumer_state_mask,
           "%s: Consumer (consumer_state_mask=%" PRIx32
           ") is already orphaned.",
           __FUNCTION__, consumer_state_mask);
  orphaned_consumer_bit_mask_ |= consumer_state_mask;

  if (IsBufferReleasedByAllActiveClientsExceptForOrphans()) {
    buffer_state_->store(0U);
    SignalAvailable();
  }

  // Atomically clear the fence state bit as an orphaned consumer will never
  // signal a release fence.
  fence_state_->fetch_and(~consumer_state_mask, std::memory_order_release);

  // Atomically set the buffer state of this consumer to released state.
  buffer_state_->fetch_and(~consumer_state_mask, std::memory_order_release);

  ALOGW(
      "%s: detected new orphaned consumer buffer_id=%d "
      "consumer_state_mask=%" PRIx32 " queue_index=%" PRId64
      " buffer_state=%" PRIx32 " fence_state=%" PRIx32 ".",
      __FUNCTION__, buffer_id(), consumer_state_mask,
      metadata_header_->queueIndex,
      buffer_state_->load(std::memory_order_acquire),
      fence_state_->load(std::memory_order_acquire));
}

void ProducerChannel::AddConsumer(ConsumerChannel* channel) {
  consumer_channels_.push_back(channel);
}

void ProducerChannel::RemoveConsumer(ConsumerChannel* channel) {
  consumer_channels_.erase(
      std::find(consumer_channels_.begin(), consumer_channels_.end(), channel));
  // Restore the consumer state bit and make it visible in other threads that
  // acquire the active_clients_bit_mask_.
  uint32_t consumer_state_mask = channel->client_state_mask();
  uint32_t current_active_clients_bit_mask =
      active_clients_bit_mask_->load(std::memory_order_acquire);
  uint32_t updated_active_clients_bit_mask =
      current_active_clients_bit_mask & (~consumer_state_mask);
  while (!active_clients_bit_mask_->compare_exchange_weak(
      current_active_clients_bit_mask, updated_active_clients_bit_mask,
      std::memory_order_acq_rel, std::memory_order_acquire)) {
    ALOGI(
        "%s: Failed to remove consumer state mask. Current active clients bit "
        "mask is changed to %" PRIx32
        " when trying to acquire and modify it to %" PRIx32
        ". About to try again.",
        __FUNCTION__, current_active_clients_bit_mask,
        updated_active_clients_bit_mask);
    updated_active_clients_bit_mask =
        current_active_clients_bit_mask & (~consumer_state_mask);
  }

  const uint32_t current_buffer_state =
      buffer_state_->load(std::memory_order_acquire);
  if (BufferHubDefs::isClientPosted(current_buffer_state,
                                    consumer_state_mask) ||
      BufferHubDefs::isClientAcquired(current_buffer_state,
                                      consumer_state_mask)) {
    // The consumer client is being destoryed without releasing. This could
    // happen in corner cases when the consumer crashes. Here we mark it
    // orphaned before remove it from producer.
    OnConsumerOrphaned(consumer_state_mask);
    return;
  }

  if (BufferHubDefs::isClientReleased(current_buffer_state,
                                      consumer_state_mask) ||
      BufferHubDefs::isAnyClientGained(current_buffer_state)) {
    // The consumer is being close while it is suppose to signal a release
    // fence. Signal the dummy fence here.
    if (fence_state_->load(std::memory_order_acquire) & consumer_state_mask) {
      epoll_event event;
      event.events = EPOLLIN;
      event.data.u32 = consumer_state_mask;
      if (epoll_ctl(release_fence_fd_.Get(), EPOLL_CTL_MOD,
                    dummy_fence_fd_.Get(), &event) < 0) {
        ALOGE(
            "%s: Failed to modify the shared release fence to include the "
            "dummy fence: %s",
            __FUNCTION__, strerror(errno));
        return;
      }
      ALOGW("%s: signal dummy release fence buffer_id=%d", __FUNCTION__,
            buffer_id());
      eventfd_write(dummy_fence_fd_.Get(), 1);
    }
  }
}

// Returns true if the given parameters match the underlying buffer
// parameters.
bool ProducerChannel::CheckParameters(uint32_t width, uint32_t height,
                                      uint32_t layer_count, uint32_t format,
                                      uint64_t usage,
                                      size_t user_metadata_size) const {
  return user_metadata_size == user_metadata_size_ &&
         buffer_.width() == width && buffer_.height() == height &&
         buffer_.layer_count() == layer_count && buffer_.format() == format &&
         buffer_.usage() == usage;
}

bool ProducerChannel::IsBufferReleasedByAllActiveClientsExceptForOrphans()
    const {
  return (buffer_state_->load(std::memory_order_acquire) &
          ~orphaned_consumer_bit_mask_ &
          active_clients_bit_mask_->load(std::memory_order_acquire)) == 0U;
}

}  // namespace dvr
}  // namespace android
