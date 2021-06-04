/*
 * Copyright 2015 The Android Open Source Project
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

#ifndef ANDROID_SF_HWC2_H
#define ANDROID_SF_HWC2_H

#define HWC2_INCLUDE_STRINGIFICATION
#define HWC2_USE_CPP11
#include <hardware/hwcomposer2.h>
#undef HWC2_INCLUDE_STRINGIFICATION
#undef HWC2_USE_CPP11

#include <gui/HdrMetadata.h>
#include <math/mat4.h>
#include <ui/GraphicTypes.h>
#include <ui/HdrCapabilities.h>
#include <ui/Region.h>
#include <utils/Log.h>
#include <utils/StrongPointer.h>
#include <utils/Timers.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace android {
    struct DisplayedFrameStats;
    class Fence;
    class FloatRect;
    class GraphicBuffer;
    namespace Hwc2 {
        class Composer;
    }

    class TestableSurfaceFlinger;
}

namespace HWC2 {

class Display;
class Layer;

// Implement this interface to receive hardware composer events.
//
// These callback functions will generally be called on a hwbinder thread, but
// when first registering the callback the onHotplugReceived() function will
// immediately be called on the thread calling registerCallback().
//
// All calls receive a sequenceId, which will be the value that was supplied to
// HWC2::Device::registerCallback(). It's used to help differentiate callbacks
// from different hardware composer instances.
class ComposerCallback {
 public:
    virtual void onHotplugReceived(int32_t sequenceId, hwc2_display_t display,
                                   Connection connection) = 0;
    virtual void onRefreshReceived(int32_t sequenceId,
                                   hwc2_display_t display) = 0;
    virtual void onVsyncReceived(int32_t sequenceId, hwc2_display_t display,
                                 int64_t timestamp) = 0;
    virtual ~ComposerCallback() = default;
};

// C++ Wrapper around hwc2_device_t. Load all functions pointers
// and handle callback registration.
class Device
{
public:
    explicit Device(std::unique_ptr<android::Hwc2::Composer> composer);

    void registerCallback(ComposerCallback* callback, int32_t sequenceId);

    // Required by HWC2

    std::string dump() const;

    const std::unordered_set<Capability>& getCapabilities() const {
        return mCapabilities;
    };

    uint32_t getMaxVirtualDisplayCount() const;
    Error getDisplayIdentificationData(hwc2_display_t hwcDisplayId, uint8_t* outPort,
                                       std::vector<uint8_t>* outData) const;

    Error createVirtualDisplay(uint32_t width, uint32_t height,
            android::ui::PixelFormat* format, Display** outDisplay);
    void destroyDisplay(hwc2_display_t displayId);

    void onHotplug(hwc2_display_t displayId, Connection connection);

    // Other Device methods

    Display* getDisplayById(hwc2_display_t id);

    android::Hwc2::Composer* getComposer() { return mComposer.get(); }

    // We buffer most state changes and flush them implicitly with
    // Display::validate, Display::present, and Display::presentOrValidate.
    // This method provides an explicit way to flush state changes to HWC.
    Error flushCommands();

private:
    // Initialization methods

    void loadCapabilities();

    // Member variables
    std::unique_ptr<android::Hwc2::Composer> mComposer;
    std::unordered_set<Capability> mCapabilities;
    std::unordered_map<hwc2_display_t, std::unique_ptr<Display>> mDisplays;
    bool mRegisteredCallback = false;
};

// Convenience C++ class to access hwc2_device_t Display functions directly.
class Display {
public:
    virtual ~Display();

    class Config {
    public:
        class Builder
        {
        public:
            Builder(Display& display, hwc2_config_t id);

            std::shared_ptr<const Config> build() {
                return std::const_pointer_cast<const Config>(
                        std::move(mConfig));
            }

            Builder& setWidth(int32_t width) {
                mConfig->mWidth = width;
                return *this;
            }
            Builder& setHeight(int32_t height) {
                mConfig->mHeight = height;
                return *this;
            }
            Builder& setVsyncPeriod(int32_t vsyncPeriod) {
                mConfig->mVsyncPeriod = vsyncPeriod;
                return *this;
            }
            Builder& setDpiX(int32_t dpiX) {
                if (dpiX == -1) {
                    mConfig->mDpiX = getDefaultDensity();
                } else {
                    mConfig->mDpiX = dpiX / 1000.0f;
                }
                return *this;
            }
            Builder& setDpiY(int32_t dpiY) {
                if (dpiY == -1) {
                    mConfig->mDpiY = getDefaultDensity();
                } else {
                    mConfig->mDpiY = dpiY / 1000.0f;
                }
                return *this;
            }

        private:
            float getDefaultDensity();
            std::shared_ptr<Config> mConfig;
        };

        hwc2_display_t getDisplayId() const { return mDisplay.getId(); }
        hwc2_config_t getId() const { return mId; }

        int32_t getWidth() const { return mWidth; }
        int32_t getHeight() const { return mHeight; }
        nsecs_t getVsyncPeriod() const { return mVsyncPeriod; }
        float getDpiX() const { return mDpiX; }
        float getDpiY() const { return mDpiY; }

    private:
        Config(Display& display, hwc2_config_t id);

        Display& mDisplay;
        hwc2_config_t mId;

        int32_t mWidth;
        int32_t mHeight;
        nsecs_t mVsyncPeriod;
        float mDpiX;
        float mDpiY;
    };

    virtual hwc2_display_t getId() const = 0;
    virtual bool isConnected() const = 0;
    virtual void setConnected(bool connected) = 0; // For use by Device only
    virtual const std::unordered_set<DisplayCapability>& getCapabilities() const = 0;

    [[clang::warn_unused_result]] virtual Error acceptChanges() = 0;
    [[clang::warn_unused_result]] virtual Error createLayer(Layer** outLayer) = 0;
    [[clang::warn_unused_result]] virtual Error destroyLayer(Layer* layer) = 0;
    [[clang::warn_unused_result]] virtual Error getActiveConfig(
            std::shared_ptr<const Config>* outConfig) const = 0;
    [[clang::warn_unused_result]] virtual Error getActiveConfigIndex(int* outIndex) const = 0;
    [[clang::warn_unused_result]] virtual Error getChangedCompositionTypes(
            std::unordered_map<Layer*, Composition>* outTypes) = 0;
    [[clang::warn_unused_result]] virtual Error getColorModes(
            std::vector<android::ui::ColorMode>* outModes) const = 0;
    // Returns a bitmask which contains HdrMetadata::Type::*.
    [[clang::warn_unused_result]] virtual int32_t getSupportedPerFrameMetadata() const = 0;
    [[clang::warn_unused_result]] virtual Error getRenderIntents(
            android::ui::ColorMode colorMode,
            std::vector<android::ui::RenderIntent>* outRenderIntents) const = 0;
    [[clang::warn_unused_result]] virtual Error getDataspaceSaturationMatrix(
            android::ui::Dataspace dataspace, android::mat4* outMatrix) = 0;

    // Doesn't call into the HWC2 device, so no Errors are possible
    virtual std::vector<std::shared_ptr<const Config>> getConfigs() const = 0;

    [[clang::warn_unused_result]] virtual Error getName(std::string* outName) const = 0;
    [[clang::warn_unused_result]] virtual Error getRequests(
            DisplayRequest* outDisplayRequests,
            std::unordered_map<Layer*, LayerRequest>* outLayerRequests) = 0;
    [[clang::warn_unused_result]] virtual Error getType(DisplayType* outType) const = 0;
    [[clang::warn_unused_result]] virtual Error supportsDoze(bool* outSupport) const = 0;
    [[clang::warn_unused_result]] virtual Error getHdrCapabilities(
            android::HdrCapabilities* outCapabilities) const = 0;
    [[clang::warn_unused_result]] virtual Error getDisplayedContentSamplingAttributes(
            android::ui::PixelFormat* outFormat, android::ui::Dataspace* outDataspace,
            uint8_t* outComponentMask) const = 0;
    [[clang::warn_unused_result]] virtual Error setDisplayContentSamplingEnabled(
            bool enabled, uint8_t componentMask, uint64_t maxFrames) const = 0;
    [[clang::warn_unused_result]] virtual Error getDisplayedContentSample(
            uint64_t maxFrames, uint64_t timestamp,
            android::DisplayedFrameStats* outStats) const = 0;
    [[clang::warn_unused_result]] virtual Error getReleaseFences(
            std::unordered_map<Layer*, android::sp<android::Fence>>* outFences) const = 0;
    [[clang::warn_unused_result]] virtual Error present(
            android::sp<android::Fence>* outPresentFence) = 0;
    [[clang::warn_unused_result]] virtual Error setActiveConfig(
            const std::shared_ptr<const Config>& config) = 0;
    [[clang::warn_unused_result]] virtual Error setClientTarget(
            uint32_t slot, const android::sp<android::GraphicBuffer>& target,
            const android::sp<android::Fence>& acquireFence, android::ui::Dataspace dataspace) = 0;
    [[clang::warn_unused_result]] virtual Error setColorMode(
            android::ui::ColorMode mode, android::ui::RenderIntent renderIntent) = 0;
    [[clang::warn_unused_result]] virtual Error setColorTransform(
            const android::mat4& matrix, android_color_transform_t hint) = 0;
    [[clang::warn_unused_result]] virtual Error setOutputBuffer(
            const android::sp<android::GraphicBuffer>& buffer,
            const android::sp<android::Fence>& releaseFence) = 0;
    [[clang::warn_unused_result]] virtual Error setPowerMode(PowerMode mode) = 0;
    [[clang::warn_unused_result]] virtual Error setVsyncEnabled(Vsync enabled) = 0;
    [[clang::warn_unused_result]] virtual Error validate(uint32_t* outNumTypes,
                                                         uint32_t* outNumRequests) = 0;
    [[clang::warn_unused_result]] virtual Error presentOrValidate(
            uint32_t* outNumTypes, uint32_t* outNumRequests,
            android::sp<android::Fence>* outPresentFence, uint32_t* state) = 0;
    [[clang::warn_unused_result]] virtual Error setDisplayBrightness(float brightness) const = 0;
};

namespace impl {

class Display : public HWC2::Display {
public:
    Display(android::Hwc2::Composer& composer, const std::unordered_set<Capability>& capabilities,
            hwc2_display_t id, DisplayType type);
    ~Display() override;

    // Required by HWC2
    Error acceptChanges() override;
    Error createLayer(Layer** outLayer) override;
    Error destroyLayer(Layer* layer) override;
    Error getActiveConfig(std::shared_ptr<const Config>* outConfig) const override;
    Error getActiveConfigIndex(int* outIndex) const override;
    Error getChangedCompositionTypes(std::unordered_map<Layer*, Composition>* outTypes) override;
    Error getColorModes(std::vector<android::ui::ColorMode>* outModes) const override;
    // Returns a bitmask which contains HdrMetadata::Type::*.
    int32_t getSupportedPerFrameMetadata() const override;
    Error getRenderIntents(android::ui::ColorMode colorMode,
                           std::vector<android::ui::RenderIntent>* outRenderIntents) const override;
    Error getDataspaceSaturationMatrix(android::ui::Dataspace dataspace,
                                       android::mat4* outMatrix) override;

    // Doesn't call into the HWC2 device, so no errors are possible
    std::vector<std::shared_ptr<const Config>> getConfigs() const override;

    Error getName(std::string* outName) const override;
    Error getRequests(DisplayRequest* outDisplayRequests,
                      std::unordered_map<Layer*, LayerRequest>* outLayerRequests) override;
    Error getType(DisplayType* outType) const override;
    Error supportsDoze(bool* outSupport) const override;
    Error getHdrCapabilities(android::HdrCapabilities* outCapabilities) const override;
    Error getDisplayedContentSamplingAttributes(android::ui::PixelFormat* outFormat,
                                                android::ui::Dataspace* outDataspace,
                                                uint8_t* outComponentMask) const override;
    Error setDisplayContentSamplingEnabled(bool enabled, uint8_t componentMask,
                                           uint64_t maxFrames) const override;
    Error getDisplayedContentSample(uint64_t maxFrames, uint64_t timestamp,
                                    android::DisplayedFrameStats* outStats) const override;
    Error getReleaseFences(
            std::unordered_map<Layer*, android::sp<android::Fence>>* outFences) const override;
    Error present(android::sp<android::Fence>* outPresentFence) override;
    Error setActiveConfig(const std::shared_ptr<const HWC2::Display::Config>& config) override;
    Error setClientTarget(uint32_t slot, const android::sp<android::GraphicBuffer>& target,
                          const android::sp<android::Fence>& acquireFence,
                          android::ui::Dataspace dataspace) override;
    Error setColorMode(android::ui::ColorMode mode,
                       android::ui::RenderIntent renderIntent) override;
    Error setColorTransform(const android::mat4& matrix, android_color_transform_t hint) override;
    Error setOutputBuffer(const android::sp<android::GraphicBuffer>& buffer,
                          const android::sp<android::Fence>& releaseFence) override;
    Error setPowerMode(PowerMode mode) override;
    Error setVsyncEnabled(Vsync enabled) override;
    Error validate(uint32_t* outNumTypes, uint32_t* outNumRequests) override;
    Error presentOrValidate(uint32_t* outNumTypes, uint32_t* outNumRequests,
                            android::sp<android::Fence>* outPresentFence, uint32_t* state) override;
    Error setDisplayBrightness(float brightness) const override;

    // Other Display methods
    hwc2_display_t getId() const override { return mId; }
    bool isConnected() const override { return mIsConnected; }
    void setConnected(bool connected) override; // For use by Device only
    const std::unordered_set<DisplayCapability>& getCapabilities() const override {
        return mDisplayCapabilities;
    };

private:
    int32_t getAttribute(hwc2_config_t configId, Attribute attribute);
    void loadConfig(hwc2_config_t configId);
    void loadConfigs();

    // This may fail (and return a null pointer) if no layer with this ID exists
    // on this display
    Layer* getLayerById(hwc2_layer_t id) const;

    friend android::TestableSurfaceFlinger;

    // Member variables

    // These are references to data owned by HWC2::Device, which will outlive
    // this HWC2::Display, so these references are guaranteed to be valid for
    // the lifetime of this object.
    android::Hwc2::Composer& mComposer;
    const std::unordered_set<Capability>& mCapabilities;

    hwc2_display_t mId;
    bool mIsConnected;
    DisplayType mType;
    std::unordered_map<hwc2_layer_t, std::unique_ptr<Layer>> mLayers;
    std::unordered_map<hwc2_config_t, std::shared_ptr<const Config>> mConfigs;
    std::once_flag mDisplayCapabilityQueryFlag;
    std::unordered_set<DisplayCapability> mDisplayCapabilities;
};
} // namespace impl

class Layer {
public:
    virtual ~Layer();

    virtual hwc2_layer_t getId() const = 0;

    [[clang::warn_unused_result]] virtual Error setCursorPosition(int32_t x, int32_t y) = 0;
    [[clang::warn_unused_result]] virtual Error setBuffer(
            uint32_t slot, const android::sp<android::GraphicBuffer>& buffer,
            const android::sp<android::Fence>& acquireFence) = 0;
    [[clang::warn_unused_result]] virtual Error setSurfaceDamage(const android::Region& damage) = 0;

    [[clang::warn_unused_result]] virtual Error setBlendMode(BlendMode mode) = 0;
    [[clang::warn_unused_result]] virtual Error setColor(hwc_color_t color) = 0;
    [[clang::warn_unused_result]] virtual Error setCompositionType(Composition type) = 0;
    [[clang::warn_unused_result]] virtual Error setDataspace(android::ui::Dataspace dataspace) = 0;
    [[clang::warn_unused_result]] virtual Error setPerFrameMetadata(
            const int32_t supportedPerFrameMetadata, const android::HdrMetadata& metadata) = 0;
    [[clang::warn_unused_result]] virtual Error setDisplayFrame(const android::Rect& frame) = 0;
    [[clang::warn_unused_result]] virtual Error setPlaneAlpha(float alpha) = 0;
    [[clang::warn_unused_result]] virtual Error setSidebandStream(
            const native_handle_t* stream) = 0;
    [[clang::warn_unused_result]] virtual Error setSourceCrop(const android::FloatRect& crop) = 0;
    [[clang::warn_unused_result]] virtual Error setTransform(Transform transform) = 0;
    [[clang::warn_unused_result]] virtual Error setVisibleRegion(const android::Region& region) = 0;
    [[clang::warn_unused_result]] virtual Error setZOrder(uint32_t z) = 0;
    [[clang::warn_unused_result]] virtual Error setInfo(uint32_t type, uint32_t appId) = 0;

    // Composer HAL 2.3
    [[clang::warn_unused_result]] virtual Error setColorTransform(const android::mat4& matrix) = 0;
};

namespace impl {

// Convenience C++ class to access hwc2_device_t Layer functions directly.

class Layer : public HWC2::Layer {
public:
    Layer(android::Hwc2::Composer& composer,
          const std::unordered_set<Capability>& capabilities,
          hwc2_display_t displayId, hwc2_layer_t layerId);
    ~Layer() override;

    hwc2_layer_t getId() const override { return mId; }

    Error setCursorPosition(int32_t x, int32_t y) override;
    Error setBuffer(uint32_t slot, const android::sp<android::GraphicBuffer>& buffer,
                    const android::sp<android::Fence>& acquireFence) override;
    Error setSurfaceDamage(const android::Region& damage) override;

    Error setBlendMode(BlendMode mode) override;
    Error setColor(hwc_color_t color) override;
    Error setCompositionType(Composition type) override;
    Error setDataspace(android::ui::Dataspace dataspace) override;
    Error setPerFrameMetadata(const int32_t supportedPerFrameMetadata,
                              const android::HdrMetadata& metadata) override;
    Error setDisplayFrame(const android::Rect& frame) override;
    Error setPlaneAlpha(float alpha) override;
    Error setSidebandStream(const native_handle_t* stream) override;
    Error setSourceCrop(const android::FloatRect& crop) override;
    Error setTransform(Transform transform) override;
    Error setVisibleRegion(const android::Region& region) override;
    Error setZOrder(uint32_t z) override;
    Error setInfo(uint32_t type, uint32_t appId) override;

    // Composer HAL 2.3
    Error setColorTransform(const android::mat4& matrix) override;

private:
    // These are references to data owned by HWC2::Device, which will outlive
    // this HWC2::Layer, so these references are guaranteed to be valid for
    // the lifetime of this object.
    android::Hwc2::Composer& mComposer;
    const std::unordered_set<Capability>& mCapabilities;

    hwc2_display_t mDisplayId;
    hwc2_layer_t mId;

    // Cached HWC2 data, to ensure the same commands aren't sent to the HWC
    // multiple times.
    android::Region mVisibleRegion = android::Region::INVALID_REGION;
    android::Region mDamageRegion = android::Region::INVALID_REGION;
    android::ui::Dataspace mDataSpace = android::ui::Dataspace::UNKNOWN;
    android::HdrMetadata mHdrMetadata;
    android::mat4 mColorMatrix;
    uint32_t mBufferSlot;
};

} // namespace impl

} // namespace HWC2

#endif // ANDROID_SF_HWC2_H
