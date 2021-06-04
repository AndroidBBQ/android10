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

#include <android/binder_ibinder_jni.h>
#include "ibinder_internal.h"

#include <android_util_Binder.h>

using ::android::IBinder;
using ::android::ibinderForJavaObject;
using ::android::javaObjectForIBinder;
using ::android::sp;

AIBinder* AIBinder_fromJavaBinder(JNIEnv* env, jobject binder) {
    if (binder == nullptr) {
        return nullptr;
    }

    sp<IBinder> ibinder = ibinderForJavaObject(env, binder);
    sp<AIBinder> cbinder = ABpBinder::lookupOrCreateFromBinder(ibinder);
    AIBinder_incStrong(cbinder.get());

    return cbinder.get();
}

jobject AIBinder_toJavaBinder(JNIEnv* env, AIBinder* binder) {
    if (binder == nullptr) {
        return nullptr;
    }

    return javaObjectForIBinder(env, binder->getBinder());
}
