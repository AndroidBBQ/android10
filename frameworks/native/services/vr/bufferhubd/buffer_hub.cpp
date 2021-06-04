#include <inttypes.h>
#include <poll.h>

#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include <log/log.h>
#include <pdx/default_transport/service_endpoint.h>
#include <private/dvr/bufferhub_rpc.h>
#include <private/dvr/buffer_hub.h>
#include <private/dvr/consumer_channel.h>
#include <private/dvr/producer_channel.h>
#include <private/dvr/producer_queue_channel.h>
#include <utils/Trace.h>

using android::pdx::Channel;
using android::pdx::ErrorStatus;
using android::pdx::Message;
using android::pdx::Status;
using android::pdx::default_transport::Endpoint;
using android::pdx::rpc::DispatchRemoteMethod;

namespace android {
namespace dvr {

BufferHubService::BufferHubService()
    : BASE("BufferHub", Endpoint::Create(BufferHubRPC::kClientPath)) {}

BufferHubService::~BufferHubService() {}

bool BufferHubService::IsInitialized() const { return BASE::IsInitialized(); }

std::string BufferHubService::DumpState(size_t /*max_length*/) {
  std::ostringstream stream;
  auto channels = GetChannels<BufferHubChannel>();

  std::sort(channels.begin(), channels.end(),
            [](const std::shared_ptr<BufferHubChannel>& a,
               const std::shared_ptr<BufferHubChannel>& b) {
              return a->buffer_id() < b->buffer_id();
            });

  stream << "Active Producer Buffers:\n";
  stream << std::right;
  stream << std::setw(6) << "Id";
  stream << " ";
  stream << std::setw(9) << "Consumers";
  stream << " ";
  stream << std::setw(14) << "Geometry";
  stream << " ";
  stream << std::setw(6) << "Format";
  stream << " ";
  stream << std::setw(10) << "Usage";
  stream << " ";
  stream << std::setw(18) << "State";
  stream << " ";
  stream << std::setw(18) << "Signaled";
  stream << " ";
  stream << std::setw(10) << "Index";
  stream << std::endl;

  for (const auto& channel : channels) {
    if (channel->channel_type() == BufferHubChannel::kProducerType) {
      BufferHubChannel::BufferInfo info = channel->GetBufferInfo();

      stream << std::right;
      stream << std::setw(6) << info.id;
      stream << " ";
      stream << std::setw(9) << info.consumer_count;
      stream << " ";
      if (info.format == HAL_PIXEL_FORMAT_BLOB) {
        std::string size = std::to_string(info.width) + " B";
        stream << std::setw(14) << size;
      } else {
        std::string dimensions = std::to_string(info.width) + "x" +
                                 std::to_string(info.height) + "x" +
                                 std::to_string(info.layer_count);
        stream << std::setw(14) << dimensions;
      }
      stream << " ";
      stream << std::setw(6) << info.format;
      stream << " ";
      stream << "0x" << std::hex << std::setfill('0');
      stream << std::setw(8) << info.usage;
      stream << std::dec << std::setfill(' ');
      stream << " ";
      stream << "0x" << std::hex << std::setfill('0');
      stream << std::setw(16) << info.state;
      stream << " ";
      stream << "0x" << std::setw(16) << info.signaled_mask;
      stream << std::dec << std::setfill(' ');
      stream << " ";
      stream << std::setw(8) << info.index;
      stream << std::endl;
    }

    if (channel->channel_type() == BufferHubChannel::kDetachedBufferType) {
      BufferHubChannel::BufferInfo info = channel->GetBufferInfo();

      stream << std::right;
      stream << std::setw(6) << info.id;
      stream << " ";
      stream << std::setw(9) << "N/A";
      stream << " ";
      if (info.format == HAL_PIXEL_FORMAT_BLOB) {
        std::string size = std::to_string(info.width) + " B";
        stream << std::setw(14) << size;
      } else {
        std::string dimensions = std::to_string(info.width) + "x" +
                                 std::to_string(info.height) + "x" +
                                 std::to_string(info.layer_count);
        stream << std::setw(14) << dimensions;
      }
      stream << " ";
      stream << std::setw(6) << info.format;
      stream << " ";
      stream << "0x" << std::hex << std::setfill('0');
      stream << std::setw(8) << info.usage;
      stream << std::dec << std::setfill(' ');
      stream << " ";
      stream << std::setw(9) << "N/A";
      stream << " ";
      stream << std::hex << std::setfill(' ');
      stream << std::setw(18) << "Detached";
      stream << " ";
      stream << std::setw(18) << "N/A";
      stream << " ";
      stream << std::setw(10) << "N/A";
      stream << std::endl;
    }
  }

  stream << std::endl;
  stream << "Active Producer Queues:\n";
  stream << std::right << std::setw(6) << "Id";
  stream << std::right << std::setw(12) << " Capacity";
  stream << std::right << std::setw(12) << " Consumers";
  stream << " UsageSetMask";
  stream << " UsageClearMask";
  stream << " UsageDenySetMask";
  stream << " UsageDenyClearMask";
  stream << " ";
  stream << std::endl;

  for (const auto& channel : channels) {
    if (channel->channel_type() == BufferHubChannel::kProducerQueueType) {
      BufferHubChannel::BufferInfo info = channel->GetBufferInfo();

      stream << std::dec << std::setfill(' ');
      stream << std::right << std::setw(6) << info.id;
      stream << std::right << std::setw(12) << info.capacity;
      stream << std::right << std::setw(12) << info.consumer_count;
      stream << std::setw(5) << std::setfill(' ') << "0x";
      stream << std::hex << std::setfill('0');
      stream << std::setw(8) << info.usage_policy.usage_set_mask;
      stream << std::setw(7) << std::setfill(' ') << "0x";
      stream << std::hex << std::setfill('0');
      stream << std::setw(8) << info.usage_policy.usage_clear_mask;
      stream << std::setw(9) << std::setfill(' ') << "0x";
      stream << std::hex << std::setfill('0');
      stream << std::setw(8) << info.usage_policy.usage_deny_set_mask;
      stream << std::setw(11) << std::setfill(' ') << "0x";
      stream << std::hex << std::setfill('0');
      stream << std::setw(8) << info.usage_policy.usage_deny_clear_mask;
      stream << std::hex << std::setfill('0');
      stream << std::endl;
    }
  }

  stream << std::endl;
  stream << "Active Consumer Queues:\n";
  stream << std::dec << std::setfill(' ');
  stream << std::right << std::setw(6) << "Id";
  stream << std::right << std::setw(12) << " Imported";
  stream << " ";
  stream << std::endl;

  for (const auto& channel : channels) {
    if (channel->channel_type() == BufferHubChannel::kConsumerQueueType) {
      BufferHubChannel::BufferInfo info = channel->GetBufferInfo();

      stream << std::right << std::setw(6) << info.id;
      stream << std::right << std::setw(12) << info.capacity;
      stream << std::endl;
    }
  }

  stream << std::endl;
  stream << "Orphaned Consumer Buffers:\n";
  stream << std::right;
  stream << std::setw(6) << "Id";
  stream << " ";
  stream << std::setw(14) << "Info";
  stream << std::endl;

  for (const auto& channel : channels) {
    BufferHubChannel::BufferInfo info = channel->GetBufferInfo();
    // consumer_count is tracked by producer. When it's zero, producer must have
    // already hung up and the consumer is orphaned.
    if (channel->channel_type() == BufferHubChannel::kConsumerType &&
        info.consumer_count == 0) {
      stream << std::right;
      stream << std::setw(6) << info.id;
      stream << " ";

      stream << std::setw(14) << "Orphaned.";
      stream << (" channel_id=" + std::to_string(channel->channel_id()));
      stream << std::endl;
    }
  }

  return stream.str();
}

void BufferHubService::HandleImpulse(Message& message) {
  ATRACE_NAME("BufferHubService::HandleImpulse");
  if (auto channel = message.GetChannel<BufferHubChannel>())
    channel->HandleImpulse(message);
}

pdx::Status<void> BufferHubService::HandleMessage(Message& message) {
  ATRACE_NAME("BufferHubService::HandleMessage");
  auto channel = message.GetChannel<BufferHubChannel>();

  ALOGD_IF(
      TRACE,
      "BufferHubService::HandleMessage: channel=%p channel_id=%d opcode=%d",
      channel.get(), message.GetChannelId(), message.GetOp());

  // If the channel is already set up, let it handle the message.
  if (channel && !channel->HandleMessage(message))
    return DefaultHandleMessage(message);

  // This channel has not been set up yet, the following are valid operations.
  switch (message.GetOp()) {
    case BufferHubRPC::CreateBuffer::Opcode:
      DispatchRemoteMethod<BufferHubRPC::CreateBuffer>(
          *this, &BufferHubService::OnCreateBuffer, message);
      return {};

    case BufferHubRPC::CreateProducerQueue::Opcode:
      DispatchRemoteMethod<BufferHubRPC::CreateProducerQueue>(
          *this, &BufferHubService::OnCreateProducerQueue, message);
      return {};

    default:
      return DefaultHandleMessage(message);
  }
}

Status<void> BufferHubService::OnCreateBuffer(Message& message, uint32_t width,
                                              uint32_t height, uint32_t format,
                                              uint64_t usage,
                                              size_t meta_size_bytes) {
  // Use the producer channel id as the global buffer id.
  const int buffer_id = message.GetChannelId();
  ALOGD_IF(TRACE,
           "BufferHubService::OnCreateBuffer: buffer_id=%d width=%u height=%u "
           "format=%u usage=%" PRIx64 " meta_size_bytes=%zu",
           buffer_id, width, height, format, usage, meta_size_bytes);

  // See if this channel is already attached to a buffer.
  if (const auto channel = message.GetChannel<BufferHubChannel>()) {
    ALOGE("BufferHubService::OnCreateBuffer: Buffer already created: buffer=%d",
          buffer_id);
    return ErrorStatus(EALREADY);
  }
  const uint32_t kDefaultLayerCount = 1;
  auto status = ProducerChannel::Create(this, buffer_id, width, height,
                                        kDefaultLayerCount, format, usage,
                                        meta_size_bytes);
  if (status) {
    message.SetChannel(status.take());
    return {};
  } else {
    ALOGE("BufferHubService::OnCreateBuffer: Failed to create producer: %s",
          status.GetErrorMessage().c_str());
    return status.error_status();
  }
}

Status<QueueInfo> BufferHubService::OnCreateProducerQueue(
    pdx::Message& message, const ProducerQueueConfig& producer_config,
    const UsagePolicy& usage_policy) {
  // Use the producer channel id as the global queue id.
  const int queue_id = message.GetChannelId();
  ALOGD_IF(TRACE, "BufferHubService::OnCreateProducerQueue: queue_id=%d",
           queue_id);

  // See if this channel is already attached to another object.
  if (const auto channel = message.GetChannel<BufferHubChannel>()) {
    ALOGE("BufferHubService::OnCreateProducerQueue: already created: queue=%d",
          queue_id);
    return ErrorStatus(EALREADY);
  }

  auto status = ProducerQueueChannel::Create(this, queue_id, producer_config,
                                             usage_policy);
  if (status) {
    message.SetChannel(status.take());
    return {{producer_config, queue_id}};
  } else {
    ALOGE("BufferHubService::OnCreateBuffer: Failed to create producer!!");
    return status.error_status();
  }
}

void BufferHubChannel::SignalAvailable() {
  ATRACE_NAME("BufferHubChannel::SignalAvailable");
  ALOGD_IF(TRACE,
           "BufferHubChannel::SignalAvailable: channel_id=%d buffer_id=%d",
           channel_id(), buffer_id());
  signaled_ = true;
  const auto status = service_->ModifyChannelEvents(channel_id_, 0, POLLIN);
  ALOGE_IF(!status,
           "BufferHubChannel::SignalAvailable: failed to signal availability "
           "channel_id=%d: %s",
           channel_id_, status.GetErrorMessage().c_str());
}

void BufferHubChannel::ClearAvailable() {
  ATRACE_NAME("BufferHubChannel::ClearAvailable");
  ALOGD_IF(TRACE,
           "BufferHubChannel::ClearAvailable: channel_id=%d buffer_id=%d",
           channel_id(), buffer_id());
  signaled_ = false;
  const auto status = service_->ModifyChannelEvents(channel_id_, POLLIN, 0);
  ALOGE_IF(!status,
           "BufferHubChannel::ClearAvailable: failed to clear availability "
           "channel_id=%d: %s",
           channel_id_, status.GetErrorMessage().c_str());
}

void BufferHubChannel::Hangup() {
  ATRACE_NAME("BufferHubChannel::Hangup");
  ALOGD_IF(TRACE, "BufferHubChannel::Hangup: channel_id=%d buffer_id=%d",
           channel_id(), buffer_id());
  const auto status = service_->ModifyChannelEvents(channel_id_, 0, POLLHUP);
  ALOGE_IF(
      !status,
      "BufferHubChannel::Hangup: failed to signal hangup channel_id=%d: %s",
      channel_id_, status.GetErrorMessage().c_str());
}

}  // namespace dvr
}  // namespace android
