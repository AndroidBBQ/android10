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

#pragma once

#include <atomic>
#include <mutex>
#include <string>

class DebugEGLImageTracker {
public:
    static DebugEGLImageTracker *getInstance();

    virtual void create(const char *from) = 0;
    virtual void destroy(const char *from) = 0;

    virtual void dump(std::string &result) = 0;

protected:
    DebugEGLImageTracker() = default;
    virtual ~DebugEGLImageTracker() = default;
    DebugEGLImageTracker(const DebugEGLImageTracker &) = delete;

    static std::mutex mInstanceLock;
    static std::atomic<DebugEGLImageTracker *> mInstance;
};

#define DEBUG_EGL_IMAGE_TRACKER_CREATE() \
    (DebugEGLImageTracker::getInstance()->create(__PRETTY_FUNCTION__))
#define DEBUG_EGL_IMAGE_TRACKER_DESTROY() \
    (DebugEGLImageTracker::getInstance()->destroy(__PRETTY_FUNCTION__))