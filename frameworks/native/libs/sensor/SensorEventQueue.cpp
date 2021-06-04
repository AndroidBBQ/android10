/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "Sensors"

#include <sensor/SensorEventQueue.h>

#include <algorithm>
#include <sys/socket.h>

#include <utils/RefBase.h>
#include <utils/Looper.h>

#include <sensor/Sensor.h>
#include <sensor/BitTube.h>
#include <sensor/ISensorEventConnection.h>

#include <android/sensor.h>
#include <hardware/sensors-base.h>

using std::min;

// ----------------------------------------------------------------------------
namespace android {
// ----------------------------------------------------------------------------

SensorEventQueue::SensorEventQueue(const sp<ISensorEventConnection>& connection)
    : mSensorEventConnection(connection), mRecBuffer(nullptr), mAvailable(0), mConsumed(0),
      mNumAcksToSend(0) {
    mRecBuffer = new ASensorEvent[MAX_RECEIVE_BUFFER_EVENT_COUNT];
}

SensorEventQueue::~SensorEventQueue() {
    delete [] mRecBuffer;
}

void SensorEventQueue::onFirstRef()
{
    mSensorChannel = mSensorEventConnection->getSensorChannel();
}

int SensorEventQueue::getFd() const
{
    return mSensorChannel->getFd();
}


ssize_t SensorEventQueue::write(const sp<BitTube>& tube,
        ASensorEvent const* events, size_t numEvents) {
    return BitTube::sendObjects(tube, events, numEvents);
}

ssize_t SensorEventQueue::read(ASensorEvent* events, size_t numEvents) {
    if (mAvailable == 0) {
        ssize_t err = BitTube::recvObjects(mSensorChannel,
                mRecBuffer, MAX_RECEIVE_BUFFER_EVENT_COUNT);
        if (err < 0) {
            return err;
        }
        mAvailable = static_cast<size_t>(err);
        mConsumed = 0;
    }
    size_t count = min(numEvents, mAvailable);
    memcpy(events, mRecBuffer + mConsumed, count * sizeof(ASensorEvent));
    mAvailable -= count;
    mConsumed += count;
    return static_cast<ssize_t>(count);
}

sp<Looper> SensorEventQueue::getLooper() const
{
    Mutex::Autolock _l(mLock);
    if (mLooper == nullptr) {
        mLooper = new Looper(true);
        mLooper->addFd(getFd(), getFd(), ALOOPER_EVENT_INPUT, nullptr, nullptr);
    }
    return mLooper;
}

status_t SensorEventQueue::waitForEvent() const
{
    const int fd = getFd();
    sp<Looper> looper(getLooper());

    int events;
    int32_t result;
    do {
        result = looper->pollOnce(-1, nullptr, &events, nullptr);
        if (result == ALOOPER_POLL_ERROR) {
            ALOGE("SensorEventQueue::waitForEvent error (errno=%d)", errno);
            result = -EPIPE; // unknown error, so we make up one
            break;
        }
        if (events & ALOOPER_EVENT_HANGUP) {
            // the other-side has died
            ALOGE("SensorEventQueue::waitForEvent error HANGUP");
            result = -EPIPE; // unknown error, so we make up one
            break;
        }
    } while (result != fd);

    return  (result == fd) ? status_t(NO_ERROR) : result;
}

status_t SensorEventQueue::wake() const
{
    sp<Looper> looper(getLooper());
    looper->wake();
    return NO_ERROR;
}

status_t SensorEventQueue::enableSensor(Sensor const* sensor) const {
    return enableSensor(sensor, SENSOR_DELAY_NORMAL);
}

status_t SensorEventQueue::enableSensor(Sensor const* sensor, int32_t samplingPeriodUs) const {
    return mSensorEventConnection->enableDisable(sensor->getHandle(), true,
                                                 us2ns(samplingPeriodUs), 0, 0);
}

status_t SensorEventQueue::disableSensor(Sensor const* sensor) const {
    return mSensorEventConnection->enableDisable(sensor->getHandle(), false, 0, 0, 0);
}

status_t SensorEventQueue::enableSensor(int32_t handle, int32_t samplingPeriodUs,
                                        int64_t maxBatchReportLatencyUs, int reservedFlags) const {
    return mSensorEventConnection->enableDisable(handle, true, us2ns(samplingPeriodUs),
                                                 us2ns(maxBatchReportLatencyUs), reservedFlags);
}

status_t SensorEventQueue::flush() const {
    return mSensorEventConnection->flush();
}

status_t SensorEventQueue::disableSensor(int32_t handle) const {
    return mSensorEventConnection->enableDisable(handle, false, 0, 0, false);
}

status_t SensorEventQueue::setEventRate(Sensor const* sensor, nsecs_t ns) const {
    return mSensorEventConnection->setEventRate(sensor->getHandle(), ns);
}

status_t SensorEventQueue::injectSensorEvent(const ASensorEvent& event) {
    do {
        // Blocking call.
        ssize_t size = ::send(mSensorChannel->getFd(), &event, sizeof(event), MSG_NOSIGNAL);
        if (size >= 0) {
            return NO_ERROR;
        } else if (size < 0 && errno == EAGAIN) {
            // If send is returning a "Try again" error, sleep for 100ms and try again. In all
            // other cases log a failure and exit.
            usleep(100000);
        } else {
            ALOGE("injectSensorEvent failure %s %zd", strerror(errno), size);
            return INVALID_OPERATION;
        }
    } while (true);
}

void SensorEventQueue::sendAck(const ASensorEvent* events, int count) {
    for (int i = 0; i < count; ++i) {
        if (events[i].flags & WAKE_UP_SENSOR_EVENT_NEEDS_ACK) {
            ++mNumAcksToSend;
        }
    }
    // Send mNumAcksToSend to acknowledge for the wake up sensor events received.
    if (mNumAcksToSend > 0) {
        ssize_t size = ::send(mSensorChannel->getFd(), &mNumAcksToSend, sizeof(mNumAcksToSend),
                MSG_DONTWAIT | MSG_NOSIGNAL);
        if (size < 0) {
            ALOGE("sendAck failure %zd %d", size, mNumAcksToSend);
        } else {
            mNumAcksToSend = 0;
        }
    }
    return;
}

ssize_t SensorEventQueue::filterEvents(ASensorEvent* events, size_t count) const {
    // Check if this Sensor Event Queue is registered to receive each type of event. If it is not,
    // then do not copy the event into the final buffer. Minimize the number of copy operations by
    // finding consecutive sequences of events that the Sensor Event Queue should receive and only
    // copying the events once an unregistered event type is reached.
    bool intervalStartLocSet = false;
    size_t intervalStartLoc = 0;
    size_t eventsInInterval = 0;
    ssize_t eventsCopied = 0;

    for (size_t i = 0; i < count; i++) {
        bool includeEvent =
                (events[i].type != SENSOR_TYPE_ADDITIONAL_INFO || requestAdditionalInfo);

        if (includeEvent) {
            // Do not copy events yet since there may be more consecutive events that should be
            // copied together. Track the start location and number of events in the current
            // sequence.
            if (!intervalStartLocSet) {
                intervalStartLoc = i;
                intervalStartLocSet = true;
                eventsInInterval = 0;
            }
            eventsInInterval++;
        }

        // Shift the events from the already processed interval once an event that should not be
        // included is reached or if this is the final event to be processed.
        if (!includeEvent || (i + 1 == count)) {
            // Only shift the events if the interval did not start with the first event. If the
            // interval started with the first event, the events are already in their correct
            // location.
            if (intervalStartLoc > 0) {
                memmove(&events[eventsCopied], &events[intervalStartLoc],
                        eventsInInterval * sizeof(ASensorEvent));
            }
            eventsCopied += eventsInInterval;

            // Reset the interval information
            eventsInInterval = 0;
            intervalStartLocSet = false;
        }
    }
    return eventsCopied;
}

// ----------------------------------------------------------------------------
}; // namespace android

