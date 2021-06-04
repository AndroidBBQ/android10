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

#ifndef ANDROID_AUDIO_RESAMPLER_FIR_PROCESS_SSE_H
#define ANDROID_AUDIO_RESAMPLER_FIR_PROCESS_SSE_H

namespace android {

// depends on AudioResamplerFirOps.h, AudioResamplerFirProcess.h

#if USE_SSE

#define TO_STRING2(x) #x
#define TO_STRING(x) TO_STRING2(x)
// uncomment to print GCC version, may be relevant for intrinsic optimizations
/* #pragma message ("GCC version: " TO_STRING(__GNUC__) \
        "." TO_STRING(__GNUC_MINOR__) \
        "." TO_STRING(__GNUC_PATCHLEVEL__)) */

//
// SSEx specializations are enabled for Process() and ProcessL() in AudioResamplerFirProcess.h
//

template <int CHANNELS, int STRIDE, bool FIXED>
static inline void ProcessSSEIntrinsic(float* out,
        int count,
        const float* coefsP,
        const float* coefsN,
        const float* sP,
        const float* sN,
        const float* volumeLR,
        float lerpP,
        const float* coefsP1,
        const float* coefsN1)
{
    ALOG_ASSERT(count > 0 && (count & 7) == 0); // multiple of 8
    static_assert(CHANNELS == 1 || CHANNELS == 2, "CHANNELS must be 1 or 2");

    sP -= CHANNELS*(4-1);   // adjust sP for a loop iteration of four

    __m128 interp;
    if (!FIXED) {
        interp = _mm_set1_ps(lerpP);
    }

    __m128 accL, accR;
    accL = _mm_setzero_ps();
    if (CHANNELS == 2) {
        accR = _mm_setzero_ps();
    }

    do {
        __m128 posCoef = _mm_load_ps(coefsP);
        __m128 negCoef = _mm_load_ps(coefsN);
        coefsP += 4;
        coefsN += 4;

        if (!FIXED) { // interpolate
            __m128 posCoef1 = _mm_load_ps(coefsP1);
            __m128 negCoef1 = _mm_load_ps(coefsN1);
            coefsP1 += 4;
            coefsN1 += 4;

            // Calculate the final coefficient for interpolation
            // posCoef = interp * (posCoef1 - posCoef) + posCoef
            // negCoef = interp * (negCoef - negCoef1) + negCoef1
            posCoef1 = _mm_sub_ps(posCoef1, posCoef);
            negCoef = _mm_sub_ps(negCoef, negCoef1);

            posCoef1 = _mm_mul_ps(posCoef1, interp);
            negCoef = _mm_mul_ps(negCoef, interp);

            posCoef = _mm_add_ps(posCoef1, posCoef);
            negCoef = _mm_add_ps(negCoef, negCoef1);
        }
        switch (CHANNELS) {
        case 1: {
            __m128 posSamp = _mm_loadu_ps(sP);
            __m128 negSamp = _mm_loadu_ps(sN);
            sP -= 4;
            sN += 4;

            posSamp = _mm_shuffle_ps(posSamp, posSamp, 0x1B);
            posSamp = _mm_mul_ps(posSamp, posCoef);
            negSamp = _mm_mul_ps(negSamp, negCoef);

            accL = _mm_add_ps(accL, posSamp);
            accL = _mm_add_ps(accL, negSamp);
        } break;
        case 2: {
            __m128 posSamp0 = _mm_loadu_ps(sP);
            __m128 posSamp1 = _mm_loadu_ps(sP+4);
            __m128 negSamp0 = _mm_loadu_ps(sN);
            __m128 negSamp1 = _mm_loadu_ps(sN+4);
            sP -= 8;
            sN += 8;

            // deinterleave everything and reverse the positives
            __m128 posSampL = _mm_shuffle_ps(posSamp1, posSamp0, 0x22);
            __m128 posSampR = _mm_shuffle_ps(posSamp1, posSamp0, 0x77);
            __m128 negSampL = _mm_shuffle_ps(negSamp0, negSamp1, 0x88);
            __m128 negSampR = _mm_shuffle_ps(negSamp0, negSamp1, 0xDD);

            posSampL = _mm_mul_ps(posSampL, posCoef);
            posSampR = _mm_mul_ps(posSampR, posCoef);
            negSampL = _mm_mul_ps(negSampL, negCoef);
            negSampR = _mm_mul_ps(negSampR, negCoef);

            accL = _mm_add_ps(accL, posSampL);
            accR = _mm_add_ps(accR, posSampR);
            accL = _mm_add_ps(accL, negSampL);
            accR = _mm_add_ps(accR, negSampR);
        } break;
        }
    } while (count -= 4);

    // multiply by volume and save
    __m128 vLR = _mm_setzero_ps();
    __m128 outSamp;
    vLR = _mm_loadl_pi(vLR, reinterpret_cast<const __m64*>(volumeLR));
    outSamp = _mm_loadl_pi(vLR, reinterpret_cast<__m64*>(out));

    // combine and funnel down accumulator
    __m128 outAccum = _mm_setzero_ps();
    if (CHANNELS == 1) {
        // duplicate accL to both L and R
        outAccum = _mm_add_ps(accL, _mm_movehl_ps(accL, accL));
        outAccum = _mm_add_ps(outAccum, _mm_shuffle_ps(outAccum, outAccum, 0x11));
    } else if (CHANNELS == 2) {
        // accR contains R, fold in
        outAccum = _mm_hadd_ps(accL, accR);
        outAccum = _mm_hadd_ps(outAccum, outAccum);
    }

    outAccum = _mm_mul_ps(outAccum, vLR);
    outSamp = _mm_add_ps(outSamp, outAccum);
    _mm_storel_pi(reinterpret_cast<__m64*>(out), outSamp);
}

template<>
inline void ProcessL<1, 16>(float* const out,
        int count,
        const float* coefsP,
        const float* coefsN,
        const float* sP,
        const float* sN,
        const float* const volumeLR)
{
    ProcessSSEIntrinsic<1, 16, true>(out, count, coefsP, coefsN, sP, sN, volumeLR,
            0 /*lerpP*/, NULL /*coefsP1*/, NULL /*coefsN1*/);
}

template<>
inline void ProcessL<2, 16>(float* const out,
        int count,
        const float* coefsP,
        const float* coefsN,
        const float* sP,
        const float* sN,
        const float* const volumeLR)
{
    ProcessSSEIntrinsic<2, 16, true>(out, count, coefsP, coefsN, sP, sN, volumeLR,
            0 /*lerpP*/, NULL /*coefsP1*/, NULL /*coefsN1*/);
}

template<>
inline void Process<1, 16>(float* const out,
        int count,
        const float* coefsP,
        const float* coefsN,
        const float* coefsP1,
        const float* coefsN1,
        const float* sP,
        const float* sN,
        float lerpP,
        const float* const volumeLR)
{
    ProcessSSEIntrinsic<1, 16, false>(out, count, coefsP, coefsN, sP, sN, volumeLR,
            lerpP, coefsP1, coefsN1);
}

template<>
inline void Process<2, 16>(float* const out,
        int count,
        const float* coefsP,
        const float* coefsN,
        const float* coefsP1,
        const float* coefsN1,
        const float* sP,
        const float* sN,
        float lerpP,
        const float* const volumeLR)
{
    ProcessSSEIntrinsic<2, 16, false>(out, count, coefsP, coefsN, sP, sN, volumeLR,
            lerpP, coefsP1, coefsN1);
}

#endif //USE_SSE

} // namespace android

#endif /*ANDROID_AUDIO_RESAMPLER_FIR_PROCESS_SSE_H*/
