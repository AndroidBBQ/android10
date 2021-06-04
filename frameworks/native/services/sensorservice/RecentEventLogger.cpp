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

#include "RecentEventLogger.h"
#include "SensorServiceUtils.h"

#include <utils/Timers.h>

#include <inttypes.h>

namespace android {
namespace SensorServiceUtil {

namespace {
    constexpr size_t LOG_SIZE = 10;
    constexpr size_t LOG_SIZE_MED = 30;  // debugging for slower sensors
    constexpr size_t LOG_SIZE_LARGE = 50;  // larger samples for debugging
}// unnamed namespace

RecentEventLogger::RecentEventLogger(int sensorType) :
        mSensorType(sensorType), mEventSize(eventSizeBySensorType(mSensorType)),
        mRecentEvents(logSizeBySensorType(sensorType)), mMaskData(false),
        mIsLastEventCurrent(false) {
    // blank
}

void RecentEventLogger::addEvent(const sensors_event_t& event) {
    std::lock_guard<std::mutex> lk(mLock);
    mRecentEvents.emplace(event);
    mIsLastEventCurrent = true;
}

bool RecentEventLogger::isEmpty() const {
    return mRecentEvents.size() == 0;
}

void RecentEventLogger::setLastEventStale() {
    std::lock_guard<std::mutex> lk(mLock);
    mIsLastEventCurrent = false;
}

std::string RecentEventLogger::dump() const {
    std::lock_guard<std::mutex> lk(mLock);

    //TODO: replace String8 with std::string completely in this function
    String8 buffer;

    buffer.appendFormat("last %zu events\n", mRecentEvents.size());
    int j = 0;
    for (int i = mRecentEvents.size() - 1; i >= 0; --i) {
        const auto& ev = mRecentEvents[i];
        struct tm * timeinfo = localtime(&(ev.mWallTime.tv_sec));
        buffer.appendFormat("\t%2d (ts=%.9f, wall=%02d:%02d:%02d.%03d) ",
                ++j, ev.mEvent.timestamp/1e9, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
                (int) ns2ms(ev.mWallTime.tv_nsec));

        // data
        if (!mMaskData) {
            if (mSensorType == SENSOR_TYPE_STEP_COUNTER) {
                buffer.appendFormat("%" PRIu64 ", ", ev.mEvent.u64.step_counter);
            } else {
                for (size_t k = 0; k < mEventSize; ++k) {
                    buffer.appendFormat("%.2f, ", ev.mEvent.data[k]);
                }
            }
        } else {
            buffer.append("[value masked]");
        }
        buffer.append("\n");
    }
    return std::string(buffer.string());
}

void RecentEventLogger::setFormat(std::string format) {
    if (format == "mask_data" ) {
        mMaskData = true;
    } else {
        mMaskData = false;
    }
}

bool RecentEventLogger::populateLastEventIfCurrent(sensors_event_t *event) const {
    std::lock_guard<std::mutex> lk(mLock);

    if (mIsLastEventCurrent && mRecentEvents.size()) {
        // Index 0 contains the latest event emplace()'ed
        *event = mRecentEvents[0].mEvent;
        return true;
    } else {
        return false;
    }
}


size_t RecentEventLogger::logSizeBySensorType(int sensorType) {
    if (sensorType == SENSOR_TYPE_STEP_COUNTER ||
        sensorType == SENSOR_TYPE_SIGNIFICANT_MOTION ||
        sensorType == SENSOR_TYPE_ACCELEROMETER ||
        sensorType == SENSOR_TYPE_LIGHT) {
        return LOG_SIZE_LARGE;
    }
    if (sensorType == SENSOR_TYPE_PROXIMITY) {
        return LOG_SIZE_MED;
    }
    return LOG_SIZE;
}

RecentEventLogger::SensorEventLog::SensorEventLog(const sensors_event_t& e) : mEvent(e) {
    clock_gettime(CLOCK_REALTIME, &mWallTime);
}

} // namespace SensorServiceUtil
} // namespace android
