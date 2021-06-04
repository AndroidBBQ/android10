/*
 * Copyright 2015, The Android Open Source Project
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

#ifndef MEDIA_BATTERY_NOTIFIER_H
#define MEDIA_BATTERY_NOTIFIER_H

#include <binder/IBatteryStats.h>
#include <utils/Singleton.h>
#include <utils/String8.h>

#include <map>
#include <utility>

namespace android {

/**
 * Class used for logging battery life events in mediaserver.
 */
class BatteryNotifier : public Singleton<BatteryNotifier> {

    friend class Singleton<BatteryNotifier>;
    BatteryNotifier();

public:
    ~BatteryNotifier();

    void noteStartVideo(uid_t uid);
    void noteStopVideo(uid_t uid);
    void noteResetVideo();
    void noteStartAudio(uid_t uid);
    void noteStopAudio(uid_t uid);
    void noteResetAudio();
    void noteFlashlightOn(const String8& id, uid_t uid);
    void noteFlashlightOff(const String8& id, uid_t uid);
    void noteResetFlashlight();
    void noteStartCamera(const String8& id, uid_t uid);
    void noteStopCamera(const String8& id, uid_t uid);
    void noteResetCamera();

private:
    void onBatteryStatServiceDied();

    class DeathNotifier : public IBinder::DeathRecipient {
        virtual void binderDied(const wp<IBinder>& /*who*/);
    };

    Mutex mLock;
    std::map<uid_t, int> mVideoRefCounts;
    std::map<uid_t, int> mAudioRefCounts;
    std::map<std::pair<String8, uid_t>, bool> mFlashlightState;
    std::map<std::pair<String8, uid_t>, bool> mCameraState;
    sp<IBatteryStats> mBatteryStatService;
    sp<DeathNotifier> mDeathNotifier;

    sp<IBatteryStats> getBatteryService_l();
};

}  // namespace android

#endif // MEDIA_BATTERY_NOTIFIER_H
