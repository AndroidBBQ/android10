#ifndef ANDROID_DVR_BUFFERHUBD_PRODUCER_QUEUE_CHANNEL_H_
#define ANDROID_DVR_BUFFERHUBD_PRODUCER_QUEUE_CHANNEL_H_

#include <pdx/status.h>
#include <private/dvr/bufferhub_rpc.h>
#include <private/dvr/buffer_hub.h>

namespace android {
namespace dvr {

class ProducerQueueChannel : public BufferHubChannel {
 public:
  static pdx::Status<std::shared_ptr<ProducerQueueChannel>> Create(
      BufferHubService* service, int channel_id,
      const ProducerQueueConfig& config, const UsagePolicy& usage_policy);
  ~ProducerQueueChannel() override;

  bool HandleMessage(pdx::Message& message) override;
  void HandleImpulse(pdx::Message& message) override;

  BufferInfo GetBufferInfo() const override;

  // Handles client request to create a new consumer queue attached to current
  // producer queue.
  // Returns a handle for the service channel, as well as the size of the
  // metadata associated with the queue.
  pdx::Status<pdx::RemoteChannelHandle> OnCreateConsumerQueue(
      pdx::Message& message, bool silent);

  pdx::Status<QueueInfo> OnGetQueueInfo(pdx::Message& message);

  // Allocate a new BufferHubProducer according to the input spec. Client may
  // handle this as if a new producer is created through kOpCreateBuffer.
  pdx::Status<std::vector<std::pair<pdx::RemoteChannelHandle, size_t>>>
  OnProducerQueueAllocateBuffers(pdx::Message& message, uint32_t width,
                                 uint32_t height, uint32_t layer_count,
                                 uint32_t format, uint64_t usage,
                                 size_t buffer_count);

  // Inserts a ProducerBuffer into the queue. Note that the buffer must be in
  // Gain'ed state for the operation to succeed.
  pdx::Status<size_t> OnProducerQueueInsertBuffer(pdx::Message& message, int buffer_cid);

  // Removes a ProducerBuffer indicated by |slot|. Note that the buffer must be
  // in Gain'ed state for the operation to succeed.
  pdx::Status<void> OnProducerQueueRemoveBuffer(pdx::Message& message,
                                                size_t slot);

  void AddConsumer(ConsumerQueueChannel* channel);
  void RemoveConsumer(ConsumerQueueChannel* channel);

 private:
  ProducerQueueChannel(BufferHubService* service, int channel_id,
                       const ProducerQueueConfig& config,
                       const UsagePolicy& usage_policy, int* error);

  // Allocate one single producer buffer by |OnProducerQueueAllocateBuffers|.
  // Note that the newly created buffer's file handle will be pushed to client
  // and our return type is a RemoteChannelHandle.
  // Returns the remote channel handle and the slot number for the newly
  // allocated buffer.
  pdx::Status<std::pair<pdx::RemoteChannelHandle, size_t>> AllocateBuffer(
      pdx::Message& message, uint32_t width, uint32_t height,
      uint32_t layer_count, uint32_t format, uint64_t usage);

  // The producer queue's configuration. Now we assume the configuration is
  // immutable once the queue is created.
  ProducerQueueConfig config_;

  // A set of variables to control what |usage| bits can this ProducerQueue
  // allocate.
  UsagePolicy usage_policy_;

  // Provides access to the |channel_id| of all consumer channels associated
  // with this producer.
  std::vector<ConsumerQueueChannel*> consumer_channels_;

  // Tracks how many buffers have this queue allocated.
  size_t capacity_;

  // Tracks of all buffer producer allocated through this buffer queue. Once
  // a buffer get allocated, it will take a logical slot in the |buffers_| array
  // and the slot number will stay unchanged during the entire life cycle of the
  // queue.
  std::weak_ptr<ProducerChannel> buffers_[BufferHubRPC::kMaxQueueCapacity];

  ProducerQueueChannel(const ProducerQueueChannel&) = delete;
  void operator=(const ProducerQueueChannel&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_BUFFERHUBD_PRODUCER_QUEUE_CHANNEL_H_
