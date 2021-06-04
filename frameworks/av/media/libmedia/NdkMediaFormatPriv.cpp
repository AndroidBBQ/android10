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

//#define LOG_NDEBUG 0
#define LOG_TAG "NdkMediaFormat"

#include <inttypes.h>

#include <utils/Log.h>
#include <utils/StrongPointer.h>
#include <media/NdkMediaFormatPriv.h>
#include <media/stagefright/foundation/AMessage.h>

#include <jni.h>

using namespace android;

namespace android {

// private functions for conversion to/from AMessage
AMediaFormat* AMediaFormat_fromMsg(sp<AMessage> *data) {
    ALOGV("private ctor");
    AMediaFormat* mData = new AMediaFormat();
    mData->mFormat = *data;
    if (mData->mFormat == NULL) {
        ALOGW("got NULL format");
        mData->mFormat = new AMessage;
    }
    return mData;
}

void AMediaFormat_getFormat(const AMediaFormat* mData, sp<AMessage> *dest) {
    *dest = mData->mFormat;
}

} // namespace android


