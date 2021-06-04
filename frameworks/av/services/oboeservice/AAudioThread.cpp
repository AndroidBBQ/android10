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

#define LOG_TAG "AAudioThread"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include <pthread.h>

#include <aaudio/AAudio.h>
#include <utility/AAudioUtilities.h>

#include "AAudioThread.h"

using namespace aaudio;

std::atomic<uint32_t> AAudioThread::mNextThreadIndex{1};

AAudioThread::AAudioThread(const char *prefix) {
    setup(prefix);
}

AAudioThread::AAudioThread() {
    setup("AAudio");
}

void AAudioThread::setup(const char *prefix) {
    // mThread is a pthread_t of unknown size so we need memset().
    memset(&mThread, 0, sizeof(mThread));

    // Name the thread with an increasing index, "prefix_#", for debugging.
    uint32_t index = mNextThreadIndex++;
    // Wrap the index so that we do not hit the 16 char limit
    // and to avoid hard-to-read large numbers.
    index = index % 100000; // arbitrary
    snprintf(mName, sizeof(mName), "%s_%u", prefix, index);
}

void AAudioThread::dispatch() {
    if (mRunnable != nullptr) {
        mRunnable->run();
    } else {
        run();
    }
}

// This is the entry point for the new thread created by createThread().
// It converts the 'C' function call to a C++ method call.
static void * AAudioThread_internalThreadProc(void *arg) {
    AAudioThread *aaudioThread = (AAudioThread *) arg;
    aaudioThread->dispatch();
    return nullptr;
}

aaudio_result_t AAudioThread::start(Runnable *runnable) {
    if (mHasThread) {
        ALOGE("start() - mHasThread already true");
        return AAUDIO_ERROR_INVALID_STATE;
    }
    // mRunnable will be read by the new thread when it starts.
    // pthread_create() forces a memory synchronization so mRunnable does not need to be atomic.
    mRunnable = runnable;
    int err = pthread_create(&mThread, nullptr, AAudioThread_internalThreadProc, this);
    if (err != 0) {
        ALOGE("start() - pthread_create() returned %d %s", err, strerror(err));
        return AAudioConvert_androidToAAudioResult(-err);
    } else {
        int err = pthread_setname_np(mThread, mName);
        ALOGW_IF((err != 0), "Could not set name of AAudioThread. err = %d", err);
        mHasThread = true;
        return AAUDIO_OK;
    }
}

aaudio_result_t AAudioThread::stop() {
    if (!mHasThread) {
        ALOGE("stop() but no thread running");
        return AAUDIO_ERROR_INVALID_STATE;
    }
    int err = pthread_join(mThread, nullptr);
    mHasThread = false;
    if (err != 0) {
        ALOGE("stop() - pthread_join() returned %d %s", err, strerror(err));
        return AAudioConvert_androidToAAudioResult(-err);
    } else {
        return AAUDIO_OK;
    }
}

