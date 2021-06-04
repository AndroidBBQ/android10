/*
 * Copyright (C) 2013 The Android Open Source Project
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

#define LOG_TAG "AudioResamplerDyn"
//#define LOG_NDEBUG 0

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <math.h>

#include <cutils/compiler.h>
#include <cutils/properties.h>
#include <utils/Debug.h>
#include <utils/Log.h>
#include <audio_utils/primitives.h>

#include "AudioResamplerFirOps.h" // USE_NEON, USE_SSE and USE_INLINE_ASSEMBLY defined here
#include "AudioResamplerFirProcess.h"
#include "AudioResamplerFirProcessNeon.h"
#include "AudioResamplerFirProcessSSE.h"
#include "AudioResamplerFirGen.h" // requires math.h
#include "AudioResamplerDyn.h"

//#define DEBUG_RESAMPLER

// use this for our buffer alignment.  Should be at least 32 bytes.
constexpr size_t CACHE_LINE_SIZE = 64;

namespace android {

/*
 * InBuffer is a type agnostic input buffer.
 *
 * Layout of the state buffer for halfNumCoefs=8.
 *
 * [rrrrrrppppppppnnnnnnnnrrrrrrrrrrrrrrrrrrr.... rrrrrrr]
 *  S            I                                R
 *
 * S = mState
 * I = mImpulse
 * R = mRingFull
 * p = past samples, convoluted with the (p)ositive side of sinc()
 * n = future samples, convoluted with the (n)egative side of sinc()
 * r = extra space for implementing the ring buffer
 */

template<typename TC, typename TI, typename TO>
AudioResamplerDyn<TC, TI, TO>::InBuffer::InBuffer()
    : mState(NULL), mImpulse(NULL), mRingFull(NULL), mStateCount(0)
{
}

template<typename TC, typename TI, typename TO>
AudioResamplerDyn<TC, TI, TO>::InBuffer::~InBuffer()
{
    init();
}

template<typename TC, typename TI, typename TO>
void AudioResamplerDyn<TC, TI, TO>::InBuffer::init()
{
    free(mState);
    mState = NULL;
    mImpulse = NULL;
    mRingFull = NULL;
    mStateCount = 0;
}

// resizes the state buffer to accommodate the appropriate filter length
template<typename TC, typename TI, typename TO>
void AudioResamplerDyn<TC, TI, TO>::InBuffer::resize(int CHANNELS, int halfNumCoefs)
{
    // calculate desired state size
    size_t stateCount = halfNumCoefs * CHANNELS * 2 * kStateSizeMultipleOfFilterLength;

    // check if buffer needs resizing
    if (mState
            && stateCount == mStateCount
            && mRingFull-mState == (ssize_t) (mStateCount-halfNumCoefs*CHANNELS)) {
        return;
    }

    // create new buffer
    TI* state = NULL;
    (void)posix_memalign(
            reinterpret_cast<void **>(&state),
            CACHE_LINE_SIZE /* alignment */,
            stateCount * sizeof(*state));
    memset(state, 0, stateCount*sizeof(*state));

    // attempt to preserve state
    if (mState) {
        TI* srcLo = mImpulse - halfNumCoefs*CHANNELS;
        TI* srcHi = mImpulse + halfNumCoefs*CHANNELS;
        TI* dst = state;

        if (srcLo < mState) {
            dst += mState-srcLo;
            srcLo = mState;
        }
        if (srcHi > mState + mStateCount) {
            srcHi = mState + mStateCount;
        }
        memcpy(dst, srcLo, (srcHi - srcLo) * sizeof(*srcLo));
        free(mState);
    }

    // set class member vars
    mState = state;
    mStateCount = stateCount;
    mImpulse = state + halfNumCoefs*CHANNELS; // actually one sample greater than needed
    mRingFull = state + mStateCount - halfNumCoefs*CHANNELS;
}

// copy in the input data into the head (impulse+halfNumCoefs) of the buffer.
template<typename TC, typename TI, typename TO>
template<int CHANNELS>
void AudioResamplerDyn<TC, TI, TO>::InBuffer::readAgain(TI*& impulse, const int halfNumCoefs,
        const TI* const in, const size_t inputIndex)
{
    TI* head = impulse + halfNumCoefs*CHANNELS;
    for (size_t i=0 ; i<CHANNELS ; i++) {
        head[i] = in[inputIndex*CHANNELS + i];
    }
}

// advance the impulse pointer, and load in data into the head (impulse+halfNumCoefs)
template<typename TC, typename TI, typename TO>
template<int CHANNELS>
void AudioResamplerDyn<TC, TI, TO>::InBuffer::readAdvance(TI*& impulse, const int halfNumCoefs,
        const TI* const in, const size_t inputIndex)
{
    impulse += CHANNELS;

    if (CC_UNLIKELY(impulse >= mRingFull)) {
        const size_t shiftDown = mRingFull - mState - halfNumCoefs*CHANNELS;
        memcpy(mState, mState+shiftDown, halfNumCoefs*CHANNELS*2*sizeof(TI));
        impulse -= shiftDown;
    }
    readAgain<CHANNELS>(impulse, halfNumCoefs, in, inputIndex);
}

template<typename TC, typename TI, typename TO>
void AudioResamplerDyn<TC, TI, TO>::InBuffer::reset()
{
    // clear resampler state
    if (mState != nullptr) {
        memset(mState, 0, mStateCount * sizeof(TI));
    }
}

template<typename TC, typename TI, typename TO>
void AudioResamplerDyn<TC, TI, TO>::Constants::set(
        int L, int halfNumCoefs, int inSampleRate, int outSampleRate)
{
    int bits = 0;
    int lscale = inSampleRate/outSampleRate < 2 ? L - 1 :
            static_cast<int>(static_cast<uint64_t>(L)*inSampleRate/outSampleRate);
    for (int i=lscale; i; ++bits, i>>=1)
        ;
    mL = L;
    mShift = kNumPhaseBits - bits;
    mHalfNumCoefs = halfNumCoefs;
}

template<typename TC, typename TI, typename TO>
AudioResamplerDyn<TC, TI, TO>::AudioResamplerDyn(
        int inChannelCount, int32_t sampleRate, src_quality quality)
    : AudioResampler(inChannelCount, sampleRate, quality),
      mResampleFunc(0), mFilterSampleRate(0), mFilterQuality(DEFAULT_QUALITY),
    mCoefBuffer(NULL)
{
    mVolumeSimd[0] = mVolumeSimd[1] = 0;
    // The AudioResampler base class assumes we are always ready for 1:1 resampling.
    // We reset mInSampleRate to 0, so setSampleRate() will calculate filters for
    // setSampleRate() for 1:1. (May be removed if precalculated filters are used.)
    mInSampleRate = 0;
    mConstants.set(128, 8, mSampleRate, mSampleRate); // TODO: set better

    // fetch property based resampling parameters
    mPropertyEnableAtSampleRate = property_get_int32(
            "ro.audio.resampler.psd.enable_at_samplerate", mPropertyEnableAtSampleRate);
    mPropertyHalfFilterLength = property_get_int32(
            "ro.audio.resampler.psd.halflength", mPropertyHalfFilterLength);
    mPropertyStopbandAttenuation = property_get_int32(
            "ro.audio.resampler.psd.stopband", mPropertyStopbandAttenuation);
    mPropertyCutoffPercent = property_get_int32(
            "ro.audio.resampler.psd.cutoff_percent", mPropertyCutoffPercent);
    mPropertyTransitionBandwidthCheat = property_get_int32(
            "ro.audio.resampler.psd.tbwcheat", mPropertyTransitionBandwidthCheat);
}

template<typename TC, typename TI, typename TO>
AudioResamplerDyn<TC, TI, TO>::~AudioResamplerDyn()
{
    free(mCoefBuffer);
}

template<typename TC, typename TI, typename TO>
void AudioResamplerDyn<TC, TI, TO>::init()
{
    mFilterSampleRate = 0; // always trigger new filter generation
    mInBuffer.init();
}

template<typename TC, typename TI, typename TO>
void AudioResamplerDyn<TC, TI, TO>::setVolume(float left, float right)
{
    AudioResampler::setVolume(left, right);
    if (is_same<TO, float>::value || is_same<TO, double>::value) {
        mVolumeSimd[0] = static_cast<TO>(left);
        mVolumeSimd[1] = static_cast<TO>(right);
    } else {  // integer requires scaling to U4_28 (rounding down)
        // integer volumes are clamped to 0 to UNITY_GAIN so there
        // are no issues with signed overflow.
        mVolumeSimd[0] = u4_28_from_float(clampFloatVol(left));
        mVolumeSimd[1] = u4_28_from_float(clampFloatVol(right));
    }
}

// TODO: update to C++11

template<typename T> T max(T a, T b) {return a > b ? a : b;}

template<typename T> T absdiff(T a, T b) {return a > b ? a - b : b - a;}

template<typename TC, typename TI, typename TO>
void AudioResamplerDyn<TC, TI, TO>::createKaiserFir(Constants &c,
        double stopBandAtten, int inSampleRate, int outSampleRate, double tbwCheat)
{
    // compute the normalized transition bandwidth
    const double tbw = firKaiserTbw(c.mHalfNumCoefs, stopBandAtten);
    const double halfbw = tbw * 0.5;

    double fcr; // compute fcr, the 3 dB amplitude cut-off.
    if (inSampleRate < outSampleRate) { // upsample
        fcr = max(0.5 * tbwCheat - halfbw, halfbw);
    } else { // downsample
        fcr = max(0.5 * tbwCheat * outSampleRate / inSampleRate - halfbw, halfbw);
    }
    createKaiserFir(c, stopBandAtten, fcr);
}

template<typename TC, typename TI, typename TO>
void AudioResamplerDyn<TC, TI, TO>::createKaiserFir(Constants &c,
        double stopBandAtten, double fcr) {
    // compute the normalized transition bandwidth
    const double tbw = firKaiserTbw(c.mHalfNumCoefs, stopBandAtten);
    const int phases = c.mL;
    const int halfLength = c.mHalfNumCoefs;

    // create buffer
    TC *coefs = nullptr;
    int ret = posix_memalign(
            reinterpret_cast<void **>(&coefs),
            CACHE_LINE_SIZE /* alignment */,
            (phases + 1) * halfLength * sizeof(TC));
    LOG_ALWAYS_FATAL_IF(ret != 0, "Cannot allocate buffer memory, ret %d", ret);
    c.mFirCoefs = coefs;
    free(mCoefBuffer);
    mCoefBuffer = coefs;

    // square the computed minimum passband value (extra safety).
    double attenuation =
            computeWindowedSincMinimumPassbandValue(stopBandAtten);
    attenuation *= attenuation;

    // design filter
    firKaiserGen(coefs, phases, halfLength, stopBandAtten, fcr, attenuation);

    // update the design criteria
    mNormalizedCutoffFrequency = fcr;
    mNormalizedTransitionBandwidth = tbw;
    mFilterAttenuation = attenuation;
    mStopbandAttenuationDb = stopBandAtten;
    mPassbandRippleDb = computeWindowedSincPassbandRippleDb(stopBandAtten);

#if 0
    // Keep this debug code in case an app causes resampler design issues.
    const double halfbw = tbw * 0.5;
    // print basic filter stats
    ALOGD("L:%d  hnc:%d  stopBandAtten:%lf  fcr:%lf  atten:%lf  tbw:%lf\n",
            c.mL, c.mHalfNumCoefs, stopBandAtten, fcr, attenuation, tbw);

    // test the filter and report results.
    // Since this is a polyphase filter, normalized fp and fs must be scaled.
    const double fp = (fcr - halfbw) / phases;
    const double fs = (fcr + halfbw) / phases;

    double passMin, passMax, passRipple;
    double stopMax, stopRipple;

    const int32_t passSteps = 1000;

    testFir(coefs, c.mL, c.mHalfNumCoefs, fp, fs, passSteps, passSteps * c.mL /*stopSteps*/,
            passMin, passMax, passRipple, stopMax, stopRipple);
    ALOGD("passband(%lf, %lf): %.8lf %.8lf %.8lf\n", 0., fp, passMin, passMax, passRipple);
    ALOGD("stopband(%lf, %lf): %.8lf %.3lf\n", fs, 0.5, stopMax, stopRipple);
#endif
}

// recursive gcd. Using objdump, it appears the tail recursion is converted to a while loop.
static int gcd(int n, int m)
{
    if (m == 0) {
        return n;
    }
    return gcd(m, n % m);
}

static bool isClose(int32_t newSampleRate, int32_t prevSampleRate,
        int32_t filterSampleRate, int32_t outSampleRate)
{

    // different upsampling ratios do not need a filter change.
    if (filterSampleRate != 0
            && filterSampleRate < outSampleRate
            && newSampleRate < outSampleRate)
        return true;

    // check design criteria again if downsampling is detected.
    int pdiff = absdiff(newSampleRate, prevSampleRate);
    int adiff = absdiff(newSampleRate, filterSampleRate);

    // allow up to 6% relative change increments.
    // allow up to 12% absolute change increments (from filter design)
    return pdiff < prevSampleRate>>4 && adiff < filterSampleRate>>3;
}

template<typename TC, typename TI, typename TO>
void AudioResamplerDyn<TC, TI, TO>::setSampleRate(int32_t inSampleRate)
{
    if (mInSampleRate == inSampleRate) {
        return;
    }
    int32_t oldSampleRate = mInSampleRate;
    uint32_t oldPhaseWrapLimit = mConstants.mL << mConstants.mShift;
    bool useS32 = false;

    mInSampleRate = inSampleRate;

    // TODO: Add precalculated Equiripple filters

    if (mFilterQuality != getQuality() ||
            !isClose(inSampleRate, oldSampleRate, mFilterSampleRate, mSampleRate)) {
        mFilterSampleRate = inSampleRate;
        mFilterQuality = getQuality();

        double stopBandAtten;
        double tbwCheat = 1.; // how much we "cheat" into aliasing
        int halfLength;
        double fcr = 0.;

        // Begin Kaiser Filter computation
        //
        // The quantization floor for S16 is about 96db - 10*log_10(#length) + 3dB.
        // Keep the stop band attenuation no greater than 84-85dB for 32 length S16 filters
        //
        // For s32 we keep the stop band attenuation at the same as 16b resolution, about
        // 96-98dB
        //

        if (mPropertyEnableAtSampleRate >= 0 && mSampleRate >= mPropertyEnableAtSampleRate) {
            // An alternative method which allows allows a greater fcr
            // at the expense of potential aliasing.
            halfLength = mPropertyHalfFilterLength;
            stopBandAtten = mPropertyStopbandAttenuation;
            useS32 = true;

            // Use either the stopband location for design (tbwCheat)
            // or use the 3dB cutoff location for design (fcr).
            // This choice is exclusive and based on whether fcr > 0.
            if (mPropertyTransitionBandwidthCheat != 0) {
                tbwCheat = mPropertyTransitionBandwidthCheat / 100.;
            } else {
                fcr = mInSampleRate <= mSampleRate
                        ? 0.5 : 0.5 * mSampleRate / mInSampleRate;
                fcr *= mPropertyCutoffPercent / 100.;
            }
        } else {
            // Voice quality devices have lower sampling rates
            // (and may be a consequence of downstream AMR-WB / G.722 codecs).
            // For these devices, we ensure a wider resampler passband
            // at the expense of aliasing noise (stopband attenuation
            // and stopband frequency).
            //
            constexpr uint32_t kVoiceDeviceSampleRate = 16000;

            if (mFilterQuality == DYN_HIGH_QUALITY) {
                // float or 32b coefficients
                useS32 = true;
                stopBandAtten = 98.;
                if (inSampleRate >= mSampleRate * 4) {
                    halfLength = 48;
                } else if (inSampleRate >= mSampleRate * 2) {
                    halfLength = 40;
                } else {
                    halfLength = 32;
                }

                if (mSampleRate <= kVoiceDeviceSampleRate) {
                    if (inSampleRate >= mSampleRate * 2) {
                        halfLength += 16;
                    } else {
                        halfLength += 8;
                    }
                    stopBandAtten = 84.;
                    tbwCheat = 1.05;
                }
            } else if (mFilterQuality == DYN_LOW_QUALITY) {
                // float or 16b coefficients
                useS32 = false;
                stopBandAtten = 80.;
                if (inSampleRate >= mSampleRate * 4) {
                    halfLength = 24;
                } else if (inSampleRate >= mSampleRate * 2) {
                    halfLength = 16;
                } else {
                    halfLength = 8;
                }
                if (mSampleRate <= kVoiceDeviceSampleRate) {
                    if (inSampleRate >= mSampleRate * 2) {
                        halfLength += 8;
                    }
                    tbwCheat = 1.05;
                } else if (inSampleRate <= mSampleRate) {
                    tbwCheat = 1.05;
                } else {
                    tbwCheat = 1.03;
                }
            } else { // DYN_MED_QUALITY
                // float or 16b coefficients
                // note: > 64 length filters with 16b coefs can have quantization noise problems
                useS32 = false;
                stopBandAtten = 84.;
                if (inSampleRate >= mSampleRate * 4) {
                    halfLength = 32;
                } else if (inSampleRate >= mSampleRate * 2) {
                    halfLength = 24;
                } else {
                    halfLength = 16;
                }

                if (mSampleRate <= kVoiceDeviceSampleRate) {
                    if (inSampleRate >= mSampleRate * 2) {
                        halfLength += 16;
                    } else {
                        halfLength += 8;
                    }
                    tbwCheat = 1.05;
                } else if (inSampleRate <= mSampleRate) {
                    tbwCheat = 1.03;
                } else {
                    tbwCheat = 1.01;
                }
            }
        }

        if (fcr > 0.) {
            ALOGV("%s: mFilterQuality:%d inSampleRate:%d mSampleRate:%d halfLength:%d "
                    "stopBandAtten:%lf fcr:%lf",
                    __func__, mFilterQuality, inSampleRate, mSampleRate, halfLength,
                    stopBandAtten, fcr);
        } else {
            ALOGV("%s: mFilterQuality:%d inSampleRate:%d mSampleRate:%d halfLength:%d "
                    "stopBandAtten:%lf tbwCheat:%lf",
                    __func__, mFilterQuality, inSampleRate, mSampleRate, halfLength,
                    stopBandAtten, tbwCheat);
        }


        // determine the number of polyphases in the filterbank.
        // for 16b, it is desirable to have 2^(16/2) = 256 phases.
        // https://ccrma.stanford.edu/~jos/resample/Relation_Interpolation_Error_Quantization.html
        //
        // We are a bit more lax on this.

        int phases = mSampleRate / gcd(mSampleRate, inSampleRate);

        // TODO: Once dynamic sample rate change is an option, the code below
        // should be modified to execute only when dynamic sample rate change is enabled.
        //
        // as above, #phases less than 63 is too few phases for accurate linear interpolation.
        // we increase the phases to compensate, but more phases means more memory per
        // filter and more time to compute the filter.
        //
        // if we know that the filter will be used for dynamic sample rate changes,
        // that would allow us skip this part for fixed sample rate resamplers.
        //
        while (phases<63) {
            phases *= 2; // this code only needed to support dynamic rate changes
        }

        if (phases>=256) {  // too many phases, always interpolate
            phases = 127;
        }

        // create the filter
        mConstants.set(phases, halfLength, inSampleRate, mSampleRate);
        if (fcr > 0.) {
            createKaiserFir(mConstants, stopBandAtten, fcr);
        } else {
            createKaiserFir(mConstants, stopBandAtten,
                    inSampleRate, mSampleRate, tbwCheat);
        }
    } // End Kaiser filter

    // update phase and state based on the new filter.
    const Constants& c(mConstants);
    mInBuffer.resize(mChannelCount, c.mHalfNumCoefs);
    const uint32_t phaseWrapLimit = c.mL << c.mShift;
    // try to preserve as much of the phase fraction as possible for on-the-fly changes
    mPhaseFraction = static_cast<unsigned long long>(mPhaseFraction)
            * phaseWrapLimit / oldPhaseWrapLimit;
    mPhaseFraction %= phaseWrapLimit; // should not do anything, but just in case.
    mPhaseIncrement = static_cast<uint32_t>(static_cast<uint64_t>(phaseWrapLimit)
            * inSampleRate / mSampleRate);

    // determine which resampler to use
    // check if locked phase (works only if mPhaseIncrement has no "fractional phase bits")
    int locked = (mPhaseIncrement << (sizeof(mPhaseIncrement)*8 - c.mShift)) == 0;
    if (locked) {
        mPhaseFraction = mPhaseFraction >> c.mShift << c.mShift; // remove fractional phase
    }

    // stride is the minimum number of filter coefficients processed per loop iteration.
    // We currently only allow a stride of 16 to match with SIMD processing.
    // This means that the filter length must be a multiple of 16,
    // or half the filter length (mHalfNumCoefs) must be a multiple of 8.
    //
    // Note: A stride of 2 is achieved with non-SIMD processing.
    int stride = ((c.mHalfNumCoefs & 7) == 0) ? 16 : 2;
    LOG_ALWAYS_FATAL_IF(stride < 16, "Resampler stride must be 16 or more");
    LOG_ALWAYS_FATAL_IF(mChannelCount < 1 || mChannelCount > 8,
            "Resampler channels(%d) must be between 1 to 8", mChannelCount);
    // stride 16 (falls back to stride 2 for machines that do not support NEON)
    if (locked) {
        switch (mChannelCount) {
        case 1:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<1, true, 16>;
            break;
        case 2:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<2, true, 16>;
            break;
        case 3:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<3, true, 16>;
            break;
        case 4:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<4, true, 16>;
            break;
        case 5:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<5, true, 16>;
            break;
        case 6:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<6, true, 16>;
            break;
        case 7:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<7, true, 16>;
            break;
        case 8:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<8, true, 16>;
            break;
        }
    } else {
        switch (mChannelCount) {
        case 1:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<1, false, 16>;
            break;
        case 2:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<2, false, 16>;
            break;
        case 3:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<3, false, 16>;
            break;
        case 4:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<4, false, 16>;
            break;
        case 5:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<5, false, 16>;
            break;
        case 6:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<6, false, 16>;
            break;
        case 7:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<7, false, 16>;
            break;
        case 8:
            mResampleFunc = &AudioResamplerDyn<TC, TI, TO>::resample<8, false, 16>;
            break;
        }
    }
#ifdef DEBUG_RESAMPLER
    printf("channels:%d  %s  stride:%d  %s  coef:%d  shift:%d\n",
            mChannelCount, locked ? "locked" : "interpolated",
            stride, useS32 ? "S32" : "S16", 2*c.mHalfNumCoefs, c.mShift);
#endif
}

template<typename TC, typename TI, typename TO>
size_t AudioResamplerDyn<TC, TI, TO>::resample(int32_t* out, size_t outFrameCount,
            AudioBufferProvider* provider)
{
    return (this->*mResampleFunc)(reinterpret_cast<TO*>(out), outFrameCount, provider);
}

template<typename TC, typename TI, typename TO>
template<int CHANNELS, bool LOCKED, int STRIDE>
size_t AudioResamplerDyn<TC, TI, TO>::resample(TO* out, size_t outFrameCount,
        AudioBufferProvider* provider)
{
    // TODO Mono -> Mono is not supported. OUTPUT_CHANNELS reflects minimum of stereo out.
    const int OUTPUT_CHANNELS = (CHANNELS < 2) ? 2 : CHANNELS;
    const Constants& c(mConstants);
    const TC* const coefs = mConstants.mFirCoefs;
    TI* impulse = mInBuffer.getImpulse();
    size_t inputIndex = 0;
    uint32_t phaseFraction = mPhaseFraction;
    const uint32_t phaseIncrement = mPhaseIncrement;
    size_t outputIndex = 0;
    size_t outputSampleCount = outFrameCount * OUTPUT_CHANNELS;
    const uint32_t phaseWrapLimit = c.mL << c.mShift;
    size_t inFrameCount = (phaseIncrement * (uint64_t)outFrameCount + phaseFraction)
            / phaseWrapLimit;
    // sanity check that inFrameCount is in signed 32 bit integer range.
    ALOG_ASSERT(0 <= inFrameCount && inFrameCount < (1U << 31));

    //ALOGV("inFrameCount:%d  outFrameCount:%d"
    //        "  phaseIncrement:%u  phaseFraction:%u  phaseWrapLimit:%u",
    //        inFrameCount, outFrameCount, phaseIncrement, phaseFraction, phaseWrapLimit);

    // NOTE: be very careful when modifying the code here. register
    // pressure is very high and a small change might cause the compiler
    // to generate far less efficient code.
    // Always sanity check the result with objdump or test-resample.

    // the following logic is a bit convoluted to keep the main processing loop
    // as tight as possible with register allocation.
    while (outputIndex < outputSampleCount) {
        //ALOGV("LOOP: inFrameCount:%d  outputIndex:%d  outFrameCount:%d"
        //        "  phaseFraction:%u  phaseWrapLimit:%u",
        //        inFrameCount, outputIndex, outFrameCount, phaseFraction, phaseWrapLimit);

        // check inputIndex overflow
        ALOG_ASSERT(inputIndex <= mBuffer.frameCount, "inputIndex%zu > frameCount%zu",
                inputIndex, mBuffer.frameCount);
        // Buffer is empty, fetch a new one if necessary (inFrameCount > 0).
        // We may not fetch a new buffer if the existing data is sufficient.
        while (mBuffer.frameCount == 0 && inFrameCount > 0) {
            mBuffer.frameCount = inFrameCount;
            provider->getNextBuffer(&mBuffer);
            if (mBuffer.raw == NULL) {
                // We are either at the end of playback or in an underrun situation.
                // Reset buffer to prevent pop noise at the next buffer.
                mInBuffer.reset();
                goto resample_exit;
            }
            inFrameCount -= mBuffer.frameCount;
            if (phaseFraction >= phaseWrapLimit) { // read in data
                mInBuffer.template readAdvance<CHANNELS>(
                        impulse, c.mHalfNumCoefs,
                        reinterpret_cast<TI*>(mBuffer.raw), inputIndex);
                inputIndex++;
                phaseFraction -= phaseWrapLimit;
                while (phaseFraction >= phaseWrapLimit) {
                    if (inputIndex >= mBuffer.frameCount) {
                        inputIndex = 0;
                        provider->releaseBuffer(&mBuffer);
                        break;
                    }
                    mInBuffer.template readAdvance<CHANNELS>(
                            impulse, c.mHalfNumCoefs,
                            reinterpret_cast<TI*>(mBuffer.raw), inputIndex);
                    inputIndex++;
                    phaseFraction -= phaseWrapLimit;
                }
            }
        }
        const TI* const in = reinterpret_cast<const TI*>(mBuffer.raw);
        const size_t frameCount = mBuffer.frameCount;
        const int coefShift = c.mShift;
        const int halfNumCoefs = c.mHalfNumCoefs;
        const TO* const volumeSimd = mVolumeSimd;

        // main processing loop
        while (CC_LIKELY(outputIndex < outputSampleCount)) {
            // caution: fir() is inlined and may be large.
            // output will be loaded with the appropriate values
            //
            // from the input samples in impulse[-halfNumCoefs+1]... impulse[halfNumCoefs]
            // from the polyphase filter of (phaseFraction / phaseWrapLimit) in coefs.
            //
            //ALOGV("LOOP2: inFrameCount:%d  outputIndex:%d  outFrameCount:%d"
            //        "  phaseFraction:%u  phaseWrapLimit:%u",
            //        inFrameCount, outputIndex, outFrameCount, phaseFraction, phaseWrapLimit);
            ALOG_ASSERT(phaseFraction < phaseWrapLimit);
            fir<CHANNELS, LOCKED, STRIDE>(
                    &out[outputIndex],
                    phaseFraction, phaseWrapLimit,
                    coefShift, halfNumCoefs, coefs,
                    impulse, volumeSimd);

            outputIndex += OUTPUT_CHANNELS;

            phaseFraction += phaseIncrement;
            while (phaseFraction >= phaseWrapLimit) {
                if (inputIndex >= frameCount) {
                    goto done;  // need a new buffer
                }
                mInBuffer.template readAdvance<CHANNELS>(impulse, halfNumCoefs, in, inputIndex);
                inputIndex++;
                phaseFraction -= phaseWrapLimit;
            }
        }
done:
        // We arrive here when we're finished or when the input buffer runs out.
        // Regardless we need to release the input buffer if we've acquired it.
        if (inputIndex > 0) {  // we've acquired a buffer (alternatively could check frameCount)
            ALOG_ASSERT(inputIndex == frameCount, "inputIndex(%zu) != frameCount(%zu)",
                    inputIndex, frameCount);  // must have been fully read.
            inputIndex = 0;
            provider->releaseBuffer(&mBuffer);
            ALOG_ASSERT(mBuffer.frameCount == 0);
        }
    }

resample_exit:
    // inputIndex must be zero in all three cases:
    // (1) the buffer never was been acquired; (2) the buffer was
    // released at "done:"; or (3) getNextBuffer() failed.
    ALOG_ASSERT(inputIndex == 0, "Releasing: inputindex:%zu frameCount:%zu  phaseFraction:%u",
            inputIndex, mBuffer.frameCount, phaseFraction);
    ALOG_ASSERT(mBuffer.frameCount == 0); // there must be no frames in the buffer
    mInBuffer.setImpulse(impulse);
    mPhaseFraction = phaseFraction;
    return outputIndex / OUTPUT_CHANNELS;
}

/* instantiate templates used by AudioResampler::create */
template class AudioResamplerDyn<float, float, float>;
template class AudioResamplerDyn<int16_t, int16_t, int32_t>;
template class AudioResamplerDyn<int32_t, int16_t, int32_t>;

// ----------------------------------------------------------------------------
} // namespace android
