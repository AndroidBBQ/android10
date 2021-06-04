#include "display_service.h"

#include <unistd.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <android-base/file.h>
#include <android-base/properties.h>
#include <dvr/dvr_display_types.h>
#include <pdx/default_transport/service_endpoint.h>
#include <pdx/rpc/remote_method.h>
#include <private/android_filesystem_config.h>
#include <private/dvr/display_protocol.h>
#include <private/dvr/numeric.h>
#include <private/dvr/trusted_uids.h>
#include <private/dvr/types.h>

using android::dvr::display::DisplayProtocol;
using android::pdx::Channel;
using android::pdx::ErrorStatus;
using android::pdx::Message;
using android::pdx::Status;
using android::pdx::default_transport::Endpoint;
using android::pdx::rpc::DispatchRemoteMethod;

namespace {

const char kDvrLensMetricsProperty[] = "ro.dvr.lens_metrics";
const char kDvrDeviceMetricsProperty[] = "ro.dvr.device_metrics";
const char kDvrDeviceConfigProperty[] = "ro.dvr.device_configuration";

}  // namespace

namespace android {
namespace dvr {

DisplayService::DisplayService(Hwc2::Composer* hidl,
                               hwc2_display_t primary_display_id,
                               RequestDisplayCallback request_display_callback)
    : BASE("DisplayService",
           Endpoint::Create(display::DisplayProtocol::kClientPath)) {
    hardware_composer_.Initialize(
        hidl, primary_display_id, request_display_callback);
}

bool DisplayService::IsInitialized() const {
  return BASE::IsInitialized() && hardware_composer_.IsInitialized();
}

std::string DisplayService::DumpState(size_t /*max_length*/) {
  std::ostringstream stream;

  auto surfaces = GetDisplaySurfaces();
  std::sort(surfaces.begin(), surfaces.end(), [](const auto& a, const auto& b) {
    return a->surface_id() < b->surface_id();
  });

  stream << "Application Surfaces:" << std::endl;

  size_t count = 0;
  for (const auto& surface : surfaces) {
    if (surface->surface_type() == SurfaceType::Application) {
      stream << "Surface " << count++ << ":";
      stream << " surface_id=" << surface->surface_id()
             << " process_id=" << surface->process_id()
             << " user_id=" << surface->user_id()
             << " visible=" << surface->visible()
             << " z_order=" << surface->z_order();

      stream << " queue_ids=";
      auto queue_ids = surface->GetQueueIds();
      std::sort(queue_ids.begin(), queue_ids.end());
      for (int32_t id : queue_ids) {
        if (id != queue_ids[0])
          stream << ",";
        stream << id;
      }
      stream << std::endl;
    }
  }
  stream << std::endl;

  stream << "Direct Surfaces:" << std::endl;

  count = 0;
  for (const auto& surface : surfaces) {
    if (surface->surface_type() == SurfaceType::Direct) {
      stream << "Surface " << count++ << ":";
      stream << " surface_id=" << surface->surface_id()
             << " process_id=" << surface->process_id()
             << " user_id=" << surface->user_id()
             << " visible=" << surface->visible()
             << " z_order=" << surface->z_order();

      stream << " queue_ids=";
      auto queue_ids = surface->GetQueueIds();
      std::sort(queue_ids.begin(), queue_ids.end());
      for (int32_t id : queue_ids) {
        if (id != queue_ids[0])
          stream << ",";
        stream << id;
      }
      stream << std::endl;
    }
  }
  stream << std::endl;

  stream << hardware_composer_.Dump();
  return stream.str();
}

void DisplayService::OnChannelClose(pdx::Message& message,
                                    const std::shared_ptr<Channel>& channel) {
  if (auto surface = std::static_pointer_cast<DisplaySurface>(channel)) {
    surface->OnSetAttributes(message,
                             {{display::SurfaceAttribute::Visible,
                               display::SurfaceAttributeValue{false}}});
  }
}

// First-level dispatch for display service messages. Directly handles messages
// that are independent of the display surface (metrics, creation) and routes
// surface-specific messages to the per-instance handlers.
Status<void> DisplayService::HandleMessage(pdx::Message& message) {
  ALOGD_IF(TRACE, "DisplayService::HandleMessage: opcode=%d", message.GetOp());
  ATRACE_NAME("DisplayService::HandleMessage");

  switch (message.GetOp()) {
    case DisplayProtocol::GetMetrics::Opcode:
      DispatchRemoteMethod<DisplayProtocol::GetMetrics>(
          *this, &DisplayService::OnGetMetrics, message);
      return {};

    case DisplayProtocol::GetConfigurationData::Opcode:
      DispatchRemoteMethod<DisplayProtocol::GetConfigurationData>(
          *this, &DisplayService::OnGetConfigurationData, message);
      return {};

    case DisplayProtocol::CreateSurface::Opcode:
      DispatchRemoteMethod<DisplayProtocol::CreateSurface>(
          *this, &DisplayService::OnCreateSurface, message);
      return {};

    case DisplayProtocol::SetupGlobalBuffer::Opcode:
      DispatchRemoteMethod<DisplayProtocol::SetupGlobalBuffer>(
          *this, &DisplayService::OnSetupGlobalBuffer, message);
      return {};

    case DisplayProtocol::DeleteGlobalBuffer::Opcode:
      DispatchRemoteMethod<DisplayProtocol::DeleteGlobalBuffer>(
          *this, &DisplayService::OnDeleteGlobalBuffer, message);
      return {};

    case DisplayProtocol::GetGlobalBuffer::Opcode:
      DispatchRemoteMethod<DisplayProtocol::GetGlobalBuffer>(
          *this, &DisplayService::OnGetGlobalBuffer, message);
      return {};

    case DisplayProtocol::IsVrAppRunning::Opcode:
      DispatchRemoteMethod<DisplayProtocol::IsVrAppRunning>(
          *this, &DisplayService::IsVrAppRunning, message);
      return {};

    // Direct the surface specific messages to the surface instance.
    case DisplayProtocol::SetAttributes::Opcode:
    case DisplayProtocol::CreateQueue::Opcode:
    case DisplayProtocol::GetSurfaceInfo::Opcode:
      return HandleSurfaceMessage(message);

    default:
      return Service::HandleMessage(message);
  }
}

Status<display::Metrics> DisplayService::OnGetMetrics(
    pdx::Message& /*message*/) {
  const auto& params = hardware_composer_.GetPrimaryDisplayParams();
  return {{static_cast<uint32_t>(params.width),
           static_cast<uint32_t>(params.height),
           static_cast<uint32_t>(params.dpi.x),
           static_cast<uint32_t>(params.dpi.y),
           static_cast<uint32_t>(params.vsync_period_ns),
           0,
           0,
           0,
           0.0,
           {},
           {}}};
}

pdx::Status<std::string> DisplayService::OnGetConfigurationData(
    pdx::Message& /*message*/, display::ConfigFileType config_type) {
  std::string property_name;
  switch (config_type) {
    case display::ConfigFileType::kLensMetrics:
      property_name = kDvrLensMetricsProperty;
      break;
    case display::ConfigFileType::kDeviceMetrics:
      property_name = kDvrDeviceMetricsProperty;
      break;
    case display::ConfigFileType::kDeviceConfiguration:
      property_name = kDvrDeviceConfigProperty;
      break;
    default:
      return ErrorStatus(EINVAL);
  }
  std::string file_path = base::GetProperty(property_name, "");
  if (file_path.empty()) {
    return ErrorStatus(ENOENT);
  }

  std::string data;
  if (!base::ReadFileToString(file_path, &data)) {
    return ErrorStatus(errno);
  }

  return std::move(data);
}

// Creates a new DisplaySurface and associates it with this channel. This may
// only be done once per channel.
Status<display::SurfaceInfo> DisplayService::OnCreateSurface(
    pdx::Message& message, const display::SurfaceAttributes& attributes) {
  // A surface may only be created once per channel.
  if (message.GetChannel())
    return ErrorStatus(EINVAL);

  ALOGI_IF(TRACE, "DisplayService::OnCreateSurface: cid=%d",
           message.GetChannelId());

  // Use the channel id as the unique surface id.
  const int surface_id = message.GetChannelId();
  const int process_id = message.GetProcessId();
  const int user_id = message.GetEffectiveUserId();

  ALOGI_IF(TRACE,
           "DisplayService::OnCreateSurface: surface_id=%d process_id=%d",
           surface_id, process_id);

  auto surface_status =
      DisplaySurface::Create(this, surface_id, process_id, user_id, attributes);
  if (!surface_status) {
    ALOGE("DisplayService::OnCreateSurface: Failed to create surface: %s",
          surface_status.GetErrorMessage().c_str());
    return ErrorStatus(surface_status.error());
  }
  auto surface = surface_status.take();
  message.SetChannel(surface);

  // Update the surface with the attributes supplied with the create call. For
  // application surfaces this has the side effect of notifying the display
  // manager of the new surface. For direct surfaces, this may trigger a mode
  // change, depending on the value of the visible attribute.
  surface->OnSetAttributes(message, attributes);

  return {{surface->surface_id(), surface->visible(), surface->z_order()}};
}

void DisplayService::SurfaceUpdated(SurfaceType surface_type,
                                    display::SurfaceUpdateFlags update_flags) {
  ALOGD_IF(TRACE, "DisplayService::SurfaceUpdated: update_flags=%x",
           update_flags.value());
  if (update_flags.value() != 0) {
    if (surface_type == SurfaceType::Application)
      NotifyDisplayConfigurationUpdate();
    else
      UpdateActiveDisplaySurfaces();
  }
}

pdx::Status<BorrowedNativeBufferHandle> DisplayService::OnSetupGlobalBuffer(
    pdx::Message& message, DvrGlobalBufferKey key, size_t size,
    uint64_t usage) {
  const int user_id = message.GetEffectiveUserId();
  const bool trusted = user_id == AID_ROOT || IsTrustedUid(user_id);

  if (!trusted) {
    ALOGE(
        "DisplayService::OnSetupGlobalBuffer: Permission denied for user_id=%d",
        user_id);
    return ErrorStatus(EPERM);
  }
  return SetupGlobalBuffer(key, size, usage);
}

pdx::Status<void> DisplayService::OnDeleteGlobalBuffer(pdx::Message& message,
                                                       DvrGlobalBufferKey key) {
  const int user_id = message.GetEffectiveUserId();
  const bool trusted = (user_id == AID_ROOT) || IsTrustedUid(user_id);

  if (!trusted) {
    ALOGE(
        "DisplayService::OnDeleteGlobalBuffer: Permission denied for "
        "user_id=%d",
        user_id);
    return ErrorStatus(EPERM);
  }
  return DeleteGlobalBuffer(key);
}

pdx::Status<BorrowedNativeBufferHandle> DisplayService::OnGetGlobalBuffer(
    pdx::Message& /* message */, DvrGlobalBufferKey key) {
  ALOGD_IF(TRACE, "DisplayService::OnGetGlobalBuffer: key=%d", key);
  auto global_buffer = global_buffers_.find(key);
  if (global_buffer != global_buffers_.end())
    return {BorrowedNativeBufferHandle(*global_buffer->second, 0)};
  else
    return pdx::ErrorStatus(EINVAL);
}

// Calls the message handler for the DisplaySurface associated with this
// channel.
Status<void> DisplayService::HandleSurfaceMessage(pdx::Message& message) {
  auto surface = std::static_pointer_cast<DisplaySurface>(message.GetChannel());
  ALOGW_IF(!surface,
           "DisplayService::HandleSurfaceMessage: surface is nullptr!");

  if (surface)
    return surface->HandleMessage(message);
  else
    return ErrorStatus(EINVAL);
}

std::shared_ptr<DisplaySurface> DisplayService::GetDisplaySurface(
    int surface_id) const {
  return std::static_pointer_cast<DisplaySurface>(GetChannel(surface_id));
}

std::vector<std::shared_ptr<DisplaySurface>>
DisplayService::GetDisplaySurfaces() const {
  return GetChannels<DisplaySurface>();
}

std::vector<std::shared_ptr<DirectDisplaySurface>>
DisplayService::GetVisibleDisplaySurfaces() const {
  std::vector<std::shared_ptr<DirectDisplaySurface>> visible_surfaces;

  ForEachDisplaySurface(
      SurfaceType::Direct,
      [&](const std::shared_ptr<DisplaySurface>& surface) mutable {
        if (surface->visible()) {
          visible_surfaces.push_back(
              std::static_pointer_cast<DirectDisplaySurface>(surface));
          surface->ClearUpdate();
        }
      });

  return visible_surfaces;
}

void DisplayService::UpdateActiveDisplaySurfaces() {
  auto visible_surfaces = GetVisibleDisplaySurfaces();
  ALOGD_IF(TRACE,
           "DisplayService::UpdateActiveDisplaySurfaces: %zd visible surfaces",
           visible_surfaces.size());
  hardware_composer_.SetDisplaySurfaces(std::move(visible_surfaces));
}

pdx::Status<BorrowedNativeBufferHandle> DisplayService::SetupGlobalBuffer(
    DvrGlobalBufferKey key, size_t size, uint64_t usage) {
  auto global_buffer = global_buffers_.find(key);
  if (global_buffer == global_buffers_.end()) {
    auto ion_buffer = std::make_unique<IonBuffer>(static_cast<int>(size), 1,
                                                  HAL_PIXEL_FORMAT_BLOB, usage);

    // Some buffers are used internally. If they were configured with an
    // invalid size or format, this will fail.
    int result = hardware_composer_.OnNewGlobalBuffer(key, *ion_buffer.get());
    if (result < 0)
      return ErrorStatus(result);
    global_buffer =
        global_buffers_.insert(std::make_pair(key, std::move(ion_buffer)))
            .first;
  }

  return {BorrowedNativeBufferHandle(*global_buffer->second, 0)};
}

pdx::Status<void> DisplayService::DeleteGlobalBuffer(DvrGlobalBufferKey key) {
  auto global_buffer = global_buffers_.find(key);
  if (global_buffer != global_buffers_.end()) {
    // Some buffers are used internally.
    hardware_composer_.OnDeletedGlobalBuffer(key);
    global_buffers_.erase(global_buffer);
  }

  return {0};
}

void DisplayService::SetDisplayConfigurationUpdateNotifier(
    DisplayConfigurationUpdateNotifier update_notifier) {
  update_notifier_ = update_notifier;
}

void DisplayService::NotifyDisplayConfigurationUpdate() {
  if (update_notifier_)
    update_notifier_();
}

Status<bool> DisplayService::IsVrAppRunning(pdx::Message& /*message*/) {
  bool visible = false;
  ForEachDisplaySurface(
      SurfaceType::Application,
      [&visible](const std::shared_ptr<DisplaySurface>& surface) {
        if (surface->visible())
          visible = true;
      });

  return {visible};
}

}  // namespace dvr
}  // namespace android
