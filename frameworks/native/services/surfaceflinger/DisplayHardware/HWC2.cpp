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

// #define LOG_NDEBUG 0

#undef LOG_TAG
#define LOG_TAG "HWC2"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "HWC2.h"
#include "ComposerHal.h"

#include <ui/Fence.h>
#include <ui/FloatRect.h>
#include <ui/GraphicBuffer.h>

#include <android/configuration.h>

#include <inttypes.h>
#include <algorithm>
#include <iterator>
#include <set>

using android::Fence;
using android::FloatRect;
using android::GraphicBuffer;
using android::HdrCapabilities;
using android::HdrMetadata;
using android::Rect;
using android::Region;
using android::sp;
using android::hardware::Return;
using android::hardware::Void;

namespace HWC2 {

namespace Hwc2 = android::Hwc2;
using android::ui::ColorMode;
using android::ui::Dataspace;
using android::ui::PixelFormat;
using android::ui::RenderIntent;

namespace {

inline bool hasMetadataKey(const std::set<Hwc2::PerFrameMetadataKey>& keys,
                           const Hwc2::PerFrameMetadataKey& key) {
    return keys.find(key) != keys.end();
}

class ComposerCallbackBridge : public Hwc2::IComposerCallback {
public:
    ComposerCallbackBridge(ComposerCallback* callback, int32_t sequenceId)
            : mCallback(callback), mSequenceId(sequenceId) {}

    Return<void> onHotplug(Hwc2::Display display,
                           IComposerCallback::Connection conn) override
    {
        HWC2::Connection connection = static_cast<HWC2::Connection>(conn);
        mCallback->onHotplugReceived(mSequenceId, display, connection);
        return Void();
    }

    Return<void> onRefresh(Hwc2::Display display) override
    {
        mCallback->onRefreshReceived(mSequenceId, display);
        return Void();
    }

    Return<void> onVsync(Hwc2::Display display, int64_t timestamp) override
    {
        mCallback->onVsyncReceived(mSequenceId, display, timestamp);
        return Void();
    }

private:
    ComposerCallback* mCallback;
    int32_t mSequenceId;
};

} // namespace anonymous


// Device methods

Device::Device(std::unique_ptr<android::Hwc2::Composer> composer) : mComposer(std::move(composer)) {
    loadCapabilities();
}

void Device::registerCallback(ComposerCallback* callback, int32_t sequenceId) {
    if (mRegisteredCallback) {
        ALOGW("Callback already registered. Ignored extra registration "
                "attempt.");
        return;
    }
    mRegisteredCallback = true;
    sp<ComposerCallbackBridge> callbackBridge(
            new ComposerCallbackBridge(callback, sequenceId));
    mComposer->registerCallback(callbackBridge);
}

// Required by HWC2 device

std::string Device::dump() const
{
    return mComposer->dumpDebugInfo();
}

uint32_t Device::getMaxVirtualDisplayCount() const
{
    return mComposer->getMaxVirtualDisplayCount();
}

Error Device::getDisplayIdentificationData(hwc2_display_t hwcDisplayId, uint8_t* outPort,
                                           std::vector<uint8_t>* outData) const {
    auto intError = mComposer->getDisplayIdentificationData(hwcDisplayId, outPort, outData);
    return static_cast<Error>(intError);
}

Error Device::createVirtualDisplay(uint32_t width, uint32_t height,
        PixelFormat* format, Display** outDisplay)
{
    ALOGI("Creating virtual display");

    hwc2_display_t displayId = 0;
    auto intError = mComposer->createVirtualDisplay(width, height,
            format, &displayId);
    auto error = static_cast<Error>(intError);
    if (error != Error::None) {
        return error;
    }

    auto display = std::make_unique<impl::Display>(*mComposer.get(), mCapabilities, displayId,
                                                   DisplayType::Virtual);
    display->setConnected(true);
    *outDisplay = display.get();
    mDisplays.emplace(displayId, std::move(display));
    ALOGI("Created virtual display");
    return Error::None;
}

void Device::destroyDisplay(hwc2_display_t displayId)
{
    ALOGI("Destroying display %" PRIu64, displayId);
    mDisplays.erase(displayId);
}

void Device::onHotplug(hwc2_display_t displayId, Connection connection) {
    if (connection == Connection::Connected) {
        // If we get a hotplug connected event for a display we already have,
        // destroy the display and recreate it. This will force us to requery
        // the display params and recreate all layers on that display.
        auto oldDisplay = getDisplayById(displayId);
        if (oldDisplay != nullptr && oldDisplay->isConnected()) {
            ALOGI("Hotplug connecting an already connected display."
                    " Clearing old display state.");
        }
        mDisplays.erase(displayId);

        DisplayType displayType;
        auto intError = mComposer->getDisplayType(displayId,
                reinterpret_cast<Hwc2::IComposerClient::DisplayType *>(
                        &displayType));
        auto error = static_cast<Error>(intError);
        if (error != Error::None) {
            ALOGE("getDisplayType(%" PRIu64 ") failed: %s (%d). "
                    "Aborting hotplug attempt.",
                    displayId, to_string(error).c_str(), intError);
            return;
        }

        auto newDisplay = std::make_unique<impl::Display>(*mComposer.get(), mCapabilities,
                                                          displayId, displayType);
        newDisplay->setConnected(true);
        mDisplays.emplace(displayId, std::move(newDisplay));
    } else if (connection == Connection::Disconnected) {
        // The display will later be destroyed by a call to
        // destroyDisplay(). For now we just mark it disconnected.
        auto display = getDisplayById(displayId);
        if (display) {
            display->setConnected(false);
        } else {
            ALOGW("Attempted to disconnect unknown display %" PRIu64,
                  displayId);
        }
    }
}

// Other Device methods

Display* Device::getDisplayById(hwc2_display_t id) {
    auto iter = mDisplays.find(id);
    return iter == mDisplays.end() ? nullptr : iter->second.get();
}

// Device initialization methods

void Device::loadCapabilities()
{
    static_assert(sizeof(Capability) == sizeof(int32_t),
            "Capability size has changed");
    auto capabilities = mComposer->getCapabilities();
    for (auto capability : capabilities) {
        mCapabilities.emplace(static_cast<Capability>(capability));
    }
}

Error Device::flushCommands()
{
    return static_cast<Error>(mComposer->executeCommands());
}

// Display methods
Display::~Display() = default;

Display::Config::Config(Display& display, hwc2_config_t id)
      : mDisplay(display),
        mId(id),
        mWidth(-1),
        mHeight(-1),
        mVsyncPeriod(-1),
        mDpiX(-1),
        mDpiY(-1) {}

Display::Config::Builder::Builder(Display& display, hwc2_config_t id)
      : mConfig(new Config(display, id)) {}

float Display::Config::Builder::getDefaultDensity() {
    // Default density is based on TVs: 1080p displays get XHIGH density, lower-
    // resolution displays get TV density. Maybe eventually we'll need to update
    // it for 4k displays, though hopefully those will just report accurate DPI
    // information to begin with. This is also used for virtual displays and
    // older HWC implementations, so be careful about orientation.

    auto longDimension = std::max(mConfig->mWidth, mConfig->mHeight);
    if (longDimension >= 1080) {
        return ACONFIGURATION_DENSITY_XHIGH;
    } else {
        return ACONFIGURATION_DENSITY_TV;
    }
}

namespace impl {
Display::Display(android::Hwc2::Composer& composer,
                 const std::unordered_set<Capability>& capabilities, hwc2_display_t id,
                 DisplayType type)
      : mComposer(composer),
        mCapabilities(capabilities),
        mId(id),
        mIsConnected(false),
        mType(type) {
    ALOGV("Created display %" PRIu64, id);
}

Display::~Display() {
    mLayers.clear();

    if (mType == DisplayType::Virtual) {
        ALOGV("Destroying virtual display");
        auto intError = mComposer.destroyVirtualDisplay(mId);
        auto error = static_cast<Error>(intError);
        ALOGE_IF(error != Error::None, "destroyVirtualDisplay(%" PRIu64
                ") failed: %s (%d)", mId, to_string(error).c_str(), intError);
    } else if (mType == DisplayType::Physical) {
        auto error = setVsyncEnabled(HWC2::Vsync::Disable);
        if (error != Error::None) {
            ALOGE("~Display: Failed to disable vsync for display %" PRIu64
                    ": %s (%d)", mId, to_string(error).c_str(),
                    static_cast<int32_t>(error));
        }
    }
}

// Required by HWC2 display
Error Display::acceptChanges()
{
    auto intError = mComposer.acceptDisplayChanges(mId);
    return static_cast<Error>(intError);
}

Error Display::createLayer(HWC2::Layer** outLayer) {
    if (!outLayer) {
        return Error::BadParameter;
    }
    hwc2_layer_t layerId = 0;
    auto intError = mComposer.createLayer(mId, &layerId);
    auto error = static_cast<Error>(intError);
    if (error != Error::None) {
        return error;
    }

    auto layer = std::make_unique<impl::Layer>(mComposer, mCapabilities, mId, layerId);
    *outLayer = layer.get();
    mLayers.emplace(layerId, std::move(layer));
    return Error::None;
}

Error Display::destroyLayer(HWC2::Layer* layer) {
    if (!layer) {
        return Error::BadParameter;
    }
    mLayers.erase(layer->getId());
    return Error::None;
}

Error Display::getActiveConfig(
        std::shared_ptr<const Display::Config>* outConfig) const
{
    ALOGV("[%" PRIu64 "] getActiveConfig", mId);
    hwc2_config_t configId = 0;
    auto intError = mComposer.getActiveConfig(mId, &configId);
    auto error = static_cast<Error>(intError);

    if (error != Error::None) {
        ALOGE("Unable to get active config for mId:[%" PRIu64 "]", mId);
        *outConfig = nullptr;
        return error;
    }

    if (mConfigs.count(configId) != 0) {
        *outConfig = mConfigs.at(configId);
    } else {
        ALOGE("[%" PRIu64 "] getActiveConfig returned unknown config %u", mId,
                configId);
        // Return no error, but the caller needs to check for a null pointer to
        // detect this case
        *outConfig = nullptr;
    }

    return Error::None;
}

Error Display::getActiveConfigIndex(int* outIndex) const {
    ALOGV("[%" PRIu64 "] getActiveConfigIndex", mId);
    hwc2_config_t configId = 0;
    auto intError = mComposer.getActiveConfig(mId, &configId);
    auto error = static_cast<Error>(intError);

    if (error != Error::None) {
        ALOGE("Unable to get active config for mId:[%" PRIu64 "]", mId);
        *outIndex = -1;
        return error;
    }

    auto pos = mConfigs.find(configId);
    if (pos != mConfigs.end()) {
        *outIndex = std::distance(mConfigs.begin(), pos);
    } else {
        ALOGE("[%" PRIu64 "] getActiveConfig returned unknown config %u", mId, configId);
        // Return no error, but the caller needs to check for a negative index
        // to detect this case
        *outIndex = -1;
    }

    return Error::None;
}

Error Display::getChangedCompositionTypes(std::unordered_map<HWC2::Layer*, Composition>* outTypes) {
    std::vector<Hwc2::Layer> layerIds;
    std::vector<Hwc2::IComposerClient::Composition> types;
    auto intError = mComposer.getChangedCompositionTypes(
            mId, &layerIds, &types);
    uint32_t numElements = layerIds.size();
    auto error = static_cast<Error>(intError);
    error = static_cast<Error>(intError);
    if (error != Error::None) {
        return error;
    }

    outTypes->clear();
    outTypes->reserve(numElements);
    for (uint32_t element = 0; element < numElements; ++element) {
        auto layer = getLayerById(layerIds[element]);
        if (layer) {
            auto type = static_cast<Composition>(types[element]);
            ALOGV("getChangedCompositionTypes: adding %" PRIu64 " %s",
                    layer->getId(), to_string(type).c_str());
            outTypes->emplace(layer, type);
        } else {
            ALOGE("getChangedCompositionTypes: invalid layer %" PRIu64 " found"
                    " on display %" PRIu64, layerIds[element], mId);
        }
    }

    return Error::None;
}

Error Display::getColorModes(std::vector<ColorMode>* outModes) const
{
    auto intError = mComposer.getColorModes(mId, outModes);
    return static_cast<Error>(intError);
}

int32_t Display::getSupportedPerFrameMetadata() const
{
    int32_t supportedPerFrameMetadata = 0;

    std::vector<Hwc2::PerFrameMetadataKey> tmpKeys = mComposer.getPerFrameMetadataKeys(mId);
    std::set<Hwc2::PerFrameMetadataKey> keys(tmpKeys.begin(), tmpKeys.end());

    // Check whether a specific metadata type is supported. A metadata type is considered
    // supported if and only if all required fields are supported.

    // SMPTE2086
    if (hasMetadataKey(keys, Hwc2::PerFrameMetadataKey::DISPLAY_RED_PRIMARY_X) &&
        hasMetadataKey(keys, Hwc2::PerFrameMetadataKey::DISPLAY_RED_PRIMARY_Y) &&
        hasMetadataKey(keys, Hwc2::PerFrameMetadataKey::DISPLAY_GREEN_PRIMARY_X) &&
        hasMetadataKey(keys, Hwc2::PerFrameMetadataKey::DISPLAY_GREEN_PRIMARY_Y) &&
        hasMetadataKey(keys, Hwc2::PerFrameMetadataKey::DISPLAY_BLUE_PRIMARY_X) &&
        hasMetadataKey(keys, Hwc2::PerFrameMetadataKey::DISPLAY_BLUE_PRIMARY_Y) &&
        hasMetadataKey(keys, Hwc2::PerFrameMetadataKey::WHITE_POINT_X) &&
        hasMetadataKey(keys, Hwc2::PerFrameMetadataKey::WHITE_POINT_Y) &&
        hasMetadataKey(keys, Hwc2::PerFrameMetadataKey::MAX_LUMINANCE) &&
        hasMetadataKey(keys, Hwc2::PerFrameMetadataKey::MIN_LUMINANCE)) {
        supportedPerFrameMetadata |= HdrMetadata::Type::SMPTE2086;
    }
    // CTA861_3
    if (hasMetadataKey(keys, Hwc2::PerFrameMetadataKey::MAX_CONTENT_LIGHT_LEVEL) &&
        hasMetadataKey(keys, Hwc2::PerFrameMetadataKey::MAX_FRAME_AVERAGE_LIGHT_LEVEL)) {
        supportedPerFrameMetadata |= HdrMetadata::Type::CTA861_3;
    }

    // HDR10PLUS
    if (hasMetadataKey(keys, Hwc2::PerFrameMetadataKey::HDR10_PLUS_SEI)) {
        supportedPerFrameMetadata |= HdrMetadata::Type::HDR10PLUS;
    }

    return supportedPerFrameMetadata;
}

Error Display::getRenderIntents(ColorMode colorMode,
        std::vector<RenderIntent>* outRenderIntents) const
{
    auto intError = mComposer.getRenderIntents(mId, colorMode, outRenderIntents);
    return static_cast<Error>(intError);
}

Error Display::getDataspaceSaturationMatrix(Dataspace dataspace, android::mat4* outMatrix)
{
    auto intError = mComposer.getDataspaceSaturationMatrix(dataspace, outMatrix);
    return static_cast<Error>(intError);
}

std::vector<std::shared_ptr<const Display::Config>> Display::getConfigs() const
{
    std::vector<std::shared_ptr<const Config>> configs;
    for (const auto& element : mConfigs) {
        configs.emplace_back(element.second);
    }
    return configs;
}

Error Display::getName(std::string* outName) const
{
    auto intError = mComposer.getDisplayName(mId, outName);
    return static_cast<Error>(intError);
}

Error Display::getRequests(HWC2::DisplayRequest* outDisplayRequests,
                           std::unordered_map<HWC2::Layer*, LayerRequest>* outLayerRequests) {
    uint32_t intDisplayRequests;
    std::vector<Hwc2::Layer> layerIds;
    std::vector<uint32_t> layerRequests;
    auto intError = mComposer.getDisplayRequests(
            mId, &intDisplayRequests, &layerIds, &layerRequests);
    uint32_t numElements = layerIds.size();
    auto error = static_cast<Error>(intError);
    if (error != Error::None) {
        return error;
    }

    *outDisplayRequests = static_cast<DisplayRequest>(intDisplayRequests);
    outLayerRequests->clear();
    outLayerRequests->reserve(numElements);
    for (uint32_t element = 0; element < numElements; ++element) {
        auto layer = getLayerById(layerIds[element]);
        if (layer) {
            auto layerRequest =
                    static_cast<LayerRequest>(layerRequests[element]);
            outLayerRequests->emplace(layer, layerRequest);
        } else {
            ALOGE("getRequests: invalid layer %" PRIu64 " found on display %"
                    PRIu64, layerIds[element], mId);
        }
    }

    return Error::None;
}

Error Display::getType(DisplayType* outType) const
{
    *outType = mType;
    return Error::None;
}

Error Display::supportsDoze(bool* outSupport) const {
    *outSupport = mDisplayCapabilities.count(DisplayCapability::Doze) > 0;
    return Error::None;
}

Error Display::getHdrCapabilities(HdrCapabilities* outCapabilities) const
{
    float maxLuminance = -1.0f;
    float maxAverageLuminance = -1.0f;
    float minLuminance = -1.0f;
    std::vector<Hwc2::Hdr> types;
    auto intError = mComposer.getHdrCapabilities(mId, &types,
            &maxLuminance, &maxAverageLuminance, &minLuminance);
    auto error = static_cast<HWC2::Error>(intError);

    if (error != Error::None) {
        return error;
    }

    *outCapabilities = HdrCapabilities(std::move(types),
            maxLuminance, maxAverageLuminance, minLuminance);
    return Error::None;
}

Error Display::getDisplayedContentSamplingAttributes(PixelFormat* outFormat,
                                                     Dataspace* outDataspace,
                                                     uint8_t* outComponentMask) const {
    auto intError = mComposer.getDisplayedContentSamplingAttributes(mId, outFormat, outDataspace,
                                                                    outComponentMask);
    return static_cast<Error>(intError);
}

Error Display::setDisplayContentSamplingEnabled(bool enabled, uint8_t componentMask,
                                                uint64_t maxFrames) const {
    auto intError =
            mComposer.setDisplayContentSamplingEnabled(mId, enabled, componentMask, maxFrames);
    return static_cast<Error>(intError);
}

Error Display::getDisplayedContentSample(uint64_t maxFrames, uint64_t timestamp,
                                         android::DisplayedFrameStats* outStats) const {
    auto intError = mComposer.getDisplayedContentSample(mId, maxFrames, timestamp, outStats);
    return static_cast<Error>(intError);
}

Error Display::getReleaseFences(std::unordered_map<HWC2::Layer*, sp<Fence>>* outFences) const {
    std::vector<Hwc2::Layer> layerIds;
    std::vector<int> fenceFds;
    auto intError = mComposer.getReleaseFences(mId, &layerIds, &fenceFds);
    auto error = static_cast<Error>(intError);
    uint32_t numElements = layerIds.size();
    if (error != Error::None) {
        return error;
    }

    std::unordered_map<HWC2::Layer*, sp<Fence>> releaseFences;
    releaseFences.reserve(numElements);
    for (uint32_t element = 0; element < numElements; ++element) {
        auto layer = getLayerById(layerIds[element]);
        if (layer) {
            sp<Fence> fence(new Fence(fenceFds[element]));
            releaseFences.emplace(layer, fence);
        } else {
            ALOGE("getReleaseFences: invalid layer %" PRIu64
                    " found on display %" PRIu64, layerIds[element], mId);
            for (; element < numElements; ++element) {
                close(fenceFds[element]);
            }
            return Error::BadLayer;
        }
    }

    *outFences = std::move(releaseFences);
    return Error::None;
}

Error Display::present(sp<Fence>* outPresentFence)
{
    int32_t presentFenceFd = -1;
    auto intError = mComposer.presentDisplay(mId, &presentFenceFd);
    auto error = static_cast<Error>(intError);
    if (error != Error::None) {
        return error;
    }

    *outPresentFence = new Fence(presentFenceFd);
    return Error::None;
}

Error Display::setActiveConfig(const std::shared_ptr<const Config>& config)
{
    if (config->getDisplayId() != mId) {
        ALOGE("setActiveConfig received config %u for the wrong display %"
                PRIu64 " (expected %" PRIu64 ")", config->getId(),
                config->getDisplayId(), mId);
        return Error::BadConfig;
    }
    auto intError = mComposer.setActiveConfig(mId, config->getId());
    return static_cast<Error>(intError);
}

Error Display::setClientTarget(uint32_t slot, const sp<GraphicBuffer>& target,
        const sp<Fence>& acquireFence, Dataspace dataspace)
{
    // TODO: Properly encode client target surface damage
    int32_t fenceFd = acquireFence->dup();
    auto intError = mComposer.setClientTarget(mId, slot, target,
            fenceFd, dataspace, std::vector<Hwc2::IComposerClient::Rect>());
    return static_cast<Error>(intError);
}

Error Display::setColorMode(ColorMode mode, RenderIntent renderIntent)
{
    auto intError = mComposer.setColorMode(mId, mode, renderIntent);
    return static_cast<Error>(intError);
}

Error Display::setColorTransform(const android::mat4& matrix,
        android_color_transform_t hint)
{
    auto intError = mComposer.setColorTransform(mId,
            matrix.asArray(), static_cast<Hwc2::ColorTransform>(hint));
    return static_cast<Error>(intError);
}

Error Display::setOutputBuffer(const sp<GraphicBuffer>& buffer,
        const sp<Fence>& releaseFence)
{
    int32_t fenceFd = releaseFence->dup();
    auto handle = buffer->getNativeBuffer()->handle;
    auto intError = mComposer.setOutputBuffer(mId, handle, fenceFd);
    close(fenceFd);
    return static_cast<Error>(intError);
}

Error Display::setPowerMode(PowerMode mode)
{
    auto intMode = static_cast<Hwc2::IComposerClient::PowerMode>(mode);
    auto intError = mComposer.setPowerMode(mId, intMode);

    if (mode == PowerMode::On) {
        std::call_once(mDisplayCapabilityQueryFlag, [this]() {
            std::vector<Hwc2::DisplayCapability> tmpCapabilities;
            auto error =
                    static_cast<Error>(mComposer.getDisplayCapabilities(mId, &tmpCapabilities));
            if (error == Error::None) {
                for (auto capability : tmpCapabilities) {
                    mDisplayCapabilities.emplace(static_cast<DisplayCapability>(capability));
                }
            } else if (error == Error::Unsupported) {
                if (mCapabilities.count(Capability::SkipClientColorTransform)) {
                    mDisplayCapabilities.emplace(DisplayCapability::SkipClientColorTransform);
                }
                bool dozeSupport = false;
                error = static_cast<Error>(mComposer.getDozeSupport(mId, &dozeSupport));
                if (error == Error::None && dozeSupport) {
                    mDisplayCapabilities.emplace(DisplayCapability::Doze);
                }
            }
        });
    }

    return static_cast<Error>(intError);
}

Error Display::setVsyncEnabled(Vsync enabled)
{
    auto intEnabled = static_cast<Hwc2::IComposerClient::Vsync>(enabled);
    auto intError = mComposer.setVsyncEnabled(mId, intEnabled);
    return static_cast<Error>(intError);
}

Error Display::validate(uint32_t* outNumTypes, uint32_t* outNumRequests)
{
    uint32_t numTypes = 0;
    uint32_t numRequests = 0;
    auto intError = mComposer.validateDisplay(mId, &numTypes, &numRequests);
    auto error = static_cast<Error>(intError);
    if (error != Error::None && error != Error::HasChanges) {
        return error;
    }

    *outNumTypes = numTypes;
    *outNumRequests = numRequests;
    return error;
}

Error Display::presentOrValidate(uint32_t* outNumTypes, uint32_t* outNumRequests,
                                 sp<android::Fence>* outPresentFence, uint32_t* state) {

    uint32_t numTypes = 0;
    uint32_t numRequests = 0;
    int32_t presentFenceFd = -1;
    auto intError = mComposer.presentOrValidateDisplay(
            mId, &numTypes, &numRequests, &presentFenceFd, state);
    auto error = static_cast<Error>(intError);
    if (error != Error::None && error != Error::HasChanges) {
        return error;
    }

    if (*state == 1) {
        *outPresentFence = new Fence(presentFenceFd);
    }

    if (*state == 0) {
        *outNumTypes = numTypes;
        *outNumRequests = numRequests;
    }
    return error;
}

Error Display::setDisplayBrightness(float brightness) const {
    auto intError = mComposer.setDisplayBrightness(mId, brightness);
    return static_cast<Error>(intError);
}

// For use by Device

void Display::setConnected(bool connected) {
    if (!mIsConnected && connected) {
        mComposer.setClientTargetSlotCount(mId);
        if (mType == DisplayType::Physical) {
            loadConfigs();
        }
    }
    mIsConnected = connected;
}

int32_t Display::getAttribute(hwc2_config_t configId, Attribute attribute)
{
    int32_t value = 0;
    auto intError = mComposer.getDisplayAttribute(mId, configId,
            static_cast<Hwc2::IComposerClient::Attribute>(attribute),
            &value);
    auto error = static_cast<Error>(intError);
    if (error != Error::None) {
        ALOGE("getDisplayAttribute(%" PRIu64 ", %u, %s) failed: %s (%d)", mId,
                configId, to_string(attribute).c_str(),
                to_string(error).c_str(), intError);
        return -1;
    }
    return value;
}

void Display::loadConfig(hwc2_config_t configId)
{
    ALOGV("[%" PRIu64 "] loadConfig(%u)", mId, configId);

    auto config = Config::Builder(*this, configId)
            .setWidth(getAttribute(configId, Attribute::Width))
            .setHeight(getAttribute(configId, Attribute::Height))
            .setVsyncPeriod(getAttribute(configId, Attribute::VsyncPeriod))
            .setDpiX(getAttribute(configId, Attribute::DpiX))
            .setDpiY(getAttribute(configId, Attribute::DpiY))
            .build();
    mConfigs.emplace(configId, std::move(config));
}

void Display::loadConfigs()
{
    ALOGV("[%" PRIu64 "] loadConfigs", mId);

    std::vector<Hwc2::Config> configIds;
    auto intError = mComposer.getDisplayConfigs(mId, &configIds);
    auto error = static_cast<Error>(intError);
    if (error != Error::None) {
        ALOGE("[%" PRIu64 "] getDisplayConfigs [2] failed: %s (%d)", mId,
                to_string(error).c_str(), intError);
        return;
    }

    for (auto configId : configIds) {
        loadConfig(configId);
    }
}

// Other Display methods

HWC2::Layer* Display::getLayerById(hwc2_layer_t id) const {
    if (mLayers.count(id) == 0) {
        return nullptr;
    }

    return mLayers.at(id).get();
}
} // namespace impl

// Layer methods

Layer::~Layer() = default;

namespace impl {

Layer::Layer(android::Hwc2::Composer& composer, const std::unordered_set<Capability>& capabilities,
             hwc2_display_t displayId, hwc2_layer_t layerId)
  : mComposer(composer),
    mCapabilities(capabilities),
    mDisplayId(displayId),
    mId(layerId),
    mColorMatrix(android::mat4())
{
    ALOGV("Created layer %" PRIu64 " on display %" PRIu64, layerId, displayId);
}

Layer::~Layer()
{
    auto intError = mComposer.destroyLayer(mDisplayId, mId);
    auto error = static_cast<Error>(intError);
    ALOGE_IF(error != Error::None, "destroyLayer(%" PRIu64 ", %" PRIu64 ")"
            " failed: %s (%d)", mDisplayId, mId, to_string(error).c_str(),
            intError);
}

Error Layer::setCursorPosition(int32_t x, int32_t y)
{
    auto intError = mComposer.setCursorPosition(mDisplayId, mId, x, y);
    return static_cast<Error>(intError);
}

Error Layer::setBuffer(uint32_t slot, const sp<GraphicBuffer>& buffer,
        const sp<Fence>& acquireFence)
{
    if (buffer == nullptr && mBufferSlot == slot) {
        return Error::None;
    }
    mBufferSlot = slot;

    int32_t fenceFd = acquireFence->dup();
    auto intError = mComposer.setLayerBuffer(mDisplayId, mId, slot, buffer,
                                             fenceFd);
    return static_cast<Error>(intError);
}

Error Layer::setSurfaceDamage(const Region& damage)
{
    if (damage.isRect() && mDamageRegion.isRect() &&
        (damage.getBounds() == mDamageRegion.getBounds())) {
        return Error::None;
    }
    mDamageRegion = damage;

    // We encode default full-screen damage as INVALID_RECT upstream, but as 0
    // rects for HWC
    Hwc2::Error intError = Hwc2::Error::NONE;
    if (damage.isRect() && damage.getBounds() == Rect::INVALID_RECT) {
        intError = mComposer.setLayerSurfaceDamage(mDisplayId,
                mId, std::vector<Hwc2::IComposerClient::Rect>());
    } else {
        size_t rectCount = 0;
        auto rectArray = damage.getArray(&rectCount);

        std::vector<Hwc2::IComposerClient::Rect> hwcRects;
        for (size_t rect = 0; rect < rectCount; ++rect) {
            hwcRects.push_back({rectArray[rect].left, rectArray[rect].top,
                    rectArray[rect].right, rectArray[rect].bottom});
        }

        intError = mComposer.setLayerSurfaceDamage(mDisplayId, mId, hwcRects);
    }

    return static_cast<Error>(intError);
}

Error Layer::setBlendMode(BlendMode mode)
{
    auto intMode = static_cast<Hwc2::IComposerClient::BlendMode>(mode);
    auto intError = mComposer.setLayerBlendMode(mDisplayId, mId, intMode);
    return static_cast<Error>(intError);
}

Error Layer::setColor(hwc_color_t color)
{
    Hwc2::IComposerClient::Color hwcColor{color.r, color.g, color.b, color.a};
    auto intError = mComposer.setLayerColor(mDisplayId, mId, hwcColor);
    return static_cast<Error>(intError);
}

Error Layer::setCompositionType(Composition type)
{
    auto intType = static_cast<Hwc2::IComposerClient::Composition>(type);
    auto intError = mComposer.setLayerCompositionType(
            mDisplayId, mId, intType);
    return static_cast<Error>(intError);
}

Error Layer::setDataspace(Dataspace dataspace)
{
    if (dataspace == mDataSpace) {
        return Error::None;
    }
    mDataSpace = dataspace;
    auto intError = mComposer.setLayerDataspace(mDisplayId, mId, mDataSpace);
    return static_cast<Error>(intError);
}

Error Layer::setPerFrameMetadata(const int32_t supportedPerFrameMetadata,
        const android::HdrMetadata& metadata)
{
    if (metadata == mHdrMetadata) {
        return Error::None;
    }

    mHdrMetadata = metadata;
    int validTypes = mHdrMetadata.validTypes & supportedPerFrameMetadata;
    std::vector<Hwc2::PerFrameMetadata> perFrameMetadatas;
    if (validTypes & HdrMetadata::SMPTE2086) {
        perFrameMetadatas.insert(perFrameMetadatas.end(),
                                 {{Hwc2::PerFrameMetadataKey::DISPLAY_RED_PRIMARY_X,
                                   mHdrMetadata.smpte2086.displayPrimaryRed.x},
                                  {Hwc2::PerFrameMetadataKey::DISPLAY_RED_PRIMARY_Y,
                                   mHdrMetadata.smpte2086.displayPrimaryRed.y},
                                  {Hwc2::PerFrameMetadataKey::DISPLAY_GREEN_PRIMARY_X,
                                   mHdrMetadata.smpte2086.displayPrimaryGreen.x},
                                  {Hwc2::PerFrameMetadataKey::DISPLAY_GREEN_PRIMARY_Y,
                                   mHdrMetadata.smpte2086.displayPrimaryGreen.y},
                                  {Hwc2::PerFrameMetadataKey::DISPLAY_BLUE_PRIMARY_X,
                                   mHdrMetadata.smpte2086.displayPrimaryBlue.x},
                                  {Hwc2::PerFrameMetadataKey::DISPLAY_BLUE_PRIMARY_Y,
                                   mHdrMetadata.smpte2086.displayPrimaryBlue.y},
                                  {Hwc2::PerFrameMetadataKey::WHITE_POINT_X,
                                   mHdrMetadata.smpte2086.whitePoint.x},
                                  {Hwc2::PerFrameMetadataKey::WHITE_POINT_Y,
                                   mHdrMetadata.smpte2086.whitePoint.y},
                                  {Hwc2::PerFrameMetadataKey::MAX_LUMINANCE,
                                   mHdrMetadata.smpte2086.maxLuminance},
                                  {Hwc2::PerFrameMetadataKey::MIN_LUMINANCE,
                                   mHdrMetadata.smpte2086.minLuminance}});
    }

    if (validTypes & HdrMetadata::CTA861_3) {
        perFrameMetadatas.insert(perFrameMetadatas.end(),
                                 {{Hwc2::PerFrameMetadataKey::MAX_CONTENT_LIGHT_LEVEL,
                                   mHdrMetadata.cta8613.maxContentLightLevel},
                                  {Hwc2::PerFrameMetadataKey::MAX_FRAME_AVERAGE_LIGHT_LEVEL,
                                   mHdrMetadata.cta8613.maxFrameAverageLightLevel}});
    }

    Error error = static_cast<Error>(
            mComposer.setLayerPerFrameMetadata(mDisplayId, mId, perFrameMetadatas));

    if (validTypes & HdrMetadata::HDR10PLUS) {
        std::vector<Hwc2::PerFrameMetadataBlob> perFrameMetadataBlobs;
        perFrameMetadataBlobs.push_back(
                {Hwc2::PerFrameMetadataKey::HDR10_PLUS_SEI, mHdrMetadata.hdr10plus});
        Error setMetadataBlobsError = static_cast<Error>(
                mComposer.setLayerPerFrameMetadataBlobs(mDisplayId, mId, perFrameMetadataBlobs));
        if (error == Error::None) {
            return setMetadataBlobsError;
        }
    }
    return error;
}

Error Layer::setDisplayFrame(const Rect& frame)
{
    Hwc2::IComposerClient::Rect hwcRect{frame.left, frame.top,
        frame.right, frame.bottom};
    auto intError = mComposer.setLayerDisplayFrame(mDisplayId, mId, hwcRect);
    return static_cast<Error>(intError);
}

Error Layer::setPlaneAlpha(float alpha)
{
    auto intError = mComposer.setLayerPlaneAlpha(mDisplayId, mId, alpha);
    return static_cast<Error>(intError);
}

Error Layer::setSidebandStream(const native_handle_t* stream)
{
    if (mCapabilities.count(Capability::SidebandStream) == 0) {
        ALOGE("Attempted to call setSidebandStream without checking that the "
                "device supports sideband streams");
        return Error::Unsupported;
    }
    auto intError = mComposer.setLayerSidebandStream(mDisplayId, mId, stream);
    return static_cast<Error>(intError);
}

Error Layer::setSourceCrop(const FloatRect& crop)
{
    Hwc2::IComposerClient::FRect hwcRect{
        crop.left, crop.top, crop.right, crop.bottom};
    auto intError = mComposer.setLayerSourceCrop(mDisplayId, mId, hwcRect);
    return static_cast<Error>(intError);
}

Error Layer::setTransform(Transform transform)
{
    auto intTransform = static_cast<Hwc2::Transform>(transform);
    auto intError = mComposer.setLayerTransform(mDisplayId, mId, intTransform);
    return static_cast<Error>(intError);
}

Error Layer::setVisibleRegion(const Region& region)
{
    if (region.isRect() && mVisibleRegion.isRect() &&
        (region.getBounds() == mVisibleRegion.getBounds())) {
        return Error::None;
    }
    mVisibleRegion = region;

    size_t rectCount = 0;
    auto rectArray = region.getArray(&rectCount);

    std::vector<Hwc2::IComposerClient::Rect> hwcRects;
    for (size_t rect = 0; rect < rectCount; ++rect) {
        hwcRects.push_back({rectArray[rect].left, rectArray[rect].top,
                rectArray[rect].right, rectArray[rect].bottom});
    }

    auto intError = mComposer.setLayerVisibleRegion(mDisplayId, mId, hwcRects);
    return static_cast<Error>(intError);
}

Error Layer::setZOrder(uint32_t z)
{
    auto intError = mComposer.setLayerZOrder(mDisplayId, mId, z);
    return static_cast<Error>(intError);
}

Error Layer::setInfo(uint32_t type, uint32_t appId)
{
  auto intError = mComposer.setLayerInfo(mDisplayId, mId, type, appId);
  return static_cast<Error>(intError);
}

// Composer HAL 2.3
Error Layer::setColorTransform(const android::mat4& matrix) {
    if (matrix == mColorMatrix) {
        return Error::None;
    }
    auto intError = mComposer.setLayerColorTransform(mDisplayId, mId, matrix.asArray());
    Error error = static_cast<Error>(intError);
    if (error != Error::None) {
        return error;
    }
    mColorMatrix = matrix;
    return error;
}

} // namespace impl
} // namespace HWC2
