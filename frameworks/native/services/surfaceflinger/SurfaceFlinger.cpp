/*
 * Copyright (C) 2007 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <sys/types.h>
#include <errno.h>
#include <dlfcn.h>

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <unordered_map>

#include <cutils/properties.h>
#include <log/log.h>

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/PermissionCache.h>

#include <compositionengine/CompositionEngine.h>
#include <compositionengine/Display.h>
#include <compositionengine/DisplayColorProfile.h>
#include <compositionengine/Layer.h>
#include <compositionengine/OutputLayer.h>
#include <compositionengine/RenderSurface.h>
#include <compositionengine/impl/LayerCompositionState.h>
#include <compositionengine/impl/OutputCompositionState.h>
#include <compositionengine/impl/OutputLayerCompositionState.h>
#include <dvr/vr_flinger.h>
#include <gui/BufferQueue.h>
#include <gui/DebugEGLImageTracker.h>

#include <gui/GuiConfig.h>
#include <gui/IDisplayEventConnection.h>
#include <gui/IProducerListener.h>
#include <gui/LayerDebugInfo.h>
#include <gui/Surface.h>
#include <input/IInputFlinger.h>
#include <renderengine/RenderEngine.h>
#include <ui/ColorSpace.h>
#include <ui/DebugUtils.h>
#include <ui/DisplayInfo.h>
#include <ui/DisplayStatInfo.h>
#include <ui/GraphicBufferAllocator.h>
#include <ui/PixelFormat.h>
#include <ui/UiConfig.h>
#include <utils/StopWatch.h>
#include <utils/String16.h>
#include <utils/String8.h>
#include <utils/Timers.h>
#include <utils/Trace.h>
#include <utils/misc.h>

#include <private/android_filesystem_config.h>
#include <private/gui/SyncFeatures.h>

#include "BufferLayer.h"
#include "BufferQueueLayer.h"
#include "BufferStateLayer.h"
#include "Client.h"
#include "ColorLayer.h"
#include "Colorizer.h"
#include "ContainerLayer.h"
#include "DisplayDevice.h"
#include "Layer.h"
#include "LayerVector.h"
#include "MonitoredProducer.h"
#include "NativeWindowSurface.h"
#include "RefreshRateOverlay.h"
#include "StartPropertySetThread.h"
#include "SurfaceFlinger.h"
#include "SurfaceInterceptor.h"

#include "DisplayHardware/ComposerHal.h"
#include "DisplayHardware/DisplayIdentification.h"
#include "DisplayHardware/FramebufferSurface.h"
#include "DisplayHardware/HWComposer.h"
#include "DisplayHardware/VirtualDisplaySurface.h"
#include "Effects/Daltonizer.h"
#include "RegionSamplingThread.h"
#include "Scheduler/DispSync.h"
#include "Scheduler/DispSyncSource.h"
#include "Scheduler/EventControlThread.h"
#include "Scheduler/EventThread.h"
#include "Scheduler/InjectVSyncSource.h"
#include "Scheduler/MessageQueue.h"
#include "Scheduler/PhaseOffsets.h"
#include "Scheduler/Scheduler.h"
#include "TimeStats/TimeStats.h"

#include <cutils/compiler.h>

#include "android-base/stringprintf.h"

#include <android/hardware/configstore/1.0/ISurfaceFlingerConfigs.h>
#include <android/hardware/configstore/1.1/ISurfaceFlingerConfigs.h>
#include <android/hardware/configstore/1.1/types.h>
#include <android/hardware/power/1.0/IPower.h>
#include <configstore/Utils.h>

#include <layerproto/LayerProtoParser.h>
#include "SurfaceFlingerProperties.h"

namespace android {

using namespace android::hardware::configstore;
using namespace android::hardware::configstore::V1_0;
using namespace android::sysprop;

using android::hardware::power::V1_0::PowerHint;
using base::StringAppendF;
using ui::ColorMode;
using ui::Dataspace;
using ui::DisplayPrimaries;
using ui::Hdr;
using ui::RenderIntent;

namespace {

#pragma clang diagnostic push
#pragma clang diagnostic error "-Wswitch-enum"

bool isWideColorMode(const ColorMode colorMode) {
    switch (colorMode) {
        case ColorMode::DISPLAY_P3:
        case ColorMode::ADOBE_RGB:
        case ColorMode::DCI_P3:
        case ColorMode::BT2020:
        case ColorMode::DISPLAY_BT2020:
        case ColorMode::BT2100_PQ:
        case ColorMode::BT2100_HLG:
            return true;
        case ColorMode::NATIVE:
        case ColorMode::STANDARD_BT601_625:
        case ColorMode::STANDARD_BT601_625_UNADJUSTED:
        case ColorMode::STANDARD_BT601_525:
        case ColorMode::STANDARD_BT601_525_UNADJUSTED:
        case ColorMode::STANDARD_BT709:
        case ColorMode::SRGB:
            return false;
    }
    return false;
}

bool isHdrColorMode(const ColorMode colorMode) {
    switch (colorMode) {
        case ColorMode::BT2100_PQ:
        case ColorMode::BT2100_HLG:
            return true;
        case ColorMode::DISPLAY_P3:
        case ColorMode::ADOBE_RGB:
        case ColorMode::DCI_P3:
        case ColorMode::BT2020:
        case ColorMode::DISPLAY_BT2020:
        case ColorMode::NATIVE:
        case ColorMode::STANDARD_BT601_625:
        case ColorMode::STANDARD_BT601_625_UNADJUSTED:
        case ColorMode::STANDARD_BT601_525:
        case ColorMode::STANDARD_BT601_525_UNADJUSTED:
        case ColorMode::STANDARD_BT709:
        case ColorMode::SRGB:
            return false;
    }
    return false;
}

ui::Transform::orientation_flags fromSurfaceComposerRotation(ISurfaceComposer::Rotation rotation) {
    switch (rotation) {
        case ISurfaceComposer::eRotateNone:
            return ui::Transform::ROT_0;
        case ISurfaceComposer::eRotate90:
            return ui::Transform::ROT_90;
        case ISurfaceComposer::eRotate180:
            return ui::Transform::ROT_180;
        case ISurfaceComposer::eRotate270:
            return ui::Transform::ROT_270;
    }
    ALOGE("Invalid rotation passed to captureScreen(): %d\n", rotation);
    return ui::Transform::ROT_0;
}

#pragma clang diagnostic pop

class ConditionalLock {
public:
    ConditionalLock(Mutex& mutex, bool lock) : mMutex(mutex), mLocked(lock) {
        if (lock) {
            mMutex.lock();
        }
    }
    ~ConditionalLock() { if (mLocked) mMutex.unlock(); }
private:
    Mutex& mMutex;
    bool mLocked;
};

// Currently we only support V0_SRGB and DISPLAY_P3 as composition preference.
bool validateCompositionDataspace(Dataspace dataspace) {
    return dataspace == Dataspace::V0_SRGB || dataspace == Dataspace::DISPLAY_P3;
}

}  // namespace anonymous

// ---------------------------------------------------------------------------

const String16 sHardwareTest("android.permission.HARDWARE_TEST");
const String16 sAccessSurfaceFlinger("android.permission.ACCESS_SURFACE_FLINGER");
const String16 sReadFramebuffer("android.permission.READ_FRAME_BUFFER");
const String16 sDump("android.permission.DUMP");

// ---------------------------------------------------------------------------
int64_t SurfaceFlinger::dispSyncPresentTimeOffset;
bool SurfaceFlinger::useHwcForRgbToYuv;
uint64_t SurfaceFlinger::maxVirtualDisplaySize;
bool SurfaceFlinger::hasSyncFramework;
bool SurfaceFlinger::useVrFlinger;
int64_t SurfaceFlinger::maxFrameBufferAcquiredBuffers;
bool SurfaceFlinger::hasWideColorDisplay;
int SurfaceFlinger::primaryDisplayOrientation = DisplayState::eOrientationDefault;
bool SurfaceFlinger::useColorManagement;
bool SurfaceFlinger::useContextPriority;
Dataspace SurfaceFlinger::defaultCompositionDataspace = Dataspace::V0_SRGB;
ui::PixelFormat SurfaceFlinger::defaultCompositionPixelFormat = ui::PixelFormat::RGBA_8888;
Dataspace SurfaceFlinger::wideColorGamutCompositionDataspace = Dataspace::V0_SRGB;
ui::PixelFormat SurfaceFlinger::wideColorGamutCompositionPixelFormat = ui::PixelFormat::RGBA_8888;

std::string getHwcServiceName() {
    char value[PROPERTY_VALUE_MAX] = {};
    property_get("debug.sf.hwc_service_name", value, "default");
    ALOGI("Using HWComposer service: '%s'", value);
    return std::string(value);
}

bool useTrebleTestingOverride() {
    char value[PROPERTY_VALUE_MAX] = {};
    property_get("debug.sf.treble_testing_override", value, "false");
    ALOGI("Treble testing override: '%s'", value);
    return std::string(value) == "true";
}

std::string decodeDisplayColorSetting(DisplayColorSetting displayColorSetting) {
    switch(displayColorSetting) {
        case DisplayColorSetting::MANAGED:
            return std::string("Managed");
        case DisplayColorSetting::UNMANAGED:
            return std::string("Unmanaged");
        case DisplayColorSetting::ENHANCED:
            return std::string("Enhanced");
        default:
            return std::string("Unknown ") +
                std::to_string(static_cast<int>(displayColorSetting));
    }
}

SurfaceFlingerBE::SurfaceFlingerBE() : mHwcServiceName(getHwcServiceName()) {}

SurfaceFlinger::SurfaceFlinger(Factory& factory, SkipInitializationTag)
      : mFactory(factory),
        mPhaseOffsets(mFactory.createPhaseOffsets()),
        mInterceptor(mFactory.createSurfaceInterceptor(this)),
        mTimeStats(mFactory.createTimeStats()),
        mEventQueue(mFactory.createMessageQueue()),
        mCompositionEngine(mFactory.createCompositionEngine()) {}

SurfaceFlinger::SurfaceFlinger(Factory& factory) : SurfaceFlinger(factory, SkipInitialization) {
    ALOGI("SurfaceFlinger is starting");

    hasSyncFramework = running_without_sync_framework(true);

    dispSyncPresentTimeOffset = present_time_offset_from_vsync_ns(0);

    useHwcForRgbToYuv = force_hwc_copy_for_virtual_displays(false);

    maxVirtualDisplaySize = max_virtual_display_dimension(0);

    // Vr flinger is only enabled on Daydream ready devices.
    useVrFlinger = use_vr_flinger(false);

    maxFrameBufferAcquiredBuffers = max_frame_buffer_acquired_buffers(2);

    hasWideColorDisplay = has_wide_color_display(false);

    useColorManagement = use_color_management(false);

    mDefaultCompositionDataspace =
            static_cast<ui::Dataspace>(default_composition_dataspace(Dataspace::V0_SRGB));
    mWideColorGamutCompositionDataspace = static_cast<ui::Dataspace>(wcg_composition_dataspace(
            hasWideColorDisplay ? Dataspace::DISPLAY_P3 : Dataspace::V0_SRGB));
    defaultCompositionDataspace = mDefaultCompositionDataspace;
    wideColorGamutCompositionDataspace = mWideColorGamutCompositionDataspace;
    defaultCompositionPixelFormat = static_cast<ui::PixelFormat>(
            default_composition_pixel_format(ui::PixelFormat::RGBA_8888));
    wideColorGamutCompositionPixelFormat =
            static_cast<ui::PixelFormat>(wcg_composition_pixel_format(ui::PixelFormat::RGBA_8888));

    mColorSpaceAgnosticDataspace =
            static_cast<ui::Dataspace>(color_space_agnostic_dataspace(Dataspace::UNKNOWN));

    useContextPriority = use_context_priority(true);

    auto tmpPrimaryDisplayOrientation = primary_display_orientation(
            SurfaceFlingerProperties::primary_display_orientation_values::ORIENTATION_0);
    switch (tmpPrimaryDisplayOrientation) {
        case SurfaceFlingerProperties::primary_display_orientation_values::ORIENTATION_90:
            SurfaceFlinger::primaryDisplayOrientation = DisplayState::eOrientation90;
            break;
        case SurfaceFlingerProperties::primary_display_orientation_values::ORIENTATION_180:
            SurfaceFlinger::primaryDisplayOrientation = DisplayState::eOrientation180;
            break;
        case SurfaceFlingerProperties::primary_display_orientation_values::ORIENTATION_270:
            SurfaceFlinger::primaryDisplayOrientation = DisplayState::eOrientation270;
            break;
        default:
            SurfaceFlinger::primaryDisplayOrientation = DisplayState::eOrientationDefault;
            break;
    }
    ALOGV("Primary Display Orientation is set to %2d.", SurfaceFlinger::primaryDisplayOrientation);

    mInternalDisplayPrimaries = sysprop::getDisplayNativePrimaries();

    // debugging stuff...
    char value[PROPERTY_VALUE_MAX];

    property_get("ro.bq.gpu_to_cpu_unsupported", value, "0");
    mGpuToCpuSupported = !atoi(value);

    property_get("debug.sf.showupdates", value, "0");
    mDebugRegion = atoi(value);

    ALOGI_IF(mDebugRegion, "showupdates enabled");

    // DDMS debugging deprecated (b/120782499)
    property_get("debug.sf.ddms", value, "0");
    int debugDdms = atoi(value);
    ALOGI_IF(debugDdms, "DDMS debugging not supported");

    property_get("debug.sf.disable_backpressure", value, "0");
    mPropagateBackpressure = !atoi(value);
    ALOGI_IF(!mPropagateBackpressure, "Disabling backpressure propagation");

    property_get("debug.sf.enable_gl_backpressure", value, "0");
    mPropagateBackpressureClientComposition = atoi(value);
    ALOGI_IF(mPropagateBackpressureClientComposition,
             "Enabling backpressure propagation for Client Composition");

    property_get("debug.sf.enable_hwc_vds", value, "0");
    mUseHwcVirtualDisplays = atoi(value);
    ALOGI_IF(mUseHwcVirtualDisplays, "Enabling HWC virtual displays");

    property_get("ro.sf.disable_triple_buffer", value, "0");
    mLayerTripleBufferingDisabled = atoi(value);
    ALOGI_IF(mLayerTripleBufferingDisabled, "Disabling Triple Buffering");

    const size_t defaultListSize = MAX_LAYERS;
    auto listSize = property_get_int32("debug.sf.max_igbp_list_size", int32_t(defaultListSize));
    mMaxGraphicBufferProducerListSize = (listSize > 0) ? size_t(listSize) : defaultListSize;

    mUseSmart90ForVideo = use_smart_90_for_video(false);
    property_get("debug.sf.use_smart_90_for_video", value, "0");

    int int_value = atoi(value);
    if (int_value) {
        mUseSmart90ForVideo = true;
    }

    property_get("debug.sf.luma_sampling", value, "1");
    mLumaSampling = atoi(value);

    const auto [early, gl, late] = mPhaseOffsets->getCurrentOffsets();
    mVsyncModulator.setPhaseOffsets(early, gl, late,
                                    mPhaseOffsets->getOffsetThresholdForNextVsync());

    // We should be reading 'persist.sys.sf.color_saturation' here
    // but since /data may be encrypted, we need to wait until after vold
    // comes online to attempt to read the property. The property is
    // instead read after the boot animation

    if (useTrebleTestingOverride()) {
        // Without the override SurfaceFlinger cannot connect to HIDL
        // services that are not listed in the manifests.  Considered
        // deriving the setting from the set service name, but it
        // would be brittle if the name that's not 'default' is used
        // for production purposes later on.
        setenv("TREBLE_TESTING_OVERRIDE", "true", true);
    }
}

void SurfaceFlinger::onFirstRef()
{
    mEventQueue->init(this);
}

SurfaceFlinger::~SurfaceFlinger() = default;

void SurfaceFlinger::binderDied(const wp<IBinder>& /* who */)
{
    // the window manager died on us. prepare its eulogy.

    // restore initial conditions (default device unblank, etc)
    initializeDisplays();

    // restart the boot-animation
    startBootAnim();
}

static sp<ISurfaceComposerClient> initClient(const sp<Client>& client) {
    status_t err = client->initCheck();
    if (err == NO_ERROR) {
        return client;
    }
    return nullptr;
}

sp<ISurfaceComposerClient> SurfaceFlinger::createConnection() {
    return initClient(new Client(this));
}

sp<IBinder> SurfaceFlinger::createDisplay(const String8& displayName,
        bool secure)
{
    class DisplayToken : public BBinder {
        sp<SurfaceFlinger> flinger;
        virtual ~DisplayToken() {
             // no more references, this display must be terminated
             Mutex::Autolock _l(flinger->mStateLock);
             flinger->mCurrentState.displays.removeItem(this);
             flinger->setTransactionFlags(eDisplayTransactionNeeded);
         }
     public:
        explicit DisplayToken(const sp<SurfaceFlinger>& flinger)
            : flinger(flinger) {
        }
    };

    sp<BBinder> token = new DisplayToken(this);

    Mutex::Autolock _l(mStateLock);
    // Display ID is assigned when virtual display is allocated by HWC.
    DisplayDeviceState state;
    state.isSecure = secure;
    state.displayName = displayName;
    mCurrentState.displays.add(token, state);
    mInterceptor->saveDisplayCreation(state);
    return token;
}

void SurfaceFlinger::destroyDisplay(const sp<IBinder>& displayToken) {
    Mutex::Autolock _l(mStateLock);

    ssize_t index = mCurrentState.displays.indexOfKey(displayToken);
    if (index < 0) {
        ALOGE("destroyDisplay: Invalid display token %p", displayToken.get());
        return;
    }

    const DisplayDeviceState& state = mCurrentState.displays.valueAt(index);
    if (!state.isVirtual()) {
        ALOGE("destroyDisplay called for non-virtual display");
        return;
    }
    mInterceptor->saveDisplayDeletion(state.sequenceId);
    mCurrentState.displays.removeItemsAt(index);
    setTransactionFlags(eDisplayTransactionNeeded);
}

std::vector<PhysicalDisplayId> SurfaceFlinger::getPhysicalDisplayIds() const {
    Mutex::Autolock lock(mStateLock);

    const auto internalDisplayId = getInternalDisplayIdLocked();
    if (!internalDisplayId) {
        return {};
    }

    std::vector<PhysicalDisplayId> displayIds;
    displayIds.reserve(mPhysicalDisplayTokens.size());
    displayIds.push_back(internalDisplayId->value);

    for (const auto& [id, token] : mPhysicalDisplayTokens) {
        if (id != *internalDisplayId) {
            displayIds.push_back(id.value);
        }
    }

    return displayIds;
}

sp<IBinder> SurfaceFlinger::getPhysicalDisplayToken(PhysicalDisplayId displayId) const {
    Mutex::Autolock lock(mStateLock);
    return getPhysicalDisplayTokenLocked(DisplayId{displayId});
}

status_t SurfaceFlinger::getColorManagement(bool* outGetColorManagement) const {
    if (!outGetColorManagement) {
        return BAD_VALUE;
    }
    *outGetColorManagement = useColorManagement;
    return NO_ERROR;
}

HWComposer& SurfaceFlinger::getHwComposer() const {
    return mCompositionEngine->getHwComposer();
}

renderengine::RenderEngine& SurfaceFlinger::getRenderEngine() const {
    return mCompositionEngine->getRenderEngine();
}

compositionengine::CompositionEngine& SurfaceFlinger::getCompositionEngine() const {
    return *mCompositionEngine.get();
}

void SurfaceFlinger::bootFinished()
{
    if (mStartPropertySetThread->join() != NO_ERROR) {
        ALOGE("Join StartPropertySetThread failed!");
    }
    const nsecs_t now = systemTime();
    const nsecs_t duration = now - mBootTime;
    ALOGI("Boot is finished (%ld ms)", long(ns2ms(duration)) );

    // wait patiently for the window manager death
    const String16 name("window");
    sp<IBinder> window(defaultServiceManager()->getService(name));
    if (window != 0) {
        window->linkToDeath(static_cast<IBinder::DeathRecipient*>(this));
    }
    sp<IBinder> input(defaultServiceManager()->getService(
            String16("inputflinger")));
    if (input == nullptr) {
        ALOGE("Failed to link to input service");
    } else {
        mInputFlinger = interface_cast<IInputFlinger>(input);
    }

    if (mVrFlinger) {
      mVrFlinger->OnBootFinished();
    }

    // stop boot animation
    // formerly we would just kill the process, but we now ask it to exit so it
    // can choose where to stop the animation.
    property_set("service.bootanim.exit", "1");

    const int LOGTAG_SF_STOP_BOOTANIM = 60110;
    LOG_EVENT_LONG(LOGTAG_SF_STOP_BOOTANIM,
                   ns2ms(systemTime(SYSTEM_TIME_MONOTONIC)));

    postMessageAsync(new LambdaMessage([this]() NO_THREAD_SAFETY_ANALYSIS {
        readPersistentProperties();
        mBootStage = BootStage::FINISHED;

        if (mRefreshRateConfigs->refreshRateSwitchingSupported()) {
            // set the refresh rate according to the policy
            const auto& performanceRefreshRate =
                    mRefreshRateConfigs->getRefreshRateFromType(RefreshRateType::PERFORMANCE);

            if (isDisplayConfigAllowed(performanceRefreshRate.configId)) {
                setRefreshRateTo(RefreshRateType::PERFORMANCE, Scheduler::ConfigEvent::None);
            } else {
                setRefreshRateTo(RefreshRateType::DEFAULT, Scheduler::ConfigEvent::None);
            }
        }
    }));
}

uint32_t SurfaceFlinger::getNewTexture() {
    {
        std::lock_guard lock(mTexturePoolMutex);
        if (!mTexturePool.empty()) {
            uint32_t name = mTexturePool.back();
            mTexturePool.pop_back();
            ATRACE_INT("TexturePoolSize", mTexturePool.size());
            return name;
        }

        // The pool was too small, so increase it for the future
        ++mTexturePoolSize;
    }

    // The pool was empty, so we need to get a new texture name directly using a
    // blocking call to the main thread
    uint32_t name = 0;
    postMessageSync(new LambdaMessage([&]() { getRenderEngine().genTextures(1, &name); }));
    return name;
}

void SurfaceFlinger::deleteTextureAsync(uint32_t texture) {
    std::lock_guard lock(mTexturePoolMutex);
    // We don't change the pool size, so the fix-up logic in postComposition will decide whether
    // to actually delete this or not based on mTexturePoolSize
    mTexturePool.push_back(texture);
    ATRACE_INT("TexturePoolSize", mTexturePool.size());
}

// Do not call property_set on main thread which will be blocked by init
// Use StartPropertySetThread instead.
void SurfaceFlinger::init() {
    ALOGI(  "SurfaceFlinger's main thread ready to run. "
            "Initializing graphics H/W...");

    ALOGI("Phase offset NS: %" PRId64 "", mPhaseOffsets->getCurrentAppOffset());

    Mutex::Autolock _l(mStateLock);

    // Get a RenderEngine for the given display / config (can't fail)
    int32_t renderEngineFeature = 0;
    renderEngineFeature |= (useColorManagement ?
                            renderengine::RenderEngine::USE_COLOR_MANAGEMENT : 0);
    renderEngineFeature |= (useContextPriority ?
                            renderengine::RenderEngine::USE_HIGH_PRIORITY_CONTEXT : 0);
    renderEngineFeature |=
            (enable_protected_contents(false) ? renderengine::RenderEngine::ENABLE_PROTECTED_CONTEXT
                                              : 0);

    // TODO(b/77156734): We need to stop casting and use HAL types when possible.
    // Sending maxFrameBufferAcquiredBuffers as the cache size is tightly tuned to single-display.
    mCompositionEngine->setRenderEngine(
            renderengine::RenderEngine::create(static_cast<int32_t>(defaultCompositionPixelFormat),
                                               renderEngineFeature, maxFrameBufferAcquiredBuffers));

    LOG_ALWAYS_FATAL_IF(mVrFlingerRequestsDisplay,
            "Starting with vr flinger active is not currently supported.");
    mCompositionEngine->setHwComposer(getFactory().createHWComposer(getBE().mHwcServiceName));
    mCompositionEngine->getHwComposer().registerCallback(this, getBE().mComposerSequenceId);
    // Process any initial hotplug and resulting display changes.
    processDisplayHotplugEventsLocked();
    const auto display = getDefaultDisplayDeviceLocked();
    LOG_ALWAYS_FATAL_IF(!display, "Missing internal display after registering composer callback.");
    LOG_ALWAYS_FATAL_IF(!getHwComposer().isConnected(*display->getId()),
                        "Internal display is disconnected.");

    if (useVrFlinger) {
        auto vrFlingerRequestDisplayCallback = [this](bool requestDisplay) {
            // This callback is called from the vr flinger dispatch thread. We
            // need to call signalTransaction(), which requires holding
            // mStateLock when we're not on the main thread. Acquiring
            // mStateLock from the vr flinger dispatch thread might trigger a
            // deadlock in surface flinger (see b/66916578), so post a message
            // to be handled on the main thread instead.
            postMessageAsync(new LambdaMessage([=] {
                ALOGI("VR request display mode: requestDisplay=%d", requestDisplay);
                mVrFlingerRequestsDisplay = requestDisplay;
                signalTransaction();
            }));
        };
        mVrFlinger = dvr::VrFlinger::Create(getHwComposer().getComposer(),
                                            getHwComposer()
                                                    .fromPhysicalDisplayId(*display->getId())
                                                    .value_or(0),
                                            vrFlingerRequestDisplayCallback);
        if (!mVrFlinger) {
            ALOGE("Failed to start vrflinger");
        }
    }

    // initialize our drawing state
    mDrawingState = mCurrentState;

    // set initial conditions (e.g. unblank default device)
    initializeDisplays();

    getRenderEngine().primeCache();

    // Inform native graphics APIs whether the present timestamp is supported:

    const bool presentFenceReliable =
            !getHwComposer().hasCapability(HWC2::Capability::PresentFenceIsNotReliable);
    mStartPropertySetThread = getFactory().createStartPropertySetThread(presentFenceReliable);

    if (mStartPropertySetThread->Start() != NO_ERROR) {
        ALOGE("Run StartPropertySetThread failed!");
    }

    ALOGV("Done initializing");
}

void SurfaceFlinger::readPersistentProperties() {
    Mutex::Autolock _l(mStateLock);

    char value[PROPERTY_VALUE_MAX];

    property_get("persist.sys.sf.color_saturation", value, "1.0");
    mGlobalSaturationFactor = atof(value);
    updateColorMatrixLocked();
    ALOGV("Saturation is set to %.2f", mGlobalSaturationFactor);

    property_get("persist.sys.sf.native_mode", value, "0");
    mDisplayColorSetting = static_cast<DisplayColorSetting>(atoi(value));

    property_get("persist.sys.sf.color_mode", value, "0");
    mForceColorMode = static_cast<ColorMode>(atoi(value));
}

void SurfaceFlinger::startBootAnim() {
    // Start boot animation service by setting a property mailbox
    // if property setting thread is already running, Start() will be just a NOP
    mStartPropertySetThread->Start();
    // Wait until property was set
    if (mStartPropertySetThread->join() != NO_ERROR) {
        ALOGE("Join StartPropertySetThread failed!");
    }
}

size_t SurfaceFlinger::getMaxTextureSize() const {
    return getRenderEngine().getMaxTextureSize();
}

size_t SurfaceFlinger::getMaxViewportDims() const {
    return getRenderEngine().getMaxViewportDims();
}

// ----------------------------------------------------------------------------

bool SurfaceFlinger::authenticateSurfaceTexture(
        const sp<IGraphicBufferProducer>& bufferProducer) const {
    Mutex::Autolock _l(mStateLock);
    return authenticateSurfaceTextureLocked(bufferProducer);
}

bool SurfaceFlinger::authenticateSurfaceTextureLocked(
        const sp<IGraphicBufferProducer>& bufferProducer) const {
    sp<IBinder> surfaceTextureBinder(IInterface::asBinder(bufferProducer));
    return mGraphicBufferProducerList.count(surfaceTextureBinder.get()) > 0;
}

status_t SurfaceFlinger::getSupportedFrameTimestamps(
        std::vector<FrameEvent>* outSupported) const {
    *outSupported = {
        FrameEvent::REQUESTED_PRESENT,
        FrameEvent::ACQUIRE,
        FrameEvent::LATCH,
        FrameEvent::FIRST_REFRESH_START,
        FrameEvent::LAST_REFRESH_START,
        FrameEvent::GPU_COMPOSITION_DONE,
        FrameEvent::DEQUEUE_READY,
        FrameEvent::RELEASE,
    };
    ConditionalLock _l(mStateLock,
            std::this_thread::get_id() != mMainThreadId);
    if (!getHwComposer().hasCapability(
            HWC2::Capability::PresentFenceIsNotReliable)) {
        outSupported->push_back(FrameEvent::DISPLAY_PRESENT);
    }
    return NO_ERROR;
}

status_t SurfaceFlinger::getDisplayConfigs(const sp<IBinder>& displayToken,
                                           Vector<DisplayInfo>* configs) {
    if (!displayToken || !configs) {
        return BAD_VALUE;
    }

    Mutex::Autolock lock(mStateLock);

    const auto displayId = getPhysicalDisplayIdLocked(displayToken);
    if (!displayId) {
        return NAME_NOT_FOUND;
    }

    // TODO: Not sure if display density should handled by SF any longer
    class Density {
        static float getDensityFromProperty(char const* propName) {
            char property[PROPERTY_VALUE_MAX];
            float density = 0.0f;
            if (property_get(propName, property, nullptr) > 0) {
                density = strtof(property, nullptr);
            }
            return density;
        }
    public:
        static float getEmuDensity() {
            return getDensityFromProperty("qemu.sf.lcd_density"); }
        static float getBuildDensity()  {
            return getDensityFromProperty("ro.sf.lcd_density"); }
    };

    configs->clear();

    for (const auto& hwConfig : getHwComposer().getConfigs(*displayId)) {
        DisplayInfo info = DisplayInfo();

        float xdpi = hwConfig->getDpiX();
        float ydpi = hwConfig->getDpiY();

        info.w = hwConfig->getWidth();
        info.h = hwConfig->getHeight();
        // Default display viewport to display width and height
        info.viewportW = info.w;
        info.viewportH = info.h;

        if (displayId == getInternalDisplayIdLocked()) {
            // The density of the device is provided by a build property
            float density = Density::getBuildDensity() / 160.0f;
            if (density == 0) {
                // the build doesn't provide a density -- this is wrong!
                // use xdpi instead
                ALOGE("ro.sf.lcd_density must be defined as a build property");
                density = xdpi / 160.0f;
            }
            if (Density::getEmuDensity()) {
                // if "qemu.sf.lcd_density" is specified, it overrides everything
                xdpi = ydpi = density = Density::getEmuDensity();
                density /= 160.0f;
            }
            info.density = density;

            // TODO: this needs to go away (currently needed only by webkit)
            const auto display = getDefaultDisplayDeviceLocked();
            info.orientation = display ? display->getOrientation() : 0;

            // This is for screenrecord
            const Rect viewport = display->getViewport();
            if (viewport.isValid()) {
                info.viewportW = uint32_t(viewport.getWidth());
                info.viewportH = uint32_t(viewport.getHeight());
            }
        } else {
            // TODO: where should this value come from?
            static const int TV_DENSITY = 213;
            info.density = TV_DENSITY / 160.0f;
            info.orientation = 0;
        }

        info.xdpi = xdpi;
        info.ydpi = ydpi;
        info.fps = 1e9 / hwConfig->getVsyncPeriod();
        const auto refreshRateType =
                mRefreshRateConfigs->getRefreshRateTypeFromHwcConfigId(hwConfig->getId());
        const auto offset = mPhaseOffsets->getOffsetsForRefreshRate(refreshRateType);
        info.appVsyncOffset = offset.late.app;

        // This is how far in advance a buffer must be queued for
        // presentation at a given time.  If you want a buffer to appear
        // on the screen at time N, you must submit the buffer before
        // (N - presentationDeadline).
        //
        // Normally it's one full refresh period (to give SF a chance to
        // latch the buffer), but this can be reduced by configuring a
        // DispSync offset.  Any additional delays introduced by the hardware
        // composer or panel must be accounted for here.
        //
        // We add an additional 1ms to allow for processing time and
        // differences between the ideal and actual refresh rate.
        info.presentationDeadline = hwConfig->getVsyncPeriod() - offset.late.sf + 1000000;

        // All non-virtual displays are currently considered secure.
        info.secure = true;

        if (displayId == getInternalDisplayIdLocked() &&
            primaryDisplayOrientation & DisplayState::eOrientationSwapMask) {
            std::swap(info.w, info.h);
        }

        configs->push_back(info);
    }

    return NO_ERROR;
}

status_t SurfaceFlinger::getDisplayStats(const sp<IBinder>&, DisplayStatInfo* stats) {
    if (!stats) {
        return BAD_VALUE;
    }

    mScheduler->getDisplayStatInfo(stats);
    return NO_ERROR;
}

int SurfaceFlinger::getActiveConfig(const sp<IBinder>& displayToken) {
    const auto display = getDisplayDevice(displayToken);
    if (!display) {
        ALOGE("getActiveConfig: Invalid display token %p", displayToken.get());
        return BAD_VALUE;
    }

    return display->getActiveConfig();
}

void SurfaceFlinger::setDesiredActiveConfig(const ActiveConfigInfo& info) {
    ATRACE_CALL();

    // Don't check against the current mode yet. Worst case we set the desired
    // config twice. However event generation config might have changed so we need to update it
    // accordingly
    std::lock_guard<std::mutex> lock(mActiveConfigLock);
    const Scheduler::ConfigEvent prevConfig = mDesiredActiveConfig.event;
    mDesiredActiveConfig = info;
    mDesiredActiveConfig.event = mDesiredActiveConfig.event | prevConfig;

    if (!mDesiredActiveConfigChanged) {
        // This will trigger HWC refresh without resetting the idle timer.
        repaintEverythingForHWC();
        // Start receiving vsync samples now, so that we can detect a period
        // switch.
        mScheduler->resyncToHardwareVsync(true, getVsyncPeriod());
        // As we called to set period, we will call to onRefreshRateChangeCompleted once
        // DispSync model is locked.
        mVsyncModulator.onRefreshRateChangeInitiated();
        mPhaseOffsets->setRefreshRateType(info.type);
        const auto [early, gl, late] = mPhaseOffsets->getCurrentOffsets();
        mVsyncModulator.setPhaseOffsets(early, gl, late,
                                        mPhaseOffsets->getOffsetThresholdForNextVsync());
    }
    mDesiredActiveConfigChanged = true;
    ATRACE_INT("DesiredActiveConfigChanged", mDesiredActiveConfigChanged);

    if (mRefreshRateOverlay) {
        mRefreshRateOverlay->changeRefreshRate(mDesiredActiveConfig.type);
    }
}

status_t SurfaceFlinger::setActiveConfig(const sp<IBinder>& displayToken, int mode) {
    ATRACE_CALL();

    std::vector<int32_t> allowedConfig;
    allowedConfig.push_back(mode);

    return setAllowedDisplayConfigs(displayToken, allowedConfig);
}

void SurfaceFlinger::setActiveConfigInternal() {
    ATRACE_CALL();

    const auto display = getDefaultDisplayDeviceLocked();
    if (!display) {
        return;
    }

    std::lock_guard<std::mutex> lock(mActiveConfigLock);
    mRefreshRateConfigs->setCurrentConfig(mUpcomingActiveConfig.configId);
    mRefreshRateStats->setConfigMode(mUpcomingActiveConfig.configId);

    display->setActiveConfig(mUpcomingActiveConfig.configId);

    mPhaseOffsets->setRefreshRateType(mUpcomingActiveConfig.type);
    const auto [early, gl, late] = mPhaseOffsets->getCurrentOffsets();
    mVsyncModulator.setPhaseOffsets(early, gl, late,
                                    mPhaseOffsets->getOffsetThresholdForNextVsync());
    ATRACE_INT("ActiveConfigMode", mUpcomingActiveConfig.configId);

    if (mUpcomingActiveConfig.event != Scheduler::ConfigEvent::None) {
        mScheduler->onConfigChanged(mAppConnectionHandle, display->getId()->value,
                                    mUpcomingActiveConfig.configId);
    }
}

void SurfaceFlinger::desiredActiveConfigChangeDone() {
    std::lock_guard<std::mutex> lock(mActiveConfigLock);
    mDesiredActiveConfig.event = Scheduler::ConfigEvent::None;
    mDesiredActiveConfigChanged = false;
    ATRACE_INT("DesiredActiveConfigChanged", mDesiredActiveConfigChanged);

    mScheduler->resyncToHardwareVsync(true, getVsyncPeriod());
    mPhaseOffsets->setRefreshRateType(mUpcomingActiveConfig.type);
    const auto [early, gl, late] = mPhaseOffsets->getCurrentOffsets();
    mVsyncModulator.setPhaseOffsets(early, gl, late,
                                    mPhaseOffsets->getOffsetThresholdForNextVsync());
}

bool SurfaceFlinger::performSetActiveConfig() {
    ATRACE_CALL();
    if (mCheckPendingFence) {
        if (previousFrameMissed()) {
            // fence has not signaled yet. wait for the next invalidate
            mEventQueue->invalidate();
            return true;
        }

        // We received the present fence from the HWC, so we assume it successfully updated
        // the config, hence we update SF.
        mCheckPendingFence = false;
        setActiveConfigInternal();
    }

    // Store the local variable to release the lock.
    ActiveConfigInfo desiredActiveConfig;
    {
        std::lock_guard<std::mutex> lock(mActiveConfigLock);
        if (!mDesiredActiveConfigChanged) {
            return false;
        }
        desiredActiveConfig = mDesiredActiveConfig;
    }

    const auto display = getDefaultDisplayDeviceLocked();
    if (!display || display->getActiveConfig() == desiredActiveConfig.configId) {
        // display is not valid or we are already in the requested mode
        // on both cases there is nothing left to do
        desiredActiveConfigChangeDone();
        return false;
    }

    // Desired active config was set, it is different than the config currently in use, however
    // allowed configs might have change by the time we process the refresh.
    // Make sure the desired config is still allowed
    if (!isDisplayConfigAllowed(desiredActiveConfig.configId)) {
        desiredActiveConfigChangeDone();
        return false;
    }

    mUpcomingActiveConfig = desiredActiveConfig;
    const auto displayId = display->getId();
    LOG_ALWAYS_FATAL_IF(!displayId);

    ATRACE_INT("ActiveConfigModeHWC", mUpcomingActiveConfig.configId);
    getHwComposer().setActiveConfig(*displayId, mUpcomingActiveConfig.configId);

    // we need to submit an empty frame to HWC to start the process
    mCheckPendingFence = true;
    mEventQueue->invalidate();
    return false;
}

status_t SurfaceFlinger::getDisplayColorModes(const sp<IBinder>& displayToken,
                                              Vector<ColorMode>* outColorModes) {
    if (!displayToken || !outColorModes) {
        return BAD_VALUE;
    }

    std::vector<ColorMode> modes;
    bool isInternalDisplay = false;
    {
        ConditionalLock lock(mStateLock, std::this_thread::get_id() != mMainThreadId);

        const auto displayId = getPhysicalDisplayIdLocked(displayToken);
        if (!displayId) {
            return NAME_NOT_FOUND;
        }

        modes = getHwComposer().getColorModes(*displayId);
        isInternalDisplay = displayId == getInternalDisplayIdLocked();
    }
    outColorModes->clear();

    // If it's built-in display and the configuration claims it's not wide color capable,
    // filter out all wide color modes. The typical reason why this happens is that the
    // hardware is not good enough to support GPU composition of wide color, and thus the
    // OEMs choose to disable this capability.
    if (isInternalDisplay && !hasWideColorDisplay) {
        std::remove_copy_if(modes.cbegin(), modes.cend(), std::back_inserter(*outColorModes),
                            isWideColorMode);
    } else {
        std::copy(modes.cbegin(), modes.cend(), std::back_inserter(*outColorModes));
    }

    return NO_ERROR;
}

status_t SurfaceFlinger::getDisplayNativePrimaries(const sp<IBinder>& displayToken,
                                                   ui::DisplayPrimaries &primaries) {
    if (!displayToken) {
        return BAD_VALUE;
    }

    // Currently we only support this API for a single internal display.
    if (getInternalDisplayToken() != displayToken) {
        return BAD_VALUE;
    }

    memcpy(&primaries, &mInternalDisplayPrimaries, sizeof(ui::DisplayPrimaries));
    return NO_ERROR;
}

ColorMode SurfaceFlinger::getActiveColorMode(const sp<IBinder>& displayToken) {
    if (const auto display = getDisplayDevice(displayToken)) {
        return display->getCompositionDisplay()->getState().colorMode;
    }
    return static_cast<ColorMode>(BAD_VALUE);
}

status_t SurfaceFlinger::setActiveColorMode(const sp<IBinder>& displayToken, ColorMode mode) {
    postMessageSync(new LambdaMessage([&] {
        Vector<ColorMode> modes;
        getDisplayColorModes(displayToken, &modes);
        bool exists = std::find(std::begin(modes), std::end(modes), mode) != std::end(modes);
        if (mode < ColorMode::NATIVE || !exists) {
            ALOGE("Attempt to set invalid active color mode %s (%d) for display token %p",
                  decodeColorMode(mode).c_str(), mode, displayToken.get());
            return;
        }
        const auto display = getDisplayDevice(displayToken);
        if (!display) {
            ALOGE("Attempt to set active color mode %s (%d) for invalid display token %p",
                  decodeColorMode(mode).c_str(), mode, displayToken.get());
        } else if (display->isVirtual()) {
            ALOGW("Attempt to set active color mode %s (%d) for virtual display",
                  decodeColorMode(mode).c_str(), mode);
        } else {
            display->getCompositionDisplay()->setColorMode(mode, Dataspace::UNKNOWN,
                                                           RenderIntent::COLORIMETRIC);
        }
    }));

    return NO_ERROR;
}

status_t SurfaceFlinger::clearAnimationFrameStats() {
    Mutex::Autolock _l(mStateLock);
    mAnimFrameTracker.clearStats();
    return NO_ERROR;
}

status_t SurfaceFlinger::getAnimationFrameStats(FrameStats* outStats) const {
    Mutex::Autolock _l(mStateLock);
    mAnimFrameTracker.getStats(outStats);
    return NO_ERROR;
}

status_t SurfaceFlinger::getHdrCapabilities(const sp<IBinder>& displayToken,
                                            HdrCapabilities* outCapabilities) const {
    Mutex::Autolock _l(mStateLock);

    const auto display = getDisplayDeviceLocked(displayToken);
    if (!display) {
        ALOGE("getHdrCapabilities: Invalid display token %p", displayToken.get());
        return BAD_VALUE;
    }

    // At this point the DisplayDeivce should already be set up,
    // meaning the luminance information is already queried from
    // hardware composer and stored properly.
    const HdrCapabilities& capabilities = display->getHdrCapabilities();
    *outCapabilities = HdrCapabilities(capabilities.getSupportedHdrTypes(),
                                       capabilities.getDesiredMaxLuminance(),
                                       capabilities.getDesiredMaxAverageLuminance(),
                                       capabilities.getDesiredMinLuminance());

    return NO_ERROR;
}

status_t SurfaceFlinger::getDisplayedContentSamplingAttributes(const sp<IBinder>& displayToken,
                                                               ui::PixelFormat* outFormat,
                                                               ui::Dataspace* outDataspace,
                                                               uint8_t* outComponentMask) const {
    if (!outFormat || !outDataspace || !outComponentMask) {
        return BAD_VALUE;
    }
    const auto display = getDisplayDevice(displayToken);
    if (!display || !display->getId()) {
        ALOGE("getDisplayedContentSamplingAttributes: Bad display token: %p", display.get());
        return BAD_VALUE;
    }
    return getHwComposer().getDisplayedContentSamplingAttributes(*display->getId(), outFormat,
                                                                 outDataspace, outComponentMask);
}

status_t SurfaceFlinger::setDisplayContentSamplingEnabled(const sp<IBinder>& displayToken,
                                                          bool enable, uint8_t componentMask,
                                                          uint64_t maxFrames) const {
    const auto display = getDisplayDevice(displayToken);
    if (!display || !display->getId()) {
        ALOGE("setDisplayContentSamplingEnabled: Bad display token: %p", display.get());
        return BAD_VALUE;
    }

    return getHwComposer().setDisplayContentSamplingEnabled(*display->getId(), enable,
                                                            componentMask, maxFrames);
}

status_t SurfaceFlinger::getDisplayedContentSample(const sp<IBinder>& displayToken,
                                                   uint64_t maxFrames, uint64_t timestamp,
                                                   DisplayedFrameStats* outStats) const {
    const auto display = getDisplayDevice(displayToken);
    if (!display || !display->getId()) {
        ALOGE("getDisplayContentSample: Bad display token: %p", displayToken.get());
        return BAD_VALUE;
    }

    return getHwComposer().getDisplayedContentSample(*display->getId(), maxFrames, timestamp,
                                                     outStats);
}

status_t SurfaceFlinger::getProtectedContentSupport(bool* outSupported) const {
    if (!outSupported) {
        return BAD_VALUE;
    }
    *outSupported = getRenderEngine().supportsProtectedContent();
    return NO_ERROR;
}

status_t SurfaceFlinger::isWideColorDisplay(const sp<IBinder>& displayToken,
                                            bool* outIsWideColorDisplay) const {
    if (!displayToken || !outIsWideColorDisplay) {
        return BAD_VALUE;
    }
    Mutex::Autolock _l(mStateLock);
    const auto display = getDisplayDeviceLocked(displayToken);
    if (!display) {
        return BAD_VALUE;
    }

    // Use hasWideColorDisplay to override built-in display.
    const auto displayId = display->getId();
    if (displayId && displayId == getInternalDisplayIdLocked()) {
        *outIsWideColorDisplay = hasWideColorDisplay;
        return NO_ERROR;
    }
    *outIsWideColorDisplay = display->hasWideColorGamut();
    return NO_ERROR;
}

status_t SurfaceFlinger::enableVSyncInjections(bool enable) {
    postMessageSync(new LambdaMessage([&] {
        Mutex::Autolock _l(mStateLock);

        if (mInjectVSyncs == enable) {
            return;
        }

        // TODO(b/128863962): Part of the Injector should be refactored, so that it
        // can be passed to Scheduler.
        if (enable) {
            ALOGV("VSync Injections enabled");
            if (mVSyncInjector.get() == nullptr) {
                mVSyncInjector = std::make_unique<InjectVSyncSource>();
                mInjectorEventThread = std::make_unique<
                        impl::EventThread>(mVSyncInjector.get(),
                                           impl::EventThread::InterceptVSyncsCallback(),
                                           "injEventThread");
            }
            mEventQueue->setEventThread(mInjectorEventThread.get(), [&] { mScheduler->resync(); });
        } else {
            ALOGV("VSync Injections disabled");
            mEventQueue->setEventThread(mScheduler->getEventThread(mSfConnectionHandle),
                                        [&] { mScheduler->resync(); });
        }

        mInjectVSyncs = enable;
    }));

    return NO_ERROR;
}

status_t SurfaceFlinger::injectVSync(nsecs_t when) {
    Mutex::Autolock _l(mStateLock);

    if (!mInjectVSyncs) {
        ALOGE("VSync Injections not enabled");
        return BAD_VALUE;
    }
    if (mInjectVSyncs && mInjectorEventThread.get() != nullptr) {
        ALOGV("Injecting VSync inside SurfaceFlinger");
        mVSyncInjector->onInjectSyncEvent(when);
    }
    return NO_ERROR;
}

status_t SurfaceFlinger::getLayerDebugInfo(std::vector<LayerDebugInfo>* outLayers) const
        NO_THREAD_SAFETY_ANALYSIS {
    // Try to acquire a lock for 1s, fail gracefully
    const status_t err = mStateLock.timedLock(s2ns(1));
    const bool locked = (err == NO_ERROR);
    if (!locked) {
        ALOGE("LayerDebugInfo: SurfaceFlinger unresponsive (%s [%d]) - exit", strerror(-err), err);
        return TIMED_OUT;
    }

    outLayers->clear();
    mCurrentState.traverseInZOrder([&](Layer* layer) {
        outLayers->push_back(layer->getLayerDebugInfo());
    });

    mStateLock.unlock();
    return NO_ERROR;
}

status_t SurfaceFlinger::getCompositionPreference(
        Dataspace* outDataspace, ui::PixelFormat* outPixelFormat,
        Dataspace* outWideColorGamutDataspace,
        ui::PixelFormat* outWideColorGamutPixelFormat) const {
    *outDataspace = mDefaultCompositionDataspace;
    *outPixelFormat = defaultCompositionPixelFormat;
    *outWideColorGamutDataspace = mWideColorGamutCompositionDataspace;
    *outWideColorGamutPixelFormat = wideColorGamutCompositionPixelFormat;
    return NO_ERROR;
}

status_t SurfaceFlinger::addRegionSamplingListener(const Rect& samplingArea,
                                                   const sp<IBinder>& stopLayerHandle,
                                                   const sp<IRegionSamplingListener>& listener) {
    if (!listener || samplingArea == Rect::INVALID_RECT) {
        return BAD_VALUE;
    }
    mRegionSamplingThread->addListener(samplingArea, stopLayerHandle, listener);
    return NO_ERROR;
}

status_t SurfaceFlinger::removeRegionSamplingListener(const sp<IRegionSamplingListener>& listener) {
    if (!listener) {
        return BAD_VALUE;
    }
    mRegionSamplingThread->removeListener(listener);
    return NO_ERROR;
}

status_t SurfaceFlinger::getDisplayBrightnessSupport(const sp<IBinder>& displayToken,
                                                     bool* outSupport) const {
    if (!displayToken || !outSupport) {
        return BAD_VALUE;
    }
    const auto displayId = getPhysicalDisplayIdLocked(displayToken);
    if (!displayId) {
        return NAME_NOT_FOUND;
    }
    *outSupport =
            getHwComposer().hasDisplayCapability(displayId, HWC2::DisplayCapability::Brightness);
    return NO_ERROR;
}

status_t SurfaceFlinger::setDisplayBrightness(const sp<IBinder>& displayToken,
                                              float brightness) const {
    if (!displayToken) {
        return BAD_VALUE;
    }
    const auto displayId = getPhysicalDisplayIdLocked(displayToken);
    if (!displayId) {
        return NAME_NOT_FOUND;
    }
    return getHwComposer().setDisplayBrightness(*displayId, brightness);
}

status_t SurfaceFlinger::notifyPowerHint(int32_t hintId) {
    PowerHint powerHint = static_cast<PowerHint>(hintId);

    if (powerHint == PowerHint::INTERACTION) {
        mScheduler->notifyTouchEvent();
    }

    return NO_ERROR;
}

// ----------------------------------------------------------------------------

sp<IDisplayEventConnection> SurfaceFlinger::createDisplayEventConnection(
        ISurfaceComposer::VsyncSource vsyncSource, ISurfaceComposer::ConfigChanged configChanged) {
    const auto& handle =
            vsyncSource == eVsyncSourceSurfaceFlinger ? mSfConnectionHandle : mAppConnectionHandle;

    return mScheduler->createDisplayEventConnection(handle, configChanged);
}

// ----------------------------------------------------------------------------

void SurfaceFlinger::waitForEvent() {
    mEventQueue->waitMessage();
}

void SurfaceFlinger::signalTransaction() {
    mScheduler->resetIdleTimer();
    mEventQueue->invalidate();
}

void SurfaceFlinger::signalLayerUpdate() {
    mScheduler->resetIdleTimer();
    mEventQueue->invalidate();
}

void SurfaceFlinger::signalRefresh() {
    mRefreshPending = true;
    mEventQueue->refresh();
}

status_t SurfaceFlinger::postMessageAsync(const sp<MessageBase>& msg,
        nsecs_t reltime, uint32_t /* flags */) {
    return mEventQueue->postMessage(msg, reltime);
}

status_t SurfaceFlinger::postMessageSync(const sp<MessageBase>& msg,
        nsecs_t reltime, uint32_t /* flags */) {
    status_t res = mEventQueue->postMessage(msg, reltime);
    if (res == NO_ERROR) {
        msg->wait();
    }
    return res;
}

void SurfaceFlinger::run() {
    do {
        waitForEvent();
    } while (true);
}

nsecs_t SurfaceFlinger::getVsyncPeriod() const {
    const auto displayId = getInternalDisplayIdLocked();
    if (!displayId || !getHwComposer().isConnected(*displayId)) {
        return 0;
    }

    const auto config = getHwComposer().getActiveConfig(*displayId);
    return config ? config->getVsyncPeriod() : 0;
}

void SurfaceFlinger::onVsyncReceived(int32_t sequenceId, hwc2_display_t hwcDisplayId,
                                     int64_t timestamp) {
    ATRACE_NAME("SF onVsync");

    Mutex::Autolock lock(mStateLock);
    // Ignore any vsyncs from a previous hardware composer.
    if (sequenceId != getBE().mComposerSequenceId) {
        return;
    }

    if (!getHwComposer().onVsync(hwcDisplayId, timestamp)) {
        return;
    }

    if (hwcDisplayId != getHwComposer().getInternalHwcDisplayId()) {
        // For now, we don't do anything with external display vsyncs.
        return;
    }

    bool periodFlushed = false;
    mScheduler->addResyncSample(timestamp, &periodFlushed);
    if (periodFlushed) {
        mVsyncModulator.onRefreshRateChangeCompleted();
    }
}

void SurfaceFlinger::getCompositorTiming(CompositorTiming* compositorTiming) {
    std::lock_guard<std::mutex> lock(getBE().mCompositorTimingLock);
    *compositorTiming = getBE().mCompositorTiming;
}

bool SurfaceFlinger::isDisplayConfigAllowed(int32_t configId) {
    return mAllowedDisplayConfigs.empty() || mAllowedDisplayConfigs.count(configId);
}

void SurfaceFlinger::setRefreshRateTo(RefreshRateType refreshRate, Scheduler::ConfigEvent event) {
    const auto display = getDefaultDisplayDeviceLocked();
    if (!display || mBootStage != BootStage::FINISHED) {
        return;
    }
    ATRACE_CALL();

    // Don't do any updating if the current fps is the same as the new one.
    const auto& refreshRateConfig = mRefreshRateConfigs->getRefreshRateFromType(refreshRate);
    const int desiredConfigId = refreshRateConfig.configId;

    if (!isDisplayConfigAllowed(desiredConfigId)) {
        ALOGV("Skipping config %d as it is not part of allowed configs", desiredConfigId);
        return;
    }

    setDesiredActiveConfig({refreshRate, desiredConfigId, event});
}

void SurfaceFlinger::onHotplugReceived(int32_t sequenceId, hwc2_display_t hwcDisplayId,
                                       HWC2::Connection connection) {
    ALOGV("%s(%d, %" PRIu64 ", %s)", __FUNCTION__, sequenceId, hwcDisplayId,
          connection == HWC2::Connection::Connected ? "connected" : "disconnected");

    // Ignore events that do not have the right sequenceId.
    if (sequenceId != getBE().mComposerSequenceId) {
        return;
    }

    // Only lock if we're not on the main thread. This function is normally
    // called on a hwbinder thread, but for the primary display it's called on
    // the main thread with the state lock already held, so don't attempt to
    // acquire it here.
    ConditionalLock lock(mStateLock, std::this_thread::get_id() != mMainThreadId);

    mPendingHotplugEvents.emplace_back(HotplugEvent{hwcDisplayId, connection});

    if (std::this_thread::get_id() == mMainThreadId) {
        // Process all pending hot plug events immediately if we are on the main thread.
        processDisplayHotplugEventsLocked();
    }

    setTransactionFlags(eDisplayTransactionNeeded);
}

void SurfaceFlinger::onRefreshReceived(int sequenceId, hwc2_display_t /*hwcDisplayId*/) {
    Mutex::Autolock lock(mStateLock);
    if (sequenceId != getBE().mComposerSequenceId) {
        return;
    }
    repaintEverythingForHWC();
}

void SurfaceFlinger::setPrimaryVsyncEnabled(bool enabled) {
    ATRACE_CALL();

    // Enable / Disable HWVsync from the main thread to avoid race conditions with
    // display power state.
    postMessageAsync(new LambdaMessage(
            [=]() NO_THREAD_SAFETY_ANALYSIS { setPrimaryVsyncEnabledInternal(enabled); }));
}

void SurfaceFlinger::setPrimaryVsyncEnabledInternal(bool enabled) {
    ATRACE_CALL();

    mHWCVsyncPendingState = enabled ? HWC2::Vsync::Enable : HWC2::Vsync::Disable;

    if (const auto displayId = getInternalDisplayIdLocked()) {
        sp<DisplayDevice> display = getDefaultDisplayDeviceLocked();
        if (display && display->isPoweredOn()) {
            setVsyncEnabledInHWC(*displayId, mHWCVsyncPendingState);
        }
    }
}

// Note: it is assumed the caller holds |mStateLock| when this is called
void SurfaceFlinger::resetDisplayState() {
    mScheduler->disableHardwareVsync(true);
    // Clear the drawing state so that the logic inside of
    // handleTransactionLocked will fire. It will determine the delta between
    // mCurrentState and mDrawingState and re-apply all changes when we make the
    // transition.
    mDrawingState.displays.clear();
    mDisplays.clear();
}

void SurfaceFlinger::updateVrFlinger() {
    ATRACE_CALL();
    if (!mVrFlinger)
        return;
    bool vrFlingerRequestsDisplay = mVrFlingerRequestsDisplay;
    if (vrFlingerRequestsDisplay == getHwComposer().isUsingVrComposer()) {
        return;
    }

    if (vrFlingerRequestsDisplay && !getHwComposer().getComposer()->isRemote()) {
        ALOGE("Vr flinger is only supported for remote hardware composer"
              " service connections. Ignoring request to transition to vr"
              " flinger.");
        mVrFlingerRequestsDisplay = false;
        return;
    }

    Mutex::Autolock _l(mStateLock);

    sp<DisplayDevice> display = getDefaultDisplayDeviceLocked();
    LOG_ALWAYS_FATAL_IF(!display);

    const int currentDisplayPowerMode = display->getPowerMode();

    // Clear out all the output layers from the composition engine for all
    // displays before destroying the hardware composer interface. This ensures
    // any HWC layers are destroyed through that interface before it becomes
    // invalid.
    for (const auto& [token, displayDevice] : mDisplays) {
        displayDevice->getCompositionDisplay()->setOutputLayersOrderedByZ(
                compositionengine::Output::OutputLayers());
    }

    // This DisplayDevice will no longer be relevant once resetDisplayState() is
    // called below. Clear the reference now so we don't accidentally use it
    // later.
    display.clear();

    if (!vrFlingerRequestsDisplay) {
        mVrFlinger->SeizeDisplayOwnership();
    }

    resetDisplayState();
    // Delete the current instance before creating the new one
    mCompositionEngine->setHwComposer(std::unique_ptr<HWComposer>());
    mCompositionEngine->setHwComposer(getFactory().createHWComposer(
            vrFlingerRequestsDisplay ? "vr" : getBE().mHwcServiceName));
    getHwComposer().registerCallback(this, ++getBE().mComposerSequenceId);

    LOG_ALWAYS_FATAL_IF(!getHwComposer().getComposer()->isRemote(),
                        "Switched to non-remote hardware composer");

    if (vrFlingerRequestsDisplay) {
        mVrFlinger->GrantDisplayOwnership();
    }

    mVisibleRegionsDirty = true;
    invalidateHwcGeometry();

    // Re-enable default display.
    display = getDefaultDisplayDeviceLocked();
    LOG_ALWAYS_FATAL_IF(!display);
    setPowerModeInternal(display, currentDisplayPowerMode);

    // Reset the timing values to account for the period of the swapped in HWC
    const nsecs_t vsyncPeriod = getVsyncPeriod();
    mAnimFrameTracker.setDisplayRefreshPeriod(vsyncPeriod);

    // The present fences returned from vr_hwc are not an accurate
    // representation of vsync times.
    mScheduler->setIgnorePresentFences(getHwComposer().isUsingVrComposer() || !hasSyncFramework);

    // Use phase of 0 since phase is not known.
    // Use latency of 0, which will snap to the ideal latency.
    DisplayStatInfo stats{0 /* vsyncTime */, vsyncPeriod};
    setCompositorTimingSnapped(stats, 0);

    mScheduler->resyncToHardwareVsync(false, vsyncPeriod);

    mRepaintEverything = true;
    setTransactionFlags(eDisplayTransactionNeeded);
}

bool SurfaceFlinger::previousFrameMissed(int graceTimeMs) NO_THREAD_SAFETY_ANALYSIS {
    ATRACE_CALL();
    // We are storing the last 2 present fences. If sf's phase offset is to be
    // woken up before the actual vsync but targeting the next vsync, we need to check
    // fence N-2
    const sp<Fence>& fence =
            mVsyncModulator.getOffsets().sf < mPhaseOffsets->getOffsetThresholdForNextVsync()
            ? mPreviousPresentFences[0]
            : mPreviousPresentFences[1];

    if (fence == Fence::NO_FENCE) {
        return false;
    }

    if (graceTimeMs > 0 && fence->getStatus() == Fence::Status::Unsignaled) {
        fence->wait(graceTimeMs);
    }

    return (fence->getStatus() == Fence::Status::Unsignaled);
}

void SurfaceFlinger::populateExpectedPresentTime() NO_THREAD_SAFETY_ANALYSIS {
    DisplayStatInfo stats;
    mScheduler->getDisplayStatInfo(&stats);
    const nsecs_t presentTime = mScheduler->getDispSyncExpectedPresentTime();
    // Inflate the expected present time if we're targetting the next vsync.
    mExpectedPresentTime =
            mVsyncModulator.getOffsets().sf < mPhaseOffsets->getOffsetThresholdForNextVsync()
            ? presentTime
            : presentTime + stats.vsyncPeriod;
}

void SurfaceFlinger::onMessageReceived(int32_t what) NO_THREAD_SAFETY_ANALYSIS {
    ATRACE_CALL();
    switch (what) {
        case MessageQueue::INVALIDATE: {
            // calculate the expected present time once and use the cached
            // value throughout this frame to make sure all layers are
            // seeing this same value.
            populateExpectedPresentTime();

            // When Backpressure propagation is enabled we want to give a small grace period
            // for the present fence to fire instead of just giving up on this frame to handle cases
            // where present fence is just about to get signaled.
            const int graceTimeForPresentFenceMs =
                    (mPropagateBackpressure &&
                     (mPropagateBackpressureClientComposition || !mHadClientComposition))
                    ? 1
                    : 0;
            bool frameMissed = previousFrameMissed(graceTimeForPresentFenceMs);
            bool hwcFrameMissed = mHadDeviceComposition && frameMissed;
            bool gpuFrameMissed = mHadClientComposition && frameMissed;
            ATRACE_INT("FrameMissed", static_cast<int>(frameMissed));
            ATRACE_INT("HwcFrameMissed", static_cast<int>(hwcFrameMissed));
            ATRACE_INT("GpuFrameMissed", static_cast<int>(gpuFrameMissed));
            if (frameMissed) {
                mFrameMissedCount++;
                mTimeStats->incrementMissedFrames();
            }

            if (hwcFrameMissed) {
                mHwcFrameMissedCount++;
            }

            if (gpuFrameMissed) {
                mGpuFrameMissedCount++;
            }

            if (mUseSmart90ForVideo) {
                // This call is made each time SF wakes up and creates a new frame. It is part
                // of video detection feature.
                mScheduler->updateFpsBasedOnContent();
            }

            if (performSetActiveConfig()) {
                break;
            }

            if (frameMissed && mPropagateBackpressure) {
                if ((hwcFrameMissed && !gpuFrameMissed) ||
                    mPropagateBackpressureClientComposition) {
                    signalLayerUpdate();
                    break;
                }
            }

            // Now that we're going to make it to the handleMessageTransaction()
            // call below it's safe to call updateVrFlinger(), which will
            // potentially trigger a display handoff.
            updateVrFlinger();

            bool refreshNeeded = handleMessageTransaction();
            refreshNeeded |= handleMessageInvalidate();

            updateCursorAsync();
            updateInputFlinger();

            refreshNeeded |= mRepaintEverything;
            if (refreshNeeded && CC_LIKELY(mBootStage != BootStage::BOOTLOADER)) {
                // Signal a refresh if a transaction modified the window state,
                // a new buffer was latched, or if HWC has requested a full
                // repaint
                signalRefresh();
            }
            break;
        }
        case MessageQueue::REFRESH: {
            handleMessageRefresh();
            break;
        }
    }
}

bool SurfaceFlinger::handleMessageTransaction() {
    ATRACE_CALL();
    uint32_t transactionFlags = peekTransactionFlags();

    bool flushedATransaction = flushTransactionQueues();

    bool runHandleTransaction = transactionFlags &&
            ((transactionFlags != eTransactionFlushNeeded) || flushedATransaction);

    if (runHandleTransaction) {
        handleTransaction(eTransactionMask);
    } else {
        getTransactionFlags(eTransactionFlushNeeded);
    }

    if (transactionFlushNeeded()) {
        setTransactionFlags(eTransactionFlushNeeded);
    }

    return runHandleTransaction;
}

void SurfaceFlinger::handleMessageRefresh() {
    ATRACE_CALL();

    mRefreshPending = false;

    const bool repaintEverything = mRepaintEverything.exchange(false);
    preComposition();
    rebuildLayerStacks();
    calculateWorkingSet();
    for (const auto& [token, display] : mDisplays) {
        beginFrame(display);
        prepareFrame(display);
        doDebugFlashRegions(display, repaintEverything);
        doComposition(display, repaintEverything);
    }

    logLayerStats();

    postFrame();
    postComposition();

    mHadClientComposition = false;
    mHadDeviceComposition = false;
    for (const auto& [token, displayDevice] : mDisplays) {
        auto display = displayDevice->getCompositionDisplay();
        const auto displayId = display->getId();
        mHadClientComposition =
                mHadClientComposition || getHwComposer().hasClientComposition(displayId);
        mHadDeviceComposition =
                mHadDeviceComposition || getHwComposer().hasDeviceComposition(displayId);
    }

    mVsyncModulator.onRefreshed(mHadClientComposition);

    mLayersWithQueuedFrames.clear();
}


bool SurfaceFlinger::handleMessageInvalidate() {
    ATRACE_CALL();
    bool refreshNeeded = handlePageFlip();

    if (mVisibleRegionsDirty) {
        computeLayerBounds();
        if (mTracingEnabled) {
            mTracing.notify("visibleRegionsDirty");
        }
    }

    for (auto& layer : mLayersPendingRefresh) {
        Region visibleReg;
        visibleReg.set(layer->getScreenBounds());
        invalidateLayerStack(layer, visibleReg);
    }
    mLayersPendingRefresh.clear();
    return refreshNeeded;
}

void SurfaceFlinger::calculateWorkingSet() {
    ATRACE_CALL();
    ALOGV(__FUNCTION__);

    // build the h/w work list
    if (CC_UNLIKELY(mGeometryInvalid)) {
        mGeometryInvalid = false;
        for (const auto& [token, displayDevice] : mDisplays) {
            auto display = displayDevice->getCompositionDisplay();

            uint32_t zOrder = 0;

            for (auto& layer : display->getOutputLayersOrderedByZ()) {
                auto& compositionState = layer->editState();
                compositionState.forceClientComposition = false;
                if (!compositionState.hwc || mDebugDisableHWC || mDebugRegion) {
                    compositionState.forceClientComposition = true;
                }

                // The output Z order is set here based on a simple counter.
                compositionState.z = zOrder++;

                // Update the display independent composition state. This goes
                // to the general composition layer state structure.
                // TODO: Do this once per compositionengine::CompositionLayer.
                layer->getLayerFE().latchCompositionState(layer->getLayer().editState().frontEnd,
                                                          true);

                // Recalculate the geometry state of the output layer.
                layer->updateCompositionState(true);

                // Write the updated geometry state to the HWC
                layer->writeStateToHWC(true);
            }
        }
    }

    // Set the per-frame data
    for (const auto& [token, displayDevice] : mDisplays) {
        auto display = displayDevice->getCompositionDisplay();
        const auto displayId = display->getId();
        if (!displayId) {
            continue;
        }
        auto* profile = display->getDisplayColorProfile();

        if (mDrawingState.colorMatrixChanged) {
            display->setColorTransform(mDrawingState.colorMatrix);
        }
        Dataspace targetDataspace = Dataspace::UNKNOWN;
        if (useColorManagement) {
            ColorMode colorMode;
            RenderIntent renderIntent;
            pickColorMode(displayDevice, &colorMode, &targetDataspace, &renderIntent);
            display->setColorMode(colorMode, targetDataspace, renderIntent);

            if (isHdrColorMode(colorMode)) {
                targetDataspace = Dataspace::UNKNOWN;
            } else if (mColorSpaceAgnosticDataspace != Dataspace::UNKNOWN) {
                targetDataspace = mColorSpaceAgnosticDataspace;
            }
        }

        for (auto& layer : displayDevice->getVisibleLayersSortedByZ()) {
            if (layer->isHdrY410()) {
                layer->forceClientComposition(displayDevice);
            } else if ((layer->getDataSpace() == Dataspace::BT2020_PQ ||
                        layer->getDataSpace() == Dataspace::BT2020_ITU_PQ) &&
                       !profile->hasHDR10Support()) {
                layer->forceClientComposition(displayDevice);
            } else if ((layer->getDataSpace() == Dataspace::BT2020_HLG ||
                        layer->getDataSpace() == Dataspace::BT2020_ITU_HLG) &&
                       !profile->hasHLGSupport()) {
                layer->forceClientComposition(displayDevice);
            }

            if (layer->getRoundedCornerState().radius > 0.0f) {
                layer->forceClientComposition(displayDevice);
            }

            if (layer->getForceClientComposition(displayDevice)) {
                ALOGV("[%s] Requesting Client composition", layer->getName().string());
                layer->setCompositionType(displayDevice,
                                          Hwc2::IComposerClient::Composition::CLIENT);
                continue;
            }

            const auto& displayState = display->getState();
            layer->setPerFrameData(displayDevice, displayState.transform, displayState.viewport,
                                   displayDevice->getSupportedPerFrameMetadata(), targetDataspace);
        }
    }

    mDrawingState.colorMatrixChanged = false;

    for (const auto& [token, displayDevice] : mDisplays) {
        auto display = displayDevice->getCompositionDisplay();
        for (auto& layer : displayDevice->getVisibleLayersSortedByZ()) {
            auto& layerState = layer->getCompositionLayer()->editState().frontEnd;
            layerState.compositionType = static_cast<Hwc2::IComposerClient::Composition>(
                    layer->getCompositionType(displayDevice));
        }
    }
}

void SurfaceFlinger::doDebugFlashRegions(const sp<DisplayDevice>& displayDevice,
                                         bool repaintEverything) {
    auto display = displayDevice->getCompositionDisplay();
    const auto& displayState = display->getState();

    // is debugging enabled
    if (CC_LIKELY(!mDebugRegion))
        return;

    if (displayState.isEnabled) {
        // transform the dirty region into this screen's coordinate space
        const Region dirtyRegion = display->getDirtyRegion(repaintEverything);
        if (!dirtyRegion.isEmpty()) {
            base::unique_fd readyFence;
            // redraw the whole screen
            doComposeSurfaces(displayDevice, dirtyRegion, &readyFence);

            display->getRenderSurface()->queueBuffer(std::move(readyFence));
        }
    }

    postFramebuffer(displayDevice);

    if (mDebugRegion > 1) {
        usleep(mDebugRegion * 1000);
    }

    prepareFrame(displayDevice);
}

void SurfaceFlinger::logLayerStats() {
    ATRACE_CALL();
    if (CC_UNLIKELY(mLayerStats.isEnabled())) {
        for (const auto& [token, display] : mDisplays) {
            if (display->isPrimary()) {
                mLayerStats.logLayerStats(dumpVisibleLayersProtoInfo(display));
                return;
            }
        }

        ALOGE("logLayerStats: no primary display");
    }
}

void SurfaceFlinger::preComposition()
{
    ATRACE_CALL();
    ALOGV("preComposition");

    mRefreshStartTime = systemTime(SYSTEM_TIME_MONOTONIC);

    bool needExtraInvalidate = false;
    mDrawingState.traverseInZOrder([&](Layer* layer) {
        if (layer->onPreComposition(mRefreshStartTime)) {
            needExtraInvalidate = true;
        }
    });

    if (needExtraInvalidate) {
        signalLayerUpdate();
    }
}

void SurfaceFlinger::updateCompositorTiming(const DisplayStatInfo& stats, nsecs_t compositeTime,
                                            std::shared_ptr<FenceTime>& presentFenceTime) {
    // Update queue of past composite+present times and determine the
    // most recently known composite to present latency.
    getBE().mCompositePresentTimes.push({compositeTime, presentFenceTime});
    nsecs_t compositeToPresentLatency = -1;
    while (!getBE().mCompositePresentTimes.empty()) {
        SurfaceFlingerBE::CompositePresentTime& cpt = getBE().mCompositePresentTimes.front();
        // Cached values should have been updated before calling this method,
        // which helps avoid duplicate syscalls.
        nsecs_t displayTime = cpt.display->getCachedSignalTime();
        if (displayTime == Fence::SIGNAL_TIME_PENDING) {
            break;
        }
        compositeToPresentLatency = displayTime - cpt.composite;
        getBE().mCompositePresentTimes.pop();
    }

    // Don't let mCompositePresentTimes grow unbounded, just in case.
    while (getBE().mCompositePresentTimes.size() > 16) {
        getBE().mCompositePresentTimes.pop();
    }

    setCompositorTimingSnapped(stats, compositeToPresentLatency);
}

void SurfaceFlinger::setCompositorTimingSnapped(const DisplayStatInfo& stats,
                                                nsecs_t compositeToPresentLatency) {
    // Integer division and modulo round toward 0 not -inf, so we need to
    // treat negative and positive offsets differently.
    nsecs_t idealLatency = (mPhaseOffsets->getCurrentSfOffset() > 0)
            ? (stats.vsyncPeriod - (mPhaseOffsets->getCurrentSfOffset() % stats.vsyncPeriod))
            : ((-mPhaseOffsets->getCurrentSfOffset()) % stats.vsyncPeriod);

    // Just in case mPhaseOffsets->getCurrentSfOffset() == -vsyncInterval.
    if (idealLatency <= 0) {
        idealLatency = stats.vsyncPeriod;
    }

    // Snap the latency to a value that removes scheduling jitter from the
    // composition and present times, which often have >1ms of jitter.
    // Reducing jitter is important if an app attempts to extrapolate
    // something (such as user input) to an accurate diasplay time.
    // Snapping also allows an app to precisely calculate mPhaseOffsets->getCurrentSfOffset()
    // with (presentLatency % interval).
    nsecs_t bias = stats.vsyncPeriod / 2;
    int64_t extraVsyncs = (compositeToPresentLatency - idealLatency + bias) / stats.vsyncPeriod;
    nsecs_t snappedCompositeToPresentLatency =
            (extraVsyncs > 0) ? idealLatency + (extraVsyncs * stats.vsyncPeriod) : idealLatency;

    std::lock_guard<std::mutex> lock(getBE().mCompositorTimingLock);
    getBE().mCompositorTiming.deadline = stats.vsyncTime - idealLatency;
    getBE().mCompositorTiming.interval = stats.vsyncPeriod;
    getBE().mCompositorTiming.presentLatency = snappedCompositeToPresentLatency;
}

void SurfaceFlinger::postComposition()
{
    ATRACE_CALL();
    ALOGV("postComposition");

    // Release any buffers which were replaced this frame
    nsecs_t dequeueReadyTime = systemTime();
    for (auto& layer : mLayersWithQueuedFrames) {
        layer->releasePendingBuffer(dequeueReadyTime);
    }

    // |mStateLock| not needed as we are on the main thread
    const auto displayDevice = getDefaultDisplayDeviceLocked();

    getBE().mGlCompositionDoneTimeline.updateSignalTimes();
    std::shared_ptr<FenceTime> glCompositionDoneFenceTime;
    if (displayDevice && getHwComposer().hasClientComposition(displayDevice->getId())) {
        glCompositionDoneFenceTime =
                std::make_shared<FenceTime>(displayDevice->getCompositionDisplay()
                                                    ->getRenderSurface()
                                                    ->getClientTargetAcquireFence());
        getBE().mGlCompositionDoneTimeline.push(glCompositionDoneFenceTime);
    } else {
        glCompositionDoneFenceTime = FenceTime::NO_FENCE;
    }

    getBE().mDisplayTimeline.updateSignalTimes();
    mPreviousPresentFences[1] = mPreviousPresentFences[0];
    mPreviousPresentFences[0] = displayDevice
            ? getHwComposer().getPresentFence(*displayDevice->getId())
            : Fence::NO_FENCE;
    auto presentFenceTime = std::make_shared<FenceTime>(mPreviousPresentFences[0]);
    getBE().mDisplayTimeline.push(presentFenceTime);

    DisplayStatInfo stats;
    mScheduler->getDisplayStatInfo(&stats);

    // We use the mRefreshStartTime which might be sampled a little later than
    // when we started doing work for this frame, but that should be okay
    // since updateCompositorTiming has snapping logic.
    updateCompositorTiming(stats, mRefreshStartTime, presentFenceTime);
    CompositorTiming compositorTiming;
    {
        std::lock_guard<std::mutex> lock(getBE().mCompositorTimingLock);
        compositorTiming = getBE().mCompositorTiming;
    }

    mDrawingState.traverseInZOrder([&](Layer* layer) {
        bool frameLatched =
                layer->onPostComposition(displayDevice->getId(), glCompositionDoneFenceTime,
                                         presentFenceTime, compositorTiming);
        if (frameLatched) {
            recordBufferingStats(layer->getName().string(),
                    layer->getOccupancyHistory(false));
        }
    });

    if (presentFenceTime->isValid()) {
        mScheduler->addPresentFence(presentFenceTime);
    }

    if (!hasSyncFramework) {
        if (displayDevice && getHwComposer().isConnected(*displayDevice->getId()) &&
            displayDevice->isPoweredOn()) {
            mScheduler->enableHardwareVsync();
        }
    }

    if (mAnimCompositionPending) {
        mAnimCompositionPending = false;

        if (presentFenceTime->isValid()) {
            mAnimFrameTracker.setActualPresentFence(
                    std::move(presentFenceTime));
        } else if (displayDevice && getHwComposer().isConnected(*displayDevice->getId())) {
            // The HWC doesn't support present fences, so use the refresh
            // timestamp instead.
            const nsecs_t presentTime =
                    getHwComposer().getRefreshTimestamp(*displayDevice->getId());
            mAnimFrameTracker.setActualPresentTime(presentTime);
        }
        mAnimFrameTracker.advanceFrame();
    }

    mTimeStats->incrementTotalFrames();
    if (mHadClientComposition) {
        mTimeStats->incrementClientCompositionFrames();
    }

    mTimeStats->setPresentFenceGlobal(presentFenceTime);

    if (displayDevice && getHwComposer().isConnected(*displayDevice->getId()) &&
        !displayDevice->isPoweredOn()) {
        return;
    }

    nsecs_t currentTime = systemTime();
    if (mHasPoweredOff) {
        mHasPoweredOff = false;
    } else {
        nsecs_t elapsedTime = currentTime - getBE().mLastSwapTime;
        size_t numPeriods = static_cast<size_t>(elapsedTime / stats.vsyncPeriod);
        if (numPeriods < SurfaceFlingerBE::NUM_BUCKETS - 1) {
            getBE().mFrameBuckets[numPeriods] += elapsedTime;
        } else {
            getBE().mFrameBuckets[SurfaceFlingerBE::NUM_BUCKETS - 1] += elapsedTime;
        }
        getBE().mTotalTime += elapsedTime;
    }
    getBE().mLastSwapTime = currentTime;

    {
        std::lock_guard lock(mTexturePoolMutex);
        if (mTexturePool.size() < mTexturePoolSize) {
            const size_t refillCount = mTexturePoolSize - mTexturePool.size();
            const size_t offset = mTexturePool.size();
            mTexturePool.resize(mTexturePoolSize);
            getRenderEngine().genTextures(refillCount, mTexturePool.data() + offset);
            ATRACE_INT("TexturePoolSize", mTexturePool.size());
        } else if (mTexturePool.size() > mTexturePoolSize) {
            const size_t deleteCount = mTexturePool.size() - mTexturePoolSize;
            const size_t offset = mTexturePoolSize;
            getRenderEngine().deleteTextures(deleteCount, mTexturePool.data() + offset);
            mTexturePool.resize(mTexturePoolSize);
            ATRACE_INT("TexturePoolSize", mTexturePool.size());
        }
    }

    mTransactionCompletedThread.addPresentFence(mPreviousPresentFences[0]);

    // Lock the mStateLock in case SurfaceFlinger is in the middle of applying a transaction.
    // If we do not lock here, a callback could be sent without all of its SurfaceControls and
    // metrics.
    {
        Mutex::Autolock _l(mStateLock);
        mTransactionCompletedThread.sendCallbacks();
    }

    if (mLumaSampling && mRegionSamplingThread) {
        mRegionSamplingThread->notifyNewContent();
    }

    // Even though ATRACE_INT64 already checks if tracing is enabled, it doesn't prevent the
    // side-effect of getTotalSize(), so we check that again here
    if (ATRACE_ENABLED()) {
        ATRACE_INT64("Total Buffer Size", GraphicBufferAllocator::get().getTotalSize());
    }
}

void SurfaceFlinger::computeLayerBounds() {
    for (const auto& pair : mDisplays) {
        const auto& displayDevice = pair.second;
        const auto display = displayDevice->getCompositionDisplay();
        for (const auto& layer : mDrawingState.layersSortedByZ) {
            // only consider the layers on the given layer stack
            if (!display->belongsInOutput(layer->getLayerStack(), layer->getPrimaryDisplayOnly())) {
                continue;
            }

            layer->computeBounds(displayDevice->getViewport().toFloatRect(), ui::Transform());
        }
    }
}

void SurfaceFlinger::rebuildLayerStacks() {
    ATRACE_CALL();
    ALOGV("rebuildLayerStacks");

    // rebuild the visible layer list per screen
    if (CC_UNLIKELY(mVisibleRegionsDirty)) {
        ATRACE_NAME("rebuildLayerStacks VR Dirty");
        mVisibleRegionsDirty = false;
        invalidateHwcGeometry();

        for (const auto& pair : mDisplays) {
            const auto& displayDevice = pair.second;
            auto display = displayDevice->getCompositionDisplay();
            const auto& displayState = display->getState();
            Region opaqueRegion;
            Region dirtyRegion;
            compositionengine::Output::OutputLayers layersSortedByZ;
            Vector<sp<Layer>> deprecated_layersSortedByZ;
            Vector<sp<Layer>> layersNeedingFences;
            const ui::Transform& tr = displayState.transform;
            const Rect bounds = displayState.bounds;
            if (displayState.isEnabled) {
                computeVisibleRegions(displayDevice, dirtyRegion, opaqueRegion);

                mDrawingState.traverseInZOrder([&](Layer* layer) {
                    auto compositionLayer = layer->getCompositionLayer();
                    if (compositionLayer == nullptr) {
                        return;
                    }

                    const auto displayId = displayDevice->getId();
                    sp<compositionengine::LayerFE> layerFE = compositionLayer->getLayerFE();
                    LOG_ALWAYS_FATAL_IF(layerFE.get() == nullptr);

                    bool needsOutputLayer = false;

                    if (display->belongsInOutput(layer->getLayerStack(),
                                                 layer->getPrimaryDisplayOnly())) {
                        Region drawRegion(tr.transform(
                                layer->visibleNonTransparentRegion));
                        drawRegion.andSelf(bounds);
                        if (!drawRegion.isEmpty()) {
                            needsOutputLayer = true;
                        }
                    }

                    if (needsOutputLayer) {
                        layersSortedByZ.emplace_back(
                                display->getOrCreateOutputLayer(displayId, compositionLayer,
                                                                layerFE));
                        deprecated_layersSortedByZ.add(layer);

                        auto& outputLayerState = layersSortedByZ.back()->editState();
                        outputLayerState.visibleRegion =
                                tr.transform(layer->visibleRegion.intersect(displayState.viewport));
                    } else if (displayId) {
                        // For layers that are being removed from a HWC display,
                        // and that have queued frames, add them to a a list of
                        // released layers so we can properly set a fence.
                        bool hasExistingOutputLayer =
                                display->getOutputLayerForLayer(compositionLayer.get()) != nullptr;
                        bool hasQueuedFrames = std::find(mLayersWithQueuedFrames.cbegin(),
                                                         mLayersWithQueuedFrames.cend(),
                                                         layer) != mLayersWithQueuedFrames.cend();

                        if (hasExistingOutputLayer && hasQueuedFrames) {
                            layersNeedingFences.add(layer);
                        }
                    }
                });
            }

            display->setOutputLayersOrderedByZ(std::move(layersSortedByZ));

            displayDevice->setVisibleLayersSortedByZ(deprecated_layersSortedByZ);
            displayDevice->setLayersNeedingFences(layersNeedingFences);

            Region undefinedRegion{bounds};
            undefinedRegion.subtractSelf(tr.transform(opaqueRegion));

            display->editState().undefinedRegion = undefinedRegion;
            display->editState().dirtyRegion.orSelf(dirtyRegion);
        }
    }
}

// Returns a data space that fits all visible layers.  The returned data space
// can only be one of
//  - Dataspace::SRGB (use legacy dataspace and let HWC saturate when colors are enhanced)
//  - Dataspace::DISPLAY_P3
//  - Dataspace::DISPLAY_BT2020
// The returned HDR data space is one of
//  - Dataspace::UNKNOWN
//  - Dataspace::BT2020_HLG
//  - Dataspace::BT2020_PQ
Dataspace SurfaceFlinger::getBestDataspace(const sp<DisplayDevice>& display,
                                           Dataspace* outHdrDataSpace,
                                           bool* outIsHdrClientComposition) const {
    Dataspace bestDataSpace = Dataspace::V0_SRGB;
    *outHdrDataSpace = Dataspace::UNKNOWN;

    for (const auto& layer : display->getVisibleLayersSortedByZ()) {
        switch (layer->getDataSpace()) {
            case Dataspace::V0_SCRGB:
            case Dataspace::V0_SCRGB_LINEAR:
            case Dataspace::BT2020:
            case Dataspace::BT2020_ITU:
            case Dataspace::BT2020_LINEAR:
            case Dataspace::DISPLAY_BT2020:
                bestDataSpace = Dataspace::DISPLAY_BT2020;
                break;
            case Dataspace::DISPLAY_P3:
                bestDataSpace = Dataspace::DISPLAY_P3;
                break;
            case Dataspace::BT2020_PQ:
            case Dataspace::BT2020_ITU_PQ:
                bestDataSpace = Dataspace::DISPLAY_P3;
                *outHdrDataSpace = Dataspace::BT2020_PQ;
                *outIsHdrClientComposition = layer->getForceClientComposition(display);
                break;
            case Dataspace::BT2020_HLG:
            case Dataspace::BT2020_ITU_HLG:
                bestDataSpace = Dataspace::DISPLAY_P3;
                // When there's mixed PQ content and HLG content, we set the HDR
                // data space to be BT2020_PQ and convert HLG to PQ.
                if (*outHdrDataSpace == Dataspace::UNKNOWN) {
                    *outHdrDataSpace = Dataspace::BT2020_HLG;
                }
                break;
            default:
                break;
        }
    }

    return bestDataSpace;
}

// Pick the ColorMode / Dataspace for the display device.
void SurfaceFlinger::pickColorMode(const sp<DisplayDevice>& display, ColorMode* outMode,
                                   Dataspace* outDataSpace, RenderIntent* outRenderIntent) const {
    if (mDisplayColorSetting == DisplayColorSetting::UNMANAGED) {
        *outMode = ColorMode::NATIVE;
        *outDataSpace = Dataspace::UNKNOWN;
        *outRenderIntent = RenderIntent::COLORIMETRIC;
        return;
    }

    Dataspace hdrDataSpace;
    bool isHdrClientComposition = false;
    Dataspace bestDataSpace = getBestDataspace(display, &hdrDataSpace, &isHdrClientComposition);

    auto* profile = display->getCompositionDisplay()->getDisplayColorProfile();

    switch (mForceColorMode) {
        case ColorMode::SRGB:
            bestDataSpace = Dataspace::V0_SRGB;
            break;
        case ColorMode::DISPLAY_P3:
            bestDataSpace = Dataspace::DISPLAY_P3;
            break;
        default:
            break;
    }

    // respect hdrDataSpace only when there is no legacy HDR support
    const bool isHdr = hdrDataSpace != Dataspace::UNKNOWN &&
            !profile->hasLegacyHdrSupport(hdrDataSpace) && !isHdrClientComposition;
    if (isHdr) {
        bestDataSpace = hdrDataSpace;
    }

    RenderIntent intent;
    switch (mDisplayColorSetting) {
        case DisplayColorSetting::MANAGED:
        case DisplayColorSetting::UNMANAGED:
            intent = isHdr ? RenderIntent::TONE_MAP_COLORIMETRIC : RenderIntent::COLORIMETRIC;
            break;
        case DisplayColorSetting::ENHANCED:
            intent = isHdr ? RenderIntent::TONE_MAP_ENHANCE : RenderIntent::ENHANCE;
            break;
        default: // vendor display color setting
            intent = static_cast<RenderIntent>(mDisplayColorSetting);
            break;
    }

    profile->getBestColorMode(bestDataSpace, intent, outDataSpace, outMode, outRenderIntent);
}

void SurfaceFlinger::beginFrame(const sp<DisplayDevice>& displayDevice) {
    auto display = displayDevice->getCompositionDisplay();
    const auto& displayState = display->getState();

    bool dirty = !display->getDirtyRegion(false).isEmpty();
    bool empty = displayDevice->getVisibleLayersSortedByZ().size() == 0;
    bool wasEmpty = !displayState.lastCompositionHadVisibleLayers;

    // If nothing has changed (!dirty), don't recompose.
    // If something changed, but we don't currently have any visible layers,
    //   and didn't when we last did a composition, then skip it this time.
    // The second rule does two things:
    // - When all layers are removed from a display, we'll emit one black
    //   frame, then nothing more until we get new layers.
    // - When a display is created with a private layer stack, we won't
    //   emit any black frames until a layer is added to the layer stack.
    bool mustRecompose = dirty && !(empty && wasEmpty);

    const char flagPrefix[] = {'-', '+'};
    static_cast<void>(flagPrefix);
    ALOGV_IF(displayDevice->isVirtual(), "%s: %s composition for %s (%cdirty %cempty %cwasEmpty)",
             __FUNCTION__, mustRecompose ? "doing" : "skipping",
             displayDevice->getDebugName().c_str(), flagPrefix[dirty], flagPrefix[empty],
             flagPrefix[wasEmpty]);

    display->getRenderSurface()->beginFrame(mustRecompose);

    if (mustRecompose) {
        display->editState().lastCompositionHadVisibleLayers = !empty;
    }
}

void SurfaceFlinger::prepareFrame(const sp<DisplayDevice>& displayDevice) {
    auto display = displayDevice->getCompositionDisplay();
    const auto& displayState = display->getState();

    if (!displayState.isEnabled) {
        return;
    }

    status_t result = display->getRenderSurface()->prepareFrame();
    ALOGE_IF(result != NO_ERROR, "prepareFrame failed for %s: %d (%s)",
             displayDevice->getDebugName().c_str(), result, strerror(-result));
}

void SurfaceFlinger::doComposition(const sp<DisplayDevice>& displayDevice, bool repaintEverything) {
    ATRACE_CALL();
    ALOGV("doComposition");

    auto display = displayDevice->getCompositionDisplay();
    const auto& displayState = display->getState();

    if (displayState.isEnabled) {
        // transform the dirty region into this screen's coordinate space
        const Region dirtyRegion = display->getDirtyRegion(repaintEverything);

        // repaint the framebuffer (if needed)
        doDisplayComposition(displayDevice, dirtyRegion);

        display->editState().dirtyRegion.clear();
        display->getRenderSurface()->flip();
    }
    postFramebuffer(displayDevice);
}

void SurfaceFlinger::postFrame()
{
    // |mStateLock| not needed as we are on the main thread
    const auto display = getDefaultDisplayDeviceLocked();
    if (display && getHwComposer().isConnected(*display->getId())) {
        uint32_t flipCount = display->getPageFlipCount();
        if (flipCount % LOG_FRAME_STATS_PERIOD == 0) {
            logFrameStats();
        }
    }
}

void SurfaceFlinger::postFramebuffer(const sp<DisplayDevice>& displayDevice) {
    ATRACE_CALL();
    ALOGV("postFramebuffer");

    auto display = displayDevice->getCompositionDisplay();
    const auto& displayState = display->getState();
    const auto displayId = display->getId();

    if (displayState.isEnabled) {
        if (displayId) {
            getHwComposer().presentAndGetReleaseFences(*displayId);
        }
        display->getRenderSurface()->onPresentDisplayCompleted();
        for (auto& layer : display->getOutputLayersOrderedByZ()) {
            sp<Fence> releaseFence = Fence::NO_FENCE;
            bool usedClientComposition = true;

            // The layer buffer from the previous frame (if any) is released
            // by HWC only when the release fence from this frame (if any) is
            // signaled.  Always get the release fence from HWC first.
            if (layer->getState().hwc) {
                const auto& hwcState = *layer->getState().hwc;
                releaseFence =
                        getHwComposer().getLayerReleaseFence(*displayId, hwcState.hwcLayer.get());
                usedClientComposition =
                        hwcState.hwcCompositionType == Hwc2::IComposerClient::Composition::CLIENT;
            }

            // If the layer was client composited in the previous frame, we
            // need to merge with the previous client target acquire fence.
            // Since we do not track that, always merge with the current
            // client target acquire fence when it is available, even though
            // this is suboptimal.
            if (usedClientComposition) {
                releaseFence =
                        Fence::merge("LayerRelease", releaseFence,
                                     display->getRenderSurface()->getClientTargetAcquireFence());
            }

            layer->getLayerFE().onLayerDisplayed(releaseFence);
        }

        // We've got a list of layers needing fences, that are disjoint with
        // display->getVisibleLayersSortedByZ.  The best we can do is to
        // supply them with the present fence.
        if (!displayDevice->getLayersNeedingFences().isEmpty()) {
            sp<Fence> presentFence =
                    displayId ? getHwComposer().getPresentFence(*displayId) : Fence::NO_FENCE;
            for (auto& layer : displayDevice->getLayersNeedingFences()) {
                layer->getCompositionLayer()->getLayerFE()->onLayerDisplayed(presentFence);
            }
        }

        if (displayId) {
            getHwComposer().clearReleaseFences(*displayId);
        }
    }
}

void SurfaceFlinger::handleTransaction(uint32_t transactionFlags)
{
    ATRACE_CALL();

    // here we keep a copy of the drawing state (that is the state that's
    // going to be overwritten by handleTransactionLocked()) outside of
    // mStateLock so that the side-effects of the State assignment
    // don't happen with mStateLock held (which can cause deadlocks).
    State drawingState(mDrawingState);

    Mutex::Autolock _l(mStateLock);
    mDebugInTransaction = systemTime();

    // Here we're guaranteed that some transaction flags are set
    // so we can call handleTransactionLocked() unconditionally.
    // We call getTransactionFlags(), which will also clear the flags,
    // with mStateLock held to guarantee that mCurrentState won't change
    // until the transaction is committed.

    mVsyncModulator.onTransactionHandled();
    transactionFlags = getTransactionFlags(eTransactionMask);
    handleTransactionLocked(transactionFlags);

    mDebugInTransaction = 0;
    invalidateHwcGeometry();
    // here the transaction has been committed
}

void SurfaceFlinger::processDisplayHotplugEventsLocked() {
    for (const auto& event : mPendingHotplugEvents) {
        const std::optional<DisplayIdentificationInfo> info =
                getHwComposer().onHotplug(event.hwcDisplayId, event.connection);

        if (!info) {
            continue;
        }

        if (event.connection == HWC2::Connection::Connected) {
            if (!mPhysicalDisplayTokens.count(info->id)) {
                ALOGV("Creating display %s", to_string(info->id).c_str());
                if (event.hwcDisplayId == getHwComposer().getInternalHwcDisplayId()) {
                    initScheduler(info->id);
                }
                mPhysicalDisplayTokens[info->id] = new BBinder();
                DisplayDeviceState state;
                state.displayId = info->id;
                state.isSecure = true; // All physical displays are currently considered secure.
                state.displayName = info->name;
                mCurrentState.displays.add(mPhysicalDisplayTokens[info->id], state);
                mInterceptor->saveDisplayCreation(state);
            }
        } else {
            ALOGV("Removing display %s", to_string(info->id).c_str());

            ssize_t index = mCurrentState.displays.indexOfKey(mPhysicalDisplayTokens[info->id]);
            if (index >= 0) {
                const DisplayDeviceState& state = mCurrentState.displays.valueAt(index);
                mInterceptor->saveDisplayDeletion(state.sequenceId);
                mCurrentState.displays.removeItemsAt(index);
            }
            mPhysicalDisplayTokens.erase(info->id);
        }

        processDisplayChangesLocked();
    }

    mPendingHotplugEvents.clear();
}

void SurfaceFlinger::dispatchDisplayHotplugEvent(PhysicalDisplayId displayId, bool connected) {
    mScheduler->hotplugReceived(mAppConnectionHandle, displayId, connected);
    mScheduler->hotplugReceived(mSfConnectionHandle, displayId, connected);
}

sp<DisplayDevice> SurfaceFlinger::setupNewDisplayDeviceInternal(
        const wp<IBinder>& displayToken, const std::optional<DisplayId>& displayId,
        const DisplayDeviceState& state, const sp<compositionengine::DisplaySurface>& dispSurface,
        const sp<IGraphicBufferProducer>& producer) {
    DisplayDeviceCreationArgs creationArgs(this, displayToken, displayId);
    creationArgs.sequenceId = state.sequenceId;
    creationArgs.isVirtual = state.isVirtual();
    creationArgs.isSecure = state.isSecure;
    creationArgs.displaySurface = dispSurface;
    creationArgs.hasWideColorGamut = false;
    creationArgs.supportedPerFrameMetadata = 0;

    const bool isInternalDisplay = displayId && displayId == getInternalDisplayIdLocked();
    creationArgs.isPrimary = isInternalDisplay;

    if (useColorManagement && displayId) {
        std::vector<ColorMode> modes = getHwComposer().getColorModes(*displayId);
        for (ColorMode colorMode : modes) {
            if (isWideColorMode(colorMode)) {
                creationArgs.hasWideColorGamut = true;
            }

            std::vector<RenderIntent> renderIntents =
                    getHwComposer().getRenderIntents(*displayId, colorMode);
            creationArgs.hwcColorModes.emplace(colorMode, renderIntents);
        }
    }

    if (displayId) {
        getHwComposer().getHdrCapabilities(*displayId, &creationArgs.hdrCapabilities);
        creationArgs.supportedPerFrameMetadata =
                getHwComposer().getSupportedPerFrameMetadata(*displayId);
    }

    auto nativeWindowSurface = getFactory().createNativeWindowSurface(producer);
    auto nativeWindow = nativeWindowSurface->getNativeWindow();
    creationArgs.nativeWindow = nativeWindow;

    // Make sure that composition can never be stalled by a virtual display
    // consumer that isn't processing buffers fast enough. We have to do this
    // here, in case the display is composed entirely by HWC.
    if (state.isVirtual()) {
        nativeWindow->setSwapInterval(nativeWindow.get(), 0);
    }

    creationArgs.displayInstallOrientation =
            isInternalDisplay ? primaryDisplayOrientation : DisplayState::eOrientationDefault;

    // virtual displays are always considered enabled
    creationArgs.initialPowerMode = state.isVirtual() ? HWC_POWER_MODE_NORMAL : HWC_POWER_MODE_OFF;

    sp<DisplayDevice> display = getFactory().createDisplayDevice(std::move(creationArgs));

    if (maxFrameBufferAcquiredBuffers >= 3) {
        nativeWindowSurface->preallocateBuffers();
    }

    ColorMode defaultColorMode = ColorMode::NATIVE;
    Dataspace defaultDataSpace = Dataspace::UNKNOWN;
    if (display->hasWideColorGamut()) {
        defaultColorMode = ColorMode::SRGB;
        defaultDataSpace = Dataspace::V0_SRGB;
    }
    display->getCompositionDisplay()->setColorMode(defaultColorMode, defaultDataSpace,
                                                   RenderIntent::COLORIMETRIC);
    if (!state.isVirtual()) {
        LOG_ALWAYS_FATAL_IF(!displayId);
        display->setActiveConfig(getHwComposer().getActiveConfigIndex(*displayId));
    }

    display->setLayerStack(state.layerStack);
    display->setProjection(state.orientation, state.viewport, state.frame);
    display->setDisplayName(state.displayName);

    return display;
}

void SurfaceFlinger::processDisplayChangesLocked() {
    // here we take advantage of Vector's copy-on-write semantics to
    // improve performance by skipping the transaction entirely when
    // know that the lists are identical
    const KeyedVector<wp<IBinder>, DisplayDeviceState>& curr(mCurrentState.displays);
    const KeyedVector<wp<IBinder>, DisplayDeviceState>& draw(mDrawingState.displays);
    if (!curr.isIdenticalTo(draw)) {
        mVisibleRegionsDirty = true;
        const size_t cc = curr.size();
        size_t dc = draw.size();

        // find the displays that were removed
        // (ie: in drawing state but not in current state)
        // also handle displays that changed
        // (ie: displays that are in both lists)
        for (size_t i = 0; i < dc;) {
            const ssize_t j = curr.indexOfKey(draw.keyAt(i));
            if (j < 0) {
                // in drawing state but not in current state
                if (const auto display = getDisplayDeviceLocked(draw.keyAt(i))) {
                    // Save display ID before disconnecting.
                    const auto displayId = display->getId();
                    display->disconnect();

                    if (!display->isVirtual()) {
                        LOG_ALWAYS_FATAL_IF(!displayId);
                        dispatchDisplayHotplugEvent(displayId->value, false);
                    }
                }

                mDisplays.erase(draw.keyAt(i));
            } else {
                // this display is in both lists. see if something changed.
                const DisplayDeviceState& state(curr[j]);
                const wp<IBinder>& displayToken = curr.keyAt(j);
                const sp<IBinder> state_binder = IInterface::asBinder(state.surface);
                const sp<IBinder> draw_binder = IInterface::asBinder(draw[i].surface);
                if (state_binder != draw_binder) {
                    // changing the surface is like destroying and
                    // recreating the DisplayDevice, so we just remove it
                    // from the drawing state, so that it get re-added
                    // below.
                    if (const auto display = getDisplayDeviceLocked(displayToken)) {
                        display->disconnect();
                    }
                    mDisplays.erase(displayToken);
                    mDrawingState.displays.removeItemsAt(i);
                    dc--;
                    // at this point we must loop to the next item
                    continue;
                }

                if (const auto display = getDisplayDeviceLocked(displayToken)) {
                    if (state.layerStack != draw[i].layerStack) {
                        display->setLayerStack(state.layerStack);
                    }
                    if ((state.orientation != draw[i].orientation) ||
                        (state.viewport != draw[i].viewport) || (state.frame != draw[i].frame)) {
                        display->setProjection(state.orientation, state.viewport, state.frame);
                    }
                    if (state.width != draw[i].width || state.height != draw[i].height) {
                        display->setDisplaySize(state.width, state.height);
                    }
                }
            }
            ++i;
        }

        // find displays that were added
        // (ie: in current state but not in drawing state)
        for (size_t i = 0; i < cc; i++) {
            if (draw.indexOfKey(curr.keyAt(i)) < 0) {
                const DisplayDeviceState& state(curr[i]);

                sp<compositionengine::DisplaySurface> dispSurface;
                sp<IGraphicBufferProducer> producer;
                sp<IGraphicBufferProducer> bqProducer;
                sp<IGraphicBufferConsumer> bqConsumer;
                getFactory().createBufferQueue(&bqProducer, &bqConsumer, false);

                std::optional<DisplayId> displayId;
                if (state.isVirtual()) {
                    // Virtual displays without a surface are dormant:
                    // they have external state (layer stack, projection,
                    // etc.) but no internal state (i.e. a DisplayDevice).
                    if (state.surface != nullptr) {
                        // Allow VR composer to use virtual displays.
                        if (mUseHwcVirtualDisplays || getHwComposer().isUsingVrComposer()) {
                            int width = 0;
                            int status = state.surface->query(NATIVE_WINDOW_WIDTH, &width);
                            ALOGE_IF(status != NO_ERROR, "Unable to query width (%d)", status);
                            int height = 0;
                            status = state.surface->query(NATIVE_WINDOW_HEIGHT, &height);
                            ALOGE_IF(status != NO_ERROR, "Unable to query height (%d)", status);
                            int intFormat = 0;
                            status = state.surface->query(NATIVE_WINDOW_FORMAT, &intFormat);
                            ALOGE_IF(status != NO_ERROR, "Unable to query format (%d)", status);
                            auto format = static_cast<ui::PixelFormat>(intFormat);

                            displayId =
                                    getHwComposer().allocateVirtualDisplay(width, height, &format);
                        }

                        // TODO: Plumb requested format back up to consumer

                        sp<VirtualDisplaySurface> vds =
                                new VirtualDisplaySurface(getHwComposer(), displayId, state.surface,
                                                          bqProducer, bqConsumer,
                                                          state.displayName);

                        dispSurface = vds;
                        producer = vds;
                    }
                } else {
                    ALOGE_IF(state.surface != nullptr,
                             "adding a supported display, but rendering "
                             "surface is provided (%p), ignoring it",
                             state.surface.get());

                    displayId = state.displayId;
                    LOG_ALWAYS_FATAL_IF(!displayId);
                    dispSurface = new FramebufferSurface(getHwComposer(), *displayId, bqConsumer);
                    producer = bqProducer;
                }

                const wp<IBinder>& displayToken = curr.keyAt(i);
                if (dispSurface != nullptr) {
                    mDisplays.emplace(displayToken,
                                      setupNewDisplayDeviceInternal(displayToken, displayId, state,
                                                                    dispSurface, producer));
                    if (!state.isVirtual()) {
                        LOG_ALWAYS_FATAL_IF(!displayId);
                        dispatchDisplayHotplugEvent(displayId->value, true);
                    }
                }
            }
        }
    }

    mDrawingState.displays = mCurrentState.displays;
}

void SurfaceFlinger::handleTransactionLocked(uint32_t transactionFlags)
{
    // Notify all layers of available frames
    mCurrentState.traverseInZOrder([](Layer* layer) {
        layer->notifyAvailableFrames();
    });

    /*
     * Traversal of the children
     * (perform the transaction for each of them if needed)
     */

    if ((transactionFlags & eTraversalNeeded) || mTraversalNeededMainThread) {
        mCurrentState.traverseInZOrder([&](Layer* layer) {
            uint32_t trFlags = layer->getTransactionFlags(eTransactionNeeded);
            if (!trFlags) return;

            const uint32_t flags = layer->doTransaction(0);
            if (flags & Layer::eVisibleRegion)
                mVisibleRegionsDirty = true;

            if (flags & Layer::eInputInfoChanged) {
                mInputInfoChanged = true;
            }
        });
        mTraversalNeededMainThread = false;
    }

    /*
     * Perform display own transactions if needed
     */

    if (transactionFlags & eDisplayTransactionNeeded) {
        processDisplayChangesLocked();
        processDisplayHotplugEventsLocked();
    }

    if (transactionFlags & (eDisplayLayerStackChanged|eDisplayTransactionNeeded)) {
        // The transform hint might have changed for some layers
        // (either because a display has changed, or because a layer
        // as changed).
        //
        // Walk through all the layers in currentLayers,
        // and update their transform hint.
        //
        // If a layer is visible only on a single display, then that
        // display is used to calculate the hint, otherwise we use the
        // default display.
        //
        // NOTE: we do this here, rather than in rebuildLayerStacks() so that
        // the hint is set before we acquire a buffer from the surface texture.
        //
        // NOTE: layer transactions have taken place already, so we use their
        // drawing state. However, SurfaceFlinger's own transaction has not
        // happened yet, so we must use the current state layer list
        // (soon to become the drawing state list).
        //
        sp<const DisplayDevice> hintDisplay;
        uint32_t currentlayerStack = 0;
        bool first = true;
        mCurrentState.traverseInZOrder([&](Layer* layer) {
            // NOTE: we rely on the fact that layers are sorted by
            // layerStack first (so we don't have to traverse the list
            // of displays for every layer).
            uint32_t layerStack = layer->getLayerStack();
            if (first || currentlayerStack != layerStack) {
                currentlayerStack = layerStack;
                // figure out if this layerstack is mirrored
                // (more than one display) if so, pick the default display,
                // if not, pick the only display it's on.
                hintDisplay = nullptr;
                for (const auto& [token, display] : mDisplays) {
                    if (display->getCompositionDisplay()
                                ->belongsInOutput(layer->getLayerStack(),
                                                  layer->getPrimaryDisplayOnly())) {
                        if (hintDisplay) {
                            hintDisplay = nullptr;
                            break;
                        } else {
                            hintDisplay = display;
                        }
                    }
                }
            }

            if (!hintDisplay) {
                // NOTE: TEMPORARY FIX ONLY. Real fix should cause layers to
                // redraw after transform hint changes. See bug 8508397.

                // could be null when this layer is using a layerStack
                // that is not visible on any display. Also can occur at
                // screen off/on times.
                hintDisplay = getDefaultDisplayDeviceLocked();
            }

            // could be null if there is no display available at all to get
            // the transform hint from.
            if (hintDisplay) {
                layer->updateTransformHint(hintDisplay);
            }

            first = false;
        });
    }


    /*
     * Perform our own transaction if needed
     */

    if (mLayersAdded) {
        mLayersAdded = false;
        // Layers have been added.
        mVisibleRegionsDirty = true;
    }

    // some layers might have been removed, so
    // we need to update the regions they're exposing.
    if (mLayersRemoved) {
        mLayersRemoved = false;
        mVisibleRegionsDirty = true;
        mDrawingState.traverseInZOrder([&](Layer* layer) {
            if (mLayersPendingRemoval.indexOf(layer) >= 0) {
                // this layer is not visible anymore
                Region visibleReg;
                visibleReg.set(layer->getScreenBounds());
                invalidateLayerStack(layer, visibleReg);
            }
        });
    }

    commitInputWindowCommands();
    commitTransaction();
}

void SurfaceFlinger::updateInputFlinger() {
    ATRACE_CALL();
    if (!mInputFlinger) {
        return;
    }

    if (mVisibleRegionsDirty || mInputInfoChanged) {
        mInputInfoChanged = false;
        updateInputWindowInfo();
    } else if (mInputWindowCommands.syncInputWindows) {
        // If the caller requested to sync input windows, but there are no
        // changes to input windows, notify immediately.
        setInputWindowsFinished();
    }

    mInputWindowCommands.clear();
}

void SurfaceFlinger::updateInputWindowInfo() {
    std::vector<InputWindowInfo> inputHandles;

    mDrawingState.traverseInReverseZOrder([&](Layer* layer) {
        if (layer->hasInput()) {
            // When calculating the screen bounds we ignore the transparent region since it may
            // result in an unwanted offset.
            inputHandles.push_back(layer->fillInputInfo());
        }
    });

    mInputFlinger->setInputWindows(inputHandles,
                                   mInputWindowCommands.syncInputWindows ? mSetInputWindowsListener
                                                                         : nullptr);
}

void SurfaceFlinger::commitInputWindowCommands() {
    mInputWindowCommands = mPendingInputWindowCommands;
    mPendingInputWindowCommands.clear();
}

void SurfaceFlinger::updateCursorAsync()
{
    for (const auto& [token, display] : mDisplays) {
        if (!display->getId()) {
            continue;
        }

        for (auto& layer : display->getVisibleLayersSortedByZ()) {
            layer->updateCursorPosition(display);
        }
    }
}

void SurfaceFlinger::latchAndReleaseBuffer(const sp<Layer>& layer) {
    if (layer->hasReadyFrame()) {
        bool ignored = false;
        layer->latchBuffer(ignored, systemTime());
    }
    layer->releasePendingBuffer(systemTime());
}

void SurfaceFlinger::initScheduler(DisplayId primaryDisplayId) {
    if (mScheduler) {
        // In practice it's not allowed to hotplug in/out the primary display once it's been
        // connected during startup, but some tests do it, so just warn and return.
        ALOGW("Can't re-init scheduler");
        return;
    }

    int currentConfig = getHwComposer().getActiveConfigIndex(primaryDisplayId);
    mRefreshRateConfigs =
            std::make_unique<scheduler::RefreshRateConfigs>(refresh_rate_switching(false),
                                                            getHwComposer().getConfigs(
                                                                    primaryDisplayId),
                                                            currentConfig);
    mRefreshRateStats =
            std::make_unique<scheduler::RefreshRateStats>(*mRefreshRateConfigs, *mTimeStats,
                                                          currentConfig, HWC_POWER_MODE_OFF);
    mRefreshRateStats->setConfigMode(currentConfig);

    // start the EventThread
    mScheduler =
            getFactory().createScheduler([this](bool enabled) { setPrimaryVsyncEnabled(enabled); },
                                         *mRefreshRateConfigs);
    mAppConnectionHandle =
            mScheduler->createConnection("app", mVsyncModulator.getOffsets().app,
                                         mPhaseOffsets->getOffsetThresholdForNextVsync(),
                                         impl::EventThread::InterceptVSyncsCallback());
    mSfConnectionHandle =
            mScheduler->createConnection("sf", mVsyncModulator.getOffsets().sf,
                                         mPhaseOffsets->getOffsetThresholdForNextVsync(),
                                         [this](nsecs_t timestamp) {
                                             mInterceptor->saveVSyncEvent(timestamp);
                                         });

    mEventQueue->setEventConnection(mScheduler->getEventConnection(mSfConnectionHandle));
    mVsyncModulator.setSchedulerAndHandles(mScheduler.get(), mAppConnectionHandle.get(),
                                           mSfConnectionHandle.get());

    mRegionSamplingThread =
            new RegionSamplingThread(*this, *mScheduler,
                                     RegionSamplingThread::EnvironmentTimingTunables());

    mScheduler->setChangeRefreshRateCallback(
            [this](RefreshRateType type, Scheduler::ConfigEvent event) {
                Mutex::Autolock lock(mStateLock);
                setRefreshRateTo(type, event);
            });
}

void SurfaceFlinger::commitTransaction()
{
    if (!mLayersPendingRemoval.isEmpty()) {
        // Notify removed layers now that they can't be drawn from
        for (const auto& l : mLayersPendingRemoval) {
            recordBufferingStats(l->getName().string(),
                    l->getOccupancyHistory(true));

            // Ensure any buffers set to display on any children are released.
            if (l->isRemovedFromCurrentState()) {
                latchAndReleaseBuffer(l);
            }

            // If the layer has been removed and has no parent, then it will not be reachable
            // when traversing layers on screen. Add the layer to the offscreenLayers set to
            // ensure we can copy its current to drawing state.
            if (!l->getParent()) {
                mOffscreenLayers.emplace(l.get());
            }
        }
        mLayersPendingRemoval.clear();
    }

    // If this transaction is part of a window animation then the next frame
    // we composite should be considered an animation as well.
    mAnimCompositionPending = mAnimTransactionPending;

    withTracingLock([&]() {
        mDrawingState = mCurrentState;
        // clear the "changed" flags in current state
        mCurrentState.colorMatrixChanged = false;

        mDrawingState.traverseInZOrder([&](Layer* layer) {
            layer->commitChildList();

            // If the layer can be reached when traversing mDrawingState, then the layer is no
            // longer offscreen. Remove the layer from the offscreenLayer set.
            if (mOffscreenLayers.count(layer)) {
                mOffscreenLayers.erase(layer);
            }
        });

        commitOffscreenLayers();
    });

    mTransactionPending = false;
    mAnimTransactionPending = false;
    mTransactionCV.broadcast();
}

void SurfaceFlinger::withTracingLock(std::function<void()> lockedOperation) {
    if (mTracingEnabledChanged) {
        mTracingEnabled = mTracing.isEnabled();
        mTracingEnabledChanged = false;
    }

    // Synchronize with Tracing thread
    std::unique_lock<std::mutex> lock;
    if (mTracingEnabled) {
        lock = std::unique_lock<std::mutex>(mDrawingStateLock);
    }

    lockedOperation();

    // Synchronize with Tracing thread
    if (mTracingEnabled) {
        lock.unlock();
    }
}

void SurfaceFlinger::commitOffscreenLayers() {
    for (Layer* offscreenLayer : mOffscreenLayers) {
        offscreenLayer->traverseInZOrder(LayerVector::StateSet::Drawing, [](Layer* layer) {
            uint32_t trFlags = layer->getTransactionFlags(eTransactionNeeded);
            if (!trFlags) return;

            layer->doTransaction(0);
            layer->commitChildList();
        });
    }
}

void SurfaceFlinger::computeVisibleRegions(const sp<const DisplayDevice>& displayDevice,
                                           Region& outDirtyRegion, Region& outOpaqueRegion) {
    ATRACE_CALL();
    ALOGV("computeVisibleRegions");

    auto display = displayDevice->getCompositionDisplay();

    Region aboveOpaqueLayers;
    Region aboveCoveredLayers;
    Region dirty;

    outDirtyRegion.clear();

    mDrawingState.traverseInReverseZOrder([&](Layer* layer) {
        // start with the whole surface at its current location
        const Layer::State& s(layer->getDrawingState());

        // only consider the layers on the given layer stack
        if (!display->belongsInOutput(layer->getLayerStack(), layer->getPrimaryDisplayOnly())) {
            return;
        }

        /*
         * opaqueRegion: area of a surface that is fully opaque.
         */
        Region opaqueRegion;

        /*
         * visibleRegion: area of a surface that is visible on screen
         * and not fully transparent. This is essentially the layer's
         * footprint minus the opaque regions above it.
         * Areas covered by a translucent surface are considered visible.
         */
        Region visibleRegion;

        /*
         * coveredRegion: area of a surface that is covered by all
         * visible regions above it (which includes the translucent areas).
         */
        Region coveredRegion;

        /*
         * transparentRegion: area of a surface that is hinted to be completely
         * transparent. This is only used to tell when the layer has no visible
         * non-transparent regions and can be removed from the layer list. It
         * does not affect the visibleRegion of this layer or any layers
         * beneath it. The hint may not be correct if apps don't respect the
         * SurfaceView restrictions (which, sadly, some don't).
         */
        Region transparentRegion;


        // handle hidden surfaces by setting the visible region to empty
        if (CC_LIKELY(layer->isVisible())) {
            const bool translucent = !layer->isOpaque(s);
            Rect bounds(layer->getScreenBounds());

            visibleRegion.set(bounds);
            ui::Transform tr = layer->getTransform();
            if (!visibleRegion.isEmpty()) {
                // Remove the transparent area from the visible region
                if (translucent) {
                    if (tr.preserveRects()) {
                        // transform the transparent region
                        transparentRegion = tr.transform(layer->getActiveTransparentRegion(s));
                    } else {
                        // transformation too complex, can't do the
                        // transparent region optimization.
                        transparentRegion.clear();
                    }
                }

                // compute the opaque region
                const int32_t layerOrientation = tr.getOrientation();
                if (layer->getAlpha() == 1.0f && !translucent &&
                        layer->getRoundedCornerState().radius == 0.0f &&
                        ((layerOrientation & ui::Transform::ROT_INVALID) == false)) {
                    // the opaque region is the layer's footprint
                    opaqueRegion = visibleRegion;
                }
            }
        }

        if (visibleRegion.isEmpty()) {
            layer->clearVisibilityRegions();
            return;
        }

        // Clip the covered region to the visible region
        coveredRegion = aboveCoveredLayers.intersect(visibleRegion);

        // Update aboveCoveredLayers for next (lower) layer
        aboveCoveredLayers.orSelf(visibleRegion);

        // subtract the opaque region covered by the layers above us
        visibleRegion.subtractSelf(aboveOpaqueLayers);

        // compute this layer's dirty region
        if (layer->contentDirty) {
            // we need to invalidate the whole region
            dirty = visibleRegion;
            // as well, as the old visible region
            dirty.orSelf(layer->visibleRegion);
            layer->contentDirty = false;
        } else {
            /* compute the exposed region:
             *   the exposed region consists of two components:
             *   1) what's VISIBLE now and was COVERED before
             *   2) what's EXPOSED now less what was EXPOSED before
             *
             * note that (1) is conservative, we start with the whole
             * visible region but only keep what used to be covered by
             * something -- which mean it may have been exposed.
             *
             * (2) handles areas that were not covered by anything but got
             * exposed because of a resize.
             */
            const Region newExposed = visibleRegion - coveredRegion;
            const Region oldVisibleRegion = layer->visibleRegion;
            const Region oldCoveredRegion = layer->coveredRegion;
            const Region oldExposed = oldVisibleRegion - oldCoveredRegion;
            dirty = (visibleRegion&oldCoveredRegion) | (newExposed-oldExposed);
        }
        dirty.subtractSelf(aboveOpaqueLayers);

        // accumulate to the screen dirty region
        outDirtyRegion.orSelf(dirty);

        // Update aboveOpaqueLayers for next (lower) layer
        aboveOpaqueLayers.orSelf(opaqueRegion);

        // Store the visible region in screen space
        layer->setVisibleRegion(visibleRegion);
        layer->setCoveredRegion(coveredRegion);
        layer->setVisibleNonTransparentRegion(
                visibleRegion.subtract(transparentRegion));
    });

    outOpaqueRegion = aboveOpaqueLayers;
}

void SurfaceFlinger::invalidateLayerStack(const sp<const Layer>& layer, const Region& dirty) {
    for (const auto& [token, displayDevice] : mDisplays) {
        auto display = displayDevice->getCompositionDisplay();
        if (display->belongsInOutput(layer->getLayerStack(), layer->getPrimaryDisplayOnly())) {
            display->editState().dirtyRegion.orSelf(dirty);
        }
    }
}

bool SurfaceFlinger::handlePageFlip()
{
    ATRACE_CALL();
    ALOGV("handlePageFlip");

    nsecs_t latchTime = systemTime();

    bool visibleRegions = false;
    bool frameQueued = false;
    bool newDataLatched = false;

    // Store the set of layers that need updates. This set must not change as
    // buffers are being latched, as this could result in a deadlock.
    // Example: Two producers share the same command stream and:
    // 1.) Layer 0 is latched
    // 2.) Layer 0 gets a new frame
    // 2.) Layer 1 gets a new frame
    // 3.) Layer 1 is latched.
    // Display is now waiting on Layer 1's frame, which is behind layer 0's
    // second frame. But layer 0's second frame could be waiting on display.
    mDrawingState.traverseInZOrder([&](Layer* layer) {
        if (layer->hasReadyFrame()) {
            frameQueued = true;
            const nsecs_t expectedPresentTime = getExpectedPresentTime();
            if (layer->shouldPresentNow(expectedPresentTime)) {
                mLayersWithQueuedFrames.push_back(layer);
            } else {
                ATRACE_NAME("!layer->shouldPresentNow()");
                layer->useEmptyDamage();
            }
        } else {
            layer->useEmptyDamage();
        }
    });

    if (!mLayersWithQueuedFrames.empty()) {
        // mStateLock is needed for latchBuffer as LayerRejecter::reject()
        // writes to Layer current state. See also b/119481871
        Mutex::Autolock lock(mStateLock);

        for (auto& layer : mLayersWithQueuedFrames) {
            if (layer->latchBuffer(visibleRegions, latchTime)) {
                mLayersPendingRefresh.push_back(layer);
            }
            layer->useSurfaceDamage();
            if (layer->isBufferLatched()) {
                newDataLatched = true;
            }
        }
    }

    mVisibleRegionsDirty |= visibleRegions;

    // If we will need to wake up at some time in the future to deal with a
    // queued frame that shouldn't be displayed during this vsync period, wake
    // up during the next vsync period to check again.
    if (frameQueued && (mLayersWithQueuedFrames.empty() || !newDataLatched)) {
        signalLayerUpdate();
    }

    // enter boot animation on first buffer latch
    if (CC_UNLIKELY(mBootStage == BootStage::BOOTLOADER && newDataLatched)) {
        ALOGI("Enter boot animation");
        mBootStage = BootStage::BOOTANIMATION;
    }

    // Only continue with the refresh if there is actually new work to do
    return !mLayersWithQueuedFrames.empty() && newDataLatched;
}

void SurfaceFlinger::invalidateHwcGeometry()
{
    mGeometryInvalid = true;
}

void SurfaceFlinger::doDisplayComposition(const sp<DisplayDevice>& displayDevice,
                                          const Region& inDirtyRegion) {
    auto display = displayDevice->getCompositionDisplay();
    // We only need to actually compose the display if:
    // 1) It is being handled by hardware composer, which may need this to
    //    keep its virtual display state machine in sync, or
    // 2) There is work to be done (the dirty region isn't empty)
    if (!displayDevice->getId() && inDirtyRegion.isEmpty()) {
        ALOGV("Skipping display composition");
        return;
    }

    ALOGV("doDisplayComposition");
    base::unique_fd readyFence;
    if (!doComposeSurfaces(displayDevice, Region::INVALID_REGION, &readyFence)) return;

    // swap buffers (presentation)
    display->getRenderSurface()->queueBuffer(std::move(readyFence));
}

bool SurfaceFlinger::doComposeSurfaces(const sp<DisplayDevice>& displayDevice,
                                       const Region& debugRegion, base::unique_fd* readyFence) {
    ATRACE_CALL();
    ALOGV("doComposeSurfaces");

    auto display = displayDevice->getCompositionDisplay();
    const auto& displayState = display->getState();
    const auto displayId = display->getId();
    auto& renderEngine = getRenderEngine();
    const bool supportProtectedContent = renderEngine.supportsProtectedContent();

    const Region bounds(displayState.bounds);
    const DisplayRenderArea renderArea(displayDevice);
    const bool hasClientComposition = getHwComposer().hasClientComposition(displayId);
    ATRACE_INT("hasClientComposition", hasClientComposition);

    bool applyColorMatrix = false;

    renderengine::DisplaySettings clientCompositionDisplay;
    std::vector<renderengine::LayerSettings> clientCompositionLayers;
    sp<GraphicBuffer> buf;
    base::unique_fd fd;

    if (hasClientComposition) {
        ALOGV("hasClientComposition");

        if (displayDevice->isPrimary() && supportProtectedContent) {
            bool needsProtected = false;
            for (auto& layer : displayDevice->getVisibleLayersSortedByZ()) {
                // If the layer is a protected layer, mark protected context is needed.
                if (layer->isProtected()) {
                    needsProtected = true;
                    break;
                }
            }
            if (needsProtected != renderEngine.isProtected()) {
                renderEngine.useProtectedContext(needsProtected);
            }
            if (needsProtected != display->getRenderSurface()->isProtected() &&
                needsProtected == renderEngine.isProtected()) {
                display->getRenderSurface()->setProtected(needsProtected);
            }
        }

        buf = display->getRenderSurface()->dequeueBuffer(&fd);

        if (buf == nullptr) {
            ALOGW("Dequeuing buffer for display [%s] failed, bailing out of "
                  "client composition for this frame",
                  displayDevice->getDisplayName().c_str());
            return false;
        }

        clientCompositionDisplay.physicalDisplay = displayState.scissor;
        clientCompositionDisplay.clip = displayState.scissor;
        const ui::Transform& displayTransform = displayState.transform;
        clientCompositionDisplay.globalTransform = displayTransform.asMatrix4();
        clientCompositionDisplay.orientation = displayState.orientation;

        const auto* profile = display->getDisplayColorProfile();
        Dataspace outputDataspace = Dataspace::UNKNOWN;
        if (profile->hasWideColorGamut()) {
            outputDataspace = displayState.dataspace;
        }
        clientCompositionDisplay.outputDataspace = outputDataspace;
        clientCompositionDisplay.maxLuminance =
                profile->getHdrCapabilities().getDesiredMaxLuminance();

        const bool hasDeviceComposition = getHwComposer().hasDeviceComposition(displayId);
        const bool skipClientColorTransform =
                getHwComposer()
                        .hasDisplayCapability(displayId,
                                              HWC2::DisplayCapability::SkipClientColorTransform);

        // Compute the global color transform matrix.
        applyColorMatrix = !hasDeviceComposition && !skipClientColorTransform;
        if (applyColorMatrix) {
            clientCompositionDisplay.colorTransform = displayState.colorTransformMat;
        }
    }

    /*
     * and then, render the layers targeted at the framebuffer
     */

    ALOGV("Rendering client layers");
    bool firstLayer = true;
    Region clearRegion = Region::INVALID_REGION;
    for (auto& layer : displayDevice->getVisibleLayersSortedByZ()) {
        const Region viewportRegion(displayState.viewport);
        const Region clip(viewportRegion.intersect(layer->visibleRegion));
        ALOGV("Layer: %s", layer->getName().string());
        ALOGV("  Composition type: %s", toString(layer->getCompositionType(displayDevice)).c_str());
        if (!clip.isEmpty()) {
            switch (layer->getCompositionType(displayDevice)) {
                case Hwc2::IComposerClient::Composition::CURSOR:
                case Hwc2::IComposerClient::Composition::DEVICE:
                case Hwc2::IComposerClient::Composition::SIDEBAND:
                case Hwc2::IComposerClient::Composition::SOLID_COLOR: {
                    LOG_ALWAYS_FATAL_IF(!displayId);
                    const Layer::State& state(layer->getDrawingState());
                    if (layer->getClearClientTarget(displayDevice) && !firstLayer &&
                        layer->isOpaque(state) && (layer->getAlpha() == 1.0f) &&
                        layer->getRoundedCornerState().radius == 0.0f && hasClientComposition) {
                        // never clear the very first layer since we're
                        // guaranteed the FB is already cleared
                        renderengine::LayerSettings layerSettings;
                        Region dummyRegion;
                        bool prepared =
                                layer->prepareClientLayer(renderArea, clip, dummyRegion,
                                                          supportProtectedContent, layerSettings);

                        if (prepared) {
                            layerSettings.source.buffer.buffer = nullptr;
                            layerSettings.source.solidColor = half3(0.0, 0.0, 0.0);
                            layerSettings.alpha = half(0.0);
                            layerSettings.disableBlending = true;
                            clientCompositionLayers.push_back(layerSettings);
                        }
                    }
                    break;
                }
                case Hwc2::IComposerClient::Composition::CLIENT: {
                    renderengine::LayerSettings layerSettings;
                    bool prepared =
                            layer->prepareClientLayer(renderArea, clip, clearRegion,
                                                      supportProtectedContent, layerSettings);
                    if (prepared) {
                        clientCompositionLayers.push_back(layerSettings);
                    }
                    break;
                }
                default:
                    break;
            }
        } else {
            ALOGV("  Skipping for empty clip");
        }
        firstLayer = false;
    }

    // Perform some cleanup steps if we used client composition.
    if (hasClientComposition) {
        clientCompositionDisplay.clearRegion = clearRegion;

        // We boost GPU frequency here because there will be color spaces conversion
        // and it's expensive. We boost the GPU frequency so that GPU composition can
        // finish in time. We must reset GPU frequency afterwards, because high frequency
        // consumes extra battery.
        const bool expensiveRenderingExpected =
                clientCompositionDisplay.outputDataspace == Dataspace::DISPLAY_P3;
        if (expensiveRenderingExpected && displayId) {
            mPowerAdvisor.setExpensiveRenderingExpected(*displayId, true);
        }
        if (!debugRegion.isEmpty()) {
            Region::const_iterator it = debugRegion.begin();
            Region::const_iterator end = debugRegion.end();
            while (it != end) {
                const Rect& rect = *it++;
                renderengine::LayerSettings layerSettings;
                layerSettings.source.buffer.buffer = nullptr;
                layerSettings.source.solidColor = half3(1.0, 0.0, 1.0);
                layerSettings.geometry.boundaries = rect.toFloatRect();
                layerSettings.alpha = half(1.0);
                clientCompositionLayers.push_back(layerSettings);
            }
        }
        renderEngine.drawLayers(clientCompositionDisplay, clientCompositionLayers,
                                buf->getNativeBuffer(), /*useFramebufferCache=*/true, std::move(fd),
                                readyFence);
    } else if (displayId) {
        mPowerAdvisor.setExpensiveRenderingExpected(*displayId, false);
    }
    return true;
}

void SurfaceFlinger::drawWormhole(const Region& region) const {
    auto& engine(getRenderEngine());
    engine.fillRegionWithColor(region, 0, 0, 0, 0);
}

status_t SurfaceFlinger::addClientLayer(const sp<Client>& client, const sp<IBinder>& handle,
                                        const sp<IGraphicBufferProducer>& gbc, const sp<Layer>& lbc,
                                        const sp<IBinder>& parentHandle,
                                        const sp<Layer>& parentLayer, bool addToCurrentState) {
    // add this layer to the current state list
    {
        Mutex::Autolock _l(mStateLock);
        sp<Layer> parent;
        if (parentHandle != nullptr) {
            parent = fromHandle(parentHandle);
            if (parent == nullptr) {
                return NAME_NOT_FOUND;
            }
        } else {
            parent = parentLayer;
        }

        if (mNumLayers >= MAX_LAYERS) {
            ALOGE("AddClientLayer failed, mNumLayers (%zu) >= MAX_LAYERS (%zu)", mNumLayers,
                  MAX_LAYERS);
            return NO_MEMORY;
        }

        mLayersByLocalBinderToken.emplace(handle->localBinder(), lbc);

        if (parent == nullptr && addToCurrentState) {
            mCurrentState.layersSortedByZ.add(lbc);
        } else if (parent == nullptr) {
            lbc->onRemovedFromCurrentState();
        } else if (parent->isRemovedFromCurrentState()) {
            parent->addChild(lbc);
            lbc->onRemovedFromCurrentState();
        } else {
            parent->addChild(lbc);
        }

        if (gbc != nullptr) {
            mGraphicBufferProducerList.insert(IInterface::asBinder(gbc).get());
            LOG_ALWAYS_FATAL_IF(mGraphicBufferProducerList.size() >
                                        mMaxGraphicBufferProducerListSize,
                                "Suspected IGBP leak: %zu IGBPs (%zu max), %zu Layers",
                                mGraphicBufferProducerList.size(),
                                mMaxGraphicBufferProducerListSize, mNumLayers);
        }
        mLayersAdded = true;
    }

    // attach this layer to the client
    client->attachLayer(handle, lbc);

    return NO_ERROR;
}

uint32_t SurfaceFlinger::peekTransactionFlags() {
    return mTransactionFlags;
}

uint32_t SurfaceFlinger::getTransactionFlags(uint32_t flags) {
    return mTransactionFlags.fetch_and(~flags) & flags;
}

uint32_t SurfaceFlinger::setTransactionFlags(uint32_t flags) {
    return setTransactionFlags(flags, Scheduler::TransactionStart::NORMAL);
}

uint32_t SurfaceFlinger::setTransactionFlags(uint32_t flags,
                                             Scheduler::TransactionStart transactionStart) {
    uint32_t old = mTransactionFlags.fetch_or(flags);
    mVsyncModulator.setTransactionStart(transactionStart);
    if ((old & flags)==0) { // wake the server up
        signalTransaction();
    }
    return old;
}

bool SurfaceFlinger::flushTransactionQueues() {
    // to prevent onHandleDestroyed from being called while the lock is held,
    // we must keep a copy of the transactions (specifically the composer
    // states) around outside the scope of the lock
    std::vector<const TransactionState> transactions;
    bool flushedATransaction = false;
    {
        Mutex::Autolock _l(mStateLock);

        auto it = mTransactionQueues.begin();
        while (it != mTransactionQueues.end()) {
            auto& [applyToken, transactionQueue] = *it;

            while (!transactionQueue.empty()) {
                const auto& transaction = transactionQueue.front();
                if (!transactionIsReadyToBeApplied(transaction.desiredPresentTime,
                                                   transaction.states)) {
                    setTransactionFlags(eTransactionFlushNeeded);
                    break;
                }
                transactions.push_back(transaction);
                applyTransactionState(transaction.states, transaction.displays, transaction.flags,
                                      mPendingInputWindowCommands, transaction.desiredPresentTime,
                                      transaction.buffer, transaction.callback,
                                      transaction.postTime, transaction.privileged,
                                      /*isMainThread*/ true);
                transactionQueue.pop();
                flushedATransaction = true;
            }

            if (transactionQueue.empty()) {
                it = mTransactionQueues.erase(it);
                mTransactionCV.broadcast();
            } else {
                it = std::next(it, 1);
            }
        }
    }
    return flushedATransaction;
}

bool SurfaceFlinger::transactionFlushNeeded() {
    return !mTransactionQueues.empty();
}

bool SurfaceFlinger::containsAnyInvalidClientState(const Vector<ComposerState>& states) {
    for (const ComposerState& state : states) {
        // Here we need to check that the interface we're given is indeed
        // one of our own. A malicious client could give us a nullptr
        // IInterface, or one of its own or even one of our own but a
        // different type. All these situations would cause us to crash.
        if (state.client == nullptr) {
            return true;
        }

        sp<IBinder> binder = IInterface::asBinder(state.client);
        if (binder == nullptr) {
            return true;
        }

        if (binder->queryLocalInterface(ISurfaceComposerClient::descriptor) == nullptr) {
            return true;
        }
    }
    return false;
}

bool SurfaceFlinger::transactionIsReadyToBeApplied(int64_t desiredPresentTime,
                                                   const Vector<ComposerState>& states) {
    nsecs_t expectedPresentTime = getExpectedPresentTime();
    // Do not present if the desiredPresentTime has not passed unless it is more than one second
    // in the future. We ignore timestamps more than 1 second in the future for stability reasons.
    if (desiredPresentTime >= 0 && desiredPresentTime >= expectedPresentTime &&
        desiredPresentTime < expectedPresentTime + s2ns(1)) {
        return false;
    }

    for (const ComposerState& state : states) {
        const layer_state_t& s = state.state;
        if (!(s.what & layer_state_t::eAcquireFenceChanged)) {
            continue;
        }
        if (s.acquireFence && s.acquireFence->getStatus() == Fence::Status::Unsignaled) {
            return false;
        }
    }
    return true;
}

void SurfaceFlinger::setTransactionState(const Vector<ComposerState>& states,
                                         const Vector<DisplayState>& displays, uint32_t flags,
                                         const sp<IBinder>& applyToken,
                                         const InputWindowCommands& inputWindowCommands,
                                         int64_t desiredPresentTime,
                                         const client_cache_t& uncacheBuffer,
                                         const std::vector<ListenerCallbacks>& listenerCallbacks) {
    ATRACE_CALL();

    const int64_t postTime = systemTime();

    bool privileged = callingThreadHasUnscopedSurfaceFlingerAccess();

    Mutex::Autolock _l(mStateLock);

    if (containsAnyInvalidClientState(states)) {
        return;
    }

    // If its TransactionQueue already has a pending TransactionState or if it is pending
    auto itr = mTransactionQueues.find(applyToken);
    // if this is an animation frame, wait until prior animation frame has
    // been applied by SF
    if (flags & eAnimation) {
        while (itr != mTransactionQueues.end()) {
            status_t err = mTransactionCV.waitRelative(mStateLock, s2ns(5));
            if (CC_UNLIKELY(err != NO_ERROR)) {
                ALOGW_IF(err == TIMED_OUT,
                         "setTransactionState timed out "
                         "waiting for animation frame to apply");
                break;
            }
            itr = mTransactionQueues.find(applyToken);
        }
    }
    if (itr != mTransactionQueues.end() ||
        !transactionIsReadyToBeApplied(desiredPresentTime, states)) {
        mTransactionQueues[applyToken].emplace(states, displays, flags, desiredPresentTime,
                                               uncacheBuffer, listenerCallbacks, postTime,
                                               privileged);
        setTransactionFlags(eTransactionFlushNeeded);
        return;
    }

    applyTransactionState(states, displays, flags, inputWindowCommands, desiredPresentTime,
                          uncacheBuffer, listenerCallbacks, postTime, privileged);
}

void SurfaceFlinger::applyTransactionState(const Vector<ComposerState>& states,
                                           const Vector<DisplayState>& displays, uint32_t flags,
                                           const InputWindowCommands& inputWindowCommands,
                                           const int64_t desiredPresentTime,
                                           const client_cache_t& uncacheBuffer,
                                           const std::vector<ListenerCallbacks>& listenerCallbacks,
                                           const int64_t postTime, bool privileged,
                                           bool isMainThread) {
    uint32_t transactionFlags = 0;

    if (flags & eAnimation) {
        // For window updates that are part of an animation we must wait for
        // previous animation "frames" to be handled.
        while (!isMainThread && mAnimTransactionPending) {
            status_t err = mTransactionCV.waitRelative(mStateLock, s2ns(5));
            if (CC_UNLIKELY(err != NO_ERROR)) {
                // just in case something goes wrong in SF, return to the
                // caller after a few seconds.
                ALOGW_IF(err == TIMED_OUT, "setTransactionState timed out "
                        "waiting for previous animation frame");
                mAnimTransactionPending = false;
                break;
            }
        }
    }

    for (const DisplayState& display : displays) {
        transactionFlags |= setDisplayStateLocked(display);
    }

    // In case the client has sent a Transaction that should receive callbacks but without any
    // SurfaceControls that should be included in the callback, send the listener and callbackIds
    // to the callback thread so it can send an empty callback
    if (!listenerCallbacks.empty()) {
        mTransactionCompletedThread.run();
    }
    for (const auto& [listener, callbackIds] : listenerCallbacks) {
        mTransactionCompletedThread.addCallback(listener, callbackIds);
    }

    uint32_t clientStateFlags = 0;
    for (const ComposerState& state : states) {
        clientStateFlags |= setClientStateLocked(state, desiredPresentTime, listenerCallbacks,
                                                 postTime, privileged);
    }

    // If the state doesn't require a traversal and there are callbacks, send them now
    if (!(clientStateFlags & eTraversalNeeded) && !listenerCallbacks.empty()) {
        mTransactionCompletedThread.sendCallbacks();
    }
    transactionFlags |= clientStateFlags;

    transactionFlags |= addInputWindowCommands(inputWindowCommands);

    if (uncacheBuffer.isValid()) {
        ClientCache::getInstance().erase(uncacheBuffer);
        getRenderEngine().unbindExternalTextureBuffer(uncacheBuffer.id);
    }

    // If a synchronous transaction is explicitly requested without any changes, force a transaction
    // anyway. This can be used as a flush mechanism for previous async transactions.
    // Empty animation transaction can be used to simulate back-pressure, so also force a
    // transaction for empty animation transactions.
    if (transactionFlags == 0 &&
            ((flags & eSynchronous) || (flags & eAnimation))) {
        transactionFlags = eTransactionNeeded;
    }

    // If we are on the main thread, we are about to preform a traversal. Clear the traversal bit
    // so we don't have to wake up again next frame to preform an uneeded traversal.
    if (isMainThread && (transactionFlags & eTraversalNeeded)) {
        transactionFlags = transactionFlags & (~eTraversalNeeded);
        mTraversalNeededMainThread = true;
    }

    if (transactionFlags) {
        if (mInterceptor->isEnabled()) {
            mInterceptor->saveTransaction(states, mCurrentState.displays, displays, flags);
        }

        // this triggers the transaction
        const auto start = (flags & eEarlyWakeup) ? Scheduler::TransactionStart::EARLY
                                                  : Scheduler::TransactionStart::NORMAL;
        setTransactionFlags(transactionFlags, start);

        // if this is a synchronous transaction, wait for it to take effect
        // before returning.
        if (flags & eSynchronous) {
            mTransactionPending = true;
        }
        if (flags & eAnimation) {
            mAnimTransactionPending = true;
        }
        if (mPendingInputWindowCommands.syncInputWindows) {
            mPendingSyncInputWindows = true;
        }

        // applyTransactionState can be called by either the main SF thread or by
        // another process through setTransactionState.  While a given process may wish
        // to wait on synchronous transactions, the main SF thread should never
        // be blocked.  Therefore, we only wait if isMainThread is false.
        while (!isMainThread && (mTransactionPending || mPendingSyncInputWindows)) {
            status_t err = mTransactionCV.waitRelative(mStateLock, s2ns(5));
            if (CC_UNLIKELY(err != NO_ERROR)) {
                // just in case something goes wrong in SF, return to the
                // called after a few seconds.
                ALOGW_IF(err == TIMED_OUT, "setTransactionState timed out!");
                mTransactionPending = false;
                mPendingSyncInputWindows = false;
                break;
            }
        }
    }
}

uint32_t SurfaceFlinger::setDisplayStateLocked(const DisplayState& s) {
    const ssize_t index = mCurrentState.displays.indexOfKey(s.token);
    if (index < 0) return 0;

    uint32_t flags = 0;
    DisplayDeviceState& state = mCurrentState.displays.editValueAt(index);

    const uint32_t what = s.what;
    if (what & DisplayState::eSurfaceChanged) {
        if (IInterface::asBinder(state.surface) != IInterface::asBinder(s.surface)) {
            state.surface = s.surface;
            flags |= eDisplayTransactionNeeded;
        }
    }
    if (what & DisplayState::eLayerStackChanged) {
        if (state.layerStack != s.layerStack) {
            state.layerStack = s.layerStack;
            flags |= eDisplayTransactionNeeded;
        }
    }
    if (what & DisplayState::eDisplayProjectionChanged) {
        if (state.orientation != s.orientation) {
            state.orientation = s.orientation;
            flags |= eDisplayTransactionNeeded;
        }
        if (state.frame != s.frame) {
            state.frame = s.frame;
            flags |= eDisplayTransactionNeeded;
        }
        if (state.viewport != s.viewport) {
            state.viewport = s.viewport;
            flags |= eDisplayTransactionNeeded;
        }
    }
    if (what & DisplayState::eDisplaySizeChanged) {
        if (state.width != s.width) {
            state.width = s.width;
            flags |= eDisplayTransactionNeeded;
        }
        if (state.height != s.height) {
            state.height = s.height;
            flags |= eDisplayTransactionNeeded;
        }
    }

    return flags;
}

bool SurfaceFlinger::callingThreadHasUnscopedSurfaceFlingerAccess() {
    IPCThreadState* ipc = IPCThreadState::self();
    const int pid = ipc->getCallingPid();
    const int uid = ipc->getCallingUid();
    if ((uid != AID_GRAPHICS && uid != AID_SYSTEM) &&
        !PermissionCache::checkPermission(sAccessSurfaceFlinger, pid, uid)) {
        return false;
    }
    return true;
}

uint32_t SurfaceFlinger::setClientStateLocked(
        const ComposerState& composerState, int64_t desiredPresentTime,
        const std::vector<ListenerCallbacks>& listenerCallbacks, int64_t postTime,
        bool privileged) {
    const layer_state_t& s = composerState.state;
    sp<Client> client(static_cast<Client*>(composerState.client.get()));

    sp<Layer> layer(client->getLayerUser(s.surface));
    if (layer == nullptr) {
        return 0;
    }

    uint32_t flags = 0;

    const uint64_t what = s.what;
    bool geometryAppliesWithResize =
            what & layer_state_t::eGeometryAppliesWithResize;

    // If we are deferring transaction, make sure to push the pending state, as otherwise the
    // pending state will also be deferred.
    if (what & layer_state_t::eDeferTransaction_legacy) {
        layer->pushPendingState();
    }

    if (what & layer_state_t::ePositionChanged) {
        if (layer->setPosition(s.x, s.y, !geometryAppliesWithResize)) {
            flags |= eTraversalNeeded;
        }
    }
    if (what & layer_state_t::eLayerChanged) {
        // NOTE: index needs to be calculated before we update the state
        const auto& p = layer->getParent();
        if (p == nullptr) {
            ssize_t idx = mCurrentState.layersSortedByZ.indexOf(layer);
            if (layer->setLayer(s.z) && idx >= 0) {
                mCurrentState.layersSortedByZ.removeAt(idx);
                mCurrentState.layersSortedByZ.add(layer);
                // we need traversal (state changed)
                // AND transaction (list changed)
                flags |= eTransactionNeeded|eTraversalNeeded;
            }
        } else {
            if (p->setChildLayer(layer, s.z)) {
                flags |= eTransactionNeeded|eTraversalNeeded;
            }
        }
    }
    if (what & layer_state_t::eRelativeLayerChanged) {
        // NOTE: index needs to be calculated before we update the state
        const auto& p = layer->getParent();
        if (p == nullptr) {
            ssize_t idx = mCurrentState.layersSortedByZ.indexOf(layer);
            if (layer->setRelativeLayer(s.relativeLayerHandle, s.z) && idx >= 0) {
                mCurrentState.layersSortedByZ.removeAt(idx);
                mCurrentState.layersSortedByZ.add(layer);
                // we need traversal (state changed)
                // AND transaction (list changed)
                flags |= eTransactionNeeded|eTraversalNeeded;
            }
        } else {
            if (p->setChildRelativeLayer(layer, s.relativeLayerHandle, s.z)) {
                flags |= eTransactionNeeded|eTraversalNeeded;
            }
        }
    }
    if (what & layer_state_t::eSizeChanged) {
        if (layer->setSize(s.w, s.h)) {
            flags |= eTraversalNeeded;
        }
    }
    if (what & layer_state_t::eAlphaChanged) {
        if (layer->setAlpha(s.alpha))
            flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eColorChanged) {
        if (layer->setColor(s.color))
            flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eColorTransformChanged) {
        if (layer->setColorTransform(s.colorTransform)) {
            flags |= eTraversalNeeded;
        }
    }
    if (what & layer_state_t::eBackgroundColorChanged) {
        if (layer->setBackgroundColor(s.color, s.bgColorAlpha, s.bgColorDataspace)) {
            flags |= eTraversalNeeded;
        }
    }
    if (what & layer_state_t::eMatrixChanged) {
        // TODO: b/109894387
        //
        // SurfaceFlinger's renderer is not prepared to handle cropping in the face of arbitrary
        // rotation. To see the problem observe that if we have a square parent, and a child
        // of the same size, then we rotate the child 45 degrees around it's center, the child
        // must now be cropped to a non rectangular 8 sided region.
        //
        // Of course we can fix this in the future. For now, we are lucky, SurfaceControl is
        // private API, and the WindowManager only uses rotation in one case, which is on a top
        // level layer in which cropping is not an issue.
        //
        // However given that abuse of rotation matrices could lead to surfaces extending outside
        // of cropped areas, we need to prevent non-root clients without permission ACCESS_SURFACE_FLINGER
        // (a.k.a. everyone except WindowManager and tests) from setting non rectangle preserving
        // transformations.
        if (layer->setMatrix(s.matrix, privileged))
            flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eTransparentRegionChanged) {
        if (layer->setTransparentRegionHint(s.transparentRegion))
            flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eFlagsChanged) {
        if (layer->setFlags(s.flags, s.mask))
            flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eCropChanged_legacy) {
        if (layer->setCrop_legacy(s.crop_legacy, !geometryAppliesWithResize))
            flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eCornerRadiusChanged) {
        if (layer->setCornerRadius(s.cornerRadius))
            flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eLayerStackChanged) {
        ssize_t idx = mCurrentState.layersSortedByZ.indexOf(layer);
        // We only allow setting layer stacks for top level layers,
        // everything else inherits layer stack from its parent.
        if (layer->hasParent()) {
            ALOGE("Attempt to set layer stack on layer with parent (%s) is invalid",
                    layer->getName().string());
        } else if (idx < 0) {
            ALOGE("Attempt to set layer stack on layer without parent (%s) that "
                    "that also does not appear in the top level layer list. Something"
                    " has gone wrong.", layer->getName().string());
        } else if (layer->setLayerStack(s.layerStack)) {
            mCurrentState.layersSortedByZ.removeAt(idx);
            mCurrentState.layersSortedByZ.add(layer);
            // we need traversal (state changed)
            // AND transaction (list changed)
            flags |= eTransactionNeeded|eTraversalNeeded|eDisplayLayerStackChanged;
        }
    }
    if (what & layer_state_t::eDeferTransaction_legacy) {
        if (s.barrierHandle_legacy != nullptr) {
            layer->deferTransactionUntil_legacy(s.barrierHandle_legacy, s.frameNumber_legacy);
        } else if (s.barrierGbp_legacy != nullptr) {
            const sp<IGraphicBufferProducer>& gbp = s.barrierGbp_legacy;
            if (authenticateSurfaceTextureLocked(gbp)) {
                const auto& otherLayer =
                    (static_cast<MonitoredProducer*>(gbp.get()))->getLayer();
                layer->deferTransactionUntil_legacy(otherLayer, s.frameNumber_legacy);
            } else {
                ALOGE("Attempt to defer transaction to to an"
                        " unrecognized GraphicBufferProducer");
            }
        }
        // We don't trigger a traversal here because if no other state is
        // changed, we don't want this to cause any more work
    }
    if (what & layer_state_t::eReparent) {
        bool hadParent = layer->hasParent();
        if (layer->reparent(s.parentHandleForChild)) {
            if (!hadParent) {
                mCurrentState.layersSortedByZ.remove(layer);
            }
            flags |= eTransactionNeeded|eTraversalNeeded;
        }
    }
    if (what & layer_state_t::eReparentChildren) {
        if (layer->reparentChildren(s.reparentHandle)) {
            flags |= eTransactionNeeded|eTraversalNeeded;
        }
    }
    if (what & layer_state_t::eDetachChildren) {
        layer->detachChildren();
    }
    if (what & layer_state_t::eOverrideScalingModeChanged) {
        layer->setOverrideScalingMode(s.overrideScalingMode);
        // We don't trigger a traversal here because if no other state is
        // changed, we don't want this to cause any more work
    }
    if (what & layer_state_t::eTransformChanged) {
        if (layer->setTransform(s.transform)) flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eTransformToDisplayInverseChanged) {
        if (layer->setTransformToDisplayInverse(s.transformToDisplayInverse))
            flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eCropChanged) {
        if (layer->setCrop(s.crop)) flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eFrameChanged) {
        if (layer->setFrame(s.frame)) flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eAcquireFenceChanged) {
        if (layer->setAcquireFence(s.acquireFence)) flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eDataspaceChanged) {
        if (layer->setDataspace(s.dataspace)) flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eHdrMetadataChanged) {
        if (layer->setHdrMetadata(s.hdrMetadata)) flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eSurfaceDamageRegionChanged) {
        if (layer->setSurfaceDamageRegion(s.surfaceDamageRegion)) flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eApiChanged) {
        if (layer->setApi(s.api)) flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eSidebandStreamChanged) {
        if (layer->setSidebandStream(s.sidebandStream)) flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eInputInfoChanged) {
        if (privileged) {
            layer->setInputInfo(s.inputInfo);
            flags |= eTraversalNeeded;
        } else {
            ALOGE("Attempt to update InputWindowInfo without permission ACCESS_SURFACE_FLINGER");
        }
    }
    if (what & layer_state_t::eMetadataChanged) {
        if (layer->setMetadata(s.metadata)) flags |= eTraversalNeeded;
    }
    if (what & layer_state_t::eColorSpaceAgnosticChanged) {
        if (layer->setColorSpaceAgnostic(s.colorSpaceAgnostic)) {
            flags |= eTraversalNeeded;
        }
    }
    std::vector<sp<CallbackHandle>> callbackHandles;
    if ((what & layer_state_t::eHasListenerCallbacksChanged) && (!listenerCallbacks.empty())) {
        for (const auto& [listener, callbackIds] : listenerCallbacks) {
            callbackHandles.emplace_back(new CallbackHandle(listener, callbackIds, s.surface));
        }
    }
    bool bufferChanged = what & layer_state_t::eBufferChanged;
    bool cacheIdChanged = what & layer_state_t::eCachedBufferChanged;
    sp<GraphicBuffer> buffer;
    if (bufferChanged && cacheIdChanged && s.buffer != nullptr) {
        buffer = s.buffer;
        bool success = ClientCache::getInstance().add(s.cachedBuffer, s.buffer);
        if (success) {
            getRenderEngine().cacheExternalTextureBuffer(s.buffer);
            success = ClientCache::getInstance()
                              .registerErasedRecipient(s.cachedBuffer,
                                                       wp<ClientCache::ErasedRecipient>(this));
            if (!success) {
                getRenderEngine().unbindExternalTextureBuffer(s.buffer->getId());
            }
        }
    } else if (cacheIdChanged) {
        buffer = ClientCache::getInstance().get(s.cachedBuffer);
    } else if (bufferChanged) {
        buffer = s.buffer;
    }
    if (buffer) {
        if (layer->setBuffer(buffer, postTime, desiredPresentTime, s.cachedBuffer)) {
            flags |= eTraversalNeeded;
        }
    }
    if (layer->setTransactionCompletedListeners(callbackHandles)) flags |= eTraversalNeeded;
    // Do not put anything that updates layer state or modifies flags after
    // setTransactionCompletedListener
    return flags;
}

uint32_t SurfaceFlinger::addInputWindowCommands(const InputWindowCommands& inputWindowCommands) {
    uint32_t flags = 0;
    if (!inputWindowCommands.transferTouchFocusCommands.empty()) {
        flags |= eTraversalNeeded;
    }

    if (inputWindowCommands.syncInputWindows) {
        flags |= eTraversalNeeded;
    }

    mPendingInputWindowCommands.merge(inputWindowCommands);
    return flags;
}

status_t SurfaceFlinger::createLayer(const String8& name, const sp<Client>& client, uint32_t w,
                                     uint32_t h, PixelFormat format, uint32_t flags,
                                     LayerMetadata metadata, sp<IBinder>* handle,
                                     sp<IGraphicBufferProducer>* gbp,
                                     const sp<IBinder>& parentHandle,
                                     const sp<Layer>& parentLayer) {
    if (int32_t(w|h) < 0) {
        ALOGE("createLayer() failed, w or h is negative (w=%d, h=%d)",
                int(w), int(h));
        return BAD_VALUE;
    }

    ALOG_ASSERT(parentLayer == nullptr || parentHandle == nullptr,
            "Expected only one of parentLayer or parentHandle to be non-null. "
            "Programmer error?");

    status_t result = NO_ERROR;

    sp<Layer> layer;

    String8 uniqueName = getUniqueLayerName(name);

    bool primaryDisplayOnly = false;

    // window type is WINDOW_TYPE_DONT_SCREENSHOT from SurfaceControl.java
    // TODO b/64227542
    if (metadata.has(METADATA_WINDOW_TYPE)) {
        int32_t windowType = metadata.getInt32(METADATA_WINDOW_TYPE, 0);
        if (windowType == 441731) {
            metadata.setInt32(METADATA_WINDOW_TYPE, InputWindowInfo::TYPE_NAVIGATION_BAR_PANEL);
            primaryDisplayOnly = true;
        }
    }

    switch (flags & ISurfaceComposerClient::eFXSurfaceMask) {
        case ISurfaceComposerClient::eFXSurfaceBufferQueue:
            result = createBufferQueueLayer(client, uniqueName, w, h, flags, std::move(metadata),
                                            format, handle, gbp, &layer);

            break;
        case ISurfaceComposerClient::eFXSurfaceBufferState:
            result = createBufferStateLayer(client, uniqueName, w, h, flags, std::move(metadata),
                                            handle, &layer);
            break;
        case ISurfaceComposerClient::eFXSurfaceColor:
            // check if buffer size is set for color layer.
            if (w > 0 || h > 0) {
                ALOGE("createLayer() failed, w or h cannot be set for color layer (w=%d, h=%d)",
                      int(w), int(h));
                return BAD_VALUE;
            }

            result = createColorLayer(client, uniqueName, w, h, flags, std::move(metadata), handle,
                                      &layer);
            break;
        case ISurfaceComposerClient::eFXSurfaceContainer:
            // check if buffer size is set for container layer.
            if (w > 0 || h > 0) {
                ALOGE("createLayer() failed, w or h cannot be set for container layer (w=%d, h=%d)",
                      int(w), int(h));
                return BAD_VALUE;
            }
            result = createContainerLayer(client, uniqueName, w, h, flags, std::move(metadata),
                                          handle, &layer);
            break;
        default:
            result = BAD_VALUE;
            break;
    }

    if (result != NO_ERROR) {
        return result;
    }

    if (primaryDisplayOnly) {
        layer->setPrimaryDisplayOnly();
    }

    bool addToCurrentState = callingThreadHasUnscopedSurfaceFlingerAccess();
    result = addClientLayer(client, *handle, *gbp, layer, parentHandle, parentLayer,
                            addToCurrentState);
    if (result != NO_ERROR) {
        return result;
    }
    mInterceptor->saveSurfaceCreation(layer);

    setTransactionFlags(eTransactionNeeded);
    return result;
}

String8 SurfaceFlinger::getUniqueLayerName(const String8& name)
{
    bool matchFound = true;
    uint32_t dupeCounter = 0;

    // Tack on our counter whether there is a hit or not, so everyone gets a tag
    String8 uniqueName = name + "#" + String8(std::to_string(dupeCounter).c_str());

    // Grab the state lock since we're accessing mCurrentState
    Mutex::Autolock lock(mStateLock);

    // Loop over layers until we're sure there is no matching name
    while (matchFound) {
        matchFound = false;
        mCurrentState.traverseInZOrder([&](Layer* layer) {
            if (layer->getName() == uniqueName) {
                matchFound = true;
                uniqueName = name + "#" + String8(std::to_string(++dupeCounter).c_str());
            }
        });
    }

    ALOGV_IF(dupeCounter > 0, "duplicate layer name: changing %s to %s", name.c_str(),
             uniqueName.c_str());

    return uniqueName;
}

status_t SurfaceFlinger::createBufferQueueLayer(const sp<Client>& client, const String8& name,
                                                uint32_t w, uint32_t h, uint32_t flags,
                                                LayerMetadata metadata, PixelFormat& format,
                                                sp<IBinder>* handle,
                                                sp<IGraphicBufferProducer>* gbp,
                                                sp<Layer>* outLayer) {
    // initialize the surfaces
    switch (format) {
    case PIXEL_FORMAT_TRANSPARENT:
    case PIXEL_FORMAT_TRANSLUCENT:
        format = PIXEL_FORMAT_RGBA_8888;
        break;
    case PIXEL_FORMAT_OPAQUE:
        format = PIXEL_FORMAT_RGBX_8888;
        break;
    }

    sp<BufferQueueLayer> layer = getFactory().createBufferQueueLayer(
            LayerCreationArgs(this, client, name, w, h, flags, std::move(metadata)));
    status_t err = layer->setDefaultBufferProperties(w, h, format);
    if (err == NO_ERROR) {
        *handle = layer->getHandle();
        *gbp = layer->getProducer();
        *outLayer = layer;
    }

    ALOGE_IF(err, "createBufferQueueLayer() failed (%s)", strerror(-err));
    return err;
}

status_t SurfaceFlinger::createBufferStateLayer(const sp<Client>& client, const String8& name,
                                                uint32_t w, uint32_t h, uint32_t flags,
                                                LayerMetadata metadata, sp<IBinder>* handle,
                                                sp<Layer>* outLayer) {
    sp<BufferStateLayer> layer = getFactory().createBufferStateLayer(
            LayerCreationArgs(this, client, name, w, h, flags, std::move(metadata)));
    *handle = layer->getHandle();
    *outLayer = layer;

    return NO_ERROR;
}

status_t SurfaceFlinger::createColorLayer(const sp<Client>& client, const String8& name, uint32_t w,
                                          uint32_t h, uint32_t flags, LayerMetadata metadata,
                                          sp<IBinder>* handle, sp<Layer>* outLayer) {
    *outLayer = getFactory().createColorLayer(
            LayerCreationArgs(this, client, name, w, h, flags, std::move(metadata)));
    *handle = (*outLayer)->getHandle();
    return NO_ERROR;
}

status_t SurfaceFlinger::createContainerLayer(const sp<Client>& client, const String8& name,
                                              uint32_t w, uint32_t h, uint32_t flags,
                                              LayerMetadata metadata, sp<IBinder>* handle,
                                              sp<Layer>* outLayer) {
    *outLayer = getFactory().createContainerLayer(
            LayerCreationArgs(this, client, name, w, h, flags, std::move(metadata)));
    *handle = (*outLayer)->getHandle();
    return NO_ERROR;
}


void SurfaceFlinger::markLayerPendingRemovalLocked(const sp<Layer>& layer) {
    mLayersPendingRemoval.add(layer);
    mLayersRemoved = true;
    setTransactionFlags(eTransactionNeeded);
}

void SurfaceFlinger::onHandleDestroyed(sp<Layer>& layer)
{
    Mutex::Autolock lock(mStateLock);
    // If a layer has a parent, we allow it to out-live it's handle
    // with the idea that the parent holds a reference and will eventually
    // be cleaned up. However no one cleans up the top-level so we do so
    // here.
    if (layer->getParent() == nullptr) {
        mCurrentState.layersSortedByZ.remove(layer);
    }
    markLayerPendingRemovalLocked(layer);

    auto it = mLayersByLocalBinderToken.begin();
    while (it != mLayersByLocalBinderToken.end()) {
        if (it->second == layer) {
            it = mLayersByLocalBinderToken.erase(it);
        } else {
            it++;
        }
    }

    layer.clear();
}

// ---------------------------------------------------------------------------

void SurfaceFlinger::onInitializeDisplays() {
    const auto display = getDefaultDisplayDeviceLocked();
    if (!display) return;

    const sp<IBinder> token = display->getDisplayToken().promote();
    LOG_ALWAYS_FATAL_IF(token == nullptr);

    // reset screen orientation and use primary layer stack
    Vector<ComposerState> state;
    Vector<DisplayState> displays;
    DisplayState d;
    d.what = DisplayState::eDisplayProjectionChanged |
             DisplayState::eLayerStackChanged;
    d.token = token;
    d.layerStack = 0;
    d.orientation = DisplayState::eOrientationDefault;
    d.frame.makeInvalid();
    d.viewport.makeInvalid();
    d.width = 0;
    d.height = 0;
    displays.add(d);
    setTransactionState(state, displays, 0, nullptr, mPendingInputWindowCommands, -1, {}, {});

    setPowerModeInternal(display, HWC_POWER_MODE_NORMAL);

    const nsecs_t vsyncPeriod = getVsyncPeriod();
    mAnimFrameTracker.setDisplayRefreshPeriod(vsyncPeriod);

    // Use phase of 0 since phase is not known.
    // Use latency of 0, which will snap to the ideal latency.
    DisplayStatInfo stats{0 /* vsyncTime */, vsyncPeriod};
    setCompositorTimingSnapped(stats, 0);
}

void SurfaceFlinger::initializeDisplays() {
    // Async since we may be called from the main thread.
    postMessageAsync(
            new LambdaMessage([this]() NO_THREAD_SAFETY_ANALYSIS { onInitializeDisplays(); }));
}

void SurfaceFlinger::setVsyncEnabledInHWC(DisplayId displayId, HWC2::Vsync enabled) {
    if (mHWCVsyncState != enabled) {
        getHwComposer().setVsyncEnabled(displayId, enabled);
        mHWCVsyncState = enabled;
    }
}

void SurfaceFlinger::setPowerModeInternal(const sp<DisplayDevice>& display, int mode) {
    if (display->isVirtual()) {
        ALOGE("%s: Invalid operation on virtual display", __FUNCTION__);
        return;
    }

    const auto displayId = display->getId();
    LOG_ALWAYS_FATAL_IF(!displayId);

    ALOGD("Setting power mode %d on display %s", mode, to_string(*displayId).c_str());

    int currentMode = display->getPowerMode();
    if (mode == currentMode) {
        return;
    }

    display->setPowerMode(mode);

    if (mInterceptor->isEnabled()) {
        mInterceptor->savePowerModeUpdate(display->getSequenceId(), mode);
    }

    if (currentMode == HWC_POWER_MODE_OFF) {
        // Turn on the display
        getHwComposer().setPowerMode(*displayId, mode);
        if (display->isPrimary() && mode != HWC_POWER_MODE_DOZE_SUSPEND) {
            setVsyncEnabledInHWC(*displayId, mHWCVsyncPendingState);
            mScheduler->onScreenAcquired(mAppConnectionHandle);
            mScheduler->resyncToHardwareVsync(true, getVsyncPeriod());
        }

        mVisibleRegionsDirty = true;
        mHasPoweredOff = true;
        repaintEverything();

        struct sched_param param = {0};
        param.sched_priority = 1;
        if (sched_setscheduler(0, SCHED_FIFO, &param) != 0) {
            ALOGW("Couldn't set SCHED_FIFO on display on");
        }
    } else if (mode == HWC_POWER_MODE_OFF) {
        // Turn off the display
        struct sched_param param = {0};
        if (sched_setscheduler(0, SCHED_OTHER, &param) != 0) {
            ALOGW("Couldn't set SCHED_OTHER on display off");
        }

        if (display->isPrimary() && currentMode != HWC_POWER_MODE_DOZE_SUSPEND) {
            mScheduler->disableHardwareVsync(true);
            mScheduler->onScreenReleased(mAppConnectionHandle);
        }

        // Make sure HWVsync is disabled before turning off the display
        setVsyncEnabledInHWC(*displayId, HWC2::Vsync::Disable);

        getHwComposer().setPowerMode(*displayId, mode);
        mVisibleRegionsDirty = true;
        // from this point on, SF will stop drawing on this display
    } else if (mode == HWC_POWER_MODE_DOZE ||
               mode == HWC_POWER_MODE_NORMAL) {
        // Update display while dozing
        getHwComposer().setPowerMode(*displayId, mode);
        if (display->isPrimary() && currentMode == HWC_POWER_MODE_DOZE_SUSPEND) {
            mScheduler->onScreenAcquired(mAppConnectionHandle);
            mScheduler->resyncToHardwareVsync(true, getVsyncPeriod());
        }
    } else if (mode == HWC_POWER_MODE_DOZE_SUSPEND) {
        // Leave display going to doze
        if (display->isPrimary()) {
            mScheduler->disableHardwareVsync(true);
            mScheduler->onScreenReleased(mAppConnectionHandle);
        }
        getHwComposer().setPowerMode(*displayId, mode);
    } else {
        ALOGE("Attempting to set unknown power mode: %d\n", mode);
        getHwComposer().setPowerMode(*displayId, mode);
    }

    if (display->isPrimary()) {
        mTimeStats->setPowerMode(mode);
        mRefreshRateStats->setPowerMode(mode);
        mScheduler->setDisplayPowerState(mode == HWC_POWER_MODE_NORMAL);
    }

    ALOGD("Finished setting power mode %d on display %s", mode, to_string(*displayId).c_str());
}

void SurfaceFlinger::setPowerMode(const sp<IBinder>& displayToken, int mode) {
    postMessageSync(new LambdaMessage([&]() NO_THREAD_SAFETY_ANALYSIS {
        const auto display = getDisplayDevice(displayToken);
        if (!display) {
            ALOGE("Attempt to set power mode %d for invalid display token %p", mode,
                  displayToken.get());
        } else if (display->isVirtual()) {
            ALOGW("Attempt to set power mode %d for virtual display", mode);
        } else {
            setPowerModeInternal(display, mode);
        }
    }));
}

// ---------------------------------------------------------------------------

status_t SurfaceFlinger::doDump(int fd, const DumpArgs& args,
                                bool asProto) NO_THREAD_SAFETY_ANALYSIS {
    std::string result;

    IPCThreadState* ipc = IPCThreadState::self();
    const int pid = ipc->getCallingPid();
    const int uid = ipc->getCallingUid();

    if ((uid != AID_SHELL) &&
            !PermissionCache::checkPermission(sDump, pid, uid)) {
        StringAppendF(&result, "Permission Denial: can't dump SurfaceFlinger from pid=%d, uid=%d\n",
                      pid, uid);
    } else {
        // Try to get the main lock, but give up after one second
        // (this would indicate SF is stuck, but we want to be able to
        // print something in dumpsys).
        status_t err = mStateLock.timedLock(s2ns(1));
        bool locked = (err == NO_ERROR);
        if (!locked) {
            StringAppendF(&result,
                          "SurfaceFlinger appears to be unresponsive (%s [%d]), dumping anyways "
                          "(no locks held)\n",
                          strerror(-err), err);
        }

        using namespace std::string_literals;

        static const std::unordered_map<std::string, Dumper> dumpers = {
                {"--clear-layer-stats"s, dumper([this](std::string&) { mLayerStats.clear(); })},
                {"--disable-layer-stats"s, dumper([this](std::string&) { mLayerStats.disable(); })},
                {"--display-id"s, dumper(&SurfaceFlinger::dumpDisplayIdentificationData)},
                {"--dispsync"s, dumper([this](std::string& s) {
                         mScheduler->dumpPrimaryDispSync(s);
                 })},
                {"--dump-layer-stats"s, dumper([this](std::string& s) { mLayerStats.dump(s); })},
                {"--enable-layer-stats"s, dumper([this](std::string&) { mLayerStats.enable(); })},
                {"--frame-events"s, dumper(&SurfaceFlinger::dumpFrameEventsLocked)},
                {"--latency"s, argsDumper(&SurfaceFlinger::dumpStatsLocked)},
                {"--latency-clear"s, argsDumper(&SurfaceFlinger::clearStatsLocked)},
                {"--list"s, dumper(&SurfaceFlinger::listLayersLocked)},
                {"--static-screen"s, dumper(&SurfaceFlinger::dumpStaticScreenStats)},
                {"--timestats"s, protoDumper(&SurfaceFlinger::dumpTimeStats)},
                {"--vsync"s, dumper(&SurfaceFlinger::dumpVSync)},
                {"--wide-color"s, dumper(&SurfaceFlinger::dumpWideColorInfo)},
        };

        const auto flag = args.empty() ? ""s : std::string(String8(args[0]));

        if (const auto it = dumpers.find(flag); it != dumpers.end()) {
            (it->second)(args, asProto, result);
        } else {
            if (asProto) {
                LayersProto layersProto = dumpProtoInfo(LayerVector::StateSet::Current);
                result.append(layersProto.SerializeAsString().c_str(), layersProto.ByteSize());
            } else {
                dumpAllLocked(args, result);
            }
        }

        if (locked) {
            mStateLock.unlock();
        }
    }
    write(fd, result.c_str(), result.size());
    return NO_ERROR;
}

status_t SurfaceFlinger::dumpCritical(int fd, const DumpArgs&, bool asProto) {
    if (asProto && mTracing.isEnabled()) {
        mTracing.writeToFileAsync();
    }

    return doDump(fd, DumpArgs(), asProto);
}

void SurfaceFlinger::listLayersLocked(std::string& result) const {
    mCurrentState.traverseInZOrder(
            [&](Layer* layer) { StringAppendF(&result, "%s\n", layer->getName().string()); });
}

void SurfaceFlinger::dumpStatsLocked(const DumpArgs& args, std::string& result) const {
    StringAppendF(&result, "%" PRId64 "\n", getVsyncPeriod());

    if (args.size() > 1) {
        const auto name = String8(args[1]);
        mCurrentState.traverseInZOrder([&](Layer* layer) {
            if (name == layer->getName()) {
                layer->dumpFrameStats(result);
            }
        });
    } else {
        mAnimFrameTracker.dumpStats(result);
    }
}

void SurfaceFlinger::clearStatsLocked(const DumpArgs& args, std::string&) {
    mCurrentState.traverseInZOrder([&](Layer* layer) {
        if (args.size() < 2 || String8(args[1]) == layer->getName()) {
            layer->clearFrameStats();
        }
    });

    mAnimFrameTracker.clearStats();
}

void SurfaceFlinger::dumpTimeStats(const DumpArgs& args, bool asProto, std::string& result) const {
    mTimeStats->parseArgs(asProto, args, result);
}

// This should only be called from the main thread.  Otherwise it would need
// the lock and should use mCurrentState rather than mDrawingState.
void SurfaceFlinger::logFrameStats() {
    mDrawingState.traverseInZOrder([&](Layer* layer) {
        layer->logFrameStats();
    });

    mAnimFrameTracker.logAndResetStats(String8("<win-anim>"));
}

void SurfaceFlinger::appendSfConfigString(std::string& result) const {
    result.append(" [sf");

    if (isLayerTripleBufferingDisabled())
        result.append(" DISABLE_TRIPLE_BUFFERING");

    StringAppendF(&result, " PRESENT_TIME_OFFSET=%" PRId64, dispSyncPresentTimeOffset);
    StringAppendF(&result, " FORCE_HWC_FOR_RBG_TO_YUV=%d", useHwcForRgbToYuv);
    StringAppendF(&result, " MAX_VIRT_DISPLAY_DIM=%" PRIu64, maxVirtualDisplaySize);
    StringAppendF(&result, " RUNNING_WITHOUT_SYNC_FRAMEWORK=%d", !hasSyncFramework);
    StringAppendF(&result, " NUM_FRAMEBUFFER_SURFACE_BUFFERS=%" PRId64,
                  maxFrameBufferAcquiredBuffers);
    result.append("]");
}

void SurfaceFlinger::dumpVSync(std::string& result) const {
    mPhaseOffsets->dump(result);
    StringAppendF(&result,
                  "    present offset: %9" PRId64 " ns\t     VSYNC period: %9" PRId64 " ns\n\n",
                  dispSyncPresentTimeOffset, getVsyncPeriod());

    StringAppendF(&result, "Scheduler enabled.");
    StringAppendF(&result, "+  Smart 90 for video detection: %s\n\n",
                  mUseSmart90ForVideo ? "on" : "off");
    StringAppendF(&result, "Allowed Display Configs: ");
    for (int32_t configId : mAllowedDisplayConfigs) {
        StringAppendF(&result, "%" PRIu32 " Hz, ",
                      mRefreshRateConfigs->getRefreshRateFromConfigId(configId).fps);
    }
    StringAppendF(&result, "(config override by backdoor: %s)\n\n",
                  mDebugDisplayConfigSetByBackdoor ? "yes" : "no");
    mScheduler->dump(mAppConnectionHandle, result);
    StringAppendF(&result, "+  Refresh rate switching: %s\n",
                  mRefreshRateConfigs->refreshRateSwitchingSupported() ? "on" : "off");
}

void SurfaceFlinger::dumpStaticScreenStats(std::string& result) const {
    result.append("Static screen stats:\n");
    for (size_t b = 0; b < SurfaceFlingerBE::NUM_BUCKETS - 1; ++b) {
        float bucketTimeSec = getBE().mFrameBuckets[b] / 1e9;
        float percent = 100.0f *
                static_cast<float>(getBE().mFrameBuckets[b]) / getBE().mTotalTime;
        StringAppendF(&result, "  < %zd frames: %.3f s (%.1f%%)\n", b + 1, bucketTimeSec, percent);
    }
    float bucketTimeSec = getBE().mFrameBuckets[SurfaceFlingerBE::NUM_BUCKETS - 1] / 1e9;
    float percent = 100.0f *
            static_cast<float>(getBE().mFrameBuckets[SurfaceFlingerBE::NUM_BUCKETS - 1]) / getBE().mTotalTime;
    StringAppendF(&result, "  %zd+ frames: %.3f s (%.1f%%)\n", SurfaceFlingerBE::NUM_BUCKETS - 1,
                  bucketTimeSec, percent);
}

void SurfaceFlinger::recordBufferingStats(const char* layerName,
        std::vector<OccupancyTracker::Segment>&& history) {
    Mutex::Autolock lock(getBE().mBufferingStatsMutex);
    auto& stats = getBE().mBufferingStats[layerName];
    for (const auto& segment : history) {
        if (!segment.usedThirdBuffer) {
            stats.twoBufferTime += segment.totalTime;
        }
        if (segment.occupancyAverage < 1.0f) {
            stats.doubleBufferedTime += segment.totalTime;
        } else if (segment.occupancyAverage < 2.0f) {
            stats.tripleBufferedTime += segment.totalTime;
        }
        ++stats.numSegments;
        stats.totalTime += segment.totalTime;
    }
}

void SurfaceFlinger::dumpFrameEventsLocked(std::string& result) {
    result.append("Layer frame timestamps:\n");

    const LayerVector& currentLayers = mCurrentState.layersSortedByZ;
    const size_t count = currentLayers.size();
    for (size_t i=0 ; i<count ; i++) {
        currentLayers[i]->dumpFrameEvents(result);
    }
}

void SurfaceFlinger::dumpBufferingStats(std::string& result) const {
    result.append("Buffering stats:\n");
    result.append("  [Layer name] <Active time> <Two buffer> "
            "<Double buffered> <Triple buffered>\n");
    Mutex::Autolock lock(getBE().mBufferingStatsMutex);
    typedef std::tuple<std::string, float, float, float> BufferTuple;
    std::map<float, BufferTuple, std::greater<float>> sorted;
    for (const auto& statsPair : getBE().mBufferingStats) {
        const char* name = statsPair.first.c_str();
        const SurfaceFlingerBE::BufferingStats& stats = statsPair.second;
        if (stats.numSegments == 0) {
            continue;
        }
        float activeTime = ns2ms(stats.totalTime) / 1000.0f;
        float twoBufferRatio = static_cast<float>(stats.twoBufferTime) /
                stats.totalTime;
        float doubleBufferRatio = static_cast<float>(
                stats.doubleBufferedTime) / stats.totalTime;
        float tripleBufferRatio = static_cast<float>(
                stats.tripleBufferedTime) / stats.totalTime;
        sorted.insert({activeTime, {name, twoBufferRatio,
                doubleBufferRatio, tripleBufferRatio}});
    }
    for (const auto& sortedPair : sorted) {
        float activeTime = sortedPair.first;
        const BufferTuple& values = sortedPair.second;
        StringAppendF(&result, "  [%s] %.2f %.3f %.3f %.3f\n", std::get<0>(values).c_str(),
                      activeTime, std::get<1>(values), std::get<2>(values), std::get<3>(values));
    }
    result.append("\n");
}

void SurfaceFlinger::dumpDisplayIdentificationData(std::string& result) const {
    for (const auto& [token, display] : mDisplays) {
        const auto displayId = display->getId();
        if (!displayId) {
            continue;
        }
        const auto hwcDisplayId = getHwComposer().fromPhysicalDisplayId(*displayId);
        if (!hwcDisplayId) {
            continue;
        }

        StringAppendF(&result,
                      "Display %s (HWC display %" PRIu64 "): ", to_string(*displayId).c_str(),
                      *hwcDisplayId);
        uint8_t port;
        DisplayIdentificationData data;
        if (!getHwComposer().getDisplayIdentificationData(*hwcDisplayId, &port, &data)) {
            result.append("no identification data\n");
            continue;
        }

        if (!isEdid(data)) {
            result.append("unknown identification data: ");
            for (uint8_t byte : data) {
                StringAppendF(&result, "%x ", byte);
            }
            result.append("\n");
            continue;
        }

        const auto edid = parseEdid(data);
        if (!edid) {
            result.append("invalid EDID: ");
            for (uint8_t byte : data) {
                StringAppendF(&result, "%x ", byte);
            }
            result.append("\n");
            continue;
        }

        StringAppendF(&result, "port=%u pnpId=%s displayName=\"", port, edid->pnpId.data());
        result.append(edid->displayName.data(), edid->displayName.length());
        result.append("\"\n");
    }
}

void SurfaceFlinger::dumpWideColorInfo(std::string& result) const {
    StringAppendF(&result, "Device has wide color built-in display: %d\n", hasWideColorDisplay);
    StringAppendF(&result, "Device uses color management: %d\n", useColorManagement);
    StringAppendF(&result, "DisplayColorSetting: %s\n",
                  decodeDisplayColorSetting(mDisplayColorSetting).c_str());

    // TODO: print out if wide-color mode is active or not

    for (const auto& [token, display] : mDisplays) {
        const auto displayId = display->getId();
        if (!displayId) {
            continue;
        }

        StringAppendF(&result, "Display %s color modes:\n", to_string(*displayId).c_str());
        std::vector<ColorMode> modes = getHwComposer().getColorModes(*displayId);
        for (auto&& mode : modes) {
            StringAppendF(&result, "    %s (%d)\n", decodeColorMode(mode).c_str(), mode);
        }

        ColorMode currentMode = display->getCompositionDisplay()->getState().colorMode;
        StringAppendF(&result, "    Current color mode: %s (%d)\n",
                      decodeColorMode(currentMode).c_str(), currentMode);
    }
    result.append("\n");
}

LayersProto SurfaceFlinger::dumpProtoInfo(LayerVector::StateSet stateSet,
                                          uint32_t traceFlags) const {
    LayersProto layersProto;
    const bool useDrawing = stateSet == LayerVector::StateSet::Drawing;
    const State& state = useDrawing ? mDrawingState : mCurrentState;
    state.traverseInZOrder([&](Layer* layer) {
        LayerProto* layerProto = layersProto.add_layers();
        layer->writeToProto(layerProto, stateSet, traceFlags);
    });

    return layersProto;
}

LayersProto SurfaceFlinger::dumpVisibleLayersProtoInfo(
        const sp<DisplayDevice>& displayDevice) const {
    LayersProto layersProto;

    SizeProto* resolution = layersProto.mutable_resolution();
    resolution->set_w(displayDevice->getWidth());
    resolution->set_h(displayDevice->getHeight());

    auto display = displayDevice->getCompositionDisplay();
    const auto& displayState = display->getState();

    layersProto.set_color_mode(decodeColorMode(displayState.colorMode));
    layersProto.set_color_transform(decodeColorTransform(displayState.colorTransform));
    layersProto.set_global_transform(displayState.orientation);

    const auto displayId = displayDevice->getId();
    LOG_ALWAYS_FATAL_IF(!displayId);
    mDrawingState.traverseInZOrder([&](Layer* layer) {
        if (!layer->visibleRegion.isEmpty() && !display->getOutputLayersOrderedByZ().empty()) {
            LayerProto* layerProto = layersProto.add_layers();
            layer->writeToProto(layerProto, displayDevice);
        }
    });

    return layersProto;
}

void SurfaceFlinger::dumpAllLocked(const DumpArgs& args, std::string& result) const {
    const bool colorize = !args.empty() && args[0] == String16("--color");
    Colorizer colorizer(colorize);

    // figure out if we're stuck somewhere
    const nsecs_t now = systemTime();
    const nsecs_t inTransaction(mDebugInTransaction);
    nsecs_t inTransactionDuration = (inTransaction) ? now-inTransaction : 0;

    /*
     * Dump library configuration.
     */

    colorizer.bold(result);
    result.append("Build configuration:");
    colorizer.reset(result);
    appendSfConfigString(result);
    appendUiConfigString(result);
    appendGuiConfigString(result);
    result.append("\n");

    result.append("\nDisplay identification data:\n");
    dumpDisplayIdentificationData(result);

    result.append("\nWide-Color information:\n");
    dumpWideColorInfo(result);

    colorizer.bold(result);
    result.append("Sync configuration: ");
    colorizer.reset(result);
    result.append(SyncFeatures::getInstance().toString());
    result.append("\n\n");

    colorizer.bold(result);
    result.append("VSYNC configuration:\n");
    colorizer.reset(result);
    dumpVSync(result);
    result.append("\n");

    dumpStaticScreenStats(result);
    result.append("\n");

    StringAppendF(&result, "Total missed frame count: %u\n", mFrameMissedCount.load());
    StringAppendF(&result, "HWC missed frame count: %u\n", mHwcFrameMissedCount.load());
    StringAppendF(&result, "GPU missed frame count: %u\n\n", mGpuFrameMissedCount.load());

    dumpBufferingStats(result);

    /*
     * Dump the visible layer list
     */
    colorizer.bold(result);
    StringAppendF(&result, "Visible layers (count = %zu)\n", mNumLayers);
    StringAppendF(&result, "GraphicBufferProducers: %zu, max %zu\n",
                  mGraphicBufferProducerList.size(), mMaxGraphicBufferProducerListSize);
    colorizer.reset(result);

    {
        LayersProto layersProto = dumpProtoInfo(LayerVector::StateSet::Current);
        auto layerTree = LayerProtoParser::generateLayerTree(layersProto);
        result.append(LayerProtoParser::layerTreeToString(layerTree));
        result.append("\n");
    }

    {
        StringAppendF(&result, "Composition layers\n");
        mDrawingState.traverseInZOrder([&](Layer* layer) {
            auto compositionLayer = layer->getCompositionLayer();
            if (compositionLayer) compositionLayer->dump(result);
        });
    }

    /*
     * Dump Display state
     */

    colorizer.bold(result);
    StringAppendF(&result, "Displays (%zu entries)\n", mDisplays.size());
    colorizer.reset(result);
    for (const auto& [token, display] : mDisplays) {
        display->dump(result);
    }
    result.append("\n");

    /*
     * Dump SurfaceFlinger global state
     */

    colorizer.bold(result);
    result.append("SurfaceFlinger global state:\n");
    colorizer.reset(result);

    getRenderEngine().dump(result);

    DebugEGLImageTracker::getInstance()->dump(result);

    if (const auto display = getDefaultDisplayDeviceLocked()) {
        display->getCompositionDisplay()->getState().undefinedRegion.dump(result,
                                                                          "undefinedRegion");
        StringAppendF(&result, "  orientation=%d, isPoweredOn=%d\n", display->getOrientation(),
                      display->isPoweredOn());
    }
    StringAppendF(&result,
                  "  transaction-flags         : %08x\n"
                  "  gpu_to_cpu_unsupported    : %d\n",
                  mTransactionFlags.load(), !mGpuToCpuSupported);

    if (const auto displayId = getInternalDisplayIdLocked();
        displayId && getHwComposer().isConnected(*displayId)) {
        const auto activeConfig = getHwComposer().getActiveConfig(*displayId);
        StringAppendF(&result,
                      "  refresh-rate              : %f fps\n"
                      "  x-dpi                     : %f\n"
                      "  y-dpi                     : %f\n",
                      1e9 / activeConfig->getVsyncPeriod(), activeConfig->getDpiX(),
                      activeConfig->getDpiY());
    }

    StringAppendF(&result, "  transaction time: %f us\n", inTransactionDuration / 1000.0);

    /*
     * Tracing state
     */
    mTracing.dump(result);
    result.append("\n");

    /*
     * HWC layer minidump
     */
    for (const auto& [token, display] : mDisplays) {
        const auto displayId = display->getId();
        if (!displayId) {
            continue;
        }

        StringAppendF(&result, "Display %s HWC layers:\n", to_string(*displayId).c_str());
        Layer::miniDumpHeader(result);
        const sp<DisplayDevice> displayDevice = display;
        mCurrentState.traverseInZOrder(
                [&](Layer* layer) { layer->miniDump(result, displayDevice); });
        result.append("\n");
    }

    /*
     * Dump HWComposer state
     */
    colorizer.bold(result);
    result.append("h/w composer state:\n");
    colorizer.reset(result);
    bool hwcDisabled = mDebugDisableHWC || mDebugRegion;
    StringAppendF(&result, "  h/w composer %s\n", hwcDisabled ? "disabled" : "enabled");
    getHwComposer().dump(result);

    /*
     * Dump gralloc state
     */
    const GraphicBufferAllocator& alloc(GraphicBufferAllocator::get());
    alloc.dump(result);

    /*
     * Dump VrFlinger state if in use.
     */
    if (mVrFlingerRequestsDisplay && mVrFlinger) {
        result.append("VrFlinger state:\n");
        result.append(mVrFlinger->Dump());
        result.append("\n");
    }

    /**
     * Scheduler dump state.
     */
    result.append("\nScheduler state:\n");
    result.append(mScheduler->doDump() + "\n");
    StringAppendF(&result, "+  Smart video mode: %s\n\n", mUseSmart90ForVideo ? "on" : "off");
    result.append(mRefreshRateStats->doDump() + "\n");

    result.append(mTimeStats->miniDump());
    result.append("\n");
}

const Vector<sp<Layer>>& SurfaceFlinger::getLayerSortedByZForHwcDisplay(DisplayId displayId) {
    // Note: mStateLock is held here
    for (const auto& [token, display] : mDisplays) {
        if (display->getId() == displayId) {
            return getDisplayDeviceLocked(token)->getVisibleLayersSortedByZ();
        }
    }

    ALOGE("%s: Invalid display %s", __FUNCTION__, to_string(displayId).c_str());
    static const Vector<sp<Layer>> empty;
    return empty;
}

void SurfaceFlinger::updateColorMatrixLocked() {
    mat4 colorMatrix;
    if (mGlobalSaturationFactor != 1.0f) {
        // Rec.709 luma coefficients
        float3 luminance{0.213f, 0.715f, 0.072f};
        luminance *= 1.0f - mGlobalSaturationFactor;
        mat4 saturationMatrix = mat4(
            vec4{luminance.r + mGlobalSaturationFactor, luminance.r, luminance.r, 0.0f},
            vec4{luminance.g, luminance.g + mGlobalSaturationFactor, luminance.g, 0.0f},
            vec4{luminance.b, luminance.b, luminance.b + mGlobalSaturationFactor, 0.0f},
            vec4{0.0f, 0.0f, 0.0f, 1.0f}
        );
        colorMatrix = mClientColorMatrix * saturationMatrix * mDaltonizer();
    } else {
        colorMatrix = mClientColorMatrix * mDaltonizer();
    }

    if (mCurrentState.colorMatrix != colorMatrix) {
        mCurrentState.colorMatrix = colorMatrix;
        mCurrentState.colorMatrixChanged = true;
        setTransactionFlags(eTransactionNeeded);
    }
}

status_t SurfaceFlinger::CheckTransactCodeCredentials(uint32_t code) {
#pragma clang diagnostic push
#pragma clang diagnostic error "-Wswitch-enum"
    switch (static_cast<ISurfaceComposerTag>(code)) {
        // These methods should at minimum make sure that the client requested
        // access to SF.
        case BOOT_FINISHED:
        case CLEAR_ANIMATION_FRAME_STATS:
        case CREATE_DISPLAY:
        case DESTROY_DISPLAY:
        case ENABLE_VSYNC_INJECTIONS:
        case GET_ANIMATION_FRAME_STATS:
        case GET_HDR_CAPABILITIES:
        case SET_ACTIVE_CONFIG:
        case SET_ALLOWED_DISPLAY_CONFIGS:
        case GET_ALLOWED_DISPLAY_CONFIGS:
        case SET_ACTIVE_COLOR_MODE:
        case INJECT_VSYNC:
        case SET_POWER_MODE:
        case GET_DISPLAYED_CONTENT_SAMPLING_ATTRIBUTES:
        case SET_DISPLAY_CONTENT_SAMPLING_ENABLED:
        case GET_DISPLAYED_CONTENT_SAMPLE:
        case NOTIFY_POWER_HINT: {
            if (!callingThreadHasUnscopedSurfaceFlingerAccess()) {
                IPCThreadState* ipc = IPCThreadState::self();
                ALOGE("Permission Denial: can't access SurfaceFlinger pid=%d, uid=%d",
                        ipc->getCallingPid(), ipc->getCallingUid());
                return PERMISSION_DENIED;
            }
            return OK;
        }
        case GET_LAYER_DEBUG_INFO: {
            IPCThreadState* ipc = IPCThreadState::self();
            const int pid = ipc->getCallingPid();
            const int uid = ipc->getCallingUid();
            if ((uid != AID_SHELL) && !PermissionCache::checkPermission(sDump, pid, uid)) {
                ALOGE("Layer debug info permission denied for pid=%d, uid=%d", pid, uid);
                return PERMISSION_DENIED;
            }
            return OK;
        }
        // Used by apps to hook Choreographer to SurfaceFlinger.
        case CREATE_DISPLAY_EVENT_CONNECTION:
        // The following calls are currently used by clients that do not
        // request necessary permissions. However, they do not expose any secret
        // information, so it is OK to pass them.
        case AUTHENTICATE_SURFACE:
        case GET_ACTIVE_COLOR_MODE:
        case GET_ACTIVE_CONFIG:
        case GET_PHYSICAL_DISPLAY_IDS:
        case GET_PHYSICAL_DISPLAY_TOKEN:
        case GET_DISPLAY_COLOR_MODES:
        case GET_DISPLAY_NATIVE_PRIMARIES:
        case GET_DISPLAY_CONFIGS:
        case GET_DISPLAY_STATS:
        case GET_SUPPORTED_FRAME_TIMESTAMPS:
        // Calling setTransactionState is safe, because you need to have been
        // granted a reference to Client* and Handle* to do anything with it.
        case SET_TRANSACTION_STATE:
        case CREATE_CONNECTION:
        case GET_COLOR_MANAGEMENT:
        case GET_COMPOSITION_PREFERENCE:
        case GET_PROTECTED_CONTENT_SUPPORT:
        case IS_WIDE_COLOR_DISPLAY:
        case GET_DISPLAY_BRIGHTNESS_SUPPORT:
        case SET_DISPLAY_BRIGHTNESS: {
            return OK;
        }
        case CAPTURE_LAYERS:
        case CAPTURE_SCREEN:
        case ADD_REGION_SAMPLING_LISTENER:
        case REMOVE_REGION_SAMPLING_LISTENER: {
            // codes that require permission check
            IPCThreadState* ipc = IPCThreadState::self();
            const int pid = ipc->getCallingPid();
            const int uid = ipc->getCallingUid();
            if ((uid != AID_GRAPHICS) &&
                !PermissionCache::checkPermission(sReadFramebuffer, pid, uid)) {
                ALOGE("Permission Denial: can't read framebuffer pid=%d, uid=%d", pid, uid);
                return PERMISSION_DENIED;
            }
            return OK;
        }
        // The following codes are deprecated and should never be allowed to access SF.
        case CONNECT_DISPLAY_UNUSED:
        case CREATE_GRAPHIC_BUFFER_ALLOC_UNUSED: {
            ALOGE("Attempting to access SurfaceFlinger with unused code: %u", code);
            return PERMISSION_DENIED;
        }
        case CAPTURE_SCREEN_BY_ID: {
            IPCThreadState* ipc = IPCThreadState::self();
            const int uid = ipc->getCallingUid();
            if (uid == AID_ROOT || uid == AID_GRAPHICS || uid == AID_SYSTEM || uid == AID_SHELL) {
                return OK;
            }
            return PERMISSION_DENIED;
        }
    }

    // These codes are used for the IBinder protocol to either interrogate the recipient
    // side of the transaction for its canonical interface descriptor or to dump its state.
    // We let them pass by default.
    if (code == IBinder::INTERFACE_TRANSACTION || code == IBinder::DUMP_TRANSACTION ||
        code == IBinder::PING_TRANSACTION || code == IBinder::SHELL_COMMAND_TRANSACTION ||
        code == IBinder::SYSPROPS_TRANSACTION) {
        return OK;
    }
    // Numbers from 1000 to 1034 are currently used for backdoors. The code
    // in onTransact verifies that the user is root, and has access to use SF.
    if (code >= 1000 && code <= 1035) {
        ALOGV("Accessing SurfaceFlinger through backdoor code: %u", code);
        return OK;
    }
    ALOGE("Permission Denial: SurfaceFlinger did not recognize request code: %u", code);
    return PERMISSION_DENIED;
#pragma clang diagnostic pop
}

status_t SurfaceFlinger::onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                    uint32_t flags) {
    status_t credentialCheck = CheckTransactCodeCredentials(code);
    if (credentialCheck != OK) {
        return credentialCheck;
    }

    status_t err = BnSurfaceComposer::onTransact(code, data, reply, flags);
    if (err == UNKNOWN_TRANSACTION || err == PERMISSION_DENIED) {
        CHECK_INTERFACE(ISurfaceComposer, data, reply);
        IPCThreadState* ipc = IPCThreadState::self();
        const int uid = ipc->getCallingUid();
        if (CC_UNLIKELY(uid != AID_SYSTEM
                && !PermissionCache::checkCallingPermission(sHardwareTest))) {
            const int pid = ipc->getCallingPid();
            ALOGE("Permission Denial: "
                    "can't access SurfaceFlinger pid=%d, uid=%d", pid, uid);
            return PERMISSION_DENIED;
        }
        int n;
        switch (code) {
            case 1000: // SHOW_CPU, NOT SUPPORTED ANYMORE
            case 1001: // SHOW_FPS, NOT SUPPORTED ANYMORE
                return NO_ERROR;
            case 1002:  // SHOW_UPDATES
                n = data.readInt32();
                mDebugRegion = n ? n : (mDebugRegion ? 0 : 1);
                invalidateHwcGeometry();
                repaintEverything();
                return NO_ERROR;
            case 1004:{ // repaint everything
                repaintEverything();
                return NO_ERROR;
            }
            case 1005:{ // force transaction
                Mutex::Autolock _l(mStateLock);
                setTransactionFlags(
                        eTransactionNeeded|
                        eDisplayTransactionNeeded|
                        eTraversalNeeded);
                return NO_ERROR;
            }
            case 1006:{ // send empty update
                signalRefresh();
                return NO_ERROR;
            }
            case 1008:  // toggle use of hw composer
                n = data.readInt32();
                mDebugDisableHWC = n != 0;
                invalidateHwcGeometry();
                repaintEverything();
                return NO_ERROR;
            case 1009:  // toggle use of transform hint
                n = data.readInt32();
                mDebugDisableTransformHint = n != 0;
                invalidateHwcGeometry();
                repaintEverything();
                return NO_ERROR;
            case 1010:  // interrogate.
                reply->writeInt32(0);
                reply->writeInt32(0);
                reply->writeInt32(mDebugRegion);
                reply->writeInt32(0);
                reply->writeInt32(mDebugDisableHWC);
                return NO_ERROR;
            case 1013: {
                const auto display = getDefaultDisplayDevice();
                if (!display) {
                    return NAME_NOT_FOUND;
                }

                reply->writeInt32(display->getPageFlipCount());
                return NO_ERROR;
            }
            case 1014: {
                Mutex::Autolock _l(mStateLock);
                // daltonize
                n = data.readInt32();
                switch (n % 10) {
                    case 1:
                        mDaltonizer.setType(ColorBlindnessType::Protanomaly);
                        break;
                    case 2:
                        mDaltonizer.setType(ColorBlindnessType::Deuteranomaly);
                        break;
                    case 3:
                        mDaltonizer.setType(ColorBlindnessType::Tritanomaly);
                        break;
                    default:
                        mDaltonizer.setType(ColorBlindnessType::None);
                        break;
                }
                if (n >= 10) {
                    mDaltonizer.setMode(ColorBlindnessMode::Correction);
                } else {
                    mDaltonizer.setMode(ColorBlindnessMode::Simulation);
                }

                updateColorMatrixLocked();
                return NO_ERROR;
            }
            case 1015: {
                Mutex::Autolock _l(mStateLock);
                // apply a color matrix
                n = data.readInt32();
                if (n) {
                    // color matrix is sent as a column-major mat4 matrix
                    for (size_t i = 0 ; i < 4; i++) {
                        for (size_t j = 0; j < 4; j++) {
                            mClientColorMatrix[i][j] = data.readFloat();
                        }
                    }
                } else {
                    mClientColorMatrix = mat4();
                }

                // Check that supplied matrix's last row is {0,0,0,1} so we can avoid
                // the division by w in the fragment shader
                float4 lastRow(transpose(mClientColorMatrix)[3]);
                if (any(greaterThan(abs(lastRow - float4{0, 0, 0, 1}), float4{1e-4f}))) {
                    ALOGE("The color transform's last row must be (0, 0, 0, 1)");
                }

                updateColorMatrixLocked();
                return NO_ERROR;
            }
            // This is an experimental interface
            // Needs to be shifted to proper binder interface when we productize
            case 1016: {
                n = data.readInt32();
                // TODO(b/113612090): Evaluate if this can be removed.
                mScheduler->setRefreshSkipCount(n);
                return NO_ERROR;
            }
            case 1017: {
                n = data.readInt32();
                mForceFullDamage = n != 0;
                return NO_ERROR;
            }
            case 1018: { // Modify Choreographer's phase offset
                n = data.readInt32();
                mScheduler->setPhaseOffset(mAppConnectionHandle, static_cast<nsecs_t>(n));
                return NO_ERROR;
            }
            case 1019: { // Modify SurfaceFlinger's phase offset
                n = data.readInt32();
                mScheduler->setPhaseOffset(mSfConnectionHandle, static_cast<nsecs_t>(n));
                return NO_ERROR;
            }
            case 1020: { // Layer updates interceptor
                n = data.readInt32();
                if (n) {
                    ALOGV("Interceptor enabled");
                    mInterceptor->enable(mDrawingState.layersSortedByZ, mDrawingState.displays);
                }
                else{
                    ALOGV("Interceptor disabled");
                    mInterceptor->disable();
                }
                return NO_ERROR;
            }
            case 1021: { // Disable HWC virtual displays
                n = data.readInt32();
                mUseHwcVirtualDisplays = !n;
                return NO_ERROR;
            }
            case 1022: { // Set saturation boost
                Mutex::Autolock _l(mStateLock);
                mGlobalSaturationFactor = std::max(0.0f, std::min(data.readFloat(), 2.0f));

                updateColorMatrixLocked();
                return NO_ERROR;
            }
            case 1023: { // Set native mode
                int32_t colorMode;

                mDisplayColorSetting = static_cast<DisplayColorSetting>(data.readInt32());
                if (data.readInt32(&colorMode) == NO_ERROR) {
                    mForceColorMode = static_cast<ColorMode>(colorMode);
                }
                invalidateHwcGeometry();
                repaintEverything();
                return NO_ERROR;
            }
            // Deprecate, use 1030 to check whether the device is color managed.
            case 1024: {
                return NAME_NOT_FOUND;
            }
            case 1025: { // Set layer tracing
                n = data.readInt32();
                if (n) {
                    ALOGD("LayerTracing enabled");
                    Mutex::Autolock lock(mStateLock);
                    mTracingEnabledChanged = true;
                    mTracing.enable();
                    reply->writeInt32(NO_ERROR);
                } else {
                    ALOGD("LayerTracing disabled");
                    bool writeFile = false;
                    {
                        Mutex::Autolock lock(mStateLock);
                        mTracingEnabledChanged = true;
                        writeFile = mTracing.disable();
                    }

                    if (writeFile) {
                        reply->writeInt32(mTracing.writeToFile());
                    } else {
                        reply->writeInt32(NO_ERROR);
                    }
                }
                return NO_ERROR;
            }
            case 1026: { // Get layer tracing status
                reply->writeBool(mTracing.isEnabled());
                return NO_ERROR;
            }
            // Is a DisplayColorSetting supported?
            case 1027: {
                const auto display = getDefaultDisplayDevice();
                if (!display) {
                    return NAME_NOT_FOUND;
                }

                DisplayColorSetting setting = static_cast<DisplayColorSetting>(data.readInt32());
                switch (setting) {
                    case DisplayColorSetting::MANAGED:
                        reply->writeBool(useColorManagement);
                        break;
                    case DisplayColorSetting::UNMANAGED:
                        reply->writeBool(true);
                        break;
                    case DisplayColorSetting::ENHANCED:
                        reply->writeBool(display->hasRenderIntent(RenderIntent::ENHANCE));
                        break;
                    default: // vendor display color setting
                        reply->writeBool(
                                display->hasRenderIntent(static_cast<RenderIntent>(setting)));
                        break;
                }
                return NO_ERROR;
            }
            // Is VrFlinger active?
            case 1028: {
                Mutex::Autolock _l(mStateLock);
                reply->writeBool(getHwComposer().isUsingVrComposer());
                return NO_ERROR;
            }
            // Set buffer size for SF tracing (value in KB)
            case 1029: {
                n = data.readInt32();
                if (n <= 0 || n > MAX_TRACING_MEMORY) {
                    ALOGW("Invalid buffer size: %d KB", n);
                    reply->writeInt32(BAD_VALUE);
                    return BAD_VALUE;
                }

                ALOGD("Updating trace buffer to %d KB", n);
                mTracing.setBufferSize(n * 1024);
                reply->writeInt32(NO_ERROR);
                return NO_ERROR;
            }
            // Is device color managed?
            case 1030: {
                reply->writeBool(useColorManagement);
                return NO_ERROR;
            }
            // Override default composition data space
            // adb shell service call SurfaceFlinger 1031 i32 1 DATASPACE_NUMBER DATASPACE_NUMBER \
            // && adb shell stop zygote && adb shell start zygote
            // to restore: adb shell service call SurfaceFlinger 1031 i32 0 && \
            // adb shell stop zygote && adb shell start zygote
            case 1031: {
                Mutex::Autolock _l(mStateLock);
                n = data.readInt32();
                if (n) {
                    n = data.readInt32();
                    if (n) {
                        Dataspace dataspace = static_cast<Dataspace>(n);
                        if (!validateCompositionDataspace(dataspace)) {
                            return BAD_VALUE;
                        }
                        mDefaultCompositionDataspace = dataspace;
                    }
                    n = data.readInt32();
                    if (n) {
                        Dataspace dataspace = static_cast<Dataspace>(n);
                        if (!validateCompositionDataspace(dataspace)) {
                            return BAD_VALUE;
                        }
                        mWideColorGamutCompositionDataspace = dataspace;
                    }
                } else {
                    // restore composition data space.
                    mDefaultCompositionDataspace = defaultCompositionDataspace;
                    mWideColorGamutCompositionDataspace = wideColorGamutCompositionDataspace;
                }
                return NO_ERROR;
            }
            // Set trace flags
            case 1033: {
                n = data.readUint32();
                ALOGD("Updating trace flags to 0x%x", n);
                mTracing.setTraceFlags(n);
                reply->writeInt32(NO_ERROR);
                return NO_ERROR;
            }
            case 1034: {
                // TODO(b/129297325): expose this via developer menu option
                n = data.readInt32();
                if (n && !mRefreshRateOverlay &&
                    mRefreshRateConfigs->refreshRateSwitchingSupported()) {
                    RefreshRateType type;
                    {
                        std::lock_guard<std::mutex> lock(mActiveConfigLock);
                        type = mDesiredActiveConfig.type;
                    }
                    mRefreshRateOverlay = std::make_unique<RefreshRateOverlay>(*this);
                    mRefreshRateOverlay->changeRefreshRate(type);
                } else if (!n) {
                    mRefreshRateOverlay.reset();
                }
                return NO_ERROR;
            }
            case 1035: {
                n = data.readInt32();
                mDebugDisplayConfigSetByBackdoor = false;
                if (n >= 0) {
                    const auto displayToken = getInternalDisplayToken();
                    status_t result = setAllowedDisplayConfigs(displayToken, {n});
                    if (result != NO_ERROR) {
                        return result;
                    }
                    mDebugDisplayConfigSetByBackdoor = true;
                }
                return NO_ERROR;
            }
        }
    }
    return err;
}

void SurfaceFlinger::repaintEverything() {
    mRepaintEverything = true;
    signalTransaction();
}

void SurfaceFlinger::repaintEverythingForHWC() {
    mRepaintEverything = true;
    mEventQueue->invalidate();
}

// A simple RAII class to disconnect from an ANativeWindow* when it goes out of scope
class WindowDisconnector {
public:
    WindowDisconnector(ANativeWindow* window, int api) : mWindow(window), mApi(api) {}
    ~WindowDisconnector() {
        native_window_api_disconnect(mWindow, mApi);
    }

private:
    ANativeWindow* mWindow;
    const int mApi;
};

status_t SurfaceFlinger::captureScreen(const sp<IBinder>& displayToken,
                                       sp<GraphicBuffer>* outBuffer, bool& outCapturedSecureLayers,
                                       const Dataspace reqDataspace,
                                       const ui::PixelFormat reqPixelFormat, Rect sourceCrop,
                                       uint32_t reqWidth, uint32_t reqHeight,
                                       bool useIdentityTransform,
                                       ISurfaceComposer::Rotation rotation,
                                       bool captureSecureLayers) {
    ATRACE_CALL();

    if (!displayToken) return BAD_VALUE;

    auto renderAreaRotation = fromSurfaceComposerRotation(rotation);

    sp<DisplayDevice> display;
    {
        Mutex::Autolock _l(mStateLock);

        display = getDisplayDeviceLocked(displayToken);
        if (!display) return BAD_VALUE;

        // set the requested width/height to the logical display viewport size
        // by default
        if (reqWidth == 0 || reqHeight == 0) {
            reqWidth = uint32_t(display->getViewport().width());
            reqHeight = uint32_t(display->getViewport().height());
        }
    }

    DisplayRenderArea renderArea(display, sourceCrop, reqWidth, reqHeight, reqDataspace,
                                 renderAreaRotation, captureSecureLayers);

    auto traverseLayers = std::bind(&SurfaceFlinger::traverseLayersInDisplay, this, display,
                                    std::placeholders::_1);
    return captureScreenCommon(renderArea, traverseLayers, outBuffer, reqPixelFormat,
                               useIdentityTransform, outCapturedSecureLayers);
}

static Dataspace pickDataspaceFromColorMode(const ColorMode colorMode) {
    switch (colorMode) {
        case ColorMode::DISPLAY_P3:
        case ColorMode::BT2100_PQ:
        case ColorMode::BT2100_HLG:
        case ColorMode::DISPLAY_BT2020:
            return Dataspace::DISPLAY_P3;
        default:
            return Dataspace::V0_SRGB;
    }
}

const sp<DisplayDevice> SurfaceFlinger::getDisplayByIdOrLayerStack(uint64_t displayOrLayerStack) {
    const sp<IBinder> displayToken = getPhysicalDisplayTokenLocked(DisplayId{displayOrLayerStack});
    if (displayToken) {
        return getDisplayDeviceLocked(displayToken);
    }
    // Couldn't find display by displayId. Try to get display by layerStack since virtual displays
    // may not have a displayId.
    for (const auto& [token, display] : mDisplays) {
        if (display->getLayerStack() == displayOrLayerStack) {
            return display;
        }
    }
    return nullptr;
}

status_t SurfaceFlinger::captureScreen(uint64_t displayOrLayerStack, Dataspace* outDataspace,
                                       sp<GraphicBuffer>* outBuffer) {
    sp<DisplayDevice> display;
    uint32_t width;
    uint32_t height;
    ui::Transform::orientation_flags captureOrientation;
    {
        Mutex::Autolock _l(mStateLock);
        display = getDisplayByIdOrLayerStack(displayOrLayerStack);
        if (!display) {
            return BAD_VALUE;
        }

        width = uint32_t(display->getViewport().width());
        height = uint32_t(display->getViewport().height());

        captureOrientation = fromSurfaceComposerRotation(
                static_cast<ISurfaceComposer::Rotation>(display->getOrientation()));
        if (captureOrientation == ui::Transform::orientation_flags::ROT_90) {
            captureOrientation = ui::Transform::orientation_flags::ROT_270;
        } else if (captureOrientation == ui::Transform::orientation_flags::ROT_270) {
            captureOrientation = ui::Transform::orientation_flags::ROT_90;
        }
        *outDataspace =
                pickDataspaceFromColorMode(display->getCompositionDisplay()->getState().colorMode);
    }

    DisplayRenderArea renderArea(display, Rect(), width, height, *outDataspace, captureOrientation,
                                 false /* captureSecureLayers */);

    auto traverseLayers = std::bind(&SurfaceFlinger::traverseLayersInDisplay, this, display,
                                    std::placeholders::_1);
    bool ignored = false;
    return captureScreenCommon(renderArea, traverseLayers, outBuffer, ui::PixelFormat::RGBA_8888,
                               false /* useIdentityTransform */,
                               ignored /* outCapturedSecureLayers */);
}

status_t SurfaceFlinger::captureLayers(
        const sp<IBinder>& layerHandleBinder, sp<GraphicBuffer>* outBuffer,
        const Dataspace reqDataspace, const ui::PixelFormat reqPixelFormat, const Rect& sourceCrop,
        const std::unordered_set<sp<IBinder>, ISurfaceComposer::SpHash<IBinder>>& excludeHandles,
        float frameScale, bool childrenOnly) {
    ATRACE_CALL();

    class LayerRenderArea : public RenderArea {
    public:
        LayerRenderArea(SurfaceFlinger* flinger, const sp<Layer>& layer, const Rect crop,
                        int32_t reqWidth, int32_t reqHeight, Dataspace reqDataSpace,
                        bool childrenOnly)
              : RenderArea(reqWidth, reqHeight, CaptureFill::CLEAR, reqDataSpace),
                mLayer(layer),
                mCrop(crop),
                mNeedsFiltering(false),
                mFlinger(flinger),
                mChildrenOnly(childrenOnly) {}
        const ui::Transform& getTransform() const override { return mTransform; }
        Rect getBounds() const override {
            const Layer::State& layerState(mLayer->getDrawingState());
            return mLayer->getBufferSize(layerState);
        }
        int getHeight() const override {
            return mLayer->getBufferSize(mLayer->getDrawingState()).getHeight();
        }
        int getWidth() const override {
            return mLayer->getBufferSize(mLayer->getDrawingState()).getWidth();
        }
        bool isSecure() const override { return false; }
        bool needsFiltering() const override { return mNeedsFiltering; }
        const sp<const DisplayDevice> getDisplayDevice() const override { return nullptr; }
        Rect getSourceCrop() const override {
            if (mCrop.isEmpty()) {
                return getBounds();
            } else {
                return mCrop;
            }
        }
        class ReparentForDrawing {
        public:
            const sp<Layer>& oldParent;
            const sp<Layer>& newParent;

            ReparentForDrawing(const sp<Layer>& oldParent, const sp<Layer>& newParent,
                               const Rect& drawingBounds)
                  : oldParent(oldParent), newParent(newParent) {
                // Compute and cache the bounds for the new parent layer.
                newParent->computeBounds(drawingBounds.toFloatRect(), ui::Transform());
                oldParent->setChildrenDrawingParent(newParent);
            }
            ~ReparentForDrawing() { oldParent->setChildrenDrawingParent(oldParent); }
        };

        void render(std::function<void()> drawLayers) override {
            const Rect sourceCrop = getSourceCrop();
            // no need to check rotation because there is none
            mNeedsFiltering = sourceCrop.width() != getReqWidth() ||
                sourceCrop.height() != getReqHeight();

            if (!mChildrenOnly) {
                mTransform = mLayer->getTransform().inverse();
                drawLayers();
            } else {
                Rect bounds = getBounds();
                screenshotParentLayer = mFlinger->getFactory().createContainerLayer(
                        LayerCreationArgs(mFlinger, nullptr, String8("Screenshot Parent"),
                                          bounds.getWidth(), bounds.getHeight(), 0,
                                          LayerMetadata()));

                ReparentForDrawing reparent(mLayer, screenshotParentLayer, sourceCrop);
                drawLayers();
            }
        }

    private:
        const sp<Layer> mLayer;
        const Rect mCrop;

        // In the "childrenOnly" case we reparent the children to a screenshot
        // layer which has no properties set and which does not draw.
        sp<ContainerLayer> screenshotParentLayer;
        ui::Transform mTransform;
        bool mNeedsFiltering;

        SurfaceFlinger* mFlinger;
        const bool mChildrenOnly;
    };

    int reqWidth = 0;
    int reqHeight = 0;
    sp<Layer> parent;
    Rect crop(sourceCrop);
    std::unordered_set<sp<Layer>, ISurfaceComposer::SpHash<Layer>> excludeLayers;

    {
        Mutex::Autolock _l(mStateLock);

        parent = fromHandle(layerHandleBinder);
        if (parent == nullptr || parent->isRemovedFromCurrentState()) {
            ALOGE("captureLayers called with an invalid or removed parent");
            return NAME_NOT_FOUND;
        }

        const int uid = IPCThreadState::self()->getCallingUid();
        const bool forSystem = uid == AID_GRAPHICS || uid == AID_SYSTEM;
        if (!forSystem && parent->getCurrentState().flags & layer_state_t::eLayerSecure) {
            ALOGW("Attempting to capture secure layer: PERMISSION_DENIED");
            return PERMISSION_DENIED;
        }

        if (sourceCrop.width() <= 0) {
            crop.left = 0;
            crop.right = parent->getBufferSize(parent->getCurrentState()).getWidth();
        }

        if (sourceCrop.height() <= 0) {
            crop.top = 0;
            crop.bottom = parent->getBufferSize(parent->getCurrentState()).getHeight();
        }
        reqWidth = crop.width() * frameScale;
        reqHeight = crop.height() * frameScale;

        for (const auto& handle : excludeHandles) {
            sp<Layer> excludeLayer = fromHandle(handle);
            if (excludeLayer != nullptr) {
                excludeLayers.emplace(excludeLayer);
            } else {
                ALOGW("Invalid layer handle passed as excludeLayer to captureLayers");
                return NAME_NOT_FOUND;
            }
        }
    } // mStateLock

    // really small crop or frameScale
    if (reqWidth <= 0) {
        reqWidth = 1;
    }
    if (reqHeight <= 0) {
        reqHeight = 1;
    }

    LayerRenderArea renderArea(this, parent, crop, reqWidth, reqHeight, reqDataspace, childrenOnly);
    auto traverseLayers = [parent, childrenOnly,
                           &excludeLayers](const LayerVector::Visitor& visitor) {
        parent->traverseChildrenInZOrder(LayerVector::StateSet::Drawing, [&](Layer* layer) {
            if (!layer->isVisible()) {
                return;
            } else if (childrenOnly && layer == parent.get()) {
                return;
            }

            sp<Layer> p = layer;
            while (p != nullptr) {
                if (excludeLayers.count(p) != 0) {
                    return;
                }
                p = p->getParent();
            }

            visitor(layer);
        });
    };

    bool outCapturedSecureLayers = false;
    return captureScreenCommon(renderArea, traverseLayers, outBuffer, reqPixelFormat, false,
                               outCapturedSecureLayers);
}

status_t SurfaceFlinger::captureScreenCommon(RenderArea& renderArea,
                                             TraverseLayersFunction traverseLayers,
                                             sp<GraphicBuffer>* outBuffer,
                                             const ui::PixelFormat reqPixelFormat,
                                             bool useIdentityTransform,
                                             bool& outCapturedSecureLayers) {
    ATRACE_CALL();

    // TODO(b/116112787) Make buffer usage a parameter.
    const uint32_t usage = GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
            GRALLOC_USAGE_HW_RENDER | GRALLOC_USAGE_HW_TEXTURE;
    *outBuffer =
            getFactory().createGraphicBuffer(renderArea.getReqWidth(), renderArea.getReqHeight(),
                                             static_cast<android_pixel_format>(reqPixelFormat), 1,
                                             usage, "screenshot");

    return captureScreenCommon(renderArea, traverseLayers, *outBuffer, useIdentityTransform,
                               outCapturedSecureLayers);
}

status_t SurfaceFlinger::captureScreenCommon(RenderArea& renderArea,
                                             TraverseLayersFunction traverseLayers,
                                             const sp<GraphicBuffer>& buffer,
                                             bool useIdentityTransform,
                                             bool& outCapturedSecureLayers) {
    // This mutex protects syncFd and captureResult for communication of the return values from the
    // main thread back to this Binder thread
    std::mutex captureMutex;
    std::condition_variable captureCondition;
    std::unique_lock<std::mutex> captureLock(captureMutex);
    int syncFd = -1;
    std::optional<status_t> captureResult;

    const int uid = IPCThreadState::self()->getCallingUid();
    const bool forSystem = uid == AID_GRAPHICS || uid == AID_SYSTEM;

    sp<LambdaMessage> message = new LambdaMessage([&] {
        // If there is a refresh pending, bug out early and tell the binder thread to try again
        // after the refresh.
        if (mRefreshPending) {
            ATRACE_NAME("Skipping screenshot for now");
            std::unique_lock<std::mutex> captureLock(captureMutex);
            captureResult = std::make_optional<status_t>(EAGAIN);
            captureCondition.notify_one();
            return;
        }

        status_t result = NO_ERROR;
        int fd = -1;
        {
            Mutex::Autolock _l(mStateLock);
            renderArea.render([&] {
                result = captureScreenImplLocked(renderArea, traverseLayers, buffer.get(),
                                                 useIdentityTransform, forSystem, &fd,
                                                 outCapturedSecureLayers);
            });
        }

        {
            std::unique_lock<std::mutex> captureLock(captureMutex);
            syncFd = fd;
            captureResult = std::make_optional<status_t>(result);
            captureCondition.notify_one();
        }
    });

    status_t result = postMessageAsync(message);
    if (result == NO_ERROR) {
        captureCondition.wait(captureLock, [&] { return captureResult; });
        while (*captureResult == EAGAIN) {
            captureResult.reset();
            result = postMessageAsync(message);
            if (result != NO_ERROR) {
                return result;
            }
            captureCondition.wait(captureLock, [&] { return captureResult; });
        }
        result = *captureResult;
    }

    if (result == NO_ERROR) {
        sync_wait(syncFd, -1);
        close(syncFd);
    }

    return result;
}

void SurfaceFlinger::renderScreenImplLocked(const RenderArea& renderArea,
                                            TraverseLayersFunction traverseLayers,
                                            ANativeWindowBuffer* buffer, bool useIdentityTransform,
                                            int* outSyncFd) {
    ATRACE_CALL();

    const auto reqWidth = renderArea.getReqWidth();
    const auto reqHeight = renderArea.getReqHeight();
    const auto rotation = renderArea.getRotationFlags();
    const auto transform = renderArea.getTransform();
    const auto sourceCrop = renderArea.getSourceCrop();

    renderengine::DisplaySettings clientCompositionDisplay;
    std::vector<renderengine::LayerSettings> clientCompositionLayers;

    // assume that bounds are never offset, and that they are the same as the
    // buffer bounds.
    clientCompositionDisplay.physicalDisplay = Rect(reqWidth, reqHeight);
    clientCompositionDisplay.clip = sourceCrop;
    clientCompositionDisplay.globalTransform = transform.asMatrix4();

    // Now take into account the rotation flag. We append a transform that
    // rotates the layer stack about the origin, then translate by buffer
    // boundaries to be in the right quadrant.
    mat4 rotMatrix;
    int displacementX = 0;
    int displacementY = 0;
    float rot90InRadians = 2.0f * static_cast<float>(M_PI) / 4.0f;
    switch (rotation) {
        case ui::Transform::ROT_90:
            rotMatrix = mat4::rotate(rot90InRadians, vec3(0, 0, 1));
            displacementX = renderArea.getBounds().getHeight();
            break;
        case ui::Transform::ROT_180:
            rotMatrix = mat4::rotate(rot90InRadians * 2.0f, vec3(0, 0, 1));
            displacementY = renderArea.getBounds().getWidth();
            displacementX = renderArea.getBounds().getHeight();
            break;
        case ui::Transform::ROT_270:
            rotMatrix = mat4::rotate(rot90InRadians * 3.0f, vec3(0, 0, 1));
            displacementY = renderArea.getBounds().getWidth();
            break;
        default:
            break;
    }

    // We need to transform the clipping window into the right spot.
    // First, rotate the clipping rectangle by the rotation hint to get the
    // right orientation
    const vec4 clipTL = vec4(sourceCrop.left, sourceCrop.top, 0, 1);
    const vec4 clipBR = vec4(sourceCrop.right, sourceCrop.bottom, 0, 1);
    const vec4 rotClipTL = rotMatrix * clipTL;
    const vec4 rotClipBR = rotMatrix * clipBR;
    const int newClipLeft = std::min(rotClipTL[0], rotClipBR[0]);
    const int newClipTop = std::min(rotClipTL[1], rotClipBR[1]);
    const int newClipRight = std::max(rotClipTL[0], rotClipBR[0]);
    const int newClipBottom = std::max(rotClipTL[1], rotClipBR[1]);

    // Now reposition the clipping rectangle with the displacement vector
    // computed above.
    const mat4 displacementMat = mat4::translate(vec4(displacementX, displacementY, 0, 1));
    clientCompositionDisplay.clip =
            Rect(newClipLeft + displacementX, newClipTop + displacementY,
                 newClipRight + displacementX, newClipBottom + displacementY);

    mat4 clipTransform = displacementMat * rotMatrix;
    clientCompositionDisplay.globalTransform =
            clipTransform * clientCompositionDisplay.globalTransform;

    clientCompositionDisplay.outputDataspace = renderArea.getReqDataSpace();
    clientCompositionDisplay.maxLuminance = DisplayDevice::sDefaultMaxLumiance;

    const float alpha = RenderArea::getCaptureFillValue(renderArea.getCaptureFill());

    renderengine::LayerSettings fillLayer;
    fillLayer.source.buffer.buffer = nullptr;
    fillLayer.source.solidColor = half3(0.0, 0.0, 0.0);
    fillLayer.geometry.boundaries = FloatRect(0.0, 0.0, 1.0, 1.0);
    fillLayer.alpha = half(alpha);
    clientCompositionLayers.push_back(fillLayer);

    Region clearRegion = Region::INVALID_REGION;
    traverseLayers([&](Layer* layer) {
        renderengine::LayerSettings layerSettings;
        bool prepared = layer->prepareClientLayer(renderArea, useIdentityTransform, clearRegion,
                                                  false, layerSettings);
        if (prepared) {
            clientCompositionLayers.push_back(layerSettings);
        }
    });

    clientCompositionDisplay.clearRegion = clearRegion;
    // Use an empty fence for the buffer fence, since we just created the buffer so
    // there is no need for synchronization with the GPU.
    base::unique_fd bufferFence;
    base::unique_fd drawFence;
    getRenderEngine().useProtectedContext(false);
    getRenderEngine().drawLayers(clientCompositionDisplay, clientCompositionLayers, buffer,
                                 /*useFramebufferCache=*/false, std::move(bufferFence), &drawFence);

    *outSyncFd = drawFence.release();
}

status_t SurfaceFlinger::captureScreenImplLocked(const RenderArea& renderArea,
                                                 TraverseLayersFunction traverseLayers,
                                                 ANativeWindowBuffer* buffer,
                                                 bool useIdentityTransform, bool forSystem,
                                                 int* outSyncFd, bool& outCapturedSecureLayers) {
    ATRACE_CALL();

    traverseLayers([&](Layer* layer) {
        outCapturedSecureLayers =
                outCapturedSecureLayers || (layer->isVisible() && layer->isSecure());
    });

    // We allow the system server to take screenshots of secure layers for
    // use in situations like the Screen-rotation animation and place
    // the impetus on WindowManager to not persist them.
    if (outCapturedSecureLayers && !forSystem) {
        ALOGW("FB is protected: PERMISSION_DENIED");
        return PERMISSION_DENIED;
    }
    renderScreenImplLocked(renderArea, traverseLayers, buffer, useIdentityTransform, outSyncFd);
    return NO_ERROR;
}

void SurfaceFlinger::setInputWindowsFinished() {
    Mutex::Autolock _l(mStateLock);

    mPendingSyncInputWindows = false;
    mTransactionCV.broadcast();
}

// ---------------------------------------------------------------------------

void SurfaceFlinger::State::traverseInZOrder(const LayerVector::Visitor& visitor) const {
    layersSortedByZ.traverseInZOrder(stateSet, visitor);
}

void SurfaceFlinger::State::traverseInReverseZOrder(const LayerVector::Visitor& visitor) const {
    layersSortedByZ.traverseInReverseZOrder(stateSet, visitor);
}

void SurfaceFlinger::traverseLayersInDisplay(const sp<const DisplayDevice>& display,
                                             const LayerVector::Visitor& visitor) {
    // We loop through the first level of layers without traversing,
    // as we need to determine which layers belong to the requested display.
    for (const auto& layer : mDrawingState.layersSortedByZ) {
        if (!layer->belongsToDisplay(display->getLayerStack(), false)) {
            continue;
        }
        // relative layers are traversed in Layer::traverseInZOrder
        layer->traverseInZOrder(LayerVector::StateSet::Drawing, [&](Layer* layer) {
            if (!layer->belongsToDisplay(display->getLayerStack(), false)) {
                return;
            }
            if (!layer->isVisible()) {
                return;
            }
            visitor(layer);
        });
    }
}

void SurfaceFlinger::setAllowedDisplayConfigsInternal(const sp<DisplayDevice>& display,
                                                      const std::vector<int32_t>& allowedConfigs) {
    if (!display->isPrimary()) {
        return;
    }

    const auto allowedDisplayConfigs = DisplayConfigs(allowedConfigs.begin(),
                                                      allowedConfigs.end());
    if (allowedDisplayConfigs == mAllowedDisplayConfigs) {
        return;
    }

    ALOGV("Updating allowed configs");
    mAllowedDisplayConfigs = std::move(allowedDisplayConfigs);

    // TODO(b/140204874): This hack triggers a notification that something has changed, so
    // that listeners that care about a change in allowed configs can get the notification.
    // Giving current ActiveConfig so that most other listeners would just drop the event
    mScheduler->onConfigChanged(mAppConnectionHandle, display->getId()->value,
                                display->getActiveConfig());

    if (mRefreshRateConfigs->refreshRateSwitchingSupported()) {
        const auto& type = mScheduler->getPreferredRefreshRateType();
        const auto& config = mRefreshRateConfigs->getRefreshRateFromType(type);
        if (isDisplayConfigAllowed(config.configId)) {
            ALOGV("switching to Scheduler preferred config %d", config.configId);
            setDesiredActiveConfig({type, config.configId, Scheduler::ConfigEvent::Changed});
        } else {
            // Set the highest allowed config by iterating backwards on available refresh rates
            const auto& refreshRates = mRefreshRateConfigs->getRefreshRateMap();
            for (auto iter = refreshRates.crbegin(); iter != refreshRates.crend(); ++iter) {
                if (isDisplayConfigAllowed(iter->second.configId)) {
                    ALOGV("switching to allowed config %d", iter->second.configId);
                    setDesiredActiveConfig(
                            {iter->first, iter->second.configId, Scheduler::ConfigEvent::Changed});
                    break;
                }
            }
        }
    } else if (!isDisplayConfigAllowed(display->getActiveConfig())) {
        ALOGV("switching to config %d", allowedConfigs[0]);
        setDesiredActiveConfig(
                {RefreshRateType::DEFAULT, allowedConfigs[0], Scheduler::ConfigEvent::Changed});
    }
}

status_t SurfaceFlinger::setAllowedDisplayConfigs(const sp<IBinder>& displayToken,
                                                  const std::vector<int32_t>& allowedConfigs) {
    ATRACE_CALL();

    if (!displayToken || allowedConfigs.empty()) {
        return BAD_VALUE;
    }

    if (mDebugDisplayConfigSetByBackdoor) {
        // ignore this request as config is overridden by backdoor
        return NO_ERROR;
    }

    postMessageSync(new LambdaMessage([&]() {
        const auto display = getDisplayDeviceLocked(displayToken);
        if (!display) {
            ALOGE("Attempt to set allowed display configs for invalid display token %p",
                  displayToken.get());
        } else if (display->isVirtual()) {
            ALOGW("Attempt to set allowed display configs for virtual display");
        } else {
            Mutex::Autolock lock(mStateLock);
            setAllowedDisplayConfigsInternal(display, allowedConfigs);
        }
    }));

    return NO_ERROR;
}

status_t SurfaceFlinger::getAllowedDisplayConfigs(const sp<IBinder>& displayToken,
                                                  std::vector<int32_t>* outAllowedConfigs) {
    ATRACE_CALL();

    if (!displayToken || !outAllowedConfigs) {
        return BAD_VALUE;
    }

    Mutex::Autolock lock(mStateLock);

    const auto display = getDisplayDeviceLocked(displayToken);
    if (!display) {
        return NAME_NOT_FOUND;
    }

    if (display->isPrimary()) {
        outAllowedConfigs->assign(mAllowedDisplayConfigs.begin(), mAllowedDisplayConfigs.end());
    }

    return NO_ERROR;
}

void SurfaceFlinger::SetInputWindowsListener::onSetInputWindowsFinished() {
    mFlinger->setInputWindowsFinished();
}

sp<Layer> SurfaceFlinger::fromHandle(const sp<IBinder>& handle) {
    BBinder *b = handle->localBinder();
    if (b == nullptr) {
        return nullptr;
    }
    auto it = mLayersByLocalBinderToken.find(b);
    if (it != mLayersByLocalBinderToken.end()) {
        return it->second.promote();
    }
    return nullptr;
}

void SurfaceFlinger::bufferErased(const client_cache_t& clientCacheId) {
    getRenderEngine().unbindExternalTextureBuffer(clientCacheId.id);
}

} // namespace android

#if defined(__gl_h_)
#error "don't include gl/gl.h in this file"
#endif

#if defined(__gl2_h_)
#error "don't include gl2/gl2.h in this file"
#endif
