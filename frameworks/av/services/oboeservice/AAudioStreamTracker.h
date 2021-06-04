/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef AAUDIO_AAUDIO_STREAM_TRACKER_H
#define AAUDIO_AAUDIO_STREAM_TRACKER_H

#include <time.h>
#include <pthread.h>

#include <aaudio/AAudio.h>

#include "binding/AAudioCommon.h"

#include "AAudioServiceStreamBase.h"

namespace aaudio {

class AAudioStreamTracker {

public:
    /**
     * Find the stream associated with the handle.
     * Decrement its reference counter. If zero and the stream needs
     * to be closed then remove the stream and return a pointer to the stream.
     * Otherwise return null if it does not need to be closed.
     *
     * @param streamHandle
     * @return strong pointer to the stream if it needs to be closed, or nullptr
     */
    android::sp<AAudioServiceStreamBase> decrementAndRemoveStreamByHandle(
            aaudio_handle_t streamHandle);

    /**
     * Look up a stream based on the handle.
     * Increment its service reference count if found.
     *
     * @param streamHandle
     * @return strong pointer to the stream if found, or nullptr
     */
    android::sp<aaudio::AAudioServiceStreamBase> getStreamByHandleAndIncrement(
            aaudio_handle_t streamHandle);

    /**
     * Look up a stream based on the AudioPolicy portHandle.
     * Increment its service reference count if found.
     *
     * @param portHandle
     * @return strong pointer to the stream if found, or nullptr
     */
    android::sp<aaudio::AAudioServiceStreamBase> findStreamByPortHandleAndIncrement(
            audio_port_handle_t portHandle);

    /**
     * Store a strong pointer to the stream and return a unique handle for future reference.
     * The handle is guaranteed not to collide with an existing stream.
     * @param serviceStream
     * @return handle for identifying the stream
     */
    aaudio_handle_t addStreamForHandle(android::sp<AAudioServiceStreamBase> serviceStream);

    /**
     * @return string that can be added to dumpsys
     */
    std::string dump() const;

private:
    static aaudio_handle_t bumpHandle(aaudio_handle_t handle);

    // Track stream using a unique handle that wraps. Only use positive half.
    mutable std::mutex                mHandleLock;
    // protected by mHandleLock
    aaudio_handle_t                   mPreviousHandle = 0;
    // protected by mHandleLock
    std::map<aaudio_handle_t, android::sp<aaudio::AAudioServiceStreamBase>> mStreamsByHandle;
};


} /* namespace aaudio */

#endif /* AAUDIO_AAUDIO_STREAM_TRACKER_H */
