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

// for random()
#include <stdlib.h>

#include "TimestampScheduler.h"

using namespace aaudio;

void TimestampScheduler::start(int64_t startTime) {
    mStartTime = startTime;
    mLastTime = startTime;
}

int64_t TimestampScheduler::nextAbsoluteTime() {
    int64_t periodsElapsed = (mLastTime - mStartTime) / mBurstPeriod;
    // This is an arbitrary schedule that could probably be improved.
    // It starts out sending a timestamp on every period because we want to
    // get an accurate picture when the stream starts. Then it slows down
    // to the occasional timestamps needed to detect a slow drift.
    int64_t minPeriodsToDelay = (periodsElapsed < 10) ? 1 :
        (periodsElapsed < 100) ? 3 :
        (periodsElapsed < 1000) ? 10 : 50;
    int64_t sleepTime = minPeriodsToDelay * mBurstPeriod;
    // Generate a random rectangular distribution one burst wide so that we get
    // an uncorrelated sampling of the MMAP pointer.
    sleepTime += (int64_t)(random() * mBurstPeriod / RAND_MAX);
    mLastTime += sleepTime;
    return mLastTime;
}
