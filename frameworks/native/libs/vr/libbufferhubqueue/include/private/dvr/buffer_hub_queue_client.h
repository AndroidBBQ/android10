#ifndef ANDROID_DVR_BUFFER_HUB_QUEUE_CLIENT_H_
#define ANDROID_DVR_BUFFER_HUB_QUEUE_CLIENT_H_

#include <ui/BufferQueueDefs.h>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#endif

// The following headers are included without checking every warning.
// TODO(b/72172820): Remove the workaround once we have enforced -Weverything
// in these headers and their dependencies.
#include <pdx/client.h>
#include <pdx/status.h>
#include <private/dvr/buffer_hub_queue_parcelable.h>
#include <private/dvr/bufferhub_rpc.h>
#include <private/dvr/consumer_buffer.h>
#include <private/dvr/epoll_file_descriptor.h>
#include <private/dvr/producer_buffer.h>

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#include <memory>
#include <queue>
#include <vector>

namespace android {
namespace dvr {

class ConsumerQueue;

// |BufferHubQueue| manages a queue of |BufferHubBase|s. Buffers are
// automatically re-requeued when released by the remote side.
class BufferHubQueue : public pdx::Client {
 public:
  using BufferAvailableCallback = std::function<void()>;
  using BufferRemovedCallback =
      std::function<void(const std::shared_ptr<BufferHubBase>&)>;

  virtual ~BufferHubQueue() {}

  // Creates a new consumer queue that is attached to the producer. Returns
  // a new consumer queue client or nullptr on failure.
  std::unique_ptr<ConsumerQueue> CreateConsumerQueue();

  // Creates a new consumer queue that is attached to the producer. This queue
  // sets each of its imported consumer buffers to the ignored state to avoid
  // participation in lifecycle events.
  std::unique_ptr<ConsumerQueue> CreateSilentConsumerQueue();

  // Returns whether the buffer queue is in async mode.
  bool is_async() const { return is_async_; }

  // Returns the default buffer width of this buffer queue.
  uint32_t default_width() const { return default_width_; }

  // Returns the default buffer height of this buffer queue.
  uint32_t default_height() const { return default_height_; }

  // Returns the default buffer format of this buffer queue.
  uint32_t default_format() const { return default_format_; }

  // Creates a new consumer in handle form for immediate transport over RPC.
  pdx::Status<pdx::LocalChannelHandle> CreateConsumerQueueHandle(
      bool silent = false);

  // Creates a new consumer in parcelable form for immediate transport over
  // Binder.
  pdx::Status<ConsumerQueueParcelable> CreateConsumerQueueParcelable(
      bool silent = false);

  // Returns the number of buffers avaiable for dequeue.
  size_t count() const { return available_buffers_.size(); }

  // Returns the total number of buffers that the queue is tracking.
  size_t capacity() const { return capacity_; }

  // Returns the size of metadata structure associated with this queue.
  size_t metadata_size() const { return user_metadata_size_; }

  // Returns whether the buffer queue is full.
  bool is_full() const {
    return available_buffers_.size() >= kMaxQueueCapacity;
  }

  // Returns whether the buffer queue is connected to bufferhubd.
  bool is_connected() const { return !!GetChannel(); }

  int GetBufferId(size_t slot) const {
    return (slot < buffers_.size() && buffers_[slot]) ? buffers_[slot]->id()
                                                      : -1;
  }

  std::shared_ptr<BufferHubBase> GetBuffer(size_t slot) const {
    return buffers_[slot];
  }

  pdx::Status<int> GetEventMask(int events) {
    if (auto* client_channel = GetChannel()) {
      return client_channel->GetEventMask(events);
    } else {
      return pdx::ErrorStatus(EINVAL);
    }
  }

  // Returns an fd that signals pending queue events using
  // EPOLLIN/POLLIN/readible. Either HandleQueueEvents or WaitForBuffers may be
  // called to handle pending queue events.
  int queue_fd() const { return epoll_fd_.Get(); }

  // Handles any pending events, returning available buffers to the queue and
  // reaping disconnected buffers. Returns true if successful, false if an error
  // occurred.
  bool HandleQueueEvents() { return WaitForBuffers(0); }

  // Set buffer event callbacks, which are std::function wrappers. The caller is
  // responsible for ensuring the validity of these callbacks' callable targets.
  void SetBufferAvailableCallback(BufferAvailableCallback callback);
  void SetBufferRemovedCallback(BufferRemovedCallback callback);

  // The queue tracks at most this many buffers.
  static constexpr size_t kMaxQueueCapacity =
      android::BufferQueueDefs::NUM_BUFFER_SLOTS;

  static constexpr int kNoTimeOut = -1;

  int id() const { return id_; }
  bool hung_up() const { return hung_up_; }

 protected:
  explicit BufferHubQueue(pdx::LocalChannelHandle channel);
  explicit BufferHubQueue(const std::string& endpoint_path);

  // Imports the queue parameters by querying BufferHub for the parameters for
  // this channel.
  pdx::Status<void> ImportQueue();

  // Sets up the queue with the given parameters.
  void SetupQueue(const QueueInfo& queue_info);

  // Register a buffer for management by the queue. Used by subclasses to add a
  // buffer to internal bookkeeping.
  pdx::Status<void> AddBuffer(const std::shared_ptr<BufferHubBase>& buffer,
                              size_t slot);

  // Called by ProducerQueue::RemoveBuffer and ConsumerQueue::RemoveBuffer only
  // to deregister a buffer for epoll and internal bookkeeping.
  virtual pdx::Status<void> RemoveBuffer(size_t slot);

  // Free all buffers that belongs to this queue. Can only be called from
  // producer side.
  virtual pdx::Status<void> FreeAllBuffers();

  // Dequeue a buffer from the free queue, blocking until one is available. The
  // timeout argument specifies the number of milliseconds that |Dequeue()| will
  // block. Specifying a timeout of -1 causes Dequeue() to block indefinitely,
  // while specifying a timeout equal to zero cause Dequeue() to return
  // immediately, even if no buffers are available.
  pdx::Status<std::shared_ptr<BufferHubBase>> Dequeue(int timeout,
                                                      size_t* slot);

  // Waits for buffers to become available and adds them to the available queue.
  bool WaitForBuffers(int timeout);

  pdx::Status<void> HandleBufferEvent(size_t slot, int event_fd,
                                      int poll_events);
  pdx::Status<void> HandleQueueEvent(int poll_events);

  // Entry in the priority queue of available buffers that stores related
  // per-buffer data.
  struct Entry {
    Entry() : slot(0) {}
    Entry(const std::shared_ptr<BufferHubBase>& in_buffer, size_t in_slot,
          uint64_t in_index)
        : buffer(in_buffer), slot(in_slot), index(in_index) {}
    Entry(const std::shared_ptr<BufferHubBase>& in_buffer,
          std::unique_ptr<uint8_t[]> in_metadata, pdx::LocalHandle in_fence,
          size_t in_slot)
        : buffer(in_buffer),
          metadata(std::move(in_metadata)),
          fence(std::move(in_fence)),
          slot(in_slot) {}
    Entry(Entry&&) = default;
    Entry& operator=(Entry&&) = default;

    std::shared_ptr<BufferHubBase> buffer;
    std::unique_ptr<uint8_t[]> metadata;
    pdx::LocalHandle fence;
    size_t slot;
    uint64_t index;
  };

  struct EntryComparator {
    bool operator()(const Entry& lhs, const Entry& rhs) {
      return lhs.index > rhs.index;
    }
  };

  // Enqueues a buffer to the available list (Gained for producer or Acquireed
  // for consumer).
  pdx::Status<void> Enqueue(Entry entry);

  // Called when a buffer is allocated remotely.
  virtual pdx::Status<void> OnBufferAllocated() { return {}; }

  // Size of the metadata that buffers in this queue cary.
  size_t user_metadata_size_{0};

  // Buffers and related data that are available for dequeue.
  std::priority_queue<Entry, std::vector<Entry>, EntryComparator>
      available_buffers_;

  // Slot of the buffers that are not available for normal dequeue. For example,
  // the slot of posted or acquired buffers in the perspective of a producer.
  std::vector<size_t> unavailable_buffers_slot_;

 private:
  void Initialize();

  // Special epoll data field indicating that the epoll event refers to the
  // queue.
  static constexpr int64_t kEpollQueueEventIndex = -1;

  static constexpr size_t kMaxEvents = 128;

  // The u64 data field of an epoll event is interpreted as int64_t:
  // When |index| >= 0 and |index| < kMaxQueueCapacity it refers to a specific
  // element of |buffers_| as a direct index;
  static bool is_buffer_event_index(int64_t index) {
    return index >= 0 &&
           index < static_cast<int64_t>(BufferHubQueue::kMaxQueueCapacity);
  }

  // When |index| == kEpollQueueEventIndex it refers to the queue itself.
  static bool is_queue_event_index(int64_t index) {
    return index == BufferHubQueue::kEpollQueueEventIndex;
  }

  // Whether the buffer queue is operating in Async mode.
  // From GVR's perspective of view, this means a buffer can be acquired
  // asynchronously by the compositor.
  // From Android Surface's perspective of view, this is equivalent to
  // IGraphicBufferProducer's async mode. When in async mode, a producer
  // will never block even if consumer is running slow.
  bool is_async_{false};

  // Default buffer width that is set during ProducerQueue's creation.
  uint32_t default_width_{1};

  // Default buffer height that is set during ProducerQueue's creation.
  uint32_t default_height_{1};

  // Default buffer format that is set during ProducerQueue's creation.
  uint32_t default_format_{1};  // PIXEL_FORMAT_RGBA_8888

  // Tracks the buffers belonging to this queue. Buffers are stored according to
  // "slot" in this vector. Each slot is a logical id of the buffer within this
  // queue regardless of its queue position or presence in the ring buffer.
  std::array<std::shared_ptr<BufferHubBase>, kMaxQueueCapacity> buffers_;

  // Keeps track with how many buffers have been added into the queue.
  size_t capacity_{0};

  // Epoll fd used to manage buffer events.
  EpollFileDescriptor epoll_fd_;

  // Flag indicating that the other side hung up. For ProducerQueues this
  // triggers when BufferHub dies or explicitly closes the queue channel. For
  // ConsumerQueues this can either mean the same or that the ProducerQueue on
  // the other end hung up.
  bool hung_up_{false};

  // Global id for the queue that is consistent across processes.
  int id_{-1};

  // Buffer event callbacks
  BufferAvailableCallback on_buffer_available_;
  BufferRemovedCallback on_buffer_removed_;

  BufferHubQueue(const BufferHubQueue&) = delete;
  void operator=(BufferHubQueue&) = delete;
};

class ProducerQueue : public pdx::ClientBase<ProducerQueue, BufferHubQueue> {
 public:
  // Usage bits in |usage_set_mask| will be automatically masked on. Usage bits
  // in |usage_clear_mask| will be automatically masked off. Note that
  // |usage_set_mask| and |usage_clear_mask| may conflict with each other, but
  // |usage_set_mask| takes precedence over |usage_clear_mask|. All buffer
  // allocation through this producer queue shall not have any of the usage bits
  // in |usage_deny_set_mask| set. Allocation calls violating this will be
  // rejected. All buffer allocation through this producer queue must have all
  // the usage bits in |usage_deny_clear_mask| set. Allocation calls violating
  // this will be rejected. Note that |usage_deny_set_mask| and
  // |usage_deny_clear_mask| shall not conflict with each other. Such
  // configuration will be treated as invalid input on creation.
  static std::unique_ptr<ProducerQueue> Create(
      const ProducerQueueConfig& config, const UsagePolicy& usage) {
    return BASE::Create(config, usage);
  }

  // Import a ProducerQueue from a channel handle.
  static std::unique_ptr<ProducerQueue> Import(pdx::LocalChannelHandle handle) {
    return BASE::Create(std::move(handle));
  }

  // Get a producer buffer. Note that the method doesn't check whether the
  // buffer slot has a valid buffer that has been allocated already. When no
  // buffer has been imported before it returns nullptr; otherwise it returns
  // a shared pointer to a ProducerBuffer.
  std::shared_ptr<ProducerBuffer> GetBuffer(size_t slot) const {
    return std::static_pointer_cast<ProducerBuffer>(
        BufferHubQueue::GetBuffer(slot));
  }

  // Batch allocate buffers. Once allocated, producer buffers are automatically
  // enqueue'd into the ProducerQueue and available to use (i.e. in GAINED
  // state). Upon success, returns a list of slots for each buffer allocated.
  pdx::Status<std::vector<size_t>> AllocateBuffers(
      uint32_t width, uint32_t height, uint32_t layer_count, uint32_t format,
      uint64_t usage, size_t buffer_count);

  // Allocate producer buffer to populate the queue. Once allocated, a producer
  // buffer is automatically enqueue'd into the ProducerQueue and available to
  // use (i.e. in GAINED state). Upon success, returns the slot number for the
  // buffer allocated.
  pdx::Status<size_t> AllocateBuffer(uint32_t width, uint32_t height,
                                     uint32_t layer_count, uint32_t format,
                                     uint64_t usage);

  // Add a producer buffer to populate the queue. Once added, a producer buffer
  // is available to use (i.e. in GAINED state).
  pdx::Status<void> AddBuffer(const std::shared_ptr<ProducerBuffer>& buffer,
                              size_t slot);

  // Inserts a ProducerBuffer into the queue. On success, the method returns the
  // |slot| number where the new buffer gets inserted. Note that the buffer
  // being inserted should be in Gain'ed state prior to the call and it's
  // considered as already Dequeued when the function returns.
  pdx::Status<size_t> InsertBuffer(
      const std::shared_ptr<ProducerBuffer>& buffer);

  // Remove producer buffer from the queue.
  pdx::Status<void> RemoveBuffer(size_t slot) override;

  // Free all buffers on this producer queue.
  pdx::Status<void> FreeAllBuffers() override {
    return BufferHubQueue::FreeAllBuffers();
  }

  // Dequeue a producer buffer to write. The returned buffer in |Gain|'ed mode,
  // and caller should call Post() once it's done writing to release the buffer
  // to the consumer side.
  // @return a buffer in gained state, which was originally in released state.
  pdx::Status<std::shared_ptr<ProducerBuffer>> Dequeue(
      int timeout, size_t* slot, pdx::LocalHandle* release_fence);

  // Dequeue a producer buffer to write. The returned buffer in |Gain|'ed mode,
  // and caller should call Post() once it's done writing to release the buffer
  // to the consumer side.
  //
  // @param timeout to dequeue a buffer.
  // @param slot is the slot of the output ProducerBuffer.
  // @param release_fence for gaining a buffer.
  // @param out_meta metadata of the output buffer.
  // @param gain_posted_buffer whether to gain posted buffer if no released
  //     buffer is available to gain.
  // @return a buffer in gained state, which was originally in released state if
  //     gain_posted_buffer is false, or in posted/released state if
  //     gain_posted_buffer is true.
  // TODO(b/112007999): gain_posted_buffer true is only used to prevent
  // libdvrtracking from starving when there are non-responding clients. This
  // gain_posted_buffer param can be removed once libdvrtracking start to use
  // the new AHardwareBuffer API.
  pdx::Status<std::shared_ptr<ProducerBuffer>> Dequeue(
      int timeout, size_t* slot, DvrNativeBufferMetadata* out_meta,
      pdx::LocalHandle* release_fence, bool gain_posted_buffer = false);

  // Enqueues a producer buffer in the queue.
  pdx::Status<void> Enqueue(const std::shared_ptr<ProducerBuffer>& buffer,
                            size_t slot, uint64_t index) {
    return BufferHubQueue::Enqueue({buffer, slot, index});
  }

  // Takes out the current producer queue as a binder parcelable object. Note
  // that the queue must be empty to be exportable. After successful export, the
  // producer queue client should no longer be used.
  pdx::Status<ProducerQueueParcelable> TakeAsParcelable();

 private:
  friend BASE;

  // Constructors are automatically exposed through ProducerQueue::Create(...)
  // static template methods inherited from ClientBase, which take the same
  // arguments as the constructors.
  explicit ProducerQueue(pdx::LocalChannelHandle handle);
  ProducerQueue(const ProducerQueueConfig& config, const UsagePolicy& usage);

  // Dequeue a producer buffer to write. The returned buffer in |Gain|'ed mode,
  // and caller should call Post() once it's done writing to release the buffer
  // to the consumer side.
  //
  // @param slot the slot of the returned buffer.
  // @return a buffer in gained state, which was originally in posted state or
  //     released state.
  pdx::Status<std::shared_ptr<ProducerBuffer>> DequeueUnacquiredBuffer(
      size_t* slot);
};

class ConsumerQueue : public BufferHubQueue {
 public:
  // Get a consumer buffer. Note that the method doesn't check whether the
  // buffer slot has a valid buffer that has been imported already. When no
  // buffer has been imported before it returns nullptr; otherwise returns a
  // shared pointer to a ConsumerBuffer.
  std::shared_ptr<ConsumerBuffer> GetBuffer(size_t slot) const {
    return std::static_pointer_cast<ConsumerBuffer>(
        BufferHubQueue::GetBuffer(slot));
  }

  // Import a ConsumerQueue from a channel handle. |ignore_on_import| controls
  // whether or not buffers are set to be ignored when imported. This may be
  // used to avoid participation in the buffer lifecycle by a consumer queue
  // that is only used to spawn other consumer queues, such as in an
  // intermediate service.
  static std::unique_ptr<ConsumerQueue> Import(pdx::LocalChannelHandle handle);

  // Import newly created buffers from the service side.
  // Returns number of buffers successfully imported or an error.
  pdx::Status<size_t> ImportBuffers();

  // Dequeue a consumer buffer to read. The returned buffer in |Acquired|'ed
  // mode, and caller should call Releasse() once it's done writing to release
  // the buffer to the producer side. |meta| is passed along from BufferHub,
  // The user of ProducerBuffer is responsible with making sure that the
  // Dequeue() is done with the corect metadata type and size with those used
  // when the buffer is orignally created.
  template <typename Meta>
  pdx::Status<std::shared_ptr<ConsumerBuffer>> Dequeue(
      int timeout, size_t* slot, Meta* meta, pdx::LocalHandle* acquire_fence) {
    return Dequeue(timeout, slot, meta, sizeof(*meta), acquire_fence);
  }
  pdx::Status<std::shared_ptr<ConsumerBuffer>> Dequeue(
      int timeout, size_t* slot, pdx::LocalHandle* acquire_fence) {
    return Dequeue(timeout, slot, nullptr, 0, acquire_fence);
  }

  pdx::Status<std::shared_ptr<ConsumerBuffer>> Dequeue(
      int timeout, size_t* slot, void* meta, size_t user_metadata_size,
      pdx::LocalHandle* acquire_fence);
  pdx::Status<std::shared_ptr<ConsumerBuffer>> Dequeue(
      int timeout, size_t* slot, DvrNativeBufferMetadata* out_meta,
      pdx::LocalHandle* acquire_fence);

 private:
  friend BufferHubQueue;

  explicit ConsumerQueue(pdx::LocalChannelHandle handle);

  // Add a consumer buffer to populate the queue. Once added, a consumer buffer
  // is NOT available to use until the producer side |Post| it. |WaitForBuffers|
  // will catch the |Post| and |Acquire| the buffer to make it available for
  // consumer.
  pdx::Status<void> AddBuffer(const std::shared_ptr<ConsumerBuffer>& buffer,
                              size_t slot);

  pdx::Status<void> OnBufferAllocated() override;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_BUFFER_HUB_QUEUE_CLIENT_H_
