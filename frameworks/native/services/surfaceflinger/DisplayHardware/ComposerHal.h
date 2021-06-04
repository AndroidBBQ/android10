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

#ifndef ANDROID_SF_COMPOSER_HAL_H
#define ANDROID_SF_COMPOSER_HAL_H

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <android/frameworks/vr/composer/1.0/IVrComposerClient.h>
#include <android/hardware/graphics/common/1.1/types.h>
#include <android/hardware/graphics/composer/2.3/IComposer.h>
#include <android/hardware/graphics/composer/2.3/IComposerClient.h>
#include <composer-command-buffer/2.3/ComposerCommandBuffer.h>
#include <gui/HdrMetadata.h>
#include <math/mat4.h>
#include <ui/DisplayedFrameStats.h>
#include <ui/GraphicBuffer.h>
#include <utils/StrongPointer.h>

namespace android {

namespace Hwc2 {

using frameworks::vr::composer::V1_0::IVrComposerClient;

namespace types = hardware::graphics::common;

namespace V2_1 = hardware::graphics::composer::V2_1;
namespace V2_2 = hardware::graphics::composer::V2_2;
namespace V2_3 = hardware::graphics::composer::V2_3;

using types::V1_0::ColorTransform;
using types::V1_0::Transform;
using types::V1_1::RenderIntent;
using types::V1_2::ColorMode;
using types::V1_2::Dataspace;
using types::V1_2::Hdr;
using types::V1_2::PixelFormat;

using V2_1::Config;
using V2_1::Display;
using V2_1::Error;
using V2_1::IComposerCallback;
using V2_1::Layer;
using V2_3::CommandReaderBase;
using V2_3::CommandWriterBase;
using V2_3::IComposer;
using V2_3::IComposerClient;
using DisplayCapability = IComposerClient::DisplayCapability;
using PerFrameMetadata = IComposerClient::PerFrameMetadata;
using PerFrameMetadataKey = IComposerClient::PerFrameMetadataKey;
using PerFrameMetadataBlob = IComposerClient::PerFrameMetadataBlob;

class Composer {
public:
    virtual ~Composer() = 0;

    virtual std::vector<IComposer::Capability> getCapabilities() = 0;
    virtual std::string dumpDebugInfo() = 0;

    virtual void registerCallback(const sp<IComposerCallback>& callback) = 0;

    // Returns true if the connected composer service is running in a remote
    // process, false otherwise. This will return false if the service is
    // configured in passthrough mode, for example.
    virtual bool isRemote() = 0;

    // Reset all pending commands in the command buffer. Useful if you want to
    // skip a frame but have already queued some commands.
    virtual void resetCommands() = 0;

    // Explicitly flush all pending commands in the command buffer.
    virtual Error executeCommands() = 0;

    virtual uint32_t getMaxVirtualDisplayCount() = 0;
    virtual bool isUsingVrComposer() const = 0;
    virtual Error createVirtualDisplay(uint32_t width, uint32_t height, PixelFormat* format,
                                       Display* outDisplay) = 0;
    virtual Error destroyVirtualDisplay(Display display) = 0;

    virtual Error acceptDisplayChanges(Display display) = 0;

    virtual Error createLayer(Display display, Layer* outLayer) = 0;
    virtual Error destroyLayer(Display display, Layer layer) = 0;

    virtual Error getActiveConfig(Display display, Config* outConfig) = 0;
    virtual Error getChangedCompositionTypes(
            Display display, std::vector<Layer>* outLayers,
            std::vector<IComposerClient::Composition>* outTypes) = 0;
    virtual Error getColorModes(Display display, std::vector<ColorMode>* outModes) = 0;
    virtual Error getDisplayAttribute(Display display, Config config,
                                      IComposerClient::Attribute attribute, int32_t* outValue) = 0;
    virtual Error getDisplayConfigs(Display display, std::vector<Config>* outConfigs) = 0;
    virtual Error getDisplayName(Display display, std::string* outName) = 0;

    virtual Error getDisplayRequests(Display display, uint32_t* outDisplayRequestMask,
                                     std::vector<Layer>* outLayers,
                                     std::vector<uint32_t>* outLayerRequestMasks) = 0;

    virtual Error getDisplayType(Display display, IComposerClient::DisplayType* outType) = 0;
    virtual Error getDozeSupport(Display display, bool* outSupport) = 0;
    virtual Error getHdrCapabilities(Display display, std::vector<Hdr>* outTypes,
                                     float* outMaxLuminance, float* outMaxAverageLuminance,
                                     float* outMinLuminance) = 0;

    virtual Error getReleaseFences(Display display, std::vector<Layer>* outLayers,
                                   std::vector<int>* outReleaseFences) = 0;

    virtual Error presentDisplay(Display display, int* outPresentFence) = 0;

    virtual Error setActiveConfig(Display display, Config config) = 0;

    /*
     * The composer caches client targets internally.  When target is nullptr,
     * the composer uses slot to look up the client target from its cache.
     * When target is not nullptr, the cache is updated with the new target.
     */
    virtual Error setClientTarget(Display display, uint32_t slot, const sp<GraphicBuffer>& target,
                                  int acquireFence, Dataspace dataspace,
                                  const std::vector<IComposerClient::Rect>& damage) = 0;
    virtual Error setColorMode(Display display, ColorMode mode, RenderIntent renderIntent) = 0;
    virtual Error setColorTransform(Display display, const float* matrix, ColorTransform hint) = 0;
    virtual Error setOutputBuffer(Display display, const native_handle_t* buffer,
                                  int releaseFence) = 0;
    virtual Error setPowerMode(Display display, IComposerClient::PowerMode mode) = 0;
    virtual Error setVsyncEnabled(Display display, IComposerClient::Vsync enabled) = 0;

    virtual Error setClientTargetSlotCount(Display display) = 0;

    virtual Error validateDisplay(Display display, uint32_t* outNumTypes,
                                  uint32_t* outNumRequests) = 0;

    virtual Error presentOrValidateDisplay(Display display, uint32_t* outNumTypes,
                                           uint32_t* outNumRequests, int* outPresentFence,
                                           uint32_t* state) = 0;

    virtual Error setCursorPosition(Display display, Layer layer, int32_t x, int32_t y) = 0;
    /* see setClientTarget for the purpose of slot */
    virtual Error setLayerBuffer(Display display, Layer layer, uint32_t slot,
                                 const sp<GraphicBuffer>& buffer, int acquireFence) = 0;
    virtual Error setLayerSurfaceDamage(Display display, Layer layer,
                                        const std::vector<IComposerClient::Rect>& damage) = 0;
    virtual Error setLayerBlendMode(Display display, Layer layer,
                                    IComposerClient::BlendMode mode) = 0;
    virtual Error setLayerColor(Display display, Layer layer,
                                const IComposerClient::Color& color) = 0;
    virtual Error setLayerCompositionType(Display display, Layer layer,
                                          IComposerClient::Composition type) = 0;
    virtual Error setLayerDataspace(Display display, Layer layer, Dataspace dataspace) = 0;
    virtual Error setLayerDisplayFrame(Display display, Layer layer,
                                       const IComposerClient::Rect& frame) = 0;
    virtual Error setLayerPlaneAlpha(Display display, Layer layer, float alpha) = 0;
    virtual Error setLayerSidebandStream(Display display, Layer layer,
                                         const native_handle_t* stream) = 0;
    virtual Error setLayerSourceCrop(Display display, Layer layer,
                                     const IComposerClient::FRect& crop) = 0;
    virtual Error setLayerTransform(Display display, Layer layer, Transform transform) = 0;
    virtual Error setLayerVisibleRegion(Display display, Layer layer,
                                        const std::vector<IComposerClient::Rect>& visible) = 0;
    virtual Error setLayerZOrder(Display display, Layer layer, uint32_t z) = 0;
    virtual Error setLayerInfo(Display display, Layer layer, uint32_t type, uint32_t appId) = 0;

    // Composer HAL 2.2
    virtual Error setLayerPerFrameMetadata(
            Display display, Layer layer,
            const std::vector<IComposerClient::PerFrameMetadata>& perFrameMetadatas) = 0;
    virtual std::vector<IComposerClient::PerFrameMetadataKey> getPerFrameMetadataKeys(
            Display display) = 0;
    virtual Error getRenderIntents(Display display, ColorMode colorMode,
            std::vector<RenderIntent>* outRenderIntents) = 0;
    virtual Error getDataspaceSaturationMatrix(Dataspace dataspace, mat4* outMatrix) = 0;

    // Composer HAL 2.3
    virtual Error getDisplayIdentificationData(Display display, uint8_t* outPort,
                                               std::vector<uint8_t>* outData) = 0;
    virtual Error setLayerColorTransform(Display display, Layer layer,
                                         const float* matrix) = 0;
    virtual Error getDisplayedContentSamplingAttributes(Display display, PixelFormat* outFormat,
                                                        Dataspace* outDataspace,
                                                        uint8_t* outComponentMask) = 0;
    virtual Error setDisplayContentSamplingEnabled(Display display, bool enabled,
                                                   uint8_t componentMask, uint64_t maxFrames) = 0;
    virtual Error getDisplayedContentSample(Display display, uint64_t maxFrames, uint64_t timestamp,
                                            DisplayedFrameStats* outStats) = 0;
    virtual Error getDisplayCapabilities(Display display,
                                         std::vector<DisplayCapability>* outCapabilities) = 0;
    virtual Error setLayerPerFrameMetadataBlobs(
            Display display, Layer layer, const std::vector<PerFrameMetadataBlob>& metadata) = 0;
    virtual Error setDisplayBrightness(Display display, float brightness) = 0;
};

namespace impl {

class CommandReader : public CommandReaderBase {
public:
    ~CommandReader();

    // Parse and execute commands from the command queue.  The commands are
    // actually return values from the server and will be saved in ReturnData.
    Error parse();

    // Get and clear saved errors.
    struct CommandError {
        uint32_t location;
        Error error;
    };
    std::vector<CommandError> takeErrors();

    bool hasChanges(Display display, uint32_t* outNumChangedCompositionTypes,
            uint32_t* outNumLayerRequestMasks) const;

    // Get and clear saved changed composition types.
    void takeChangedCompositionTypes(Display display,
            std::vector<Layer>* outLayers,
            std::vector<IComposerClient::Composition>* outTypes);

    // Get and clear saved display requests.
    void takeDisplayRequests(Display display,
        uint32_t* outDisplayRequestMask, std::vector<Layer>* outLayers,
        std::vector<uint32_t>* outLayerRequestMasks);

    // Get and clear saved release fences.
    void takeReleaseFences(Display display, std::vector<Layer>* outLayers,
            std::vector<int>* outReleaseFences);

    // Get and clear saved present fence.
    void takePresentFence(Display display, int* outPresentFence);

    // Get what stage succeeded during PresentOrValidate: Present or Validate
    void takePresentOrValidateStage(Display display, uint32_t * state);

private:
    void resetData();

    bool parseSelectDisplay(uint16_t length);
    bool parseSetError(uint16_t length);
    bool parseSetChangedCompositionTypes(uint16_t length);
    bool parseSetDisplayRequests(uint16_t length);
    bool parseSetPresentFence(uint16_t length);
    bool parseSetReleaseFences(uint16_t length);
    bool parseSetPresentOrValidateDisplayResult(uint16_t length);

    struct ReturnData {
        uint32_t displayRequests = 0;

        std::vector<Layer> changedLayers;
        std::vector<IComposerClient::Composition> compositionTypes;

        std::vector<Layer> requestedLayers;
        std::vector<uint32_t> requestMasks;

        int presentFence = -1;

        std::vector<Layer> releasedLayers;
        std::vector<int> releaseFences;

        uint32_t presentOrValidateState;
    };

    std::vector<CommandError> mErrors;
    std::unordered_map<Display, ReturnData> mReturnData;

    // When SELECT_DISPLAY is parsed, this is updated to point to the
    // display's return data in mReturnData.  We use it to avoid repeated
    // map lookups.
    ReturnData* mCurrentReturnData;
};

// Composer is a wrapper to IComposer, a proxy to server-side composer.
class Composer final : public Hwc2::Composer {
public:
    explicit Composer(const std::string& serviceName);
    ~Composer() override;

    std::vector<IComposer::Capability> getCapabilities() override;
    std::string dumpDebugInfo() override;

    void registerCallback(const sp<IComposerCallback>& callback) override;

    // Returns true if the connected composer service is running in a remote
    // process, false otherwise. This will return false if the service is
    // configured in passthrough mode, for example.
    bool isRemote() override;

    // Reset all pending commands in the command buffer. Useful if you want to
    // skip a frame but have already queued some commands.
    void resetCommands() override;

    // Explicitly flush all pending commands in the command buffer.
    Error executeCommands() override;

    uint32_t getMaxVirtualDisplayCount() override;
    bool isUsingVrComposer() const override { return mIsUsingVrComposer; }
    Error createVirtualDisplay(uint32_t width, uint32_t height, PixelFormat* format,
                               Display* outDisplay) override;
    Error destroyVirtualDisplay(Display display) override;

    Error acceptDisplayChanges(Display display) override;

    Error createLayer(Display display, Layer* outLayer) override;
    Error destroyLayer(Display display, Layer layer) override;

    Error getActiveConfig(Display display, Config* outConfig) override;
    Error getChangedCompositionTypes(Display display, std::vector<Layer>* outLayers,
                                     std::vector<IComposerClient::Composition>* outTypes) override;
    Error getColorModes(Display display, std::vector<ColorMode>* outModes) override;
    Error getDisplayAttribute(Display display, Config config, IComposerClient::Attribute attribute,
                              int32_t* outValue) override;
    Error getDisplayConfigs(Display display, std::vector<Config>* outConfigs);
    Error getDisplayName(Display display, std::string* outName) override;

    Error getDisplayRequests(Display display, uint32_t* outDisplayRequestMask,
                             std::vector<Layer>* outLayers,
                             std::vector<uint32_t>* outLayerRequestMasks) override;

    Error getDisplayType(Display display, IComposerClient::DisplayType* outType) override;
    Error getDozeSupport(Display display, bool* outSupport) override;
    Error getHdrCapabilities(Display display, std::vector<Hdr>* outTypes, float* outMaxLuminance,
                             float* outMaxAverageLuminance, float* outMinLuminance) override;

    Error getReleaseFences(Display display, std::vector<Layer>* outLayers,
                           std::vector<int>* outReleaseFences) override;

    Error presentDisplay(Display display, int* outPresentFence) override;

    Error setActiveConfig(Display display, Config config) override;

    /*
     * The composer caches client targets internally.  When target is nullptr,
     * the composer uses slot to look up the client target from its cache.
     * When target is not nullptr, the cache is updated with the new target.
     */
    Error setClientTarget(Display display, uint32_t slot, const sp<GraphicBuffer>& target,
                          int acquireFence, Dataspace dataspace,
                          const std::vector<IComposerClient::Rect>& damage) override;
    Error setColorMode(Display display, ColorMode mode, RenderIntent renderIntent) override;
    Error setColorTransform(Display display, const float* matrix, ColorTransform hint) override;
    Error setOutputBuffer(Display display, const native_handle_t* buffer,
                          int releaseFence) override;
    Error setPowerMode(Display display, IComposerClient::PowerMode mode) override;
    Error setVsyncEnabled(Display display, IComposerClient::Vsync enabled) override;

    Error setClientTargetSlotCount(Display display) override;

    Error validateDisplay(Display display, uint32_t* outNumTypes,
                          uint32_t* outNumRequests) override;

    Error presentOrValidateDisplay(Display display, uint32_t* outNumTypes, uint32_t* outNumRequests,
                                   int* outPresentFence, uint32_t* state) override;

    Error setCursorPosition(Display display, Layer layer, int32_t x, int32_t y) override;
    /* see setClientTarget for the purpose of slot */
    Error setLayerBuffer(Display display, Layer layer, uint32_t slot,
                         const sp<GraphicBuffer>& buffer, int acquireFence) override;
    Error setLayerSurfaceDamage(Display display, Layer layer,
                                const std::vector<IComposerClient::Rect>& damage) override;
    Error setLayerBlendMode(Display display, Layer layer, IComposerClient::BlendMode mode) override;
    Error setLayerColor(Display display, Layer layer, const IComposerClient::Color& color) override;
    Error setLayerCompositionType(Display display, Layer layer,
                                  IComposerClient::Composition type) override;
    Error setLayerDataspace(Display display, Layer layer, Dataspace dataspace) override;
    Error setLayerDisplayFrame(Display display, Layer layer,
                               const IComposerClient::Rect& frame) override;
    Error setLayerPlaneAlpha(Display display, Layer layer, float alpha) override;
    Error setLayerSidebandStream(Display display, Layer layer,
                                 const native_handle_t* stream) override;
    Error setLayerSourceCrop(Display display, Layer layer,
                             const IComposerClient::FRect& crop) override;
    Error setLayerTransform(Display display, Layer layer, Transform transform) override;
    Error setLayerVisibleRegion(Display display, Layer layer,
                                const std::vector<IComposerClient::Rect>& visible) override;
    Error setLayerZOrder(Display display, Layer layer, uint32_t z) override;
    Error setLayerInfo(Display display, Layer layer, uint32_t type, uint32_t appId) override;

    // Composer HAL 2.2
    Error setLayerPerFrameMetadata(
            Display display, Layer layer,
            const std::vector<IComposerClient::PerFrameMetadata>& perFrameMetadatas) override;
    std::vector<IComposerClient::PerFrameMetadataKey> getPerFrameMetadataKeys(
            Display display) override;
    Error getRenderIntents(Display display, ColorMode colorMode,
            std::vector<RenderIntent>* outRenderIntents) override;
    Error getDataspaceSaturationMatrix(Dataspace dataspace, mat4* outMatrix) override;

    // Composer HAL 2.3
    Error getDisplayIdentificationData(Display display, uint8_t* outPort,
                                       std::vector<uint8_t>* outData) override;
    Error setLayerColorTransform(Display display, Layer layer, const float* matrix) override;
    Error getDisplayedContentSamplingAttributes(Display display, PixelFormat* outFormat,
                                                Dataspace* outDataspace,
                                                uint8_t* outComponentMask) override;
    Error setDisplayContentSamplingEnabled(Display display, bool enabled, uint8_t componentMask,
                                           uint64_t maxFrames) override;
    Error getDisplayedContentSample(Display display, uint64_t maxFrames, uint64_t timestamp,
                                    DisplayedFrameStats* outStats) override;
    Error getDisplayCapabilities(Display display,
                                 std::vector<DisplayCapability>* outCapabilities) override;
    Error setLayerPerFrameMetadataBlobs(
            Display display, Layer layer,
            const std::vector<IComposerClient::PerFrameMetadataBlob>& metadata) override;
    Error setDisplayBrightness(Display display, float brightness) override;

private:
    class CommandWriter : public CommandWriterBase {
    public:
        explicit CommandWriter(uint32_t initialMaxSize);
        ~CommandWriter() override;

        void setLayerInfo(uint32_t type, uint32_t appId);
        void setClientTargetMetadata(
                const IVrComposerClient::BufferMetadata& metadata);
        void setLayerBufferMetadata(
                const IVrComposerClient::BufferMetadata& metadata);

    private:
        void writeBufferMetadata(
                const IVrComposerClient::BufferMetadata& metadata);
    };

    // Many public functions above simply write a command into the command
    // queue to batch the calls.  validateDisplay and presentDisplay will call
    // this function to execute the command queue.
    Error execute();

    sp<V2_1::IComposer> mComposer;

    sp<V2_1::IComposerClient> mClient;
    sp<V2_2::IComposerClient> mClient_2_2;
    sp<IComposerClient> mClient_2_3;

    // 64KiB minus a small space for metadata such as read/write pointers
    static constexpr size_t kWriterInitialSize =
        64 * 1024 / sizeof(uint32_t) - 16;
    CommandWriter mWriter;
    CommandReader mReader;

    // When true, the we attach to the vr_hwcomposer service instead of the
    // hwcomposer. This allows us to redirect surfaces to 3d surfaces in vr.
    const bool mIsUsingVrComposer;
};

} // namespace impl

} // namespace Hwc2

} // namespace android

#endif // ANDROID_SF_COMPOSER_HAL_H
