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

#include "Renderer.h"

#define LOG_TAG "TestAppRender"

#include <vector>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include <GLES2/gl2.h>

#include <android/native_window.h>

#include "Log.h"

#include "swappy/swappy.h"
#include "swappy/swappy_extra.h"
#include "tuningfork/tuningfork.h"

#include "Scene.h"

extern bool swappy_enabled;

using namespace std::chrono_literals;

namespace samples {

Renderer *Renderer::getInstance() {
    static std::unique_ptr<Renderer> sRenderer = std::make_unique<Renderer>(ConstructorTag{});
    return sRenderer.get();
}

void Renderer::setWindow(ANativeWindow *window, int32_t width, int32_t height) {
    enqueue([=](State* state) {
        state->clearSurface();

        ALOGI("Creating window surface %dx%d", width, height);

        if (!window) return;

        state->surface =
            eglCreateWindowSurface(state->display, state->config, window, NULL);
        ANativeWindow_release(window);
        if (!state->makeCurrent(state->surface)) {
            ALOGE("Unable to eglMakeCurrent");
            state->surface = EGL_NO_SURFACE;
            return;
        }

        state->width = width;
        state->height = height;
    });
}

void Renderer::start() {
    enqueue([this](State* state) {
        state->isStarted = true;
        requestDraw();
    });
}

void Renderer::stop() {
    enqueue([=](State *state) { state->isStarted = false; });
}

float Renderer::getAverageFps() {
    return averageFps;
}

void Renderer::requestDraw() {
    enqueue([this](State* state) { if (state->isStarted) draw(state);});
}

void Renderer::enqueue(Task task) {
    std::lock_guard<std::mutex> lock(mQueueMutex);
    mQueue.emplace(std::move(task));
    mCondVar.notify_all();
}
void Renderer::launchThread() {
    std::lock_guard<std::mutex> threadLock(mThreadMutex);
    if (mThread.joinable()) {
        terminateThread();
    }
    mThread = std::thread([this]() { threadMain(); });
}

void Renderer::terminateThread() REQUIRES(mThreadMutex) {
    {
        std::lock_guard<std::mutex> lock(mQueueMutex);
        mIsActive = false;
        mCondVar.notify_all();
    }
    mThread.join();
}
void Renderer::threadMain() {
    pthread_setname_np(pthread_self(), "Renderer");
    State state(MAX_N_SPHERES);

    std::lock_guard<std::mutex> lock(mQueueMutex);
    while (mIsActive) {
        mCondVar.wait(mQueueMutex,
                            [this]() REQUIRES(mQueueMutex) {
            return !mQueue.empty() || !mIsActive;
        });
        if (!mQueue.empty()) {
            auto head = mQueue.front();
            mQueue.pop();

            // Drop the mutex while we execute
            mQueueMutex.unlock();
            head(&state);
            mQueueMutex.lock();
        }
    }
}
Renderer::State::State(int nSpheres) : scene(nSpheres) {
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);

    const EGLint configAttributes[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };

    EGLint numConfigs = 0;
    eglChooseConfig(display, configAttributes, nullptr, 0, &numConfigs);
    std::vector<EGLConfig> supportedConfigs(static_cast<size_t>(numConfigs));
    eglChooseConfig(display, configAttributes, supportedConfigs.data(), numConfigs, &numConfigs);

    // Choose a config, either a match if possible or the first config otherwise

    const auto configMatches = [&](EGLConfig config) {
        if (!configHasAttribute(EGL_RED_SIZE, 8)) return false;
        if (!configHasAttribute(EGL_GREEN_SIZE, 8)) return false;
        if (!configHasAttribute(EGL_BLUE_SIZE, 8)) return false;
        return configHasAttribute(EGL_DEPTH_SIZE, 24);
    };

    const auto configIter = std::find_if(supportedConfigs.cbegin(), supportedConfigs.cend(),
                                         configMatches);

    config = (configIter != supportedConfigs.cend()) ? *configIter : supportedConfigs[0];

    const EGLint contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    context = eglCreateContext(display, config, nullptr, contextAttributes);

}

Renderer::State::~State() {
    clearSurface();
    if (context != EGL_NO_CONTEXT) eglDestroyContext(display, context);
    if (display != EGL_NO_DISPLAY) eglTerminate(display);
}

void Renderer::State::clearSurface() {
    if (surface == EGL_NO_SURFACE) return;

    makeCurrent(EGL_NO_SURFACE);
    eglDestroySurface(display, surface);
    surface = EGL_NO_SURFACE;
}

bool Renderer::State::configHasAttribute(EGLint attribute, EGLint value) {
    EGLint outValue = 0;
    EGLBoolean result = eglGetConfigAttrib(display, config, attribute, &outValue);
    return result && (outValue == value);
}

EGLBoolean Renderer::State::makeCurrent(EGLSurface surface) {
    return eglMakeCurrent(display, surface, surface, context);
}

// should be called once per draw as this function maintains the time delta between calls
void Renderer::calculateFps() {
    static constexpr int FPS_SAMPLES = 10;
    static std::chrono::steady_clock::time_point prev = std::chrono::steady_clock::now();
    static float fpsSum = 0;
    static int fpsCount = 0;


    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    fpsSum += 1.0f / ((now - prev).count() / 1e9f);
    fpsCount++;
    if (fpsCount == FPS_SAMPLES) {
        averageFps = fpsSum / fpsCount;
        fpsSum = 0;
        fpsCount = 0;
    }
    prev = now;
}

void Renderer::setQuality(int num_spheres, int tesselation_percent) {
    enqueue([=](State* state) {
        state->scene.nSpheres = std::max(1,std::min(MAX_N_SPHERES, num_spheres));
        mTesselation = std::max(0,std::min(100,tesselation_percent));
    });
}

void Renderer::draw(State *state) {
    // Don't render if we have no surface
    if (state->surface == EGL_NO_SURFACE) {
        // Sleep a bit so we don't churn too fast
        std::this_thread::sleep_for(50ms);
        requestDraw();
        return;
    }

    if (swappy_enabled)
        Swappy_recordFrameStart(state->display, state->surface);

    calculateFps();

    float deltaSeconds = state->swapIntervalNS / 1e9f;
    auto dt = std::chrono::steady_clock::now() - state->lastUpdate;
    if ( dt <= 100ms ) {
        deltaSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(dt).count() / 1e9f;
    }
    state->lastUpdate += dt;

    state->scene.tick(deltaSeconds);

    // Just fill the screen with a color.
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    const float aspectRatio = static_cast<float>(state->width) / state->height;

    state->scene.draw(aspectRatio, mTesselation);

    if (swappy_enabled)
        Swappy_swap(state->display, state->surface);
    else {
        TuningFork_frameTick(TFTICK_SYSCPU);
        eglSwapBuffers(state->display, state->surface);
    }

    // If we're still started, request another frame
    requestDraw();
}

} // namespace samples
