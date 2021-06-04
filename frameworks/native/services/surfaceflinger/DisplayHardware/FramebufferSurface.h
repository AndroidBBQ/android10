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

#ifndef ANDROID_SF_FRAMEBUFFER_SURFACE_H
#define ANDROID_SF_FRAMEBUFFER_SURFACE_H

#include <stdint.h>
#include <sys/types.h>

#include <compositionengine/DisplaySurface.h>
#include <compositionengine/impl/HwcBufferCache.h>
#include <gui/ConsumerBase.h>

#include "DisplayIdentification.h"

// ---------------------------------------------------------------------------
namespace android {
// ---------------------------------------------------------------------------

class Rect;
class String8;
class HWComposer;

// ---------------------------------------------------------------------------

class FramebufferSurface : public ConsumerBase, public compositionengine::DisplaySurface {
public:
    FramebufferSurface(HWComposer& hwc, DisplayId displayId,
                       const sp<IGraphicBufferConsumer>& consumer);

    virtual status_t beginFrame(bool mustRecompose);
    virtual status_t prepareFrame(CompositionType compositionType);
    virtual status_t advanceFrame();
    virtual void onFrameCommitted();
    virtual void dumpAsString(String8& result) const;

    virtual void resizeBuffers(const uint32_t width, const uint32_t height);

    virtual const sp<Fence>& getClientTargetAcquireFence() const override;

private:
    virtual ~FramebufferSurface() { }; // this class cannot be overloaded

    virtual void freeBufferLocked(int slotIndex);

    virtual void dumpLocked(String8& result, const char* prefix) const;

    // nextBuffer waits for and then latches the next buffer from the
    // BufferQueue and releases the previously latched buffer to the
    // BufferQueue.  The new buffer is returned in the 'buffer' argument.
    status_t nextBuffer(uint32_t& outSlot, sp<GraphicBuffer>& outBuffer,
            sp<Fence>& outFence, ui::Dataspace& outDataspace);

    const DisplayId mDisplayId;

    // mCurrentBufferIndex is the slot index of the current buffer or
    // INVALID_BUFFER_SLOT to indicate that either there is no current buffer
    // or the buffer is not associated with a slot.
    int mCurrentBufferSlot;

    // mDataSpace is the dataspace of the current composition buffer for
    // this FramebufferSurface. It will be 0 when HWC is doing the
    // compositing. Otherwise it will display the dataspace of the buffer
    // use for compositing which can change as wide-color content is
    // on/off.
    ui::Dataspace mDataSpace;

    // mCurrentBuffer is the current buffer or nullptr to indicate that there is
    // no current buffer.
    sp<GraphicBuffer> mCurrentBuffer;

    // mCurrentFence is the current buffer's acquire fence
    sp<Fence> mCurrentFence;

    // Hardware composer, owned by SurfaceFlinger.
    HWComposer& mHwc;

    compositionengine::impl::HwcBufferCache mHwcBufferCache;

    // Previous buffer to release after getting an updated retire fence
    bool mHasPendingRelease;
    int mPreviousBufferSlot;
    sp<GraphicBuffer> mPreviousBuffer;
};

// ---------------------------------------------------------------------------
}; // namespace android
// ---------------------------------------------------------------------------

#endif // ANDROID_SF_FRAMEBUFFER_SURFACE_H

