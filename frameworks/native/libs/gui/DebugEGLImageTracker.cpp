/*
 * Copyright 2019 The Android Open Source Project
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

#include <android-base/stringprintf.h>
#include <cutils/properties.h>
#include <gui/DebugEGLImageTracker.h>

#include <cinttypes>
#include <unordered_map>

using android::base::StringAppendF;

std::mutex DebugEGLImageTracker::mInstanceLock;
std::atomic<DebugEGLImageTracker *> DebugEGLImageTracker::mInstance;

class DebugEGLImageTrackerNoOp : public DebugEGLImageTracker {
public:
    DebugEGLImageTrackerNoOp() = default;
    ~DebugEGLImageTrackerNoOp() override = default;
    void create(const char * /*from*/) override {}
    void destroy(const char * /*from*/) override {}

    void dump(std::string & /*result*/) override {}
};

class DebugEGLImageTrackerImpl : public DebugEGLImageTracker {
public:
    DebugEGLImageTrackerImpl() = default;
    ~DebugEGLImageTrackerImpl() override = default;
    void create(const char * /*from*/) override;
    void destroy(const char * /*from*/) override;

    void dump(std::string & /*result*/) override;

private:
    std::mutex mLock;
    std::unordered_map<std::string, int64_t> mCreateTracker;
    std::unordered_map<std::string, int64_t> mDestroyTracker;

    int64_t mTotalCreated = 0;
    int64_t mTotalDestroyed = 0;
};

DebugEGLImageTracker *DebugEGLImageTracker::getInstance() {
    std::lock_guard lock(mInstanceLock);
    if (mInstance == nullptr) {
        char value[PROPERTY_VALUE_MAX];
        property_get("debug.sf.enable_egl_image_tracker", value, "0");
        const bool enabled = static_cast<bool>(atoi(value));

        if (enabled) {
            mInstance = new DebugEGLImageTrackerImpl();
        } else {
            mInstance = new DebugEGLImageTrackerNoOp();
        }
    }

    return mInstance;
}

void DebugEGLImageTrackerImpl::create(const char *from) {
    std::lock_guard lock(mLock);
    mCreateTracker[from]++;
    mTotalCreated++;
}

void DebugEGLImageTrackerImpl::destroy(const char *from) {
    std::lock_guard lock(mLock);
    mDestroyTracker[from]++;
    mTotalDestroyed++;
}

void DebugEGLImageTrackerImpl::dump(std::string &result) {
    std::lock_guard lock(mLock);
    StringAppendF(&result, "Live EGL Image objects: %" PRIi64 "\n",
                  mTotalCreated - mTotalDestroyed);
    StringAppendF(&result, "Total EGL Image created: %" PRIi64 "\n", mTotalCreated);
    for (const auto &[from, count] : mCreateTracker) {
        StringAppendF(&result, "\t%s: %" PRIi64 "\n", from.c_str(), count);
    }
    StringAppendF(&result, "Total EGL Image destroyed: %" PRIi64 "\n", mTotalDestroyed);
    for (const auto &[from, count] : mDestroyTracker) {
        StringAppendF(&result, "\t%s: %" PRIi64 "\n", from.c_str(), count);
    }
}
