/*
 * Copyright 2015 The Android Open Source Project
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

#ifndef FRAME_RENDER_TRACKER_H_

#define FRAME_RENDER_TRACKER_H_

#include <utils/RefBase.h>
#include <utils/Timers.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AString.h>
#include <ui/Fence.h>
#include <ui/GraphicBuffer.h>

#include <list>

struct ANativeWindowBuffer;

namespace android {

// Tracks the render information about a frame. Frames go through several states while
// the render information is tracked:
//
// 1. queued frame: mMediaTime and mGraphicBuffer are set for the frame. mFence is the
// queue fence (read fence). mIndex is negative, and mRenderTimeNs is invalid.
// Key characteristics: mFence is not NULL and mIndex is negative.
//
// 2. dequeued frame: mFence is updated with the dequeue fence (write fence). mIndex is set.
// Key characteristics: mFence is not NULL and mIndex is non-negative. mRenderTime is still
// invalid.
//
// 3. rendered frame or frame: mFence is cleared, mRenderTimeNs is set.
// Key characteristics: mFence is NULL.
//
struct RenderedFrameInfo {
    // set by client during onFrameQueued or onFrameRendered
    int64_t getMediaTimeUs() const  { return mMediaTimeUs; }

    // -1 if frame is not yet rendered
    nsecs_t getRenderTimeNs() const { return mRenderTimeNs; }

    // set by client during updateRenderInfoForDequeuedBuffer; -1 otherwise
    ssize_t getIndex() const        { return mIndex; }

    // creates information for a queued frame
    RenderedFrameInfo(int64_t mediaTimeUs, const sp<GraphicBuffer> &graphicBuffer,
            const sp<Fence> &fence)
        : mMediaTimeUs(mediaTimeUs),
          mRenderTimeNs(-1),
          mIndex(-1),
          mGraphicBuffer(graphicBuffer),
          mFence(fence) {
    }

    // creates information for a frame rendered on a tunneled surface
    RenderedFrameInfo(int64_t mediaTimeUs, nsecs_t renderTimeNs)
        : mMediaTimeUs(mediaTimeUs),
          mRenderTimeNs(renderTimeNs),
          mIndex(-1),
          mGraphicBuffer(NULL),
          mFence(NULL) {
    }

private:
    int64_t mMediaTimeUs;
    nsecs_t mRenderTimeNs;
    ssize_t mIndex;         // to be used by client
    sp<GraphicBuffer> mGraphicBuffer;
    sp<Fence> mFence;

    friend struct FrameRenderTracker;
};

struct FrameRenderTracker {
    typedef RenderedFrameInfo Info;

    FrameRenderTracker();

    void setComponentName(const AString &componentName);

    // clears all tracked frames, and resets last render time
    void clear(nsecs_t lastRenderTimeNs);

    // called when |graphicBuffer| corresponding to |mediaTimeUs| is
    // queued to the output surface using |fence|.
    void onFrameQueued(
            int64_t mediaTimeUs, const sp<GraphicBuffer> &graphicBuffer, const sp<Fence> &fence);

    // Called when we have dequeued a buffer |buf| from the native window to track render info.
    // |fenceFd| is the dequeue fence, and |index| is a positive buffer ID to be usable by the
    // client to track this render info among the dequeued buffers.
    // Returns pointer to the tracked info, or NULL if buffer is not tracked or if |index|
    // is negative.
    Info *updateInfoForDequeuedBuffer(ANativeWindowBuffer *buf, int fenceFd, int index);

    // called when tunneled codec signals frame rendered event
    // returns BAD_VALUE if systemNano is not monotonic. Otherwise, returns OK.
    status_t onFrameRendered(int64_t mediaTimeUs, nsecs_t systemNano);

    // Checks to see if any frames have rendered up until |until|. If |until| is NULL or not a
    // tracked info, this method searches the entire render queue.
    // Returns list of rendered frames up-until the frame pointed to by |until| or the first
    // unrendered frame, as well as any dropped frames (those with invalid fence) up-until |until|.
    // These frames are removed from the render queue.
    // If |dropIncomplete| is true, unrendered frames up-until |until| will also be dropped from the
    // queue, allowing all rendered framed up till then to be notified of.
    // (This will effectively clear the render queue up-until (and including) |until|.)
    std::list<Info> checkFencesAndGetRenderedFrames(const Info *until, bool dropIncomplete);

    // Stop tracking a queued frame (e.g. if the frame has been discarded). If |info| is NULL or is
    // not tracked, this method is a no-op. If |index| is specified, all indices larger that |index|
    // are decremented. This is useful if the untracked frame is deleted from the frame vector.
    void untrackFrame(const Info *info, ssize_t index = SSIZE_MAX);

    void dumpRenderQueue() const;

    virtual ~FrameRenderTracker();

private:

    // Render information for buffers. Regular surface buffers are queued in the order of
    // rendering. Tunneled buffers are queued in the order of receipt.
    std::list<Info> mRenderQueue;
    nsecs_t mLastRenderTimeNs;
    AString mComponentName;

    DISALLOW_EVIL_CONSTRUCTORS(FrameRenderTracker);
};

}  // namespace android

#endif  // FRAME_RENDER_TRACKER_H_
