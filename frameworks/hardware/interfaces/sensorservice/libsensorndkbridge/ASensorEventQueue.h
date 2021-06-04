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

#ifndef A_SENSOR_EVENT_QUEUE_H_

#define A_SENSOR_EVENT_QUEUE_H_

#include <android/frameworks/sensorservice/1.0/IEventQueue.h>
#include <android/frameworks/sensorservice/1.0/IEventQueueCallback.h>
#include <android/looper.h>
#include <android/sensor.h>
#include <android-base/macros.h>
#include <sensors/convert.h>
#include <utils/Mutex.h>

#include <atomic>

struct ALooper;

struct ASensorEventQueue
    : public android::frameworks::sensorservice::V1_0::IEventQueueCallback {
    using Event = android::hardware::sensors::V1_0::Event;
    using IEventQueue = android::frameworks::sensorservice::V1_0::IEventQueue;

    ASensorEventQueue(
            ALooper *looper,
            ALooper_callbackFunc callback,
            void *data);

    android::hardware::Return<void> onEvent(const Event &event) override;

    void setImpl(const android::sp<IEventQueue> &queueImpl);

    int registerSensor(
            ASensorRef sensor,
            int32_t samplingPeriodUs,
            int64_t maxBatchReportLatencyUs);

    int enableSensor(ASensorRef sensor);
    int disableSensor(ASensorRef sensor);

    int setEventRate(ASensorRef sensor, int32_t samplingPeriodUs);

    int requestAdditionalInfoEvents(bool enable);

    ssize_t getEvents(ASensorEvent *events, size_t count);
    int hasEvents() const;

    void dispatchCallback();

    void invalidate();

private:
    ALooper *mLooper;
    ALooper_callbackFunc mCallback;
    void *mData;
    android::sp<IEventQueue> mQueueImpl;

    android::Mutex mLock;
    std::vector<sensors_event_t> mQueue;

    std::atomic_bool mRequestAdditionalInfo;

    DISALLOW_COPY_AND_ASSIGN(ASensorEventQueue);
};

#endif  // A_SENSOR_EVENT_QUEUE_H_

