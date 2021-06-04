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

#ifndef A_LOOPER_H_

#define A_LOOPER_H_

#include <android-base/macros.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>

#include <set>

struct ASensorEventQueue;

struct ALooper {
    ALooper();

    void signalSensorEvents(ASensorEventQueue *queue);
    void wake();

    int pollOnce(int timeoutMillis, int *outFd, int *outEvents, void **outData);

    void invalidateSensorQueue(ASensorEventQueue *queue);

private:
    android::Mutex mLock;
    android::Condition mCondition;

    std::set<ASensorEventQueue *> mReadyQueues;
    bool mAwoken;

    DISALLOW_COPY_AND_ASSIGN(ALooper);
};

#endif  // A_LOOPER_H_
