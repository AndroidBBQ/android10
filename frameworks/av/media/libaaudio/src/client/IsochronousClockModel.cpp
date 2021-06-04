/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "IsochronousClockModel"
//#define LOG_NDEBUG 0
#include <log/log.h>

#include <stdint.h>
#include <algorithm>

#include "utility/AudioClock.h"
#include "IsochronousClockModel.h"

using namespace aaudio;

IsochronousClockModel::IsochronousClockModel()
        : mMarkerFramePosition(0)
        , mMarkerNanoTime(0)
        , mSampleRate(48000)
        , mFramesPerBurst(64)
        , mMaxMeasuredLatenessNanos(0)
        , mState(STATE_STOPPED)
{
}

IsochronousClockModel::~IsochronousClockModel() {
}

void IsochronousClockModel::setPositionAndTime(int64_t framePosition, int64_t nanoTime) {
    ALOGV("setPositionAndTime, %lld, %lld", (long long) framePosition, (long long) nanoTime);
    mMarkerFramePosition = framePosition;
    mMarkerNanoTime = nanoTime;
}

void IsochronousClockModel::start(int64_t nanoTime) {
    ALOGV("start(nanos = %lld)\n", (long long) nanoTime);
    mMarkerNanoTime = nanoTime;
    mState = STATE_STARTING;
}

void IsochronousClockModel::stop(int64_t nanoTime) {
    ALOGD("stop(nanos = %lld) max lateness = %d micros\n",
        (long long) nanoTime,
        (int) (mMaxMeasuredLatenessNanos / 1000));
    setPositionAndTime(convertTimeToPosition(nanoTime), nanoTime);
    // TODO should we set position?
    mState = STATE_STOPPED;
}

bool IsochronousClockModel::isStarting() const {
    return mState == STATE_STARTING;
}

bool IsochronousClockModel::isRunning() const {
    return mState == STATE_RUNNING;
}

void IsochronousClockModel::processTimestamp(int64_t framePosition, int64_t nanoTime) {
    mTimestampCount++;
// Log position and time in CSV format so we can import it easily into spreadsheets.
    //ALOGD("%s() CSV, %d, %lld, %lld", __func__,
          //mTimestampCount, (long long)framePosition, (long long)nanoTime);
    int64_t framesDelta = framePosition - mMarkerFramePosition;
    int64_t nanosDelta = nanoTime - mMarkerNanoTime;
    if (nanosDelta < 1000) {
        return;
    }

//    ALOGD("processTimestamp() - mMarkerFramePosition = %lld at mMarkerNanoTime %llu",
//         (long long)mMarkerFramePosition,
//         (long long)mMarkerNanoTime);

    int64_t expectedNanosDelta = convertDeltaPositionToTime(framesDelta);
//    ALOGD("processTimestamp() - expectedNanosDelta = %lld, nanosDelta = %llu",
//         (long long)expectedNanosDelta,
//         (long long)nanosDelta);

//    ALOGD("processTimestamp() - mSampleRate = %d", mSampleRate);
//    ALOGD("processTimestamp() - mState = %d", mState);
    switch (mState) {
    case STATE_STOPPED:
        break;
    case STATE_STARTING:
        setPositionAndTime(framePosition, nanoTime);
        mState = STATE_SYNCING;
        break;
    case STATE_SYNCING:
        // This will handle a burst of rapid transfer at the beginning.
        if (nanosDelta < expectedNanosDelta) {
            setPositionAndTime(framePosition, nanoTime);
        } else {
//            ALOGD("processTimestamp() - advance to STATE_RUNNING");
            mState = STATE_RUNNING;
        }
        break;
    case STATE_RUNNING:
        if (nanosDelta < expectedNanosDelta) {
            // Earlier than expected timestamp.
            // This data is probably more accurate, so use it.
            // Or we may be drifting due to a fast HW clock.
            //int microsDelta = (int) (nanosDelta / 1000);
            //int expectedMicrosDelta = (int) (expectedNanosDelta / 1000);
            //ALOGD("%s() - STATE_RUNNING - #%d, %4d micros EARLY",
                //__func__, mTimestampCount, expectedMicrosDelta - microsDelta);

            setPositionAndTime(framePosition, nanoTime);
        } else if (nanosDelta > (expectedNanosDelta + (2 * mBurstPeriodNanos))) {
            // In this case we do not update mMaxMeasuredLatenessNanos because it
            // would force it too high.
            // mMaxMeasuredLatenessNanos should range from 1 to 2 * mBurstPeriodNanos
            //int32_t measuredLatenessNanos = (int32_t)(nanosDelta - expectedNanosDelta);
            //ALOGD("%s() - STATE_RUNNING - #%d, lateness %d - max %d = %4d micros VERY LATE",
                  //__func__,
                  //mTimestampCount,
                  //measuredLatenessNanos / 1000,
                  //mMaxMeasuredLatenessNanos / 1000,
                  //(measuredLatenessNanos - mMaxMeasuredLatenessNanos) / 1000
                  //);

            // This typically happens when we are modelling a service instead of a DSP.
            setPositionAndTime(framePosition,  nanoTime - (2 * mBurstPeriodNanos));
        } else if (nanosDelta > (expectedNanosDelta + mMaxMeasuredLatenessNanos)) {
            //int32_t previousLatenessNanos = mMaxMeasuredLatenessNanos;
            mMaxMeasuredLatenessNanos = (int32_t)(nanosDelta - expectedNanosDelta);

            //ALOGD("%s() - STATE_RUNNING - #%d, newmax %d - oldmax %d = %4d micros LATE",
                  //__func__,
                  //mTimestampCount,
                  //mMaxMeasuredLatenessNanos / 1000,
                  //previousLatenessNanos / 1000,
                  //(mMaxMeasuredLatenessNanos - previousLatenessNanos) / 1000
                  //);

            // When we are late, it may be because of preemption in the kernel,
            // or timing jitter caused by resampling in the DSP,
            // or we may be drifting due to a slow HW clock.
            // We add slight drift value just in case there is actual long term drift
            // forward caused by a slower clock.
            // If the clock is faster than the model will get pushed earlier
            // by the code in the preceding branch.
            // The two opposing forces should allow the model to track the real clock
            // over a long time.
            int64_t driftingTime = mMarkerNanoTime + expectedNanosDelta + kDriftNanos;
            setPositionAndTime(framePosition,  driftingTime);
            //ALOGD("%s() - #%d, max lateness = %d micros",
                  //__func__,
                  //mTimestampCount,
                  //(int) (mMaxMeasuredLatenessNanos / 1000));
        }
        break;
    default:
        break;
    }

//    ALOGD("processTimestamp() - mState = %d", mState);
}

void IsochronousClockModel::setSampleRate(int32_t sampleRate) {
    mSampleRate = sampleRate;
    update();
}

void IsochronousClockModel::setFramesPerBurst(int32_t framesPerBurst) {
    mFramesPerBurst = framesPerBurst;
    update();
}

// Update expected lateness based on sampleRate and framesPerBurst
void IsochronousClockModel::update() {
    mBurstPeriodNanos = convertDeltaPositionToTime(mFramesPerBurst); // uses mSampleRate
    // Timestamps may be late by up to a burst because we are randomly sampling the time period
    // after the DSP position is actually updated.
    mMaxMeasuredLatenessNanos = mBurstPeriodNanos;
}

int64_t IsochronousClockModel::convertDeltaPositionToTime(int64_t framesDelta) const {
    return (AAUDIO_NANOS_PER_SECOND * framesDelta) / mSampleRate;
}

int64_t IsochronousClockModel::convertDeltaTimeToPosition(int64_t nanosDelta) const {
    return (mSampleRate * nanosDelta) / AAUDIO_NANOS_PER_SECOND;
}

int64_t IsochronousClockModel::convertPositionToTime(int64_t framePosition) const {
    if (mState == STATE_STOPPED) {
        return mMarkerNanoTime;
    }
    int64_t nextBurstIndex = (framePosition + mFramesPerBurst - 1) / mFramesPerBurst;
    int64_t nextBurstPosition = mFramesPerBurst * nextBurstIndex;
    int64_t framesDelta = nextBurstPosition - mMarkerFramePosition;
    int64_t nanosDelta = convertDeltaPositionToTime(framesDelta);
    int64_t time = mMarkerNanoTime + nanosDelta;
//    ALOGD("convertPositionToTime: pos = %llu --> time = %llu",
//         (unsigned long long)framePosition,
//         (unsigned long long)time);
    return time;
}

int64_t IsochronousClockModel::convertTimeToPosition(int64_t nanoTime) const {
    if (mState == STATE_STOPPED) {
        return mMarkerFramePosition;
    }
    int64_t nanosDelta = nanoTime - mMarkerNanoTime;
    int64_t framesDelta = convertDeltaTimeToPosition(nanosDelta);
    int64_t nextBurstPosition = mMarkerFramePosition + framesDelta;
    int64_t nextBurstIndex = nextBurstPosition / mFramesPerBurst;
    int64_t position = nextBurstIndex * mFramesPerBurst;
//    ALOGD("convertTimeToPosition: time = %llu --> pos = %llu",
//         (unsigned long long)nanoTime,
//         (unsigned long long)position);
//    ALOGD("convertTimeToPosition: framesDelta = %llu, mFramesPerBurst = %d",
//         (long long) framesDelta, mFramesPerBurst);
    return position;
}

int32_t IsochronousClockModel::getLateTimeOffsetNanos() const {
    // This will never be < 0 because mMaxLatenessNanos starts at
    // mBurstPeriodNanos and only gets bigger.
    return (mMaxMeasuredLatenessNanos - mBurstPeriodNanos) + kExtraLatenessNanos;
}

int64_t IsochronousClockModel::convertPositionToLatestTime(int64_t framePosition) const {
    return convertPositionToTime(framePosition) + getLateTimeOffsetNanos();
}

int64_t IsochronousClockModel::convertLatestTimeToPosition(int64_t nanoTime) const {
    return convertTimeToPosition(nanoTime - getLateTimeOffsetNanos());
}

void IsochronousClockModel::dump() const {
    ALOGD("mMarkerFramePosition = %lld", (long long) mMarkerFramePosition);
    ALOGD("mMarkerNanoTime      = %lld", (long long) mMarkerNanoTime);
    ALOGD("mSampleRate          = %6d", mSampleRate);
    ALOGD("mFramesPerBurst      = %6d", mFramesPerBurst);
    ALOGD("mMaxMeasuredLatenessNanos = %6d", mMaxMeasuredLatenessNanos);
    ALOGD("mState               = %6d", mState);
}
