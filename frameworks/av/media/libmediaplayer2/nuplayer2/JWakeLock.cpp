/*
 * Copyright 2017 The Android Open Source Project
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
#define LOG_TAG "JWakeLock"
#include <utils/Log.h>

#include "JWakeLock.h"

#include <media/stagefright/foundation/ADebug.h>

namespace android {

JWakeLock::JWakeLock(const sp<JObjectHolder> &context) :
    mWakeLockCount(0),
    mWakeLock(NULL),
    mContext(context) {}

JWakeLock::~JWakeLock() {
    clearJavaWakeLock();
}

bool JWakeLock::acquire() {
    if (mWakeLockCount == 0) {
        if (mWakeLock == NULL) {
            JNIEnv *env = JavaVMHelper::getJNIEnv();
            jclass jContextCls = env->FindClass("android/content/Context");
            jclass jPowerManagerCls = env->FindClass("android/os/PowerManager");

            jmethodID jGetSystemService = env->GetMethodID(jContextCls,
                    "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
            jobject javaPowerManagerObj = env->CallObjectMethod(mContext->getJObject(),
                    jGetSystemService, env->NewStringUTF("power"));

            jfieldID jPARTIAL_WAKE_LOCK = env->GetStaticFieldID(jPowerManagerCls,
                    "PARTIAL_WAKE_LOCK", "I");
            jint PARTIAL_WAKE_LOCK = env->GetStaticIntField(jPowerManagerCls, jPARTIAL_WAKE_LOCK);

            jmethodID jNewWakeLock = env->GetMethodID(jPowerManagerCls,
                    "newWakeLock", "(ILjava/lang/String;)Landroid/os/PowerManager$WakeLock;");
            jobject javaWakeLock = env->CallObjectMethod(javaPowerManagerObj,
                    jNewWakeLock, PARTIAL_WAKE_LOCK, env->NewStringUTF("JWakeLock"));
            mWakeLock = new JObjectHolder(javaWakeLock);
            env->DeleteLocalRef(javaPowerManagerObj);
            env->DeleteLocalRef(javaWakeLock);
        }
        if (mWakeLock != NULL) {
            JNIEnv *env = JavaVMHelper::getJNIEnv();
            jclass wakeLockCls = env->FindClass("android/os/PowerManager$WakeLock");
            jmethodID jAcquire = env->GetMethodID(wakeLockCls, "acquire", "()V");
            env->CallVoidMethod(mWakeLock->getJObject(), jAcquire);
            mWakeLockCount++;
            return true;
        }
    } else {
        mWakeLockCount++;
        return true;
    }
    return false;
}

void JWakeLock::release(bool force) {
    if (mWakeLockCount == 0) {
        return;
    }
    if (force) {
        // Force wakelock release below by setting reference count to 1.
        mWakeLockCount = 1;
    }
    if (--mWakeLockCount == 0) {
        if (mWakeLock != NULL) {
            JNIEnv *env = JavaVMHelper::getJNIEnv();
            jclass wakeLockCls = env->FindClass("android/os/PowerManager$WakeLock");
            jmethodID jRelease = env->GetMethodID(wakeLockCls, "release", "()V");
            env->CallVoidMethod(mWakeLock->getJObject(), jRelease);
        }
    }
}

void JWakeLock::clearJavaWakeLock() {
    release(true);
}

}  // namespace android
