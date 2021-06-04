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

#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include <ui/GraphicBuffer.h>

namespace android {
namespace renderengine {
namespace gl {

class GLESRenderEngine;

class ImageManager {
public:
    struct Barrier {
        std::mutex mutex;
        std::condition_variable_any condition;
        bool isOpen GUARDED_BY(mutex) = false;
        status_t result GUARDED_BY(mutex) = NO_ERROR;
    };
    ImageManager(GLESRenderEngine* engine);
    ~ImageManager();
    void cacheAsync(const sp<GraphicBuffer>& buffer, const std::shared_ptr<Barrier>& barrier)
            EXCLUDES(mMutex);
    status_t cache(const sp<GraphicBuffer>& buffer);
    void releaseAsync(uint64_t bufferId, const std::shared_ptr<Barrier>& barrier) EXCLUDES(mMutex);

private:
    struct QueueEntry {
        enum class Operation { Delete, Insert };

        Operation op = Operation::Delete;
        sp<GraphicBuffer> buffer = nullptr;
        uint64_t bufferId = 0;
        std::shared_ptr<Barrier> barrier = nullptr;
    };

    void queueOperation(const QueueEntry&& entry);
    void threadMain();
    GLESRenderEngine* const mEngine;
    std::thread mThread = std::thread([this]() { threadMain(); });
    std::condition_variable_any mCondition;
    std::mutex mMutex;
    std::queue<QueueEntry> mQueue GUARDED_BY(mMutex);

    bool mRunning GUARDED_BY(mMutex) = true;
};

} // namespace gl
} // namespace renderengine
} // namespace android
