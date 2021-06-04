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

/*
 * This file defines an NDK API.
 * Do not remove methods.
 * Do not change method signatures.
 * Do not change the value of constants.
 * Do not change the size of any of the classes defined in here.
 * Do not reference types that are not part of the NDK.
 * Do not #include files that aren't part of the NDK.
 */

#ifndef _NDK_MEDIA_FORMAT_PRIV_H
#define _NDK_MEDIA_FORMAT_PRIV_H

#include <utils/KeyedVector.h>
#include <utils/String8.h>
#include <utils/StrongPointer.h>
#include <media/stagefright/foundation/AMessage.h>

#ifdef __cplusplus
extern "C" {
#endif

struct AMediaFormat {
    android::sp<android::AMessage> mFormat;
    android::String8 mDebug;
    android::KeyedVector<android::String8, android::String8> mStringCache;
};

#ifdef __cplusplus
} // extern "C"
#endif

namespace android {

AMediaFormat* AMediaFormat_fromMsg(sp<AMessage> *);
void AMediaFormat_getFormat(const AMediaFormat* mData, sp<AMessage> *dest);

} // namespace android

#endif // _NDK_MEDIA_FORMAT_PRIV_H

