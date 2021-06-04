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

#undef LOG_TAG
#define LOG_TAG "OccupancyTracker"

#include <gui/OccupancyTracker.h>
#include <binder/Parcel.h>
#include <utils/String8.h>
#include <utils/Trace.h>

#include <inttypes.h>

namespace android {

status_t OccupancyTracker::Segment::writeToParcel(Parcel* parcel) const {
    status_t result = parcel->writeInt64(totalTime);
    if (result != OK) {
        return result;
    }
    result = parcel->writeUint64(static_cast<uint64_t>(numFrames));
    if (result != OK) {
        return result;
    }
    result = parcel->writeFloat(occupancyAverage);
    if (result != OK) {
        return result;
    }
    return parcel->writeBool(usedThirdBuffer);
}

status_t OccupancyTracker::Segment::readFromParcel(const Parcel* parcel) {
    status_t result = parcel->readInt64(&totalTime);
    if (result != OK) {
        return result;
    }
    uint64_t uintNumFrames = 0;
    result = parcel->readUint64(&uintNumFrames);
    if (result != OK) {
        return result;
    }
    numFrames = static_cast<size_t>(uintNumFrames);
    result = parcel->readFloat(&occupancyAverage);
    if (result != OK) {
        return result;
    }
    return parcel->readBool(&usedThirdBuffer);
}

void OccupancyTracker::registerOccupancyChange(size_t occupancy) {
    ATRACE_CALL();
    nsecs_t now = systemTime();
    nsecs_t delta = now - mLastOccupancyChangeTime;
    if (delta > NEW_SEGMENT_DELAY) {
        recordPendingSegment();
    } else {
        mPendingSegment.totalTime += delta;
        if (mPendingSegment.mOccupancyTimes.count(mLastOccupancy)) {
            mPendingSegment.mOccupancyTimes[mLastOccupancy] += delta;
        } else {
            mPendingSegment.mOccupancyTimes[mLastOccupancy] = delta;
        }
    }
    if (occupancy > mLastOccupancy) {
        ++mPendingSegment.numFrames;
    }
    mLastOccupancyChangeTime = now;
    mLastOccupancy = occupancy;
}

std::vector<OccupancyTracker::Segment> OccupancyTracker::getSegmentHistory(
        bool forceFlush) {
    if (forceFlush) {
        recordPendingSegment();
    }
    std::vector<Segment> segments(mSegmentHistory.cbegin(),
            mSegmentHistory.cend());
    mSegmentHistory.clear();
    return segments;
}

void OccupancyTracker::recordPendingSegment() {
    // Only record longer segments to get a better measurement of actual double-
    // vs. triple-buffered time
    if (mPendingSegment.numFrames > LONG_SEGMENT_THRESHOLD) {
        float occupancyAverage = 0.0f;
        bool usedThirdBuffer = false;
        for (const auto& timePair : mPendingSegment.mOccupancyTimes) {
            size_t occupancy = timePair.first;
            float timeRatio = static_cast<float>(timePair.second) /
                    mPendingSegment.totalTime;
            occupancyAverage += timeRatio * occupancy;
            usedThirdBuffer = usedThirdBuffer || (occupancy > 1);
        }
        mSegmentHistory.push_front({mPendingSegment.totalTime,
                mPendingSegment.numFrames, occupancyAverage, usedThirdBuffer});
        if (mSegmentHistory.size() > MAX_HISTORY_SIZE) {
            mSegmentHistory.pop_back();
        }
    }
    mPendingSegment.clear();
}

} // namespace android
