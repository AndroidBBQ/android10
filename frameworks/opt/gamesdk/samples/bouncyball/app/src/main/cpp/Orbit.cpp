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

#define LOG_TAG "Orbit"

#include <cmath>
#include <string>

#include <jni.h>

#include <android/native_window_jni.h>

#include "Log.h"
#include "Settings.h"

#include "swappy/swappy.h"
#include "swappy/swappy_extra.h"

#include "Renderer.h"

using std::chrono::nanoseconds;
using namespace samples;

namespace {

std::string to_string(jstring jstr, JNIEnv *env) {
    const char *utf = env->GetStringUTFChars(jstr, nullptr);
    std::string str(utf);
    env->ReleaseStringUTFChars(jstr, utf);
    return str;
}

} // anonymous namespace

extern "C" {

void startFrameCallback(void *, int, long) {
}

void swapIntervalChangedCallback(void *) {
    uint64_t swap_ns = Swappy_getSwapIntervalNS();
    ALOGI("Swappy changed swap interval to %.2fms", swap_ns / 1e6f);
}

JNIEXPORT void JNICALL
Java_com_prefabulated_bouncyball_OrbitActivity_nInit(JNIEnv *env, jobject activity) {
    // Get the Renderer instance to create it
    Renderer::getInstance();

    Swappy_init(env, activity);

    SwappyTracer tracers;
    tracers.preWait = nullptr;
    tracers.postWait = nullptr;
    tracers.preSwapBuffers = nullptr;
    tracers.postSwapBuffers = nullptr;
    tracers.startFrame = startFrameCallback;
    tracers.userData = nullptr;
    tracers.swapIntervalChanged = swapIntervalChangedCallback;

    Swappy_injectTracer(&tracers);
}

JNIEXPORT void JNICALL
Java_com_prefabulated_bouncyball_OrbitActivity_nSetSurface(JNIEnv *env, jobject /* this */,
                                                    jobject surface, jint width, jint height) {
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    Renderer::getInstance()->setWindow(window,
                                     static_cast<int32_t>(width),
                                     static_cast<int32_t>(height));
}

JNIEXPORT void JNICALL
Java_com_prefabulated_bouncyball_OrbitActivity_nClearSurface(JNIEnv * /* env */, jobject /* this */) {
    Renderer::getInstance()->setWindow(nullptr, 0, 0);
}

JNIEXPORT void JNICALL
Java_com_prefabulated_bouncyball_OrbitActivity_nStart(JNIEnv * /* env */, jobject /* this */) {
    ALOGI("start");
    Renderer::getInstance()->start();
}

JNIEXPORT void JNICALL
Java_com_prefabulated_bouncyball_OrbitActivity_nStop(JNIEnv * /* env */, jobject /* this */) {
    ALOGI("stop");
    Renderer::getInstance()->stop();
}

JNIEXPORT void JNICALL
Java_com_prefabulated_bouncyball_OrbitActivity_nSetPreference(JNIEnv *env, jobject /* this */,
                                                         jstring key, jstring value) {
    Settings::getInstance()->setPreference(to_string(key, env), to_string(value, env));
}

JNIEXPORT void JNICALL
Java_com_prefabulated_bouncyball_OrbitActivity_nSetAutoSwapInterval(JNIEnv *env, jobject /* this */,
                                                              jboolean enabled) {
    Swappy_setAutoSwapInterval(enabled);
}

JNIEXPORT float JNICALL
Java_com_prefabulated_bouncyball_OrbitActivity_nGetAverageFps(JNIEnv * /* env */, jobject /* this */) {
    return Renderer::getInstance()->getAverageFps();
}

JNIEXPORT void JNICALL
Java_com_prefabulated_bouncyball_OrbitActivity_nSetWorkload(JNIEnv * /* env */, jobject /* this */,
                                                            jint load) {
    Renderer::getInstance()->setWorkload(load);
}

JNIEXPORT int JNICALL
Java_com_prefabulated_bouncyball_OrbitActivity_nGetSwappyStats(JNIEnv * /* env */,
                                                               jobject /* this */,
                                                               jint stat,
                                                               jint bin) {
    static bool enabled = false;
    if (!enabled) {
        Swappy_enableStats(true);
        enabled = true;
    }

    // stats are read one by one, query once per stat
    static Swappy_Stats stats;
    static int stat_idx = -1;

    if (stat_idx != stat) {
        Swappy_getStats(&stats);
        stat_idx = stat;
    }

    int value = 0;

    if (stats.totalFrames) {
        switch (stat) {
            case 0:
                value = stats.idleFrames[bin];
                break;
            case 1:
                value = stats.lateFrames[bin];
                break;
            case 2:
                value = stats.offsetFromPreviousFrame[bin];
                break;
            case 3:
                value = stats.latencyFrames[bin];
                break;
            default:
                return stats.totalFrames;
        }
        value = std::round(value * 100.0f / stats.totalFrames);
    }

    return value;
}

} // extern "C"