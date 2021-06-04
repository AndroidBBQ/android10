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

#ifndef J_WAKELOCK_H_
#define J_WAKELOCK_H_

#include <media/stagefright/foundation/ABase.h>
#include <mediaplayer2/JObjectHolder.h>
#include <utils/RefBase.h>

namespace android {

class JWakeLock : public RefBase {

public:
    JWakeLock(const sp<JObjectHolder> &context);

    // NOTE: acquire and release are not thread safe

    // returns true if wakelock was acquired
    bool acquire();
    void release(bool force = false);

    virtual ~JWakeLock();

private:
    uint32_t                mWakeLockCount;
    sp<JObjectHolder>       mWakeLock;
    const sp<JObjectHolder> mContext;

    void clearJavaWakeLock();

    DISALLOW_EVIL_CONSTRUCTORS(JWakeLock);
};

}  // namespace android

#endif  // J_WAKELOCK_H_
