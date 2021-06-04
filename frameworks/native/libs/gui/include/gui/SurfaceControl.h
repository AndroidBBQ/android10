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

#ifndef ANDROID_GUI_SURFACE_CONTROL_H
#define ANDROID_GUI_SURFACE_CONTROL_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/threads.h>

#include <ui/FrameStats.h>
#include <ui/PixelFormat.h>
#include <ui/Region.h>

#include <gui/ISurfaceComposerClient.h>
#include <math/vec3.h>

namespace android {

// ---------------------------------------------------------------------------

class IGraphicBufferProducer;
class Surface;
class SurfaceComposerClient;

// ---------------------------------------------------------------------------

class SurfaceControl : public RefBase
{
public:
    static sp<SurfaceControl> readFromParcel(Parcel* parcel);
    void writeToParcel(Parcel* parcel);

    static bool isValid(const sp<SurfaceControl>& surface) {
        return (surface != nullptr) && surface->isValid();
    }

    bool isValid() {
        return mHandle!=nullptr && mClient!=nullptr;
    }

    static bool isSameSurface(
            const sp<SurfaceControl>& lhs, const sp<SurfaceControl>& rhs);

    // Release the handles assosciated with the SurfaceControl, without reparenting
    // them off-screen. At the moment if this isn't executed before ~SurfaceControl
    // is called then the destructor will reparent the layer off-screen for you.
    void        release();
    // Reparent off-screen and release. This is invoked by the destructor.
    void destroy();

    // disconnect any api that's connected
    void        disconnect();

    static status_t writeSurfaceToParcel(
            const sp<SurfaceControl>& control, Parcel* parcel);

    sp<Surface> getSurface() const;
    sp<Surface> createSurface() const;
    sp<IBinder> getHandle() const;

    sp<IGraphicBufferProducer> getIGraphicBufferProducer() const;

    status_t clearLayerFrameStats() const;
    status_t getLayerFrameStats(FrameStats* outStats) const;

    sp<SurfaceComposerClient> getClient() const;
    
    explicit SurfaceControl(const sp<SurfaceControl>& other);

    SurfaceControl(const sp<SurfaceComposerClient>& client, const sp<IBinder>& handle,
                   const sp<IGraphicBufferProducer>& gbp, bool owned);

private:
    // can't be copied
    SurfaceControl& operator = (SurfaceControl& rhs);
    SurfaceControl(const SurfaceControl& rhs);

    friend class SurfaceComposerClient;
    friend class Surface;

    ~SurfaceControl();

    sp<Surface> generateSurfaceLocked() const;
    status_t validate() const;

    sp<SurfaceComposerClient>   mClient;
    sp<IBinder>                 mHandle;
    sp<IGraphicBufferProducer>  mGraphicBufferProducer;
    mutable Mutex               mLock;
    mutable sp<Surface>         mSurfaceData;
    bool                        mOwned;
};

}; // namespace android

#endif // ANDROID_GUI_SURFACE_CONTROL_H
