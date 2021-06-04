/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef ANDROID_SF_HWCOMPOSER_H
#define ANDROID_SF_HWCOMPOSER_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <android-base/thread_annotations.h>
#include <ui/Fence.h>
#include <ui/GraphicTypes.h>
#include <utils/StrongPointer.h>
#include <utils/Timers.h>

#include "DisplayIdentification.h"
#include "HWC2.h"

namespace android {

struct DisplayedFrameStats;
class GraphicBuffer;
class TestableSurfaceFlinger;
struct CompositionInfo;

namespace Hwc2 {
class Composer;
} // namespace Hwc2

namespace compositionengine {
class Output;
} // namespace compositionengine

class HWComposer {
public:
    virtual ~HWComposer();

    virtual void registerCallback(HWC2::ComposerCallback* callback, int32_t sequenceId) = 0;

    virtual bool getDisplayIdentificationData(hwc2_display_t hwcDisplayId, uint8_t* outPort,
                                              DisplayIdentificationData* outData) const = 0;

    virtual bool hasCapability(HWC2::Capability capability) const = 0;
    virtual bool hasDisplayCapability(const std::optional<DisplayId>& displayId,
                                      HWC2::DisplayCapability capability) const = 0;

    // Attempts to allocate a virtual display and returns its ID if created on the HWC device.
    virtual std::optional<DisplayId> allocateVirtualDisplay(uint32_t width, uint32_t height,
                                                            ui::PixelFormat* format) = 0;

    // Attempts to create a new layer on this display
    virtual HWC2::Layer* createLayer(DisplayId displayId) = 0;
    // Destroy a previously created layer
    virtual void destroyLayer(DisplayId displayId, HWC2::Layer* layer) = 0;

    // Asks the HAL what it can do
    virtual status_t prepare(DisplayId displayId, const compositionengine::Output&) = 0;

    virtual status_t setClientTarget(DisplayId displayId, uint32_t slot,
                                     const sp<Fence>& acquireFence, const sp<GraphicBuffer>& target,
                                     ui::Dataspace dataspace) = 0;

    // Present layers to the display and read releaseFences.
    virtual status_t presentAndGetReleaseFences(DisplayId displayId) = 0;

    // set power mode
    virtual status_t setPowerMode(DisplayId displayId, int mode) = 0;

    // set active config
    virtual status_t setActiveConfig(DisplayId displayId, size_t configId) = 0;

    // Sets a color transform to be applied to the result of composition
    virtual status_t setColorTransform(DisplayId displayId, const mat4& transform) = 0;

    // reset state when an external, non-virtual display is disconnected
    virtual void disconnectDisplay(DisplayId displayId) = 0;

    // does this display have layers handled by HWC
    virtual bool hasDeviceComposition(const std::optional<DisplayId>& displayId) const = 0;

    // does this display have pending request to flip client target
    virtual bool hasFlipClientTargetRequest(const std::optional<DisplayId>& displayId) const = 0;

    // does this display have layers handled by GLES
    virtual bool hasClientComposition(const std::optional<DisplayId>& displayId) const = 0;

    // get the present fence received from the last call to present.
    virtual sp<Fence> getPresentFence(DisplayId displayId) const = 0;

    // Get last release fence for the given layer
    virtual sp<Fence> getLayerReleaseFence(DisplayId displayId, HWC2::Layer* layer) const = 0;

    // Set the output buffer and acquire fence for a virtual display.
    // Returns INVALID_OPERATION if displayId is not a virtual display.
    virtual status_t setOutputBuffer(DisplayId displayId, const sp<Fence>& acquireFence,
                                     const sp<GraphicBuffer>& buffer) = 0;

    // After SurfaceFlinger has retrieved the release fences for all the frames,
    // it can call this to clear the shared pointers in the release fence map
    virtual void clearReleaseFences(DisplayId displayId) = 0;

    // Fetches the HDR capabilities of the given display
    virtual status_t getHdrCapabilities(DisplayId displayId, HdrCapabilities* outCapabilities) = 0;

    virtual int32_t getSupportedPerFrameMetadata(DisplayId displayId) const = 0;

    // Returns the available RenderIntent of the given display.
    virtual std::vector<ui::RenderIntent> getRenderIntents(DisplayId displayId,
                                                           ui::ColorMode colorMode) const = 0;

    virtual mat4 getDataspaceSaturationMatrix(DisplayId displayId, ui::Dataspace dataspace) = 0;

    // Returns the attributes of the color sampling engine.
    virtual status_t getDisplayedContentSamplingAttributes(DisplayId displayId,
                                                           ui::PixelFormat* outFormat,
                                                           ui::Dataspace* outDataspace,
                                                           uint8_t* outComponentMask) = 0;
    virtual status_t setDisplayContentSamplingEnabled(DisplayId displayId, bool enabled,
                                                      uint8_t componentMask,
                                                      uint64_t maxFrames) = 0;
    virtual status_t getDisplayedContentSample(DisplayId displayId, uint64_t maxFrames,
                                               uint64_t timestamp,
                                               DisplayedFrameStats* outStats) = 0;

    // Sets the brightness of a display.
    virtual status_t setDisplayBrightness(DisplayId displayId, float brightness) = 0;

    // Events handling ---------------------------------------------------------

    // Returns stable display ID (and display name on connection of new or previously disconnected
    // display), or std::nullopt if hotplug event was ignored.
    virtual std::optional<DisplayIdentificationInfo> onHotplug(hwc2_display_t hwcDisplayId,
                                                               HWC2::Connection connection) = 0;

    virtual bool onVsync(hwc2_display_t hwcDisplayId, int64_t timestamp) = 0;
    virtual void setVsyncEnabled(DisplayId displayId, HWC2::Vsync enabled) = 0;

    virtual nsecs_t getRefreshTimestamp(DisplayId displayId) const = 0;
    virtual bool isConnected(DisplayId displayId) const = 0;

    // Non-const because it can update configMap inside of mDisplayData
    virtual std::vector<std::shared_ptr<const HWC2::Display::Config>> getConfigs(
            DisplayId displayId) const = 0;

    virtual std::shared_ptr<const HWC2::Display::Config> getActiveConfig(
            DisplayId displayId) const = 0;
    virtual int getActiveConfigIndex(DisplayId displayId) const = 0;

    virtual std::vector<ui::ColorMode> getColorModes(DisplayId displayId) const = 0;

    virtual status_t setActiveColorMode(DisplayId displayId, ui::ColorMode mode,
                                        ui::RenderIntent renderIntent) = 0;

    virtual bool isUsingVrComposer() const = 0;

    // for debugging ----------------------------------------------------------
    virtual void dump(std::string& out) const = 0;

    virtual Hwc2::Composer* getComposer() const = 0;

    // TODO(b/74619554): Remove special cases for internal/external display.
    virtual std::optional<hwc2_display_t> getInternalHwcDisplayId() const = 0;
    virtual std::optional<hwc2_display_t> getExternalHwcDisplayId() const = 0;

    virtual std::optional<DisplayId> toPhysicalDisplayId(hwc2_display_t hwcDisplayId) const = 0;
    virtual std::optional<hwc2_display_t> fromPhysicalDisplayId(DisplayId displayId) const = 0;
};

namespace impl {

class HWComposer final : public android::HWComposer {
public:
    explicit HWComposer(std::unique_ptr<Hwc2::Composer> composer);

    ~HWComposer() override;

    void registerCallback(HWC2::ComposerCallback* callback, int32_t sequenceId) override;

    bool getDisplayIdentificationData(hwc2_display_t hwcDisplayId, uint8_t* outPort,
                                      DisplayIdentificationData* outData) const override;

    bool hasCapability(HWC2::Capability capability) const override;
    bool hasDisplayCapability(const std::optional<DisplayId>& displayId,
                              HWC2::DisplayCapability capability) const override;

    // Attempts to allocate a virtual display and returns its ID if created on the HWC device.
    std::optional<DisplayId> allocateVirtualDisplay(uint32_t width, uint32_t height,
                                                    ui::PixelFormat* format) override;

    // Attempts to create a new layer on this display
    HWC2::Layer* createLayer(DisplayId displayId) override;
    // Destroy a previously created layer
    void destroyLayer(DisplayId displayId, HWC2::Layer* layer) override;

    // Asks the HAL what it can do
    status_t prepare(DisplayId displayId, const compositionengine::Output&) override;

    status_t setClientTarget(DisplayId displayId, uint32_t slot, const sp<Fence>& acquireFence,
                             const sp<GraphicBuffer>& target, ui::Dataspace dataspace) override;

    // Present layers to the display and read releaseFences.
    status_t presentAndGetReleaseFences(DisplayId displayId) override;

    // set power mode
    status_t setPowerMode(DisplayId displayId, int mode) override;

    // set active config
    status_t setActiveConfig(DisplayId displayId, size_t configId) override;

    // Sets a color transform to be applied to the result of composition
    status_t setColorTransform(DisplayId displayId, const mat4& transform) override;

    // reset state when an external, non-virtual display is disconnected
    void disconnectDisplay(DisplayId displayId) override;

    // does this display have layers handled by HWC
    bool hasDeviceComposition(const std::optional<DisplayId>& displayId) const override;

    // does this display have pending request to flip client target
    bool hasFlipClientTargetRequest(const std::optional<DisplayId>& displayId) const override;

    // does this display have layers handled by GLES
    bool hasClientComposition(const std::optional<DisplayId>& displayId) const override;

    // get the present fence received from the last call to present.
    sp<Fence> getPresentFence(DisplayId displayId) const override;

    // Get last release fence for the given layer
    sp<Fence> getLayerReleaseFence(DisplayId displayId, HWC2::Layer* layer) const override;

    // Set the output buffer and acquire fence for a virtual display.
    // Returns INVALID_OPERATION if displayId is not a virtual display.
    status_t setOutputBuffer(DisplayId displayId, const sp<Fence>& acquireFence,
                             const sp<GraphicBuffer>& buffer) override;

    // After SurfaceFlinger has retrieved the release fences for all the frames,
    // it can call this to clear the shared pointers in the release fence map
    void clearReleaseFences(DisplayId displayId) override;

    // Fetches the HDR capabilities of the given display
    status_t getHdrCapabilities(DisplayId displayId, HdrCapabilities* outCapabilities) override;

    int32_t getSupportedPerFrameMetadata(DisplayId displayId) const override;

    // Returns the available RenderIntent of the given display.
    std::vector<ui::RenderIntent> getRenderIntents(DisplayId displayId,
                                                   ui::ColorMode colorMode) const override;

    mat4 getDataspaceSaturationMatrix(DisplayId displayId, ui::Dataspace dataspace) override;

    // Returns the attributes of the color sampling engine.
    status_t getDisplayedContentSamplingAttributes(DisplayId displayId, ui::PixelFormat* outFormat,
                                                   ui::Dataspace* outDataspace,
                                                   uint8_t* outComponentMask) override;
    status_t setDisplayContentSamplingEnabled(DisplayId displayId, bool enabled,
                                              uint8_t componentMask, uint64_t maxFrames) override;
    status_t getDisplayedContentSample(DisplayId displayId, uint64_t maxFrames, uint64_t timestamp,
                                       DisplayedFrameStats* outStats) override;
    status_t setDisplayBrightness(DisplayId displayId, float brightness) override;

    // Events handling ---------------------------------------------------------

    // Returns stable display ID (and display name on connection of new or previously disconnected
    // display), or std::nullopt if hotplug event was ignored.
    std::optional<DisplayIdentificationInfo> onHotplug(hwc2_display_t hwcDisplayId,
                                                       HWC2::Connection connection) override;

    bool onVsync(hwc2_display_t hwcDisplayId, int64_t timestamp) override;
    void setVsyncEnabled(DisplayId displayId, HWC2::Vsync enabled) override;

    nsecs_t getRefreshTimestamp(DisplayId displayId) const override;
    bool isConnected(DisplayId displayId) const override;

    // Non-const because it can update configMap inside of mDisplayData
    std::vector<std::shared_ptr<const HWC2::Display::Config>> getConfigs(
            DisplayId displayId) const override;

    std::shared_ptr<const HWC2::Display::Config> getActiveConfig(
            DisplayId displayId) const override;
    int getActiveConfigIndex(DisplayId displayId) const override;

    std::vector<ui::ColorMode> getColorModes(DisplayId displayId) const override;

    status_t setActiveColorMode(DisplayId displayId, ui::ColorMode mode,
                                ui::RenderIntent renderIntent) override;

    bool isUsingVrComposer() const override;

    // for debugging ----------------------------------------------------------
    void dump(std::string& out) const override;

    Hwc2::Composer* getComposer() const override { return mHwcDevice->getComposer(); }

    // TODO(b/74619554): Remove special cases for internal/external display.
    std::optional<hwc2_display_t> getInternalHwcDisplayId() const override {
        return mInternalHwcDisplayId;
    }
    std::optional<hwc2_display_t> getExternalHwcDisplayId() const override {
        return mExternalHwcDisplayId;
    }

    std::optional<DisplayId> toPhysicalDisplayId(hwc2_display_t hwcDisplayId) const override;
    std::optional<hwc2_display_t> fromPhysicalDisplayId(DisplayId displayId) const override;

private:
    // For unit tests
    friend TestableSurfaceFlinger;

    std::optional<DisplayIdentificationInfo> onHotplugConnect(hwc2_display_t hwcDisplayId);

    static void validateChange(HWC2::Composition from, HWC2::Composition to);

    struct DisplayData {
        bool isVirtual = false;
        bool hasClientComposition = false;
        bool hasDeviceComposition = false;
        HWC2::Display* hwcDisplay = nullptr;
        HWC2::DisplayRequest displayRequests;
        sp<Fence> lastPresentFence = Fence::NO_FENCE; // signals when the last set op retires
        std::unordered_map<HWC2::Layer*, sp<Fence>> releaseFences;
        buffer_handle_t outbufHandle = nullptr;
        sp<Fence> outbufAcquireFence = Fence::NO_FENCE;
        mutable std::unordered_map<int32_t,
                std::shared_ptr<const HWC2::Display::Config>> configMap;

        bool validateWasSkipped;
        HWC2::Error presentError;

        bool vsyncTraceToggle = false;

        std::mutex vsyncEnabledLock;
        HWC2::Vsync vsyncEnabled GUARDED_BY(vsyncEnabledLock) = HWC2::Vsync::Disable;

        mutable std::mutex lastHwVsyncLock;
        nsecs_t lastHwVsync GUARDED_BY(lastHwVsyncLock) = 0;
    };

    std::unordered_map<DisplayId, DisplayData> mDisplayData;

    // This must be destroyed before mDisplayData, because destructor may call back into HWComposer
    // and look up DisplayData.
    std::unique_ptr<HWC2::Device> mHwcDevice;

    std::unordered_map<hwc2_display_t, DisplayId> mPhysicalDisplayIdMap;
    std::optional<hwc2_display_t> mInternalHwcDisplayId;
    std::optional<hwc2_display_t> mExternalHwcDisplayId;
    bool mHasMultiDisplaySupport = false;

    std::unordered_set<DisplayId> mFreeVirtualDisplayIds;
    uint32_t mNextVirtualDisplayId = 0;
    uint32_t mRemainingHwcVirtualDisplays{mHwcDevice->getMaxVirtualDisplayCount()};
};

} // namespace impl
} // namespace android

#endif // ANDROID_SF_HWCOMPOSER_H
