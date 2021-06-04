/*
 * Copyright (C) 2006 The Android Open Source Project
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

#ifndef ANDROID_GUI_ISURFACE_COMPOSER_H
#define ANDROID_GUI_ISURFACE_COMPOSER_H

#include <stdint.h>
#include <sys/types.h>

#include <binder/IBinder.h>
#include <binder/IInterface.h>

#include <gui/ITransactionCompletedListener.h>

#include <ui/ConfigStoreTypes.h>
#include <ui/DisplayedFrameStats.h>
#include <ui/FrameStats.h>
#include <ui/GraphicBuffer.h>
#include <ui/GraphicTypes.h>
#include <ui/PixelFormat.h>

#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Timers.h>
#include <utils/Vector.h>

#include <optional>
#include <unordered_set>
#include <vector>

namespace android {
// ----------------------------------------------------------------------------

struct client_cache_t;
struct ComposerState;
struct DisplayState;
struct DisplayInfo;
struct DisplayStatInfo;
struct InputWindowCommands;
class LayerDebugInfo;
class HdrCapabilities;
class IDisplayEventConnection;
class IGraphicBufferProducer;
class ISurfaceComposerClient;
class IRegionSamplingListener;
class Rect;
enum class FrameEvent;

/*
 * This class defines the Binder IPC interface for accessing various
 * SurfaceFlinger features.
 */
class ISurfaceComposer: public IInterface {
public:
    DECLARE_META_INTERFACE(SurfaceComposer)

    // flags for setTransactionState()
    enum {
        eSynchronous = 0x01,
        eAnimation   = 0x02,

        // Indicates that this transaction will likely result in a lot of layers being composed, and
        // thus, SurfaceFlinger should wake-up earlier to avoid missing frame deadlines. In this
        // case SurfaceFlinger will wake up at (sf vsync offset - debug.sf.early_phase_offset_ns)
        eEarlyWakeup = 0x04
    };

    enum Rotation {
        eRotateNone = 0,
        eRotate90   = 1,
        eRotate180  = 2,
        eRotate270  = 3
    };

    enum VsyncSource {
        eVsyncSourceApp = 0,
        eVsyncSourceSurfaceFlinger = 1
    };

    enum ConfigChanged { eConfigChangedSuppress = 0, eConfigChangedDispatch = 1 };

    /*
     * Create a connection with SurfaceFlinger.
     */
    virtual sp<ISurfaceComposerClient> createConnection() = 0;

    /* return an IDisplayEventConnection */
    virtual sp<IDisplayEventConnection> createDisplayEventConnection(
            VsyncSource vsyncSource = eVsyncSourceApp,
            ConfigChanged configChanged = eConfigChangedSuppress) = 0;

    /* create a virtual display
     * requires ACCESS_SURFACE_FLINGER permission.
     */
    virtual sp<IBinder> createDisplay(const String8& displayName,
            bool secure) = 0;

    /* destroy a virtual display
     * requires ACCESS_SURFACE_FLINGER permission.
     */
    virtual void destroyDisplay(const sp<IBinder>& display) = 0;

    /* get stable IDs for connected physical displays.
     */
    virtual std::vector<PhysicalDisplayId> getPhysicalDisplayIds() const = 0;

    // TODO(b/74619554): Remove this stopgap once the framework is display-agnostic.
    std::optional<PhysicalDisplayId> getInternalDisplayId() const {
        const auto displayIds = getPhysicalDisplayIds();
        return displayIds.empty() ? std::nullopt : std::make_optional(displayIds.front());
    }

    /* get token for a physical display given its stable ID obtained via getPhysicalDisplayIds or a
     * DisplayEventReceiver hotplug event.
     */
    virtual sp<IBinder> getPhysicalDisplayToken(PhysicalDisplayId displayId) const = 0;

    // TODO(b/74619554): Remove this stopgap once the framework is display-agnostic.
    sp<IBinder> getInternalDisplayToken() const {
        const auto displayId = getInternalDisplayId();
        return displayId ? getPhysicalDisplayToken(*displayId) : nullptr;
    }

    /* open/close transactions. requires ACCESS_SURFACE_FLINGER permission */
    virtual void setTransactionState(const Vector<ComposerState>& state,
                                     const Vector<DisplayState>& displays, uint32_t flags,
                                     const sp<IBinder>& applyToken,
                                     const InputWindowCommands& inputWindowCommands,
                                     int64_t desiredPresentTime,
                                     const client_cache_t& uncacheBuffer,
                                     const std::vector<ListenerCallbacks>& listenerCallbacks) = 0;

    /* signal that we're done booting.
     * Requires ACCESS_SURFACE_FLINGER permission
     */
    virtual void bootFinished() = 0;

    /* verify that an IGraphicBufferProducer was created by SurfaceFlinger.
     */
    virtual bool authenticateSurfaceTexture(
            const sp<IGraphicBufferProducer>& surface) const = 0;

    /* Returns the frame timestamps supported by SurfaceFlinger.
     */
    virtual status_t getSupportedFrameTimestamps(
            std::vector<FrameEvent>* outSupported) const = 0;

    /* set display power mode. depending on the mode, it can either trigger
     * screen on, off or low power mode and wait for it to complete.
     * requires ACCESS_SURFACE_FLINGER permission.
     */
    virtual void setPowerMode(const sp<IBinder>& display, int mode) = 0;

    /* returns information for each configuration of the given display
     * intended to be used to get information about built-in displays */
    virtual status_t getDisplayConfigs(const sp<IBinder>& display,
            Vector<DisplayInfo>* configs) = 0;

    /* returns display statistics for a given display
     * intended to be used by the media framework to properly schedule
     * video frames */
    virtual status_t getDisplayStats(const sp<IBinder>& display,
            DisplayStatInfo* stats) = 0;

    /* indicates which of the configurations returned by getDisplayInfo is
     * currently active */
    virtual int getActiveConfig(const sp<IBinder>& display) = 0;

    /* specifies which configuration (of those returned by getDisplayInfo)
     * should be used */
    virtual status_t setActiveConfig(const sp<IBinder>& display, int id) = 0;

    virtual status_t getDisplayColorModes(const sp<IBinder>& display,
            Vector<ui::ColorMode>* outColorModes) = 0;
    virtual status_t getDisplayNativePrimaries(const sp<IBinder>& display,
            ui::DisplayPrimaries& primaries) = 0;
    virtual ui::ColorMode getActiveColorMode(const sp<IBinder>& display) = 0;
    virtual status_t setActiveColorMode(const sp<IBinder>& display,
            ui::ColorMode colorMode) = 0;

    /**
     * Capture the specified screen. This requires READ_FRAME_BUFFER
     * permission.  This function will fail if there is a secure window on
     * screen.
     *
     * This function can capture a subregion (the source crop) of the screen.
     * The subregion can be optionally rotated.  It will also be scaled to
     * match the size of the output buffer.
     *
     * reqDataspace and reqPixelFormat specify the data space and pixel format
     * of the buffer. The caller should pick the data space and pixel format
     * that it can consume.
     *
     * sourceCrop is the crop on the logical display.
     *
     * reqWidth and reqHeight specifies the size of the buffer.  When either
     * of them is 0, they are set to the size of the logical display viewport.
     *
     * When useIdentityTransform is true, layer transformations are disabled.
     *
     * rotation specifies the rotation of the source crop (and the pixels in
     * it) around its center.
     */
    virtual status_t captureScreen(const sp<IBinder>& display, sp<GraphicBuffer>* outBuffer,
                                   bool& outCapturedSecureLayers, const ui::Dataspace reqDataspace,
                                   const ui::PixelFormat reqPixelFormat, Rect sourceCrop,
                                   uint32_t reqWidth, uint32_t reqHeight, bool useIdentityTransform,
                                   Rotation rotation = eRotateNone,
                                   bool captureSecureLayers = false) = 0;
    /**
     * Capture the specified screen. This requires READ_FRAME_BUFFER
     * permission.  This function will fail if there is a secure window on
     * screen.
     *
     * This function can capture a subregion (the source crop) of the screen
     * into an sRGB buffer with RGBA_8888 pixel format.
     * The subregion can be optionally rotated.  It will also be scaled to
     * match the size of the output buffer.
     *
     * At the moment, sourceCrop is ignored and is always set to the visible
     * region (projected display viewport) of the screen.
     *
     * reqWidth and reqHeight specifies the size of the buffer.  When either
     * of them is 0, they are set to the size of the logical display viewport.
     *
     * When useIdentityTransform is true, layer transformations are disabled.
     *
     * rotation specifies the rotation of the source crop (and the pixels in
     * it) around its center.
     */
    virtual status_t captureScreen(const sp<IBinder>& display, sp<GraphicBuffer>* outBuffer,
                                   Rect sourceCrop, uint32_t reqWidth, uint32_t reqHeight,
                                   bool useIdentityTransform, Rotation rotation = eRotateNone) {
        bool outIgnored;
        return captureScreen(display, outBuffer, outIgnored, ui::Dataspace::V0_SRGB,
                             ui::PixelFormat::RGBA_8888, sourceCrop, reqWidth, reqHeight,
                             useIdentityTransform, rotation);
    }

    virtual status_t captureScreen(uint64_t displayOrLayerStack, ui::Dataspace* outDataspace,
                                   sp<GraphicBuffer>* outBuffer) = 0;

    template <class AA>
    struct SpHash {
        size_t operator()(const sp<AA>& k) const { return std::hash<AA*>()(k.get()); }
    };

    /**
     * Capture a subtree of the layer hierarchy, potentially ignoring the root node.
     *
     * reqDataspace and reqPixelFormat specify the data space and pixel format
     * of the buffer. The caller should pick the data space and pixel format
     * that it can consume.
     */
    virtual status_t captureLayers(
            const sp<IBinder>& layerHandleBinder, sp<GraphicBuffer>* outBuffer,
            const ui::Dataspace reqDataspace, const ui::PixelFormat reqPixelFormat,
            const Rect& sourceCrop,
            const std::unordered_set<sp<IBinder>, SpHash<IBinder>>& excludeHandles,
            float frameScale = 1.0, bool childrenOnly = false) = 0;

    /**
     * Capture a subtree of the layer hierarchy into an sRGB buffer with RGBA_8888 pixel format,
     * potentially ignoring the root node.
     */
    status_t captureLayers(const sp<IBinder>& layerHandleBinder, sp<GraphicBuffer>* outBuffer,
                           const Rect& sourceCrop, float frameScale = 1.0,
                           bool childrenOnly = false) {
        return captureLayers(layerHandleBinder, outBuffer, ui::Dataspace::V0_SRGB,
                             ui::PixelFormat::RGBA_8888, sourceCrop, {}, frameScale, childrenOnly);
    }

    /* Clears the frame statistics for animations.
     *
     * Requires the ACCESS_SURFACE_FLINGER permission.
     */
    virtual status_t clearAnimationFrameStats() = 0;

    /* Gets the frame statistics for animations.
     *
     * Requires the ACCESS_SURFACE_FLINGER permission.
     */
    virtual status_t getAnimationFrameStats(FrameStats* outStats) const = 0;

    /* Gets the supported HDR capabilities of the given display.
     *
     * Requires the ACCESS_SURFACE_FLINGER permission.
     */
    virtual status_t getHdrCapabilities(const sp<IBinder>& display,
            HdrCapabilities* outCapabilities) const = 0;

    virtual status_t enableVSyncInjections(bool enable) = 0;

    virtual status_t injectVSync(nsecs_t when) = 0;

    /* Gets the list of active layers in Z order for debugging purposes
     *
     * Requires the ACCESS_SURFACE_FLINGER permission.
     */
    virtual status_t getLayerDebugInfo(std::vector<LayerDebugInfo>* outLayers) const = 0;

    virtual status_t getColorManagement(bool* outGetColorManagement) const = 0;

    /* Gets the composition preference of the default data space and default pixel format,
     * as well as the wide color gamut data space and wide color gamut pixel format.
     * If the wide color gamut data space is V0_SRGB, then it implies that the platform
     * has no wide color gamut support.
     *
     * Requires the ACCESS_SURFACE_FLINGER permission.
     */
    virtual status_t getCompositionPreference(ui::Dataspace* defaultDataspace,
                                              ui::PixelFormat* defaultPixelFormat,
                                              ui::Dataspace* wideColorGamutDataspace,
                                              ui::PixelFormat* wideColorGamutPixelFormat) const = 0;
    /*
     * Requires the ACCESS_SURFACE_FLINGER permission.
     */
    virtual status_t getDisplayedContentSamplingAttributes(const sp<IBinder>& display,
                                                           ui::PixelFormat* outFormat,
                                                           ui::Dataspace* outDataspace,
                                                           uint8_t* outComponentMask) const = 0;

    /* Turns on the color sampling engine on the display.
     *
     * Requires the ACCESS_SURFACE_FLINGER permission.
     */
    virtual status_t setDisplayContentSamplingEnabled(const sp<IBinder>& display, bool enable,
                                                      uint8_t componentMask,
                                                      uint64_t maxFrames) const = 0;

    /* Returns statistics on the color profile of the last frame displayed for a given display
     *
     * Requires the ACCESS_SURFACE_FLINGER permission.
     */
    virtual status_t getDisplayedContentSample(const sp<IBinder>& display, uint64_t maxFrames,
                                               uint64_t timestamp,
                                               DisplayedFrameStats* outStats) const = 0;

    /*
     * Gets whether SurfaceFlinger can support protected content in GPU composition.
     * Requires the ACCESS_SURFACE_FLINGER permission.
     */
    virtual status_t getProtectedContentSupport(bool* outSupported) const = 0;

    /*
     * Queries whether the given display is a wide color display.
     * Requires the ACCESS_SURFACE_FLINGER permission.
     */
    virtual status_t isWideColorDisplay(const sp<IBinder>& token,
                                        bool* outIsWideColorDisplay) const = 0;

    /* Registers a listener to stream median luma updates from SurfaceFlinger.
     *
     * The sampling area is bounded by both samplingArea and the given stopLayerHandle
     * (i.e., only layers behind the stop layer will be captured and sampled).
     *
     * Multiple listeners may be provided so long as they have independent listeners.
     * If multiple listeners are provided, the effective sampling region for each listener will
     * be bounded by whichever stop layer has a lower Z value.
     *
     * Requires the same permissions as captureLayers and captureScreen.
     */
    virtual status_t addRegionSamplingListener(const Rect& samplingArea,
                                               const sp<IBinder>& stopLayerHandle,
                                               const sp<IRegionSamplingListener>& listener) = 0;

    /*
     * Removes a listener that was streaming median luma updates from SurfaceFlinger.
     */
    virtual status_t removeRegionSamplingListener(const sp<IRegionSamplingListener>& listener) = 0;

    /*
     * Sets the allowed display configurations to be used.
     * The allowedConfigs in a vector of indexes corresponding to the configurations
     * returned from getDisplayConfigs().
     */
    virtual status_t setAllowedDisplayConfigs(const sp<IBinder>& displayToken,
                                              const std::vector<int32_t>& allowedConfigs) = 0;

    /*
     * Returns the allowed display configurations currently set.
     * The allowedConfigs in a vector of indexes corresponding to the configurations
     * returned from getDisplayConfigs().
     */
    virtual status_t getAllowedDisplayConfigs(const sp<IBinder>& displayToken,
                                              std::vector<int32_t>* outAllowedConfigs) = 0;
    /*
     * Gets whether brightness operations are supported on a display.
     *
     * displayToken
     *      The token of the display.
     * outSupport
     *      An output parameter for whether brightness operations are supported.
     *
     * Returns NO_ERROR upon success. Otherwise,
     *      NAME_NOT_FOUND if the display is invalid, or
     *      BAD_VALUE      if the output parameter is invalid.
     */
    virtual status_t getDisplayBrightnessSupport(const sp<IBinder>& displayToken,
                                                 bool* outSupport) const = 0;

    /*
     * Sets the brightness of a display.
     *
     * displayToken
     *      The token of the display whose brightness is set.
     * brightness
     *      A number between 0.0f (minimum brightness) and 1.0 (maximum brightness), or -1.0f to
     *      turn the backlight off.
     *
     * Returns NO_ERROR upon success. Otherwise,
     *      NAME_NOT_FOUND    if the display is invalid, or
     *      BAD_VALUE         if the brightness is invalid, or
     *      INVALID_OPERATION if brightness operations are not supported.
     */
    virtual status_t setDisplayBrightness(const sp<IBinder>& displayToken,
                                          float brightness) const = 0;

    /*
     * Sends a power hint to the composer. This function is asynchronous.
     *
     * hintId
     *      hint id according to android::hardware::power::V1_0::PowerHint
     *
     * Returns NO_ERROR upon success.
     */
    virtual status_t notifyPowerHint(int32_t hintId) = 0;
};

// ----------------------------------------------------------------------------

class BnSurfaceComposer: public BnInterface<ISurfaceComposer> {
public:
    enum ISurfaceComposerTag {
        // Note: BOOT_FINISHED must remain this value, it is called from
        // Java by ActivityManagerService.
        BOOT_FINISHED = IBinder::FIRST_CALL_TRANSACTION,
        CREATE_CONNECTION,
        CREATE_GRAPHIC_BUFFER_ALLOC_UNUSED, // unused, fails permissions check
        CREATE_DISPLAY_EVENT_CONNECTION,
        CREATE_DISPLAY,
        DESTROY_DISPLAY,
        GET_PHYSICAL_DISPLAY_TOKEN,
        SET_TRANSACTION_STATE,
        AUTHENTICATE_SURFACE,
        GET_SUPPORTED_FRAME_TIMESTAMPS,
        GET_DISPLAY_CONFIGS,
        GET_ACTIVE_CONFIG,
        SET_ACTIVE_CONFIG,
        CONNECT_DISPLAY_UNUSED, // unused, fails permissions check
        CAPTURE_SCREEN,
        CAPTURE_LAYERS,
        CLEAR_ANIMATION_FRAME_STATS,
        GET_ANIMATION_FRAME_STATS,
        SET_POWER_MODE,
        GET_DISPLAY_STATS,
        GET_HDR_CAPABILITIES,
        GET_DISPLAY_COLOR_MODES,
        GET_ACTIVE_COLOR_MODE,
        SET_ACTIVE_COLOR_MODE,
        ENABLE_VSYNC_INJECTIONS,
        INJECT_VSYNC,
        GET_LAYER_DEBUG_INFO,
        GET_COMPOSITION_PREFERENCE,
        GET_COLOR_MANAGEMENT,
        GET_DISPLAYED_CONTENT_SAMPLING_ATTRIBUTES,
        SET_DISPLAY_CONTENT_SAMPLING_ENABLED,
        GET_DISPLAYED_CONTENT_SAMPLE,
        GET_PROTECTED_CONTENT_SUPPORT,
        IS_WIDE_COLOR_DISPLAY,
        GET_DISPLAY_NATIVE_PRIMARIES,
        GET_PHYSICAL_DISPLAY_IDS,
        ADD_REGION_SAMPLING_LISTENER,
        REMOVE_REGION_SAMPLING_LISTENER,
        SET_ALLOWED_DISPLAY_CONFIGS,
        GET_ALLOWED_DISPLAY_CONFIGS,
        GET_DISPLAY_BRIGHTNESS_SUPPORT,
        SET_DISPLAY_BRIGHTNESS,
        CAPTURE_SCREEN_BY_ID,
        NOTIFY_POWER_HINT,
        // Always append new enum to the end.
    };

    virtual status_t onTransact(uint32_t code, const Parcel& data,
            Parcel* reply, uint32_t flags = 0);
};

// ----------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_GUI_ISURFACE_COMPOSER_H
