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

#define LOG_TAG "DPBase"
//#define LOG_NDEBUG 0

#include <log/log.h>
#include "DPBase.h"
#include "DPFrequency.h"

namespace dp_fx {

DPStage::DPStage() : mInUse(DP_DEFAULT_STAGE_INUSE),
        mEnabled(DP_DEFAULT_STAGE_ENABLED) {
}

void DPStage::init(bool inUse, bool enabled) {
    mInUse = inUse;
    mEnabled = enabled;
}

//----
DPBandStage::DPBandStage() : mBandCount(0) {
}

void DPBandStage::init(bool inUse, bool enabled, int bandCount) {
    DPStage::init(inUse, enabled);
    mBandCount = inUse ? bandCount : 0;
}

//---
DPBandBase::DPBandBase() {
    init(DP_DEFAULT_BAND_ENABLED,
            DP_DEFAULT_BAND_CUTOFF_FREQUENCY_HZ);
}

void DPBandBase::init(bool enabled, float cutoffFrequency){
    mEnabled = enabled;
    mCutoofFrequencyHz = cutoffFrequency;
}

//-----
DPEqBand::DPEqBand() {
    init(DP_DEFAULT_BAND_ENABLED,
            DP_DEFAULT_BAND_CUTOFF_FREQUENCY_HZ,
            DP_DEFAULT_GAIN_DB);
}

void DPEqBand::init(bool enabled, float cutoffFrequency, float gain) {
    DPBandBase::init(enabled, cutoffFrequency);
    setGain(gain);
}

float DPEqBand::getGain() const{
    return mGainDb;
}

void DPEqBand::setGain(float gain) {
    mGainDb = gain;
}

//------
DPMbcBand::DPMbcBand() {
    init(DP_DEFAULT_BAND_ENABLED,
            DP_DEFAULT_BAND_CUTOFF_FREQUENCY_HZ,
            DP_DEFAULT_ATTACK_TIME_MS,
            DP_DEFAULT_RELEASE_TIME_MS,
            DP_DEFAULT_RATIO,
            DP_DEFAULT_THRESHOLD_DB,
            DP_DEFAULT_KNEE_WIDTH_DB,
            DP_DEFAULT_NOISE_GATE_THRESHOLD_DB,
            DP_DEFAULT_EXPANDER_RATIO,
            DP_DEFAULT_GAIN_DB,
            DP_DEFAULT_GAIN_DB);
}

void DPMbcBand::init(bool enabled, float cutoffFrequency, float attackTime, float releaseTime,
        float ratio, float threshold, float kneeWidth, float noiseGateThreshold,
        float expanderRatio, float preGain, float postGain) {
    DPBandBase::init(enabled, cutoffFrequency);
    setAttackTime(attackTime);
    setReleaseTime(releaseTime);
    setRatio(ratio);
    setThreshold(threshold);
    setKneeWidth(kneeWidth);
    setNoiseGateThreshold(noiseGateThreshold);
    setExpanderRatio(expanderRatio);
    setPreGain(preGain);
    setPostGain(postGain);
}

//------
DPEq::DPEq() {
}

void DPEq::init(bool inUse, bool enabled, uint32_t bandCount) {
    DPBandStage::init(inUse, enabled, bandCount);
    mBands.resize(getBandCount());
}

DPEqBand * DPEq::getBand(uint32_t band) {
    if (band < getBandCount()) {
        return &mBands[band];
    }
    return NULL;
}

void DPEq::setBand(uint32_t band, DPEqBand &src) {
    if (band < getBandCount()) {
        mBands[band] = src;
    }
}

//------
DPMbc::DPMbc() {
}

void DPMbc::init(bool inUse, bool enabled, uint32_t bandCount) {
    DPBandStage::init(inUse, enabled, bandCount);
    if (isInUse()) {
        mBands.resize(bandCount);
    } else {
        mBands.resize(0);
    }
}

DPMbcBand * DPMbc::getBand(uint32_t band) {
    if (band < getBandCount()) {
        return &mBands[band];
    }
    return NULL;
}

void DPMbc::setBand(uint32_t band, DPMbcBand &src) {
    if (band < getBandCount()) {
        mBands[band] = src;
    }
}

//------
DPLimiter::DPLimiter() {
    init(DP_DEFAULT_STAGE_INUSE,
            DP_DEFAULT_STAGE_ENABLED,
            DP_DEFAULT_LINK_GROUP,
            DP_DEFAULT_ATTACK_TIME_MS,
            DP_DEFAULT_RELEASE_TIME_MS,
            DP_DEFAULT_RATIO,
            DP_DEFAULT_THRESHOLD_DB,
            DP_DEFAULT_GAIN_DB);
}

void DPLimiter::init(bool inUse, bool enabled, uint32_t linkGroup, float attackTime, float releaseTime,
        float ratio, float threshold, float postGain) {
    DPStage::init(inUse, enabled);
    setLinkGroup(linkGroup);
    setAttackTime(attackTime);
    setReleaseTime(releaseTime);
    setRatio(ratio);
    setThreshold(threshold);
    setPostGain(postGain);
}

//----
DPChannel::DPChannel() : mInitialized(false), mInputGainDb(0), mOutputGainDb(0),
        mPreEqInUse(false), mMbcInUse(false), mPostEqInUse(false), mLimiterInUse(false) {
}

void DPChannel::init(float inputGain, bool preEqInUse, uint32_t preEqBandCount,
        bool mbcInUse, uint32_t mbcBandCount, bool postEqInUse, uint32_t postEqBandCount,
        bool limiterInUse) {
    setInputGain(inputGain);
    mPreEqInUse = preEqInUse;
    mMbcInUse = mbcInUse;
    mPostEqInUse = postEqInUse;
    mLimiterInUse = limiterInUse;

    mPreEq.init(mPreEqInUse, false, preEqBandCount);
    mMbc.init(mMbcInUse, false, mbcBandCount);
    mPostEq.init(mPostEqInUse, false, postEqBandCount);
    mLimiter.init(mLimiterInUse, false, 0, 50, 120, 2, -30, 0);
    mInitialized = true;
}

DPEq* DPChannel::getPreEq() {
    if (!mInitialized) {
        return NULL;
    }
    return &mPreEq;
}

DPMbc* DPChannel::getMbc() {
    if (!mInitialized) {
        return NULL;
    }
    return &mMbc;
}

DPEq* DPChannel::getPostEq() {
    if (!mInitialized) {
        return NULL;
    }
    return &mPostEq;
}

DPLimiter* DPChannel::getLimiter() {
    if (!mInitialized) {
        return NULL;
    }
    return &mLimiter;
}

void DPChannel::setLimiter(DPLimiter &limiter) {
    if (!mInitialized) {
        return;
    }
    mLimiter = limiter;
}

//----
DPBase::DPBase() : mInitialized(false), mChannelCount(0), mPreEqInUse(false), mPreEqBandCount(0),
        mMbcInUse(false), mMbcBandCount(0), mPostEqInUse(false), mPostEqBandCount(0),
        mLimiterInUse(false) {
}

void DPBase::init(uint32_t channelCount, bool preEqInUse, uint32_t preEqBandCount,
        bool mbcInUse, uint32_t mbcBandCount, bool postEqInUse, uint32_t postEqBandCount,
        bool limiterInUse) {
    ALOGV("DPBase::init");
    mChannelCount = channelCount;
    mPreEqInUse = preEqInUse;
    mPreEqBandCount = preEqBandCount;
    mMbcInUse = mbcInUse;
    mMbcBandCount = mbcBandCount;
    mPostEqInUse = postEqInUse;
    mPostEqBandCount = postEqBandCount;
    mLimiterInUse = limiterInUse;
    mChannel.resize(mChannelCount);
    for (size_t ch = 0; ch < mChannelCount; ch++) {
        mChannel[ch].init(0, preEqInUse, preEqBandCount, mbcInUse, mbcBandCount,
                postEqInUse, postEqBandCount, limiterInUse);
    }
    mInitialized = true;
}

DPChannel* DPBase::getChannel(uint32_t channelIndex) {
    if (!mInitialized || channelIndex < 0 || channelIndex >= mChannel.size()) {
        return NULL;
    }
    return & mChannel[channelIndex];
}

} //namespace dp_fx
