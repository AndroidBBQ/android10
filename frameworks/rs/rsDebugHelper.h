/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef ANDROID_RS_DEBUG_HELPER_H
#define ANDROID_RS_DEBUG_HELPER_H

#include "rsUtils.h"
#include "rsInternalDefines.h"

#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
#include <utils/CallStack.h>
#endif

namespace android {
namespace renderscript {


class DebugHelper {
public:
    DebugHelper() {
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
        mStack.update(2);
#endif
    }

    void dump() {
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
        ALOGV("%s", mStack.toString().string());
        //mStack.dump();
#endif
    }

private:
#if !defined(RS_VENDOR_LIB) && !defined(RS_COMPATIBILITY_LIB)
    CallStack mStack;
#endif
};


}  // namespace renderscript
}  // namespace android

#endif //ANDROID_RS_DEBUG_HELPER_H

