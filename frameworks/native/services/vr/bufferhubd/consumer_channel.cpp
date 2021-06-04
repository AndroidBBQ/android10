#include <thread>

#include <log/log.h>
#include <private/dvr/bufferhub_rpc.h>
#include <private/dvr/consumer_channel.h>
#include <private/dvr/producer_channel.h>
#include <utils/Trace.h>

using android::pdx::BorrowedHandle;
using android::pdx::Channel;
using android::pdx::ErrorStatus;
using android::pdx::Message;
using android::pdx::Status;
using android::pdx::rpc::DispatchRemoteMethod;

namespace android {
namespace dvr {

ConsumerChannel::ConsumerChannel(BufferHubService* service, int buffer_id,
                                 int channel_id, uint32_t client_state_mask,
                                 const std::shared_ptr<Channel> producer)
    : BufferHubChannel(service, buffer_id, channel_id, kConsumerType),
      client_state_mask_(client_state_mask),
      producer_(producer) {
  GetProducer()->AddConsumer(this);
}

ConsumerChannel::~ConsumerChannel() {
  ALOGD_IF(TRACE,
           "ConsumerChannel::~ConsumerChannel: channel_id=%d buffer_id=%d",
           channel_id(), buffer_id());

  if (auto producer = GetProducer()) {
    producer->RemoveConsumer(this);
  }
}

BufferHubChannel::BufferInfo ConsumerChannel::GetBufferInfo() const {
  BufferHubChannel::BufferInfo info;
  if (auto producer = GetProducer()) {
    // If producer has not hung up, copy most buffer info from the producer.
    info = producer->GetBufferInfo();
  } else {
    info.signaled_mask = client_state_mask();
  }
  info.id = buffer_id();
  return info;
}

std::shared_ptr<ProducerChannel> ConsumerChannel::GetProducer() const {
  return std::static_pointer_cast<ProducerChannel>(producer_.lock());
}

void ConsumerChannel::HandleImpulse(Message& message) {
  ATRACE_NAME("ConsumerChannel::HandleImpulse");
  switch (message.GetOp()) {
    case BufferHubRPC::ConsumerAcquire::Opcode:
      OnConsumerAcquire(message);
      break;
    case BufferHubRPC::ConsumerRelease::Opcode:
      OnConsumerRelease(message, {});
      break;
  }
}

bool ConsumerChannel::HandleMessage(Message& message) {
  ATRACE_NAME("ConsumerChannel::HandleMessage");
  auto producer = GetProducer();
  if (!producer)
    REPLY_ERROR_RETURN(message, EPIPE, true);

  switch (message.GetOp()) {
    case BufferHubRPC::GetBuffer::Opcode:
      DispatchRemoteMethod<BufferHubRPC::GetBuffer>(
          *this, &ConsumerChannel::OnGetBuffer, message);
      return true;

    case BufferHubRPC::NewConsumer::Opcode:
      DispatchRemoteMethod<BufferHubRPC::NewConsumer>(
          *producer, &ProducerChannel::OnNewConsumer, message);
      return true;

    case BufferHubRPC::ConsumerAcquire::Opcode:
      DispatchRemoteMethod<BufferHubRPC::ConsumerAcquire>(
          *this, &ConsumerChannel::OnConsumerAcquire, message);
      return true;

    case BufferHubRPC::ConsumerRelease::Opcode:
      DispatchRemoteMethod<BufferHubRPC::ConsumerRelease>(
          *this, &ConsumerChannel::OnConsumerRelease, message);
      return true;

    default:
      return false;
  }
}

Status<BufferDescription<BorrowedHandle>> ConsumerChannel::OnGetBuffer(
    Message& /*message*/) {
  ATRACE_NAME("ConsumerChannel::OnGetBuffer");
  ALOGD_IF(TRACE, "ConsumerChannel::OnGetBuffer: buffer=%d", buffer_id());
  if (auto producer = GetProducer()) {
    return {producer->GetBuffer(client_state_mask_)};
  } else {
    return ErrorStatus(EPIPE);
  }
}

Status<LocalFence> ConsumerChannel::OnConsumerAcquire(Message& message) {
  ATRACE_NAME("ConsumerChannel::OnConsumerAcquire");
  auto producer = GetProducer();
  if (!producer)
    return ErrorStatus(EPIPE);

  if (acquired_ || released_) {
    ALOGE(
        "ConsumerChannel::OnConsumerAcquire: Acquire when not posted: "
        "acquired=%d released=%d channel_id=%d buffer_id=%d",
        acquired_, released_, message.GetChannelId(), producer->buffer_id());
    return ErrorStatus(EBUSY);
  } else {
    auto status = producer->OnConsumerAcquire(message);
    if (status) {
      ClearAvailable();
      acquired_ = true;
    }
    return status;
  }
}

Status<void> ConsumerChannel::OnConsumerRelease(Message& message,
                                                LocalFence release_fence) {
  ATRACE_NAME("ConsumerChannel::OnConsumerRelease");
  auto producer = GetProducer();
  if (!producer)
    return ErrorStatus(EPIPE);

  if (!acquired_ || released_) {
    ALOGE(
        "ConsumerChannel::OnConsumerRelease: Release when not acquired: "
        "acquired=%d released=%d channel_id=%d buffer_id=%d",
        acquired_, released_, message.GetChannelId(), producer->buffer_id());
    return ErrorStatus(EBUSY);
  } else {
    auto status =
        producer->OnConsumerRelease(message, std::move(release_fence));
    if (status) {
      ClearAvailable();
      acquired_ = false;
      released_ = true;
    }
    return status;
  }
}

void ConsumerChannel::OnProducerGained() {
  // Clear the signal if exist. There is a possiblity that the signal still
  // exist in consumer client when producer gains the buffer, e.g. newly added
  // consumer fail to acquire the previous posted buffer in time. Then, when
  // producer gains back the buffer, posts the buffer again and signal the
  // consumer later, there won't be an signal change in eventfd, and thus,
  // consumer will miss the posted buffer later. Thus, we need to clear the
  // signal in consumer clients if the signal exist.
  ClearAvailable();
}

void ConsumerChannel::OnProducerPosted() {
  acquired_ = false;
  released_ = false;
  SignalAvailable();
}

void ConsumerChannel::OnProducerClosed() {
  producer_.reset();
  Hangup();
}

}  // namespace dvr
}  // namespace android
