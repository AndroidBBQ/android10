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

#include <ui/FrameStats.h>

namespace android {

bool FrameStats::isFixedSize() const {
    return false;
}

size_t FrameStats::getFlattenedSize() const {
    const size_t timestampSize = sizeof(nsecs_t);

    size_t size = timestampSize;
    size += 3 * desiredPresentTimesNano.size() * timestampSize;

    return size;
}

status_t FrameStats::flatten(void* buffer, size_t size) const {
    if (size < getFlattenedSize()) {
        return NO_MEMORY;
    }

    nsecs_t* timestamps = reinterpret_cast<nsecs_t*>(buffer);
    const size_t timestampSize = sizeof(nsecs_t);
    size_t frameCount = desiredPresentTimesNano.size();

    memcpy(timestamps, &refreshPeriodNano, timestampSize);
    timestamps += 1;

    memcpy(timestamps, desiredPresentTimesNano.array(), frameCount * timestampSize);
    timestamps += frameCount;

    memcpy(timestamps, actualPresentTimesNano.array(), frameCount * timestampSize);
    timestamps += frameCount;

    memcpy(timestamps, frameReadyTimesNano.array(), frameCount * timestampSize);

    return NO_ERROR;
}

status_t FrameStats::unflatten(void const* buffer, size_t size) {
    const size_t timestampSize = sizeof(nsecs_t);

    if (size < timestampSize) {
        return NO_MEMORY;
    }

    nsecs_t const* timestamps = reinterpret_cast<nsecs_t const*>(buffer);
    size_t frameCount = (size - timestampSize) / (3 * timestampSize);

    memcpy(&refreshPeriodNano, timestamps, timestampSize);
    timestamps += 1;

    desiredPresentTimesNano.resize(frameCount);
    memcpy(desiredPresentTimesNano.editArray(), timestamps, frameCount * timestampSize);
    timestamps += frameCount;

    actualPresentTimesNano.resize(frameCount);
    memcpy(actualPresentTimesNano.editArray(), timestamps, frameCount * timestampSize);
    timestamps += frameCount;

    frameReadyTimesNano.resize(frameCount);
    memcpy(frameReadyTimesNano.editArray(), timestamps, frameCount * timestampSize);

    return NO_ERROR;
}

} // namespace android
