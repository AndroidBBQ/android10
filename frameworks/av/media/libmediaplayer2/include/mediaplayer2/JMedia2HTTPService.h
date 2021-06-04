/*
 * Copyright 2017, The Android Open Source Project
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

#ifndef _J_MEDIA2_HTTP_SERVICE_H_
#define _J_MEDIA2_HTTP_SERVICE_H_

#include <jni.h>
#include <utils/RefBase.h>

#include <media/MediaHTTPService.h>
#include <media/MediaHTTPConnection.h>
#include <media/stagefright/foundation/ABase.h>

namespace android {

struct JMedia2HTTPService : public MediaHTTPService {
    JMedia2HTTPService(JNIEnv *env, jobject thiz);

    virtual sp<MediaHTTPConnection> makeHTTPConnection() override;

protected:
    virtual ~JMedia2HTTPService();

private:
    jobject mMedia2HTTPServiceObj;

    jmethodID mMakeHTTPConnectionMethod;

    DISALLOW_EVIL_CONSTRUCTORS(JMedia2HTTPService);
};

}  // namespace android

#endif  // _J_MEDIA2_HTTP_SERVICE_H_
