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
#ifndef ANDROID_DVR_HARDWARE_COMPOSER_IMPL_VR_HWC_H
#define ANDROID_DVR_HARDWARE_COMPOSER_IMPL_VR_HWC_H

#include <android-base/unique_fd.h>
#include <android/frameworks/vr/composer/1.0/IVrComposerClient.h>
#include <android/hardware/graphics/composer/2.1/IComposer.h>
#include <composer-hal/2.1/ComposerHal.h>
#include <private/dvr/vsync_service.h>
#include <ui/Fence.h>
#include <ui/GraphicBuffer.h>
#include <utils/StrongPointer.h>

#include <mutex>
#include <unordered_map>

using namespace android::frameworks::vr::composer::V1_0;
using namespace android::hardware::graphics::common::V1_0;
using namespace android::hardware::graphics::composer::V2_1;

using android::hardware::hidl_handle;
using android::hardware::hidl_string;
using android::hardware::hidl_vec;
using android::hardware::Return;
using android::hardware::Void;

namespace android {

class Fence;

namespace dvr {

class VrComposerClient;

using android::hardware::graphics::common::V1_0::PixelFormat;
using android::hardware::graphics::composer::V2_1::hal::ComposerHal;

class ComposerView {
 public:
  struct ComposerLayer {
    using Recti = hardware::graphics::composer::V2_1::IComposerClient::Rect;
    using Rectf = hardware::graphics::composer::V2_1::IComposerClient::FRect;
    using BlendMode =
        hardware::graphics::composer::V2_1::IComposerClient::BlendMode;

    Layer id;
    sp<GraphicBuffer> buffer;
    sp<Fence> fence;
    Recti display_frame;
    Rectf crop;
    BlendMode blend_mode;
    float alpha;
    uint32_t type;
    uint32_t app_id;
    uint32_t z_order;
    int32_t cursor_x;
    int32_t cursor_y;
    IComposerClient::Color color;
    int32_t dataspace;
    int32_t transform;
    std::vector<hwc_rect_t> visible_regions;
    std::vector<hwc_rect_t> damaged_regions;
  };

  struct Frame {
    Display display_id;
    // This is set to true to notify the upper layer that the display is
    // being removed, or left false in the case of a normal frame. The upper
    // layer tracks display IDs and will handle new ones showing up.
    bool removed = false;
    int32_t display_width;
    int32_t display_height;
    Config active_config;
    ColorMode color_mode;
    IComposerClient::PowerMode power_mode;
    IComposerClient::Vsync vsync_enabled;
    float color_transform[16];
    int32_t color_transform_hint;
    std::vector<ComposerLayer> layers;
  };

  class Observer {
   public:
    virtual ~Observer() {}

    // Returns a list of layers that need to be shown together. Layers are
    // returned in z-order, with the lowest layer first.
    virtual base::unique_fd OnNewFrame(const Frame& frame) = 0;
  };

  virtual ~ComposerView() {}

  virtual void ForceDisplaysRefresh() = 0;
  virtual void RegisterObserver(Observer* observer) = 0;
  virtual void UnregisterObserver(Observer* observer) = 0;
};

struct HwcLayer {
  using Composition =
      hardware::graphics::composer::V2_1::IComposerClient::Composition;

  explicit HwcLayer(Layer new_id) { info.id = new_id; }

  void dumpDebugInfo(std::string* result) const;

  Composition composition_type;
  ComposerView::ComposerLayer info;
  IVrComposerClient::BufferMetadata buffer_metadata;
};

class HwcDisplay {
 public:
  HwcDisplay(int32_t width, int32_t height);
  ~HwcDisplay();

  int32_t width() const { return width_; }
  int32_t height() const { return height_; }

  HwcLayer* CreateLayer();
  bool DestroyLayer(Layer id);
  HwcLayer* GetLayer(Layer id);

  bool SetClientTarget(const native_handle_t* handle, base::unique_fd fence);
  void SetClientTargetMetadata(
      const IVrComposerClient::BufferMetadata& metadata);

  void GetChangedCompositionTypes(
      std::vector<Layer>* layer_ids,
      std::vector<IComposerClient::Composition>* composition);

  Error GetFrame(std::vector<ComposerView::ComposerLayer>* out_frame);

  std::vector<Layer> UpdateLastFrameAndGetLastFrameLayers();

  Config active_config() const { return active_config_; }
  void set_active_config(Config config) { active_config_ = config; }

  ColorMode color_mode() const { return color_mode_; }
  void set_color_mode(ColorMode mode) { color_mode_ = mode; }

  IComposerClient::PowerMode power_mode() const { return power_mode_; }
  void set_power_mode(IComposerClient::PowerMode mode) { power_mode_ = mode; }

  bool vsync_enabled() const { return vsync_enabled_; }
  void set_vsync_enabled(bool vsync) {vsync_enabled_ = vsync;}

  const float* color_transform() const { return color_transform_; }
  int32_t color_transform_hint() const { return color_transform_hint_; }
  void SetColorTransform(const float* matrix, int32_t hint);

  void dumpDebugInfo(std::string* result) const;

 private:
  // The client target buffer and the associated fence.
  sp<GraphicBuffer> buffer_;
  IVrComposerClient::BufferMetadata buffer_metadata_;
  sp<Fence> fence_;

  // List of currently active layers.
  std::vector<HwcLayer> layers_;

  std::vector<Layer> last_frame_layers_ids_;

  // Layer ID generator.
  uint64_t layer_ids_ = 1;

  int32_t width_;
  int32_t height_;

  Config active_config_;
  ColorMode color_mode_;
  IComposerClient::PowerMode power_mode_;
  bool vsync_enabled_ = false;
  float color_transform_[16];
  int32_t color_transform_hint_;

  HwcDisplay(const HwcDisplay&) = delete;
  void operator=(const HwcDisplay&) = delete;
};

class VrHwc : public IComposer, public ComposerHal, public ComposerView {
 public:
  VrHwc();
  ~VrHwc() override;

  Error setLayerInfo(Display display, Layer layer, uint32_t type,
                     uint32_t appId);
  Error setClientTargetMetadata(
      Display display, const IVrComposerClient::BufferMetadata& metadata);
  Error setLayerBufferMetadata(
      Display display, Layer layer,
      const IVrComposerClient::BufferMetadata& metadata);

  // ComposerHal
  bool hasCapability(hwc2_capability_t capability) override;

  std::string dumpDebugInfo() override { return {}; }
  void registerEventCallback(EventCallback* callback) override;
  void unregisterEventCallback() override;

  uint32_t getMaxVirtualDisplayCount() override;
  Error createVirtualDisplay(uint32_t width, uint32_t height,
      PixelFormat* format, Display* outDisplay) override;
  Error destroyVirtualDisplay(Display display) override;

  Error createLayer(Display display, Layer* outLayer) override;
  Error destroyLayer(Display display, Layer layer) override;

  Error getActiveConfig(Display display, Config* outConfig) override;
  Error getClientTargetSupport(Display display,
          uint32_t width, uint32_t height,
          PixelFormat format, Dataspace dataspace) override;
  Error getColorModes(Display display, hidl_vec<ColorMode>* outModes) override;
  Error getDisplayAttribute(Display display, Config config,
          IComposerClient::Attribute attribute, int32_t* outValue) override;
  Error getDisplayConfigs(Display display, hidl_vec<Config>* outConfigs) override;
  Error getDisplayName(Display display, hidl_string* outName) override;
  Error getDisplayType(Display display,
          IComposerClient::DisplayType* outType) override;
  Error getDozeSupport(Display display, bool* outSupport) override;
  Error getHdrCapabilities(Display display, hidl_vec<Hdr>* outTypes,
          float* outMaxLuminance, float* outMaxAverageLuminance,
          float* outMinLuminance) override;

  Error setActiveConfig(Display display, Config config) override;
  Error setColorMode(Display display, ColorMode mode) override;
  Error setPowerMode(Display display, IComposerClient::PowerMode mode) override;
  Error setVsyncEnabled(Display display, IComposerClient::Vsync enabled) override;

  Error setColorTransform(Display display, const float* matrix,
          int32_t hint) override;
  Error setClientTarget(Display display, buffer_handle_t target,
          int32_t acquireFence, int32_t dataspace,
          const std::vector<hwc_rect_t>& damage) override;
  Error setOutputBuffer(Display display, buffer_handle_t buffer,
          int32_t releaseFence) override;
  Error validateDisplay(Display display,
          std::vector<Layer>* outChangedLayers,
          std::vector<IComposerClient::Composition>* outCompositionTypes,
          uint32_t* outDisplayRequestMask,
          std::vector<Layer>* outRequestedLayers,
          std::vector<uint32_t>* outRequestMasks) override;
  Error acceptDisplayChanges(Display display) override;
  Error presentDisplay(Display display, int32_t* outPresentFence,
          std::vector<Layer>* outLayers,
          std::vector<int32_t>* outReleaseFences) override;

  Error setLayerCursorPosition(Display display, Layer layer,
          int32_t x, int32_t y) override;
  Error setLayerBuffer(Display display, Layer layer,
          buffer_handle_t buffer, int32_t acquireFence) override;
  Error setLayerSurfaceDamage(Display display, Layer layer,
          const std::vector<hwc_rect_t>& damage) override;
  Error setLayerBlendMode(Display display, Layer layer, int32_t mode) override;
  Error setLayerColor(Display display, Layer layer,
          IComposerClient::Color color) override;
  Error setLayerCompositionType(Display display, Layer layer,
          int32_t type) override;
  Error setLayerDataspace(Display display, Layer layer,
          int32_t dataspace) override;
  Error setLayerDisplayFrame(Display display, Layer layer,
          const hwc_rect_t& frame) override;
  Error setLayerPlaneAlpha(Display display, Layer layer, float alpha) override;
  Error setLayerSidebandStream(Display display, Layer layer,
          buffer_handle_t stream) override;
  Error setLayerSourceCrop(Display display, Layer layer,
          const hwc_frect_t& crop) override;
  Error setLayerTransform(Display display, Layer layer,
          int32_t transform) override;
  Error setLayerVisibleRegion(Display display, Layer layer,
          const std::vector<hwc_rect_t>& visible) override;
  Error setLayerZOrder(Display display, Layer layer, uint32_t z) override;

  // IComposer:
  Return<void> getCapabilities(getCapabilities_cb hidl_cb) override;
  Return<void> dumpDebugInfo(dumpDebugInfo_cb hidl_cb) override;
  Return<void> createClient(createClient_cb hidl_cb) override;

  // ComposerView:
  void ForceDisplaysRefresh() override;
  void RegisterObserver(Observer* observer) override;
  void UnregisterObserver(Observer* observer) override;

 private:
  class VsyncCallback : public BnVsyncCallback {
   public:
    status_t onVsync(int64_t vsync_timestamp) override;
    void SetEventCallback(EventCallback* callback);
   private:
    std::mutex mutex_;
    EventCallback* callback_;
  };

  HwcDisplay* FindDisplay(Display display);

  // Re-evaluate whether or not we should start making onVsync() callbacks to
  // the client. We need enableCallback(true) to have been called, and
  // setVsyncEnabled() to have been called for the primary display. The caller
  // must have mutex_ locked already.
  void UpdateVsyncCallbackEnabledLocked();

  wp<VrComposerClient> client_;

  // Guard access to internal state from binder threads.
  std::mutex mutex_;

  std::unordered_map<Display, std::unique_ptr<HwcDisplay>> displays_;
  Display display_count_ = 2;

  EventCallback* event_callback_ = nullptr;
  Observer* observer_ = nullptr;

  sp<VsyncCallback> vsync_callback_;

  VrHwc(const VrHwc&) = delete;
  void operator=(const VrHwc&) = delete;
};

}  // namespace dvr
}  // namespace android

#endif  // ANDROID_DVR_HARDWARE_COMPOSER_IMPL_VR_HWC_H
