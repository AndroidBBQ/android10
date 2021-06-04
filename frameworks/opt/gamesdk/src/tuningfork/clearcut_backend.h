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

#pragma once

#include <sstream>
#include <jni.h>
#include <string>

#include "tuningfork_internal.h"

namespace tuningfork {

class ClearcutBackend : public Backend {
public:
    // Return false if google play services are not available
    bool Init(JNIEnv* env, jobject activity, ProtoPrint *proto_print);

    ~ClearcutBackend() override;
    bool Process(const ProtobufSerialization &tuningfork_log_event) override;

private:
    JavaVM* vm_;
    jobject clearcut_logger_;
    jmethodID new_event_;
    jmethodID log_method_;
    ProtoPrint* proto_print_;

    bool IsGooglePlayServiceAvailable(JNIEnv* env, jobject context);
    bool InitWithClearcut(JNIEnv* env, jobject activity, bool anonymousLogging);
    bool CheckException(JNIEnv* env); ///Need to check for exceptions after each JNI call
};

} //namespace tuningfork {
