#include <private/dvr/consumer_buffer.h>

using android::pdx::LocalChannelHandle;
using android::pdx::LocalHandle;
using android::pdx::Status;

namespace android {
namespace dvr {

ConsumerBuffer::ConsumerBuffer(LocalChannelHandle channel)
    : BASE(std::move(channel)) {
  const int ret = ImportBuffer();
  if (ret < 0) {
    ALOGE("ConsumerBuffer::ConsumerBuffer: Failed to import buffer: %s",
          strerror(-ret));
    Close(ret);
  }
}

std::unique_ptr<ConsumerBuffer> ConsumerBuffer::Import(
    LocalChannelHandle channel) {
  ATRACE_NAME("ConsumerBuffer::Import");
  ALOGD_IF(TRACE, "ConsumerBuffer::Import: channel=%d", channel.value());
  return ConsumerBuffer::Create(std::move(channel));
}

std::unique_ptr<ConsumerBuffer> ConsumerBuffer::Import(
    Status<LocalChannelHandle> status) {
  return Import(status ? status.take()
                       : LocalChannelHandle{nullptr, -status.error()});
}

int ConsumerBuffer::LocalAcquire(DvrNativeBufferMetadata* out_meta,
                                 LocalHandle* out_fence) {
  if (!out_meta)
    return -EINVAL;

  // The buffer can be acquired iff the buffer state for this client is posted.
  uint32_t current_buffer_state =
      buffer_state_->load(std::memory_order_acquire);
  if (!BufferHubDefs::isClientPosted(current_buffer_state,
                                     client_state_mask())) {
    ALOGE(
        "%s: Failed to acquire the buffer. The buffer is not posted, id=%d "
        "state=%" PRIx32 " client_state_mask=%" PRIx32 ".",
        __FUNCTION__, id(), current_buffer_state, client_state_mask());
    return -EBUSY;
  }

  // Change the buffer state for this consumer from posted to acquired.
  uint32_t updated_buffer_state = current_buffer_state ^ client_state_mask();
  while (!buffer_state_->compare_exchange_weak(
      current_buffer_state, updated_buffer_state, std::memory_order_acq_rel,
      std::memory_order_acquire)) {
    ALOGD(
        "%s Failed to acquire the buffer. Current buffer state was changed to "
        "%" PRIx32
        " when trying to acquire the buffer and modify the buffer state to "
        "%" PRIx32 ". About to try again if the buffer is still posted.",
        __FUNCTION__, current_buffer_state, updated_buffer_state);
    if (!BufferHubDefs::isClientPosted(current_buffer_state,
                                       client_state_mask())) {
      ALOGE(
          "%s: Failed to acquire the buffer. The buffer is no longer posted, "
          "id=%d state=%" PRIx32 " client_state_mask=%" PRIx32 ".",
          __FUNCTION__, id(), current_buffer_state, client_state_mask());
      return -EBUSY;
    }
    // The failure of compare_exchange_weak updates current_buffer_state.
    updated_buffer_state = current_buffer_state ^ client_state_mask();
  }

  // Copy the canonical metadata.
  void* metadata_ptr = reinterpret_cast<void*>(&metadata_header_->metadata);
  memcpy(out_meta, metadata_ptr, sizeof(DvrNativeBufferMetadata));
  // Fill in the user_metadata_ptr in address space of the local process.
  if (out_meta->user_metadata_size) {
    out_meta->user_metadata_ptr =
        reinterpret_cast<uint64_t>(user_metadata_ptr_);
  } else {
    out_meta->user_metadata_ptr = 0;
  }

  uint32_t fence_state = fence_state_->load(std::memory_order_acquire);
  // If there is an acquire fence from producer, we need to return it.
  // The producer state bit mask is kFirstClientBitMask for now.
  if (fence_state & BufferHubDefs::kFirstClientBitMask) {
    *out_fence = shared_acquire_fence_.Duplicate();
  }

  return 0;
}

int ConsumerBuffer::Acquire(LocalHandle* ready_fence) {
  return Acquire(ready_fence, nullptr, 0);
}

int ConsumerBuffer::Acquire(LocalHandle* ready_fence, void* meta,
                            size_t user_metadata_size) {
  ATRACE_NAME("ConsumerBuffer::Acquire");

  if (const int error = CheckMetadata(user_metadata_size))
    return error;

  DvrNativeBufferMetadata canonical_meta;
  if (const int error = LocalAcquire(&canonical_meta, ready_fence))
    return error;

  if (meta && user_metadata_size) {
    void* metadata_src =
        reinterpret_cast<void*>(canonical_meta.user_metadata_ptr);
    if (metadata_src) {
      memcpy(meta, metadata_src, user_metadata_size);
    } else {
      ALOGW("ConsumerBuffer::Acquire: no user-defined metadata.");
    }
  }

  auto status = InvokeRemoteMethod<BufferHubRPC::ConsumerAcquire>();
  if (!status)
    return -status.error();
  return 0;
}

int ConsumerBuffer::AcquireAsync(DvrNativeBufferMetadata* out_meta,
                                 LocalHandle* out_fence) {
  ATRACE_NAME("ConsumerBuffer::AcquireAsync");

  if (const int error = LocalAcquire(out_meta, out_fence))
    return error;

  auto status = SendImpulse(BufferHubRPC::ConsumerAcquire::Opcode);
  if (!status)
    return -status.error();
  return 0;
}

int ConsumerBuffer::LocalRelease(const DvrNativeBufferMetadata* meta,
                                 const LocalHandle& release_fence) {
  if (const int error = CheckMetadata(meta->user_metadata_size))
    return error;

  // Set the buffer state of this client to released if it is not already in
  // released state.
  uint32_t current_buffer_state =
      buffer_state_->load(std::memory_order_acquire);
  if (BufferHubDefs::isClientReleased(current_buffer_state,
                                      client_state_mask())) {
    return 0;
  }
  uint32_t updated_buffer_state = current_buffer_state & (~client_state_mask());
  while (!buffer_state_->compare_exchange_weak(
      current_buffer_state, updated_buffer_state, std::memory_order_acq_rel,
      std::memory_order_acquire)) {
    ALOGD(
        "%s: Failed to release the buffer. Current buffer state was changed to "
        "%" PRIx32
        " when trying to release the buffer and modify the buffer state to "
        "%" PRIx32 ". About to try again.",
        __FUNCTION__, current_buffer_state, updated_buffer_state);
    // The failure of compare_exchange_weak updates current_buffer_state.
    updated_buffer_state = current_buffer_state & (~client_state_mask());
  }

  // On release, only the user requested metadata is copied back into the shared
  // memory for metadata. Since there are multiple consumers, it doesn't make
  // sense to send the canonical metadata back to the producer. However, one of
  // the consumer can still choose to write up to user_metadata_size bytes of
  // data into user_metadata_ptr.
  if (meta->user_metadata_ptr && meta->user_metadata_size) {
    void* metadata_src = reinterpret_cast<void*>(meta->user_metadata_ptr);
    memcpy(user_metadata_ptr_, metadata_src, meta->user_metadata_size);
  }

  // Send out the release fence through the shared epoll fd. Note that during
  // releasing the producer is not expected to be polling on the fence.
  if (const int error = UpdateSharedFence(release_fence, shared_release_fence_))
    return error;

  return 0;
}

int ConsumerBuffer::Release(const LocalHandle& release_fence) {
  ATRACE_NAME("ConsumerBuffer::Release");

  DvrNativeBufferMetadata meta;
  if (const int error = LocalRelease(&meta, release_fence))
    return error;

  return ReturnStatusOrError(InvokeRemoteMethod<BufferHubRPC::ConsumerRelease>(
      BorrowedFence(release_fence.Borrow())));
}

int ConsumerBuffer::ReleaseAsync() {
  DvrNativeBufferMetadata meta;
  return ReleaseAsync(&meta, LocalHandle());
}

int ConsumerBuffer::ReleaseAsync(const DvrNativeBufferMetadata* meta,
                                 const LocalHandle& release_fence) {
  ATRACE_NAME("ConsumerBuffer::ReleaseAsync");

  if (const int error = LocalRelease(meta, release_fence))
    return error;

  return ReturnStatusOrError(
      SendImpulse(BufferHubRPC::ConsumerRelease::Opcode));
}

int ConsumerBuffer::Discard() { return Release(LocalHandle()); }

}  // namespace dvr
}  // namespace android
