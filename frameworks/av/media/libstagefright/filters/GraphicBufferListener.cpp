/*
 * Copyright (C) 2014 The Android Open Source Project
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
#define LOG_TAG "GraphicBufferListener"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaErrors.h>

#include <gui/BufferItem.h>
#include <utils/String8.h>

#include "GraphicBufferListener.h"

namespace android {

status_t GraphicBufferListener::init(
        const sp<AMessage> &notify,
        size_t bufferWidth, size_t bufferHeight, size_t bufferCount) {
    mNotify = notify;

    String8 name("GraphicBufferListener");
    BufferQueue::createBufferQueue(&mProducer, &mConsumer);
    mConsumer->setConsumerName(name);
    mConsumer->setDefaultBufferSize(bufferWidth, bufferHeight);
    mConsumer->setConsumerUsageBits(GRALLOC_USAGE_SW_READ_OFTEN);

    status_t err = mConsumer->setMaxAcquiredBufferCount(bufferCount);
    if (err != NO_ERROR) {
        ALOGE("Unable to set BQ max acquired buffer count to %zu: %d",
                bufferCount, err);
        return err;
    }

    wp<BufferQueue::ConsumerListener> listener =
        static_cast<BufferQueue::ConsumerListener*>(this);
    sp<BufferQueue::ProxyConsumerListener> proxy =
        new BufferQueue::ProxyConsumerListener(listener);

    err = mConsumer->consumerConnect(proxy, false);
    if (err != NO_ERROR) {
        ALOGE("Error connecting to BufferQueue: %s (%d)",
                strerror(-err), err);
        return err;
    }

    ALOGV("init() successful.");

    return OK;
}

void GraphicBufferListener::onFrameAvailable(const BufferItem& /* item */) {
    ALOGV("onFrameAvailable() called");

    {
        Mutex::Autolock autoLock(mMutex);
        mNumFramesAvailable++;
    }

    sp<AMessage> notify = mNotify->dup();
    mNotify->setWhat(kWhatFrameAvailable);
    mNotify->post();
}

void GraphicBufferListener::onBuffersReleased() {
    ALOGV("onBuffersReleased() called");
    // nothing to do
}

void GraphicBufferListener::onSidebandStreamChanged() {
    ALOGW("GraphicBufferListener cannot consume sideband streams.");
    // nothing to do
}

BufferItem GraphicBufferListener::getBufferItem() {
    BufferItem item;

    {
        Mutex::Autolock autoLock(mMutex);
        if (mNumFramesAvailable <= 0) {
            ALOGE("getBuffer() called with no frames available");
            return item;
        }
        mNumFramesAvailable--;
    }

    status_t err = mConsumer->acquireBuffer(&item, 0);
    if (err == BufferQueue::NO_BUFFER_AVAILABLE) {
        // shouldn't happen, since we track num frames available
        ALOGE("frame was not available");
        item.mSlot = -1;
        return item;
    } else if (err != OK) {
        ALOGE("acquireBuffer returned err=%d", err);
        item.mSlot = -1;
        return item;
    }

    // Wait for it to become available.
    err = item.mFence->waitForever("GraphicBufferListener::getBufferItem");
    if (err != OK) {
        ALOGW("failed to wait for buffer fence: %d", err);
        // keep going
    }

    // If this is the first time we're seeing this buffer, add it to our
    // slot table.
    if (item.mGraphicBuffer != NULL) {
        ALOGV("setting mBufferSlot %d", item.mSlot);
        mBufferSlot[item.mSlot] = item.mGraphicBuffer;
    }

    return item;
}

sp<GraphicBuffer> GraphicBufferListener::getBuffer(BufferItem item) {
    sp<GraphicBuffer> buf;
    if (item.mSlot < 0 || item.mSlot >= BufferQueue::NUM_BUFFER_SLOTS) {
        ALOGE("getBuffer() received invalid BufferItem: mSlot==%d", item.mSlot);
        return buf;
    }

    buf = mBufferSlot[item.mSlot];
    CHECK(buf.get() != NULL);

    return buf;
}

status_t GraphicBufferListener::releaseBuffer(BufferItem item) {
    if (item.mSlot < 0 || item.mSlot >= BufferQueue::NUM_BUFFER_SLOTS) {
        ALOGE("getBuffer() received invalid BufferItem: mSlot==%d", item.mSlot);
        return ERROR_OUT_OF_RANGE;
    }

    mConsumer->releaseBuffer(item.mSlot, item.mFrameNumber,
            EGL_NO_DISPLAY, EGL_NO_SYNC_KHR, Fence::NO_FENCE);

    return OK;
}

}   // namespace android
