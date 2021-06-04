#include "include/private/dvr/display_client.h"

#include <cutils/native_handle.h>
#include <log/log.h>
#include <pdx/default_transport/client_channel.h>
#include <pdx/default_transport/client_channel_factory.h>
#include <pdx/status.h>

#include <mutex>

#include <private/dvr/display_protocol.h>

using android::pdx::ErrorStatus;
using android::pdx::LocalHandle;
using android::pdx::LocalChannelHandle;
using android::pdx::Status;
using android::pdx::Transaction;
using android::pdx::rpc::IfAnyOf;

namespace android {
namespace dvr {
namespace display {

Surface::Surface(LocalChannelHandle channel_handle, int* error)
    : BASE{pdx::default_transport::ClientChannel::Create(
          std::move(channel_handle))} {
  auto status = InvokeRemoteMethod<DisplayProtocol::GetSurfaceInfo>();
  if (!status) {
    ALOGE("Surface::Surface: Failed to get surface info: %s",
          status.GetErrorMessage().c_str());
    Close(status.error());
    if (error)
      *error = status.error();
  }

  surface_id_ = status.get().surface_id;
  z_order_ = status.get().z_order;
  visible_ = status.get().visible;
}

Surface::Surface(const SurfaceAttributes& attributes, int* error)
    : BASE{pdx::default_transport::ClientChannelFactory::Create(
               DisplayProtocol::kClientPath),
           kInfiniteTimeout} {
  auto status = InvokeRemoteMethod<DisplayProtocol::CreateSurface>(attributes);
  if (!status) {
    ALOGE("Surface::Surface: Failed to create display surface: %s",
          status.GetErrorMessage().c_str());
    Close(status.error());
    if (error)
      *error = status.error();
  }

  surface_id_ = status.get().surface_id;
  z_order_ = status.get().z_order;
  visible_ = status.get().visible;
}

Status<void> Surface::SetVisible(bool visible) {
  return SetAttributes(
      {{SurfaceAttribute::Visible, SurfaceAttributeValue{visible}}});
}

Status<void> Surface::SetZOrder(int z_order) {
  return SetAttributes(
      {{SurfaceAttribute::ZOrder, SurfaceAttributeValue{z_order}}});
}

Status<void> Surface::SetAttributes(const SurfaceAttributes& attributes) {
  auto status = InvokeRemoteMethod<DisplayProtocol::SetAttributes>(attributes);
  if (!status) {
    ALOGE(
        "Surface::SetAttributes: Failed to set display surface "
        "attributes: %s",
        status.GetErrorMessage().c_str());
    return status.error_status();
  }

  // Set the local cached copies of the attributes we care about from the full
  // set of attributes sent to the display service.
  for (const auto& attribute : attributes) {
    const auto& key = attribute.first;
    const auto* variant = &attribute.second;
    bool invalid_value = false;
    switch (key) {
      case SurfaceAttribute::Visible:
        invalid_value =
            !IfAnyOf<int32_t, int64_t, bool>::Get(variant, &visible_);
        break;
      case SurfaceAttribute::ZOrder:
        invalid_value = !IfAnyOf<int32_t>::Get(variant, &z_order_);
        break;
    }

    if (invalid_value) {
      ALOGW(
          "Surface::SetAttributes: Failed to set display surface "
          "attribute %d because of incompatible type: %d",
          key, variant->index());
    }
  }

  return {};
}

Status<std::unique_ptr<ProducerQueue>> Surface::CreateQueue(
    uint32_t width, uint32_t height, uint32_t format, size_t metadata_size) {
  ALOGD_IF(TRACE, "Surface::CreateQueue: Creating empty queue.");
  auto status = InvokeRemoteMethod<DisplayProtocol::CreateQueue>(
      ProducerQueueConfigBuilder()
          .SetDefaultWidth(width)
          .SetDefaultHeight(height)
          .SetDefaultFormat(format)
          .SetMetadataSize(metadata_size)
          .Build());
  if (!status) {
    ALOGE("Surface::CreateQueue: Failed to create queue: %s",
          status.GetErrorMessage().c_str());
    return status.error_status();
  }

  auto producer_queue = ProducerQueue::Import(status.take());
  if (!producer_queue) {
    ALOGE("Surface::CreateQueue: Failed to import producer queue!");
    return ErrorStatus(ENOMEM);
  }

  return {std::move(producer_queue)};
}

Status<std::unique_ptr<ProducerQueue>> Surface::CreateQueue(
    uint32_t width, uint32_t height, uint32_t layer_count, uint32_t format,
    uint64_t usage, size_t capacity, size_t metadata_size) {
  ALOGD_IF(TRACE,
           "Surface::CreateQueue: width=%u height=%u layer_count=%u format=%u "
           "usage=%" PRIx64 " capacity=%zu",
           width, height, layer_count, format, usage, capacity);
  auto status = CreateQueue(width, height, format, metadata_size);
  if (!status)
    return status.error_status();

  auto producer_queue = status.take();

  ALOGD_IF(TRACE, "Surface::CreateQueue: Allocating %zu buffers...", capacity);
  auto allocate_status = producer_queue->AllocateBuffers(
      width, height, layer_count, format, usage, capacity);
  if (!allocate_status) {
    ALOGE("Surface::CreateQueue: Failed to allocate buffer on queue_id=%d: %s",
          producer_queue->id(), allocate_status.GetErrorMessage().c_str());
    return allocate_status.error_status();
  }

  return {std::move(producer_queue)};
}

DisplayClient::DisplayClient(int* error)
    : BASE(pdx::default_transport::ClientChannelFactory::Create(
               DisplayProtocol::kClientPath),
           kInfiniteTimeout) {
  if (error)
    *error = Client::error();
}

Status<Metrics> DisplayClient::GetDisplayMetrics() {
  return InvokeRemoteMethod<DisplayProtocol::GetMetrics>();
}

Status<std::string> DisplayClient::GetConfigurationData(
    ConfigFileType config_type) {
  auto status =
      InvokeRemoteMethod<DisplayProtocol::GetConfigurationData>(config_type);
  if (!status && status.error() != ENOENT) {
    ALOGE(
        "DisplayClient::GetConfigurationData: Unable to get"
        "configuration data. Error: %s",
        status.GetErrorMessage().c_str());
  }
  return status;
}

Status<std::unique_ptr<Surface>> DisplayClient::CreateSurface(
    const SurfaceAttributes& attributes) {
  int error;
  if (auto client = Surface::Create(attributes, &error))
    return {std::move(client)};
  else
    return ErrorStatus(error);
}

pdx::Status<std::unique_ptr<IonBuffer>> DisplayClient::SetupGlobalBuffer(
    DvrGlobalBufferKey key, size_t size, uint64_t usage) {
  auto status =
      InvokeRemoteMethod<DisplayProtocol::SetupGlobalBuffer>(key, size, usage);
  if (!status) {
    ALOGE(
        "DisplayClient::SetupGlobalBuffer: Failed to create the global buffer "
        "%s",
        status.GetErrorMessage().c_str());
    return status.error_status();
  }

  auto ion_buffer = std::make_unique<IonBuffer>();
  auto native_buffer_handle = status.take();
  const int ret = native_buffer_handle.Import(ion_buffer.get());
  if (ret < 0) {
    ALOGE(
        "DisplayClient::GetGlobalBuffer: Failed to import global buffer: "
        "key=%d; error=%s",
        key, strerror(-ret));
    return ErrorStatus(-ret);
  }

  return {std::move(ion_buffer)};
}

pdx::Status<void> DisplayClient::DeleteGlobalBuffer(DvrGlobalBufferKey key) {
  auto status = InvokeRemoteMethod<DisplayProtocol::DeleteGlobalBuffer>(key);
  if (!status) {
    ALOGE("DisplayClient::DeleteGlobalBuffer Failed: %s",
          status.GetErrorMessage().c_str());
  }

  return status;
}

Status<std::unique_ptr<IonBuffer>> DisplayClient::GetGlobalBuffer(
    DvrGlobalBufferKey key) {
  auto status = InvokeRemoteMethod<DisplayProtocol::GetGlobalBuffer>(key);
  if (!status) {
    ALOGE(
        "DisplayClient::GetGlobalBuffer: Failed to get named buffer: key=%d; "
        "error=%s",
        key, status.GetErrorMessage().c_str());
    return status.error_status();
  }

  auto ion_buffer = std::make_unique<IonBuffer>();
  auto native_buffer_handle = status.take();
  const int ret = native_buffer_handle.Import(ion_buffer.get());
  if (ret < 0) {
    ALOGE(
        "DisplayClient::GetGlobalBuffer: Failed to import global buffer: "
        "key=%d; error=%s",
        key, strerror(-ret));
    return ErrorStatus(-ret);
  }

  return {std::move(ion_buffer)};
}

Status<bool> DisplayClient::IsVrAppRunning() {
  return InvokeRemoteMethod<DisplayProtocol::IsVrAppRunning>();
}

}  // namespace display
}  // namespace dvr
}  // namespace android
