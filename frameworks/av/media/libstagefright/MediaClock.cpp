/*
 * Copyright (C) 2015 The Android Open Source Project
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
#define LOG_TAG "MediaClock"
#include <utils/Log.h>
#include <map>

#include <media/stagefright/MediaClock.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>

namespace android {

// Maximum allowed time backwards from anchor change.
// If larger than this threshold, it's treated as discontinuity.
static const int64_t kAnchorFluctuationAllowedUs = 10000LL;

MediaClock::Timer::Timer(const sp<AMessage> &notify, int64_t mediaTimeUs, int64_t adjustRealUs)
    : mNotify(notify),
      mMediaTimeUs(mediaTimeUs),
      mAdjustRealUs(adjustRealUs) {
}

MediaClock::MediaClock()
    : mAnchorTimeMediaUs(-1),
      mAnchorTimeRealUs(-1),
      mMaxTimeMediaUs(INT64_MAX),
      mStartingTimeMediaUs(-1),
      mPlaybackRate(1.0),
      mGeneration(0) {
    mLooper = new ALooper;
    mLooper->setName("MediaClock");
    mLooper->start(false /* runOnCallingThread */,
                   false /* canCallJava */,
                   ANDROID_PRIORITY_AUDIO);
}

void MediaClock::init() {
    mLooper->registerHandler(this);
}

MediaClock::~MediaClock() {
    reset();
    if (mLooper != NULL) {
        mLooper->unregisterHandler(id());
        mLooper->stop();
    }
}

void MediaClock::reset() {
    Mutex::Autolock autoLock(mLock);
    auto it = mTimers.begin();
    while (it != mTimers.end()) {
        it->mNotify->setInt32("reason", TIMER_REASON_RESET);
        it->mNotify->post();
        it = mTimers.erase(it);
    }
    mMaxTimeMediaUs = INT64_MAX;
    mStartingTimeMediaUs = -1;
    updateAnchorTimesAndPlaybackRate_l(-1, -1, 1.0);
    ++mGeneration;
}

void MediaClock::setStartingTimeMedia(int64_t startingTimeMediaUs) {
    Mutex::Autolock autoLock(mLock);
    mStartingTimeMediaUs = startingTimeMediaUs;
}

void MediaClock::clearAnchor() {
    Mutex::Autolock autoLock(mLock);
    updateAnchorTimesAndPlaybackRate_l(-1, -1, mPlaybackRate);
}

void MediaClock::updateAnchor(
        int64_t anchorTimeMediaUs,
        int64_t anchorTimeRealUs,
        int64_t maxTimeMediaUs) {
    if (anchorTimeMediaUs < 0 || anchorTimeRealUs < 0) {
        ALOGW("reject anchor time since it is negative.");
        return;
    }

    Mutex::Autolock autoLock(mLock);
    int64_t nowUs = ALooper::GetNowUs();
    int64_t nowMediaUs =
        anchorTimeMediaUs + (nowUs - anchorTimeRealUs) * (double)mPlaybackRate;
    if (nowMediaUs < 0) {
        ALOGW("reject anchor time since it leads to negative media time.");
        return;
    }

    if (maxTimeMediaUs != -1) {
        mMaxTimeMediaUs = maxTimeMediaUs;
    }
    if (mAnchorTimeRealUs != -1) {
        int64_t oldNowMediaUs =
            mAnchorTimeMediaUs + (nowUs - mAnchorTimeRealUs) * (double)mPlaybackRate;
        if (nowMediaUs < oldNowMediaUs + kAnchorFluctuationAllowedUs
                && nowMediaUs > oldNowMediaUs - kAnchorFluctuationAllowedUs) {
            return;
        }
    }
    updateAnchorTimesAndPlaybackRate_l(nowMediaUs, nowUs, mPlaybackRate);

    ++mGeneration;
    processTimers_l();
}

void MediaClock::updateMaxTimeMedia(int64_t maxTimeMediaUs) {
    Mutex::Autolock autoLock(mLock);
    mMaxTimeMediaUs = maxTimeMediaUs;
}

void MediaClock::setPlaybackRate(float rate) {
    CHECK_GE(rate, 0.0);
    Mutex::Autolock autoLock(mLock);
    if (mAnchorTimeRealUs == -1) {
        mPlaybackRate = rate;
        return;
    }

    int64_t nowUs = ALooper::GetNowUs();
    int64_t nowMediaUs = mAnchorTimeMediaUs + (nowUs - mAnchorTimeRealUs) * (double)mPlaybackRate;
    if (nowMediaUs < 0) {
        ALOGW("setRate: anchor time should not be negative, set to 0.");
        nowMediaUs = 0;
    }
    updateAnchorTimesAndPlaybackRate_l(nowMediaUs, nowUs, rate);

    if (rate > 0.0) {
        ++mGeneration;
        processTimers_l();
    }
}

float MediaClock::getPlaybackRate() const {
    Mutex::Autolock autoLock(mLock);
    return mPlaybackRate;
}

status_t MediaClock::getMediaTime(
        int64_t realUs, int64_t *outMediaUs, bool allowPastMaxTime) const {
    if (outMediaUs == NULL) {
        return BAD_VALUE;
    }

    Mutex::Autolock autoLock(mLock);
    return getMediaTime_l(realUs, outMediaUs, allowPastMaxTime);
}

status_t MediaClock::getMediaTime_l(
        int64_t realUs, int64_t *outMediaUs, bool allowPastMaxTime) const {
    if (mAnchorTimeRealUs == -1) {
        return NO_INIT;
    }

    int64_t mediaUs = mAnchorTimeMediaUs
            + (realUs - mAnchorTimeRealUs) * (double)mPlaybackRate;
    if (mediaUs > mMaxTimeMediaUs && !allowPastMaxTime) {
        mediaUs = mMaxTimeMediaUs;
    }
    if (mediaUs < mStartingTimeMediaUs) {
        mediaUs = mStartingTimeMediaUs;
    }
    if (mediaUs < 0) {
        mediaUs = 0;
    }
    *outMediaUs = mediaUs;
    return OK;
}

status_t MediaClock::getRealTimeFor(
        int64_t targetMediaUs, int64_t *outRealUs) const {
    if (outRealUs == NULL) {
        return BAD_VALUE;
    }

    Mutex::Autolock autoLock(mLock);
    if (mPlaybackRate == 0.0) {
        return NO_INIT;
    }

    int64_t nowUs = ALooper::GetNowUs();
    int64_t nowMediaUs;
    status_t status =
            getMediaTime_l(nowUs, &nowMediaUs, true /* allowPastMaxTime */);
    if (status != OK) {
        return status;
    }
    *outRealUs = (targetMediaUs - nowMediaUs) / (double)mPlaybackRate + nowUs;
    return OK;
}

void MediaClock::addTimer(const sp<AMessage> &notify, int64_t mediaTimeUs,
                          int64_t adjustRealUs) {
    Mutex::Autolock autoLock(mLock);

    bool updateTimer = (mPlaybackRate != 0.0);
    if (updateTimer) {
        auto it = mTimers.begin();
        while (it != mTimers.end()) {
            if (((it->mAdjustRealUs - (double)adjustRealUs) * (double)mPlaybackRate
                + (it->mMediaTimeUs - mediaTimeUs)) <= 0) {
                updateTimer = false;
                break;
            }
            ++it;
        }
    }

    mTimers.emplace_back(notify, mediaTimeUs, adjustRealUs);

    if (updateTimer) {
        ++mGeneration;
        processTimers_l();
    }
}

void MediaClock::onMessageReceived(const sp<AMessage> &msg) {
    switch (msg->what()) {
        case kWhatTimeIsUp:
        {
            int32_t generation;
            CHECK(msg->findInt32("generation", &generation));

            Mutex::Autolock autoLock(mLock);
            if (generation != mGeneration) {
                break;
            }
            processTimers_l();
            break;
        }

        default:
            TRESPASS();
            break;
    }
}

void MediaClock::processTimers_l() {
    int64_t nowMediaTimeUs;
    status_t status = getMediaTime_l(
            ALooper::GetNowUs(), &nowMediaTimeUs, false /* allowPastMaxTime */);

    if (status != OK) {
        return;
    }

    int64_t nextLapseRealUs = INT64_MAX;
    std::multimap<int64_t, Timer> notifyList;
    auto it = mTimers.begin();
    while (it != mTimers.end()) {
        double diff = it->mAdjustRealUs * (double)mPlaybackRate
            + it->mMediaTimeUs - nowMediaTimeUs;
        int64_t diffMediaUs;
        if (diff > (double)INT64_MAX) {
            diffMediaUs = INT64_MAX;
        } else if (diff < (double)INT64_MIN) {
            diffMediaUs = INT64_MIN;
        } else {
            diffMediaUs = diff;
        }

        if (diffMediaUs <= 0) {
            notifyList.emplace(diffMediaUs, *it);
            it = mTimers.erase(it);
        } else {
            if (mPlaybackRate != 0.0
                && (double)diffMediaUs < INT64_MAX * (double)mPlaybackRate) {
                int64_t targetRealUs = diffMediaUs / (double)mPlaybackRate;
                if (targetRealUs < nextLapseRealUs) {
                    nextLapseRealUs = targetRealUs;
                }
            }
            ++it;
        }
    }

    auto itNotify = notifyList.begin();
    while (itNotify != notifyList.end()) {
        itNotify->second.mNotify->setInt32("reason", TIMER_REASON_REACHED);
        itNotify->second.mNotify->post();
        itNotify = notifyList.erase(itNotify);
    }

    if (mTimers.empty() || mPlaybackRate == 0.0 || mAnchorTimeMediaUs < 0
        || nextLapseRealUs == INT64_MAX) {
        return;
    }

    sp<AMessage> msg = new AMessage(kWhatTimeIsUp, this);
    msg->setInt32("generation", mGeneration);
    msg->post(nextLapseRealUs);
}

void MediaClock::updateAnchorTimesAndPlaybackRate_l(int64_t anchorTimeMediaUs,
        int64_t anchorTimeRealUs, float playbackRate) {
    if (mAnchorTimeMediaUs != anchorTimeMediaUs
            || mAnchorTimeRealUs != anchorTimeRealUs
            || mPlaybackRate != playbackRate) {
        mAnchorTimeMediaUs = anchorTimeMediaUs;
        mAnchorTimeRealUs = anchorTimeRealUs;
        mPlaybackRate = playbackRate;
        notifyDiscontinuity_l();
    }
}

void MediaClock::setNotificationMessage(const sp<AMessage> &msg) {
    Mutex::Autolock autoLock(mLock);
    mNotify = msg;
}

void MediaClock::notifyDiscontinuity_l() {
    if (mNotify != nullptr) {
        sp<AMessage> msg = mNotify->dup();
        msg->setInt64("anchor-media-us", mAnchorTimeMediaUs);
        msg->setInt64("anchor-real-us", mAnchorTimeRealUs);
        msg->setFloat("playback-rate", mPlaybackRate);
        msg->post();
    }
}

}  // namespace android
