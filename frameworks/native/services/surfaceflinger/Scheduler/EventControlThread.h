/*
 * Copyright (C) 2013 The Android Open Source Project
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

#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <thread>

#include <android-base/thread_annotations.h>

namespace android {

class EventControlThread {
public:
    virtual ~EventControlThread();

    virtual void setVsyncEnabled(bool enabled) = 0;
};

namespace impl {

class EventControlThread final : public android::EventControlThread {
public:
    using SetVSyncEnabledFunction = std::function<void(bool)>;

    explicit EventControlThread(SetVSyncEnabledFunction function);
    ~EventControlThread();

    // EventControlThread implementation
    void setVsyncEnabled(bool enabled) override;

private:
    void threadMain();

    std::mutex mMutex;
    std::condition_variable mCondition;

    const SetVSyncEnabledFunction mSetVSyncEnabled;
    bool mVsyncEnabled GUARDED_BY(mMutex) = false;
    bool mKeepRunning GUARDED_BY(mMutex) = true;

    // Must be last so that everything is initialized before the thread starts.
    std::thread mThread{&EventControlThread::threadMain, this};
};

} // namespace impl
} // namespace android
