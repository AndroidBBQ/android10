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

#ifndef ANDROID_GUI_SURFACE_COMPOSER_CLIENT_H
#define ANDROID_GUI_SURFACE_COMPOSER_CLIENT_H

#include <stdint.h>
#include <sys/types.h>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <binder/IBinder.h>

#include <utils/RefBase.h>
#include <utils/Singleton.h>
#include <utils/SortedVector.h>
#include <utils/threads.h>

#include <ui/ConfigStoreTypes.h>
#include <ui/DisplayedFrameStats.h>
#include <ui/FrameStats.h>
#include <ui/GraphicTypes.h>
#include <ui/PixelFormat.h>

#include <gui/CpuConsumer.h>
#include <gui/ISurfaceComposer.h>
#include <gui/ITransactionCompletedListener.h>
#include <gui/LayerState.h>
#include <gui/SurfaceControl.h>
#include <math/vec3.h>

namespace android {

// ---------------------------------------------------------------------------

struct DisplayInfo;
class HdrCapabilities;
class ISurfaceComposerClient;
class IGraphicBufferProducer;
class IRegionSamplingListener;
class Region;

// ---------------------------------------------------------------------------

struct SurfaceControlStats {
    SurfaceControlStats(const sp<SurfaceControl>& sc, nsecs_t time,
                        const sp<Fence>& prevReleaseFence)
          : surfaceControl(sc), acquireTime(time), previousReleaseFence(prevReleaseFence) {}

    sp<SurfaceControl> surfaceControl;
    nsecs_t acquireTime = -1;
    sp<Fence> previousReleaseFence;
};

using TransactionCompletedCallbackTakesContext =
        std::function<void(void* /*context*/, nsecs_t /*latchTime*/,
                           const sp<Fence>& /*presentFence*/,
                           const std::vector<SurfaceControlStats>& /*stats*/)>;
using TransactionCompletedCallback =
        std::function<void(nsecs_t /*latchTime*/, const sp<Fence>& /*presentFence*/,
                           const std::vector<SurfaceControlStats>& /*stats*/)>;

// ---------------------------------------------------------------------------

class SurfaceComposerClient : public RefBase
{
    friend class Composer;
public:
                SurfaceComposerClient();
                SurfaceComposerClient(const sp<ISurfaceComposerClient>& client);
    virtual     ~SurfaceComposerClient();

    // Always make sure we could initialize
    status_t    initCheck() const;

    // Return the connection of this client
    sp<IBinder> connection() const;

    // Forcibly remove connection before all references have gone away.
    void        dispose();

    // callback when the composer is dies
    status_t linkToComposerDeath(const sp<IBinder::DeathRecipient>& recipient,
            void* cookie = nullptr, uint32_t flags = 0);

    // Get a list of supported configurations for a given display
    static status_t getDisplayConfigs(const sp<IBinder>& display,
            Vector<DisplayInfo>* configs);

    // Get the DisplayInfo for the currently-active configuration
    static status_t getDisplayInfo(const sp<IBinder>& display,
            DisplayInfo* info);

    // Get the index of the current active configuration (relative to the list
    // returned by getDisplayInfo)
    static int getActiveConfig(const sp<IBinder>& display);

    // Set a new active configuration using an index relative to the list
    // returned by getDisplayInfo
    static status_t setActiveConfig(const sp<IBinder>& display, int id);

    // Sets the allowed display configurations to be used.
    // The allowedConfigs in a vector of indexes corresponding to the configurations
    // returned from getDisplayConfigs().
    static status_t setAllowedDisplayConfigs(const sp<IBinder>& displayToken,
                                             const std::vector<int32_t>& allowedConfigs);

    // Returns the allowed display configurations currently set.
    // The allowedConfigs in a vector of indexes corresponding to the configurations
    // returned from getDisplayConfigs().
    static status_t getAllowedDisplayConfigs(const sp<IBinder>& displayToken,
                                             std::vector<int32_t>* outAllowedConfigs);

    // Gets the list of supported color modes for the given display
    static status_t getDisplayColorModes(const sp<IBinder>& display,
            Vector<ui::ColorMode>* outColorModes);

    // Get the coordinates of the display's native color primaries
    static status_t getDisplayNativePrimaries(const sp<IBinder>& display,
            ui::DisplayPrimaries& outPrimaries);

    // Gets the active color mode for the given display
    static ui::ColorMode getActiveColorMode(const sp<IBinder>& display);

    // Sets the active color mode for the given display
    static status_t setActiveColorMode(const sp<IBinder>& display,
            ui::ColorMode colorMode);

    /* Triggers screen on/off or low power mode and waits for it to complete */
    static void setDisplayPowerMode(const sp<IBinder>& display, int mode);

    /* Returns the composition preference of the default data space and default pixel format,
     * as well as the wide color gamut data space and wide color gamut pixel format.
     * If the wide color gamut data space is V0_SRGB, then it implies that the platform
     * has no wide color gamut support.
     */
    static status_t getCompositionPreference(ui::Dataspace* defaultDataspace,
                                             ui::PixelFormat* defaultPixelFormat,
                                             ui::Dataspace* wideColorGamutDataspace,
                                             ui::PixelFormat* wideColorGamutPixelFormat);

    /*
     * Gets whether SurfaceFlinger can support protected content in GPU composition.
     * Requires the ACCESS_SURFACE_FLINGER permission.
     */
    static bool getProtectedContentSupport();

    /**
     * Called from SurfaceControl d'tor to 'destroy' the surface (or rather, reparent it
     * to null), but without needing an sp<SurfaceControl> to avoid infinite ressurection.
     */
    static void doDropReferenceTransaction(const sp<IBinder>& handle,
            const sp<ISurfaceComposerClient>& client);

    /**
     * Uncaches a buffer in ISurfaceComposer. It must be uncached via a transaction so that it is
     * in order with other transactions that use buffers.
     */
    static void doUncacheBufferTransaction(uint64_t cacheId);

    // Queries whether a given display is wide color display.
    static status_t isWideColorDisplay(const sp<IBinder>& display, bool* outIsWideColorDisplay);

    /*
     * Returns whether brightness operations are supported on a display.
     *
     * displayToken
     *      The token of the display.
     *
     * Returns whether brightness operations are supported on a display or not.
     */
    static bool getDisplayBrightnessSupport(const sp<IBinder>& displayToken);

    /*
     * Sets the brightness of a display.
     *
     * displayToken
     *      The token of the display whose brightness is set.
     * brightness
     *      A number between 0.0 (minimum brightness) and 1.0 (maximum brightness), or -1.0f to
     *      turn the backlight off.
     *
     * Returns NO_ERROR upon success. Otherwise,
     *      NAME_NOT_FOUND    if the display handle is invalid, or
     *      BAD_VALUE         if the brightness value is invalid, or
     *      INVALID_OPERATION if brightness operaetions are not supported.
     */
    static status_t setDisplayBrightness(const sp<IBinder>& displayToken, float brightness);

    /*
     * Sends a power hint to the composer. This function is asynchronous.
     *
     * hintId
     *      hint id according to android::hardware::power::V1_0::PowerHint
     *
     * Returns NO_ERROR upon success.
     */
    static status_t notifyPowerHint(int32_t hintId);

    // ------------------------------------------------------------------------
    // surface creation / destruction

    static sp<SurfaceComposerClient> getDefault();

    //! Create a surface
    sp<SurfaceControl> createSurface(const String8& name,              // name of the surface
                                     uint32_t w,                       // width in pixel
                                     uint32_t h,                       // height in pixel
                                     PixelFormat format,               // pixel-format desired
                                     uint32_t flags = 0,               // usage flags
                                     SurfaceControl* parent = nullptr, // parent
                                     LayerMetadata metadata = LayerMetadata() // metadata
    );

    status_t createSurfaceChecked(const String8& name, // name of the surface
                                  uint32_t w,          // width in pixel
                                  uint32_t h,          // height in pixel
                                  PixelFormat format,  // pixel-format desired
                                  sp<SurfaceControl>* outSurface,
                                  uint32_t flags = 0,                      // usage flags
                                  SurfaceControl* parent = nullptr,        // parent
                                  LayerMetadata metadata = LayerMetadata() // metadata
    );

    //! Create a surface
    sp<SurfaceControl> createWithSurfaceParent(const String8& name,       // name of the surface
                                               uint32_t w,                // width in pixel
                                               uint32_t h,                // height in pixel
                                               PixelFormat format,        // pixel-format desired
                                               uint32_t flags = 0,        // usage flags
                                               Surface* parent = nullptr, // parent
                                               LayerMetadata metadata = LayerMetadata() // metadata
    );

    //! Create a virtual display
    static sp<IBinder> createDisplay(const String8& displayName, bool secure);

    //! Destroy a virtual display
    static void destroyDisplay(const sp<IBinder>& display);

    //! Get stable IDs for connected physical displays
    static std::vector<PhysicalDisplayId> getPhysicalDisplayIds();
    static std::optional<PhysicalDisplayId> getInternalDisplayId();

    //! Get token for a physical display given its stable ID
    static sp<IBinder> getPhysicalDisplayToken(PhysicalDisplayId displayId);
    static sp<IBinder> getInternalDisplayToken();

    static status_t enableVSyncInjections(bool enable);

    static status_t injectVSync(nsecs_t when);

    struct SCHash {
        std::size_t operator()(const sp<SurfaceControl>& sc) const {
            return std::hash<SurfaceControl *>{}(sc.get());
        }
    };

    struct TCLHash {
        std::size_t operator()(const sp<ITransactionCompletedListener>& tcl) const {
            return std::hash<IBinder*>{}((tcl) ? IInterface::asBinder(tcl).get() : nullptr);
        }
    };

    struct CallbackInfo {
        // All the callbacks that have been requested for a TransactionCompletedListener in the
        // Transaction
        std::unordered_set<CallbackId> callbackIds;
        // All the SurfaceControls that have been modified in this TransactionCompletedListener's
        // process that require a callback if there is one or more callbackIds set.
        std::unordered_set<sp<SurfaceControl>, SCHash> surfaceControls;
    };

    class Transaction {
        std::unordered_map<sp<SurfaceControl>, ComposerState, SCHash> mComposerStates;
        SortedVector<DisplayState > mDisplayStates;
        std::unordered_map<sp<ITransactionCompletedListener>, CallbackInfo, TCLHash>
                mListenerCallbacks;

        uint32_t                    mForceSynchronous = 0;
        uint32_t                    mTransactionNestCount = 0;
        bool                        mAnimation = false;
        bool                        mEarlyWakeup = false;

        // Indicates that the Transaction contains a buffer that should be cached
        bool mContainsBuffer = false;

        // mDesiredPresentTime is the time in nanoseconds that the client would like the transaction
        // to be presented. When it is not possible to present at exactly that time, it will be
        // presented after the time has passed.
        //
        // Desired present times that are more than 1 second in the future may be ignored.
        // When a desired present time has already passed, the transaction will be presented as soon
        // as possible.
        //
        // Transactions from the same process are presented in the same order that they are applied.
        // The desired present time does not affect this ordering.
        int64_t mDesiredPresentTime = -1;

        InputWindowCommands mInputWindowCommands;
        int mStatus = NO_ERROR;

        layer_state_t* getLayerState(const sp<SurfaceControl>& sc);
        DisplayState& getDisplayState(const sp<IBinder>& token);

        void cacheBuffers();
        void registerSurfaceControlForCallback(const sp<SurfaceControl>& sc);

    public:
        Transaction() = default;
        virtual ~Transaction() = default;
        Transaction(Transaction const& other);

        status_t apply(bool synchronous = false);
        // Merge another transaction in to this one, clearing other
        // as if it had been applied.
        Transaction& merge(Transaction&& other);
        Transaction& show(const sp<SurfaceControl>& sc);
        Transaction& hide(const sp<SurfaceControl>& sc);
        Transaction& setPosition(const sp<SurfaceControl>& sc,
                float x, float y);
        Transaction& setSize(const sp<SurfaceControl>& sc,
                uint32_t w, uint32_t h);
        Transaction& setLayer(const sp<SurfaceControl>& sc,
                int32_t z);

        // Sets a Z order relative to the Surface specified by "relativeTo" but
        // without becoming a full child of the relative. Z-ordering works exactly
        // as if it were a child however.
        //
        // As a nod to sanity, only non-child surfaces may have a relative Z-order.
        //
        // This overrides any previous call and is overriden by any future calls
        // to setLayer.
        //
        // If the relative is removed, the Surface will have no layer and be
        // invisible, until the next time set(Relative)Layer is called.
        Transaction& setRelativeLayer(const sp<SurfaceControl>& sc,
                const sp<IBinder>& relativeTo, int32_t z);
        Transaction& setFlags(const sp<SurfaceControl>& sc,
                uint32_t flags, uint32_t mask);
        Transaction& setTransparentRegionHint(const sp<SurfaceControl>& sc,
                const Region& transparentRegion);
        Transaction& setAlpha(const sp<SurfaceControl>& sc,
                float alpha);
        Transaction& setMatrix(const sp<SurfaceControl>& sc,
                float dsdx, float dtdx, float dtdy, float dsdy);
        Transaction& setCrop_legacy(const sp<SurfaceControl>& sc, const Rect& crop);
        Transaction& setCornerRadius(const sp<SurfaceControl>& sc, float cornerRadius);
        Transaction& setLayerStack(const sp<SurfaceControl>& sc, uint32_t layerStack);
        Transaction& setMetadata(const sp<SurfaceControl>& sc, uint32_t key,
                                 std::vector<uint8_t> data);
        // Defers applying any changes made in this transaction until the Layer
        // identified by handle reaches the given frameNumber. If the Layer identified
        // by handle is removed, then we will apply this transaction regardless of
        // what frame number has been reached.
        Transaction& deferTransactionUntil_legacy(const sp<SurfaceControl>& sc,
                                                  const sp<IBinder>& handle, uint64_t frameNumber);
        // A variant of deferTransactionUntil_legacy which identifies the Layer we wait for by
        // Surface instead of Handle. Useful for clients which may not have the
        // SurfaceControl for some of their Surfaces. Otherwise behaves identically.
        Transaction& deferTransactionUntil_legacy(const sp<SurfaceControl>& sc,
                                                  const sp<Surface>& barrierSurface,
                                                  uint64_t frameNumber);
        // Reparents all children of this layer to the new parent handle.
        Transaction& reparentChildren(const sp<SurfaceControl>& sc,
                const sp<IBinder>& newParentHandle);

        /// Reparents the current layer to the new parent handle. The new parent must not be null.
        // This can be used instead of reparentChildren if the caller wants to
        // only re-parent a specific child.
        Transaction& reparent(const sp<SurfaceControl>& sc,
                const sp<IBinder>& newParentHandle);

        Transaction& setColor(const sp<SurfaceControl>& sc, const half3& color);

        // Sets the background color of a layer with the specified color, alpha, and dataspace
        Transaction& setBackgroundColor(const sp<SurfaceControl>& sc, const half3& color,
                                        float alpha, ui::Dataspace dataspace);

        Transaction& setTransform(const sp<SurfaceControl>& sc, uint32_t transform);
        Transaction& setTransformToDisplayInverse(const sp<SurfaceControl>& sc,
                                                  bool transformToDisplayInverse);
        Transaction& setCrop(const sp<SurfaceControl>& sc, const Rect& crop);
        Transaction& setFrame(const sp<SurfaceControl>& sc, const Rect& frame);
        Transaction& setBuffer(const sp<SurfaceControl>& sc, const sp<GraphicBuffer>& buffer);
        Transaction& setCachedBuffer(const sp<SurfaceControl>& sc, int32_t bufferId);
        Transaction& setAcquireFence(const sp<SurfaceControl>& sc, const sp<Fence>& fence);
        Transaction& setDataspace(const sp<SurfaceControl>& sc, ui::Dataspace dataspace);
        Transaction& setHdrMetadata(const sp<SurfaceControl>& sc, const HdrMetadata& hdrMetadata);
        Transaction& setSurfaceDamageRegion(const sp<SurfaceControl>& sc,
                                            const Region& surfaceDamageRegion);
        Transaction& setApi(const sp<SurfaceControl>& sc, int32_t api);
        Transaction& setSidebandStream(const sp<SurfaceControl>& sc,
                                       const sp<NativeHandle>& sidebandStream);
        Transaction& setDesiredPresentTime(nsecs_t desiredPresentTime);
        Transaction& setColorSpaceAgnostic(const sp<SurfaceControl>& sc, const bool agnostic);

        Transaction& addTransactionCompletedCallback(
                TransactionCompletedCallbackTakesContext callback, void* callbackContext);

        // Detaches all child surfaces (and their children recursively)
        // from their SurfaceControl.
        // The child SurfaceControls will not throw exceptions or return errors,
        // but transactions will have no effect.
        // The child surfaces will continue to follow their parent surfaces,
        // and remain eligible for rendering, but their relative state will be
        // frozen. We use this in the WindowManager, in app shutdown/relaunch
        // scenarios, where the app would otherwise clean up its child Surfaces.
        // Sometimes the WindowManager needs to extend their lifetime slightly
        // in order to perform an exit animation or prevent flicker.
        Transaction& detachChildren(const sp<SurfaceControl>& sc);
        // Set an override scaling mode as documented in <system/window.h>
        // the override scaling mode will take precedence over any client
        // specified scaling mode. -1 will clear the override scaling mode.
        Transaction& setOverrideScalingMode(const sp<SurfaceControl>& sc,
                int32_t overrideScalingMode);

        // If the size changes in this transaction, all geometry updates specified
        // in this transaction will not complete until a buffer of the new size
        // arrives. As some elements normally apply immediately, this enables
        // freezing the total geometry of a surface until a resize is completed.
        Transaction& setGeometryAppliesWithResize(const sp<SurfaceControl>& sc);

#ifndef NO_INPUT
        Transaction& setInputWindowInfo(const sp<SurfaceControl>& sc, const InputWindowInfo& info);
        Transaction& transferTouchFocus(const sp<IBinder>& fromToken, const sp<IBinder>& toToken);
        Transaction& syncInputWindows();
#endif

        // Set a color transform matrix on the given layer on the built-in display.
        Transaction& setColorTransform(const sp<SurfaceControl>& sc, const mat3& matrix,
                                       const vec3& translation);

        Transaction& setGeometry(const sp<SurfaceControl>& sc,
                const Rect& source, const Rect& dst, int transform);

        status_t setDisplaySurface(const sp<IBinder>& token,
                const sp<IGraphicBufferProducer>& bufferProducer);

        void setDisplayLayerStack(const sp<IBinder>& token, uint32_t layerStack);

        /* setDisplayProjection() defines the projection of layer stacks
         * to a given display.
         *
         * - orientation defines the display's orientation.
         * - layerStackRect defines which area of the window manager coordinate
         * space will be used.
         * - displayRect defines where on the display will layerStackRect be
         * mapped to. displayRect is specified post-orientation, that is
         * it uses the orientation seen by the end-user.
         */
        void setDisplayProjection(const sp<IBinder>& token,
                uint32_t orientation,
                const Rect& layerStackRect,
                const Rect& displayRect);
        void setDisplaySize(const sp<IBinder>& token, uint32_t width, uint32_t height);
        void setAnimationTransaction();
        void setEarlyWakeup();
    };

    status_t clearLayerFrameStats(const sp<IBinder>& token) const;
    status_t getLayerFrameStats(const sp<IBinder>& token, FrameStats* outStats) const;
    static status_t clearAnimationFrameStats();
    static status_t getAnimationFrameStats(FrameStats* outStats);

    static status_t getHdrCapabilities(const sp<IBinder>& display,
            HdrCapabilities* outCapabilities);

    static void setDisplayProjection(const sp<IBinder>& token,
            uint32_t orientation,
            const Rect& layerStackRect,
            const Rect& displayRect);

    inline sp<ISurfaceComposerClient> getClient() { return mClient; }

    static status_t getDisplayedContentSamplingAttributes(const sp<IBinder>& display,
                                                          ui::PixelFormat* outFormat,
                                                          ui::Dataspace* outDataspace,
                                                          uint8_t* outComponentMask);
    static status_t setDisplayContentSamplingEnabled(const sp<IBinder>& display, bool enable,
                                                     uint8_t componentMask, uint64_t maxFrames);

    static status_t getDisplayedContentSample(const sp<IBinder>& display, uint64_t maxFrames,
                                              uint64_t timestamp, DisplayedFrameStats* outStats);
    static status_t addRegionSamplingListener(const Rect& samplingArea,
                                              const sp<IBinder>& stopLayerHandle,
                                              const sp<IRegionSamplingListener>& listener);
    static status_t removeRegionSamplingListener(const sp<IRegionSamplingListener>& listener);

private:
    virtual void onFirstRef();

    mutable     Mutex                       mLock;
                status_t                    mStatus;
                sp<ISurfaceComposerClient>  mClient;
};

// ---------------------------------------------------------------------------

class ScreenshotClient {
public:
    // if cropping isn't required, callers may pass in a default Rect, e.g.:
    //   capture(display, producer, Rect(), reqWidth, ...);
    static status_t capture(const sp<IBinder>& display, const ui::Dataspace reqDataSpace,
                            const ui::PixelFormat reqPixelFormat, Rect sourceCrop,
                            uint32_t reqWidth, uint32_t reqHeight, bool useIdentityTransform,
                            uint32_t rotation, bool captureSecureLayers,
                            sp<GraphicBuffer>* outBuffer, bool& outCapturedSecureLayers);
    static status_t capture(const sp<IBinder>& display, const ui::Dataspace reqDataSpace,
                            const ui::PixelFormat reqPixelFormat, Rect sourceCrop,
                            uint32_t reqWidth, uint32_t reqHeight, bool useIdentityTransform,
                            uint32_t rotation, sp<GraphicBuffer>* outBuffer);
    static status_t capture(uint64_t displayOrLayerStack, ui::Dataspace* outDataspace,
                            sp<GraphicBuffer>* outBuffer);
    static status_t captureLayers(const sp<IBinder>& layerHandle, const ui::Dataspace reqDataSpace,
                                  const ui::PixelFormat reqPixelFormat, Rect sourceCrop,
                                  float frameScale, sp<GraphicBuffer>* outBuffer);
    static status_t captureChildLayers(
            const sp<IBinder>& layerHandle, const ui::Dataspace reqDataSpace,
            const ui::PixelFormat reqPixelFormat, Rect sourceCrop,
            const std::unordered_set<sp<IBinder>, ISurfaceComposer::SpHash<IBinder>>&
                    excludeHandles,
            float frameScale, sp<GraphicBuffer>* outBuffer);
};

// ---------------------------------------------------------------------------

class TransactionCompletedListener : public BnTransactionCompletedListener {
    TransactionCompletedListener();

    CallbackId getNextIdLocked() REQUIRES(mMutex);

    std::mutex mMutex;

    bool mListening GUARDED_BY(mMutex) = false;

    CallbackId mCallbackIdCounter GUARDED_BY(mMutex) = 1;

    struct IBinderHash {
        std::size_t operator()(const sp<IBinder>& iBinder) const {
            return std::hash<IBinder*>{}(iBinder.get());
        }
    };

    struct CallbackTranslation {
        TransactionCompletedCallback callbackFunction;
        std::unordered_map<sp<IBinder>, sp<SurfaceControl>, IBinderHash> surfaceControls;
    };

    std::unordered_map<CallbackId, CallbackTranslation> mCallbacks GUARDED_BY(mMutex);

public:
    static sp<TransactionCompletedListener> getInstance();
    static sp<ITransactionCompletedListener> getIInstance();

    void startListeningLocked() REQUIRES(mMutex);

    CallbackId addCallbackFunction(
            const TransactionCompletedCallback& callbackFunction,
            const std::unordered_set<sp<SurfaceControl>, SurfaceComposerClient::SCHash>&
                    surfaceControls);

    void addSurfaceControlToCallbacks(const sp<SurfaceControl>& surfaceControl,
                                      const std::unordered_set<CallbackId>& callbackIds);

    // Overrides BnTransactionCompletedListener's onTransactionCompleted
    void onTransactionCompleted(ListenerStats stats) override;
};

// ---------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_GUI_SURFACE_COMPOSER_CLIENT_H
