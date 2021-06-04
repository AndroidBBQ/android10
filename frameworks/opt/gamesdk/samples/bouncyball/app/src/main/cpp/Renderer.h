/*
 * Copyright 2018 The Android Open Source Project
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
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include <EGL/egl.h>

#include "Thread.h"

#include "WorkerThread.h"

namespace samples {

class Settings;

class Renderer {
    // Allows construction with std::unique_ptr from a static method, but disallows construction
    // outside of the class since no one else can construct a ConstructorTag
    struct ConstructorTag {
    };

public:
    explicit Renderer(ConstructorTag) {}

    static Renderer *getInstance();

    // Sets the active window to render into
    // Takes ownership of window and will release its reference
    void setWindow(ANativeWindow *window, int32_t width, int32_t height);

    void start();

    void stop();

    float getAverageFps();

    void requestDraw();

    void setWorkload(int load);

private:
    class ThreadState {
    public:
        ThreadState();

        ~ThreadState();

        void onSettingsChanged(const Settings *);

        void clearSurface();

        bool configHasAttribute(EGLint attribute, EGLint value);

        EGLBoolean makeCurrent(EGLSurface surface);

        EGLDisplay display = EGL_NO_DISPLAY;
        EGLConfig config = static_cast<EGLConfig>(0);
        EGLSurface surface = EGL_NO_SURFACE;
        EGLContext context = EGL_NO_CONTEXT;

        bool isStarted = false;

        std::chrono::time_point<std::chrono::steady_clock> lastUpdate = std::chrono::steady_clock::now();
        float x = 0.0f;
        float velocity = 1.6f;

        std::chrono::nanoseconds refreshPeriod = std::chrono::nanoseconds{0};
        int64_t swapIntervalNS = 0;
        int32_t width = 0;
        int32_t height = 0;
    };

    void draw(ThreadState *threadState);
    void calculateFps();

    WorkerThread<ThreadState> mWorkerThread = {"Renderer", Affinity::Odd};

    class HotPocketState {
    public:
        void onSettingsChanged(const Settings *);

        bool isEnabled = false;
        bool isStarted = false;
    };

    WorkerThread<HotPocketState> mHotPocketThread = {"HotPocket", Affinity::Even};

    void spin();

    float averageFps = -1.0f;

    int mWorkload = 0;
};

} // namespace samples
