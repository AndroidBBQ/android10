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

#ifndef AAUDIO_THREAD_H
#define AAUDIO_THREAD_H

#include <atomic>
#include <pthread.h>

#include <aaudio/AAudio.h>

namespace aaudio {

/**
 * Abstract class similar to Java Runnable.
 */
class Runnable {
public:
    Runnable() {};
    virtual ~Runnable() = default;

    virtual void run() = 0;
};

/**
 * Abstraction for a host dependent thread.
 * TODO Consider using Android "Thread" class or std::thread instead.
 */
class AAudioThread
{
public:
    AAudioThread();

    explicit AAudioThread(const char *prefix);

    virtual ~AAudioThread() = default;

    /**
     * Start the thread running.
     */
    aaudio_result_t start(Runnable *runnable = nullptr);

    /**
     * Join the thread.
     * The caller must somehow tell the thread to exit before calling join().
     */
    aaudio_result_t stop();

    /**
     * This will get called in the thread.
     * Override this or pass a Runnable to start().
     */
    virtual void run() {};

    void dispatch(); // called internally from 'C' thread wrapper

private:

    void setup(const char *prefix);

    Runnable    *mRunnable = nullptr;
    bool         mHasThread = false;
    pthread_t    mThread; // initialized in constructor

    static std::atomic<uint32_t> mNextThreadIndex;
    char         mName[16]; // max length for a pthread_name
};

} /* namespace aaudio */

#endif ///AAUDIO_THREAD_H
