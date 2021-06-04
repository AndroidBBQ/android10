/*
 * Copyright (C) 2011 The Android Open Source Project
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
#define LOG_TAG "HTTPBase"
#include <utils/Log.h>

#include "include/HTTPBase.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>

#include <cutils/properties.h>
#include <cutils/qtaguid.h>

namespace android {

HTTPBase::HTTPBase()
    : mNumBandwidthHistoryItems(0),
      mTotalTransferTimeUs(0),
      mTotalTransferBytes(0),
      mMaxBandwidthHistoryItems(100),
      mPrevBandwidthMeasureTimeUs(0),
      mPrevEstimatedBandWidthKbps(0),
      mBandWidthCollectFreqMs(5000) {
    mName = String8("HTTPBase(<disconnected>)");
}

void HTTPBase::addBandwidthMeasurement(
        size_t numBytes, int64_t delayUs) {
    Mutex::Autolock autoLock(mLock);

    BandwidthEntry entry;
    entry.mDelayUs = delayUs;
    entry.mNumBytes = numBytes;
    mTotalTransferTimeUs += delayUs;
    mTotalTransferBytes += numBytes;

    mBandwidthHistory.push_back(entry);
    if (++mNumBandwidthHistoryItems > mMaxBandwidthHistoryItems) {
        BandwidthEntry *entry = &*mBandwidthHistory.begin();
        mTotalTransferTimeUs -= entry->mDelayUs;
        mTotalTransferBytes -= entry->mNumBytes;
        mBandwidthHistory.erase(mBandwidthHistory.begin());
        --mNumBandwidthHistoryItems;

        int64_t timeNowUs = ALooper::GetNowUs();
        if (timeNowUs - mPrevBandwidthMeasureTimeUs >=
                mBandWidthCollectFreqMs * 1000LL) {

            if (mPrevBandwidthMeasureTimeUs != 0) {
                mPrevEstimatedBandWidthKbps =
                    (mTotalTransferBytes * 8E3 / mTotalTransferTimeUs);
            }
            mPrevBandwidthMeasureTimeUs = timeNowUs;
        }
    }

}

bool HTTPBase::estimateBandwidth(int32_t *bandwidth_bps) {
    Mutex::Autolock autoLock(mLock);

    // Do not do bandwidth estimation if we don't have enough samples, or
    // total bytes download are too small (<64K).
    // Bandwidth estimation from these samples can often shoot up and cause
    // unwanted bw adaption behaviors.
    if (mNumBandwidthHistoryItems < 2 || mTotalTransferBytes < 65536) {
        return false;
    }

    *bandwidth_bps = ((double)mTotalTransferBytes * 8E6 / mTotalTransferTimeUs);

    return true;
}

status_t HTTPBase::getEstimatedBandwidthKbps(int32_t *kbps) {
    Mutex::Autolock autoLock(mLock);
    *kbps = mPrevEstimatedBandWidthKbps;
    return OK;
}

status_t HTTPBase::setBandwidthStatCollectFreq(int32_t freqMs) {
    Mutex::Autolock autoLock(mLock);

    if (freqMs < kMinBandwidthCollectFreqMs
            || freqMs > kMaxBandwidthCollectFreqMs) {

        ALOGE("frequency (%d ms) is out of range [1000, 60000]", freqMs);
        return BAD_VALUE;
    }

    ALOGI("frequency set to %d ms", freqMs);
    mBandWidthCollectFreqMs = freqMs;
    return OK;
}

void HTTPBase::setBandwidthHistorySize(size_t numHistoryItems) {
    mMaxBandwidthHistoryItems = numHistoryItems;
}

}  // namespace android
