#include "acquired_buffer.h"

#include <log/log.h>
#include <sync/sync.h>

using android::pdx::LocalHandle;

namespace android {
namespace dvr {

AcquiredBuffer::AcquiredBuffer(const std::shared_ptr<ConsumerBuffer>& buffer,
                               LocalHandle acquire_fence, std::size_t slot)
    : buffer_(buffer), acquire_fence_(std::move(acquire_fence)), slot_(slot) {}

AcquiredBuffer::AcquiredBuffer(const std::shared_ptr<ConsumerBuffer>& buffer,
                               int* error) {
  LocalHandle fence;
  const int ret = buffer->Acquire(&fence);

  if (error)
    *error = ret;

  if (ret < 0) {
    ALOGW("AcquiredBuffer::AcquiredBuffer: Failed to acquire buffer: %s",
          strerror(-ret));
    buffer_ = nullptr;
    // Default construct sets acquire_fence_ to empty.
  } else {
    buffer_ = buffer;
    acquire_fence_ = std::move(fence);
  }
}

AcquiredBuffer::AcquiredBuffer(AcquiredBuffer&& other) noexcept {
  *this = std::move(other);
}

AcquiredBuffer::~AcquiredBuffer() { Release(LocalHandle(kEmptyFence)); }

AcquiredBuffer& AcquiredBuffer::operator=(AcquiredBuffer&& other) noexcept {
  if (this != &other) {
    Release();

    using std::swap;
    swap(buffer_, other.buffer_);
    swap(acquire_fence_, other.acquire_fence_);
    swap(slot_, other.slot_);
  }
  return *this;
}

bool AcquiredBuffer::IsAvailable() const {
  if (IsEmpty())
    return false;

  // Only check the fence if the acquire fence is not empty.
  if (acquire_fence_) {
    const int ret = sync_wait(acquire_fence_.Get(), 0);
    ALOGD_IF(TRACE || (ret < 0 && errno != ETIME),
             "AcquiredBuffer::IsAvailable: buffer_id=%d acquire_fence=%d "
             "sync_wait()=%d errno=%d.",
             buffer_->id(), acquire_fence_.Get(), ret, ret < 0 ? errno : 0);
    if (ret == 0) {
      // The fence is completed, so to avoid further calls to sync_wait we close
      // it here.
      acquire_fence_.Close();
    }
    return ret == 0;
  } else {
    return true;
  }
}

LocalHandle AcquiredBuffer::ClaimAcquireFence() {
  return std::move(acquire_fence_);
}

std::shared_ptr<ConsumerBuffer> AcquiredBuffer::ClaimBuffer() {
  return std::move(buffer_);
}

int AcquiredBuffer::Release(LocalHandle release_fence) {
  ALOGD_IF(TRACE, "AcquiredBuffer::Release: buffer_id=%d release_fence=%d",
           buffer_ ? buffer_->id() : -1, release_fence.Get());
  if (buffer_) {
    const int ret = buffer_->ReleaseAsync();
    if (ret < 0) {
      ALOGE("AcquiredBuffer::Release: Failed to release buffer %d: %s",
            buffer_->id(), strerror(-ret));
      if (ret != -ESHUTDOWN)
        return ret;
    }

    buffer_ = nullptr;
  }

  acquire_fence_.Close();
  slot_ = 0;
  return 0;
}

}  // namespace dvr
}  // namespace android
