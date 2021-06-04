#ifndef ANDROID_DVR_BUFFER_HUB_BASE_H_
#define ANDROID_DVR_BUFFER_HUB_BASE_H_

#include <vector>

#include <private/dvr/bufferhub_rpc.h>

namespace android {
namespace dvr {

// Base class of two types of BufferHub clients: dvr::ProducerBuffer and
// dvr::ConsumerBuffer.
class BufferHubBase : public pdx::Client {
 public:
  using LocalHandle = pdx::LocalHandle;
  using LocalChannelHandle = pdx::LocalChannelHandle;
  template <typename T>
  using Status = pdx::Status<T>;

  // Create a new consumer channel that is attached to the producer. Returns
  // a file descriptor for the new channel or a negative error code.
  Status<LocalChannelHandle> CreateConsumer();

  // Gets a blob buffer that was created with ProducerBuffer::CreateBlob.
  // Locking and Unlocking is handled internally. There's no need to Unlock
  // after calling this method.
  int GetBlobReadWritePointer(size_t size, void** addr);

  // Returns a dup'd file descriptor for accessing the blob shared memory. The
  // caller takes ownership of the file descriptor and must close it or pass on
  // ownership. Some GPU API extensions can take file descriptors to bind shared
  // memory gralloc buffers to GPU buffer objects.
  LocalHandle GetBlobFd() const {
    // Current GPU vendor puts the buffer allocation in one FD. If we change GPU
    // vendors and this is the wrong fd, late-latching and EDS will very clearly
    // stop working and we will need to correct this. The alternative is to use
    // a GL context in the pose service to allocate this buffer or to use the
    // ION API directly instead of gralloc.
    return LocalHandle(dup(native_handle()->data[0]));
  }

  using Client::event_fd;

  Status<int> GetEventMask(int events) {
    if (auto* client_channel = GetChannel()) {
      return client_channel->GetEventMask(events);
    } else {
      return pdx::ErrorStatus(EINVAL);
    }
  }

  std::vector<pdx::ClientChannel::EventSource> GetEventSources() const {
    if (auto* client_channel = GetChannel()) {
      return client_channel->GetEventSources();
    } else {
      return {};
    }
  }

  native_handle_t* native_handle() const {
    return const_cast<native_handle_t*>(buffer_.handle());
  }

  IonBuffer* buffer() { return &buffer_; }
  const IonBuffer* buffer() const { return &buffer_; }

  // Gets ID of the buffer client. All BufferHub clients derived from the same
  // buffer in bufferhubd share the same buffer id.
  int id() const { return id_; }

  // Gets the channel id of the buffer client. Each BufferHub client has its
  // system unique channel id.
  int cid() const { return cid_; }

  // Returns the buffer buffer state.
  uint32_t buffer_state() {
    return buffer_state_->load(std::memory_order_acquire);
  };

  // Returns whether the buffer is already released by all current clients.
  bool is_released() {
    return (buffer_state() &
            active_clients_bit_mask_->load(std::memory_order_acquire)) == 0;
  }

  // A state mask which is unique to a buffer hub client among all its siblings
  // sharing the same concrete graphic buffer.
  uint32_t client_state_mask() const { return client_state_mask_; }

  // The following methods return settings of the first buffer. Currently,
  // it is only possible to create multi-buffer BufferHubBases with the same
  // settings.
  uint32_t width() const { return buffer_.width(); }
  uint32_t height() const { return buffer_.height(); }
  uint32_t stride() const { return buffer_.stride(); }
  uint32_t format() const { return buffer_.format(); }
  uint32_t usage() const { return buffer_.usage(); }
  uint32_t layer_count() const { return buffer_.layer_count(); }

  uint64_t GetQueueIndex() const { return metadata_header_->queueIndex; }
  void SetQueueIndex(uint64_t index) { metadata_header_->queueIndex = index; }

 protected:
  explicit BufferHubBase(LocalChannelHandle channel);
  explicit BufferHubBase(const std::string& endpoint_path);
  virtual ~BufferHubBase();

  // Initialization helper.
  int ImportBuffer();

  // Check invalid metadata operation. Returns 0 if requested metadata is valid.
  int CheckMetadata(size_t user_metadata_size) const;

  // Send out the new fence by updating the shared fence (shared_release_fence
  // for producer and shared_acquire_fence for consumer). Note that during this
  // should only be used in LocalPost() or LocalRelease, and the shared fence
  // shouldn't be poll'ed by the other end.
  int UpdateSharedFence(const LocalHandle& new_fence,
                        const LocalHandle& shared_fence);

  // Locks the area specified by (x, y, width, height) for a specific usage. If
  // the usage is software then |addr| will be updated to point to the address
  // of the buffer in virtual memory. The caller should only access/modify the
  // pixels in the specified area. anything else is undefined behavior.
  int Lock(int usage, int x, int y, int width, int height, void** addr);

  // Must be called after Lock() when the caller has finished changing the
  // buffer.
  int Unlock();

  // IonBuffer that is shared between bufferhubd, producer, and consumers.
  size_t metadata_buf_size_{0};
  size_t user_metadata_size_{0};
  BufferHubDefs::MetadataHeader* metadata_header_ = nullptr;
  void* user_metadata_ptr_ = nullptr;
  std::atomic<uint32_t>* buffer_state_ = nullptr;
  std::atomic<uint32_t>* fence_state_ = nullptr;
  std::atomic<uint32_t>* active_clients_bit_mask_ = nullptr;

  LocalHandle shared_acquire_fence_;
  LocalHandle shared_release_fence_;

  // A local fence fd that holds the ownership of the fence fd on Post (for
  // producer) and Release (for consumer).
  LocalHandle pending_fence_fd_;

 private:
  BufferHubBase(const BufferHubBase&) = delete;
  void operator=(const BufferHubBase&) = delete;

  // Global id for the buffer that is consistent across processes. It is meant
  // for logging and debugging purposes only and should not be used for lookup
  // or any other functional purpose as a security precaution.
  int id_;

  // Channel id.
  int cid_;

  // Client bit mask which indicates the locations of this client object in the
  // buffer_state_.
  uint32_t client_state_mask_{0U};
  IonBuffer buffer_;
  IonBuffer metadata_buffer_;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_BUFFER_HUB_BASE_H_
