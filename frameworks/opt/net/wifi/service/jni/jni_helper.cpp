/*
 * Copyright 2016, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "wifi"

#include <hardware_legacy/wifi_hal.h>
#include <nativehelper/ScopedUtfChars.h>
#include <utils/Log.h>
#include <utils/String16.h>
#include <utils/misc.h>

#include "jni_helper.h"

namespace android {

/* JNI Helpers for wifi_hal implementation */

JNIHelper::JNIHelper(JavaVM *vm)
{
    vm->AttachCurrentThread(&mEnv, NULL);
    mVM = vm;
}

JNIHelper::JNIHelper(JNIEnv *env)
{
    mVM  = NULL;
    mEnv = env;
}

JNIHelper::~JNIHelper()
{
    if (mVM != NULL) {
        // mVM->DetachCurrentThread();  /* 'attempting to detach while still running code' */
        mVM = NULL;                     /* not really required; but may help debugging */
        mEnv = NULL;                    /* not really required; but may help debugging */
    }
}

jobject JNIHelper::newLocalRef(jobject obj) {
      return mEnv->NewLocalRef(obj);
}

void JNIHelper::deleteLocalRef(jobject obj) {
      mEnv->DeleteLocalRef(obj);
}

JNIObject<jbyteArray> JNIHelper::newByteArray(int num) {
    return JNIObject<jbyteArray>(*this, mEnv->NewByteArray(num));
}

void JNIHelper::setByteArrayRegion(jbyteArray array, int from, int to, const jbyte *bytes) {
    mEnv->SetByteArrayRegion(array, from, to, bytes);
}
}; // namespace android
