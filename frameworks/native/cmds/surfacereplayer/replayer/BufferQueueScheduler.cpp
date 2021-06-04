/*
 * Copyright 2016 The Android Open Source Project
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
#define LOG_TAG "BufferQueueScheduler"

#include "BufferQueueScheduler.h"

#include <android/native_window.h>
#include <gui/Surface.h>

using namespace android;

BufferQueueScheduler::BufferQueueScheduler(
        const sp<SurfaceControl>& surfaceControl, const HSV& color, int id)
      : mSurfaceControl(surfaceControl), mColor(color), mSurfaceId(id), mContinueScheduling(true) {}

void BufferQueueScheduler::startScheduling() {
    ALOGV("Starting Scheduler for %d Layer", mSurfaceId);
    std::unique_lock<std::mutex> lock(mMutex);
    if (mSurfaceControl == nullptr) {
        mCondition.wait(lock, [&] { return (mSurfaceControl != nullptr); });
    }

    while (mContinueScheduling) {
        while (true) {
            if (mBufferEvents.empty()) {
                break;
            }

            BufferEvent event = mBufferEvents.front();
            lock.unlock();

            bufferUpdate(event.dimensions);
            fillSurface(event.event);
            mColor.modulate();
            lock.lock();
            mBufferEvents.pop();
        }
        mCondition.wait(lock);
    }
}

void BufferQueueScheduler::addEvent(const BufferEvent& event) {
    std::lock_guard<std::mutex> lock(mMutex);
    mBufferEvents.push(event);
    mCondition.notify_one();
}

void BufferQueueScheduler::stopScheduling() {
    std::lock_guard<std::mutex> lock(mMutex);
    mContinueScheduling = false;
    mCondition.notify_one();
}

void BufferQueueScheduler::setSurfaceControl(
        const sp<SurfaceControl>& surfaceControl, const HSV& color) {
    std::lock_guard<std::mutex> lock(mMutex);
    mSurfaceControl = surfaceControl;
    mColor = color;
    mCondition.notify_one();
}

void BufferQueueScheduler::bufferUpdate(const Dimensions& dimensions) {
    sp<Surface> s = mSurfaceControl->getSurface();
    s->setBuffersDimensions(dimensions.width, dimensions.height);
}

void BufferQueueScheduler::fillSurface(const std::shared_ptr<Event>& event) {
    ANativeWindow_Buffer outBuffer;
    sp<Surface> s = mSurfaceControl->getSurface();

    status_t status = s->lock(&outBuffer, nullptr);

    if (status != NO_ERROR) {
        ALOGE("fillSurface: failed to lock buffer, (%d)", status);
        return;
    }

    auto color = mColor.getRGB();

    auto img = reinterpret_cast<uint8_t*>(outBuffer.bits);
    for (int y = 0; y < outBuffer.height; y++) {
        for (int x = 0; x < outBuffer.width; x++) {
            uint8_t* pixel = img + (4 * (y * outBuffer.stride + x));
            pixel[0] = color.r;
            pixel[1] = color.g;
            pixel[2] = color.b;
            pixel[3] = LAYER_ALPHA;
        }
    }

    event->readyToExecute();

    status = s->unlockAndPost();

    ALOGE_IF(status != NO_ERROR, "fillSurface: failed to unlock and post buffer, (%d)", status);
}
