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

#define LOG_TAG "BatteryNotifier"
//#define LOG_NDEBUG 0

#include "include/mediautils/BatteryNotifier.h"

#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include <private/android_filesystem_config.h>

namespace android {

void BatteryNotifier::DeathNotifier::binderDied(const wp<IBinder>& /*who*/) {
    BatteryNotifier::getInstance().onBatteryStatServiceDied();
}

BatteryNotifier::BatteryNotifier() {}

BatteryNotifier::~BatteryNotifier() {
    Mutex::Autolock _l(mLock);
    if (mDeathNotifier != nullptr) {
        IInterface::asBinder(mBatteryStatService)->unlinkToDeath(mDeathNotifier);
    }
}

void BatteryNotifier::noteStartVideo(uid_t uid) {
    Mutex::Autolock _l(mLock);
    sp<IBatteryStats> batteryService = getBatteryService_l();
    if (mVideoRefCounts[uid] == 0 && batteryService != nullptr) {
        batteryService->noteStartVideo(uid);
    }
    mVideoRefCounts[uid]++;
}

void BatteryNotifier::noteStopVideo(uid_t uid) {
    Mutex::Autolock _l(mLock);
    if (mVideoRefCounts.find(uid) == mVideoRefCounts.end()) {
        ALOGW("%s: video refcount is broken for uid(%d).", __FUNCTION__, (int)uid);
        return;
    }

    sp<IBatteryStats> batteryService = getBatteryService_l();

    mVideoRefCounts[uid]--;
    if (mVideoRefCounts[uid] == 0) {
        if (batteryService != nullptr) {
            batteryService->noteStopVideo(uid);
        }
        mVideoRefCounts.erase(uid);
    }
}

void BatteryNotifier::noteResetVideo() {
    Mutex::Autolock _l(mLock);
    sp<IBatteryStats> batteryService = getBatteryService_l();
    mVideoRefCounts.clear();
    if (batteryService != nullptr) {
        batteryService->noteResetVideo();
    }
}

void BatteryNotifier::noteStartAudio(uid_t uid) {
    Mutex::Autolock _l(mLock);
    sp<IBatteryStats> batteryService = getBatteryService_l();
    if (mAudioRefCounts[uid] == 0 && batteryService != nullptr) {
        batteryService->noteStartAudio(uid);
    }
    mAudioRefCounts[uid]++;
}

void BatteryNotifier::noteStopAudio(uid_t uid) {
    Mutex::Autolock _l(mLock);
    if (mAudioRefCounts.find(uid) == mAudioRefCounts.end()) {
        ALOGW("%s: audio refcount is broken for uid(%d).", __FUNCTION__, (int)uid);
        return;
    }

    sp<IBatteryStats> batteryService = getBatteryService_l();

    mAudioRefCounts[uid]--;
    if (mAudioRefCounts[uid] == 0) {
        if (batteryService != nullptr) {
            batteryService->noteStopAudio(uid);
        }
        mAudioRefCounts.erase(uid);
    }
}

void BatteryNotifier::noteResetAudio() {
    Mutex::Autolock _l(mLock);
    sp<IBatteryStats> batteryService = getBatteryService_l();
    mAudioRefCounts.clear();
    if (batteryService != nullptr) {
        batteryService->noteResetAudio();
    }
}

void BatteryNotifier::noteFlashlightOn(const String8& id, uid_t uid) {
    Mutex::Autolock _l(mLock);
    sp<IBatteryStats> batteryService = getBatteryService_l();

    std::pair<String8, uid_t> k = std::make_pair(id, uid);
    if (!mFlashlightState[k]) {
        mFlashlightState[k] = true;
        if (batteryService != nullptr) {
            batteryService->noteFlashlightOn(uid);
        }
    }
}

void BatteryNotifier::noteFlashlightOff(const String8& id, uid_t uid) {
    Mutex::Autolock _l(mLock);
    sp<IBatteryStats> batteryService = getBatteryService_l();

    std::pair<String8, uid_t> k = std::make_pair(id, uid);
    if (mFlashlightState[k]) {
        mFlashlightState[k] = false;
        if (batteryService != nullptr) {
            batteryService->noteFlashlightOff(uid);
        }
    }
}

void BatteryNotifier::noteResetFlashlight() {
    Mutex::Autolock _l(mLock);
    sp<IBatteryStats> batteryService = getBatteryService_l();
    mFlashlightState.clear();
    if (batteryService != nullptr) {
        batteryService->noteResetFlashlight();
    }
}

void BatteryNotifier::noteStartCamera(const String8& id, uid_t uid) {
    Mutex::Autolock _l(mLock);
    sp<IBatteryStats> batteryService = getBatteryService_l();
    std::pair<String8, uid_t> k = std::make_pair(id, uid);
    if (!mCameraState[k]) {
        mCameraState[k] = true;
        if (batteryService != nullptr) {
            batteryService->noteStartCamera(uid);
        }
    }
}

void BatteryNotifier::noteStopCamera(const String8& id, uid_t uid) {
    Mutex::Autolock _l(mLock);
    sp<IBatteryStats> batteryService = getBatteryService_l();
    std::pair<String8, uid_t> k = std::make_pair(id, uid);
    if (mCameraState[k]) {
        mCameraState[k] = false;
        if (batteryService != nullptr) {
            batteryService->noteStopCamera(uid);
        }
    }
}

void BatteryNotifier::noteResetCamera() {
    Mutex::Autolock _l(mLock);
    sp<IBatteryStats> batteryService = getBatteryService_l();
    mCameraState.clear();
    if (batteryService != nullptr) {
        batteryService->noteResetCamera();
    }
}

void BatteryNotifier::onBatteryStatServiceDied() {
    Mutex::Autolock _l(mLock);
    mBatteryStatService.clear();
    mDeathNotifier.clear();
    // Do not reset mVideoRefCounts and mAudioRefCounts here. The ref
    // counting is independent of the battery service availability.
    // We need this if battery service becomes available after media
    // started.

}

sp<IBatteryStats> BatteryNotifier::getBatteryService_l() {
    if (mBatteryStatService != nullptr) {
        return mBatteryStatService;
    }
    // Get battery service from service manager
    const sp<IServiceManager> sm(defaultServiceManager());
    if (sm != nullptr) {
        const String16 name("batterystats");
        mBatteryStatService = interface_cast<IBatteryStats>(sm->checkService(name));
        if (mBatteryStatService == nullptr) {
            // this may occur normally during the init sequence as mediaserver
            // and audioserver start before the batterystats service is available.
            ALOGW("batterystats service unavailable!");
            return nullptr;
        }

        mDeathNotifier = new DeathNotifier();
        IInterface::asBinder(mBatteryStatService)->linkToDeath(mDeathNotifier);

        // Notify start now if mediaserver or audioserver is already started.
        // 1) mediaserver and audioserver is started before batterystats service
        // 2) batterystats server may have crashed.
        std::map<uid_t, int>::iterator it = mVideoRefCounts.begin();
        for (; it != mVideoRefCounts.end(); ++it) {
            mBatteryStatService->noteStartVideo(it->first);
        }
        it = mAudioRefCounts.begin();
        for (; it != mAudioRefCounts.end(); ++it) {
            mBatteryStatService->noteStartAudio(it->first);
        }
        // TODO: Notify for camera and flashlight state as well?
    }
    return mBatteryStatService;
}

ANDROID_SINGLETON_STATIC_INSTANCE(BatteryNotifier);

}  // namespace android
