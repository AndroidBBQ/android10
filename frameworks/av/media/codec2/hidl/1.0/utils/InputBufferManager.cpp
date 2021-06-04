/*
 * Copyright 2018 The Android Open Source Project
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
#define LOG_TAG "Codec2-InputBufferManager"
#include <android-base/logging.h>

#include <codec2/hidl/1.0/InputBufferManager.h>
#include <codec2/hidl/1.0/types.h>

#include <android/hardware/media/c2/1.0/IComponentListener.h>
#include <android-base/logging.h>

#include <C2Buffer.h>
#include <C2Work.h>

#include <chrono>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using namespace ::android;

void InputBufferManager::registerFrameData(
        const sp<IComponentListener>& listener,
        const C2FrameData& input) {
    getInstance()._registerFrameData(listener, input);
}

void InputBufferManager::unregisterFrameData(
        const wp<IComponentListener>& listener,
        const C2FrameData& input) {
    getInstance()._unregisterFrameData(listener, input);
}

void InputBufferManager::unregisterFrameData(
        const wp<IComponentListener>& listener) {
    getInstance()._unregisterFrameData(listener);
}

void InputBufferManager::setNotificationInterval(
        nsecs_t notificationIntervalNs) {
    getInstance()._setNotificationInterval(notificationIntervalNs);
}

void InputBufferManager::_registerFrameData(
        const sp<IComponentListener>& listener,
        const C2FrameData& input) {
    uint64_t frameIndex = input.ordinal.frameIndex.peeku();
    LOG(VERBOSE) << "InputBufferManager::_registerFrameData -- called with "
                 << "listener @ 0x" << std::hex << listener.get()
                 << ", frameIndex = " << std::dec << frameIndex
                 << ".";
    std::lock_guard<std::mutex> lock(mMutex);

    std::set<TrackedBuffer*> &bufferIds =
            mTrackedBuffersMap[listener][frameIndex];

    for (size_t i = 0; i < input.buffers.size(); ++i) {
        if (!input.buffers[i]) {
            LOG(VERBOSE) << "InputBufferManager::_registerFrameData -- "
                         << "Input buffer at index " << i << " is null.";
            continue;
        }
        TrackedBuffer *bufferId =
            new TrackedBuffer(listener, frameIndex, i, input.buffers[i]);
        mTrackedBufferCache.emplace(bufferId);
        bufferIds.emplace(bufferId);

        c2_status_t status = input.buffers[i]->registerOnDestroyNotify(
                onBufferDestroyed,
                reinterpret_cast<void*>(bufferId));
        if (status != C2_OK) {
            LOG(DEBUG) << "InputBufferManager::_registerFrameData -- "
                       << "registerOnDestroyNotify() failed "
                       << "(listener @ 0x" << std::hex << listener.get()
                       << ", frameIndex = " << std::dec << frameIndex
                       << ", bufferIndex = " << i
                       << ") => status = " << status
                       << ".";
        }
    }

    mDeathNotifications.emplace(
            listener,
            DeathNotifications(
                mNotificationIntervalNs.load(std::memory_order_relaxed)));
}

// Remove a pair (listener, frameIndex) from mTrackedBuffersMap and
// mDeathNotifications. This implies all bufferIndices are removed.
//
// This is called from onWorkDone() and flush().
void InputBufferManager::_unregisterFrameData(
        const wp<IComponentListener>& listener,
        const C2FrameData& input) {
    uint64_t frameIndex = input.ordinal.frameIndex.peeku();
    LOG(VERBOSE) << "InputBufferManager::_unregisterFrameData -- called with "
                 << "listener @ 0x" << std::hex << listener.unsafe_get()
                 << ", frameIndex = " << std::dec << frameIndex
                 << ".";
    std::lock_guard<std::mutex> lock(mMutex);

    auto findListener = mTrackedBuffersMap.find(listener);
    if (findListener != mTrackedBuffersMap.end()) {
        std::map<uint64_t, std::set<TrackedBuffer*>> &frameIndex2BufferIds
                = findListener->second;
        auto findFrameIndex = frameIndex2BufferIds.find(frameIndex);
        if (findFrameIndex != frameIndex2BufferIds.end()) {
            std::set<TrackedBuffer*> &bufferIds = findFrameIndex->second;
            for (TrackedBuffer* bufferId : bufferIds) {
                std::shared_ptr<C2Buffer> buffer = bufferId->buffer.lock();
                if (buffer) {
                    c2_status_t status = buffer->unregisterOnDestroyNotify(
                            onBufferDestroyed,
                            reinterpret_cast<void*>(bufferId));
                    if (status != C2_OK) {
                        LOG(DEBUG) << "InputBufferManager::_unregisterFrameData "
                                   << "-- unregisterOnDestroyNotify() failed "
                                   << "(listener @ 0x"
                                        << std::hex
                                        << bufferId->listener.unsafe_get()
                                   << ", frameIndex = "
                                        << std::dec << bufferId->frameIndex
                                   << ", bufferIndex = " << bufferId->bufferIndex
                                   << ") => status = " << status
                                   << ".";
                    }
                }
                mTrackedBufferCache.erase(bufferId);
                delete bufferId;
            }

            frameIndex2BufferIds.erase(findFrameIndex);
            if (frameIndex2BufferIds.empty()) {
                mTrackedBuffersMap.erase(findListener);
            }
        }
    }

    auto findListenerD = mDeathNotifications.find(listener);
    if (findListenerD != mDeathNotifications.end()) {
        DeathNotifications &deathNotifications = findListenerD->second;
        auto findFrameIndex = deathNotifications.indices.find(frameIndex);
        if (findFrameIndex != deathNotifications.indices.end()) {
            std::vector<size_t> &bufferIndices = findFrameIndex->second;
            deathNotifications.count -= bufferIndices.size();
            deathNotifications.indices.erase(findFrameIndex);
        }
    }
}

// Remove listener from mTrackedBuffersMap and mDeathNotifications. This implies
// all frameIndices and bufferIndices are removed.
//
// This is called when the component cleans up all input buffers, i.e., when
// reset(), release(), stop() or ~Component() is called.
void InputBufferManager::_unregisterFrameData(
        const wp<IComponentListener>& listener) {
    LOG(VERBOSE) << "InputBufferManager::_unregisterFrameData -- called with "
                 << "listener @ 0x" << std::hex << listener.unsafe_get()
                 << std::dec << ".";
    std::lock_guard<std::mutex> lock(mMutex);

    auto findListener = mTrackedBuffersMap.find(listener);
    if (findListener != mTrackedBuffersMap.end()) {
        std::map<uint64_t, std::set<TrackedBuffer*>> &frameIndex2BufferIds =
                findListener->second;
        for (auto findFrameIndex = frameIndex2BufferIds.begin();
                findFrameIndex != frameIndex2BufferIds.end();
                ++findFrameIndex) {
            std::set<TrackedBuffer*> &bufferIds = findFrameIndex->second;
            for (TrackedBuffer* bufferId : bufferIds) {
                std::shared_ptr<C2Buffer> buffer = bufferId->buffer.lock();
                if (buffer) {
                    c2_status_t status = buffer->unregisterOnDestroyNotify(
                            onBufferDestroyed,
                            reinterpret_cast<void*>(bufferId));
                    if (status != C2_OK) {
                        LOG(DEBUG) << "InputBufferManager::_unregisterFrameData "
                                   << "-- unregisterOnDestroyNotify() failed "
                                   << "(listener @ 0x"
                                        << std::hex
                                        << bufferId->listener.unsafe_get()
                                   << ", frameIndex = "
                                        << std::dec << bufferId->frameIndex
                                   << ", bufferIndex = " << bufferId->bufferIndex
                                   << ") => status = " << status
                                   << ".";
                    }
                    mTrackedBufferCache.erase(bufferId);
                    delete bufferId;
                }
            }
        }
        mTrackedBuffersMap.erase(findListener);
    }

    mDeathNotifications.erase(listener);
}

// Set mNotificationIntervalNs.
void InputBufferManager::_setNotificationInterval(
        nsecs_t notificationIntervalNs) {
    mNotificationIntervalNs.store(
            notificationIntervalNs,
            std::memory_order_relaxed);
}

// Move a buffer from mTrackedBuffersMap to mDeathNotifications.
// This is called when a registered C2Buffer object is destroyed.
void InputBufferManager::onBufferDestroyed(const C2Buffer* buf, void* arg) {
    getInstance()._onBufferDestroyed(buf, arg);
}

void InputBufferManager::_onBufferDestroyed(const C2Buffer* buf, void* arg) {
    if (!buf || !arg) {
        LOG(WARNING) << "InputBufferManager::_onBufferDestroyed -- called with "
                     << "null argument (s): "
                     << "buf @ 0x" << std::hex << buf
                     << ", arg @ 0x" << std::hex << arg
                     << std::dec << ".";
        return;
    }

    std::lock_guard<std::mutex> lock(mMutex);
    TrackedBuffer *bufferId = reinterpret_cast<TrackedBuffer*>(arg);

    if (mTrackedBufferCache.find(bufferId) == mTrackedBufferCache.end()) {
        LOG(VERBOSE) << "InputBufferManager::_onBufferDestroyed -- called with "
                     << "unregistered buffer: "
                     << "buf @ 0x" << std::hex << buf
                     << ", arg @ 0x" << std::hex << arg
                     << std::dec << ".";
        return;
    }

    LOG(VERBOSE) << "InputBufferManager::_onBufferDestroyed -- called with "
                 << "buf @ 0x" << std::hex << buf
                 << ", arg @ 0x" << std::hex << arg
                 << std::dec << " -- "
                 << "listener @ 0x" << std::hex << bufferId->listener.unsafe_get()
                 << ", frameIndex = " << std::dec << bufferId->frameIndex
                 << ", bufferIndex = " << bufferId->bufferIndex
                 << ".";
    auto findListener = mTrackedBuffersMap.find(bufferId->listener);
    if (findListener == mTrackedBuffersMap.end()) {
        LOG(VERBOSE) << "InputBufferManager::_onBufferDestroyed -- "
                     << "received invalid listener: "
                     << "listener @ 0x" << std::hex << bufferId->listener.unsafe_get()
                     << " (frameIndex = " << std::dec << bufferId->frameIndex
                     << ", bufferIndex = " << bufferId->bufferIndex
                     << ").";
        return;
    }

    std::map<uint64_t, std::set<TrackedBuffer*>> &frameIndex2BufferIds
            = findListener->second;
    auto findFrameIndex = frameIndex2BufferIds.find(bufferId->frameIndex);
    if (findFrameIndex == frameIndex2BufferIds.end()) {
        LOG(DEBUG) << "InputBufferManager::_onBufferDestroyed -- "
                   << "received invalid frame index: "
                   << "frameIndex = " << bufferId->frameIndex
                   << " (listener @ 0x" << std::hex << bufferId->listener.unsafe_get()
                   << ", bufferIndex = " << std::dec << bufferId->bufferIndex
                   << ").";
        return;
    }

    std::set<TrackedBuffer*> &bufferIds = findFrameIndex->second;
    auto findBufferId = bufferIds.find(bufferId);
    if (findBufferId == bufferIds.end()) {
        LOG(DEBUG) << "InputBufferManager::_onBufferDestroyed -- "
                   << "received invalid buffer index: "
                   << "bufferIndex = " << bufferId->bufferIndex
                   << " (frameIndex = " << bufferId->frameIndex
                   << ", listener @ 0x" << std::hex << bufferId->listener.unsafe_get()
                   << std::dec << ").";
        return;
    }

    bufferIds.erase(findBufferId);
    if (bufferIds.empty()) {
        frameIndex2BufferIds.erase(findFrameIndex);
        if (frameIndex2BufferIds.empty()) {
            mTrackedBuffersMap.erase(findListener);
        }
    }

    DeathNotifications &deathNotifications = mDeathNotifications[bufferId->listener];
    deathNotifications.indices[bufferId->frameIndex].emplace_back(bufferId->bufferIndex);
    ++deathNotifications.count;
    mOnBufferDestroyed.notify_one();

    mTrackedBufferCache.erase(bufferId);
    delete bufferId;
}

// Notify the clients about buffer destructions.
// Return false if all destructions have been notified.
// Return true and set timeToRetry to the time point to wait for before
// retrying if some destructions have not been notified.
bool InputBufferManager::processNotifications(nsecs_t* timeToRetryNs) {

    struct Notification {
        sp<IComponentListener> listener;
        hidl_vec<IComponentListener::InputBuffer> inputBuffers;
        Notification(const sp<IComponentListener>& l, size_t s)
              : listener(l), inputBuffers(s) {}
    };
    std::list<Notification> notifications;
    nsecs_t notificationIntervalNs =
            mNotificationIntervalNs.load(std::memory_order_relaxed);

    bool retry = false;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        *timeToRetryNs = notificationIntervalNs;
        nsecs_t timeNowNs = systemTime();
        for (auto it = mDeathNotifications.begin();
                it != mDeathNotifications.end(); ) {
            sp<IComponentListener> listener = it->first.promote();
            if (!listener) {
                ++it;
                continue;
            }
            DeathNotifications &deathNotifications = it->second;

            nsecs_t timeSinceLastNotifiedNs =
                    timeNowNs - deathNotifications.lastSentNs;
            // If not enough time has passed since the last callback, leave the
            // notifications for this listener untouched for now and retry
            // later.
            if (timeSinceLastNotifiedNs < notificationIntervalNs) {
                retry = true;
                *timeToRetryNs = std::min(*timeToRetryNs,
                        notificationIntervalNs - timeSinceLastNotifiedNs);
                LOG(VERBOSE) << "InputBufferManager::processNotifications -- "
                             << "Notifications for listener @ "
                                 << std::hex << listener.get()
                             << " will be postponed.";
                ++it;
                continue;
            }

            // If enough time has passed since the last notification to this
            // listener but there are currently no pending notifications, the
            // listener can be removed from mDeathNotifications---there is no
            // need to keep track of the last notification time anymore.
            if (deathNotifications.count == 0) {
                it = mDeathNotifications.erase(it);
                continue;
            }

            // Create the argument for the callback.
            notifications.emplace_back(listener, deathNotifications.count);
            hidl_vec<IComponentListener::InputBuffer> &inputBuffers =
                    notifications.back().inputBuffers;
            size_t i = 0;
            for (std::pair<const uint64_t, std::vector<size_t>>& p :
                    deathNotifications.indices) {
                uint64_t frameIndex = p.first;
                const std::vector<size_t> &bufferIndices = p.second;
                for (const size_t& bufferIndex : bufferIndices) {
                    IComponentListener::InputBuffer &inputBuffer
                            = inputBuffers[i++];
                    inputBuffer.arrayIndex = bufferIndex;
                    inputBuffer.frameIndex = frameIndex;
                }
            }

            // Clear deathNotifications for this listener and set retry to true
            // so processNotifications will be called again. This will
            // guarantee that a listener with no pending notifications will
            // eventually be removed from mDeathNotifications after
            // mNotificationIntervalNs nanoseconds has passed.
            retry = true;
            deathNotifications.indices.clear();
            deathNotifications.count = 0;
            deathNotifications.lastSentNs = timeNowNs;
            ++it;
        }
    }

    // Call onInputBuffersReleased() outside the lock to avoid deadlock.
    for (const Notification& notification : notifications) {
        if (!notification.listener->onInputBuffersReleased(
                notification.inputBuffers).isOk()) {
            // This may trigger if the client has died.
            LOG(DEBUG) << "InputBufferManager::processNotifications -- "
                       << "failed to send death notifications to "
                       << "listener @ 0x" << std::hex
                                          << notification.listener.get()
                       << std::dec << ".";
        } else {
#if LOG_NDEBUG == 0
            std::stringstream inputBufferLog;
            for (const IComponentListener::InputBuffer& inputBuffer :
                    notification.inputBuffers) {
                inputBufferLog << " (" << inputBuffer.frameIndex
                               << ", " << inputBuffer.arrayIndex
                               << ")";
            }
            LOG(VERBOSE) << "InputBufferManager::processNotifications -- "
                         << "death notifications sent to "
                         << "listener @ 0x" << std::hex
                                            << notification.listener.get()
                                            << std::dec
                         << " with these (frameIndex, bufferIndex) pairs:"
                         << inputBufferLog.str();
#endif
        }
    }
#if LOG_NDEBUG == 0
    if (retry) {
        LOG(VERBOSE) << "InputBufferManager::processNotifications -- "
                     << "will retry again in " << *timeToRetryNs << "ns.";
    } else {
        LOG(VERBOSE) << "InputBufferManager::processNotifications -- "
                     << "no pending death notifications.";
    }
#endif
    return retry;
}

void InputBufferManager::main() {
    LOG(VERBOSE) << "InputBufferManager main -- started.";
    nsecs_t timeToRetryNs;
    while (true) {
        std::unique_lock<std::mutex> lock(mMutex);
        while (mDeathNotifications.empty()) {
            mOnBufferDestroyed.wait(lock);
        }
        lock.unlock();
        while (processNotifications(&timeToRetryNs)) {
            std::this_thread::sleep_for(
                    std::chrono::nanoseconds(timeToRetryNs));
        }
    }
}

InputBufferManager::InputBufferManager()
      : mMainThread{&InputBufferManager::main, this} {
}

InputBufferManager& InputBufferManager::getInstance() {
    static InputBufferManager instance{};
    return instance;
}

}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android



