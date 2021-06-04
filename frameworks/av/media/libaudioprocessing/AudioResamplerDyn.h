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

#ifndef ANDROID_AUDIO_RESAMPLER_DYN_H
#define ANDROID_AUDIO_RESAMPLER_DYN_H

#include <stdint.h>
#include <sys/types.h>
#include <android/log.h>

#include <media/AudioResampler.h>

namespace android {

/* AudioResamplerDyn
 *
 * This class template is used for floating point and integer resamplers.
 *
 * Type variables:
 * TC = filter coefficient type (one of int16_t, int32_t, or float)
 * TI = input data type (one of int16_t or float)
 * TO = output data type (one of int32_t or float)
 *
 * For integer input data types TI, the coefficient type TC is either int16_t or int32_t.
 * For float input data types TI, the coefficient type TC is float.
 */

template<typename TC, typename TI, typename TO>
class AudioResamplerDyn: public AudioResampler {
public:
    AudioResamplerDyn(int inChannelCount,
            int32_t sampleRate, src_quality quality);

    virtual ~AudioResamplerDyn();

    virtual void init();

    virtual void setSampleRate(int32_t inSampleRate);

    virtual void setVolume(float left, float right);

    virtual size_t resample(int32_t* out, size_t outFrameCount,
            AudioBufferProvider* provider);

    void reset() override {
        AudioResampler::reset();
        mInBuffer.reset();
    }

    // Make available key design criteria for testing
    int getHalfLength() const {
        return mConstants.mHalfNumCoefs;
    }

    const TC *getFilterCoefs() const {
        return mConstants.mFirCoefs;
    }

    int getPhases() const {
        return mConstants.mL;
    }

    double getStopbandAttenuationDb() const {
        return mStopbandAttenuationDb;
    }

    double getPassbandRippleDb() const {
        return mPassbandRippleDb;
    }

    double getNormalizedTransitionBandwidth() const {
        return mNormalizedTransitionBandwidth;
    }

    double getFilterAttenuation() const {
        return mFilterAttenuation;
    }

    double getNormalizedCutoffFrequency() const {
        return mNormalizedCutoffFrequency;
    }

private:

    class Constants { // stores the filter constants.
    public:
        Constants() :
            mL(0), mShift(0), mHalfNumCoefs(0), mFirCoefs(NULL)
        {}
        void set(int L, int halfNumCoefs,
                int inSampleRate, int outSampleRate);

                 int mL;            // interpolation phases in the filter.
                 int mShift;        // right shift to get polyphase index
        unsigned int mHalfNumCoefs; // filter half #coefs
           const TC* mFirCoefs;     // polyphase filter bank
    };

    class InBuffer { // buffer management for input type TI
    public:
        InBuffer();
        ~InBuffer();
        void init();

        void resize(int CHANNELS, int halfNumCoefs);

        // used for direct management of the mImpulse pointer
        inline TI* getImpulse() {
            return mImpulse;
        }

        inline void setImpulse(TI *impulse) {
            mImpulse = impulse;
        }

        template<int CHANNELS>
        inline void readAgain(TI*& impulse, const int halfNumCoefs,
                const TI* const in, const size_t inputIndex);

        template<int CHANNELS>
        inline void readAdvance(TI*& impulse, const int halfNumCoefs,
                const TI* const in, const size_t inputIndex);

        void reset();

    private:
        // tuning parameter guidelines: 2 <= multiple <= 8
        static const int kStateSizeMultipleOfFilterLength = 4;

        // in general, mRingFull = mState + mStateSize - halfNumCoefs*CHANNELS.
           TI* mState;      // base pointer for the input buffer storage
           TI* mImpulse;    // current location of the impulse response (centered)
           TI* mRingFull;   // mState <= mImpulse < mRingFull
        size_t mStateCount; // size of state in units of TI.
    };

    void createKaiserFir(Constants &c, double stopBandAtten,
            int inSampleRate, int outSampleRate, double tbwCheat);

    void createKaiserFir(Constants &c, double stopBandAtten, double fcr);

    template<int CHANNELS, bool LOCKED, int STRIDE>
    size_t resample(TO* out, size_t outFrameCount, AudioBufferProvider* provider);

    // define a pointer to member function type for resample
    typedef size_t (AudioResamplerDyn<TC, TI, TO>::*resample_ABP_t)(TO* out,
            size_t outFrameCount, AudioBufferProvider* provider);

    // data - the contiguous storage and layout of these is important.
           InBuffer mInBuffer;
          Constants mConstants;        // current set of coefficient parameters
    TO __attribute__ ((aligned (8))) mVolumeSimd[2]; // must be aligned or NEON may crash
     resample_ABP_t mResampleFunc;     // called function for resampling
            int32_t mFilterSampleRate; // designed filter sample rate.
        src_quality mFilterQuality;    // designed filter quality.
              void* mCoefBuffer;       // if a filter is created, this is not null

    // Property selected design parameters.
              // This will enable fixed high quality resampling.

              // 32 char PROP_NAME_MAX limit enforced before Android O

              // Use for sample rates greater than or equal to this value.
              // Set to non-negative to enable, negative to disable.
              int32_t mPropertyEnableAtSampleRate = 48000;
                      // "ro.audio.resampler.psd.enable_at_samplerate"

              // Specify HALF the resampling filter length.
              // Set to a value which is a multiple of 4.
              int32_t mPropertyHalfFilterLength = 32;
                      // "ro.audio.resampler.psd.halflength"

              // Specify the stopband attenuation in positive dB.
              // Set to a value greater or equal to 20.
              int32_t mPropertyStopbandAttenuation = 90;
                      // "ro.audio.resampler.psd.stopband"

              // Specify the cutoff frequency as a percentage of Nyquist.
              // Set to a value between 50 and 100.
              int32_t mPropertyCutoffPercent = 100;
                      // "ro.audio.resampler.psd.cutoff_percent"

              // Specify the transition bandwidth extension beyond Nyquist.
              // If this is nonzero then mPropertyCutoffPercent is ignored.
              // A value of 100 or greater is typically used, where 100 means the
              // stopband is at Nyquist (this is a typical design).
              int32_t mPropertyTransitionBandwidthCheat = 0;
                      // "ro.audio.resampler.psd.tbwcheat"

    // Filter creation design parameters, see setSampleRate()
             double mStopbandAttenuationDb = 0.;
             double mPassbandRippleDb = 0.;
             double mNormalizedTransitionBandwidth = 0.;
             double mFilterAttenuation = 0.;
             double mNormalizedCutoffFrequency = 0.;
};

} // namespace android

#endif /*ANDROID_AUDIO_RESAMPLER_DYN_H*/
