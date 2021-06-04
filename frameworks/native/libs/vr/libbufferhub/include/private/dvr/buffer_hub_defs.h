#ifndef ANDROID_DVR_BUFFER_HUB_DEFS_H_
#define ANDROID_DVR_BUFFER_HUB_DEFS_H_

#include <dvr/dvr_api.h>
#include <hardware/gralloc.h>
#include <pdx/channel_handle.h>
#include <pdx/file_handle.h>
#include <pdx/rpc/remote_method.h>
#include <pdx/rpc/serializable.h>
#include <private/dvr/native_handle_wrapper.h>
#include <ui/BufferHubDefs.h>

namespace android {
namespace dvr {

namespace BufferHubDefs {

static constexpr uint32_t kMetadataFormat = HAL_PIXEL_FORMAT_BLOB;
static constexpr uint32_t kMetadataUsage =
    GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN;

// See more details in libs/ui/include/ui/BufferHubDefs.h
static constexpr int kMaxNumberOfClients =
    android::BufferHubDefs::kMaxNumberOfClients;
static constexpr uint32_t kLowbitsMask = android::BufferHubDefs::kLowbitsMask;
static constexpr uint32_t kHighBitsMask = android::BufferHubDefs::kHighBitsMask;
static constexpr uint32_t kFirstClientBitMask =
    android::BufferHubDefs::kFirstClientBitMask;

static inline bool isAnyClientGained(uint32_t state) {
  return android::BufferHubDefs::isAnyClientGained(state);
}

static inline bool isClientGained(uint32_t state, uint32_t client_bit_mask) {
  return android::BufferHubDefs::isClientGained(state, client_bit_mask);
}

static inline bool isAnyClientPosted(uint32_t state) {
  return android::BufferHubDefs::isAnyClientPosted(state);
}

static inline bool isClientPosted(uint32_t state, uint32_t client_bit_mask) {
  return android::BufferHubDefs::isClientPosted(state, client_bit_mask);
}

static inline bool isAnyClientAcquired(uint32_t state) {
  return android::BufferHubDefs::isAnyClientAcquired(state);
}

static inline bool isClientAcquired(uint32_t state, uint32_t client_bit_mask) {
  return android::BufferHubDefs::isClientAcquired(state, client_bit_mask);
}

static inline bool isClientReleased(uint32_t state, uint32_t client_bit_mask) {
  return android::BufferHubDefs::isClientReleased(state, client_bit_mask);
}

// Returns the next available buffer client's client_state_masks.
// @params union_bits. Union of all existing clients' client_state_masks.
static inline uint32_t findNextAvailableClientStateMask(uint32_t union_bits) {
  return android::BufferHubDefs::findNextAvailableClientStateMask(union_bits);
}

using MetadataHeader = android::BufferHubDefs::MetadataHeader;
static constexpr size_t kMetadataHeaderSize =
    android::BufferHubDefs::kMetadataHeaderSize;

}  // namespace BufferHubDefs

template <typename FileHandleType>
class BufferTraits {
 public:
  BufferTraits() = default;
  BufferTraits(const native_handle_t* buffer_handle,
               const FileHandleType& metadata_handle, int id,
               uint32_t client_state_mask, uint64_t metadata_size,
               uint32_t width, uint32_t height, uint32_t layer_count,
               uint32_t format, uint64_t usage, uint32_t stride,
               const FileHandleType& acquire_fence_fd,
               const FileHandleType& release_fence_fd)
      : id_(id),
        client_state_mask_(client_state_mask),
        metadata_size_(metadata_size),
        width_(width),
        height_(height),
        layer_count_(layer_count),
        format_(format),
        usage_(usage),
        stride_(stride),
        buffer_handle_(buffer_handle),
        metadata_handle_(metadata_handle.Borrow()),
        acquire_fence_fd_(acquire_fence_fd.Borrow()),
        release_fence_fd_(release_fence_fd.Borrow()) {}

  BufferTraits(BufferTraits&& other) = default;
  BufferTraits& operator=(BufferTraits&& other) = default;

  // ID of the buffer client. All BufferHubBuffer clients derived from the same
  // buffer in bufferhubd share the same buffer id.
  int id() const { return id_; }

  // State mask of the buffer client. Each BufferHubBuffer client backed by the
  // same buffer channel has uniqued state bit among its siblings. For a
  // producer buffer the bit must be kFirstClientBitMask; for a consumer the bit
  // must be one of the kConsumerStateMask.
  uint32_t client_state_mask() const { return client_state_mask_; }
  uint64_t metadata_size() const { return metadata_size_; }

  uint32_t width() { return width_; }
  uint32_t height() { return height_; }
  uint32_t layer_count() { return layer_count_; }
  uint32_t format() { return format_; }
  uint64_t usage() { return usage_; }
  uint32_t stride() { return stride_; }

  const NativeHandleWrapper<FileHandleType>& buffer_handle() const {
    return buffer_handle_;
  }

  NativeHandleWrapper<FileHandleType> take_buffer_handle() {
    return std::move(buffer_handle_);
  }
  FileHandleType take_metadata_handle() { return std::move(metadata_handle_); }
  FileHandleType take_acquire_fence() { return std::move(acquire_fence_fd_); }
  FileHandleType take_release_fence() { return std::move(release_fence_fd_); }

 private:
  // BufferHub specific traits.
  int id_ = -1;
  uint32_t client_state_mask_;
  uint64_t metadata_size_;

  // Traits for a GraphicBuffer.
  uint32_t width_;
  uint32_t height_;
  uint32_t layer_count_;
  uint32_t format_;
  uint64_t usage_;
  uint32_t stride_;

  // Native handle for the graphic buffer.
  NativeHandleWrapper<FileHandleType> buffer_handle_;

  // File handle of an ashmem that holds buffer metadata.
  FileHandleType metadata_handle_;

  // Pamameters for shared fences.
  FileHandleType acquire_fence_fd_;
  FileHandleType release_fence_fd_;

  PDX_SERIALIZABLE_MEMBERS(BufferTraits<FileHandleType>, id_,
                           client_state_mask_, metadata_size_, stride_, width_,
                           height_, layer_count_, format_, usage_,
                           buffer_handle_, metadata_handle_, acquire_fence_fd_,
                           release_fence_fd_);

  BufferTraits(const BufferTraits&) = delete;
  void operator=(const BufferTraits&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_BUFFER_HUB_DEFS_H_
