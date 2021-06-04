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

#include <gui/ISurfaceComposerClient.h>

#include <gui/IGraphicBufferProducer.h>

#include <binder/SafeInterface.h>

#include <ui/FrameStats.h>

namespace android {

namespace { // Anonymous

enum class Tag : uint32_t {
    CREATE_SURFACE = IBinder::FIRST_CALL_TRANSACTION,
    CREATE_WITH_SURFACE_PARENT,
    CLEAR_LAYER_FRAME_STATS,
    GET_LAYER_FRAME_STATS,
    LAST = GET_LAYER_FRAME_STATS,
};

} // Anonymous namespace

class BpSurfaceComposerClient : public SafeBpInterface<ISurfaceComposerClient> {
public:
    explicit BpSurfaceComposerClient(const sp<IBinder>& impl)
          : SafeBpInterface<ISurfaceComposerClient>(impl, "BpSurfaceComposerClient") {}

    ~BpSurfaceComposerClient() override;

    status_t createSurface(const String8& name, uint32_t width, uint32_t height, PixelFormat format,
                           uint32_t flags, const sp<IBinder>& parent, LayerMetadata metadata,
                           sp<IBinder>* handle, sp<IGraphicBufferProducer>* gbp) override {
        return callRemote<decltype(&ISurfaceComposerClient::createSurface)>(Tag::CREATE_SURFACE,
                                                                            name, width, height,
                                                                            format, flags, parent,
                                                                            std::move(metadata),
                                                                            handle, gbp);
    }

    status_t createWithSurfaceParent(const String8& name, uint32_t width, uint32_t height,
                                     PixelFormat format, uint32_t flags,
                                     const sp<IGraphicBufferProducer>& parent,
                                     LayerMetadata metadata, sp<IBinder>* handle,
                                     sp<IGraphicBufferProducer>* gbp) override {
        return callRemote<decltype(
                &ISurfaceComposerClient::createWithSurfaceParent)>(Tag::CREATE_WITH_SURFACE_PARENT,
                                                                   name, width, height, format,
                                                                   flags, parent,
                                                                   std::move(metadata), handle,
                                                                   gbp);
    }

    status_t clearLayerFrameStats(const sp<IBinder>& handle) const override {
        return callRemote<decltype(
                &ISurfaceComposerClient::clearLayerFrameStats)>(Tag::CLEAR_LAYER_FRAME_STATS,
                                                                handle);
    }

    status_t getLayerFrameStats(const sp<IBinder>& handle, FrameStats* outStats) const override {
        return callRemote<decltype(
                &ISurfaceComposerClient::getLayerFrameStats)>(Tag::GET_LAYER_FRAME_STATS, handle,
                                                              outStats);
    }
};

// Out-of-line virtual method definition to trigger vtable emission in this
// translation unit (see clang warning -Wweak-vtables)
BpSurfaceComposerClient::~BpSurfaceComposerClient() {}

IMPLEMENT_META_INTERFACE(SurfaceComposerClient, "android.ui.ISurfaceComposerClient");

// ----------------------------------------------------------------------

status_t BnSurfaceComposerClient::onTransact(uint32_t code, const Parcel& data, Parcel* reply,
                                             uint32_t flags) {
    if (code < IBinder::FIRST_CALL_TRANSACTION || code > static_cast<uint32_t>(Tag::LAST)) {
        return BBinder::onTransact(code, data, reply, flags);
    }
    auto tag = static_cast<Tag>(code);
    switch (tag) {
        case Tag::CREATE_SURFACE:
            return callLocal(data, reply, &ISurfaceComposerClient::createSurface);
        case Tag::CREATE_WITH_SURFACE_PARENT:
            return callLocal(data, reply, &ISurfaceComposerClient::createWithSurfaceParent);
        case Tag::CLEAR_LAYER_FRAME_STATS:
            return callLocal(data, reply, &ISurfaceComposerClient::clearLayerFrameStats);
        case Tag::GET_LAYER_FRAME_STATS:
            return callLocal(data, reply, &ISurfaceComposerClient::getLayerFrameStats);
    }
}

} // namespace android
