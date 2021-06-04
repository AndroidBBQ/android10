/*
 * Copyright 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "impl/vr_hwc.h"

#include "android-base/stringprintf.h"
#include <binder/IServiceManager.h>
#include <cutils/properties.h>
#include <private/dvr/display_client.h>
#include <ui/Fence.h>
#include <utils/Trace.h>

#include <mutex>

#include "vr_composer_client.h"

using namespace android::hardware::graphics::common::V1_0;
using namespace android::hardware::graphics::composer::V2_1;

using android::base::StringPrintf;
using android::hardware::hidl_handle;
using android::hardware::hidl_string;
using android::hardware::hidl_vec;
using android::hardware::Return;
using android::hardware::Void;

namespace android {
namespace dvr {
namespace {

using android::hardware::graphics::common::V1_0::PixelFormat;

const Display kDefaultDisplayId = 1;
const Config kDefaultConfigId = 1;

sp<GraphicBuffer> CreateGraphicBuffer(
    const native_handle_t* handle,
    const IVrComposerClient::BufferMetadata& metadata) {
   sp<GraphicBuffer> buffer = new GraphicBuffer(
      handle, GraphicBuffer::CLONE_HANDLE, metadata.width, metadata.height,
      static_cast<int32_t>(metadata.format), metadata.layerCount,
      metadata.usage, metadata.stride);
   if (buffer->initCheck() != OK) {
     ALOGE("Failed to create graphic buffer");
     return nullptr;
   }

   return buffer;
}

void GetPrimaryDisplaySize(int32_t* width, int32_t* height) {
  *width = 1080;
  *height = 1920;

  int error = 0;
  auto display_client = display::DisplayClient::Create(&error);
  if (!display_client) {
    ALOGE("Could not connect to display service : %s(%d)", strerror(error),
          error);
    return;
  }

  auto status = display_client->GetDisplayMetrics();
  if (!status) {
    ALOGE("Could not get display metrics from display service : %s(%d)",
          status.GetErrorMessage().c_str(), status.error());
    return;
  }

  *width = status.get().display_width;
  *height = status.get().display_height;
}

}  // namespace

HwcDisplay::HwcDisplay(int32_t width, int32_t height)
    : width_(width), height_(height) {}

HwcDisplay::~HwcDisplay() {}

bool HwcDisplay::SetClientTarget(const native_handle_t* handle,
                                 base::unique_fd fence) {
  if (handle)
    buffer_ = CreateGraphicBuffer(handle, buffer_metadata_);

  fence_ = new Fence(fence.release());
  return true;
}

void HwcDisplay::SetClientTargetMetadata(
    const IVrComposerClient::BufferMetadata& metadata) {
  buffer_metadata_ = metadata;
}

HwcLayer* HwcDisplay::CreateLayer() {
  uint64_t layer_id = layer_ids_++;
  layers_.push_back(HwcLayer(layer_id));
  return &layers_.back();
}

HwcLayer* HwcDisplay::GetLayer(Layer id) {
  for (size_t i = 0; i < layers_.size(); ++i)
    if (layers_[i].info.id == id)
      return &layers_[i];

  return nullptr;
}

bool HwcDisplay::DestroyLayer(Layer id) {
  for (auto it = layers_.begin(); it != layers_.end(); ++it) {
    if (it->info.id == id) {
      layers_.erase(it);
      return true;
    }
  }

  return false;
}

void HwcDisplay::GetChangedCompositionTypes(
    std::vector<Layer>* layer_ids,
    std::vector<IComposerClient::Composition>* types) {
  std::sort(layers_.begin(), layers_.end(),
            [](const auto& lhs, const auto& rhs) {
              return lhs.info.z_order < rhs.info.z_order;
            });

  const size_t no_layer = std::numeric_limits<size_t>::max();
  size_t first_client_layer = no_layer, last_client_layer = no_layer;
  for (size_t i = 0; i < layers_.size(); ++i) {
    switch (layers_[i].composition_type) {
      case IComposerClient::Composition::SOLID_COLOR:
      case IComposerClient::Composition::CURSOR:
      case IComposerClient::Composition::SIDEBAND:
        if (first_client_layer == no_layer)
          first_client_layer = i;

        last_client_layer = i;
        break;
      default:
        break;
    }
  }

  for (size_t i = 0; i < layers_.size(); ++i) {
    if (i >= first_client_layer && i <= last_client_layer) {
      if (layers_[i].composition_type != IComposerClient::Composition::CLIENT) {
        layer_ids->push_back(layers_[i].info.id);
        types->push_back(IComposerClient::Composition::CLIENT);
        layers_[i].composition_type = IComposerClient::Composition::CLIENT;
      }

      continue;
    }

    if (layers_[i].composition_type != IComposerClient::Composition::DEVICE) {
      layer_ids->push_back(layers_[i].info.id);
      types->push_back(IComposerClient::Composition::DEVICE);
      layers_[i].composition_type = IComposerClient::Composition::DEVICE;
    }
  }
}

Error HwcDisplay::GetFrame(
    std::vector<ComposerView::ComposerLayer>* out_frames) {
  bool queued_client_target = false;
  std::vector<ComposerView::ComposerLayer> frame;
  for (const auto& layer : layers_) {
    if (layer.composition_type == IComposerClient::Composition::CLIENT) {
      if (queued_client_target)
        continue;

      if (!buffer_.get()) {
        ALOGE("Client composition requested but no client target buffer");
        return Error::BAD_LAYER;
      }

      ComposerView::ComposerLayer client_target_layer = {
          .buffer = buffer_,
          .fence = fence_.get() ? fence_ : new Fence(-1),
          .display_frame = {0, 0, static_cast<int32_t>(buffer_->getWidth()),
            static_cast<int32_t>(buffer_->getHeight())},
          .crop = {0.0f, 0.0f, static_cast<float>(buffer_->getWidth()),
            static_cast<float>(buffer_->getHeight())},
          .blend_mode = IComposerClient::BlendMode::NONE,
      };

      frame.push_back(client_target_layer);
      queued_client_target = true;
    } else {
      if (!layer.info.buffer.get() || !layer.info.fence.get()) {
        ALOGV("Layer requested without valid buffer");
        continue;
      }

      frame.push_back(layer.info);
    }
  }

  out_frames->swap(frame);
  return Error::NONE;
}

std::vector<Layer> HwcDisplay::UpdateLastFrameAndGetLastFrameLayers() {
  std::vector<Layer> last_frame_layers;
  last_frame_layers.swap(last_frame_layers_ids_);

  for (const auto& layer : layers_)
    last_frame_layers_ids_.push_back(layer.info.id);

  return last_frame_layers;
}

void HwcDisplay::SetColorTransform(const float* matrix, int32_t hint) {
  color_transform_hint_ = hint;
  if (matrix)
    memcpy(color_transform_, matrix, sizeof(color_transform_));
}

void HwcDisplay::dumpDebugInfo(std::string* result) const {
  if (!result) {
    return;
  }
  *result += StringPrintf("HwcDisplay: width: %d, height: %d, layers size: %zu, colormode: %d\
      , config: %d\n", width_, height_, layers_.size(), color_mode_, active_config_);
  *result += StringPrintf("HwcDisplay buffer metadata: width: %d, height: %d, stride: %d,\
      layerCount: %d, pixelFormat: %d\n", buffer_metadata_.width, buffer_metadata_.height,
      buffer_metadata_.stride, buffer_metadata_.layerCount, buffer_metadata_.format);
  for (const auto& layer : layers_) {
    layer.dumpDebugInfo(result);
  }
}

////////////////////////////////////////////////////////////////////////////////
// VrHwcClient

VrHwc::VrHwc() {
  vsync_callback_ = new VsyncCallback;
}

VrHwc::~VrHwc() {
  vsync_callback_->SetEventCallback(nullptr);
}

bool VrHwc::hasCapability(hwc2_capability_t /* capability */) { return false; }

void VrHwc::registerEventCallback(EventCallback* callback) {
  std::unique_lock<std::mutex> lock(mutex_);
  event_callback_ = callback;
  int32_t width, height;
  GetPrimaryDisplaySize(&width, &height);
  // Create the primary display late to avoid initialization issues between
  // VR HWC and SurfaceFlinger.
  displays_[kDefaultDisplayId].reset(new HwcDisplay(width, height));

  // Surface flinger will make calls back into vr_hwc when it receives the
  // onHotplug() call, so it's important to release mutex_ here.
  lock.unlock();
  event_callback_->onHotplug(kDefaultDisplayId,
                             IComposerCallback::Connection::CONNECTED);
  lock.lock();
  UpdateVsyncCallbackEnabledLocked();
}

void VrHwc::unregisterEventCallback() {
  std::lock_guard<std::mutex> guard(mutex_);
  event_callback_ = nullptr;
  UpdateVsyncCallbackEnabledLocked();
}

uint32_t VrHwc::getMaxVirtualDisplayCount() { return 1; }

Error VrHwc::createVirtualDisplay(uint32_t width, uint32_t height,
                                  PixelFormat* format, Display* outDisplay) {
  *format = PixelFormat::RGBA_8888;
  *outDisplay = display_count_;
  displays_[display_count_].reset(new HwcDisplay(width, height));
  display_count_++;
  return Error::NONE;
}

Error VrHwc::destroyVirtualDisplay(Display display) {
  std::lock_guard<std::mutex> guard(mutex_);
  if (display == kDefaultDisplayId || displays_.erase(display) == 0)
    return Error::BAD_DISPLAY;
  ComposerView::Frame frame;
  frame.display_id = display;
  frame.removed = true;
  if (observer_)
    observer_->OnNewFrame(frame);
  return Error::NONE;
}

Error VrHwc::createLayer(Display display, Layer* outLayer) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* layer = display_ptr->CreateLayer();
  *outLayer = layer->info.id;
  return Error::NONE;
}

Error VrHwc::destroyLayer(Display display, Layer layer) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr) {
    return Error::BAD_DISPLAY;
  }

  return display_ptr->DestroyLayer(layer) ? Error::NONE : Error::BAD_LAYER;
}

Error VrHwc::getActiveConfig(Display display, Config* outConfig) {
  std::lock_guard<std::mutex> guard(mutex_);
  if (!FindDisplay(display))
    return Error::BAD_DISPLAY;
  *outConfig = kDefaultConfigId;
  return Error::NONE;
}

Error VrHwc::getClientTargetSupport(Display display, uint32_t /* width */,
                                    uint32_t /* height */,
                                    PixelFormat /* format */,
                                    Dataspace /* dataspace */) {
  std::lock_guard<std::mutex> guard(mutex_);
  if (!FindDisplay(display))
    return Error::BAD_DISPLAY;

  return Error::NONE;
}

Error VrHwc::getColorModes(Display /* display */,
                           hidl_vec<ColorMode>* outModes) {
  std::vector<ColorMode> color_modes(1, ColorMode::NATIVE);
  *outModes = hidl_vec<ColorMode>(color_modes);
  return Error::NONE;
}

Error VrHwc::getDisplayAttribute(Display display, Config config,
                                 IComposerClient::Attribute attribute,
                                 int32_t* outValue) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr) {
    return Error::BAD_DISPLAY;
  }
  if (config != kDefaultConfigId) {
    return Error::BAD_CONFIG;
  }

  switch (attribute) {
    case IComposerClient::Attribute::WIDTH:
      *outValue = display_ptr->width();
      break;
    case IComposerClient::Attribute::HEIGHT:
      *outValue = display_ptr->height();
      break;
    case IComposerClient::Attribute::VSYNC_PERIOD:
      {
        int error = 0;
        auto display_client = display::DisplayClient::Create(&error);
        if (!display_client) {
          ALOGE("Could not connect to display service : %s(%d)",
                strerror(error), error);
          // Return a default value of 30 fps
          *outValue = 1000 * 1000 * 1000 / 30;
        } else {
          auto metrics = display_client->GetDisplayMetrics();
          *outValue = metrics.get().vsync_period_ns;
        }
      }
      break;
    case IComposerClient::Attribute::DPI_X:
    case IComposerClient::Attribute::DPI_Y:
      {
        constexpr int32_t kDefaultDPI = 300;
        int32_t dpi = property_get_int32("ro.vr.hwc.dpi", kDefaultDPI);
        if (dpi <= 0) {
          dpi = kDefaultDPI;
        }
        *outValue = 1000 * dpi;
      }
      break;
    default:
      return Error::BAD_PARAMETER;
  }

  return Error::NONE;
}

Error VrHwc::getDisplayConfigs(Display display, hidl_vec<Config>* outConfigs) {
  std::lock_guard<std::mutex> guard(mutex_);
  if (!FindDisplay(display))
    return Error::BAD_DISPLAY;
  std::vector<Config> configs(1, kDefaultConfigId);
  *outConfigs = hidl_vec<Config>(configs);
  return Error::NONE;
}

Error VrHwc::getDisplayName(Display /* display */, hidl_string* outName) {
  *outName = hidl_string();
  return Error::NONE;
}

Error VrHwc::getDisplayType(Display display,
                            IComposerClient::DisplayType* outType) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr) {
    *outType = IComposerClient::DisplayType::INVALID;
    return Error::BAD_DISPLAY;
  }

  if (display == kDefaultDisplayId)
    *outType = IComposerClient::DisplayType::PHYSICAL;
  else
    *outType = IComposerClient::DisplayType::VIRTUAL;

  return Error::NONE;
}

Error VrHwc::getDozeSupport(Display display, bool* outSupport) {
  *outSupport = false;
  std::lock_guard<std::mutex> guard(mutex_);
  if (!FindDisplay(display))
    return Error::BAD_DISPLAY;
  return Error::NONE;
}

Error VrHwc::getHdrCapabilities(Display /* display */,
                                hidl_vec<Hdr>* /* outTypes */,
                                float* outMaxLuminance,
                                float* outMaxAverageLuminance,
                                float* outMinLuminance) {
  *outMaxLuminance = 0;
  *outMaxAverageLuminance = 0;
  *outMinLuminance = 0;
  return Error::NONE;
}

Error VrHwc::setActiveConfig(Display display, Config config) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;
  if (config != kDefaultConfigId)
    return Error::BAD_CONFIG;

  display_ptr->set_active_config(config);
  return Error::NONE;
}

Error VrHwc::setColorMode(Display display, ColorMode mode) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  if (mode < ColorMode::NATIVE || mode > ColorMode::DISPLAY_P3)
    return Error::BAD_PARAMETER;

  display_ptr->set_color_mode(mode);
  return Error::NONE;
}

Error VrHwc::setPowerMode(Display display, IComposerClient::PowerMode mode) {
  bool dozeSupported = false;

  Error dozeSupportError = getDozeSupport(display, &dozeSupported);

  if (dozeSupportError != Error::NONE)
    return dozeSupportError;

  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  if (mode < IComposerClient::PowerMode::OFF ||
      mode > IComposerClient::PowerMode::DOZE_SUSPEND) {
    return Error::BAD_PARAMETER;
  }

  if (!dozeSupported &&
      (mode == IComposerClient::PowerMode::DOZE ||
       mode == IComposerClient::PowerMode::DOZE_SUSPEND)) {
    return Error::UNSUPPORTED;
  }

  display_ptr->set_power_mode(mode);
  return Error::NONE;
}

Error VrHwc::setVsyncEnabled(Display display, IComposerClient::Vsync enabled) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  if (enabled != IComposerClient::Vsync::ENABLE &&
      enabled != IComposerClient::Vsync::DISABLE) {
    return Error::BAD_PARAMETER;
  }

  Error set_vsync_result = Error::NONE;
  if (display == kDefaultDisplayId) {
    sp<IVsyncService> vsync_service = interface_cast<IVsyncService>(
        defaultServiceManager()->getService(
            String16(IVsyncService::GetServiceName())));
    if (vsync_service == nullptr) {
      ALOGE("Failed to get vsync service");
      return Error::NO_RESOURCES;
    }

    if (enabled == IComposerClient::Vsync::ENABLE) {
      ALOGI("Enable vsync");
      display_ptr->set_vsync_enabled(true);
      status_t result = vsync_service->registerCallback(vsync_callback_);
      if (result != OK) {
        ALOGE("%s service registerCallback() failed: %s (%d)",
            IVsyncService::GetServiceName(), strerror(-result), result);
        set_vsync_result = Error::NO_RESOURCES;
      }
    } else if (enabled == IComposerClient::Vsync::DISABLE) {
      ALOGI("Disable vsync");
      display_ptr->set_vsync_enabled(false);
      status_t result = vsync_service->unregisterCallback(vsync_callback_);
      if (result != OK) {
        ALOGE("%s service unregisterCallback() failed: %s (%d)",
            IVsyncService::GetServiceName(), strerror(-result), result);
        set_vsync_result = Error::NO_RESOURCES;
      }
    }

    UpdateVsyncCallbackEnabledLocked();
  }

  return set_vsync_result;
}

Error VrHwc::setColorTransform(Display display, const float* matrix,
                               int32_t hint) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  display_ptr->SetColorTransform(matrix, hint);
  return Error::NONE;
}

Error VrHwc::setClientTarget(Display display, buffer_handle_t target,
                             int32_t acquireFence, int32_t /* dataspace */,
                             const std::vector<hwc_rect_t>& /* damage */) {
  base::unique_fd fence(acquireFence);
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  if (target == nullptr)
    return Error::NONE;

  if (!display_ptr->SetClientTarget(target, std::move(fence)))
    return Error::BAD_PARAMETER;

  return Error::NONE;
}

Error VrHwc::setOutputBuffer(Display display, buffer_handle_t /* buffer */,
                             int32_t releaseFence) {
  base::unique_fd fence(releaseFence);
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  // TODO(dnicoara): Is it necessary to do anything here?
  return Error::NONE;
}

Error VrHwc::validateDisplay(
    Display display, std::vector<Layer>* outChangedLayers,
    std::vector<IComposerClient::Composition>* outCompositionTypes,
    uint32_t* /* outDisplayRequestMask */,
    std::vector<Layer>* /* outRequestedLayers */,
    std::vector<uint32_t>* /* outRequestMasks */) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  display_ptr->GetChangedCompositionTypes(outChangedLayers,
                                          outCompositionTypes);
  return Error::NONE;
}

Error VrHwc::acceptDisplayChanges(Display /* display */) { return Error::NONE; }

Error VrHwc::presentDisplay(Display display, int32_t* outPresentFence,
                            std::vector<Layer>* outLayers,
                            std::vector<int32_t>* outReleaseFences) {
  *outPresentFence = -1;
  outLayers->clear();
  outReleaseFences->clear();

  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);

  if (!display_ptr)
    return Error::BAD_DISPLAY;

  ComposerView::Frame frame;
  std::vector<Layer> last_frame_layers;
  Error status = display_ptr->GetFrame(&frame.layers);
  frame.display_id = display;
  frame.display_width = display_ptr->width();
  frame.display_height = display_ptr->height();
  frame.active_config = display_ptr->active_config();
  frame.power_mode = display_ptr->power_mode();
  frame.vsync_enabled = display_ptr->vsync_enabled() ?
      IComposerClient::Vsync::ENABLE : IComposerClient::Vsync::DISABLE;
  frame.color_transform_hint = display_ptr->color_transform_hint();
  frame.color_mode = display_ptr->color_mode();
  memcpy(frame.color_transform, display_ptr->color_transform(),
         sizeof(frame.color_transform));
  if (status != Error::NONE)
    return status;

  last_frame_layers = display_ptr->UpdateLastFrameAndGetLastFrameLayers();

  base::unique_fd fence;
  if (observer_)
    fence = observer_->OnNewFrame(frame);

  if (fence.get() < 0)
    return Error::NONE;

  *outPresentFence = dup(fence.get());
  outLayers->swap(last_frame_layers);
  for (size_t i = 0; i < outLayers->size(); ++i)
    outReleaseFences->push_back(dup(fence.get()));

  return Error::NONE;
}

Error VrHwc::setLayerCursorPosition(Display display, Layer layer, int32_t x,
                                    int32_t y) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->info.cursor_x = x;
  hwc_layer->info.cursor_y = y;
  return Error::NONE;
}

Error VrHwc::setLayerBuffer(Display display, Layer layer,
                            buffer_handle_t buffer, int32_t acquireFence) {
  base::unique_fd fence(acquireFence);
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->info.buffer = CreateGraphicBuffer(
      buffer, hwc_layer->buffer_metadata);
  hwc_layer->info.fence = new Fence(fence.release());

  return Error::NONE;
}

Error VrHwc::setLayerSurfaceDamage(Display display, Layer layer,
                                   const std::vector<hwc_rect_t>& damage) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->info.damaged_regions = damage;
  return Error::NONE;
}

Error VrHwc::setLayerBlendMode(Display display, Layer layer, int32_t mode) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->info.blend_mode =
      static_cast<ComposerView::ComposerLayer::BlendMode>(mode);

  return Error::NONE;
}

Error VrHwc::setLayerColor(Display display, Layer layer,
                           IComposerClient::Color color) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->info.color = color;
  return Error::NONE;
}

Error VrHwc::setLayerCompositionType(Display display, Layer layer,
                                     int32_t type) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->composition_type = static_cast<HwcLayer::Composition>(type);

  return Error::NONE;
}

Error VrHwc::setLayerDataspace(Display display, Layer layer,
                               int32_t dataspace) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->info.dataspace = dataspace;
  return Error::NONE;
}

Error VrHwc::setLayerDisplayFrame(Display display, Layer layer,
                                  const hwc_rect_t& frame) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->info.display_frame =
      {frame.left, frame.top, frame.right, frame.bottom};

  return Error::NONE;
}

Error VrHwc::setLayerPlaneAlpha(Display display, Layer layer, float alpha) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->info.alpha = alpha;

  return Error::NONE;
}

Error VrHwc::setLayerSidebandStream(Display display, Layer /* layer */,
                                    buffer_handle_t /* stream */) {
  std::lock_guard<std::mutex> guard(mutex_);
  if (!FindDisplay(display))
    return Error::BAD_DISPLAY;
  return Error::NONE;
}

Error VrHwc::setLayerSourceCrop(Display display, Layer layer,
                                const hwc_frect_t& crop) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->info.crop = {crop.left, crop.top, crop.right, crop.bottom};

  return Error::NONE;
}

Error VrHwc::setLayerTransform(Display display, Layer layer,
                               int32_t transform) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->info.transform = transform;
  return Error::NONE;
}

Error VrHwc::setLayerVisibleRegion(Display display, Layer layer,
                                   const std::vector<hwc_rect_t>& visible) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->info.visible_regions = visible;
  return Error::NONE;
}

Error VrHwc::setLayerZOrder(Display display, Layer layer, uint32_t z) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->info.z_order = z;

  return Error::NONE;
}

Error VrHwc::setLayerInfo(Display display, Layer layer, uint32_t type,
                          uint32_t appId) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->info.type = type;
  hwc_layer->info.app_id = appId;

  return Error::NONE;
}

Error VrHwc::setClientTargetMetadata(
    Display display, const IVrComposerClient::BufferMetadata& metadata) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  display_ptr->SetClientTargetMetadata(metadata);

  return Error::NONE;
}

Error VrHwc::setLayerBufferMetadata(
    Display display, Layer layer,
    const IVrComposerClient::BufferMetadata& metadata) {
  std::lock_guard<std::mutex> guard(mutex_);
  auto display_ptr = FindDisplay(display);
  if (!display_ptr)
    return Error::BAD_DISPLAY;

  HwcLayer* hwc_layer = display_ptr->GetLayer(layer);
  if (!hwc_layer)
    return Error::BAD_LAYER;

  hwc_layer->buffer_metadata = metadata;

  return Error::NONE;
}

Return<void> VrHwc::getCapabilities(getCapabilities_cb hidl_cb) {
  hidl_cb(hidl_vec<Capability>());
  return Void();
}

Return<void> VrHwc::dumpDebugInfo(dumpDebugInfo_cb hidl_cb) {
  std::string result;

  {
    std::lock_guard<std::mutex> guard(mutex_);
    result = "\nVrHwc states:\n";
    for (const auto& pair : displays_) {
      result += StringPrintf("Display id: %lu\n", (unsigned long)pair.first);
      pair.second->dumpDebugInfo(&result);
    }
    result += "\n";
  }

  hidl_cb(hidl_string(result));
  return Void();
}

Return<void> VrHwc::createClient(createClient_cb hidl_cb) {
  std::lock_guard<std::mutex> guard(mutex_);

  Error status = Error::NONE;
  sp<VrComposerClient> client;
  if (!client_.promote().get()) {
    client = new VrComposerClient(*this);
  } else {
    ALOGE("Already have a client");
    status = Error::NO_RESOURCES;
  }

  client_ = client;
  hidl_cb(status, client);
  return Void();
}

void VrHwc::ForceDisplaysRefresh() {
  std::lock_guard<std::mutex> guard(mutex_);
  if (event_callback_ != nullptr) {
    for (const auto& pair : displays_)
      event_callback_->onRefresh(pair.first);
  }
}

void VrHwc::RegisterObserver(Observer* observer) {
  std::lock_guard<std::mutex> guard(mutex_);
  if (observer_)
    ALOGE("Overwriting observer");
  else
    observer_ = observer;
}

void VrHwc::UnregisterObserver(Observer* observer) {
  std::lock_guard<std::mutex> guard(mutex_);
  if (observer != observer_)
    ALOGE("Trying to unregister unknown observer");
  else
    observer_ = nullptr;
}

HwcDisplay* VrHwc::FindDisplay(Display display) {
  auto iter = displays_.find(display);
  return iter == displays_.end() ? nullptr : iter->second.get();
}

void VrHwc::UpdateVsyncCallbackEnabledLocked() {
  auto primary_display = FindDisplay(kDefaultDisplayId);
  LOG_ALWAYS_FATAL_IF(event_callback_ != nullptr && primary_display == nullptr,
      "Should have created the primary display by now");
  bool send_vsync =
      event_callback_ != nullptr && primary_display->vsync_enabled();
  vsync_callback_->SetEventCallback(send_vsync ? event_callback_ : nullptr);
}

void HwcLayer::dumpDebugInfo(std::string* result) const {
  if (!result) {
    return;
  }
  *result += StringPrintf("Layer: composition_type: %d, type: %d, app_id: %d, z_order: %d,\
      cursor_x: %d, cursor_y: %d, color(rgba): (%d,%d,%d,%d), dataspace: %d, transform: %d,\
      display_frame(LTRB): (%d,%d,%d,%d), crop(LTRB): (%.1f,%.1f,%.1f,%.1f), blend_mode: %d\n",
      composition_type, info.type, info.app_id, info.z_order, info.cursor_x, info.cursor_y,
      info.color.r, info.color.g, info.color.b, info.color.a, info.dataspace, info.transform,
      info.display_frame.left, info.display_frame.top, info.display_frame.right,
      info.display_frame.bottom, info.crop.left, info.crop.top, info.crop.right,
      info.crop.bottom, info.blend_mode);
  *result += StringPrintf("Layer buffer metadata: width: %d, height: %d, stride: %d, layerCount: %d\
      , pixelFormat: %d\n", buffer_metadata.width, buffer_metadata.height, buffer_metadata.stride,
      buffer_metadata.layerCount, buffer_metadata.format);
}

status_t VrHwc::VsyncCallback::onVsync(int64_t vsync_timestamp) {
  ATRACE_NAME("vr_hwc onVsync");
  std::lock_guard<std::mutex> guard(mutex_);
  if (callback_ != nullptr)
    callback_->onVsync(kDefaultDisplayId, vsync_timestamp);
  return OK;
}

void VrHwc::VsyncCallback::SetEventCallback(EventCallback* callback) {
  std::lock_guard<std::mutex> guard(mutex_);
  callback_ = callback;
}

}  // namespace dvr
}  // namespace android
