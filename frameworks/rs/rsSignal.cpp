/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include "rsSignal.h"
#include <errno.h>

namespace android {
namespace renderscript {

Signal::Signal() {
    mSet = true;
}

Signal::~Signal() {
    pthread_mutex_destroy(&mMutex);
    pthread_cond_destroy(&mCondition);
}

bool Signal::init() {
    int status = pthread_mutex_init(&mMutex, nullptr);
    if (status) {
        ALOGE("Signal::init: mutex init failure: %s", strerror(status));
        return false;
    }

    status = pthread_cond_init(&mCondition, nullptr);
    if (status) {
        ALOGE("Signal::init: condition init failure: %s", strerror(status));
        pthread_mutex_destroy(&mMutex);
        return false;
    }

    return true;
}

void Signal::set() {
    int status = pthread_mutex_lock(&mMutex);
    if (status) {
        ALOGE("Signal::set: error locking for set condition: %s", strerror(status));
        return;
    }

    mSet = true;

    status = pthread_cond_signal(&mCondition);
    if (status) {
        ALOGE("Signal::set: error on set condition: %s", strerror(status));
    }

    status = pthread_mutex_unlock(&mMutex);
    if (status) {
        ALOGE("Signal::set: error unlocking for set condition: %s", strerror(status));
    }
}

void Signal::wait() {
    int status = pthread_mutex_lock(&mMutex);
    if (status) {
        ALOGE("Signal::wait: error locking for condition: %s", strerror(status));
        return;
    }

    if (!mSet) {
        status = pthread_cond_wait(&mCondition, &mMutex);
    }

    if (!status) {
        mSet = false;
    } else {
        ALOGE("Signal::wait: error waiting for condition: %s", strerror(status));
    }

    status = pthread_mutex_unlock(&mMutex);
    if (status) {
        ALOGE("Signal::wait: error unlocking for condition: %s", strerror(status));
    }
}

} // namespace renderscript
} // namespace android
