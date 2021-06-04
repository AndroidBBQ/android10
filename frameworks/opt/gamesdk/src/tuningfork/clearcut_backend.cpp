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

#include <sstream>
#include <string>
#include <iostream>
#include <jni.h>

#include "modp_b64.h"

#define LOG_TAG "TuningFork.Clearcut"
#include "Log.h"

#include "clearcut_backend.h"
#include "clearcutserializer.h"
#include "uploadthread.h"
#include "tuningfork/protobuf_nano_util.h"
#include "tuningfork_internal.h"

namespace tuningfork {

static char s_clearcut_log_source[] = "TUNING_FORK";

ClearcutBackend::~ClearcutBackend() {}

bool ClearcutBackend::Process(const ProtobufSerialization &evt_ser) {

    ALOGI("Process log");

    if(proto_print_ != nullptr)
        proto_print_->Print(evt_ser);

    JNIEnv* env;
    //Attach thread
    int envStatus  = vm_->GetEnv((void**)&env, JNI_VERSION_1_6);

    switch(envStatus) {
        case JNI_OK:
            break;
        case JNI_EVERSION:
            ALOGW("JNI Version is not supported, status : %d", envStatus);
            return false;
        case JNI_EDETACHED: {
            int attachStatus = vm_->AttachCurrentThread(&env, (void *) NULL);
            if (attachStatus != JNI_OK) {
                ALOGW("Thread is not attached, status : %d", attachStatus);
                return false;
            }
        }
            break;
        default:
            ALOGW("JNIEnv is not OK, status : %d", envStatus);
            return false;
    }

    //Cast to jbytearray
    jsize length = evt_ser.size();
    jbyteArray  output = env->NewByteArray(length);
    env->SetByteArrayRegion(output, 0, length, reinterpret_cast<const jbyte *>(evt_ser.data()));

    //Send to Clearcut
    jobject newBuilder = env->CallObjectMethod(clearcut_logger_, new_event_, output);
    env->CallVoidMethod(newBuilder, log_method_);
    bool hasException = CheckException(env);

    // Detach thread.
    vm_->DetachCurrentThread();
    ALOGI("Message was sent to clearcut");
    return !hasException;
}

bool ClearcutBackend::Init(JNIEnv *env, jobject activity, ProtoPrint* proto_print) {
    ALOGI("%s", "Start clearcut initialization...");

    proto_print_ = proto_print;
    env->GetJavaVM(&vm_);
    if(vm_ == nullptr) {
        ALOGE("%s", "JavaVM is null...");
        return false;
    }

    try {
        bool inited = InitWithClearcut(env, activity, false);
        ALOGI("Clearcut status: %s available", inited ? "" : "not");
        return  inited;
    } catch (const std::exception& e) {
        ALOGI("Clearcut status: not available");
        return false;
    }

}

bool ClearcutBackend::IsGooglePlayServiceAvailable(JNIEnv* env, jobject context) {
    jclass availabilityClass =
            env->FindClass("com/google/android/gms/common/GoogleApiAvailability");
    if(CheckException(env)) return false;

    jmethodID getInstanceMethod = env->GetStaticMethodID(
        availabilityClass,
        "getInstance",
        "()Lcom/google/android/gms/common/GoogleApiAvailability;");
    if(CheckException(env)) return false;

    jobject availabilityInstance = env->CallStaticObjectMethod(
        availabilityClass,
        getInstanceMethod);
    if(CheckException(env)) return false;

    jmethodID isAvailableMethod = env->GetMethodID(
        availabilityClass,
        "isGooglePlayServicesAvailable",
        "(Landroid/content/Context;)I");
    if(CheckException(env)) return false;

    jint jresult = env->CallIntMethod(availabilityInstance, isAvailableMethod, context);
    if(CheckException(env)) return false;

    int result = reinterpret_cast<int>(jresult);

    ALOGI("Google Play Services status : %d", result);

    if(result == 0) {
         jfieldID  versionField =
            env->GetStaticFieldID(availabilityClass, "GOOGLE_PLAY_SERVICES_VERSION_CODE", "I");
        if(CheckException(env)) return false;

        jint versionCode = env->GetStaticIntField(availabilityClass, versionField);
        if(CheckException(env)) return false;

        ALOGI("Google Play Services version : %d", versionCode);
    }

    return result == 0;
}

bool ClearcutBackend::CheckException(JNIEnv *env) {
    if(env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return true;
    }
    return false;
}

bool ClearcutBackend::InitWithClearcut(JNIEnv* env, jobject activity, bool anonymousLogging) {
    ALOGI("Start searching for clearcut...");

    // Get Application Context
    jclass activityClass = env->GetObjectClass(activity);
    if (CheckException(env)) return false;
    jmethodID getContext = env->GetMethodID(
            activityClass,
            "getApplicationContext",
            "()Landroid/content/Context;");
    if (CheckException(env)) return false;
    jobject context = env->CallObjectMethod(activity, getContext);

    //Check if Google Play Services are available
    bool available = IsGooglePlayServiceAvailable(env, context);
    if (!available) {
        ALOGW("Google Play Service is not available");
        return false;
    }

    // Searching for  classes
    jclass loggerClass = env->FindClass("com/google/android/gms/clearcut/ClearcutLogger");
    if (CheckException(env)) return false;
    jclass stringClass = env->FindClass("java/lang/String");
    if (CheckException(env)) return false;
    jclass builderClass = env->FindClass(
            "com/google/android/gms/clearcut/ClearcutLogger$LogEventBuilder");
    if (CheckException(env)) return false;

    //Searching for all methods
    log_method_ = env->GetMethodID(builderClass, "log", "()V");
    if (CheckException(env)) return false;
    new_event_ = env->GetMethodID(
            loggerClass,
            "newEvent",
            "([B)Lcom/google/android/gms/clearcut/ClearcutLogger$LogEventBuilder;");
    if (CheckException(env)) return false;

    jmethodID anonymousLogger = env->GetStaticMethodID(
            loggerClass,
            "anonymousLogger",
            "(Landroid/content/Context;"
            "Ljava/lang/String;)"
            "Lcom/google/android/gms/clearcut/ClearcutLogger;");
    if (CheckException(env)) return false;

    jmethodID loggerConstructor = env->GetMethodID(
            loggerClass,
            "<init>",
            "(Landroid/content/Context;"
            "Ljava/lang/String;"
            "Ljava/lang/String;)"
            "V");
    if (CheckException(env)) return false;

    //Create logger type
    jstring ccName = env->NewStringUTF(s_clearcut_log_source);
    if (CheckException(env)) return false;

    //Create logger instance
    jobject localClearcutLogger;
    if (anonymousLogging) {
        localClearcutLogger = env->CallStaticObjectMethod(loggerClass, anonymousLogger, context,
                                                          ccName);
    } else {
        localClearcutLogger = env->NewObject(loggerClass, loggerConstructor, context, ccName, NULL);
    }
    if (CheckException(env)) return false;

    clearcut_logger_ = reinterpret_cast<jobject>(env->NewGlobalRef(localClearcutLogger));
    if (CheckException(env)) return false;

    ALOGI("Clearcut is succesfully found.");
    return true;
}

void ProtoPrint::Print(const ProtobufSerialization &evt_ser) {
    if (evt_ser.size() == 0) return;
    auto encode_len = modp_b64_encode_len(evt_ser.size());
    std::vector<char> dest_buf(encode_len);
    // This fills the dest buffer with a null-terminated string. It returns the length of
    //  the string, not including the null char
    auto n_encoded = modp_b64_encode(&dest_buf[0], reinterpret_cast<const char*>(&evt_ser[0]),
                                     evt_ser.size());
    if (n_encoded == -1 || encode_len != n_encoded+1) {
        ALOGW("Could not b64 encode protobuf");
        return;
    }
    std::string s(&dest_buf[0], n_encoded);
    // Split the serialization into <128-byte chunks to avoid logcat line
    //  truncation.
    constexpr size_t maxStrLen = 128;
    int n = (s.size() + maxStrLen - 1) / maxStrLen; // Round up
    for (int i = 0, j = 0; i < n; ++i) {
        std::stringstream str;
        str << "(TCL" << (i + 1) << "/" << n << ")";
        int m = std::min(s.size() - j, maxStrLen);
        str << s.substr(j, m);
        j += m;
        ALOGI("%s", str.str().c_str());
    }
    return;
}
}
