#include <private/dvr/producer_buffer.h>

using android::pdx::LocalChannelHandle;
using android::pdx::LocalHandle;
using android::pdx::Status;

namespace android {
namespace dvr {

ProducerBuffer::ProducerBuffer(uint32_t width, uint32_t height, uint32_t format,
                               uint64_t usage, size_t user_metadata_size)
    : BASE(BufferHubRPC::kClientPath) {
  ATRACE_NAME("ProducerBuffer::ProducerBuffer");
  ALOGD_IF(TRACE,
           "ProducerBuffer::ProducerBuffer: fd=%d width=%u height=%u format=%u "
           "usage=%" PRIx64 " user_metadata_size=%zu",
           event_fd(), width, height, format, usage, user_metadata_size);

  auto status = InvokeRemoteMethod<BufferHubRPC::CreateBuffer>(
      width, height, format, usage, user_metadata_size);
  if (!status) {
    ALOGE(
        "ProducerBuffer::ProducerBuffer: Failed to create producer buffer: %s",
        status.GetErrorMessage().c_str());
    Close(-status.error());
    return;
  }

  const int ret = ImportBuffer();
  if (ret < 0) {
    ALOGE(
        "ProducerBuffer::ProducerBuffer: Failed to import producer buffer: %s",
        strerror(-ret));
    Close(ret);
  }
}

ProducerBuffer::ProducerBuffer(uint64_t usage, size_t size)
    : BASE(BufferHubRPC::kClientPath) {
  ATRACE_NAME("ProducerBuffer::ProducerBuffer");
  ALOGD_IF(TRACE, "ProducerBuffer::ProducerBuffer: usage=%" PRIx64 " size=%zu",
           usage, size);
  const int width = static_cast<int>(size);
  const int height = 1;
  const int format = HAL_PIXEL_FORMAT_BLOB;
  const size_t user_metadata_size = 0;

  auto status = InvokeRemoteMethod<BufferHubRPC::CreateBuffer>(
      width, height, format, usage, user_metadata_size);
  if (!status) {
    ALOGE("ProducerBuffer::ProducerBuffer: Failed to create blob: %s",
          status.GetErrorMessage().c_str());
    Close(-status.error());
    return;
  }

  const int ret = ImportBuffer();
  if (ret < 0) {
    ALOGE(
        "ProducerBuffer::ProducerBuffer: Failed to import producer buffer: %s",
        strerror(-ret));
    Close(ret);
  }
}

ProducerBuffer::ProducerBuffer(LocalChannelHandle channel)
    : BASE(std::move(channel)) {
  const int ret = ImportBuffer();
  if (ret < 0) {
    ALOGE(
        "ProducerBuffer::ProducerBuffer: Failed to import producer buffer: %s",
        strerror(-ret));
    Close(ret);
  }
}

int ProducerBuffer::LocalPost(const DvrNativeBufferMetadata* meta,
                              const LocalHandle& ready_fence) {
  if (const int error = CheckMetadata(meta->user_metadata_size))
    return error;

  // The buffer can be posted iff the buffer state for this client is gained.
  uint32_t current_buffer_state =
      buffer_state_->load(std::memory_order_acquire);
  if (!BufferHubDefs::isClientGained(current_buffer_state,
                                     client_state_mask())) {
    ALOGE("%s: not gained, id=%d state=%" PRIx32 ".", __FUNCTION__, id(),
          current_buffer_state);
    return -EBUSY;
  }

  // Set the producer client buffer state to released, that of all other clients
  // (both existing and non-existing clients) to posted.
  uint32_t updated_buffer_state =
      (~client_state_mask()) & BufferHubDefs::kHighBitsMask;
  while (!buffer_state_->compare_exchange_weak(
      current_buffer_state, updated_buffer_state, std::memory_order_acq_rel,
      std::memory_order_acquire)) {
    ALOGD(
        "%s: Failed to post the buffer. Current buffer state was changed to "
        "%" PRIx32
        " when trying to post the buffer and modify the buffer state to "
        "%" PRIx32
        ". About to try again if the buffer is still gained by this client.",
        __FUNCTION__, current_buffer_state, updated_buffer_state);
    if (!BufferHubDefs::isClientGained(current_buffer_state,
                                       client_state_mask())) {
      ALOGE(
          "%s: Failed to post the buffer. The buffer is no longer gained, "
          "id=%d state=%" PRIx32 ".",
          __FUNCTION__, id(), current_buffer_state);
      return -EBUSY;
    }
  }

  // Copy the canonical metadata.
  void* metadata_ptr = reinterpret_cast<void*>(&metadata_header_->metadata);
  memcpy(metadata_ptr, meta, sizeof(DvrNativeBufferMetadata));
  // Copy extra user requested metadata.
  if (meta->user_metadata_ptr && meta->user_metadata_size) {
    void* metadata_src = reinterpret_cast<void*>(meta->user_metadata_ptr);
    memcpy(user_metadata_ptr_, metadata_src, meta->user_metadata_size);
  }

  // Send out the acquire fence through the shared epoll fd. Note that during
  // posting no consumer is not expected to be polling on the fence.
  if (const int error = UpdateSharedFence(ready_fence, shared_acquire_fence_))
    return error;

  return 0;
}

int ProducerBuffer::Post(const LocalHandle& ready_fence, const void* meta,
                         size_t user_metadata_size) {
  ATRACE_NAME("ProducerBuffer::Post");

  // Populate cononical metadata for posting.
  DvrNativeBufferMetadata canonical_meta;
  canonical_meta.user_metadata_ptr = reinterpret_cast<uint64_t>(meta);
  canonical_meta.user_metadata_size = user_metadata_size;

  if (const int error = LocalPost(&canonical_meta, ready_fence))
    return error;

  return ReturnStatusOrError(InvokeRemoteMethod<BufferHubRPC::ProducerPost>(
      BorrowedFence(ready_fence.Borrow())));
}

int ProducerBuffer::PostAsync(const DvrNativeBufferMetadata* meta,
                              const LocalHandle& ready_fence) {
  ATRACE_NAME("ProducerBuffer::PostAsync");

  if (const int error = LocalPost(meta, ready_fence))
    return error;

  return ReturnStatusOrError(SendImpulse(BufferHubRPC::ProducerPost::Opcode));
}

int ProducerBuffer::LocalGain(DvrNativeBufferMetadata* out_meta,
                              LocalHandle* out_fence, bool gain_posted_buffer) {
  if (!out_meta)
    return -EINVAL;

  uint32_t current_buffer_state =
      buffer_state_->load(std::memory_order_acquire);
  ALOGD_IF(TRACE, "%s: buffer=%d, state=%" PRIx32 ".", __FUNCTION__, id(),
           current_buffer_state);

  if (BufferHubDefs::isClientGained(current_buffer_state,
                                    client_state_mask())) {
    ALOGV("%s: already gained id=%d.", __FUNCTION__, id());
    return 0;
  }
  if (BufferHubDefs::isAnyClientAcquired(current_buffer_state) ||
      BufferHubDefs::isAnyClientGained(current_buffer_state) ||
      (BufferHubDefs::isAnyClientPosted(
           current_buffer_state &
           active_clients_bit_mask_->load(std::memory_order_acquire)) &&
       !gain_posted_buffer)) {
    ALOGE("%s: not released id=%d state=%" PRIx32 ".", __FUNCTION__, id(),
          current_buffer_state);
    return -EBUSY;
  }
  // Change the buffer state to gained state.
  uint32_t updated_buffer_state = client_state_mask();
  while (!buffer_state_->compare_exchange_weak(
      current_buffer_state, updated_buffer_state, std::memory_order_acq_rel,
      std::memory_order_acquire)) {
    ALOGD(
        "%s: Failed to gain the buffer. Current buffer state was changed to "
        "%" PRIx32
        " when trying to gain the buffer and modify the buffer state to "
        "%" PRIx32
        ". About to try again if the buffer is still not read by other "
        "clients.",
        __FUNCTION__, current_buffer_state, updated_buffer_state);

    if (BufferHubDefs::isAnyClientAcquired(current_buffer_state) ||
        BufferHubDefs::isAnyClientGained(current_buffer_state) ||
        (BufferHubDefs::isAnyClientPosted(
             current_buffer_state &
             active_clients_bit_mask_->load(std::memory_order_acquire)) &&
         !gain_posted_buffer)) {
      ALOGE(
          "%s: Failed to gain the buffer. The buffer is no longer released. "
          "id=%d state=%" PRIx32 ".",
          __FUNCTION__, id(), current_buffer_state);
      return -EBUSY;
    }
  }

  // Canonical metadata is undefined on Gain. Except for user_metadata and
  // release_fence_mask. Fill in the user_metadata_ptr in address space of the
  // local process.
  if (metadata_header_->metadata.user_metadata_size && user_metadata_ptr_) {
    out_meta->user_metadata_size =
        metadata_header_->metadata.user_metadata_size;
    out_meta->user_metadata_ptr =
        reinterpret_cast<uint64_t>(user_metadata_ptr_);
  } else {
    out_meta->user_metadata_size = 0;
    out_meta->user_metadata_ptr = 0;
  }

  uint32_t current_fence_state = fence_state_->load(std::memory_order_acquire);
  uint32_t current_active_clients_bit_mask =
      active_clients_bit_mask_->load(std::memory_order_acquire);
  // If there are release fence(s) from consumer(s), we need to return it to the
  // consumer(s).
  // TODO(b/112007999) add an atomic variable in metadata header in shared
  // memory to indicate which client is the last producer of the buffer.
  // Currently, assume the first client is the only producer to the buffer.
  if (current_fence_state & current_active_clients_bit_mask &
      (~BufferHubDefs::kFirstClientBitMask)) {
    *out_fence = shared_release_fence_.Duplicate();
    out_meta->release_fence_mask = current_fence_state &
                                   current_active_clients_bit_mask &
                                   (~BufferHubDefs::kFirstClientBitMask);
  }

  return 0;
}

int ProducerBuffer::Gain(LocalHandle* release_fence, bool gain_posted_buffer) {
  ATRACE_NAME("ProducerBuffer::Gain");

  DvrNativeBufferMetadata meta;
  if (const int error = LocalGain(&meta, release_fence, gain_posted_buffer))
    return error;

  auto status = InvokeRemoteMethod<BufferHubRPC::ProducerGain>();
  if (!status)
    return -status.error();
  return 0;
}

int ProducerBuffer::GainAsync(DvrNativeBufferMetadata* out_meta,
                              LocalHandle* release_fence,
                              bool gain_posted_buffer) {
  ATRACE_NAME("ProducerBuffer::GainAsync");

  if (const int error = LocalGain(out_meta, release_fence, gain_posted_buffer))
    return error;

  return ReturnStatusOrError(SendImpulse(BufferHubRPC::ProducerGain::Opcode));
}

int ProducerBuffer::GainAsync() {
  DvrNativeBufferMetadata meta;
  LocalHandle fence;
  return GainAsync(&meta, &fence);
}

std::unique_ptr<ProducerBuffer> ProducerBuffer::Import(
    LocalChannelHandle channel) {
  ALOGD_IF(TRACE, "ProducerBuffer::Import: channel=%d", channel.value());
  return ProducerBuffer::Create(std::move(channel));
}

std::unique_ptr<ProducerBuffer> ProducerBuffer::Import(
    Status<LocalChannelHandle> status) {
  return Import(status ? status.take()
                       : LocalChannelHandle{nullptr, -status.error()});
}

Status<LocalChannelHandle> ProducerBuffer::Detach() {
  // TODO(b/112338294) remove after migrate producer buffer to binder
  ALOGW("ProducerBuffer::Detach: not supported operation during migration");
  return {};

  // TODO(b/112338294) Keep here for reference. Remove it after new logic is
  // written.
  /* uint32_t buffer_state = buffer_state_->load(std::memory_order_acquire);
  if (!BufferHubDefs::isClientGained(
      buffer_state, BufferHubDefs::kFirstClientStateMask)) {
    // Can only detach a ProducerBuffer when it's in gained state.
    ALOGW("ProducerBuffer::Detach: The buffer (id=%d, state=0x%" PRIx32
          ") is not in gained state.",
          id(), buffer_state);
    return {};
  }

  Status<LocalChannelHandle> status =
      InvokeRemoteMethod<BufferHubRPC::ProducerBufferDetach>();
  ALOGE_IF(!status,
           "ProducerBuffer::Detach: Failed to detach buffer (id=%d): %s.", id(),
           status.GetErrorMessage().c_str());
  return status; */
}

}  // namespace dvr
}  // namespace android
