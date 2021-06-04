/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "ASensorEventQueue.h"

#include "ALooper.h"

#define LOG_TAG "libsensorndkbridge"
#include <android-base/logging.h>

using android::sp;
using android::frameworks::sensorservice::V1_0::Result;
using android::hardware::sensors::V1_0::SensorInfo;
using android::OK;
using android::BAD_VALUE;
using android::Mutex;
using android::hardware::Return;

ASensorEventQueue::ASensorEventQueue(ALooper* looper, ALooper_callbackFunc callback, void* data)
    : mLooper(looper), mCallback(callback), mData(data), mRequestAdditionalInfo(false) {}

void ASensorEventQueue::setImpl(const sp<IEventQueue> &queueImpl) {
    mQueueImpl = queueImpl;
}

int ASensorEventQueue::registerSensor(
        ASensorRef sensor,
        int32_t samplingPeriodUs,
        int64_t maxBatchReportLatencyUs) {
    Return<Result> ret = mQueueImpl->enableSensor(
            reinterpret_cast<const SensorInfo *>(sensor)->sensorHandle,
            samplingPeriodUs,
            maxBatchReportLatencyUs);

    if (!ret.isOk()) {
        return BAD_VALUE;
    }

    return OK;
}

int ASensorEventQueue::enableSensor(ASensorRef sensor) {
    static constexpr int32_t SENSOR_DELAY_NORMAL = 200000;

    return registerSensor(
            sensor, SENSOR_DELAY_NORMAL, 0 /* maxBatchReportLatencyUs */);
}

int ASensorEventQueue::setEventRate(
        ASensorRef sensor, int32_t samplingPeriodUs) {
    // Technically this is not supposed to enable the sensor but using this
    // API without enabling the sensor first is a no-op, so...
    return registerSensor(
            sensor, samplingPeriodUs, 0 /* maxBatchReportLatencyUs */);
}

int ASensorEventQueue::requestAdditionalInfoEvents(bool enable) {
    mRequestAdditionalInfo = enable;
    return OK;
}

int ASensorEventQueue::disableSensor(ASensorRef sensor) {
    Return<Result> ret = mQueueImpl->disableSensor(
            reinterpret_cast<const SensorInfo *>(sensor)->sensorHandle);

    return ret.isOk() ? OK : BAD_VALUE;
}

ssize_t ASensorEventQueue::getEvents(ASensorEvent *events, size_t count) {
    // XXX Should this block if there aren't any events in the queue?

    Mutex::Autolock autoLock(mLock);

    static_assert(
            sizeof(ASensorEvent) == sizeof(sensors_event_t), "mismatched size");

    size_t copy = std::min(count, mQueue.size());
    for (size_t i = 0; i < copy; ++i) {
        reinterpret_cast<sensors_event_t *>(events)[i] = mQueue[i];
    }
    mQueue.erase(mQueue.begin(), mQueue.begin() + copy);

    LOG(VERBOSE) << "ASensorEventQueue::getEvents() returned " << copy << " events.";

    return copy;
}

int ASensorEventQueue::hasEvents() const {
    return !mQueue.empty();
}

Return<void> ASensorEventQueue::onEvent(const Event &event) {
    LOG(VERBOSE) << "ASensorEventQueue::onEvent";

    if (static_cast<int32_t>(event.sensorType) != ASENSOR_TYPE_ADDITIONAL_INFO ||
        mRequestAdditionalInfo.load()) {
        {
            Mutex::Autolock autoLock(mLock);

            mQueue.emplace_back();
            sensors_event_t* sensorEvent = &mQueue[mQueue.size() - 1];
            android::hardware::sensors::V1_0::implementation::convertToSensorEvent(event,
                                                                                   sensorEvent);
        }

        mLooper->signalSensorEvents(this);
    }

    return android::hardware::Void();
}

void ASensorEventQueue::dispatchCallback() {
    if (mCallback != NULL) {
        int res = (*mCallback)(-1 /* fd */, ALOOPER_EVENT_INPUT, mData);

        if (res == 0) {
            mCallback = NULL;
            mData = NULL;
        }
    }
}

void ASensorEventQueue::invalidate() {
    mLooper->invalidateSensorQueue(this);
    setImpl(nullptr);
}

