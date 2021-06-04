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

#ifndef CODEC2_HIDL_V1_0_UTILS_INPUT_BUFFER_MANAGER_H
#define CODEC2_HIDL_V1_0_UTILS_INPUT_BUFFER_MANAGER_H

#include <android/hardware/media/c2/1.0/IComponentListener.h>
#include <utils/Timers.h>

#include <C2Buffer.h>
#include <C2Work.h>

#include <set>
#include <map>
#include <thread>

namespace android {
namespace hardware {
namespace media {
namespace c2 {
namespace V1_0 {
namespace utils {

using namespace ::android;

/**
 * InputBufferManager
 * ==================
 *
 * InputBufferManager presents a way to track and untrack input buffers in this
 * (codec) process and send a notification to a listener, possibly in a
 * different process, when a tracked buffer no longer has any references in this
 * process.
 *
 * InputBufferManager holds a collection of records representing tracked buffers
 * and their callback listeners. Conceptually, one record is a triple (listener,
 * frameIndex, bufferIndex) where
 *
 * - (frameIndex, bufferIndex) is a pair of indices used to identify the buffer.
 * - listener is of type IComponentListener. Its onInputBuffersReleased()
 *   function will be called after the associated buffer dies. The argument of
 *   onInputBuffersReleased() is a list of InputBuffer objects, each of which
 *   has the following members:
 *
 *     uint64_t frameIndex
 *     uint32_t arrayIndex
 *
 * When a tracked buffer associated to the triple (listener, frameIndex,
 * bufferIndex) goes out of scope, listener->onInputBuffersReleased() will be
 * called with an InputBuffer object whose members are set as follows:
 *
 *     inputBuffer.frameIndex = frameIndex
 *     inputBuffer.arrayIndex = bufferIndex
 *
 * IPC Optimization
 * ----------------
 *
 * Since onInputBuffersReleased() is an IPC call, InputBufferManager tries not
 * to call it too often. Any two calls to the same listener are at least
 * mNotificationIntervalNs nanoseconds apart, where mNotificationIntervalNs is
 * configurable via calling setNotificationInterval(). The default value of
 * mNotificationIntervalNs is kDefaultNotificationInternalNs.
 *
 * Public Member Functions
 * -----------------------
 *
 * InputBufferManager is a singleton class. Its only instance is accessible via
 * the following public functions:
 *
 * - registerFrameData(const sp<IComponentListener>& listener,
 *                     const C2FrameData& input)
 *
 * - unregisterFrameData(const sp<IComponentListener>& listener,
 *                       const C2FrameData& input)
 *
 * - unregisterFrameData(const sp<IComponentListener>& listener)
 *
 * - setNotificationInterval(nsecs_t notificationIntervalNs)
 *
 */

struct InputBufferManager {

    /**
     * The default value for the time interval between 2 subsequent IPCs.
     */
    static constexpr nsecs_t kDefaultNotificationIntervalNs = 1000000; /* 1ms */

    /**
     * Track all buffers in a C2FrameData object.
     *
     * input (C2FrameData) has the following two members that are of interest:
     *
     *   C2WorkOrdinal                ordinal
     *   vector<shared_ptr<C2Buffer>> buffers
     *
     * Calling registerFrameData(listener, input) will register multiple
     * triples (listener, frameIndex, bufferIndex) where frameIndex is equal to
     * input.ordinal.frameIndex and bufferIndex runs through the indices of
     * input.buffers such that input.buffers[bufferIndex] is not null.
     *
     * This should be called from queue().
     *
     * \param listener Listener of death notifications.
     * \param input Input frame data whose input buffers are to be tracked.
     */
    static void registerFrameData(
            const sp<IComponentListener>& listener,
            const C2FrameData& input);

    /**
     * Untrack all buffers in a C2FrameData object.
     *
     * Calling unregisterFrameData(listener, input) will unregister and remove
     * pending notifications for all triples (l, fi, bufferIndex) such that
     * l = listener and fi = input.ordinal.frameIndex.
     *
     * This should be called from onWorkDone() and flush().
     *
     * \param listener Previously registered listener.
     * \param input Previously registered frame data.
     */
    static void unregisterFrameData(
            const wp<IComponentListener>& listener,
            const C2FrameData& input);

    /**
     * Untrack all buffers associated to a given listener.
     *
     * Calling unregisterFrameData(listener) will unregister and remove
     * pending notifications for all triples (l, frameIndex, bufferIndex) such
     * that l = listener.
     *
     * This should be called when the component cleans up all input buffers,
     * i.e., when reset(), release(), stop() or ~Component() is called.
     *
     * \param listener Previously registered listener.
     */
    static void unregisterFrameData(
            const wp<IComponentListener>& listener);

    /**
     * Set the notification interval.
     *
     * \param notificationIntervalNs New notification interval, in nanoseconds.
     */
    static void setNotificationInterval(nsecs_t notificationIntervalNs);

private:
    void _registerFrameData(
            const sp<IComponentListener>& listener,
            const C2FrameData& input);
    void _unregisterFrameData(
            const wp<IComponentListener>& listener,
            const C2FrameData& input);
    void _unregisterFrameData(
            const wp<IComponentListener>& listener);
    void _setNotificationInterval(nsecs_t notificationIntervalNs);

    // The callback function tied to C2Buffer objects.
    //
    // Note: This function assumes that sInstance is the only instance of this
    //       class.
    static void onBufferDestroyed(const C2Buffer* buf, void* arg);
    void _onBufferDestroyed(const C2Buffer* buf, void* arg);

    // Persistent data to be passed as "arg" in onBufferDestroyed().
    // This is essentially the triple (listener, frameIndex, bufferIndex) plus a
    // weak pointer to the C2Buffer object.
    //
    // Note that the "key" is bufferIndex according to operator<(). This is
    // designed to work with TrackedBuffersMap defined below.
    struct TrackedBuffer {
        wp<IComponentListener> listener;
        uint64_t frameIndex;
        size_t bufferIndex;
        std::weak_ptr<C2Buffer> buffer;
        TrackedBuffer(const wp<IComponentListener>& listener,
                      uint64_t frameIndex,
                      size_t bufferIndex,
                      const std::shared_ptr<C2Buffer>& buffer)
              : listener(listener),
                frameIndex(frameIndex),
                bufferIndex(bufferIndex),
                buffer(buffer) {}
    };

    // Map: listener -> frameIndex -> set<TrackedBuffer*>.
    // Essentially, this is used to store triples (listener, frameIndex,
    // bufferIndex) that's searchable by listener and (listener, frameIndex).
    // However, the value of the innermost map is TrackedBuffer, which also
    // contains an extra copy of listener and frameIndex. This is needed
    // because onBufferDestroyed() needs to know listener and frameIndex too.
    typedef std::map<wp<IComponentListener>,
                     std::map<uint64_t,
                              std::set<TrackedBuffer*>>> TrackedBuffersMap;

    // Storage for pending (unsent) death notifications for one listener.
    // Each pair in member named "indices" are (frameIndex, bufferIndex) from
    // the (listener, frameIndex, bufferIndex) triple.
    struct DeathNotifications {

        // The number of pending notifications for this listener.
        // count may be 0, in which case the DeathNotifications object will
        // remain valid for only a small period (specified
        // nanoseconds).
        size_t count;

        // The timestamp of the most recent callback on this listener. This is
        // used to guarantee that callbacks do not occur too frequently, and
        // also to trigger expiration of a DeathNotifications object that has
        // count = 0.
        nsecs_t lastSentNs;

        // Map: frameIndex -> vector of bufferIndices
        // This is essentially a collection of (framdeIndex, bufferIndex).
        std::map<uint64_t, std::vector<size_t>> indices;

        DeathNotifications(
                nsecs_t notificationIntervalNs = kDefaultNotificationIntervalNs)
              : count(0),
                lastSentNs(systemTime() - notificationIntervalNs),
                indices() {}
    };

    // The minimum time period between IPC calls to notify the client about the
    // destruction of input buffers.
    std::atomic<nsecs_t> mNotificationIntervalNs{kDefaultNotificationIntervalNs};

    // Mutex for the management of all input buffers.
    std::mutex mMutex;

    // Cache for all TrackedBuffers.
    //
    // Whenever registerOnDestroyNotify() is called, an argument of type
    // TrackedBuffer is created and stored into this cache.
    // Whenever unregisterOnDestroyNotify() or onBufferDestroyed() is called,
    // the TrackedBuffer is removed from this cache.
    //
    // mTrackedBuffersMap stores references to TrackedBuffers inside this cache.
    std::set<TrackedBuffer*> mTrackedBufferCache;

    // Tracked input buffers.
    TrackedBuffersMap mTrackedBuffersMap;

    // Death notifications to be sent.
    //
    // A DeathNotifications object is associated to each listener. An entry in
    // this map will be removed if its associated DeathNotifications has count =
    // 0 and lastSentNs < systemTime() - mNotificationIntervalNs.
    std::map<wp<IComponentListener>, DeathNotifications> mDeathNotifications;

    // Condition variable signaled when an entry is added to mDeathNotifications.
    std::condition_variable mOnBufferDestroyed;

    // Notify the clients about buffer destructions.
    // Return false if all destructions have been notified.
    // Return true and set timeToRetry to the duration to wait for before
    // retrying if some destructions have not been notified.
    bool processNotifications(nsecs_t* timeToRetryNs);

    // Main function for the input buffer manager thread.
    void main();

    // The thread that manages notifications.
    //
    // Note: This variable is declared last so its initialization will happen
    // after all other member variables have been initialized.
    std::thread mMainThread;

    // Private constructor.
    InputBufferManager();

    // The only instance of this class.
    static InputBufferManager& getInstance();

};

}  // namespace utils
}  // namespace V1_0
}  // namespace c2
}  // namespace media
}  // namespace hardware
}  // namespace android

#endif  // CODEC2_HIDL_V1_0_UTILS_INPUT_BUFFER_MANAGER_H

