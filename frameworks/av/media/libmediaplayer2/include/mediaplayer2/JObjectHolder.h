/*
 * Copyright 2018, The Android Open Source Project
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

#ifndef JOBJECT_HOLDER_H_

#define JOBJECT_HOLDER_H_

#include "jni.h"
#include <mediaplayer2/JavaVMHelper.h>
#include <utils/RefBase.h>

namespace android {

// Helper class for managing global reference of jobject.
struct JObjectHolder : public RefBase {
    JObjectHolder(jobject obj) {
        JNIEnv *env = JavaVMHelper::getJNIEnv();
        mJObject = reinterpret_cast<jobject>(env->NewGlobalRef(obj));
    }

    virtual ~JObjectHolder() {
        JNIEnv *env = JavaVMHelper::getJNIEnv();
        env->DeleteGlobalRef(mJObject);
    }

    jobject getJObject() { return mJObject; }

private:
    jobject mJObject;
};

}  //" android

#endif  // JOBJECT_HOLDER_H_
