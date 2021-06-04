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

#include "EventQueue.h"
#include "utils.h"

#include <utils/Looper.h>

namespace android {
namespace frameworks {
namespace sensorservice {
namespace V1_0 {
namespace implementation {

class EventQueueLooperCallback : public ::android::LooperCallback {
public:
    EventQueueLooperCallback(sp<::android::SensorEventQueue> queue,
                             sp<IEventQueueCallback> callback)
            : mQueue(queue), mCallback(callback) {
    }

    int handleEvent(__unused int fd, __unused int events, __unused void* data) {

        ASensorEvent event;
        ssize_t actual;

        auto internalQueue = mQueue.promote();
        if (internalQueue == nullptr) {
            return 1;
        }

        while ((actual = internalQueue->read(&event, 1 /* count */)) > 0) {
            internalQueue->sendAck(&event, actual);
            Return<void> ret = mCallback->onEvent(convertEvent(event));
            (void)ret.isOk(); // ignored
        }

        return 1; // continue to receive callbacks
    }

private:
    wp<::android::SensorEventQueue> mQueue;
    sp<IEventQueueCallback> mCallback;
};

EventQueue::EventQueue(
        sp<IEventQueueCallback> callback,
        sp<::android::Looper> looper,
        sp<::android::SensorEventQueue> internalQueue)
            : mLooper(looper),
              mInternalQueue(internalQueue) {

    mLooper->addFd(internalQueue->getFd(), ALOOPER_POLL_CALLBACK, ALOOPER_EVENT_INPUT,
            new EventQueueLooperCallback(internalQueue, callback), nullptr /* data */);
}

void EventQueue::onLastStrongRef(const void *id) {
    IEventQueue::onLastStrongRef(id);
    mLooper->removeFd(mInternalQueue->getFd());
}

// Methods from ::android::frameworks::sensorservice::V1_0::IEventQueue follow.
Return<Result> EventQueue::enableSensor(int32_t sensorHandle, int32_t samplingPeriodUs,
        int64_t maxBatchReportLatencyUs) {
    return convertResult(mInternalQueue->enableSensor(sensorHandle, samplingPeriodUs,
            maxBatchReportLatencyUs, 0 /* reserved flags */));
}

Return<Result> EventQueue::disableSensor(int32_t sensorHandle) {
    return convertResult(mInternalQueue->disableSensor(sensorHandle));
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace sensorservice
}  // namespace frameworks
}  // namespace android
