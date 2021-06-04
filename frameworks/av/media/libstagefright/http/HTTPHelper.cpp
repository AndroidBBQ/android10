/*
 * Copyright (C) 2014 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "HTTPHelper"
#include <utils/Log.h>

#include "HTTPHelper.h"

#include "android_runtime/AndroidRuntime.h"
#include "android_util_Binder.h"
#include <media/IMediaHTTPService.h>
#include <media/stagefright/foundation/ADebug.h>
#include <nativehelper/ScopedLocalRef.h>
#include "jni.h"

namespace android {

sp<IMediaHTTPService> CreateHTTPServiceInCurrentJavaContext() {
    if (AndroidRuntime::getJavaVM() == NULL) {
        ALOGE("CreateHTTPServiceInCurrentJavaContext called outside "
              "JAVA environment.");
        return NULL;
    }

    JNIEnv *env = AndroidRuntime::getJNIEnv();

    ScopedLocalRef<jclass> clazz(
            env, env->FindClass("android/media/MediaHTTPService"));
    CHECK(clazz.get() != NULL);

    jmethodID constructID = env->GetMethodID(clazz.get(), "<init>", "(Ljava/util/List;)V");
    CHECK(constructID != NULL);

    ScopedLocalRef<jobject> httpServiceObj(
            env, env->NewObject(clazz.get(), constructID, NULL));

    sp<IMediaHTTPService> httpService;
    if (httpServiceObj.get() != NULL) {
        jmethodID asBinderID =
            env->GetMethodID(clazz.get(), "asBinder", "()Landroid/os/IBinder;");
        CHECK(asBinderID != NULL);

        ScopedLocalRef<jobject> httpServiceBinderObj(
                env, env->CallObjectMethod(httpServiceObj.get(), asBinderID));
        CHECK(httpServiceBinderObj.get() != NULL);

        sp<IBinder> binder =
            ibinderForJavaObject(env, httpServiceBinderObj.get());

        httpService = interface_cast<IMediaHTTPService>(binder);
    }

    return httpService;
}

}  // namespace android
