/*
 * Copyright (C) 2018 The Android Open Source Project
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

#define LOG_TAG "CarServiceHelperService-JNI"

//#define LOG_NDEBUG 0

#include "jni.h"

#include <nativehelper/JNIHelp.h>
#include <suspend/autosuspend.h>
#include <utils/Log.h>

namespace android {

// ----------------------------------------------------------------------------

static jint nativeForceSuspend(JNIEnv* /* env */, jclass /* clazz */, jint timeoutMs) {
    jint ret = autosuspend_force_suspend(timeoutMs);
    ALOGD("nativeForceSuspend returned %d", ret);
    return ret;
}

// ----------------------------------------------------------------------------

static const JNINativeMethod gCarServiceHelperServiceMethods[] = {
    /* name, signature, funcPtr */
    { "nativeForceSuspend", "(I)I",
            (void*) nativeForceSuspend },
};

int register_android_internal_car_CarServiceHelperService(JNIEnv* env) {
    int res = jniRegisterNativeMethods(env, "com/android/internal/car/CarServiceHelperService",
            gCarServiceHelperServiceMethods, NELEM(gCarServiceHelperServiceMethods));
    (void) res;  // Faked use when LOG_NDEBUG.
    LOG_FATAL_IF(res < 0, "Unable to register native methods.");
    return 0;
}

} /* namespace android */


using namespace android;

extern "C" jint JNI_OnLoad(JavaVM* vm, void* /* reserved */)
{
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("GetEnv failed!");
        return result;
    }
    ALOG_ASSERT(env, "Could not retrieve the env!");

    register_android_internal_car_CarServiceHelperService(env);
    return JNI_VERSION_1_4;
}
