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

// tag as surfaceflinger
#define LOG_TAG "SurfaceFlinger"

#include <stdint.h>
#include <sys/types.h>

#include <binder/Parcel.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>

#include <gui/IDisplayEventConnection.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/IRegionSamplingListener.h>
#include <gui/ISurfaceComposer.h>
#include <gui/ISurfaceComposerClient.h>
#include <gui/LayerDebugInfo.h>
#include <gui/LayerState.h>

#include <system/graphics.h>

#include <ui/DisplayInfo.h>
#include <ui/DisplayStatInfo.h>
#include <ui/HdrCapabilities.h>

#include <utils/Log.h>

// ---------------------------------------------------------------------------

namespace android {

using ui::ColorMode;

class BpSurfaceComposer : public BpInterface<ISurfaceComposer>
{
public:
    explicit BpSurfaceComposer(const sp<IBinder>& impl)
        : BpInterface<ISurfaceComposer>(impl)
    {
    }

    virtual ~BpSurfaceComposer();

    virtual sp<ISurfaceComposerClient> createConnection()
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        remote()->transact(BnSurfaceComposer::CREATE_CONNECTION, data, &reply);
        return interface_cast<ISurfaceComposerClient>(reply.readStrongBinder());
    }

    virtual void setTransactionState(const Vector<ComposerState>& state,
                                     const Vector<DisplayState>& displays, uint32_t flags,
                                     const sp<IBinder>& applyToken,
                                     const InputWindowCommands& commands,
                                     int64_t desiredPresentTime,
                                     const client_cache_t& uncacheBuffer,
                                     const std::vector<ListenerCallbacks>& listenerCallbacks) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());

        data.writeUint32(static_cast<uint32_t>(state.size()));
        for (const auto& s : state) {
            s.write(data);
        }

        data.writeUint32(static_cast<uint32_t>(displays.size()));
        for (const auto& d : displays) {
            d.write(data);
        }

        data.writeUint32(flags);
        data.writeStrongBinder(applyToken);
        commands.write(data);
        data.writeInt64(desiredPresentTime);
        data.writeWeakBinder(uncacheBuffer.token);
        data.writeUint64(uncacheBuffer.id);

        if (data.writeVectorSize(listenerCallbacks) == NO_ERROR) {
            for (const auto& [listener, callbackIds] : listenerCallbacks) {
                data.writeStrongBinder(IInterface::asBinder(listener));
                data.writeInt64Vector(callbackIds);
            }
        }

        remote()->transact(BnSurfaceComposer::SET_TRANSACTION_STATE, data, &reply);
    }

    virtual void bootFinished()
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        remote()->transact(BnSurfaceComposer::BOOT_FINISHED, data, &reply);
    }

    virtual status_t captureScreen(const sp<IBinder>& display, sp<GraphicBuffer>* outBuffer,
                                   bool& outCapturedSecureLayers, const ui::Dataspace reqDataspace,
                                   const ui::PixelFormat reqPixelFormat, Rect sourceCrop,
                                   uint32_t reqWidth, uint32_t reqHeight, bool useIdentityTransform,
                                   ISurfaceComposer::Rotation rotation, bool captureSecureLayers) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        data.writeStrongBinder(display);
        data.writeInt32(static_cast<int32_t>(reqDataspace));
        data.writeInt32(static_cast<int32_t>(reqPixelFormat));
        data.write(sourceCrop);
        data.writeUint32(reqWidth);
        data.writeUint32(reqHeight);
        data.writeInt32(static_cast<int32_t>(useIdentityTransform));
        data.writeInt32(static_cast<int32_t>(rotation));
        data.writeInt32(static_cast<int32_t>(captureSecureLayers));
        status_t result = remote()->transact(BnSurfaceComposer::CAPTURE_SCREEN, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("captureScreen failed to transact: %d", result);
            return result;
        }
        result = reply.readInt32();
        if (result != NO_ERROR) {
            ALOGE("captureScreen failed to readInt32: %d", result);
            return result;
        }

        *outBuffer = new GraphicBuffer();
        reply.read(**outBuffer);
        outCapturedSecureLayers = reply.readBool();

        return result;
    }

    virtual status_t captureScreen(uint64_t displayOrLayerStack, ui::Dataspace* outDataspace,
                                   sp<GraphicBuffer>* outBuffer) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        data.writeUint64(displayOrLayerStack);
        status_t result = remote()->transact(BnSurfaceComposer::CAPTURE_SCREEN_BY_ID, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("captureScreen failed to transact: %d", result);
            return result;
        }
        result = reply.readInt32();
        if (result != NO_ERROR) {
            ALOGE("captureScreen failed to readInt32: %d", result);
            return result;
        }

        *outDataspace = static_cast<ui::Dataspace>(reply.readInt32());
        *outBuffer = new GraphicBuffer();
        reply.read(**outBuffer);
        return result;
    }

    virtual status_t captureLayers(
            const sp<IBinder>& layerHandleBinder, sp<GraphicBuffer>* outBuffer,
            const ui::Dataspace reqDataspace, const ui::PixelFormat reqPixelFormat,
            const Rect& sourceCrop,
            const std::unordered_set<sp<IBinder>, SpHash<IBinder>>& excludeLayers, float frameScale,
            bool childrenOnly) {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        data.writeStrongBinder(layerHandleBinder);
        data.writeInt32(static_cast<int32_t>(reqDataspace));
        data.writeInt32(static_cast<int32_t>(reqPixelFormat));
        data.write(sourceCrop);
        data.writeInt32(excludeLayers.size());
        for (auto el : excludeLayers) {
            data.writeStrongBinder(el);
        }
        data.writeFloat(frameScale);
        data.writeBool(childrenOnly);
        status_t result = remote()->transact(BnSurfaceComposer::CAPTURE_LAYERS, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("captureLayers failed to transact: %d", result);
            return result;
        }
        result = reply.readInt32();
        if (result != NO_ERROR) {
            ALOGE("captureLayers failed to readInt32: %d", result);
            return result;
        }

        *outBuffer = new GraphicBuffer();
        reply.read(**outBuffer);

        return result;
    }

    virtual bool authenticateSurfaceTexture(
            const sp<IGraphicBufferProducer>& bufferProducer) const
    {
        Parcel data, reply;
        int err = NO_ERROR;
        err = data.writeInterfaceToken(
                ISurfaceComposer::getInterfaceDescriptor());
        if (err != NO_ERROR) {
            ALOGE("ISurfaceComposer::authenticateSurfaceTexture: error writing "
                    "interface descriptor: %s (%d)", strerror(-err), -err);
            return false;
        }
        err = data.writeStrongBinder(IInterface::asBinder(bufferProducer));
        if (err != NO_ERROR) {
            ALOGE("ISurfaceComposer::authenticateSurfaceTexture: error writing "
                    "strong binder to parcel: %s (%d)", strerror(-err), -err);
            return false;
        }
        err = remote()->transact(BnSurfaceComposer::AUTHENTICATE_SURFACE, data,
                &reply);
        if (err != NO_ERROR) {
            ALOGE("ISurfaceComposer::authenticateSurfaceTexture: error "
                    "performing transaction: %s (%d)", strerror(-err), -err);
            return false;
        }
        int32_t result = 0;
        err = reply.readInt32(&result);
        if (err != NO_ERROR) {
            ALOGE("ISurfaceComposer::authenticateSurfaceTexture: error "
                    "retrieving result: %s (%d)", strerror(-err), -err);
            return false;
        }
        return result != 0;
    }

    virtual status_t getSupportedFrameTimestamps(
            std::vector<FrameEvent>* outSupported) const {
        if (!outSupported) {
            return UNEXPECTED_NULL;
        }
        outSupported->clear();

        Parcel data, reply;

        status_t err = data.writeInterfaceToken(
                ISurfaceComposer::getInterfaceDescriptor());
        if (err != NO_ERROR) {
            return err;
        }

        err = remote()->transact(
                BnSurfaceComposer::GET_SUPPORTED_FRAME_TIMESTAMPS,
                data, &reply);
        if (err != NO_ERROR) {
            return err;
        }

        int32_t result = 0;
        err = reply.readInt32(&result);
        if (err != NO_ERROR) {
            return err;
        }
        if (result != NO_ERROR) {
            return result;
        }

        std::vector<int32_t> supported;
        err = reply.readInt32Vector(&supported);
        if (err != NO_ERROR) {
            return err;
        }

        outSupported->reserve(supported.size());
        for (int32_t s : supported) {
            outSupported->push_back(static_cast<FrameEvent>(s));
        }
        return NO_ERROR;
    }

    virtual sp<IDisplayEventConnection> createDisplayEventConnection(VsyncSource vsyncSource,
                                                                     ConfigChanged configChanged) {
        Parcel data, reply;
        sp<IDisplayEventConnection> result;
        int err = data.writeInterfaceToken(
                ISurfaceComposer::getInterfaceDescriptor());
        if (err != NO_ERROR) {
            return result;
        }
        data.writeInt32(static_cast<int32_t>(vsyncSource));
        data.writeInt32(static_cast<int32_t>(configChanged));
        err = remote()->transact(
                BnSurfaceComposer::CREATE_DISPLAY_EVENT_CONNECTION,
                data, &reply);
        if (err != NO_ERROR) {
            ALOGE("ISurfaceComposer::createDisplayEventConnection: error performing "
                    "transaction: %s (%d)", strerror(-err), -err);
            return result;
        }
        result = interface_cast<IDisplayEventConnection>(reply.readStrongBinder());
        return result;
    }

    virtual sp<IBinder> createDisplay(const String8& displayName, bool secure)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        data.writeString8(displayName);
        data.writeInt32(secure ? 1 : 0);
        remote()->transact(BnSurfaceComposer::CREATE_DISPLAY, data, &reply);
        return reply.readStrongBinder();
    }

    virtual void destroyDisplay(const sp<IBinder>& display)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        data.writeStrongBinder(display);
        remote()->transact(BnSurfaceComposer::DESTROY_DISPLAY, data, &reply);
    }

    virtual std::vector<PhysicalDisplayId> getPhysicalDisplayIds() const {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (remote()->transact(BnSurfaceComposer::GET_PHYSICAL_DISPLAY_IDS, data, &reply) ==
            NO_ERROR) {
            std::vector<PhysicalDisplayId> displayIds;
            if (reply.readUint64Vector(&displayIds) == NO_ERROR) {
                return displayIds;
            }
        }

        return {};
    }

    virtual sp<IBinder> getPhysicalDisplayToken(PhysicalDisplayId displayId) const {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        data.writeUint64(displayId);
        remote()->transact(BnSurfaceComposer::GET_PHYSICAL_DISPLAY_TOKEN, data, &reply);
        return reply.readStrongBinder();
    }

    virtual void setPowerMode(const sp<IBinder>& display, int mode)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        data.writeStrongBinder(display);
        data.writeInt32(mode);
        remote()->transact(BnSurfaceComposer::SET_POWER_MODE, data, &reply);
    }

    virtual status_t getDisplayConfigs(const sp<IBinder>& display,
            Vector<DisplayInfo>* configs)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        data.writeStrongBinder(display);
        remote()->transact(BnSurfaceComposer::GET_DISPLAY_CONFIGS, data, &reply);
        status_t result = reply.readInt32();
        if (result == NO_ERROR) {
            size_t numConfigs = reply.readUint32();
            configs->clear();
            configs->resize(numConfigs);
            for (size_t c = 0; c < numConfigs; ++c) {
                memcpy(&(configs->editItemAt(c)),
                        reply.readInplace(sizeof(DisplayInfo)),
                        sizeof(DisplayInfo));
            }
        }
        return result;
    }

    virtual status_t getDisplayStats(const sp<IBinder>& display,
            DisplayStatInfo* stats)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        data.writeStrongBinder(display);
        remote()->transact(BnSurfaceComposer::GET_DISPLAY_STATS, data, &reply);
        status_t result = reply.readInt32();
        if (result == NO_ERROR) {
            memcpy(stats,
                    reply.readInplace(sizeof(DisplayStatInfo)),
                    sizeof(DisplayStatInfo));
        }
        return result;
    }

    virtual int getActiveConfig(const sp<IBinder>& display)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        data.writeStrongBinder(display);
        remote()->transact(BnSurfaceComposer::GET_ACTIVE_CONFIG, data, &reply);
        return reply.readInt32();
    }

    virtual status_t setActiveConfig(const sp<IBinder>& display, int id)
    {
        Parcel data, reply;
        status_t result = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (result != NO_ERROR) {
            ALOGE("setActiveConfig failed to writeInterfaceToken: %d", result);
            return result;
        }
        result = data.writeStrongBinder(display);
        if (result != NO_ERROR) {
            ALOGE("setActiveConfig failed to writeStrongBinder: %d", result);
            return result;
        }
        result = data.writeInt32(id);
        if (result != NO_ERROR) {
            ALOGE("setActiveConfig failed to writeInt32: %d", result);
            return result;
        }
        result = remote()->transact(BnSurfaceComposer::SET_ACTIVE_CONFIG, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("setActiveConfig failed to transact: %d", result);
            return result;
        }
        return reply.readInt32();
    }

    virtual status_t getDisplayColorModes(const sp<IBinder>& display,
            Vector<ColorMode>* outColorModes) {
        Parcel data, reply;
        status_t result = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (result != NO_ERROR) {
            ALOGE("getDisplayColorModes failed to writeInterfaceToken: %d", result);
            return result;
        }
        result = data.writeStrongBinder(display);
        if (result != NO_ERROR) {
            ALOGE("getDisplayColorModes failed to writeStrongBinder: %d", result);
            return result;
        }
        result = remote()->transact(BnSurfaceComposer::GET_DISPLAY_COLOR_MODES, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("getDisplayColorModes failed to transact: %d", result);
            return result;
        }
        result = static_cast<status_t>(reply.readInt32());
        if (result == NO_ERROR) {
            size_t numModes = reply.readUint32();
            outColorModes->clear();
            outColorModes->resize(numModes);
            for (size_t i = 0; i < numModes; ++i) {
                outColorModes->replaceAt(static_cast<ColorMode>(reply.readInt32()), i);
            }
        }
        return result;
    }

    virtual status_t getDisplayNativePrimaries(const sp<IBinder>& display,
            ui::DisplayPrimaries& primaries) {
        Parcel data, reply;
        status_t result = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (result != NO_ERROR) {
            ALOGE("getDisplayNativePrimaries failed to writeInterfaceToken: %d", result);
            return result;
        }
        result = data.writeStrongBinder(display);
        if (result != NO_ERROR) {
            ALOGE("getDisplayNativePrimaries failed to writeStrongBinder: %d", result);
            return result;
        }
        result = remote()->transact(BnSurfaceComposer::GET_DISPLAY_NATIVE_PRIMARIES, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("getDisplayNativePrimaries failed to transact: %d", result);
            return result;
        }
        result = reply.readInt32();
        if (result == NO_ERROR) {
            memcpy(&primaries, reply.readInplace(sizeof(ui::DisplayPrimaries)),
                    sizeof(ui::DisplayPrimaries));
        }
        return result;
    }

    virtual ColorMode getActiveColorMode(const sp<IBinder>& display) {
        Parcel data, reply;
        status_t result = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (result != NO_ERROR) {
            ALOGE("getActiveColorMode failed to writeInterfaceToken: %d", result);
            return static_cast<ColorMode>(result);
        }
        result = data.writeStrongBinder(display);
        if (result != NO_ERROR) {
            ALOGE("getActiveColorMode failed to writeStrongBinder: %d", result);
            return static_cast<ColorMode>(result);
        }
        result = remote()->transact(BnSurfaceComposer::GET_ACTIVE_COLOR_MODE, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("getActiveColorMode failed to transact: %d", result);
            return static_cast<ColorMode>(result);
        }
        return static_cast<ColorMode>(reply.readInt32());
    }

    virtual status_t setActiveColorMode(const sp<IBinder>& display,
            ColorMode colorMode) {
        Parcel data, reply;
        status_t result = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (result != NO_ERROR) {
            ALOGE("setActiveColorMode failed to writeInterfaceToken: %d", result);
            return result;
        }
        result = data.writeStrongBinder(display);
        if (result != NO_ERROR) {
            ALOGE("setActiveColorMode failed to writeStrongBinder: %d", result);
            return result;
        }
        result = data.writeInt32(static_cast<int32_t>(colorMode));
        if (result != NO_ERROR) {
            ALOGE("setActiveColorMode failed to writeInt32: %d", result);
            return result;
        }
        result = remote()->transact(BnSurfaceComposer::SET_ACTIVE_COLOR_MODE, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("setActiveColorMode failed to transact: %d", result);
            return result;
        }
        return static_cast<status_t>(reply.readInt32());
    }

    virtual status_t clearAnimationFrameStats() {
        Parcel data, reply;
        status_t result = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (result != NO_ERROR) {
            ALOGE("clearAnimationFrameStats failed to writeInterfaceToken: %d", result);
            return result;
        }
        result = remote()->transact(BnSurfaceComposer::CLEAR_ANIMATION_FRAME_STATS, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("clearAnimationFrameStats failed to transact: %d", result);
            return result;
        }
        return reply.readInt32();
    }

    virtual status_t getAnimationFrameStats(FrameStats* outStats) const {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        remote()->transact(BnSurfaceComposer::GET_ANIMATION_FRAME_STATS, data, &reply);
        reply.read(*outStats);
        return reply.readInt32();
    }

    virtual status_t getHdrCapabilities(const sp<IBinder>& display,
            HdrCapabilities* outCapabilities) const {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        status_t result = data.writeStrongBinder(display);
        if (result != NO_ERROR) {
            ALOGE("getHdrCapabilities failed to writeStrongBinder: %d", result);
            return result;
        }
        result = remote()->transact(BnSurfaceComposer::GET_HDR_CAPABILITIES,
                data, &reply);
        if (result != NO_ERROR) {
            ALOGE("getHdrCapabilities failed to transact: %d", result);
            return result;
        }
        result = reply.readInt32();
        if (result == NO_ERROR) {
            result = reply.read(*outCapabilities);
        }
        return result;
    }

    virtual status_t enableVSyncInjections(bool enable) {
        Parcel data, reply;
        status_t result = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (result != NO_ERROR) {
            ALOGE("enableVSyncInjections failed to writeInterfaceToken: %d", result);
            return result;
        }
        result = data.writeBool(enable);
        if (result != NO_ERROR) {
            ALOGE("enableVSyncInjections failed to writeBool: %d", result);
            return result;
        }
        result = remote()->transact(BnSurfaceComposer::ENABLE_VSYNC_INJECTIONS, data, &reply,
                                    IBinder::FLAG_ONEWAY);
        if (result != NO_ERROR) {
            ALOGE("enableVSyncInjections failed to transact: %d", result);
            return result;
        }
        return result;
    }

    virtual status_t injectVSync(nsecs_t when) {
        Parcel data, reply;
        status_t result = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (result != NO_ERROR) {
            ALOGE("injectVSync failed to writeInterfaceToken: %d", result);
            return result;
        }
        result = data.writeInt64(when);
        if (result != NO_ERROR) {
            ALOGE("injectVSync failed to writeInt64: %d", result);
            return result;
        }
        result = remote()->transact(BnSurfaceComposer::INJECT_VSYNC, data, &reply,
                                    IBinder::FLAG_ONEWAY);
        if (result != NO_ERROR) {
            ALOGE("injectVSync failed to transact: %d", result);
            return result;
        }
        return result;
    }

    virtual status_t getLayerDebugInfo(std::vector<LayerDebugInfo>* outLayers) const
    {
        if (!outLayers) {
            return UNEXPECTED_NULL;
        }

        Parcel data, reply;

        status_t err = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (err != NO_ERROR) {
            return err;
        }

        err = remote()->transact(BnSurfaceComposer::GET_LAYER_DEBUG_INFO, data, &reply);
        if (err != NO_ERROR) {
            return err;
        }

        int32_t result = 0;
        err = reply.readInt32(&result);
        if (err != NO_ERROR) {
            return err;
        }
        if (result != NO_ERROR) {
            return result;
        }

        outLayers->clear();
        return reply.readParcelableVector(outLayers);
    }

    virtual status_t getCompositionPreference(ui::Dataspace* defaultDataspace,
                                              ui::PixelFormat* defaultPixelFormat,
                                              ui::Dataspace* wideColorGamutDataspace,
                                              ui::PixelFormat* wideColorGamutPixelFormat) const {
        Parcel data, reply;
        status_t error = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (error != NO_ERROR) {
            return error;
        }
        error = remote()->transact(BnSurfaceComposer::GET_COMPOSITION_PREFERENCE, data, &reply);
        if (error != NO_ERROR) {
            return error;
        }
        error = static_cast<status_t>(reply.readInt32());
        if (error == NO_ERROR) {
            *defaultDataspace = static_cast<ui::Dataspace>(reply.readInt32());
            *defaultPixelFormat = static_cast<ui::PixelFormat>(reply.readInt32());
            *wideColorGamutDataspace = static_cast<ui::Dataspace>(reply.readInt32());
            *wideColorGamutPixelFormat = static_cast<ui::PixelFormat>(reply.readInt32());
        }
        return error;
    }

    virtual status_t getColorManagement(bool* outGetColorManagement) const {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        remote()->transact(BnSurfaceComposer::GET_COLOR_MANAGEMENT, data, &reply);
        bool result;
        status_t err = reply.readBool(&result);
        if (err == NO_ERROR) {
            *outGetColorManagement = result;
        }
        return err;
    }

    virtual status_t getDisplayedContentSamplingAttributes(const sp<IBinder>& display,
                                                           ui::PixelFormat* outFormat,
                                                           ui::Dataspace* outDataspace,
                                                           uint8_t* outComponentMask) const {
        if (!outFormat || !outDataspace || !outComponentMask) return BAD_VALUE;
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        data.writeStrongBinder(display);

        status_t error =
                remote()->transact(BnSurfaceComposer::GET_DISPLAYED_CONTENT_SAMPLING_ATTRIBUTES,
                                   data, &reply);
        if (error != NO_ERROR) {
            return error;
        }

        uint32_t value = 0;
        error = reply.readUint32(&value);
        if (error != NO_ERROR) {
            return error;
        }
        *outFormat = static_cast<ui::PixelFormat>(value);

        error = reply.readUint32(&value);
        if (error != NO_ERROR) {
            return error;
        }
        *outDataspace = static_cast<ui::Dataspace>(value);

        error = reply.readUint32(&value);
        if (error != NO_ERROR) {
            return error;
        }
        *outComponentMask = static_cast<uint8_t>(value);
        return error;
    }

    virtual status_t setDisplayContentSamplingEnabled(const sp<IBinder>& display, bool enable,
                                                      uint8_t componentMask,
                                                      uint64_t maxFrames) const {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        data.writeStrongBinder(display);
        data.writeBool(enable);
        data.writeByte(static_cast<int8_t>(componentMask));
        data.writeUint64(maxFrames);
        status_t result =
                remote()->transact(BnSurfaceComposer::SET_DISPLAY_CONTENT_SAMPLING_ENABLED, data,
                                   &reply);
        return result;
    }

    virtual status_t getDisplayedContentSample(const sp<IBinder>& display, uint64_t maxFrames,
                                               uint64_t timestamp,
                                               DisplayedFrameStats* outStats) const {
        if (!outStats) return BAD_VALUE;

        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        data.writeStrongBinder(display);
        data.writeUint64(maxFrames);
        data.writeUint64(timestamp);

        status_t result =
                remote()->transact(BnSurfaceComposer::GET_DISPLAYED_CONTENT_SAMPLE, data, &reply);

        if (result != NO_ERROR) {
            return result;
        }

        result = reply.readUint64(&outStats->numFrames);
        if (result != NO_ERROR) {
            return result;
        }

        result = reply.readUint64Vector(&outStats->component_0_sample);
        if (result != NO_ERROR) {
            return result;
        }
        result = reply.readUint64Vector(&outStats->component_1_sample);
        if (result != NO_ERROR) {
            return result;
        }
        result = reply.readUint64Vector(&outStats->component_2_sample);
        if (result != NO_ERROR) {
            return result;
        }
        result = reply.readUint64Vector(&outStats->component_3_sample);
        return result;
    }

    virtual status_t getProtectedContentSupport(bool* outSupported) const {
        Parcel data, reply;
        data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        status_t error =
                remote()->transact(BnSurfaceComposer::GET_PROTECTED_CONTENT_SUPPORT, data, &reply);
        if (error != NO_ERROR) {
            return error;
        }
        error = reply.readBool(outSupported);
        return error;
    }

    virtual status_t isWideColorDisplay(const sp<IBinder>& token,
                                        bool* outIsWideColorDisplay) const {
        Parcel data, reply;
        status_t error = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (error != NO_ERROR) {
            return error;
        }
        error = data.writeStrongBinder(token);
        if (error != NO_ERROR) {
            return error;
        }

        error = remote()->transact(BnSurfaceComposer::IS_WIDE_COLOR_DISPLAY, data, &reply);
        if (error != NO_ERROR) {
            return error;
        }
        error = reply.readBool(outIsWideColorDisplay);
        return error;
    }

    virtual status_t addRegionSamplingListener(const Rect& samplingArea,
                                               const sp<IBinder>& stopLayerHandle,
                                               const sp<IRegionSamplingListener>& listener) {
        Parcel data, reply;
        status_t error = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (error != NO_ERROR) {
            ALOGE("addRegionSamplingListener: Failed to write interface token");
            return error;
        }
        error = data.write(samplingArea);
        if (error != NO_ERROR) {
            ALOGE("addRegionSamplingListener: Failed to write sampling area");
            return error;
        }
        error = data.writeStrongBinder(stopLayerHandle);
        if (error != NO_ERROR) {
            ALOGE("addRegionSamplingListener: Failed to write stop layer handle");
            return error;
        }
        error = data.writeStrongBinder(IInterface::asBinder(listener));
        if (error != NO_ERROR) {
            ALOGE("addRegionSamplingListener: Failed to write listener");
            return error;
        }
        error = remote()->transact(BnSurfaceComposer::ADD_REGION_SAMPLING_LISTENER, data, &reply);
        if (error != NO_ERROR) {
            ALOGE("addRegionSamplingListener: Failed to transact");
        }
        return error;
    }

    virtual status_t removeRegionSamplingListener(const sp<IRegionSamplingListener>& listener) {
        Parcel data, reply;
        status_t error = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (error != NO_ERROR) {
            ALOGE("removeRegionSamplingListener: Failed to write interface token");
            return error;
        }
        error = data.writeStrongBinder(IInterface::asBinder(listener));
        if (error != NO_ERROR) {
            ALOGE("removeRegionSamplingListener: Failed to write listener");
            return error;
        }
        error = remote()->transact(BnSurfaceComposer::REMOVE_REGION_SAMPLING_LISTENER, data,
                                   &reply);
        if (error != NO_ERROR) {
            ALOGE("removeRegionSamplingListener: Failed to transact");
        }
        return error;
    }

    virtual status_t setAllowedDisplayConfigs(const sp<IBinder>& displayToken,
                                              const std::vector<int32_t>& allowedConfigs) {
        Parcel data, reply;
        status_t result = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (result != NO_ERROR) {
            ALOGE("setAllowedDisplayConfigs failed to writeInterfaceToken: %d", result);
            return result;
        }
        result = data.writeStrongBinder(displayToken);
        if (result != NO_ERROR) {
            ALOGE("setAllowedDisplayConfigs failed to writeStrongBinder: %d", result);
            return result;
        }
        result = data.writeInt32Vector(allowedConfigs);
        if (result != NO_ERROR) {
            ALOGE("setAllowedDisplayConfigs failed to writeInt32Vector: %d", result);
            return result;
        }
        result = remote()->transact(BnSurfaceComposer::SET_ALLOWED_DISPLAY_CONFIGS, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("setAllowedDisplayConfigs failed to transact: %d", result);
            return result;
        }
        return reply.readInt32();
    }

    virtual status_t getAllowedDisplayConfigs(const sp<IBinder>& displayToken,
                                              std::vector<int32_t>* outAllowedConfigs) {
        if (!outAllowedConfigs) return BAD_VALUE;
        Parcel data, reply;
        status_t result = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (result != NO_ERROR) {
            ALOGE("getAllowedDisplayConfigs failed to writeInterfaceToken: %d", result);
            return result;
        }
        result = data.writeStrongBinder(displayToken);
        if (result != NO_ERROR) {
            ALOGE("getAllowedDisplayConfigs failed to writeStrongBinder: %d", result);
            return result;
        }
        result = remote()->transact(BnSurfaceComposer::GET_ALLOWED_DISPLAY_CONFIGS, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("getAllowedDisplayConfigs failed to transact: %d", result);
            return result;
        }
        result = reply.readInt32Vector(outAllowedConfigs);
        if (result != NO_ERROR) {
            ALOGE("getAllowedDisplayConfigs failed to readInt32Vector: %d", result);
            return result;
        }
        return reply.readInt32();
    }

    virtual status_t getDisplayBrightnessSupport(const sp<IBinder>& displayToken,
                                                 bool* outSupport) const {
        Parcel data, reply;
        status_t error = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (error != NO_ERROR) {
            ALOGE("getDisplayBrightnessSupport: failed to write interface token: %d", error);
            return error;
        }
        error = data.writeStrongBinder(displayToken);
        if (error != NO_ERROR) {
            ALOGE("getDisplayBrightnessSupport: failed to write display token: %d", error);
            return error;
        }
        error = remote()->transact(BnSurfaceComposer::GET_DISPLAY_BRIGHTNESS_SUPPORT, data, &reply);
        if (error != NO_ERROR) {
            ALOGE("getDisplayBrightnessSupport: failed to transact: %d", error);
            return error;
        }
        bool support;
        error = reply.readBool(&support);
        if (error != NO_ERROR) {
            ALOGE("getDisplayBrightnessSupport: failed to read support: %d", error);
            return error;
        }
        *outSupport = support;
        return NO_ERROR;
    }

    virtual status_t setDisplayBrightness(const sp<IBinder>& displayToken, float brightness) const {
        Parcel data, reply;
        status_t error = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (error != NO_ERROR) {
            ALOGE("setDisplayBrightness: failed to write interface token: %d", error);
            return error;
        }
        error = data.writeStrongBinder(displayToken);
        if (error != NO_ERROR) {
            ALOGE("setDisplayBrightness: failed to write display token: %d", error);
            return error;
        }
        error = data.writeFloat(brightness);
        if (error != NO_ERROR) {
            ALOGE("setDisplayBrightness: failed to write brightness: %d", error);
            return error;
        }
        error = remote()->transact(BnSurfaceComposer::SET_DISPLAY_BRIGHTNESS, data, &reply);
        if (error != NO_ERROR) {
            ALOGE("setDisplayBrightness: failed to transact: %d", error);
            return error;
        }
        return NO_ERROR;
    }

    virtual status_t notifyPowerHint(int32_t hintId) {
        Parcel data, reply;
        status_t error = data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
        if (error != NO_ERROR) {
            ALOGE("notifyPowerHint: failed to write interface token: %d", error);
            return error;
        }
        error = data.writeInt32(hintId);
        if (error != NO_ERROR) {
            ALOGE("notifyPowerHint: failed to write hintId: %d", error);
            return error;
        }
        error = remote()->transact(BnSurfaceComposer::NOTIFY_POWER_HINT, data, &reply,
                                   IBinder::FLAG_ONEWAY);
        if (error != NO_ERROR) {
            ALOGE("notifyPowerHint: failed to transact: %d", error);
            return error;
        }
        return NO_ERROR;
    }
};

// Out-of-line virtual method definition to trigger vtable emission in this
// translation unit (see clang warning -Wweak-vtables)
BpSurfaceComposer::~BpSurfaceComposer() {}

IMPLEMENT_META_INTERFACE(SurfaceComposer, "android.ui.ISurfaceComposer");

// ----------------------------------------------------------------------

status_t BnSurfaceComposer::onTransact(
    uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code) {
        case CREATE_CONNECTION: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> b = IInterface::asBinder(createConnection());
            reply->writeStrongBinder(b);
            return NO_ERROR;
        }
        case SET_TRANSACTION_STATE: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);

            size_t count = data.readUint32();
            if (count > data.dataSize()) {
                return BAD_VALUE;
            }
            Vector<ComposerState> state;
            state.setCapacity(count);
            for (size_t i = 0; i < count; i++) {
                ComposerState s;
                if (s.read(data) == BAD_VALUE) {
                    return BAD_VALUE;
                }
                state.add(s);
            }

            count = data.readUint32();
            if (count > data.dataSize()) {
                return BAD_VALUE;
            }
            DisplayState d;
            Vector<DisplayState> displays;
            displays.setCapacity(count);
            for (size_t i = 0; i < count; i++) {
                if (d.read(data) == BAD_VALUE) {
                    return BAD_VALUE;
                }
                displays.add(d);
            }

            uint32_t stateFlags = data.readUint32();
            sp<IBinder> applyToken = data.readStrongBinder();
            InputWindowCommands inputWindowCommands;
            inputWindowCommands.read(data);

            int64_t desiredPresentTime = data.readInt64();

            client_cache_t uncachedBuffer;
            uncachedBuffer.token = data.readWeakBinder();
            uncachedBuffer.id = data.readUint64();

            std::vector<ListenerCallbacks> listenerCallbacks;
            int32_t listenersSize = data.readInt32();
            for (int32_t i = 0; i < listenersSize; i++) {
                auto listener =
                        interface_cast<ITransactionCompletedListener>(data.readStrongBinder());
                std::vector<CallbackId> callbackIds;
                data.readInt64Vector(&callbackIds);
                listenerCallbacks.emplace_back(listener, callbackIds);
            }

            setTransactionState(state, displays, stateFlags, applyToken, inputWindowCommands,
                                desiredPresentTime, uncachedBuffer, listenerCallbacks);
            return NO_ERROR;
        }
        case BOOT_FINISHED: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            bootFinished();
            return NO_ERROR;
        }
        case CAPTURE_SCREEN: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> display = data.readStrongBinder();
            ui::Dataspace reqDataspace = static_cast<ui::Dataspace>(data.readInt32());
            ui::PixelFormat reqPixelFormat = static_cast<ui::PixelFormat>(data.readInt32());
            sp<GraphicBuffer> outBuffer;
            Rect sourceCrop(Rect::EMPTY_RECT);
            data.read(sourceCrop);
            uint32_t reqWidth = data.readUint32();
            uint32_t reqHeight = data.readUint32();
            bool useIdentityTransform = static_cast<bool>(data.readInt32());
            int32_t rotation = data.readInt32();
            bool captureSecureLayers = static_cast<bool>(data.readInt32());

            bool capturedSecureLayers = false;
            status_t res = captureScreen(display, &outBuffer, capturedSecureLayers, reqDataspace,
                                         reqPixelFormat, sourceCrop, reqWidth, reqHeight,
                                         useIdentityTransform,
                                         static_cast<ISurfaceComposer::Rotation>(rotation),
                                         captureSecureLayers);

            reply->writeInt32(res);
            if (res == NO_ERROR) {
                reply->write(*outBuffer);
                reply->writeBool(capturedSecureLayers);
            }
            return NO_ERROR;
        }
        case CAPTURE_SCREEN_BY_ID: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            uint64_t displayOrLayerStack = data.readUint64();
            ui::Dataspace outDataspace = ui::Dataspace::V0_SRGB;
            sp<GraphicBuffer> outBuffer;
            status_t res = captureScreen(displayOrLayerStack, &outDataspace, &outBuffer);
            reply->writeInt32(res);
            if (res == NO_ERROR) {
                reply->writeInt32(static_cast<int32_t>(outDataspace));
                reply->write(*outBuffer);
            }
            return NO_ERROR;
        }
        case CAPTURE_LAYERS: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> layerHandleBinder = data.readStrongBinder();
            ui::Dataspace reqDataspace = static_cast<ui::Dataspace>(data.readInt32());
            ui::PixelFormat reqPixelFormat = static_cast<ui::PixelFormat>(data.readInt32());
            sp<GraphicBuffer> outBuffer;
            Rect sourceCrop(Rect::EMPTY_RECT);
            data.read(sourceCrop);

            std::unordered_set<sp<IBinder>, SpHash<IBinder>> excludeHandles;
            int numExcludeHandles = data.readInt32();
            excludeHandles.reserve(numExcludeHandles);
            for (int i = 0; i < numExcludeHandles; i++) {
                excludeHandles.emplace(data.readStrongBinder());
            }

            float frameScale = data.readFloat();
            bool childrenOnly = data.readBool();

            status_t res =
                    captureLayers(layerHandleBinder, &outBuffer, reqDataspace, reqPixelFormat,
                                  sourceCrop, excludeHandles, frameScale, childrenOnly);
            reply->writeInt32(res);
            if (res == NO_ERROR) {
                reply->write(*outBuffer);
            }
            return NO_ERROR;
        }
        case AUTHENTICATE_SURFACE: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IGraphicBufferProducer> bufferProducer =
                    interface_cast<IGraphicBufferProducer>(data.readStrongBinder());
            int32_t result = authenticateSurfaceTexture(bufferProducer) ? 1 : 0;
            reply->writeInt32(result);
            return NO_ERROR;
        }
        case GET_SUPPORTED_FRAME_TIMESTAMPS: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            std::vector<FrameEvent> supportedTimestamps;
            status_t result = getSupportedFrameTimestamps(&supportedTimestamps);
            status_t err = reply->writeInt32(result);
            if (err != NO_ERROR) {
                return err;
            }
            if (result != NO_ERROR) {
                return result;
            }

            std::vector<int32_t> supported;
            supported.reserve(supportedTimestamps.size());
            for (FrameEvent s : supportedTimestamps) {
                supported.push_back(static_cast<int32_t>(s));
            }
            return reply->writeInt32Vector(supported);
        }
        case CREATE_DISPLAY_EVENT_CONNECTION: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            auto vsyncSource = static_cast<ISurfaceComposer::VsyncSource>(data.readInt32());
            auto configChanged = static_cast<ISurfaceComposer::ConfigChanged>(data.readInt32());

            sp<IDisplayEventConnection> connection(
                    createDisplayEventConnection(vsyncSource, configChanged));
            reply->writeStrongBinder(IInterface::asBinder(connection));
            return NO_ERROR;
        }
        case CREATE_DISPLAY: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            String8 displayName = data.readString8();
            bool secure = bool(data.readInt32());
            sp<IBinder> display(createDisplay(displayName, secure));
            reply->writeStrongBinder(display);
            return NO_ERROR;
        }
        case DESTROY_DISPLAY: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> display = data.readStrongBinder();
            destroyDisplay(display);
            return NO_ERROR;
        }
        case GET_PHYSICAL_DISPLAY_TOKEN: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            PhysicalDisplayId displayId = data.readUint64();
            sp<IBinder> display = getPhysicalDisplayToken(displayId);
            reply->writeStrongBinder(display);
            return NO_ERROR;
        }
        case GET_DISPLAY_CONFIGS: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            Vector<DisplayInfo> configs;
            sp<IBinder> display = data.readStrongBinder();
            status_t result = getDisplayConfigs(display, &configs);
            reply->writeInt32(result);
            if (result == NO_ERROR) {
                reply->writeUint32(static_cast<uint32_t>(configs.size()));
                for (size_t c = 0; c < configs.size(); ++c) {
                    memcpy(reply->writeInplace(sizeof(DisplayInfo)),
                            &configs[c], sizeof(DisplayInfo));
                }
            }
            return NO_ERROR;
        }
        case GET_DISPLAY_STATS: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            DisplayStatInfo stats;
            sp<IBinder> display = data.readStrongBinder();
            status_t result = getDisplayStats(display, &stats);
            reply->writeInt32(result);
            if (result == NO_ERROR) {
                memcpy(reply->writeInplace(sizeof(DisplayStatInfo)),
                        &stats, sizeof(DisplayStatInfo));
            }
            return NO_ERROR;
        }
        case GET_ACTIVE_CONFIG: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> display = data.readStrongBinder();
            int id = getActiveConfig(display);
            reply->writeInt32(id);
            return NO_ERROR;
        }
        case SET_ACTIVE_CONFIG: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> display = data.readStrongBinder();
            int id = data.readInt32();
            status_t result = setActiveConfig(display, id);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        case GET_DISPLAY_COLOR_MODES: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            Vector<ColorMode> colorModes;
            sp<IBinder> display = nullptr;
            status_t result = data.readStrongBinder(&display);
            if (result != NO_ERROR) {
                ALOGE("getDisplayColorModes failed to readStrongBinder: %d", result);
                return result;
            }
            result = getDisplayColorModes(display, &colorModes);
            reply->writeInt32(result);
            if (result == NO_ERROR) {
                reply->writeUint32(static_cast<uint32_t>(colorModes.size()));
                for (size_t i = 0; i < colorModes.size(); ++i) {
                    reply->writeInt32(static_cast<int32_t>(colorModes[i]));
                }
            }
            return NO_ERROR;
        }
        case GET_DISPLAY_NATIVE_PRIMARIES: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            ui::DisplayPrimaries primaries;
            sp<IBinder> display = nullptr;

            status_t result = data.readStrongBinder(&display);
            if (result != NO_ERROR) {
                ALOGE("getDisplayNativePrimaries failed to readStrongBinder: %d", result);
                return result;
            }

            result = getDisplayNativePrimaries(display, primaries);
            reply->writeInt32(result);
            if (result == NO_ERROR) {
                memcpy(reply->writeInplace(sizeof(ui::DisplayPrimaries)), &primaries,
                        sizeof(ui::DisplayPrimaries));
            }

            return NO_ERROR;
        }
        case GET_ACTIVE_COLOR_MODE: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> display = nullptr;
            status_t result = data.readStrongBinder(&display);
            if (result != NO_ERROR) {
                ALOGE("getActiveColorMode failed to readStrongBinder: %d", result);
                return result;
            }
            ColorMode colorMode = getActiveColorMode(display);
            result = reply->writeInt32(static_cast<int32_t>(colorMode));
            return result;
        }
        case SET_ACTIVE_COLOR_MODE: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> display = nullptr;
            status_t result = data.readStrongBinder(&display);
            if (result != NO_ERROR) {
                ALOGE("getActiveColorMode failed to readStrongBinder: %d", result);
                return result;
            }
            int32_t colorModeInt = 0;
            result = data.readInt32(&colorModeInt);
            if (result != NO_ERROR) {
                ALOGE("setActiveColorMode failed to readInt32: %d", result);
                return result;
            }
            result = setActiveColorMode(display,
                    static_cast<ColorMode>(colorModeInt));
            result = reply->writeInt32(result);
            return result;
        }
        case CLEAR_ANIMATION_FRAME_STATS: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            status_t result = clearAnimationFrameStats();
            reply->writeInt32(result);
            return NO_ERROR;
        }
        case GET_ANIMATION_FRAME_STATS: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            FrameStats stats;
            status_t result = getAnimationFrameStats(&stats);
            reply->write(stats);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        case SET_POWER_MODE: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> display = data.readStrongBinder();
            int32_t mode = data.readInt32();
            setPowerMode(display, mode);
            return NO_ERROR;
        }
        case GET_HDR_CAPABILITIES: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> display = nullptr;
            status_t result = data.readStrongBinder(&display);
            if (result != NO_ERROR) {
                ALOGE("getHdrCapabilities failed to readStrongBinder: %d",
                        result);
                return result;
            }
            HdrCapabilities capabilities;
            result = getHdrCapabilities(display, &capabilities);
            reply->writeInt32(result);
            if (result == NO_ERROR) {
                reply->write(capabilities);
            }
            return NO_ERROR;
        }
        case ENABLE_VSYNC_INJECTIONS: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            bool enable = false;
            status_t result = data.readBool(&enable);
            if (result != NO_ERROR) {
                ALOGE("enableVSyncInjections failed to readBool: %d", result);
                return result;
            }
            return enableVSyncInjections(enable);
        }
        case INJECT_VSYNC: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            int64_t when = 0;
            status_t result = data.readInt64(&when);
            if (result != NO_ERROR) {
                ALOGE("enableVSyncInjections failed to readInt64: %d", result);
                return result;
            }
            return injectVSync(when);
        }
        case GET_LAYER_DEBUG_INFO: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            std::vector<LayerDebugInfo> outLayers;
            status_t result = getLayerDebugInfo(&outLayers);
            reply->writeInt32(result);
            if (result == NO_ERROR)
            {
                result = reply->writeParcelableVector(outLayers);
            }
            return result;
        }
        case GET_COMPOSITION_PREFERENCE: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            ui::Dataspace defaultDataspace;
            ui::PixelFormat defaultPixelFormat;
            ui::Dataspace wideColorGamutDataspace;
            ui::PixelFormat wideColorGamutPixelFormat;
            status_t error =
                    getCompositionPreference(&defaultDataspace, &defaultPixelFormat,
                                             &wideColorGamutDataspace, &wideColorGamutPixelFormat);
            reply->writeInt32(error);
            if (error == NO_ERROR) {
                reply->writeInt32(static_cast<int32_t>(defaultDataspace));
                reply->writeInt32(static_cast<int32_t>(defaultPixelFormat));
                reply->writeInt32(static_cast<int32_t>(wideColorGamutDataspace));
                reply->writeInt32(static_cast<int32_t>(wideColorGamutPixelFormat));
            }
            return error;
        }
        case GET_COLOR_MANAGEMENT: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            bool result;
            status_t error = getColorManagement(&result);
            if (error == NO_ERROR) {
                reply->writeBool(result);
            }
            return error;
        }
        case GET_DISPLAYED_CONTENT_SAMPLING_ATTRIBUTES: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);

            sp<IBinder> display = data.readStrongBinder();
            ui::PixelFormat format;
            ui::Dataspace dataspace;
            uint8_t component = 0;
            auto result =
                    getDisplayedContentSamplingAttributes(display, &format, &dataspace, &component);
            if (result == NO_ERROR) {
                reply->writeUint32(static_cast<uint32_t>(format));
                reply->writeUint32(static_cast<uint32_t>(dataspace));
                reply->writeUint32(static_cast<uint32_t>(component));
            }
            return result;
        }
        case SET_DISPLAY_CONTENT_SAMPLING_ENABLED: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);

            sp<IBinder> display = nullptr;
            bool enable = false;
            int8_t componentMask = 0;
            uint64_t maxFrames = 0;
            status_t result = data.readStrongBinder(&display);
            if (result != NO_ERROR) {
                ALOGE("setDisplayContentSamplingEnabled failure in reading Display token: %d",
                      result);
                return result;
            }

            result = data.readBool(&enable);
            if (result != NO_ERROR) {
                ALOGE("setDisplayContentSamplingEnabled failure in reading enable: %d", result);
                return result;
            }

            result = data.readByte(static_cast<int8_t*>(&componentMask));
            if (result != NO_ERROR) {
                ALOGE("setDisplayContentSamplingEnabled failure in reading component mask: %d",
                      result);
                return result;
            }

            result = data.readUint64(&maxFrames);
            if (result != NO_ERROR) {
                ALOGE("setDisplayContentSamplingEnabled failure in reading max frames: %d", result);
                return result;
            }

            return setDisplayContentSamplingEnabled(display, enable,
                                                    static_cast<uint8_t>(componentMask), maxFrames);
        }
        case GET_DISPLAYED_CONTENT_SAMPLE: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);

            sp<IBinder> display = data.readStrongBinder();
            uint64_t maxFrames = 0;
            uint64_t timestamp = 0;

            status_t result = data.readUint64(&maxFrames);
            if (result != NO_ERROR) {
                ALOGE("getDisplayedContentSample failure in reading max frames: %d", result);
                return result;
            }

            result = data.readUint64(&timestamp);
            if (result != NO_ERROR) {
                ALOGE("getDisplayedContentSample failure in reading timestamp: %d", result);
                return result;
            }

            DisplayedFrameStats stats;
            result = getDisplayedContentSample(display, maxFrames, timestamp, &stats);
            if (result == NO_ERROR) {
                reply->writeUint64(stats.numFrames);
                reply->writeUint64Vector(stats.component_0_sample);
                reply->writeUint64Vector(stats.component_1_sample);
                reply->writeUint64Vector(stats.component_2_sample);
                reply->writeUint64Vector(stats.component_3_sample);
            }
            return result;
        }
        case GET_PROTECTED_CONTENT_SUPPORT: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            bool result;
            status_t error = getProtectedContentSupport(&result);
            if (error == NO_ERROR) {
                reply->writeBool(result);
            }
            return error;
        }
        case IS_WIDE_COLOR_DISPLAY: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> display = nullptr;
            status_t error = data.readStrongBinder(&display);
            if (error != NO_ERROR) {
                return error;
            }
            bool result;
            error = isWideColorDisplay(display, &result);
            if (error == NO_ERROR) {
                reply->writeBool(result);
            }
            return error;
        }
        case GET_PHYSICAL_DISPLAY_IDS: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            return reply->writeUint64Vector(getPhysicalDisplayIds());
        }
        case ADD_REGION_SAMPLING_LISTENER: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            Rect samplingArea;
            status_t result = data.read(samplingArea);
            if (result != NO_ERROR) {
                ALOGE("addRegionSamplingListener: Failed to read sampling area");
                return result;
            }
            sp<IBinder> stopLayerHandle;
            result = data.readNullableStrongBinder(&stopLayerHandle);
            if (result != NO_ERROR) {
                ALOGE("addRegionSamplingListener: Failed to read stop layer handle");
                return result;
            }
            sp<IRegionSamplingListener> listener;
            result = data.readNullableStrongBinder(&listener);
            if (result != NO_ERROR) {
                ALOGE("addRegionSamplingListener: Failed to read listener");
                return result;
            }
            return addRegionSamplingListener(samplingArea, stopLayerHandle, listener);
        }
        case REMOVE_REGION_SAMPLING_LISTENER: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IRegionSamplingListener> listener;
            status_t result = data.readNullableStrongBinder(&listener);
            if (result != NO_ERROR) {
                ALOGE("removeRegionSamplingListener: Failed to read listener");
                return result;
            }
            return removeRegionSamplingListener(listener);
        }
        case SET_ALLOWED_DISPLAY_CONFIGS: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> displayToken = data.readStrongBinder();
            std::vector<int32_t> allowedConfigs;
            data.readInt32Vector(&allowedConfigs);
            status_t result = setAllowedDisplayConfigs(displayToken, allowedConfigs);
            reply->writeInt32(result);
            return result;
        }
        case GET_ALLOWED_DISPLAY_CONFIGS: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> displayToken = data.readStrongBinder();
            std::vector<int32_t> allowedConfigs;
            status_t result = getAllowedDisplayConfigs(displayToken, &allowedConfigs);
            reply->writeInt32Vector(allowedConfigs);
            reply->writeInt32(result);
            return result;
        }
        case GET_DISPLAY_BRIGHTNESS_SUPPORT: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> displayToken;
            status_t error = data.readNullableStrongBinder(&displayToken);
            if (error != NO_ERROR) {
                ALOGE("getDisplayBrightnessSupport: failed to read display token: %d", error);
                return error;
            }
            bool support = false;
            error = getDisplayBrightnessSupport(displayToken, &support);
            reply->writeBool(support);
            return error;
        }
        case SET_DISPLAY_BRIGHTNESS: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            sp<IBinder> displayToken;
            status_t error = data.readNullableStrongBinder(&displayToken);
            if (error != NO_ERROR) {
                ALOGE("setDisplayBrightness: failed to read display token: %d", error);
                return error;
            }
            float brightness = -1.0f;
            error = data.readFloat(&brightness);
            if (error != NO_ERROR) {
                ALOGE("setDisplayBrightness: failed to read brightness: %d", error);
                return error;
            }
            return setDisplayBrightness(displayToken, brightness);
        }
        case NOTIFY_POWER_HINT: {
            CHECK_INTERFACE(ISurfaceComposer, data, reply);
            int32_t hintId;
            status_t error = data.readInt32(&hintId);
            if (error != NO_ERROR) {
                ALOGE("notifyPowerHint: failed to read hintId: %d", error);
                return error;
            }
            return notifyPowerHint(hintId);
        }
        default: {
            return BBinder::onTransact(code, data, reply, flags);
        }
    }
}

} // namespace android
