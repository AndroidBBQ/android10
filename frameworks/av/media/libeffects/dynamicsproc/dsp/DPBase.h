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

#ifndef DPBASE_H_
#define DPBASE_H_


#include <stdint.h>
#include <cmath>
#include <vector>
#include <android/log.h>

namespace dp_fx {

#define DP_DEFAULT_BAND_ENABLED false
#define DP_DEFAULT_BAND_CUTOFF_FREQUENCY_HZ 1000
#define DP_DEFAULT_ATTACK_TIME_MS 50
#define DP_DEFAULT_RELEASE_TIME_MS 120
#define DP_DEFAULT_RATIO 2
#define DP_DEFAULT_THRESHOLD_DB -30
#define DP_DEFAULT_KNEE_WIDTH_DB 0
#define DP_DEFAULT_NOISE_GATE_THRESHOLD_DB -90
#define DP_DEFAULT_EXPANDER_RATIO 1
#define DP_DEFAULT_GAIN_DB 0
#define DP_DEFAULT_STAGE_INUSE false
#define DP_DEFAULT_STAGE_ENABLED false
#define DP_DEFAULT_LINK_GROUP 0



class DPStage {
public:
    DPStage();
    ~DPStage() = default;
    void init(bool inUse, bool enabled);
    bool isInUse() const {
        return mInUse;
    }
    bool isEnabled() const {
        return mEnabled;
    }
    void setEnabled(bool enabled) {
        mEnabled = enabled;
    }
private:
    bool mInUse;
    bool mEnabled;
};

class DPBandStage : public DPStage {
public:
    DPBandStage();
    ~DPBandStage() = default;
    void init(bool inUse, bool enabled, int bandCount);
    uint32_t getBandCount() const {
        return mBandCount;
    }
    void setBandCount(uint32_t bandCount) {
        mBandCount = bandCount;
    }
private:
    uint32_t mBandCount;
};

class DPBandBase {
public:
    DPBandBase();
    ~DPBandBase() = default;
    void init(bool enabled, float cutoffFrequency);
    bool isEnabled() const {
        return mEnabled;
    }
    void setEnabled(bool enabled) {
        mEnabled = enabled;
    }
    float getCutoffFrequency() const {
        return mCutoofFrequencyHz;
    }
    void setCutoffFrequency(float cutoffFrequency) {
        mCutoofFrequencyHz = cutoffFrequency;
    }
private:
    bool mEnabled;
    float mCutoofFrequencyHz;
};

class DPEqBand : public DPBandBase {
public:
    DPEqBand();
    ~DPEqBand() = default;
    void init(bool enabled, float cutoffFrequency, float gain);
    float getGain() const;
    void setGain(float gain);
private:
    float mGainDb;
};

class DPMbcBand : public DPBandBase {
public:
    DPMbcBand();
    ~DPMbcBand() = default;
    void init(bool enabled, float cutoffFrequency, float attackTime, float releaseTime,
            float ratio, float threshold, float kneeWidth, float noiseGateThreshold,
            float expanderRatio, float preGain, float postGain);
    float getAttackTime() const {
        return mAttackTimeMs;
    }
    void setAttackTime(float attackTime) {
        mAttackTimeMs = attackTime;
    }
    float getReleaseTime() const {
        return mReleaseTimeMs;
    }
    void setReleaseTime(float releaseTime) {
        mReleaseTimeMs = releaseTime;
    }
    float getRatio() const {
        return mRatio;
    }
    void setRatio(float ratio) {
        mRatio = ratio;
    }
    float getThreshold() const {
        return mThresholdDb;
    }
    void setThreshold(float threshold) {
        mThresholdDb = threshold;
    }
    float getKneeWidth() const {
        return mKneeWidthDb;
    }
    void setKneeWidth(float kneeWidth) {
        mKneeWidthDb = kneeWidth;
    }
    float getNoiseGateThreshold() const {
        return mNoiseGateThresholdDb;
    }
    void setNoiseGateThreshold(float noiseGateThreshold) {
        mNoiseGateThresholdDb = noiseGateThreshold;
    }
    float getExpanderRatio() const {
        return mExpanderRatio;
    }
    void setExpanderRatio(float expanderRatio) {
        mExpanderRatio = expanderRatio;
    }
    float getPreGain() const {
        return mPreGainDb;
    }
    void setPreGain(float preGain) {
        mPreGainDb = preGain;
    }
    float getPostGain() const {
        return mPostGainDb;
    }
    void setPostGain(float postGain) {
        mPostGainDb = postGain;
    }
private:
    float mAttackTimeMs;
    float mReleaseTimeMs;
    float mRatio;
    float mThresholdDb;
    float mKneeWidthDb;
    float mNoiseGateThresholdDb;
    float mExpanderRatio;
    float mPreGainDb;
    float mPostGainDb;
};

class DPEq : public DPBandStage {
public:
    DPEq();
    ~DPEq() = default;
    void init(bool inUse, bool enabled, uint32_t bandCount);
    DPEqBand * getBand(uint32_t band);
    void setBand(uint32_t band, DPEqBand &src);
private:
    std::vector<DPEqBand> mBands;
};

class DPMbc : public DPBandStage {
public:
    DPMbc();
    ~DPMbc() = default;
    void init(bool inUse, bool enabled, uint32_t bandCount);
    DPMbcBand * getBand(uint32_t band);
    void setBand(uint32_t band, DPMbcBand &src);
private:
    std::vector<DPMbcBand> mBands;
};

class DPLimiter : public DPStage {
public:
    DPLimiter();
    ~DPLimiter() = default;
    void init(bool inUse, bool enabled, uint32_t linkGroup, float attackTime, float releaseTime,
            float ratio, float threshold, float postGain);
    uint32_t getLinkGroup() const {
        return mLinkGroup;
    }
    void setLinkGroup(uint32_t linkGroup) {
        mLinkGroup = linkGroup;
    }
    float getAttackTime() const {
        return mAttackTimeMs;
    }
    void setAttackTime(float attackTime) {
        mAttackTimeMs = attackTime;
    }
    float getReleaseTime() const {
        return mReleaseTimeMs;
    }
    void setReleaseTime(float releaseTime) {
        mReleaseTimeMs = releaseTime;
    }
    float getRatio() const {
        return mRatio;
    }
    void setRatio(float ratio) {
        mRatio = ratio;
    }
    float getThreshold() const {
        return mThresholdDb;
    }
    void setThreshold(float threshold) {
        mThresholdDb = threshold;
    }
    float getPostGain() const {
        return mPostGainDb;
    }
    void setPostGain(float postGain) {
        mPostGainDb = postGain;
    }
private:
    uint32_t mLinkGroup;
    float mAttackTimeMs;
    float mReleaseTimeMs;
    float mRatio;
    float mThresholdDb;
    float mPostGainDb;
};

class DPChannel {
public:
    DPChannel();
    ~DPChannel() = default;
    void init(float inputGain, bool preEqInUse, uint32_t preEqBandCount,
            bool mbcInUse, uint32_t mbcBandCount, bool postEqInUse, uint32_t postEqBandCount,
            bool limiterInUse);

    float getInputGain() const {
        if (!mInitialized) {
            return 0;
        }
        return mInputGainDb;
    }
    void setInputGain(float gain) {
        mInputGainDb = gain;
    }

    float getOutputGain() const {
        if (!mInitialized) {
            return 0;
        }
        return mOutputGainDb;
    }
    void setOutputGain(float gain) {
        mOutputGainDb = gain;
    }

    DPEq* getPreEq();
    DPMbc* getMbc();
    DPEq* getPostEq();
    DPLimiter *getLimiter();
    void setLimiter(DPLimiter &limiter);

private:
    bool mInitialized;
    float mInputGainDb;
    float mOutputGainDb;

    DPEq mPreEq;
    DPMbc mMbc;
    DPEq mPostEq;
    DPLimiter mLimiter;

    bool mPreEqInUse;
    bool mMbcInUse;
    bool mPostEqInUse;
    bool mLimiterInUse;
};

class DPBase {
public:
    DPBase();
    virtual ~DPBase() = default;

    void init(uint32_t channelCount, bool preEqInUse, uint32_t preEqBandCount,
            bool mbcInUse, uint32_t mbcBandCount, bool postEqInUse, uint32_t postEqBandCount,
            bool limiterInUse);
    virtual size_t processSamples(const float *in, float *out, size_t samples) = 0;
    virtual void reset() = 0;

    DPChannel* getChannel(uint32_t channelIndex);
    uint32_t getChannelCount() const {
        return mChannelCount;
    }
    uint32_t getPreEqBandCount() const {
        return mPreEqBandCount;
    }
    uint32_t getMbcBandCount() const {
        return mMbcBandCount;
    }
    uint32_t getPostEqBandCount() const {
        return mPostEqBandCount;
    }
    bool isPreEQInUse() const {
        return mPreEqInUse;
    }
    bool isMbcInUse() const {
        return mMbcInUse;
    }
    bool isPostEqInUse() const {
        return mPostEqInUse;
    }
    bool isLimiterInUse() const {
        return mLimiterInUse;
    }

private:
    bool mInitialized;
    //general
    uint32_t mChannelCount;
    bool mPreEqInUse;
    uint32_t mPreEqBandCount;
    bool mMbcInUse;
    uint32_t mMbcBandCount;
    bool mPostEqInUse;
    uint32_t mPostEqBandCount;
    bool mLimiterInUse;

    std::vector<DPChannel> mChannel;
};

} //namespace dp_fx


#endif  // DPBASE_H_
