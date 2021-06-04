#ifndef ANDROID_DVR_SERVICES_DISPLAYD_ACQUIRED_BUFFER_H_
#define ANDROID_DVR_SERVICES_DISPLAYD_ACQUIRED_BUFFER_H_

#include <pdx/file_handle.h>
#include <private/dvr/consumer_buffer.h>

#include <memory>

namespace android {
namespace dvr {

// Manages the ACQUIRE/RELEASE ownership cycle of a ConsumerBuffer.
class AcquiredBuffer {
 public:
  static constexpr int kEmptyFence = pdx::LocalHandle::kEmptyFileHandle;

  AcquiredBuffer() : buffer_(nullptr), acquire_fence_(kEmptyFence) {}

  // Constructs an AcquiredBuffer from a ConsumerBuffer pointer and an acquire
  // fence. The ConsumerBuffer MUST be in the ACQUIRED state prior to calling
  // this constructor; the constructor does not attempt to ACQUIRE the buffer
  // itself.
  AcquiredBuffer(const std::shared_ptr<ConsumerBuffer>& buffer,
                 pdx::LocalHandle acquire_fence, std::size_t slot = 0);

  // Constructs an AcquiredBuffer from a ConsumerBuffer. The ConsumerBuffer MUST
  // be in the POSTED state prior to calling this constructor, as this
  // constructor attempts to ACQUIRE the buffer. If ACQUIRING the buffer fails
  // this instance is left in the empty state. An optional error code is
  // returned in |error|, which may be nullptr if not needed.
  AcquiredBuffer(const std::shared_ptr<ConsumerBuffer>& buffer, int* error);

  // Move constructor. Behaves similarly to the move assignment operator below.
  AcquiredBuffer(AcquiredBuffer&& other) noexcept;

  ~AcquiredBuffer();

  // Move assignment operator. Moves the ConsumerBuffer and acquire fence from
  // |other| into this instance after RELEASING the current ConsumerBuffer and
  // closing the acquire fence. After the move |other| is left in the empty
  // state.
  AcquiredBuffer& operator=(AcquiredBuffer&& other) noexcept;

  // Accessors for the underlying ConsumerBuffer, the acquire fence, and the
  // use-case specific sequence value from the acquisition (see
  // private/dvr/consumer_buffer.h).
  std::shared_ptr<ConsumerBuffer> buffer() const { return buffer_; }
  int acquire_fence() const { return acquire_fence_.Get(); }

  // When non-empty, returns true if the acquired fence was signaled (or if the
  // fence is empty). Returns false when empty or if the fence is not signaled.
  bool IsAvailable() const;

  bool IsEmpty() const { return buffer_ == nullptr; }

  // Returns the acquire fence, passing ownership to the caller.
  pdx::LocalHandle ClaimAcquireFence();

  // Returns the buffer, passing ownership to the caller. Caller is responsible
  // for calling Release on the returned buffer.
  std::shared_ptr<ConsumerBuffer> ClaimBuffer();

  // Releases the ConsumerBuffer, passing the release fence in |release_fence|
  // to the producer. On success, the ConsumerBuffer and acquire fence are set
  // to empty state; if release fails, the ConsumerBuffer and acquire fence are
  // left in place and a negative error code is returned.
  int Release(pdx::LocalHandle release_fence = {});

  // Returns the slot in the queue this buffer belongs to. Buffers that are not
  // part of a queue return 0.
  std::size_t slot() const { return slot_; }

 private:
  std::shared_ptr<ConsumerBuffer> buffer_;
  // Mutable so that the fence can be closed when it is determined to be
  // signaled during IsAvailable().
  mutable pdx::LocalHandle acquire_fence_;
  std::size_t slot_{0};

  AcquiredBuffer(const AcquiredBuffer&) = delete;
  void operator=(const AcquiredBuffer&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_SERVICES_DISPLAYD_ACQUIRED_BUFFER_H_
