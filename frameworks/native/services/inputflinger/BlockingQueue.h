/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef _UI_INPUT_BLOCKING_QUEUE_H
#define _UI_INPUT_BLOCKING_QUEUE_H

#include "android-base/thread_annotations.h"
#include <condition_variable>
#include <mutex>
#include <vector>

namespace android {

/**
 * A FIFO queue that stores up to <i>capacity</i> objects.
 * Objects can always be added. Objects are added immediately.
 * If the queue is full, new objects cannot be added.
 *
 * The action of retrieving an object will block until an element is available.
 */
template <class T>
class BlockingQueue {
public:
    BlockingQueue(size_t capacity) : mCapacity(capacity) {
        mQueue.reserve(mCapacity);
    };

    /**
     * Retrieve and remove the oldest object.
     * Blocks execution while queue is empty.
     */
    T pop() {
        std::unique_lock lock(mLock);
        android::base::ScopedLockAssertion assumeLock(mLock);
        mHasElements.wait(lock, [this]{
                android::base::ScopedLockAssertion assumeLock(mLock);
                return !this->mQueue.empty();
        });
        T t = std::move(mQueue.front());
        mQueue.erase(mQueue.begin());
        return t;
    };

    /**
     * Add a new object to the queue.
     * Does not block.
     * Return true if an element was successfully added.
     * Return false if the queue is full.
     */
    bool push(T&& t) {
        {
            std::scoped_lock lock(mLock);
            if (mQueue.size() == mCapacity) {
                return false;
            }
            mQueue.push_back(std::move(t));
        }
        mHasElements.notify_one();
        return true;
    };

    void erase(const std::function<bool(const T&)>& lambda) {
        std::scoped_lock lock(mLock);
        mQueue.erase(std::remove_if(mQueue.begin(), mQueue.end(),
                [&lambda](const T& t) { return lambda(t); }), mQueue.end());
    }

    /**
     * Remove all elements.
     * Does not block.
     */
    void clear() {
        std::scoped_lock lock(mLock);
        mQueue.clear();
    };

    /**
     * How many elements are currently stored in the queue.
     * Primary used for debugging.
     * Does not block.
     */
    size_t size() {
        std::scoped_lock lock(mLock);
        return mQueue.size();
    }

private:
    const size_t mCapacity;
    /**
     * Used to signal that mQueue is non-empty.
     */
    std::condition_variable mHasElements;
    /**
     * Lock for accessing and waiting on elements.
     */
    std::mutex mLock;
    std::vector<T> mQueue GUARDED_BY(mLock);
};


} // namespace android
#endif
