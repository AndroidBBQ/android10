#ifndef ANDROID_DVR_BUFFERHUBD_CONSUMER_CHANNEL_H_
#define ANDROID_DVR_BUFFERHUBD_CONSUMER_CHANNEL_H_

#include <pdx/rpc/buffer_wrapper.h>
#include <private/dvr/bufferhub_rpc.h>
#include <private/dvr/buffer_hub.h>

namespace android {
namespace dvr {

// Consumer channels are attached to a Producer channel
class ConsumerChannel : public BufferHubChannel {
 public:
  using BorrowedHandle = pdx::BorrowedHandle;
  using Channel = pdx::Channel;
  using Message = pdx::Message;

  ConsumerChannel(BufferHubService* service, int buffer_id, int channel_id,
                  uint32_t client_state_mask,
                  const std::shared_ptr<Channel> producer);
  ~ConsumerChannel() override;

  bool HandleMessage(Message& message) override;
  void HandleImpulse(Message& message) override;

  uint32_t client_state_mask() const { return client_state_mask_; }
  BufferInfo GetBufferInfo() const override;

  void OnProducerGained();
  void OnProducerPosted();
  void OnProducerClosed();

 private:
  std::shared_ptr<ProducerChannel> GetProducer() const;

  pdx::Status<BufferDescription<BorrowedHandle>> OnGetBuffer(Message& message);

  pdx::Status<LocalFence> OnConsumerAcquire(Message& message);
  pdx::Status<void> OnConsumerRelease(Message& message,
                                      LocalFence release_fence);

  uint32_t client_state_mask_{0U};
  bool acquired_{false};
  bool released_{true};
  std::weak_ptr<Channel> producer_;

  ConsumerChannel(const ConsumerChannel&) = delete;
  void operator=(const ConsumerChannel&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_BUFFERHUBD_CONSUMER_CHANNEL_H_
