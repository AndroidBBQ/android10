/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef BATTERY_CHECKER_H_
#define BATTERY_CHECKER_H_

#include <media/stagefright/foundation/AMessage.h>

namespace android {

struct BatteryChecker : public RefBase {
    BatteryChecker(const sp<AMessage> &msg, int64_t timeout = 3000000ll);

    void setExecuting(bool executing) { mIsExecuting = executing; }
    void onCodecActivity(std::function<void()> batteryOnCb);
    void onCheckBatteryTimer(const sp<AMessage>& msg, std::function<void()> batteryOffCb);
    void onClientRemoved();

private:
    const int64_t mTimeoutUs;
    int64_t mLastActivityTimeUs;
    bool mBatteryStatNotified;
    int32_t mBatteryCheckerGeneration;
    bool mIsExecuting;
    sp<AMessage> mBatteryCheckerMsg;

    bool isExecuting() { return mIsExecuting; }

    DISALLOW_EVIL_CONSTRUCTORS(BatteryChecker);
};

}  // namespace android

#endif // BATTERY_CHECKER_H_
