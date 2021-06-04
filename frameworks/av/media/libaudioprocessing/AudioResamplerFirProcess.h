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

#ifndef ANDROID_AUDIO_RESAMPLER_FIR_PROCESS_H
#define ANDROID_AUDIO_RESAMPLER_FIR_PROCESS_H

namespace android {

// depends on AudioResamplerFirOps.h

/* variant for input type TI = int16_t input samples */
template<typename TC>
static inline
void mac(int32_t& l, int32_t& r, TC coef, const int16_t* samples)
{
    uint32_t rl = *reinterpret_cast<const uint32_t*>(samples);
    l = mulAddRL(1, rl, coef, l);
    r = mulAddRL(0, rl, coef, r);
}

template<typename TC>
static inline
void mac(int32_t& l, TC coef, const int16_t* samples)
{
    l = mulAdd(samples[0], coef, l);
}

/* variant for input type TI = float input samples */
template<typename TC>
static inline
void mac(float& l, float& r, TC coef,  const float* samples)
{
    l += *samples++ * coef;
    r += *samples * coef;
}

template<typename TC>
static inline
void mac(float& l, TC coef,  const float* samples)
{
    l += *samples * coef;
}

/* variant for output type TO = int32_t output samples */
static inline
int32_t volumeAdjust(int32_t value, int32_t volume)
{
    return 2 * mulRL(0, value, volume);  // Note: only use top 16b
}

/* variant for output type TO = float output samples */
static inline
float volumeAdjust(float value, float volume)
{
    return value * volume;
}

/*
 * Helper template functions for loop unrolling accumulator operations.
 *
 * Unrolling the loops achieves about 2x gain.
 * Using a recursive template rather than an array of TO[] for the accumulator
 * values is an additional 10-20% gain.
 */

template<int CHANNELS, typename TO>
class Accumulator : public Accumulator<CHANNELS-1, TO> // recursive
{
public:
    inline void clear() {
        value = 0;
        Accumulator<CHANNELS-1, TO>::clear();
    }
    template<typename TC, typename TI>
    inline void acc(TC coef, const TI*& data) {
        mac(value, coef, data++);
        Accumulator<CHANNELS-1, TO>::acc(coef, data);
    }
    inline void volume(TO*& out, TO gain) {
        *out++ += volumeAdjust(value, gain);
        Accumulator<CHANNELS-1, TO>::volume(out, gain);
    }

    TO value; // one per recursive inherited base class
};

template<typename TO>
class Accumulator<0, TO> {
public:
    inline void clear() {
    }
    template<typename TC, typename TI>
    inline void acc(TC coef __unused, const TI*& data __unused) {
    }
    inline void volume(TO*& out __unused, TO gain __unused) {
    }
};

template<typename TC, typename TINTERP>
inline
TC interpolate(TC coef_0, TC coef_1, TINTERP lerp)
{
    return lerp * (coef_1 - coef_0) + coef_0;
}

template<>
inline
int16_t interpolate<int16_t, uint32_t>(int16_t coef_0, int16_t coef_1, uint32_t lerp)
{   // in some CPU architectures 16b x 16b multiplies are faster.
    return (static_cast<int16_t>(lerp) * static_cast<int16_t>(coef_1 - coef_0) >> 15) + coef_0;
}

template<>
inline
int32_t interpolate<int32_t, uint32_t>(int32_t coef_0, int32_t coef_1, uint32_t lerp)
{
    return (lerp * static_cast<int64_t>(coef_1 - coef_0) >> 31) + coef_0;
}

/* class scope for passing in functions into templates */
struct InterpCompute {
    template<typename TC, typename TINTERP>
    static inline
    TC interpolatep(TC coef_0, TC coef_1, TINTERP lerp) {
        return interpolate(coef_0, coef_1, lerp);
    }

    template<typename TC, typename TINTERP>
    static inline
    TC interpolaten(TC coef_0, TC coef_1, TINTERP lerp) {
        return interpolate(coef_0, coef_1, lerp);
    }
};

struct InterpNull {
    template<typename TC, typename TINTERP>
    static inline
    TC interpolatep(TC coef_0, TC coef_1 __unused, TINTERP lerp __unused) {
        return coef_0;
    }

    template<typename TC, typename TINTERP>
    static inline
    TC interpolaten(TC coef_0 __unused, TC coef_1, TINTERP lerp __unused) {
        return coef_1;
    }
};

/*
 * Calculates a single output frame (two samples).
 *
 * The Process*() functions compute both the positive half FIR dot product and
 * the negative half FIR dot product, accumulates, and then applies the volume.
 *
 * Use fir() to compute the proper coefficient pointers for a polyphase
 * filter bank.
 *
 * ProcessBase() is the fundamental processing template function.
 *
 * ProcessL() calls ProcessBase() with TFUNC = InterpNull, for fixed/locked phase.
 * Process() calls ProcessBase() with TFUNC = InterpCompute, for interpolated phase.
 */

template <int CHANNELS, int STRIDE, typename TFUNC, typename TC, typename TI, typename TO,
        typename TINTERP>
static inline
void ProcessBase(TO* const out,
        size_t count,
        const TC* coefsP,
        const TC* coefsN,
        const TI* sP,
        const TI* sN,
        TINTERP lerpP,
        const TO* const volumeLR)
{
    static_assert(CHANNELS > 0, "CHANNELS must be > 0");

    if (CHANNELS > 2) {
        // TO accum[CHANNELS];
        Accumulator<CHANNELS, TO> accum;

        // for (int j = 0; j < CHANNELS; ++j) accum[j] = 0;
        accum.clear();
        for (size_t i = 0; i < count; ++i) {
            TC c = TFUNC::interpolatep(coefsP[0], coefsP[count], lerpP);

            // for (int j = 0; j < CHANNELS; ++j) mac(accum[j], c, sP + j);
            const TI *tmp_data = sP; // tmp_ptr seems to work better
            accum.acc(c, tmp_data);

            coefsP++;
            sP -= CHANNELS;
            c = TFUNC::interpolaten(coefsN[count], coefsN[0], lerpP);

            // for (int j = 0; j < CHANNELS; ++j) mac(accum[j], c, sN + j);
            tmp_data = sN; // tmp_ptr seems faster than directly using sN
            accum.acc(c, tmp_data);

            coefsN++;
            sN += CHANNELS;
        }
        // for (int j = 0; j < CHANNELS; ++j) out[j] += volumeAdjust(accum[j], volumeLR[0]);
        TO *tmp_out = out; // may remove if const out definition changes.
        accum.volume(tmp_out, volumeLR[0]);
    } else if (CHANNELS == 2) {
        TO l = 0;
        TO r = 0;
        for (size_t i = 0; i < count; ++i) {
            mac(l, r, TFUNC::interpolatep(coefsP[0], coefsP[count], lerpP), sP);
            coefsP++;
            sP -= CHANNELS;
            mac(l, r, TFUNC::interpolaten(coefsN[count], coefsN[0], lerpP), sN);
            coefsN++;
            sN += CHANNELS;
        }
        out[0] += volumeAdjust(l, volumeLR[0]);
        out[1] += volumeAdjust(r, volumeLR[1]);
    } else { /* CHANNELS == 1 */
        TO l = 0;
        for (size_t i = 0; i < count; ++i) {
            mac(l, TFUNC::interpolatep(coefsP[0], coefsP[count], lerpP), sP);
            coefsP++;
            sP -= CHANNELS;
            mac(l, TFUNC::interpolaten(coefsN[count], coefsN[0], lerpP), sN);
            coefsN++;
            sN += CHANNELS;
        }
        out[0] += volumeAdjust(l, volumeLR[0]);
        out[1] += volumeAdjust(l, volumeLR[1]);
    }
}

/* Calculates a single output frame from a polyphase resampling filter.
 * See Process() for parameter details.
 */
template <int CHANNELS, int STRIDE, typename TC, typename TI, typename TO>
static inline
void ProcessL(TO* const out,
        int count,
        const TC* coefsP,
        const TC* coefsN,
        const TI* sP,
        const TI* sN,
        const TO* const volumeLR)
{
    ProcessBase<CHANNELS, STRIDE, InterpNull>(out, count, coefsP, coefsN, sP, sN, 0, volumeLR);
}

/*
 * Calculates a single output frame from a polyphase resampling filter,
 * with filter phase interpolation.
 *
 * @param out should point to the output buffer with space for at least one output frame.
 *
 * @param count should be half the size of the total filter length (halfNumCoefs), as we
 * use symmetry in filter coefficients to evaluate two dot products.
 *
 * @param coefsP is one phase of the polyphase filter bank of size halfNumCoefs, corresponding
 * to the positive sP.
 *
 * @param coefsN is one phase of the polyphase filter bank of size halfNumCoefs, corresponding
 * to the negative sN.
 *
 * @param coefsP1 is the next phase of coefsP (used for interpolation).
 *
 * @param coefsN1 is the next phase of coefsN (used for interpolation).
 *
 * @param sP is the positive half of the coefficients (as viewed by a convolution),
 * starting at the original samples pointer and decrementing (by CHANNELS).
 *
 * @param sN is the negative half of the samples (as viewed by a convolution),
 * starting at the original samples pointer + CHANNELS and incrementing (by CHANNELS).
 *
 * @param lerpP The fractional siting between the polyphase indices is given by the bits
 * below coefShift. See fir() for details.
 *
 * @param volumeLR is a pointer to an array of two 32 bit volume values, one per stereo channel,
 * expressed as a S32 integer or float.  A negative value inverts the channel 180 degrees.
 * The pointer volumeLR should be aligned to a minimum of 8 bytes.
 * A typical value for volume is 0x1000 to align to a unity gain output of 20.12.
 */
template <int CHANNELS, int STRIDE, typename TC, typename TI, typename TO, typename TINTERP>
static inline
void Process(TO* const out,
        int count,
        const TC* coefsP,
        const TC* coefsN,
        const TC* coefsP1 __unused,
        const TC* coefsN1 __unused,
        const TI* sP,
        const TI* sN,
        TINTERP lerpP,
        const TO* const volumeLR)
{
    ProcessBase<CHANNELS, STRIDE, InterpCompute>(out, count, coefsP, coefsN, sP, sN, lerpP,
            volumeLR);
}

/*
 * Calculates a single output frame from input sample pointer.
 *
 * This sets up the params for the accelerated Process() and ProcessL()
 * functions to do the appropriate dot products.
 *
 * @param out should point to the output buffer with space for at least one output frame.
 *
 * @param phase is the fractional distance between input frames for interpolation:
 * phase >= 0  && phase < phaseWrapLimit.  It can be thought of as a rational fraction
 * of phase/phaseWrapLimit.
 *
 * @param phaseWrapLimit is #polyphases<<coefShift, where #polyphases is the number of polyphases
 * in the polyphase filter. Likewise, #polyphases can be obtained as (phaseWrapLimit>>coefShift).
 *
 * @param coefShift gives the bit alignment of the polyphase index in the phase parameter.
 *
 * @param halfNumCoefs is the half the number of coefficients per polyphase filter. Since the
 * overall filterbank is odd-length symmetric, only halfNumCoefs need be stored.
 *
 * @param coefs is the polyphase filter bank, starting at from polyphase index 0, and ranging to
 * and including the #polyphases.  Each polyphase of the filter has half-length halfNumCoefs
 * (due to symmetry).  The total size of the filter bank in coefficients is
 * (#polyphases+1)*halfNumCoefs.
 *
 * The filter bank coefs should be aligned to a minimum of 16 bytes (preferrably to cache line).
 *
 * The coefs should be attenuated (to compensate for passband ripple)
 * if storing back into the native format.
 *
 * @param samples are unaligned input samples.  The position is in the "middle" of the
 * sample array with respect to the FIR filter:
 * the negative half of the filter is dot product from samples+1 to samples+halfNumCoefs;
 * the positive half of the filter is dot product from samples to samples-halfNumCoefs+1.
 *
 * @param volumeLR is a pointer to an array of two 32 bit volume values, one per stereo channel,
 * expressed as a S32 integer or float.  A negative value inverts the channel 180 degrees.
 * The pointer volumeLR should be aligned to a minimum of 8 bytes.
 * A typical value for volume is 0x1000 to align to a unity gain output of 20.12.
 *
 * In between calls to filterCoefficient, the phase is incremented by phaseIncrement, where
 * phaseIncrement is calculated as inputSampling * phaseWrapLimit / outputSampling.
 *
 * The filter polyphase index is given by indexP = phase >> coefShift. Due to
 * odd length symmetric filter, the polyphase index of the negative half depends on
 * whether interpolation is used.
 *
 * The fractional siting between the polyphase indices is given by the bits below coefShift:
 *
 * lerpP = phase << 32 - coefShift >> 1;  // for 32 bit unsigned phase multiply
 * lerpP = phase << 32 - coefShift >> 17; // for 16 bit unsigned phase multiply
 *
 * For integer types, this is expressed as:
 *
 * lerpP = phase << sizeof(phase)*8 - coefShift
 *              >> (sizeof(phase)-sizeof(*coefs))*8 + 1;
 *
 * For floating point, lerpP is the fractional phase scaled to [0.0, 1.0):
 *
 * lerpP = (phase << 32 - coefShift) / (1 << 32); // floating point equivalent
 */

template<int CHANNELS, bool LOCKED, int STRIDE, typename TC, typename TI, typename TO>
static inline
void fir(TO* const out,
        const uint32_t phase, const uint32_t phaseWrapLimit,
        const int coefShift, const int halfNumCoefs, const TC* const coefs,
        const TI* const samples, const TO* const volumeLR)
{
    // NOTE: be very careful when modifying the code here. register
    // pressure is very high and a small change might cause the compiler
    // to generate far less efficient code.
    // Always sanity check the result with objdump or test-resample.

    if (LOCKED) {
        // locked polyphase (no interpolation)
        // Compute the polyphase filter index on the positive and negative side.
        uint32_t indexP = phase >> coefShift;
        uint32_t indexN = (phaseWrapLimit - phase) >> coefShift;
        const TC* coefsP = coefs + indexP*halfNumCoefs;
        const TC* coefsN = coefs + indexN*halfNumCoefs;
        const TI* sP = samples;
        const TI* sN = samples + CHANNELS;

        // dot product filter.
        ProcessL<CHANNELS, STRIDE>(out,
                halfNumCoefs, coefsP, coefsN, sP, sN, volumeLR);
    } else {
        // interpolated polyphase
        // Compute the polyphase filter index on the positive and negative side.
        uint32_t indexP = phase >> coefShift;
        uint32_t indexN = (phaseWrapLimit - phase - 1) >> coefShift; // one's complement.
        const TC* coefsP = coefs + indexP*halfNumCoefs;
        const TC* coefsN = coefs + indexN*halfNumCoefs;
        const TC* coefsP1 = coefsP + halfNumCoefs;
        const TC* coefsN1 = coefsN + halfNumCoefs;
        const TI* sP = samples;
        const TI* sN = samples + CHANNELS;

        // Interpolation fraction lerpP derived by shifting all the way up and down
        // to clear the appropriate bits and align to the appropriate level
        // for the integer multiply.  The constants should resolve in compile time.
        //
        // The interpolated filter coefficient is derived as follows for the pos/neg half:
        //
        // interpolated[P] = index[P]*lerpP + index[P+1]*(1-lerpP)
        // interpolated[N] = index[N+1]*lerpP + index[N]*(1-lerpP)

        // on-the-fly interpolated dot product filter
        if (is_same<TC, float>::value || is_same<TC, double>::value) {
            static const TC scale = 1. / (65536. * 65536.); // scale phase bits to [0.0, 1.0)
            TC lerpP = TC(phase << (sizeof(phase)*8 - coefShift)) * scale;

            Process<CHANNELS, STRIDE>(out,
                    halfNumCoefs, coefsP, coefsN, coefsP1, coefsN1, sP, sN, lerpP, volumeLR);
        } else {
            uint32_t lerpP = phase << (sizeof(phase)*8 - coefShift)
                    >> ((sizeof(phase)-sizeof(*coefs))*8 + 1);

            Process<CHANNELS, STRIDE>(out,
                    halfNumCoefs, coefsP, coefsN, coefsP1, coefsN1, sP, sN, lerpP, volumeLR);
        }
    }
}

} // namespace android

#endif /*ANDROID_AUDIO_RESAMPLER_FIR_PROCESS_H*/
