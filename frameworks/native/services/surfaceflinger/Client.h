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

#ifndef ANDROID_SF_CLIENT_H
#define ANDROID_SF_CLIENT_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/Mutex.h>

#include <gui/ISurfaceComposerClient.h>

namespace android {

// ---------------------------------------------------------------------------

class Layer;
class SurfaceFlinger;

// ---------------------------------------------------------------------------

class Client : public BnSurfaceComposerClient
{
public:
    explicit Client(const sp<SurfaceFlinger>& flinger);
    ~Client() = default;

    status_t initCheck() const;

    // protected by SurfaceFlinger::mStateLock
    void attachLayer(const sp<IBinder>& handle, const sp<Layer>& layer);
    void detachLayer(const Layer* layer);

    sp<Layer> getLayerUser(const sp<IBinder>& handle) const;

private:
    // ISurfaceComposerClient interface
    virtual status_t createSurface(const String8& name, uint32_t w, uint32_t h, PixelFormat format,
                                   uint32_t flags, const sp<IBinder>& parent,
                                   LayerMetadata metadata, sp<IBinder>* handle,
                                   sp<IGraphicBufferProducer>* gbp);

    virtual status_t createWithSurfaceParent(const String8& name, uint32_t w, uint32_t h,
                                             PixelFormat format, uint32_t flags,
                                             const sp<IGraphicBufferProducer>& parent,
                                             LayerMetadata metadata, sp<IBinder>* handle,
                                             sp<IGraphicBufferProducer>* gbp);

    virtual status_t clearLayerFrameStats(const sp<IBinder>& handle) const;

    virtual status_t getLayerFrameStats(const sp<IBinder>& handle, FrameStats* outStats) const;

    // constant
    sp<SurfaceFlinger> mFlinger;

    // protected by mLock
    DefaultKeyedVector< wp<IBinder>, wp<Layer> > mLayers;

    // thread-safe
    mutable Mutex mLock;
};

// ---------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_SF_CLIENT_H
