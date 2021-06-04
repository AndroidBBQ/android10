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

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include <EGL/egl.h>
#include "Thread.h"
#include "Scene.h"

namespace samples {

constexpr int MAX_N_SPHERES = 50;

class Renderer {
    // Allows construction with std::unique_ptr from a static method, but disallows construction
    // outside of the class since no one else can construct a ConstructorTag
    struct ConstructorTag {
    };

public:
    explicit Renderer(ConstructorTag) {
        launchThread();
    }

    static Renderer *getInstance();

    // Sets the active window to render into
    // Takes ownership of window and will release its reference
    void setWindow(ANativeWindow *window, int32_t width, int32_t height);

    void start();

    void stop();

    float getAverageFps();

    void requestDraw();

    void setQuality(int num_spheres, int tesselation_percent);

private:
    class State {
    public:
        State(int nSpheres);
        ~State();

        void clearSurface();
        bool configHasAttribute(EGLint attribute, EGLint value);
        EGLBoolean makeCurrent(EGLSurface surface);

        EGLDisplay display = EGL_NO_DISPLAY;
        EGLConfig config = static_cast<EGLConfig>(0);
        EGLSurface surface = EGL_NO_SURFACE;
        EGLContext context = EGL_NO_CONTEXT;

        bool isStarted = false;
        std::chrono::time_point<std::chrono::steady_clock> lastUpdate = std::chrono::steady_clock::now();
        Scene scene;

        std::chrono::nanoseconds refreshPeriod = std::chrono::nanoseconds{0};
        int64_t swapIntervalNS = 0;
        int32_t width = 0;
        int32_t height = 0;
    };
    using Task = std::function<void(State*)>;

    void draw(State* );
    void calculateFps();
    float averageFps = -1.0f;
    int mTesselation = 20; // 0-100, 100 = MAX_PHI*MAX_THETA*2 triangles per sphere

    void launchThread();
    void terminateThread();
    void threadMain();
    void enqueue(Task t);

    std::mutex mThreadMutex;
    std::thread mThread GUARDED_BY(mThreadMutex);
    std::condition_variable_any mCondVar;
    std::mutex mQueueMutex;
    std::queue<Task> mQueue GUARDED_BY(mQueueMutex);
    bool mIsActive GUARDED_BY(mQueueMutex) = true;
};

} // namespace samples
