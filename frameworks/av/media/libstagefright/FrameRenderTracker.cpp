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

//#define LOG_NDEBUG 0
#define LOG_TAG "FrameRenderTracker"

#include <inttypes.h>
#include <gui/Surface.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/FrameRenderTracker.h>

namespace android {

FrameRenderTracker::FrameRenderTracker()
    : mLastRenderTimeNs(-1),
      mComponentName("unknown component") {
}

FrameRenderTracker::~FrameRenderTracker() {
}

void FrameRenderTracker::setComponentName(const AString &componentName) {
    mComponentName = componentName;
}

void FrameRenderTracker::clear(nsecs_t lastRenderTimeNs) {
    mRenderQueue.clear();
    mLastRenderTimeNs = lastRenderTimeNs;
}

void FrameRenderTracker::onFrameQueued(
        int64_t mediaTimeUs, const sp<GraphicBuffer> &graphicBuffer, const sp<Fence> &fence) {
    mRenderQueue.emplace_back(mediaTimeUs, graphicBuffer, fence);
}

FrameRenderTracker::Info *FrameRenderTracker::updateInfoForDequeuedBuffer(
        ANativeWindowBuffer *buf, int fenceFd, int index) {
    if (index < 0) {
        return NULL;
    }

    // see if this is a buffer that was to be rendered
    std::list<Info>::iterator renderInfo = mRenderQueue.end();
    for (std::list<Info>::iterator it = mRenderQueue.begin();
            it != mRenderQueue.end(); ++it) {
        if (it->mGraphicBuffer->handle == buf->handle) {
            renderInfo = it;
            break;
        }
    }
    if (renderInfo == mRenderQueue.end()) {
        // could have been canceled after fence has signaled
        return NULL;
    }

    if (renderInfo->mIndex >= 0) {
        // buffer has been dequeued before, so there is nothing to do
        return NULL;
    }

    // was this frame dropped (we could also infer this if the fence is invalid or a dup of
    // the queued fence; however, there is no way to figure that out.)
    if (fenceFd < 0) {
        // frame is new or was dropped
        mRenderQueue.erase(renderInfo);
        return NULL;
    }

    // store dequeue fence and buffer index
    renderInfo->mFence = new Fence(::dup(fenceFd));
    renderInfo->mIndex = index;
    return &*renderInfo;
}

status_t FrameRenderTracker::onFrameRendered(int64_t mediaTimeUs, nsecs_t systemNano) {
    // ensure monotonic timestamps
    if (mLastRenderTimeNs > systemNano ||
        // EOS is normally marked on the last frame
        (mLastRenderTimeNs == systemNano && mediaTimeUs != INT64_MAX)) {
        ALOGW("[%s] Ignoring out of order/stale system nano %lld for media time %lld from codec.",
                mComponentName.c_str(), (long long)systemNano, (long long)mediaTimeUs);
        return BAD_VALUE;
    }

    nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
    if (systemNano > now) {
        ALOGW("[%s] Ignoring system nano %lld in the future for media time %lld from codec.",
                mComponentName.c_str(), (long long)systemNano, (long long)mediaTimeUs);
        return OK;
    }

    mRenderQueue.emplace_back(mediaTimeUs, systemNano);
    mLastRenderTimeNs = systemNano;
    return OK;
}

std::list<FrameRenderTracker::Info> FrameRenderTracker::checkFencesAndGetRenderedFrames(
        const FrameRenderTracker::Info *until, bool dropIncomplete) {
    std::list<Info> done;

    // complete any frames queued prior to this and drop any incomplete ones if requested
    for (std::list<Info>::iterator it = mRenderQueue.begin();
            it != mRenderQueue.end(); ) {
        bool drop = false; // whether to drop each frame
        if (it->mIndex < 0) {
            // frame not yet dequeued (or already rendered on a tunneled surface)
            drop = dropIncomplete;
        } else if (it->mFence != NULL) {
            // check if fence signaled
            nsecs_t signalTime = it->mFence->getSignalTime();
            if (signalTime < 0) { // invalid fence
                drop = true;
            } else if (signalTime == INT64_MAX) { // unsignaled fence
                drop = dropIncomplete;
            } else { // signaled
                // save render time
                it->mFence.clear();
                it->mRenderTimeNs = signalTime;
            }
        }
        bool foundFrame = (Info *)&*it == until;

        // Return frames with signaled fences at the start of the queue, as they are
        // in submit order, and we don't have to wait for any in-between frames.
        // Also return any dropped frames.
        if (drop || (it->mFence == NULL && it == mRenderQueue.begin())) {
            // (unrendered) dropped frames have their mRenderTimeNs still set to -1
            done.splice(done.end(), mRenderQueue, it++);
        } else {
            ++it;
        }
        if (foundFrame) {
            break;
        }
    }

    return done;
}

void FrameRenderTracker::untrackFrame(const FrameRenderTracker::Info *info, ssize_t index) {
    if (info == NULL && index == SSIZE_MAX) {
        // nothing to do
        return;
    }

    for (std::list<Info>::iterator it = mRenderQueue.begin();
            it != mRenderQueue.end(); ) {
        if (&*it == info) {
            mRenderQueue.erase(it++);
        } else {
            if (it->mIndex > index) {
                --(it->mIndex);
            }
            ++it;
        }
    }
}

void FrameRenderTracker::dumpRenderQueue() const {
    ALOGI("[%s] Render Queue: (last render time: %lldns)",
            mComponentName.c_str(), (long long)mLastRenderTimeNs);
    for (std::list<Info>::const_iterator it = mRenderQueue.cbegin();
            it != mRenderQueue.cend(); ++it) {
        if (it->mFence == NULL) {
            ALOGI("  RENDERED: handle: %p, media time: %lldus, index: %zd, render time: %lldns",
                    it->mGraphicBuffer == NULL ? NULL : it->mGraphicBuffer->handle,
                    (long long)it->mMediaTimeUs, it->mIndex, (long long)it->mRenderTimeNs);
        } else if (it->mIndex < 0) {
            ALOGI("    QUEUED: handle: %p, media time: %lldus, fence: %s",
                    it->mGraphicBuffer->handle, (long long)it->mMediaTimeUs,
                    it->mFence->isValid() ? "YES" : "NO");
        } else {
            ALOGI("  DEQUEUED: handle: %p, media time: %lldus, index: %zd",
                    it->mGraphicBuffer->handle, (long long)it->mMediaTimeUs, it->mIndex);
        }
    }
}

}  // namespace android
