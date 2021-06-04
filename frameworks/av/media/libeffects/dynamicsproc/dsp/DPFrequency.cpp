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

#define LOG_TAG "DPFrequency"
//#define LOG_NDEBUG 0

#include <log/log.h>
#include "DPFrequency.h"
#include <algorithm>
#include <sys/param.h>

namespace dp_fx {

using Eigen::MatrixXd;
#define MAX_BLOCKSIZE 16384 //For this implementation
#define MIN_BLOCKSIZE 8

#define CIRCULAR_BUFFER_UPSAMPLE 4  //4 times buffer size

static constexpr float MIN_ENVELOPE = 1e-6f; //-120 dB
static constexpr float EPSILON = 0.0000001f;

static inline bool isZero(float f) {
    return fabs(f) <= EPSILON;
}

template <class T>
bool compareEquality(T a, T b) {
    return (a == b);
}

template <> bool compareEquality<float>(float a, float b) {
    return isZero(a - b);
}

//TODO: avoid using macro for estimating change and assignment.
#define IS_CHANGED(c, a, b) { c |= !compareEquality(a,b); \
    (a) = (b); }

//ChannelBuffers helper
void ChannelBuffer::initBuffers(unsigned int blockSize, unsigned int overlapSize,
        unsigned int halfFftSize, unsigned int samplingRate, DPBase &dpBase) {
    ALOGV("ChannelBuffer::initBuffers blockSize %d, overlap %d, halfFft %d",
            blockSize, overlapSize, halfFftSize);

    mSamplingRate = samplingRate;
    mBlockSize = blockSize;

    cBInput.resize(mBlockSize * CIRCULAR_BUFFER_UPSAMPLE);
    cBOutput.resize(mBlockSize * CIRCULAR_BUFFER_UPSAMPLE);

    //temp vectors
    input.resize(mBlockSize);
    output.resize(mBlockSize);
    outTail.resize(overlapSize);

    //module vectors
    mPreEqFactorVector.resize(halfFftSize, 1.0);
    mPostEqFactorVector.resize(halfFftSize, 1.0);

    mPreEqBands.resize(dpBase.getPreEqBandCount());
    mMbcBands.resize(dpBase.getMbcBandCount());
    mPostEqBands.resize(dpBase.getPostEqBandCount());
    ALOGV("mPreEqBands %zu, mMbcBands %zu, mPostEqBands %zu",mPreEqBands.size(),
            mMbcBands.size(), mPostEqBands.size());

    DPChannel *pChannel = dpBase.getChannel(0);
    if (pChannel != nullptr) {
        mPreEqInUse = pChannel->getPreEq()->isInUse();
        mMbcInUse = pChannel->getMbc()->isInUse();
        mPostEqInUse = pChannel->getPostEq()->isInUse();
        mLimiterInUse = pChannel->getLimiter()->isInUse();
    }

    mLimiterParams.linkGroup = -1; //no group.
}

void ChannelBuffer::computeBinStartStop(BandParams &bp, size_t binStart) {

    bp.binStart = binStart;
    bp.binStop = (int)(0.5 + bp.freqCutoffHz * mBlockSize / mSamplingRate);
}

//== LinkedLimiters Helper
void LinkedLimiters::reset() {
    mGroupsMap.clear();
}

void LinkedLimiters::update(int32_t group, int index) {
    mGroupsMap[group].push_back(index);
}

void LinkedLimiters::remove(int index) {
    //check all groups and if index is found, remove it.
    //if group is empty afterwards, remove it.
    for (auto it = mGroupsMap.begin(); it != mGroupsMap.end(); ) {
        for (auto itIndex = it->second.begin(); itIndex != it->second.end(); ) {
            if (*itIndex == index) {
                itIndex = it->second.erase(itIndex);
            } else {
                ++itIndex;
            }
        }
        if (it->second.size() == 0) {
            it = mGroupsMap.erase(it);
        } else {
            ++it;
        }
    }
}

//== DPFrequency
void DPFrequency::reset() {
}

size_t DPFrequency::getMinBockSize() {
    return MIN_BLOCKSIZE;
}

size_t DPFrequency::getMaxBockSize() {
    return MAX_BLOCKSIZE;
}

void DPFrequency::configure(size_t blockSize, size_t overlapSize,
        size_t samplingRate) {
    ALOGV("configure");
    mBlockSize = blockSize;
    if (mBlockSize > MAX_BLOCKSIZE) {
        mBlockSize = MAX_BLOCKSIZE;
    } else if (mBlockSize < MIN_BLOCKSIZE) {
        mBlockSize = MIN_BLOCKSIZE;
    } else {
        if (!powerof2(blockSize)) {
            //find next highest power of 2.
            mBlockSize = 1 << (32 - __builtin_clz(blockSize));
        }
    }

    mHalfFFTSize = 1 + mBlockSize / 2; //including Nyquist bin
    mOverlapSize = std::min(overlapSize, mBlockSize/2);

    int channelcount = getChannelCount();
    mSamplingRate = samplingRate;
    mChannelBuffers.resize(channelcount);
    for (int ch = 0; ch < channelcount; ch++) {
        mChannelBuffers[ch].initBuffers(mBlockSize, mOverlapSize, mHalfFFTSize,
                mSamplingRate, *this);
    }

    //effective number of frames processed per second
    mBlocksPerSecond = (float)mSamplingRate / (mBlockSize - mOverlapSize);

    fill_window(mVWindow, RDSP_WINDOW_HANNING_FLAT_TOP, mBlockSize, mOverlapSize);

    //split window into analysis and synthesis. Both are the sqrt() of original
    //window
    Eigen::Map<Eigen::VectorXf> eWindow(&mVWindow[0], mVWindow.size());
    eWindow = eWindow.array().sqrt();

    //compute window rms for energy compensation
    mWindowRms = 0;
    for (size_t i = 0; i < mVWindow.size(); i++) {
        mWindowRms += mVWindow[i] * mVWindow[i];
    }

    //Making sure window rms is not zero.
    mWindowRms = std::max(sqrt(mWindowRms / mVWindow.size()), MIN_ENVELOPE);
}

void DPFrequency::updateParameters(ChannelBuffer &cb, int channelIndex) {
    DPChannel *pChannel = getChannel(channelIndex);

    if (pChannel == nullptr) {
        ALOGE("Error: updateParameters null DPChannel %d", channelIndex);
        return;
    }

    //===Input Gain and preEq
    {
        bool changed = false;
        IS_CHANGED(changed, cb.inputGainDb, pChannel->getInputGain());
        //===EqPre
        if (cb.mPreEqInUse) {
            DPEq *pPreEq = pChannel->getPreEq();
            if (pPreEq == nullptr) {
                ALOGE("Error: updateParameters null PreEq for channel: %d", channelIndex);
                return;
            }
            IS_CHANGED(changed, cb.mPreEqEnabled, pPreEq->isEnabled());
            if (cb.mPreEqEnabled) {
                for (unsigned int b = 0; b < getPreEqBandCount(); b++) {
                    DPEqBand *pEqBand = pPreEq->getBand(b);
                    if (pEqBand == nullptr) {
                        ALOGE("Error: updateParameters null PreEqBand for band %d", b);
                        return; //failed.
                    }
                    ChannelBuffer::EqBandParams *pEqBandParams = &cb.mPreEqBands[b];
                    IS_CHANGED(changed, pEqBandParams->enabled, pEqBand->isEnabled());
                    IS_CHANGED(changed, pEqBandParams->freqCutoffHz,
                            pEqBand->getCutoffFrequency());
                    IS_CHANGED(changed, pEqBandParams->gainDb, pEqBand->getGain());
                }
            }
        }

        if (changed) {
            float inputGainFactor = dBtoLinear(cb.inputGainDb);
            if (cb.mPreEqInUse && cb.mPreEqEnabled) {
                ALOGV("preEq changed, recomputing! channel %d", channelIndex);
                size_t binNext = 0;
                for (unsigned int b = 0; b < getPreEqBandCount(); b++) {
                    ChannelBuffer::EqBandParams *pEqBandParams = &cb.mPreEqBands[b];

                    //frequency translation
                    cb.computeBinStartStop(*pEqBandParams, binNext);
                    binNext = pEqBandParams->binStop + 1;
                    float factor = dBtoLinear(pEqBandParams->gainDb);
                    if (!pEqBandParams->enabled) {
                        factor = inputGainFactor;
                    }
                    for (size_t k = pEqBandParams->binStart;
                            k <= pEqBandParams->binStop && k < mHalfFFTSize; k++) {
                        cb.mPreEqFactorVector[k] = factor * inputGainFactor;
                    }
                }
            } else {
                ALOGV("only input gain changed, recomputing!");
                //populate PreEq factor with input gain factor.
                for (size_t k = 0; k < mHalfFFTSize; k++) {
                    cb.mPreEqFactorVector[k] = inputGainFactor;
                }
            }
        }
    } //inputGain and preEq

    //===EqPost
    if (cb.mPostEqInUse) {
        bool changed = false;

        DPEq *pPostEq = pChannel->getPostEq();
        if (pPostEq == nullptr) {
            ALOGE("Error: updateParameters null postEq for channel: %d", channelIndex);
            return; //failed.
        }
        IS_CHANGED(changed, cb.mPostEqEnabled, pPostEq->isEnabled());
        if (cb.mPostEqEnabled) {
            for (unsigned int b = 0; b < getPostEqBandCount(); b++) {
                DPEqBand *pEqBand = pPostEq->getBand(b);
                if (pEqBand == nullptr) {
                    ALOGE("Error: updateParameters PostEqBand NULL for band %d", b);
                    return; //failed.
                }
                ChannelBuffer::EqBandParams *pEqBandParams = &cb.mPostEqBands[b];
                IS_CHANGED(changed, pEqBandParams->enabled, pEqBand->isEnabled());
                IS_CHANGED(changed, pEqBandParams->freqCutoffHz,
                        pEqBand->getCutoffFrequency());
                IS_CHANGED(changed, pEqBandParams->gainDb, pEqBand->getGain());
            }
            if (changed) {
                ALOGV("postEq changed, recomputing! channel %d", channelIndex);
                size_t binNext = 0;
                for (unsigned int b = 0; b < getPostEqBandCount(); b++) {
                    ChannelBuffer::EqBandParams *pEqBandParams = &cb.mPostEqBands[b];

                    //frequency translation
                    cb.computeBinStartStop(*pEqBandParams, binNext);
                    binNext = pEqBandParams->binStop + 1;
                    float factor = dBtoLinear(pEqBandParams->gainDb);
                    if (!pEqBandParams->enabled) {
                        factor = 1.0;
                    }
                    for (size_t k = pEqBandParams->binStart;
                            k <= pEqBandParams->binStop && k < mHalfFFTSize; k++) {
                        cb.mPostEqFactorVector[k] = factor;
                    }
                }
            }
        } //enabled
    }

    //===MBC
    if (cb.mMbcInUse) {
        DPMbc *pMbc = pChannel->getMbc();
        if (pMbc == nullptr) {
            ALOGE("Error: updateParameters Mbc NULL for channel: %d", channelIndex);
            return;
        }
        cb.mMbcEnabled = pMbc->isEnabled();
        if (cb.mMbcEnabled) {
            bool changed = false;
            for (unsigned int b = 0; b < getMbcBandCount(); b++) {
                DPMbcBand *pMbcBand = pMbc->getBand(b);
                if (pMbcBand == nullptr) {
                    ALOGE("Error: updateParameters MbcBand NULL for band %d", b);
                    return; //failed.
                }
                ChannelBuffer::MbcBandParams *pMbcBandParams = &cb.mMbcBands[b];
                pMbcBandParams->enabled = pMbcBand->isEnabled();
                IS_CHANGED(changed, pMbcBandParams->freqCutoffHz,
                        pMbcBand->getCutoffFrequency());

                pMbcBandParams->gainPreDb = pMbcBand->getPreGain();
                pMbcBandParams->gainPostDb = pMbcBand->getPostGain();
                pMbcBandParams->attackTimeMs = pMbcBand->getAttackTime();
                pMbcBandParams->releaseTimeMs = pMbcBand->getReleaseTime();
                pMbcBandParams->ratio = pMbcBand->getRatio();
                pMbcBandParams->thresholdDb = pMbcBand->getThreshold();
                pMbcBandParams->kneeWidthDb = pMbcBand->getKneeWidth();
                pMbcBandParams->noiseGateThresholdDb = pMbcBand->getNoiseGateThreshold();
                pMbcBandParams->expanderRatio = pMbcBand->getExpanderRatio();

            }

            if (changed) {
                ALOGV("mbc changed, recomputing! channel %d", channelIndex);
                size_t binNext= 0;
                for (unsigned int b = 0; b < getMbcBandCount(); b++) {
                    ChannelBuffer::MbcBandParams *pMbcBandParams = &cb.mMbcBands[b];

                    pMbcBandParams->previousEnvelope = 0;

                    //frequency translation
                    cb.computeBinStartStop(*pMbcBandParams, binNext);
                    binNext = pMbcBandParams->binStop + 1;
                }
            }
        }
    }

    //===Limiter
    if (cb.mLimiterInUse) {
        bool changed = false;
        DPLimiter *pLimiter = pChannel->getLimiter();
        if (pLimiter == nullptr) {
            ALOGE("Error: updateParameters Limiter NULL for channel: %d", channelIndex);
            return;
        }
        cb.mLimiterEnabled = pLimiter->isEnabled();
        if (cb.mLimiterEnabled) {
            IS_CHANGED(changed, cb.mLimiterParams.linkGroup ,
                    (int32_t)pLimiter->getLinkGroup());
            cb.mLimiterParams.attackTimeMs = pLimiter->getAttackTime();
            cb.mLimiterParams.releaseTimeMs = pLimiter->getReleaseTime();
            cb.mLimiterParams.ratio = pLimiter->getRatio();
            cb.mLimiterParams.thresholdDb = pLimiter->getThreshold();
            cb.mLimiterParams.postGainDb = pLimiter->getPostGain();
        }

        if (changed) {
            ALOGV("limiter changed, recomputing linkGroups for %d", channelIndex);
            mLinkedLimiters.remove(channelIndex); //in case it was already there.
            mLinkedLimiters.update(cb.mLimiterParams.linkGroup, channelIndex);
        }
    }

    //=== Output Gain
    cb.outputGainDb = pChannel->getOutputGain();
}

size_t DPFrequency::processSamples(const float *in, float *out, size_t samples) {
       const float *pIn = in;
       float *pOut = out;

       int channelCount = mChannelBuffers.size();
       if (channelCount < 1) {
           ALOGW("warning: no Channels ready for processing");
           return 0;
       }

       //**Check if parameters have changed and update
       for (int ch = 0; ch < channelCount; ch++) {
           updateParameters(mChannelBuffers[ch], ch);
       }

       //**separate into channels
       for (size_t k = 0; k < samples; k += channelCount) {
           for (int ch = 0; ch < channelCount; ch++) {
               mChannelBuffers[ch].cBInput.write(*pIn++);
           }
       }

       //**process all channelBuffers
       processChannelBuffers(mChannelBuffers);

       //** estimate how much data is available in ALL channels
       size_t available = mChannelBuffers[0].cBOutput.availableToRead();
       for (int ch = 1; ch < channelCount; ch++) {
           available = std::min(available, mChannelBuffers[ch].cBOutput.availableToRead());
       }

       //** make sure to output just what the buffer can handle
       if (available > samples/channelCount) {
           available = samples/channelCount;
       }

       //**Prepend zeroes if necessary
       size_t fill = samples - (channelCount * available);
       for (size_t k = 0; k < fill; k++) {
           *pOut++ = 0;
       }

       //**interleave channels
       for (size_t k = 0; k < available; k++) {
           for (int ch = 0; ch < channelCount; ch++) {
               *pOut++ = mChannelBuffers[ch].cBOutput.read();
           }
       }

       return samples;
}

size_t DPFrequency::processChannelBuffers(CBufferVector &channelBuffers) {
    const int channelCount = channelBuffers.size();
    size_t processedSamples = 0;
    size_t processFrames = mBlockSize - mOverlapSize;

    size_t available = channelBuffers[0].cBInput.availableToRead();
    for (int ch = 1; ch < channelCount; ch++) {
        available = std::min(available, channelBuffers[ch].cBInput.availableToRead());
    }

    while (available >= processFrames) {
        //First pass
        for (int ch = 0; ch < channelCount; ch++) {
            ChannelBuffer * pCb = &channelBuffers[ch];
            //move tail of previous
            std::copy(pCb->input.begin() + processFrames,
                    pCb->input.end(),
                    pCb->input.begin());

            //read new available data
            for (unsigned int k = 0; k < processFrames; k++) {
                pCb->input[mOverlapSize + k] = pCb->cBInput.read();
            }
            //first stages: fft, preEq, mbc, postEq and start of Limiter
            processedSamples += processFirstStages(*pCb);
        }

        //**compute linked limiters and update levels if needed
        processLinkedLimiters(channelBuffers);

        //final pass.
        for (int ch = 0; ch < channelCount; ch++) {
            ChannelBuffer * pCb = &channelBuffers[ch];

            //linked limiter and ifft
            processLastStages(*pCb);

            //mix tail (and capture new tail
            for (unsigned int k = 0; k < mOverlapSize; k++) {
                pCb->output[k] += pCb->outTail[k];
                pCb->outTail[k] = pCb->output[processFrames + k]; //new tail
            }

            //output data
            for (unsigned int k = 0; k < processFrames; k++) {
                pCb->cBOutput.write(pCb->output[k]);
            }
        }
        available -= processFrames;
    }
    return processedSamples;
}
size_t DPFrequency::processFirstStages(ChannelBuffer &cb) {

    //##apply window
    Eigen::Map<Eigen::VectorXf> eWindow(&mVWindow[0], mVWindow.size());
    Eigen::Map<Eigen::VectorXf> eInput(&cb.input[0], cb.input.size());

    Eigen::VectorXf eWin = eInput.cwiseProduct(eWindow); //apply window

    //##fft
    //Note: we are using eigen with the default scaling, which ensures that
    //  IFFT( FFT(x) ) = x.
    // TODO: optimize by using the noscale option, and compensate with dB scale offsets
    mFftServer.fwd(cb.complexTemp, eWin);

    size_t cSize = cb.complexTemp.size();
    size_t maxBin = std::min(cSize/2, mHalfFFTSize);

    //== EqPre (always runs)
    for (size_t k = 0; k < maxBin; k++) {
        cb.complexTemp[k] *= cb.mPreEqFactorVector[k];
    }

    //== MBC
    if (cb.mMbcInUse && cb.mMbcEnabled) {
        for (size_t band = 0; band < cb.mMbcBands.size(); band++) {
            ChannelBuffer::MbcBandParams *pMbcBandParams = &cb.mMbcBands[band];
            float fEnergySum = 0;

            //apply pre gain.
            float preGainFactor = dBtoLinear(pMbcBandParams->gainPreDb);
            float preGainSquared = preGainFactor * preGainFactor;

            for (size_t k = pMbcBandParams->binStart; k <= pMbcBandParams->binStop; k++) {
                fEnergySum += std::norm(cb.complexTemp[k]) * preGainSquared; //mag squared
            }

            //Eigen FFT is full spectrum, even if the source was real data.
            // Each half spectrum has half the energy. This is taken into account with the * 2
            // factor in the energy computations.
            // energy = sqrt(sum_components_squared) number_points
            // in here, the fEnergySum is duplicated to account for the second half spectrum,
            // and the windowRms is used to normalize by the expected energy reduction
            // caused by the window used (expected for steady state signals)
            fEnergySum = sqrt(fEnergySum * 2) / (mBlockSize * mWindowRms);

            // updates computed per frame advance.
            float fTheta = 0.0;
            float fFAttSec = pMbcBandParams->attackTimeMs / 1000; //in seconds
            float fFRelSec = pMbcBandParams->releaseTimeMs / 1000; //in seconds

            if (fEnergySum > pMbcBandParams->previousEnvelope) {
                fTheta = exp(-1.0 / (fFAttSec * mBlocksPerSecond));
            } else {
                fTheta = exp(-1.0 / (fFRelSec * mBlocksPerSecond));
            }

            float fEnv = (1.0 - fTheta) * fEnergySum + fTheta * pMbcBandParams->previousEnvelope;
            //preserve for next iteration
            pMbcBandParams->previousEnvelope = fEnv;

            if (fEnv < MIN_ENVELOPE) {
                fEnv = MIN_ENVELOPE;
            }
            const float envDb = linearToDb(fEnv);
            float newLevelDb = envDb;
            //using shorter variables for code clarity
            const float thresholdDb = pMbcBandParams->thresholdDb;
            const float ratio = pMbcBandParams->ratio;
            const float kneeWidthDbHalf = pMbcBandParams->kneeWidthDb / 2;
            const float noiseGateThresholdDb = pMbcBandParams->noiseGateThresholdDb;
            const float expanderRatio = pMbcBandParams->expanderRatio;

            //find segment
            if (envDb > thresholdDb + kneeWidthDbHalf) {
                //compression segment
                newLevelDb = envDb + ((1 / ratio) - 1) * (envDb - thresholdDb);
            } else if (envDb > thresholdDb - kneeWidthDbHalf) {
                //knee-compression segment
                float temp = (envDb - thresholdDb + kneeWidthDbHalf);
                newLevelDb = envDb + ((1 / ratio) - 1) *
                        temp * temp / (kneeWidthDbHalf * 4);
            } else if (envDb < noiseGateThresholdDb) {
                //expander segment
                newLevelDb = noiseGateThresholdDb -
                        expanderRatio * (noiseGateThresholdDb - envDb);
            }

            float newFactor = dBtoLinear(newLevelDb - envDb);

            //apply post gain.
            newFactor *= dBtoLinear(pMbcBandParams->gainPostDb);

            //apply to this band
            for (size_t k = pMbcBandParams->binStart; k <= pMbcBandParams->binStop; k++) {
                cb.complexTemp[k] *= newFactor;
            }

        } //end per band process

    } //end MBC

    //== EqPost
    if (cb.mPostEqInUse && cb.mPostEqEnabled) {
        for (size_t k = 0; k < maxBin; k++) {
            cb.complexTemp[k] *= cb.mPostEqFactorVector[k];
        }
    }

    //== Limiter. First Pass
    if (cb.mLimiterInUse && cb.mLimiterEnabled) {
        float fEnergySum = 0;
        for (size_t k = 0; k < maxBin; k++) {
            fEnergySum += std::norm(cb.complexTemp[k]);
        }

        //see explanation above for energy computation logic
        fEnergySum = sqrt(fEnergySum * 2) / (mBlockSize * mWindowRms);
        float fTheta = 0.0;
        float fFAttSec = cb.mLimiterParams.attackTimeMs / 1000; //in seconds
        float fFRelSec = cb.mLimiterParams.releaseTimeMs / 1000; //in seconds

        if (fEnergySum > cb.mLimiterParams.previousEnvelope) {
            fTheta = exp(-1.0 / (fFAttSec * mBlocksPerSecond));
        } else {
            fTheta = exp(-1.0 / (fFRelSec * mBlocksPerSecond));
        }

        float fEnv = (1.0 - fTheta) * fEnergySum + fTheta * cb.mLimiterParams.previousEnvelope;
        //preserve for next iteration
        cb.mLimiterParams.previousEnvelope = fEnv;

        const float envDb = linearToDb(fEnv);
        float newFactorDb = 0;
        //using shorter variables for code clarity
        const float thresholdDb = cb.mLimiterParams.thresholdDb;
        const float ratio = cb.mLimiterParams.ratio;

        if (envDb > thresholdDb) {
            //limiter segment
            newFactorDb = ((1 / ratio) - 1) * (envDb - thresholdDb);
        }

        float newFactor = dBtoLinear(newFactorDb);

        cb.mLimiterParams.newFactor = newFactor;

    } //end Limiter
    return mBlockSize;
}

void DPFrequency::processLinkedLimiters(CBufferVector &channelBuffers) {

    const int channelCount = channelBuffers.size();
    for (auto &groupPair : mLinkedLimiters.mGroupsMap) {
        float minFactor = 1.0;
        //estimate minfactor for all linked
        for(int index : groupPair.second) {
            if (index >= 0 && index < channelCount) {
                minFactor = std::min(channelBuffers[index].mLimiterParams.newFactor, minFactor);
            }
        }
        //apply minFactor
        for(int index : groupPair.second) {
            if (index >= 0 && index < channelCount) {
                channelBuffers[index].mLimiterParams.linkFactor = minFactor;
            }
        }
    }
}

size_t DPFrequency::processLastStages(ChannelBuffer &cb) {

    float outputGainFactor = dBtoLinear(cb.outputGainDb);
    //== Limiter. last Pass
    if (cb.mLimiterInUse && cb.mLimiterEnabled) {
        //compute factor, with post-gain
        float factor = cb.mLimiterParams.linkFactor * dBtoLinear(cb.mLimiterParams.postGainDb);
        outputGainFactor *= factor;
    }

    //apply to all if != 1.0
    if (!compareEquality(outputGainFactor, 1.0f)) {
        size_t cSize = cb.complexTemp.size();
        size_t maxBin = std::min(cSize/2, mHalfFFTSize);
        for (size_t k = 0; k < maxBin; k++) {
            cb.complexTemp[k] *= outputGainFactor;
        }
    }

    //##ifft directly to output.
    Eigen::Map<Eigen::VectorXf> eOutput(&cb.output[0], cb.output.size());
    mFftServer.inv(eOutput, cb.complexTemp);

    //apply rest of window for resynthesis
    Eigen::Map<Eigen::VectorXf> eWindow(&mVWindow[0], mVWindow.size());
    eOutput = eOutput.cwiseProduct(eWindow);

    return mBlockSize;
}

} //namespace dp_fx
