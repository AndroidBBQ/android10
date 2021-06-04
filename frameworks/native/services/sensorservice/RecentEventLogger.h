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

#ifndef ANDROID_SENSOR_SERVICE_UTIL_RECENT_EVENT_LOGGER_H
#define ANDROID_SENSOR_SERVICE_UTIL_RECENT_EVENT_LOGGER_H

#include "RingBuffer.h"
#include "SensorServiceUtils.h"

#include <hardware/sensors.h>
#include <utils/String8.h>

#include <mutex>

namespace android {
namespace SensorServiceUtil {

// A circular buffer that record the last N events of a sensor type for debugging. The size of this
// buffer depends on sensor type and is controlled by logSizeBySensorType(). The last N events
// generated from the sensor are stored in this buffer.  The buffer is NOT cleared when the sensor
// unregisters and as a result very old data in the dumpsys output can be seen, which is an intended
// behavior.
class RecentEventLogger : public Dumpable {
public:
    explicit RecentEventLogger(int sensorType);
    void addEvent(const sensors_event_t& event);

    // Populate event with the last recorded sensor event if it is not stale. An event is
    // considered stale if the sensor has become deactivated since the event was recorded.
    // returns true on success, false if no recent event is available or the last event is stale
    bool populateLastEventIfCurrent(sensors_event_t *event) const;
    bool isEmpty() const;
    void setLastEventStale();
    virtual ~RecentEventLogger() {}

    // Dumpable interface
    virtual std::string dump() const override;
    virtual void setFormat(std::string format) override;

protected:
    struct SensorEventLog {
        explicit SensorEventLog(const sensors_event_t& e);
        timespec mWallTime;
        sensors_event_t mEvent;
    };

    const int mSensorType;
    const size_t mEventSize;

    mutable std::mutex mLock;
    RingBuffer<SensorEventLog> mRecentEvents;

    bool mMaskData;
    bool mIsLastEventCurrent;

private:
    static size_t logSizeBySensorType(int sensorType);
};

} // namespace SensorServiceUtil
} // namespace android;

#endif // ANDROID_SENSOR_SERVICE_UTIL_RECENT_EVENT_LOGGER_H

