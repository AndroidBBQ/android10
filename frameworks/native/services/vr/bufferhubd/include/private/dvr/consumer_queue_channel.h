#ifndef ANDROID_DVR_BUFFERHUBD_CONSUMER_QUEUE_CHANNEL_H_
#define ANDROID_DVR_BUFFERHUBD_CONSUMER_QUEUE_CHANNEL_H_

#include <queue>

#include <private/dvr/buffer_hub.h>
#include <private/dvr/bufferhub_rpc.h>
#include <private/dvr/consumer_channel.h>
#include <private/dvr/producer_queue_channel.h>

namespace android {
namespace dvr {

class ConsumerQueueChannel : public BufferHubChannel {
 public:
  using Message = pdx::Message;
  using RemoteChannelHandle = pdx::RemoteChannelHandle;

  ConsumerQueueChannel(BufferHubService* service, int buffer_id, int channel_id,
                       const std::shared_ptr<Channel>& producer, bool silent);
  ~ConsumerQueueChannel() override;

  bool HandleMessage(Message& message) override;
  void HandleImpulse(Message& message) override;

  BufferInfo GetBufferInfo() const override;

  // Called by ProdcuerQueueChannel to notify consumer queue that a new
  // buffer has been allocated.
  void RegisterNewBuffer(
      const std::shared_ptr<ProducerChannel>& producer_channel,
      size_t producer_slot);

  // Called after clients been signaled by service that new buffer has been
  // allocated. Clients uses kOpConsumerQueueImportBuffers to import new
  // consumer buffers and this handler returns a vector of fd representing
  // BufferConsumers that clients can import.
  pdx::Status<std::vector<std::pair<RemoteChannelHandle, size_t>>>
  OnConsumerQueueImportBuffers(Message& message);

  void OnProducerClosed();

 private:
  // Data structure to store relavant info of a newly allocated producer buffer
  // so that consumer channel and buffer can be created later.
  struct PendingBuffer {
    PendingBuffer(std::shared_ptr<ProducerChannel> channel, size_t slot,
                  uint64_t mask) {
      producer_channel = channel;
      producer_slot = slot;
      consumer_state_mask = mask;
    }
    PendingBuffer() = delete;

    std::weak_ptr<ProducerChannel> producer_channel;
    size_t producer_slot;
    uint64_t consumer_state_mask;
  };

  std::shared_ptr<ProducerQueueChannel> GetProducer() const;

  // Pointer to the producer channel.
  std::weak_ptr<Channel> producer_;

  // Tracks newly allocated buffer producers along with it's slot number.
  std::queue<PendingBuffer> pending_buffer_slots_;

  // Tracks how many buffers have this queue imported.
  size_t capacity_;

  // A silent queue does not signal or export buffers. It is only used to spawn
  // another consumer queue.
  bool silent_;

  ConsumerQueueChannel(const ConsumerQueueChannel&) = delete;
  void operator=(const ConsumerQueueChannel&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_BUFFERHUBD_CONSUMER_QUEUE_CHANNEL_H_
