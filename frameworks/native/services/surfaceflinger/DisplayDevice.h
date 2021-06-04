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

#ifndef ANDROID_DISPLAY_DEVICE_H
#define ANDROID_DISPLAY_DEVICE_H

#include <stdlib.h>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include <android/native_window.h>
#include <binder/IBinder.h>
#include <gui/LayerState.h>
#include <hardware/hwcomposer_defs.h>
#include <math/mat4.h>
#include <renderengine/RenderEngine.h>
#include <system/window.h>
#include <ui/GraphicTypes.h>
#include <ui/HdrCapabilities.h>
#include <ui/Region.h>
#include <ui/Transform.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <utils/Timers.h>

#include "DisplayHardware/DisplayIdentification.h"
#include "RenderArea.h"

namespace android {

class Fence;
class HWComposer;
class IGraphicBufferProducer;
class Layer;
class SurfaceFlinger;

struct CompositionInfo;
struct DisplayDeviceCreationArgs;
struct DisplayInfo;

namespace compositionengine {
class Display;
class DisplaySurface;
} // namespace compositionengine

class DisplayDevice : public LightRefBase<DisplayDevice> {
public:
    constexpr static float sDefaultMinLumiance = 0.0;
    constexpr static float sDefaultMaxLumiance = 500.0;

    enum {
        NO_LAYER_STACK = 0xFFFFFFFF,
    };

    explicit DisplayDevice(DisplayDeviceCreationArgs&& args);
    virtual ~DisplayDevice();

    std::shared_ptr<compositionengine::Display> getCompositionDisplay() const {
        return mCompositionDisplay;
    }

    bool isVirtual() const { return mIsVirtual; }
    bool isPrimary() const { return mIsPrimary; }

    // isSecure indicates whether this display can be trusted to display
    // secure surfaces.
    bool isSecure() const;

    int         getWidth() const;
    int         getHeight() const;
    int         getInstallOrientation() const { return mDisplayInstallOrientation; }

    void                    setVisibleLayersSortedByZ(const Vector< sp<Layer> >& layers);
    const Vector< sp<Layer> >& getVisibleLayersSortedByZ() const;
    void                    setLayersNeedingFences(const Vector< sp<Layer> >& layers);
    const Vector< sp<Layer> >& getLayersNeedingFences() const;

    void                    setLayerStack(uint32_t stack);
    void                    setDisplaySize(const int newWidth, const int newHeight);
    void                    setProjection(int orientation, const Rect& viewport, const Rect& frame);

    int                     getOrientation() const { return mOrientation; }
    static uint32_t         getPrimaryDisplayOrientationTransform();
    const ui::Transform& getTransform() const;
    const Rect& getViewport() const;
    const Rect& getFrame() const;
    const Rect& getScissor() const;
    bool needsFiltering() const;
    uint32_t getLayerStack() const;

    const std::optional<DisplayId>& getId() const;
    const wp<IBinder>& getDisplayToken() const { return mDisplayToken; }
    int32_t getSequenceId() const { return mSequenceId; }

    const Region& getUndefinedRegion() const;

    int32_t getSupportedPerFrameMetadata() const;

    bool hasWideColorGamut() const;
    // Whether h/w composer has native support for specific HDR type.
    bool hasHDR10PlusSupport() const;
    bool hasHDR10Support() const;
    bool hasHLGSupport() const;
    bool hasDolbyVisionSupport() const;

    // The returned HdrCapabilities is the combination of HDR capabilities from
    // hardware composer and RenderEngine. When the DisplayDevice supports wide
    // color gamut, RenderEngine is able to simulate HDR support in Display P3
    // color space for both PQ and HLG HDR contents. The minimum and maximum
    // luminance will be set to sDefaultMinLumiance and sDefaultMaxLumiance
    // respectively if hardware composer doesn't return meaningful values.
    const HdrCapabilities& getHdrCapabilities() const;

    // Return true if intent is supported by the display.
    bool hasRenderIntent(ui::RenderIntent intent) const;

    const Rect& getBounds() const;
    const Rect& bounds() const { return getBounds(); }

    void setDisplayName(const std::string& displayName);
    const std::string& getDisplayName() const { return mDisplayName; }

    /* ------------------------------------------------------------------------
     * Display power mode management.
     */
    int getPowerMode() const;
    void setPowerMode(int mode);
    bool isPoweredOn() const;

    ui::Dataspace getCompositionDataSpace() const;

    /* ------------------------------------------------------------------------
     * Display active config management.
     */
    int getActiveConfig() const;
    void setActiveConfig(int mode);

    // release HWC resources (if any) for removable displays
    void disconnect();

    /* ------------------------------------------------------------------------
     * Debugging
     */
    uint32_t getPageFlipCount() const;
    std::string getDebugName() const;
    void dump(std::string& result) const;

private:
    /*
     *  Constants, set during initialization
     */
    const sp<SurfaceFlinger> mFlinger;
    const wp<IBinder> mDisplayToken;
    const int32_t mSequenceId;

    const int mDisplayInstallOrientation;
    const std::shared_ptr<compositionengine::Display> mCompositionDisplay;

    std::string mDisplayName;
    const bool mIsVirtual;

    /*
     * Can only accessed from the main thread, these members
     * don't need synchronization.
     */

    // list of visible layers on that display
    Vector< sp<Layer> > mVisibleLayersSortedByZ;
    // list of layers needing fences
    Vector< sp<Layer> > mLayersNeedingFences;

    /*
     * Transaction state
     */
    static uint32_t displayStateOrientationToTransformOrientation(int orientation);
    static status_t orientationToTransfrom(int orientation,
                                           int w, int h, ui::Transform* tr);

    int mOrientation;
    static uint32_t sPrimaryDisplayOrientation;

    // Current power mode
    int mPowerMode;
    // Current active config
    int mActiveConfig;

    // TODO(b/74619554): Remove special cases for primary display.
    const bool mIsPrimary;
};

struct DisplayDeviceState {
    bool isVirtual() const { return !displayId.has_value(); }

    int32_t sequenceId = sNextSequenceId++;
    std::optional<DisplayId> displayId;
    sp<IGraphicBufferProducer> surface;
    uint32_t layerStack = DisplayDevice::NO_LAYER_STACK;
    Rect viewport;
    Rect frame;
    uint8_t orientation = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    std::string displayName;
    bool isSecure = false;

private:
    static std::atomic<int32_t> sNextSequenceId;
};

struct DisplayDeviceCreationArgs {
    // We use a constructor to ensure some of the values are set, without
    // assuming a default value.
    DisplayDeviceCreationArgs(const sp<SurfaceFlinger>& flinger, const wp<IBinder>& displayToken,
                              const std::optional<DisplayId>& displayId);

    const sp<SurfaceFlinger> flinger;
    const wp<IBinder> displayToken;
    const std::optional<DisplayId> displayId;

    int32_t sequenceId{0};
    bool isVirtual{false};
    bool isSecure{false};
    sp<ANativeWindow> nativeWindow;
    sp<compositionengine::DisplaySurface> displaySurface;
    int displayInstallOrientation{DisplayState::eOrientationDefault};
    bool hasWideColorGamut{false};
    HdrCapabilities hdrCapabilities;
    int32_t supportedPerFrameMetadata{0};
    std::unordered_map<ui::ColorMode, std::vector<ui::RenderIntent>> hwcColorModes;
    int initialPowerMode{HWC_POWER_MODE_NORMAL};
    bool isPrimary{false};
};

class DisplayRenderArea : public RenderArea {
public:
    DisplayRenderArea(const sp<const DisplayDevice> device,
                      ui::Transform::orientation_flags rotation = ui::Transform::ROT_0)
          : DisplayRenderArea(device, device->getBounds(), device->getWidth(), device->getHeight(),
                              device->getCompositionDataSpace(), rotation) {}
    DisplayRenderArea(const sp<const DisplayDevice> device, Rect sourceCrop, uint32_t reqWidth,
                      uint32_t reqHeight, ui::Dataspace reqDataSpace,
                      ui::Transform::orientation_flags rotation, bool allowSecureLayers = true)
          : RenderArea(reqWidth, reqHeight, CaptureFill::OPAQUE, reqDataSpace,
                       getDisplayRotation(rotation, device->getInstallOrientation())),
            mDevice(device),
            mSourceCrop(sourceCrop),
            mAllowSecureLayers(allowSecureLayers) {}

    const ui::Transform& getTransform() const override { return mDevice->getTransform(); }
    Rect getBounds() const override { return mDevice->getBounds(); }
    int getHeight() const override { return mDevice->getHeight(); }
    int getWidth() const override { return mDevice->getWidth(); }
    bool isSecure() const override { return mAllowSecureLayers && mDevice->isSecure(); }
    const sp<const DisplayDevice> getDisplayDevice() const override { return mDevice; }

    bool needsFiltering() const override {
        // check if the projection from the logical display to the physical
        // display needs filtering
        if (mDevice->needsFiltering()) {
            return true;
        }

        // check if the projection from the logical render area (i.e., the
        // physical display) to the physical render area requires filtering
        const Rect sourceCrop = getSourceCrop();
        int width = sourceCrop.width();
        int height = sourceCrop.height();
        if (getRotationFlags() & ui::Transform::ROT_90) {
            std::swap(width, height);
        }
        return width != getReqWidth() || height != getReqHeight();
    }

    Rect getSourceCrop() const override {
        // use the projected display viewport by default.
        if (mSourceCrop.isEmpty()) {
            return mDevice->getScissor();
        }

        // Recompute the device transformation for the source crop.
        ui::Transform rotation;
        ui::Transform translatePhysical;
        ui::Transform translateLogical;
        ui::Transform scale;
        const Rect& viewport = mDevice->getViewport();
        const Rect& scissor = mDevice->getScissor();
        const Rect& frame = mDevice->getFrame();

        const int orientation = mDevice->getInstallOrientation();
        // Install orientation is transparent to the callers.  Apply it now.
        uint32_t flags = 0x00;
        switch (orientation) {
            case DisplayState::eOrientation90:
                flags = ui::Transform::ROT_90;
                break;
            case DisplayState::eOrientation180:
                flags = ui::Transform::ROT_180;
                break;
            case DisplayState::eOrientation270:
                flags = ui::Transform::ROT_270;
                break;
            default:
                break;
        }
        rotation.set(flags, getWidth(), getHeight());
        translateLogical.set(-viewport.left, -viewport.top);
        translatePhysical.set(scissor.left, scissor.top);
        scale.set(frame.getWidth() / float(viewport.getWidth()), 0, 0,
                  frame.getHeight() / float(viewport.getHeight()));
        const ui::Transform finalTransform =
                rotation * translatePhysical * scale * translateLogical;
        return finalTransform.transform(mSourceCrop);
    }

private:
    // Install orientation is transparent to the callers.  We need to cancel
    // it out by modifying rotation flags.
    static ui::Transform::orientation_flags getDisplayRotation(
            ui::Transform::orientation_flags rotation, int orientation) {
        if (orientation == DisplayState::eOrientationDefault) {
            return rotation;
        }

        // convert hw orientation into flag presentation
        // here inverse transform needed
        uint8_t hw_rot_90 = 0x00;
        uint8_t hw_flip_hv = 0x00;
        switch (orientation) {
            case DisplayState::eOrientation90:
                hw_rot_90 = ui::Transform::ROT_90;
                hw_flip_hv = ui::Transform::ROT_180;
                break;
            case DisplayState::eOrientation180:
                hw_flip_hv = ui::Transform::ROT_180;
                break;
            case DisplayState::eOrientation270:
                hw_rot_90 = ui::Transform::ROT_90;
                break;
        }

        // transform flags operation
        // 1) flip H V if both have ROT_90 flag
        // 2) XOR these flags
        uint8_t rotation_rot_90 = rotation & ui::Transform::ROT_90;
        uint8_t rotation_flip_hv = rotation & ui::Transform::ROT_180;
        if (rotation_rot_90 & hw_rot_90) {
            rotation_flip_hv = (~rotation_flip_hv) & ui::Transform::ROT_180;
        }

        return static_cast<ui::Transform::orientation_flags>(
                (rotation_rot_90 ^ hw_rot_90) | (rotation_flip_hv ^ hw_flip_hv));
    }

    const sp<const DisplayDevice> mDevice;
    const Rect mSourceCrop;
    const bool mAllowSecureLayers;
};

}; // namespace android

#endif // ANDROID_DISPLAY_DEVICE_H
