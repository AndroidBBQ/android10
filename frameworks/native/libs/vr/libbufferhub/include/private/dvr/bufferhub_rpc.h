#ifndef ANDROID_DVR_BUFFERHUB_RPC_H_
#define ANDROID_DVR_BUFFERHUB_RPC_H_

#include "buffer_hub_defs.h"

#include <cutils/native_handle.h>
#include <ui/BufferQueueDefs.h>

#include <pdx/channel_handle.h>
#include <pdx/file_handle.h>
#include <pdx/rpc/remote_method.h>
#include <pdx/rpc/serializable.h>
#include <private/dvr/ion_buffer.h>

namespace android {
namespace dvr {

template <typename FileHandleType>
class NativeBufferHandle {
 public:
  NativeBufferHandle() { Clear(); }
  NativeBufferHandle(const IonBuffer& buffer, int id)
      : id_(id),
        stride_(buffer.stride()),
        width_(buffer.width()),
        height_(buffer.height()),
        layer_count_(buffer.layer_count()),
        format_(buffer.format()),
        usage_(buffer.usage()) {
    // Populate the fd and int vectors: native_handle->data[] is an array of fds
    // followed by an array of opaque ints.
    const int fd_count = buffer.handle()->numFds;
    const int int_count = buffer.handle()->numInts;
    for (int i = 0; i < fd_count; i++) {
      fds_.emplace_back(FileHandleType::AsDuplicate(buffer.handle()->data[i]));
    }
    for (int i = 0; i < int_count; i++) {
      opaque_ints_.push_back(buffer.handle()->data[fd_count + i]);
    }
  }
  NativeBufferHandle(NativeBufferHandle&& other) noexcept = default;
  NativeBufferHandle& operator=(NativeBufferHandle&& other) noexcept = default;

  // Imports the native handle into the given IonBuffer instance.
  int Import(IonBuffer* buffer) {
    // This is annoying, but we need to convert the vector of FileHandles into a
    // vector of ints for the Import API.
    std::vector<int> fd_ints;
    for (const auto& fd : fds_)
      fd_ints.push_back(fd.Get());

    const int ret =
        buffer->Import(fd_ints.data(), fd_ints.size(), opaque_ints_.data(),
                       opaque_ints_.size(), width_, height_, layer_count_,
                       stride_, format_, usage_);
    if (ret < 0)
      return ret;

    // Import succeeded, release the file handles which are now owned by the
    // IonBuffer and clear members.
    for (auto& fd : fds_)
      fd.Release();
    opaque_ints_.clear();
    Clear();

    return 0;
  }

  int id() const { return id_; }
  size_t IntCount() const { return opaque_ints_.size(); }
  size_t FdCount() const { return fds_.size(); }

 private:
  int id_;
  uint32_t stride_;
  uint32_t width_;
  uint32_t height_;
  uint32_t layer_count_;
  uint32_t format_;
  uint64_t usage_;
  std::vector<int> opaque_ints_;
  std::vector<FileHandleType> fds_;

  void Clear() {
    id_ = -1;
    stride_ = width_ = height_ = format_ = usage_ = 0;
  }

  PDX_SERIALIZABLE_MEMBERS(NativeBufferHandle<FileHandleType>, id_, stride_,
                           width_, height_, layer_count_, format_, usage_,
                           opaque_ints_, fds_);

  NativeBufferHandle(const NativeBufferHandle&) = delete;
  void operator=(const NativeBufferHandle&) = delete;
};

template <typename FileHandleType>
class BufferDescription {
 public:
  BufferDescription() = default;
  BufferDescription(const IonBuffer& buffer, const IonBuffer& metadata, int id,
                    int buffer_cid, uint32_t client_state_mask,
                    const FileHandleType& acquire_fence_fd,
                    const FileHandleType& release_fence_fd)
      : id_(id),
        buffer_cid_(buffer_cid),
        client_state_mask_(client_state_mask),
        buffer_(buffer, id),
        metadata_(metadata, id),
        acquire_fence_fd_(acquire_fence_fd.Borrow()),
        release_fence_fd_(release_fence_fd.Borrow()) {}

  BufferDescription(BufferDescription&& other) noexcept = default;
  BufferDescription& operator=(BufferDescription&& other) noexcept = default;

  // ID of the buffer client. All BufferHub clients derived from the same buffer
  // in bufferhubd share the same buffer id.
  int id() const { return id_; }

  // Channel ID of the buffer client. Each BufferHub client has its system
  // unique channel id.
  int buffer_cid() const { return buffer_cid_; }

  // State mask of the buffer client. Each BufferHub client backed by the
  // same buffer channel has uniqued state bit among its siblings.
  uint32_t client_state_mask() const { return client_state_mask_; }
  FileHandleType take_acquire_fence() { return std::move(acquire_fence_fd_); }
  FileHandleType take_release_fence() { return std::move(release_fence_fd_); }

  int ImportBuffer(IonBuffer* buffer) { return buffer_.Import(buffer); }
  int ImportMetadata(IonBuffer* metadata) { return metadata_.Import(metadata); }

 private:
  int id_{-1};
  int buffer_cid_{-1};
  uint32_t client_state_mask_{0U};
  // Two IonBuffers: one for the graphic buffer and one for metadata.
  NativeBufferHandle<FileHandleType> buffer_;
  NativeBufferHandle<FileHandleType> metadata_;

  // Pamameters for shared fences.
  FileHandleType acquire_fence_fd_;
  FileHandleType release_fence_fd_;

  PDX_SERIALIZABLE_MEMBERS(BufferDescription<FileHandleType>, id_, buffer_cid_,
                           client_state_mask_, buffer_, metadata_,
                           acquire_fence_fd_, release_fence_fd_);

  BufferDescription(const BufferDescription&) = delete;
  void operator=(const BufferDescription&) = delete;
};

using BorrowedNativeBufferHandle = NativeBufferHandle<pdx::BorrowedHandle>;
using LocalNativeBufferHandle = NativeBufferHandle<pdx::LocalHandle>;

template <typename FileHandleType>
class FenceHandle {
 public:
  FenceHandle() = default;
  explicit FenceHandle(int fence) : fence_{fence} {}
  explicit FenceHandle(FileHandleType&& fence) : fence_{std::move(fence)} {}
  FenceHandle(FenceHandle&&) noexcept = default;
  FenceHandle& operator=(FenceHandle&&) noexcept = default;

  explicit operator bool() const { return fence_.IsValid(); }

  const FileHandleType& get() const { fence_; }
  FileHandleType&& take() { return std::move(fence_); }

  int get_fd() const { return fence_.Get(); }
  void close() { fence_.Close(); }

  FenceHandle<pdx::BorrowedHandle> borrow() const {
    return FenceHandle<pdx::BorrowedHandle>(fence_.Borrow());
  }

 private:
  FileHandleType fence_;

  PDX_SERIALIZABLE_MEMBERS(FenceHandle<FileHandleType>, fence_);

  FenceHandle(const FenceHandle&) = delete;
  void operator=(const FenceHandle&) = delete;
};

using LocalFence = FenceHandle<pdx::LocalHandle>;
using BorrowedFence = FenceHandle<pdx::BorrowedHandle>;

struct ProducerQueueConfig {
  // Whether the buffer queue is operating in Async mode.
  // From GVR's perspective of view, this means a buffer can be acquired
  // asynchronously by the compositor.
  // From Android Surface's perspective of view, this is equivalent to
  // IGraphicBufferProducer's async mode. When in async mode, a producer
  // will never block even if consumer is running slow.
  bool is_async;

  // Default buffer width that is set during ProducerQueue's creation.
  uint32_t default_width;

  // Default buffer height that is set during ProducerQueue's creation.
  uint32_t default_height;

  // Default buffer format that is set during ProducerQueue's creation.
  uint32_t default_format;

  // Size of the meta data associated with all the buffers allocated from the
  // queue.
  size_t user_metadata_size;

 private:
  PDX_SERIALIZABLE_MEMBERS(ProducerQueueConfig, is_async, default_width,
                           default_height, default_format, user_metadata_size);
};

class ProducerQueueConfigBuilder {
 public:
  // Build a ProducerQueueConfig object.
  ProducerQueueConfig Build() {
    return {is_async_, default_width_, default_height_, default_format_,
            user_metadata_size_};
  }

  ProducerQueueConfigBuilder& SetIsAsync(bool is_async) {
    is_async_ = is_async;
    return *this;
  }

  ProducerQueueConfigBuilder& SetDefaultWidth(uint32_t width) {
    default_width_ = width;
    return *this;
  }

  ProducerQueueConfigBuilder& SetDefaultHeight(uint32_t height) {
    default_height_ = height;
    return *this;
  }

  ProducerQueueConfigBuilder& SetDefaultFormat(uint32_t format) {
    default_format_ = format;
    return *this;
  }

  template <typename Meta>
  ProducerQueueConfigBuilder& SetMetadata() {
    user_metadata_size_ = sizeof(Meta);
    return *this;
  }

  ProducerQueueConfigBuilder& SetMetadataSize(size_t user_metadata_size) {
    user_metadata_size_ = user_metadata_size;
    return *this;
  }

 private:
  bool is_async_{false};
  uint32_t default_width_{1};
  uint32_t default_height_{1};
  uint32_t default_format_{1};  // PIXEL_FORMAT_RGBA_8888
  size_t user_metadata_size_{0};
};

// Explicit specializations of ProducerQueueConfigBuilder::Build for void
// metadata type.
template <>
inline ProducerQueueConfigBuilder&
ProducerQueueConfigBuilder::SetMetadata<void>() {
  user_metadata_size_ = 0;
  return *this;
}

struct QueueInfo {
  ProducerQueueConfig producer_config;
  int id;

 private:
  PDX_SERIALIZABLE_MEMBERS(QueueInfo, producer_config, id);
};

struct UsagePolicy {
  uint64_t usage_set_mask{0};
  uint64_t usage_clear_mask{0};
  uint64_t usage_deny_set_mask{0};
  uint64_t usage_deny_clear_mask{0};

 private:
  PDX_SERIALIZABLE_MEMBERS(UsagePolicy, usage_set_mask, usage_clear_mask,
                           usage_deny_set_mask, usage_deny_clear_mask);
};

// BufferHub Service RPC interface. Defines the endpoints, op codes, and method
// type signatures supported by bufferhubd.
struct BufferHubRPC {
  // Service path.
  static constexpr char kClientPath[] = "system/buffer_hub/client";

  // |BufferHubQueue| will keep track of at most this value of buffers.
  // Attempts at runtime to increase the number of buffers past this
  // will fail. Note that the value is in sync with |android::BufferQueue|, so
  // that slot id can be shared between |android::dvr::BufferHubQueueProducer|
  // and |android::BufferQueueProducer| which both implements the same
  // interface: |android::IGraphicBufferProducer|.
  static constexpr size_t kMaxQueueCapacity =
      android::BufferQueueDefs::NUM_BUFFER_SLOTS;

  // Op codes.
  enum {
    kOpCreateBuffer = 0,
    kOpGetBuffer,
    kOpNewConsumer,
    kOpProducerPost,
    kOpProducerGain,
    kOpConsumerAcquire,
    kOpConsumerRelease,
    kOpConsumerBufferDetach,
    kOpCreateProducerQueue,
    kOpCreateConsumerQueue,
    kOpGetQueueInfo,
    kOpProducerQueueAllocateBuffers,
    kOpProducerQueueInsertBuffer,
    kOpProducerQueueRemoveBuffer,
    kOpConsumerQueueImportBuffers,
    // TODO(b/77153033): Separate all those RPC operations into subclasses.
  };

  // Aliases.
  using LocalChannelHandle = pdx::LocalChannelHandle;
  using LocalHandle = pdx::LocalHandle;
  using Void = pdx::rpc::Void;

  // Methods.
  PDX_REMOTE_METHOD(CreateBuffer, kOpCreateBuffer,
                    void(uint32_t width, uint32_t height, uint32_t format,
                         uint64_t usage, size_t user_metadata_size));
  PDX_REMOTE_METHOD(GetBuffer, kOpGetBuffer,
                    BufferDescription<LocalHandle>(Void));
  PDX_REMOTE_METHOD(NewConsumer, kOpNewConsumer, LocalChannelHandle(Void));
  PDX_REMOTE_METHOD(ProducerPost, kOpProducerPost,
                    void(LocalFence acquire_fence));
  PDX_REMOTE_METHOD(ProducerGain, kOpProducerGain, LocalFence(Void));
  PDX_REMOTE_METHOD(ConsumerAcquire, kOpConsumerAcquire, LocalFence(Void));
  PDX_REMOTE_METHOD(ConsumerRelease, kOpConsumerRelease,
                    void(LocalFence release_fence));

  // Detaches a ConsumerBuffer from an existing producer/consumer set. Can only
  // be called when the consumer is the only consumer and it has exclusive
  // access to the buffer (i.e. in the acquired'ed state). On the successful
  // return of the IPC call, a new DetachedBufferChannel handle will be returned
  // and all existing producer and consumer channels will be closed. Further
  // IPCs towards those channels will return error.
  PDX_REMOTE_METHOD(ConsumerBufferDetach, kOpConsumerBufferDetach,
                    LocalChannelHandle(Void));

  // Buffer Queue Methods.
  PDX_REMOTE_METHOD(CreateProducerQueue, kOpCreateProducerQueue,
                    QueueInfo(const ProducerQueueConfig& producer_config,
                              const UsagePolicy& usage_policy));
  PDX_REMOTE_METHOD(CreateConsumerQueue, kOpCreateConsumerQueue,
                    LocalChannelHandle(bool silent_queue));
  PDX_REMOTE_METHOD(GetQueueInfo, kOpGetQueueInfo, QueueInfo(Void));
  PDX_REMOTE_METHOD(ProducerQueueAllocateBuffers,
                    kOpProducerQueueAllocateBuffers,
                    std::vector<std::pair<LocalChannelHandle, size_t>>(
                        uint32_t width, uint32_t height, uint32_t layer_count,
                        uint32_t format, uint64_t usage, size_t buffer_count));
  PDX_REMOTE_METHOD(ProducerQueueInsertBuffer, kOpProducerQueueInsertBuffer,
                    size_t(int buffer_cid));
  PDX_REMOTE_METHOD(ProducerQueueRemoveBuffer, kOpProducerQueueRemoveBuffer,
                    void(size_t slot));
  PDX_REMOTE_METHOD(ConsumerQueueImportBuffers, kOpConsumerQueueImportBuffers,
                    std::vector<std::pair<LocalChannelHandle, size_t>>(Void));
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_BUFFERHUB_RPC_H_
