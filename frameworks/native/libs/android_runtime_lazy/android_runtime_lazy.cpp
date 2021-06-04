/*
 * Copyright (C) 2019 The Android Open Source Project
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
#define LOG_TAG "ANDROID_RUNTIME_LAZY"
#include "android_runtime/AndroidRuntime.h"
#include "android_util_Binder.h"

#include <dlfcn.h>
#include <mutex>

#include <log/log.h>

namespace android {
namespace {

std::once_flag loadFlag;

typedef JNIEnv* (*getJNIEnv_t)();
typedef sp<IBinder> (*ibinderForJavaObject_t)(JNIEnv* env, jobject obj);
typedef jobject (*javaObjectForIBinder_t)(JNIEnv* env, const sp<IBinder>& val);

getJNIEnv_t _getJNIEnv;
ibinderForJavaObject_t _ibinderForJavaObject;
javaObjectForIBinder_t _javaObjectForIBinder;

void load() {
    std::call_once(loadFlag, []() {
        void* handle = dlopen("libandroid_runtime.so", RTLD_LAZY);
        if (handle == nullptr) {
            ALOGE("Could not open libandroid_runtime.");
            return;
        }

        _getJNIEnv = reinterpret_cast<getJNIEnv_t>(
                dlsym(handle, "_ZN7android14AndroidRuntime9getJNIEnvEv"));
        if (_getJNIEnv == nullptr) {
            ALOGW("Could not find getJNIEnv.");
            // no return
        }

        _ibinderForJavaObject = reinterpret_cast<ibinderForJavaObject_t>(
                dlsym(handle, "_ZN7android20ibinderForJavaObjectEP7_JNIEnvP8_jobject"));
        if (_ibinderForJavaObject == nullptr) {
            ALOGW("Could not find ibinderForJavaObject.");
            // no return
        }

        _javaObjectForIBinder = reinterpret_cast<javaObjectForIBinder_t>(
                dlsym(handle,
                      "_ZN7android20javaObjectForIBinderEP7_JNIEnvRKNS_2spINS_7IBinderEEE"));
        if (_javaObjectForIBinder == nullptr) {
            ALOGW("Could not find javaObjectForIBinder.");
            // no return
        }
    });
}

} // namespace

// exports delegate functions

JNIEnv* AndroidRuntime::getJNIEnv() {
    load();
    if (_getJNIEnv == nullptr) {
        return nullptr;
    }
    return _getJNIEnv();
}

sp<IBinder> ibinderForJavaObject(JNIEnv* env, jobject obj) {
    load();
    if (_ibinderForJavaObject == nullptr) {
        return nullptr;
    }
    return _ibinderForJavaObject(env, obj);
}

jobject javaObjectForIBinder(JNIEnv* env, const sp<IBinder>& val) {
    load();
    if (_javaObjectForIBinder == nullptr) {
        return nullptr;
    }
    return _javaObjectForIBinder(env, val);
}

} // namespace android
