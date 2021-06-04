#include <inttypes.h>

#include <private/dvr/consumer_queue_channel.h>
#include <private/dvr/producer_channel.h>
#include <private/dvr/producer_queue_channel.h>

using android::pdx::ErrorStatus;
using android::pdx::Message;
using android::pdx::RemoteChannelHandle;
using android::pdx::Status;
using android::pdx::rpc::DispatchRemoteMethod;

namespace android {
namespace dvr {

ProducerQueueChannel::ProducerQueueChannel(BufferHubService* service,
                                           int channel_id,
                                           const ProducerQueueConfig& config,
                                           const UsagePolicy& usage_policy,
                                           int* error)
    : BufferHubChannel(service, channel_id, channel_id, kProducerQueueType),
      config_(config),
      usage_policy_(usage_policy),
      capacity_(0) {
  *error = 0;
}

ProducerQueueChannel::~ProducerQueueChannel() {
  ALOGD_IF(TRACE, "ProducerQueueChannel::~ProducerQueueChannel: queue_id=%d",
           buffer_id());
  for (auto* consumer : consumer_channels_)
    consumer->OnProducerClosed();
}

/* static */
Status<std::shared_ptr<ProducerQueueChannel>> ProducerQueueChannel::Create(
    BufferHubService* service, int channel_id,
    const ProducerQueueConfig& config, const UsagePolicy& usage_policy) {
  // Configuration between |usage_deny_set_mask| and |usage_deny_clear_mask|
  // should be mutually exclusive.
  if ((usage_policy.usage_deny_set_mask & usage_policy.usage_deny_clear_mask)) {
    ALOGE(
        "BufferHubService::OnCreateProducerQueue: illegal usage mask "
        "configuration: usage_deny_set_mask=%" PRIx64
        " usage_deny_clear_mask=%" PRIx64,
        usage_policy.usage_deny_set_mask, usage_policy.usage_deny_clear_mask);
    return ErrorStatus(EINVAL);
  }

  int error = 0;
  std::shared_ptr<ProducerQueueChannel> producer(new ProducerQueueChannel(
      service, channel_id, config, usage_policy, &error));
  if (error < 0)
    return ErrorStatus(-error);
  else
    return {std::move(producer)};
}

bool ProducerQueueChannel::HandleMessage(Message& message) {
  ATRACE_NAME("ProducerQueueChannel::HandleMessage");
  switch (message.GetOp()) {
    case BufferHubRPC::CreateConsumerQueue::Opcode:
      DispatchRemoteMethod<BufferHubRPC::CreateConsumerQueue>(
          *this, &ProducerQueueChannel::OnCreateConsumerQueue, message);
      return true;

    case BufferHubRPC::GetQueueInfo::Opcode:
      DispatchRemoteMethod<BufferHubRPC::GetQueueInfo>(
          *this, &ProducerQueueChannel::OnGetQueueInfo, message);
      return true;

    case BufferHubRPC::ProducerQueueAllocateBuffers::Opcode:
      DispatchRemoteMethod<BufferHubRPC::ProducerQueueAllocateBuffers>(
          *this, &ProducerQueueChannel::OnProducerQueueAllocateBuffers,
          message);
      return true;

    case BufferHubRPC::ProducerQueueInsertBuffer::Opcode:
      DispatchRemoteMethod<BufferHubRPC::ProducerQueueInsertBuffer>(
          *this, &ProducerQueueChannel::OnProducerQueueInsertBuffer, message);
      return true;

    case BufferHubRPC::ProducerQueueRemoveBuffer::Opcode:
      DispatchRemoteMethod<BufferHubRPC::ProducerQueueRemoveBuffer>(
          *this, &ProducerQueueChannel::OnProducerQueueRemoveBuffer, message);
      return true;

    default:
      return false;
  }
}

void ProducerQueueChannel::HandleImpulse(Message& /* message */) {
  ATRACE_NAME("ProducerQueueChannel::HandleImpulse");
}

BufferHubChannel::BufferInfo ProducerQueueChannel::GetBufferInfo() const {
  return BufferInfo(channel_id(), consumer_channels_.size(), capacity_,
                    usage_policy_);
}

Status<RemoteChannelHandle> ProducerQueueChannel::OnCreateConsumerQueue(
    Message& message, bool silent) {
  ATRACE_NAME("ProducerQueueChannel::OnCreateConsumerQueue");
  ALOGD_IF(
      TRACE,
      "ProducerQueueChannel::OnCreateConsumerQueue: channel_id=%d slient=%d",
      channel_id(), silent);

  int channel_id;
  auto status = message.PushChannel(0, nullptr, &channel_id);
  if (!status) {
    ALOGE(
        "ProducerQueueChannel::OnCreateConsumerQueue: failed to push consumer "
        "channel: %s",
        status.GetErrorMessage().c_str());
    return ErrorStatus(ENOMEM);
  }

  auto consumer_queue_channel = std::make_shared<ConsumerQueueChannel>(
      service(), buffer_id(), channel_id, shared_from_this(), silent);

  // Register the existing buffers with the new consumer queue.
  for (size_t slot = 0; slot < BufferHubRPC::kMaxQueueCapacity; slot++) {
    if (auto buffer = buffers_[slot].lock())
      consumer_queue_channel->RegisterNewBuffer(buffer, slot);
  }

  const auto channel_status =
      service()->SetChannel(channel_id, consumer_queue_channel);
  if (!channel_status) {
    ALOGE(
        "ProducerQueueChannel::OnCreateConsumerQueue: Failed to set channel: "
        "%s",
        channel_status.GetErrorMessage().c_str());
    return ErrorStatus(ENOMEM);
  }

  return {status.take()};
}

Status<QueueInfo> ProducerQueueChannel::OnGetQueueInfo(Message&) {
  return {{config_, buffer_id()}};
}

Status<std::vector<std::pair<RemoteChannelHandle, size_t>>>
ProducerQueueChannel::OnProducerQueueAllocateBuffers(
    Message& message, uint32_t width, uint32_t height, uint32_t layer_count,
    uint32_t format, uint64_t usage, size_t buffer_count) {
  ATRACE_NAME("ProducerQueueChannel::OnProducerQueueAllocateBuffers");
  ALOGD_IF(TRACE,
           "ProducerQueueChannel::OnProducerQueueAllocateBuffers: "
           "producer_channel_id=%d",
           channel_id());

  std::vector<std::pair<RemoteChannelHandle, size_t>> buffer_handles;

  // Deny buffer allocation violating preset rules.
  if (usage & usage_policy_.usage_deny_set_mask) {
    ALOGE(
        "ProducerQueueChannel::OnProducerQueueAllocateBuffers: usage: %" PRIx64
        " is not permitted. Violating usage_deny_set_mask, the following  bits "
        "shall not be set: %" PRIx64 ".",
        usage, usage_policy_.usage_deny_set_mask);
    return ErrorStatus(EINVAL);
  }

  if (~usage & usage_policy_.usage_deny_clear_mask) {
    ALOGE(
        "ProducerQueueChannel::OnProducerQueueAllocateBuffers: usage: %" PRIx64
        " is not permitted. Violating usage_deny_clear_mask, the following "
        "bits must be set: %" PRIx64 ".",
        usage, usage_policy_.usage_deny_clear_mask);
    return ErrorStatus(EINVAL);
  }

  // Force set mask and clear mask. Note that |usage_policy_.usage_set_mask_|
  // takes precedence and will overwrite |usage_policy_.usage_clear_mask|.
  uint64_t effective_usage =
      (usage & ~usage_policy_.usage_clear_mask) | usage_policy_.usage_set_mask;

  for (size_t i = 0; i < buffer_count; i++) {
    auto status = AllocateBuffer(message, width, height, layer_count, format,
                                 effective_usage);
    if (!status) {
      ALOGE(
          "ProducerQueueChannel::OnProducerQueueAllocateBuffers: Failed to "
          "allocate new buffer.");
      return ErrorStatus(status.error());
    }
    buffer_handles.push_back(status.take());
  }

  return {std::move(buffer_handles)};
}

Status<std::pair<RemoteChannelHandle, size_t>>
ProducerQueueChannel::AllocateBuffer(Message& message, uint32_t width,
                                     uint32_t height, uint32_t layer_count,
                                     uint32_t format, uint64_t usage) {
  ATRACE_NAME("ProducerQueueChannel::AllocateBuffer");
  ALOGD_IF(TRACE,
           "ProducerQueueChannel::AllocateBuffer: producer_channel_id=%d",
           channel_id());

  if (capacity_ >= BufferHubRPC::kMaxQueueCapacity) {
    ALOGE("ProducerQueueChannel::AllocateBuffer: reaches kMaxQueueCapacity.");
    return ErrorStatus(E2BIG);
  }

  // Here we are creating a new BufferHubBuffer, initialize the producer
  // channel, and returning its file handle back to the client.
  // buffer_id is the id of the producer channel of BufferHubBuffer.
  int buffer_id;
  auto status = message.PushChannel(0, nullptr, &buffer_id);

  if (!status) {
    ALOGE("ProducerQueueChannel::AllocateBuffer: failed to push channel: %s",
          status.GetErrorMessage().c_str());
    return ErrorStatus(status.error());
  }

  ALOGD_IF(TRACE,
           "ProducerQueueChannel::AllocateBuffer: buffer_id=%d width=%u "
           "height=%u layer_count=%u format=%u usage=%" PRIx64,
           buffer_id, width, height, layer_count, format, usage);
  auto buffer_handle = status.take();

  auto producer_channel_status =
      ProducerChannel::Create(service(), buffer_id, width, height, layer_count,
                              format, usage, config_.user_metadata_size);
  if (!producer_channel_status) {
    ALOGE(
        "ProducerQueueChannel::AllocateBuffer: Failed to create producer "
        "buffer: %s",
        producer_channel_status.GetErrorMessage().c_str());
    return ErrorStatus(ENOMEM);
  }
  auto producer_channel = producer_channel_status.take();

  ALOGD_IF(
      TRACE,
      "ProducerQueueChannel::AllocateBuffer: buffer_id=%d, buffer_handle=%d",
      buffer_id, buffer_handle.value());

  const auto channel_status =
      service()->SetChannel(buffer_id, producer_channel);
  if (!channel_status) {
    ALOGE(
        "ProducerQueueChannel::AllocateBuffer: failed to set producer channel "
        "for new BufferHubBuffer: %s",
        channel_status.GetErrorMessage().c_str());
    return ErrorStatus(ENOMEM);
  }

  // Register the newly allocated buffer's channel_id into the first empty
  // buffer slot.
  size_t slot = 0;
  for (; slot < BufferHubRPC::kMaxQueueCapacity; slot++) {
    if (buffers_[slot].expired())
      break;
  }
  if (slot == BufferHubRPC::kMaxQueueCapacity) {
    ALOGE(
        "ProducerQueueChannel::AllocateBuffer: Cannot find empty slot for new "
        "buffer allocation.");
    return ErrorStatus(E2BIG);
  }

  buffers_[slot] = producer_channel;
  capacity_++;

  // Notify each consumer channel about the new buffer.
  for (auto* consumer_channel : consumer_channels_) {
    ALOGD(
        "ProducerQueueChannel::AllocateBuffer: Notified consumer with new "
        "buffer, buffer_id=%d",
        buffer_id);
    consumer_channel->RegisterNewBuffer(producer_channel, slot);
  }

  return {{std::move(buffer_handle), slot}};
}

Status<size_t> ProducerQueueChannel::OnProducerQueueInsertBuffer(
    pdx::Message& message, int buffer_cid) {
  ATRACE_NAME("ProducerQueueChannel::InsertBuffer");
  ALOGD_IF(TRACE,
           "ProducerQueueChannel::InsertBuffer: channel_id=%d, buffer_cid=%d",
           channel_id(), buffer_cid);

  if (capacity_ >= BufferHubRPC::kMaxQueueCapacity) {
    ALOGE("ProducerQueueChannel::InsertBuffer: reaches kMaxQueueCapacity.");
    return ErrorStatus(E2BIG);
  }
  auto producer_channel = std::static_pointer_cast<ProducerChannel>(
      service()->GetChannel(buffer_cid));
  if (producer_channel == nullptr ||
      producer_channel->channel_type() != BufferHubChannel::kProducerType) {
    // Rejects the request if the requested buffer channel is invalid and/or
    // it's not a ProducerChannel.
    ALOGE(
        "ProducerQueueChannel::InsertBuffer: Invalid buffer_cid=%d, "
        "producer_buffer=0x%p, channel_type=%d.",
        buffer_cid, producer_channel.get(),
        producer_channel == nullptr ? -1 : producer_channel->channel_type());
    return ErrorStatus(EINVAL);
  }
  if (producer_channel->GetActiveProcessId() != message.GetProcessId()) {
    // Rejects the request if the requested buffer channel is not currently
    // connected to the caller this is IPC request. This effectively prevents
    // fake buffer_cid from being injected.
    ALOGE(
        "ProducerQueueChannel::InsertBuffer: Requested buffer channel "
        "(buffer_cid=%d) is not connected to the calling process (pid=%d). "
        "It's connected to a different process (pid=%d).",
        buffer_cid, message.GetProcessId(),
        producer_channel->GetActiveProcessId());
    return ErrorStatus(EINVAL);
  }
  uint64_t buffer_state = producer_channel->buffer_state();
  // TODO(b/112007999) add an atomic variable in metadata header in shared
  // memory to indicate which client is the last producer of the buffer.
  // Currently, the first client is the only producer to the buffer.
  // Thus, it checks whether the first client gains the buffer below.
  if (!BufferHubDefs::isClientGained(buffer_state,
                                     BufferHubDefs::kFirstClientBitMask)) {
    // Rejects the request if the requested buffer is not in Gained state.
    ALOGE(
        "ProducerQueueChannel::InsertBuffer: The buffer (cid=%d, "
        "state=0x%" PRIx64 ") is not in gained state.",
        buffer_cid, buffer_state);
    return ErrorStatus(EINVAL);
  }

  // Register the to-be-inserted buffer's channel_id into the first empty
  // buffer slot.
  size_t slot = 0;
  for (; slot < BufferHubRPC::kMaxQueueCapacity; slot++) {
    if (buffers_[slot].expired())
      break;
  }
  if (slot == BufferHubRPC::kMaxQueueCapacity) {
    ALOGE(
        "ProducerQueueChannel::AllocateBuffer: Cannot find empty slot for new "
        "buffer allocation.");
    return ErrorStatus(E2BIG);
  }

  buffers_[slot] = producer_channel;
  capacity_++;

  // Notify each consumer channel about the new buffer.
  for (auto* consumer_channel : consumer_channels_) {
    ALOGD(
        "ProducerQueueChannel::AllocateBuffer: Notified consumer with new "
        "buffer, buffer_cid=%d",
        buffer_cid);
    consumer_channel->RegisterNewBuffer(producer_channel, slot);
  }

  return {slot};
}

Status<void> ProducerQueueChannel::OnProducerQueueRemoveBuffer(
    Message& /*message*/, size_t slot) {
  if (buffers_[slot].expired()) {
    ALOGE(
        "ProducerQueueChannel::OnProducerQueueRemoveBuffer: trying to remove "
        "an invalid buffer producer at slot %zu",
        slot);
    return ErrorStatus(EINVAL);
  }

  if (capacity_ == 0) {
    ALOGE(
        "ProducerQueueChannel::OnProducerQueueRemoveBuffer: trying to remove a "
        "buffer producer while the queue's capacity is already zero.");
    return ErrorStatus(EINVAL);
  }

  buffers_[slot].reset();
  capacity_--;
  return {};
}

void ProducerQueueChannel::AddConsumer(ConsumerQueueChannel* channel) {
  consumer_channels_.push_back(channel);
}

void ProducerQueueChannel::RemoveConsumer(ConsumerQueueChannel* channel) {
  consumer_channels_.erase(
      std::find(consumer_channels_.begin(), consumer_channels_.end(), channel));
}

}  // namespace dvr
}  // namespace android
