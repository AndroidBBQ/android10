/*
 * Copyright (C) 2012 The Android Open Source Project
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

#include <stdint.h>
#include <sys/types.h>

#include <binder/IPCThreadState.h>

#include <private/android_filesystem_config.h>

#include "Client.h"
#include "Layer.h"
#include "SurfaceFlinger.h"

namespace android {

// ---------------------------------------------------------------------------

const String16 sAccessSurfaceFlinger("android.permission.ACCESS_SURFACE_FLINGER");

// ---------------------------------------------------------------------------

Client::Client(const sp<SurfaceFlinger>& flinger)
    : mFlinger(flinger)
{
}

status_t Client::initCheck() const {
    return NO_ERROR;
}

void Client::attachLayer(const sp<IBinder>& handle, const sp<Layer>& layer)
{
    Mutex::Autolock _l(mLock);
    mLayers.add(handle, layer);
}

void Client::detachLayer(const Layer* layer)
{
    Mutex::Autolock _l(mLock);
    // we do a linear search here, because this doesn't happen often
    const size_t count = mLayers.size();
    for (size_t i=0 ; i<count ; i++) {
        if (mLayers.valueAt(i) == layer) {
            mLayers.removeItemsAt(i, 1);
            break;
        }
    }
}
sp<Layer> Client::getLayerUser(const sp<IBinder>& handle) const
{
    Mutex::Autolock _l(mLock);
    sp<Layer> lbc;
    wp<Layer> layer(mLayers.valueFor(handle));
    if (layer != 0) {
        lbc = layer.promote();
        ALOGE_IF(lbc==0, "getLayerUser(name=%p) is dead", handle.get());
    }
    return lbc;
}

status_t Client::createSurface(const String8& name, uint32_t w, uint32_t h, PixelFormat format,
                               uint32_t flags, const sp<IBinder>& parentHandle,
                               LayerMetadata metadata, sp<IBinder>* handle,
                               sp<IGraphicBufferProducer>* gbp) {
    // We rely on createLayer to check permissions.
    return mFlinger->createLayer(name, this, w, h, format, flags, std::move(metadata), handle, gbp,
                                 parentHandle);
}

status_t Client::createWithSurfaceParent(const String8& name, uint32_t w, uint32_t h,
                                         PixelFormat format, uint32_t flags,
                                         const sp<IGraphicBufferProducer>& parent,
                                         LayerMetadata metadata, sp<IBinder>* handle,
                                         sp<IGraphicBufferProducer>* gbp) {
    if (mFlinger->authenticateSurfaceTexture(parent) == false) {
        ALOGE("failed to authenticate surface texture");
        return BAD_VALUE;
    }

    const auto& layer = (static_cast<MonitoredProducer*>(parent.get()))->getLayer();
    if (layer == nullptr) {
        ALOGE("failed to find parent layer");
        return BAD_VALUE;
    }

    return mFlinger->createLayer(name, this, w, h, format, flags, std::move(metadata), handle, gbp,
                                 nullptr, layer);
}

status_t Client::clearLayerFrameStats(const sp<IBinder>& handle) const {
    sp<Layer> layer = getLayerUser(handle);
    if (layer == nullptr) {
        return NAME_NOT_FOUND;
    }
    layer->clearFrameStats();
    return NO_ERROR;
}

status_t Client::getLayerFrameStats(const sp<IBinder>& handle, FrameStats* outStats) const {
    sp<Layer> layer = getLayerUser(handle);
    if (layer == nullptr) {
        return NAME_NOT_FOUND;
    }
    layer->getFrameStats(outStats);
    return NO_ERROR;
}

// ---------------------------------------------------------------------------
}; // namespace android
