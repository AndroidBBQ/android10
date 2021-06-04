#ifndef ANDROID_DVR_BUFFERHUBD_PRODUCER_CHANNEL_H_
#define ANDROID_DVR_BUFFERHUBD_PRODUCER_CHANNEL_H_

#include <functional>
#include <memory>
#include <vector>

#include <pdx/channel_handle.h>
#include <pdx/file_handle.h>
#include <pdx/rpc/buffer_wrapper.h>
#include <private/dvr/buffer_hub.h>
#include <private/dvr/bufferhub_rpc.h>
#include <private/dvr/ion_buffer.h>

namespace android {
namespace dvr {

// The buffer changes ownership according to the following sequence:
// POST -> ACQUIRE/RELEASE (all consumers) -> GAIN (producer acquires) -> POST

// The producer channel is owned by a single app that writes into buffers and
// calls POST when drawing is complete. This channel has a set of consumer
// channels associated with it that are waiting for notifications.
class ProducerChannel : public BufferHubChannel {
 public:
  using Message = pdx::Message;
  using BorrowedHandle = pdx::BorrowedHandle;
  using RemoteChannelHandle = pdx::RemoteChannelHandle;
  template <typename T>
  using BufferWrapper = pdx::rpc::BufferWrapper<T>;

  static std::unique_ptr<ProducerChannel> Create(BufferHubService* service,
                                                 int buffer_id, int channel_id,
                                                 IonBuffer buffer,
                                                 IonBuffer metadata_buffer,
                                                 size_t user_metadata_size);

  static pdx::Status<std::shared_ptr<ProducerChannel>> Create(
      BufferHubService* service, int channel_id, uint32_t width,
      uint32_t height, uint32_t layer_count, uint32_t format, uint64_t usage,
      size_t user_metadata_size);

  ~ProducerChannel() override;

  uint32_t buffer_state() const {
    return buffer_state_->load(std::memory_order_acquire);
  }

  bool HandleMessage(Message& message) override;
  void HandleImpulse(Message& message) override;

  BufferInfo GetBufferInfo() const override;

  BufferDescription<BorrowedHandle> GetBuffer(uint32_t client_state_mask);

  pdx::Status<RemoteChannelHandle> CreateConsumer(Message& message,
                                                  uint32_t consumer_state_mask);
  pdx::Status<uint32_t> CreateConsumerStateMask();
  pdx::Status<RemoteChannelHandle> OnNewConsumer(Message& message);

  pdx::Status<LocalFence> OnConsumerAcquire(Message& message);
  pdx::Status<void> OnConsumerRelease(Message& message,
                                      LocalFence release_fence);

  void OnConsumerOrphaned(const uint32_t& consumer_state_mask);

  void AddConsumer(ConsumerChannel* channel);
  void RemoveConsumer(ConsumerChannel* channel);

  bool CheckParameters(uint32_t width, uint32_t height, uint32_t layer_count,
                       uint32_t format, uint64_t usage,
                       size_t user_metadata_size) const;

 private:
  std::vector<ConsumerChannel*> consumer_channels_;

  IonBuffer buffer_;

  // IonBuffer that is shared between bufferhubd, producer, and consumers.
  IonBuffer metadata_buffer_;
  BufferHubDefs::MetadataHeader* metadata_header_ = nullptr;
  std::atomic<uint32_t>* buffer_state_ = nullptr;
  std::atomic<uint32_t>* fence_state_ = nullptr;
  std::atomic<uint32_t>* active_clients_bit_mask_ = nullptr;

  // All orphaned consumer bits. Valid bits are the lower 63 bits, while the
  // highest bit is reserved for the producer and should not be set.
  uint32_t orphaned_consumer_bit_mask_{0U};

  LocalFence post_fence_;
  LocalFence returned_fence_;
  size_t user_metadata_size_;  // size of user requested buffer buffer size.
  size_t metadata_buf_size_;   // size of the ion buffer that holds metadata.

  pdx::LocalHandle acquire_fence_fd_;
  pdx::LocalHandle release_fence_fd_;
  pdx::LocalHandle dummy_fence_fd_;

  ProducerChannel(BufferHubService* service, int buffer_id, int channel_id,
                  IonBuffer buffer, IonBuffer metadata_buffer,
                  size_t user_metadata_size, int* error);
  ProducerChannel(BufferHubService* service, int channel, uint32_t width,
                  uint32_t height, uint32_t layer_count, uint32_t format,
                  uint64_t usage, size_t user_metadata_size, int* error);

  int InitializeBuffer();
  pdx::Status<BufferDescription<BorrowedHandle>> OnGetBuffer(Message& message);
  pdx::Status<void> OnProducerPost(Message& message, LocalFence acquire_fence);
  pdx::Status<LocalFence> OnProducerGain(Message& message);

  // Remove consumer from atomics in shared memory based on consumer_state_mask.
  // This function is used for clean up for failures in CreateConsumer method.
  void RemoveConsumerClientMask(uint32_t consumer_state_mask);

  // Checks whether the buffer is released by all active clients, excluding
  // orphaned consumers.
  bool IsBufferReleasedByAllActiveClientsExceptForOrphans() const;

  ProducerChannel(const ProducerChannel&) = delete;
  void operator=(const ProducerChannel&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_BUFFERHUBD_PRODUCER_CHANNEL_H_
