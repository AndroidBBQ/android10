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

#ifndef JAVA_VM_HELPER_H_

#define JAVA_VM_HELPER_H_

#include "jni.h"

#include <atomic>

namespace android {

struct JavaVMHelper {
    static JNIEnv *getJNIEnv();
    static JavaVM *getJavaVM();
    static void setJavaVM(JavaVM *vm);

private:
    // Once a valid JavaVM has been set, it should never be reset or changed.
    // However, as it may be accessed from multiple threads, access needs to be
    // synchronized.
    static std::atomic<JavaVM *> sJavaVM;
};

}  // namespace android

#endif  // JAVA_VM_HELPER_H_
