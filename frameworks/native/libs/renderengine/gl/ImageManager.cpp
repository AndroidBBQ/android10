/*
 * Copyright 2019 The Android Open Source Project
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

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <pthread.h>

#include <processgroup/sched_policy.h>
#include <utils/Trace.h>
#include "GLESRenderEngine.h"
#include "ImageManager.h"

namespace android {
namespace renderengine {
namespace gl {

ImageManager::ImageManager(GLESRenderEngine* engine) : mEngine(engine) {
    pthread_setname_np(mThread.native_handle(), "ImageManager");
    // Use SCHED_FIFO to minimize jitter
    struct sched_param param = {0};
    param.sched_priority = 2;
    if (pthread_setschedparam(mThread.native_handle(), SCHED_FIFO, &param) != 0) {
        ALOGE("Couldn't set SCHED_FIFO for ImageManager");
    }
}

ImageManager::~ImageManager() {
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mRunning = false;
    }
    mCondition.notify_all();
    if (mThread.joinable()) {
        mThread.join();
    }
}

void ImageManager::cacheAsync(const sp<GraphicBuffer>& buffer,
                              const std::shared_ptr<Barrier>& barrier) {
    if (buffer == nullptr) {
        {
            std::lock_guard<std::mutex> lock(barrier->mutex);
            barrier->isOpen = true;
            barrier->result = BAD_VALUE;
        }
        barrier->condition.notify_one();
        return;
    }
    ATRACE_CALL();
    QueueEntry entry = {QueueEntry::Operation::Insert, buffer, buffer->getId(), barrier};
    queueOperation(std::move(entry));
}

status_t ImageManager::cache(const sp<GraphicBuffer>& buffer) {
    ATRACE_CALL();
    auto barrier = std::make_shared<Barrier>();
    cacheAsync(buffer, barrier);
    std::lock_guard<std::mutex> lock(barrier->mutex);
    barrier->condition.wait(barrier->mutex,
                            [&]() REQUIRES(barrier->mutex) { return barrier->isOpen; });
    return barrier->result;
}

void ImageManager::releaseAsync(uint64_t bufferId, const std::shared_ptr<Barrier>& barrier) {
    ATRACE_CALL();
    QueueEntry entry = {QueueEntry::Operation::Delete, nullptr, bufferId, barrier};
    queueOperation(std::move(entry));
}

void ImageManager::queueOperation(const QueueEntry&& entry) {
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mQueue.emplace(entry);
        ATRACE_INT("ImageManagerQueueDepth", mQueue.size());
    }
    mCondition.notify_one();
}

void ImageManager::threadMain() {
    set_sched_policy(0, SP_FOREGROUND);
    bool run;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        run = mRunning;
    }
    while (run) {
        QueueEntry entry;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mCondition.wait(mMutex,
                            [&]() REQUIRES(mMutex) { return !mQueue.empty() || !mRunning; });
            run = mRunning;

            if (!mRunning) {
                // if mRunning is false, then ImageManager is being destroyed, so
                // bail out now.
                break;
            }

            entry = mQueue.front();
            mQueue.pop();
            ATRACE_INT("ImageManagerQueueDepth", mQueue.size());
        }

        status_t result = NO_ERROR;
        switch (entry.op) {
            case QueueEntry::Operation::Delete:
                mEngine->unbindExternalTextureBufferInternal(entry.bufferId);
                break;
            case QueueEntry::Operation::Insert:
                result = mEngine->cacheExternalTextureBufferInternal(entry.buffer);
                break;
        }
        if (entry.barrier != nullptr) {
            {
                std::lock_guard<std::mutex> entryLock(entry.barrier->mutex);
                entry.barrier->result = result;
                entry.barrier->isOpen = true;
            }
            entry.barrier->condition.notify_one();
        }
    }
}

} // namespace gl
} // namespace renderengine
} // namespace android
