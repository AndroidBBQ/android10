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

#ifndef ANDROID_AUDIO_RESAMPLER_FIR_PROCESS_NEON_H
#define ANDROID_AUDIO_RESAMPLER_FIR_PROCESS_NEON_H

namespace android {

// depends on AudioResamplerFirOps.h, AudioResamplerFirProcess.h

#if USE_NEON

// use intrinsics if inline arm32 assembly is not possible
#if !USE_INLINE_ASSEMBLY
#define USE_INTRINSIC
#endif

// following intrinsics available only on ARM 64 bit ACLE
#ifndef __aarch64__
#undef vld1q_f32_x2
#undef vld1q_s32_x2
#endif

#define TO_STRING2(x) #x
#define TO_STRING(x) TO_STRING2(x)
// uncomment to print GCC version, may be relevant for intrinsic optimizations
/* #pragma message ("GCC version: " TO_STRING(__GNUC__) \
        "." TO_STRING(__GNUC_MINOR__) \
        "." TO_STRING(__GNUC_PATCHLEVEL__)) */

//
// NEON specializations are enabled for Process() and ProcessL() in AudioResamplerFirProcess.h
//
// Two variants are presented here:
// ARM NEON inline assembly which appears up to 10-15% faster than intrinsics (gcc 4.9) for arm32.
// ARM NEON intrinsics which can also be used by arm64 and x86/64 with NEON header.
//

// Macros to save a mono/stereo accumulator sample in q0 (and q4) as stereo out.
// These are only used for inline assembly.
#define ASSEMBLY_ACCUMULATE_MONO \
        "vld1.s32       {d2}, [%[vLR]:64]        \n"/* (1) load volumes */\
        "vld1.s32       {d3}, %[out]             \n"/* (2) unaligned load the output */\
        "vpadd.s32      d0, d0, d1               \n"/* (1) add all 4 partial sums */\
        "vpadd.s32      d0, d0, d0               \n"/* (1+4d) and replicate L/R */\
        "vqrdmulh.s32   d0, d0, d2               \n"/* (2+3d) apply volume */\
        "vqadd.s32      d3, d3, d0               \n"/* (1+4d) accumulate result (saturating) */\
        "vst1.s32       {d3}, %[out]             \n"/* (2+2d) store result */

#define ASSEMBLY_ACCUMULATE_STEREO \
        "vld1.s32       {d2}, [%[vLR]:64]        \n"/* (1) load volumes*/\
        "vld1.s32       {d3}, %[out]             \n"/* (2) unaligned load the output*/\
        "vpadd.s32      d0, d0, d1               \n"/* (1) add all 4 partial sums from q0*/\
        "vpadd.s32      d8, d8, d9               \n"/* (1) add all 4 partial sums from q4*/\
        "vpadd.s32      d0, d0, d8               \n"/* (1+4d) combine into L/R*/\
        "vqrdmulh.s32   d0, d0, d2               \n"/* (2+3d) apply volume*/\
        "vqadd.s32      d3, d3, d0               \n"/* (1+4d) accumulate result (saturating)*/\
        "vst1.s32       {d3}, %[out]             \n"/* (2+2d)store result*/

template <int CHANNELS, int STRIDE, bool FIXED>
static inline void ProcessNeonIntrinsic(int32_t* out,
        int count,
        const int16_t* coefsP,
        const int16_t* coefsN,
        const int16_t* sP,
        const int16_t* sN,
        const int32_t* volumeLR,
        uint32_t lerpP,
        const int16_t* coefsP1,
        const int16_t* coefsN1)
{
    ALOG_ASSERT(count > 0 && (count & 7) == 0); // multiple of 8
    static_assert(CHANNELS == 1 || CHANNELS == 2, "CHANNELS must be 1 or 2");

    sP -= CHANNELS*((STRIDE>>1)-1);
    coefsP = (const int16_t*)__builtin_assume_aligned(coefsP, 16);
    coefsN = (const int16_t*)__builtin_assume_aligned(coefsN, 16);

    int16x4_t interp;
    if (!FIXED) {
        interp = vdup_n_s16(lerpP);
        //interp = (int16x4_t)vset_lane_s32 ((int32x2_t)lerpP, interp, 0);
        coefsP1 = (const int16_t*)__builtin_assume_aligned(coefsP1, 16);
        coefsN1 = (const int16_t*)__builtin_assume_aligned(coefsN1, 16);
    }
    int32x4_t accum, accum2;
    // warning uninitialized if we use veorq_s32
    // (alternative to below) accum = veorq_s32(accum, accum);
    accum = vdupq_n_s32(0);
    if (CHANNELS == 2) {
        // (alternative to below) accum2 = veorq_s32(accum2, accum2);
        accum2 = vdupq_n_s32(0);
    }
    do {
        int16x8_t posCoef = vld1q_s16(coefsP);
        coefsP += 8;
        int16x8_t negCoef = vld1q_s16(coefsN);
        coefsN += 8;
        if (!FIXED) { // interpolate
            int16x8_t posCoef1 = vld1q_s16(coefsP1);
            coefsP1 += 8;
            int16x8_t negCoef1 = vld1q_s16(coefsN1);
            coefsN1 += 8;

            posCoef1 = vsubq_s16(posCoef1, posCoef);
            negCoef = vsubq_s16(negCoef, negCoef1);

            posCoef1 = vqrdmulhq_lane_s16(posCoef1, interp, 0);
            negCoef = vqrdmulhq_lane_s16(negCoef, interp, 0);

            posCoef = vaddq_s16(posCoef, posCoef1);
            negCoef = vaddq_s16(negCoef, negCoef1);
        }
        switch (CHANNELS) {
        case 1: {
            int16x8_t posSamp = vld1q_s16(sP);
            int16x8_t negSamp = vld1q_s16(sN);
            sN += 8;
            posSamp = vrev64q_s16(posSamp);

            // dot product
            accum = vmlal_s16(accum, vget_low_s16(posSamp), vget_high_s16(posCoef)); // reversed
            accum = vmlal_s16(accum, vget_high_s16(posSamp), vget_low_s16(posCoef)); // reversed
            accum = vmlal_s16(accum, vget_low_s16(negSamp), vget_low_s16(negCoef));
            accum = vmlal_s16(accum, vget_high_s16(negSamp), vget_high_s16(negCoef));
            sP -= 8;
        } break;
        case 2: {
            int16x8x2_t posSamp = vld2q_s16(sP);
            int16x8x2_t negSamp = vld2q_s16(sN);
            sN += 16;
            posSamp.val[0] = vrev64q_s16(posSamp.val[0]);
            posSamp.val[1] = vrev64q_s16(posSamp.val[1]);

            // dot product
            accum = vmlal_s16(accum, vget_low_s16(posSamp.val[0]), vget_high_s16(posCoef)); // r
            accum = vmlal_s16(accum, vget_high_s16(posSamp.val[0]), vget_low_s16(posCoef)); // r
            accum2 = vmlal_s16(accum2, vget_low_s16(posSamp.val[1]), vget_high_s16(posCoef)); // r
            accum2 = vmlal_s16(accum2, vget_high_s16(posSamp.val[1]), vget_low_s16(posCoef)); // r
            accum = vmlal_s16(accum, vget_low_s16(negSamp.val[0]), vget_low_s16(negCoef));
            accum = vmlal_s16(accum, vget_high_s16(negSamp.val[0]), vget_high_s16(negCoef));
            accum2 = vmlal_s16(accum2, vget_low_s16(negSamp.val[1]), vget_low_s16(negCoef));
            accum2 = vmlal_s16(accum2, vget_high_s16(negSamp.val[1]), vget_high_s16(negCoef));
            sP -= 16;
        } break;
        }
    } while (count -= 8);

    // multiply by volume and save
    volumeLR = (const int32_t*)__builtin_assume_aligned(volumeLR, 8);
    int32x2_t vLR = vld1_s32(volumeLR);
    int32x2_t outSamp = vld1_s32(out);
    // combine and funnel down accumulator
    int32x2_t outAccum = vpadd_s32(vget_low_s32(accum), vget_high_s32(accum));
    if (CHANNELS == 1) {
        // duplicate accum to both L and R
        outAccum = vpadd_s32(outAccum, outAccum);
    } else if (CHANNELS == 2) {
        // accum2 contains R, fold in
        int32x2_t outAccum2 = vpadd_s32(vget_low_s32(accum2), vget_high_s32(accum2));
        outAccum = vpadd_s32(outAccum, outAccum2);
    }
    outAccum = vqrdmulh_s32(outAccum, vLR);
    outSamp = vqadd_s32(outSamp, outAccum);
    vst1_s32(out, outSamp);
}

template <int CHANNELS, int STRIDE, bool FIXED>
static inline void ProcessNeonIntrinsic(int32_t* out,
        int count,
        const int32_t* coefsP,
        const int32_t* coefsN,
        const int16_t* sP,
        const int16_t* sN,
        const int32_t* volumeLR,
        uint32_t lerpP,
        const int32_t* coefsP1,
        const int32_t* coefsN1)
{
    ALOG_ASSERT(count > 0 && (count & 7) == 0); // multiple of 8
    static_assert(CHANNELS == 1 || CHANNELS == 2, "CHANNELS must be 1 or 2");

    sP -= CHANNELS*((STRIDE>>1)-1);
    coefsP = (const int32_t*)__builtin_assume_aligned(coefsP, 16);
    coefsN = (const int32_t*)__builtin_assume_aligned(coefsN, 16);

    int32x2_t interp;
    if (!FIXED) {
        interp = vdup_n_s32(lerpP);
        coefsP1 = (const int32_t*)__builtin_assume_aligned(coefsP1, 16);
        coefsN1 = (const int32_t*)__builtin_assume_aligned(coefsN1, 16);
    }
    int32x4_t accum, accum2;
    // warning uninitialized if we use veorq_s32
    // (alternative to below) accum = veorq_s32(accum, accum);
    accum = vdupq_n_s32(0);
    if (CHANNELS == 2) {
        // (alternative to below) accum2 = veorq_s32(accum2, accum2);
        accum2 = vdupq_n_s32(0);
    }
    do {
#ifdef vld1q_s32_x2
        int32x4x2_t posCoef = vld1q_s32_x2(coefsP);
        coefsP += 8;
        int32x4x2_t negCoef = vld1q_s32_x2(coefsN);
        coefsN += 8;
#else
        int32x4x2_t posCoef;
        posCoef.val[0] = vld1q_s32(coefsP);
        coefsP += 4;
        posCoef.val[1] = vld1q_s32(coefsP);
        coefsP += 4;
        int32x4x2_t negCoef;
        negCoef.val[0] = vld1q_s32(coefsN);
        coefsN += 4;
        negCoef.val[1] = vld1q_s32(coefsN);
        coefsN += 4;
#endif
        if (!FIXED) { // interpolate
#ifdef vld1q_s32_x2
            int32x4x2_t posCoef1 = vld1q_s32_x2(coefsP1);
            coefsP1 += 8;
            int32x4x2_t negCoef1 = vld1q_s32_x2(coefsN1);
            coefsN1 += 8;
#else
            int32x4x2_t posCoef1;
            posCoef1.val[0] = vld1q_s32(coefsP1);
            coefsP1 += 4;
            posCoef1.val[1] = vld1q_s32(coefsP1);
            coefsP1 += 4;
            int32x4x2_t negCoef1;
            negCoef1.val[0] = vld1q_s32(coefsN1);
            coefsN1 += 4;
            negCoef1.val[1] = vld1q_s32(coefsN1);
            coefsN1 += 4;
#endif

            posCoef1.val[0] = vsubq_s32(posCoef1.val[0], posCoef.val[0]);
            posCoef1.val[1] = vsubq_s32(posCoef1.val[1], posCoef.val[1]);
            negCoef.val[0] = vsubq_s32(negCoef.val[0], negCoef1.val[0]);
            negCoef.val[1] = vsubq_s32(negCoef.val[1], negCoef1.val[1]);

            posCoef1.val[0] = vqrdmulhq_lane_s32(posCoef1.val[0], interp, 0);
            posCoef1.val[1] = vqrdmulhq_lane_s32(posCoef1.val[1], interp, 0);
            negCoef.val[0] = vqrdmulhq_lane_s32(negCoef.val[0], interp, 0);
            negCoef.val[1] = vqrdmulhq_lane_s32(negCoef.val[1], interp, 0);

            posCoef.val[0] = vaddq_s32(posCoef.val[0], posCoef1.val[0]);
            posCoef.val[1] = vaddq_s32(posCoef.val[1], posCoef1.val[1]);
            negCoef.val[0] = vaddq_s32(negCoef.val[0], negCoef1.val[0]);
            negCoef.val[1] = vaddq_s32(negCoef.val[1], negCoef1.val[1]);
        }
        switch (CHANNELS) {
        case 1: {
            int16x8_t posSamp = vld1q_s16(sP);
            int16x8_t negSamp = vld1q_s16(sN);
            sN += 8;
            posSamp = vrev64q_s16(posSamp);

            int32x4_t posSamp0 = vshll_n_s16(vget_low_s16(posSamp), 15);
            int32x4_t posSamp1 = vshll_n_s16(vget_high_s16(posSamp), 15);
            int32x4_t negSamp0 = vshll_n_s16(vget_low_s16(negSamp), 15);
            int32x4_t negSamp1 = vshll_n_s16(vget_high_s16(negSamp), 15);

            // dot product
            posSamp0 = vqrdmulhq_s32(posSamp0, posCoef.val[1]); // reversed
            posSamp1 = vqrdmulhq_s32(posSamp1, posCoef.val[0]); // reversed
            negSamp0 = vqrdmulhq_s32(negSamp0, negCoef.val[0]);
            negSamp1 = vqrdmulhq_s32(negSamp1, negCoef.val[1]);

            accum = vaddq_s32(accum, posSamp0);
            negSamp0 = vaddq_s32(negSamp0, negSamp1);
            accum = vaddq_s32(accum, posSamp1);
            accum = vaddq_s32(accum, negSamp0);

            sP -= 8;
        } break;
        case 2: {
            int16x8x2_t posSamp = vld2q_s16(sP);
            int16x8x2_t negSamp = vld2q_s16(sN);
            sN += 16;
            posSamp.val[0] = vrev64q_s16(posSamp.val[0]);
            posSamp.val[1] = vrev64q_s16(posSamp.val[1]);

            // left
            int32x4_t posSamp0 = vshll_n_s16(vget_low_s16(posSamp.val[0]), 15);
            int32x4_t posSamp1 = vshll_n_s16(vget_high_s16(posSamp.val[0]), 15);
            int32x4_t negSamp0 = vshll_n_s16(vget_low_s16(negSamp.val[0]), 15);
            int32x4_t negSamp1 = vshll_n_s16(vget_high_s16(negSamp.val[0]), 15);

            // dot product
            posSamp0 = vqrdmulhq_s32(posSamp0, posCoef.val[1]); // reversed
            posSamp1 = vqrdmulhq_s32(posSamp1, posCoef.val[0]); // reversed
            negSamp0 = vqrdmulhq_s32(negSamp0, negCoef.val[0]);
            negSamp1 = vqrdmulhq_s32(negSamp1, negCoef.val[1]);

            accum = vaddq_s32(accum, posSamp0);
            negSamp0 = vaddq_s32(negSamp0, negSamp1);
            accum = vaddq_s32(accum, posSamp1);
            accum = vaddq_s32(accum, negSamp0);

            // right
            posSamp0 = vshll_n_s16(vget_low_s16(posSamp.val[1]), 15);
            posSamp1 = vshll_n_s16(vget_high_s16(posSamp.val[1]), 15);
            negSamp0 = vshll_n_s16(vget_low_s16(negSamp.val[1]), 15);
            negSamp1 = vshll_n_s16(vget_high_s16(negSamp.val[1]), 15);

            // dot product
            posSamp0 = vqrdmulhq_s32(posSamp0, posCoef.val[1]); // reversed
            posSamp1 = vqrdmulhq_s32(posSamp1, posCoef.val[0]); // reversed
            negSamp0 = vqrdmulhq_s32(negSamp0, negCoef.val[0]);
            negSamp1 = vqrdmulhq_s32(negSamp1, negCoef.val[1]);

            accum2 = vaddq_s32(accum2, posSamp0);
            negSamp0 = vaddq_s32(negSamp0, negSamp1);
            accum2 = vaddq_s32(accum2, posSamp1);
            accum2 = vaddq_s32(accum2, negSamp0);

            sP -= 16;
        } break;
        }
    } while (count -= 8);

    // multiply by volume and save
    volumeLR = (const int32_t*)__builtin_assume_aligned(volumeLR, 8);
    int32x2_t vLR = vld1_s32(volumeLR);
    int32x2_t outSamp = vld1_s32(out);
    // combine and funnel down accumulator
    int32x2_t outAccum = vpadd_s32(vget_low_s32(accum), vget_high_s32(accum));
    if (CHANNELS == 1) {
        // duplicate accum to both L and R
        outAccum = vpadd_s32(outAccum, outAccum);
    } else if (CHANNELS == 2) {
        // accum2 contains R, fold in
        int32x2_t outAccum2 = vpadd_s32(vget_low_s32(accum2), vget_high_s32(accum2));
        outAccum = vpadd_s32(outAccum, outAccum2);
    }
    outAccum = vqrdmulh_s32(outAccum, vLR);
    outSamp = vqadd_s32(outSamp, outAccum);
    vst1_s32(out, outSamp);
}

template <int CHANNELS, int STRIDE, bool FIXED>
static inline void ProcessNeonIntrinsic(float* out,
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

    sP -= CHANNELS*((STRIDE>>1)-1);
    coefsP = (const float*)__builtin_assume_aligned(coefsP, 16);
    coefsN = (const float*)__builtin_assume_aligned(coefsN, 16);

    float32x2_t interp;
    if (!FIXED) {
        interp = vdup_n_f32(lerpP);
        coefsP1 = (const float*)__builtin_assume_aligned(coefsP1, 16);
        coefsN1 = (const float*)__builtin_assume_aligned(coefsN1, 16);
    }
    float32x4_t accum, accum2;
    // warning uninitialized if we use veorq_s32
    // (alternative to below) accum = veorq_s32(accum, accum);
    accum = vdupq_n_f32(0);
    if (CHANNELS == 2) {
        // (alternative to below) accum2 = veorq_s32(accum2, accum2);
        accum2 = vdupq_n_f32(0);
    }
    do {
#ifdef vld1q_f32_x2
        float32x4x2_t posCoef = vld1q_f32_x2(coefsP);
        coefsP += 8;
        float32x4x2_t negCoef = vld1q_f32_x2(coefsN);
        coefsN += 8;
#else
        float32x4x2_t posCoef;
        posCoef.val[0] = vld1q_f32(coefsP);
        coefsP += 4;
        posCoef.val[1] = vld1q_f32(coefsP);
        coefsP += 4;
        float32x4x2_t negCoef;
        negCoef.val[0] = vld1q_f32(coefsN);
        coefsN += 4;
        negCoef.val[1] = vld1q_f32(coefsN);
        coefsN += 4;
#endif
        if (!FIXED) { // interpolate
#ifdef vld1q_f32_x2
            float32x4x2_t posCoef1 = vld1q_f32_x2(coefsP1);
            coefsP1 += 8;
            float32x4x2_t negCoef1 = vld1q_f32_x2(coefsN1);
            coefsN1 += 8;
#else
            float32x4x2_t posCoef1;
            posCoef1.val[0] = vld1q_f32(coefsP1);
            coefsP1 += 4;
            posCoef1.val[1] = vld1q_f32(coefsP1);
            coefsP1 += 4;
            float32x4x2_t negCoef1;
            negCoef1.val[0] = vld1q_f32(coefsN1);
            coefsN1 += 4;
            negCoef1.val[1] = vld1q_f32(coefsN1);
            coefsN1 += 4;
#endif
            posCoef1.val[0] = vsubq_f32(posCoef1.val[0], posCoef.val[0]);
            posCoef1.val[1] = vsubq_f32(posCoef1.val[1], posCoef.val[1]);
            negCoef.val[0] = vsubq_f32(negCoef.val[0], negCoef1.val[0]);
            negCoef.val[1] = vsubq_f32(negCoef.val[1], negCoef1.val[1]);

            posCoef.val[0] = vmlaq_lane_f32(posCoef.val[0], posCoef1.val[0], interp, 0);
            posCoef.val[1] = vmlaq_lane_f32(posCoef.val[1], posCoef1.val[1], interp, 0);
            negCoef.val[0] = vmlaq_lane_f32(negCoef1.val[0], negCoef.val[0], interp, 0); // rev
            negCoef.val[1] = vmlaq_lane_f32(negCoef1.val[1], negCoef.val[1], interp, 0); // rev
        }
        switch (CHANNELS) {
        case 1: {
#ifdef vld1q_f32_x2
            float32x4x2_t posSamp = vld1q_f32_x2(sP);
            float32x4x2_t negSamp = vld1q_f32_x2(sN);
            sN += 8;
            sP -= 8;
#else
            float32x4x2_t posSamp;
            posSamp.val[0] = vld1q_f32(sP);
            sP += 4;
            posSamp.val[1] = vld1q_f32(sP);
            sP -= 12;
            float32x4x2_t negSamp;
            negSamp.val[0] = vld1q_f32(sN);
            sN += 4;
            negSamp.val[1] = vld1q_f32(sN);
            sN += 4;
#endif
            // effectively we want a vrev128q_f32()
            posSamp.val[0] = vrev64q_f32(posSamp.val[0]);
            posSamp.val[1] = vrev64q_f32(posSamp.val[1]);
            posSamp.val[0] = vcombine_f32(
                    vget_high_f32(posSamp.val[0]), vget_low_f32(posSamp.val[0]));
            posSamp.val[1] = vcombine_f32(
                    vget_high_f32(posSamp.val[1]), vget_low_f32(posSamp.val[1]));

            accum = vmlaq_f32(accum, posSamp.val[0], posCoef.val[1]);
            accum = vmlaq_f32(accum, posSamp.val[1], posCoef.val[0]);
            accum = vmlaq_f32(accum, negSamp.val[0], negCoef.val[0]);
            accum = vmlaq_f32(accum, negSamp.val[1], negCoef.val[1]);
        } break;
        case 2: {
            float32x4x2_t posSamp0 = vld2q_f32(sP);
            sP += 8;
            float32x4x2_t negSamp0 = vld2q_f32(sN);
            sN += 8;
            posSamp0.val[0] = vrev64q_f32(posSamp0.val[0]);
            posSamp0.val[1] = vrev64q_f32(posSamp0.val[1]);
            posSamp0.val[0] = vcombine_f32(
                    vget_high_f32(posSamp0.val[0]), vget_low_f32(posSamp0.val[0]));
            posSamp0.val[1] = vcombine_f32(
                    vget_high_f32(posSamp0.val[1]), vget_low_f32(posSamp0.val[1]));

            float32x4x2_t posSamp1 = vld2q_f32(sP);
            sP -= 24;
            float32x4x2_t negSamp1 = vld2q_f32(sN);
            sN += 8;
            posSamp1.val[0] = vrev64q_f32(posSamp1.val[0]);
            posSamp1.val[1] = vrev64q_f32(posSamp1.val[1]);
            posSamp1.val[0] = vcombine_f32(
                    vget_high_f32(posSamp1.val[0]), vget_low_f32(posSamp1.val[0]));
            posSamp1.val[1] = vcombine_f32(
                    vget_high_f32(posSamp1.val[1]), vget_low_f32(posSamp1.val[1]));

            // Note: speed is affected by accumulation order.
            // Also, speed appears slower using vmul/vadd instead of vmla for
            // stereo case, comparable for mono.

            accum = vmlaq_f32(accum, negSamp0.val[0], negCoef.val[0]);
            accum = vmlaq_f32(accum, negSamp1.val[0], negCoef.val[1]);
            accum2 = vmlaq_f32(accum2, negSamp0.val[1], negCoef.val[0]);
            accum2 = vmlaq_f32(accum2, negSamp1.val[1], negCoef.val[1]);

            accum = vmlaq_f32(accum, posSamp0.val[0], posCoef.val[1]); // reversed
            accum = vmlaq_f32(accum, posSamp1.val[0], posCoef.val[0]); // reversed
            accum2 = vmlaq_f32(accum2, posSamp0.val[1], posCoef.val[1]); // reversed
            accum2 = vmlaq_f32(accum2, posSamp1.val[1], posCoef.val[0]); // reversed
        } break;
        }
    } while (count -= 8);

    // multiply by volume and save
    volumeLR = (const float*)__builtin_assume_aligned(volumeLR, 8);
    float32x2_t vLR = vld1_f32(volumeLR);
    float32x2_t outSamp = vld1_f32(out);
    // combine and funnel down accumulator
    float32x2_t outAccum = vpadd_f32(vget_low_f32(accum), vget_high_f32(accum));
    if (CHANNELS == 1) {
        // duplicate accum to both L and R
        outAccum = vpadd_f32(outAccum, outAccum);
    } else if (CHANNELS == 2) {
        // accum2 contains R, fold in
        float32x2_t outAccum2 = vpadd_f32(vget_low_f32(accum2), vget_high_f32(accum2));
        outAccum = vpadd_f32(outAccum, outAccum2);
    }
    outSamp = vmla_f32(outSamp, outAccum, vLR);
    vst1_f32(out, outSamp);
}

template <>
inline void ProcessL<1, 16>(int32_t* const out,
        int count,
        const int16_t* coefsP,
        const int16_t* coefsN,
        const int16_t* sP,
        const int16_t* sN,
        const int32_t* const volumeLR)
{
#ifdef USE_INTRINSIC
    ProcessNeonIntrinsic<1, 16, true>(out, count, coefsP, coefsN, sP, sN, volumeLR,
            0 /*lerpP*/, NULL /*coefsP1*/, NULL /*coefsN1*/);
#else
    const int CHANNELS = 1; // template specialization does not preserve params
    const int STRIDE = 16;
    sP -= CHANNELS*((STRIDE>>1)-1);
    asm (
        "veor           q0, q0, q0               \n"// (0 - combines+) accumulator = 0

        "1:                                      \n"

        "vld1.16        {q2}, [%[sP]]            \n"// (2+0d) load 8 16-bits mono samples
        "vld1.16        {q3}, [%[sN]]!           \n"// (2) load 8 16-bits mono samples
        "vld1.16        {q8}, [%[coefsP0]:128]!  \n"// (1) load 8 16-bits coefs
        "vld1.16        {q10}, [%[coefsN0]:128]! \n"// (1) load 8 16-bits coefs

        "vrev64.16      q2, q2                   \n"// (1) reverse s3, s2, s1, s0, s7, s6, s5, s4

        // reordering the vmal to do d6, d7 before d4, d5 is slower(?)
        "vmlal.s16      q0, d4, d17              \n"// (1+0d) multiply (reversed)samples by coef
        "vmlal.s16      q0, d5, d16              \n"// (1) multiply (reversed)samples by coef
        "vmlal.s16      q0, d6, d20              \n"// (1) multiply neg samples
        "vmlal.s16      q0, d7, d21              \n"// (1) multiply neg samples

        // moving these ARM instructions before neon above seems to be slower
        "subs           %[count], %[count], #8   \n"// (1) update loop counter
        "sub            %[sP], %[sP], #16        \n"// (0) move pointer to next set of samples

        // sP used after branch (warning)
        "bne            1b                       \n"// loop

         ASSEMBLY_ACCUMULATE_MONO

        : [out]     "=Uv" (out[0]),
          [count]   "+r" (count),
          [coefsP0] "+r" (coefsP),
          [coefsN0] "+r" (coefsN),
          [sP]      "+r" (sP),
          [sN]      "+r" (sN)
        : [vLR]     "r" (volumeLR)
        : "cc", "memory",
          "q0", "q1", "q2", "q3",
          "q8", "q10"
    );
#endif
}

template <>
inline void ProcessL<2, 16>(int32_t* const out,
        int count,
        const int16_t* coefsP,
        const int16_t* coefsN,
        const int16_t* sP,
        const int16_t* sN,
        const int32_t* const volumeLR)
{
#ifdef USE_INTRINSIC
    ProcessNeonIntrinsic<2, 16, true>(out, count, coefsP, coefsN, sP, sN, volumeLR,
            0 /*lerpP*/, NULL /*coefsP1*/, NULL /*coefsN1*/);
#else
    const int CHANNELS = 2; // template specialization does not preserve params
    const int STRIDE = 16;
    sP -= CHANNELS*((STRIDE>>1)-1);
    asm (
        "veor           q0, q0, q0               \n"// (1) acc_L = 0
        "veor           q4, q4, q4               \n"// (0 combines+) acc_R = 0

        "1:                                      \n"

        "vld2.16        {q2, q3}, [%[sP]]        \n"// (3+0d) load 8 16-bits stereo frames
        "vld2.16        {q5, q6}, [%[sN]]!       \n"// (3) load 8 16-bits stereo frames
        "vld1.16        {q8}, [%[coefsP0]:128]!  \n"// (1) load 8 16-bits coefs
        "vld1.16        {q10}, [%[coefsN0]:128]! \n"// (1) load 8 16-bits coefs

        "vrev64.16      q2, q2                   \n"// (1) reverse 8 samples of positive left
        "vrev64.16      q3, q3                   \n"// (0 combines+) reverse positive right

        "vmlal.s16      q0, d4, d17              \n"// (1) multiply (reversed) samples left
        "vmlal.s16      q0, d5, d16              \n"// (1) multiply (reversed) samples left
        "vmlal.s16      q4, d6, d17              \n"// (1) multiply (reversed) samples right
        "vmlal.s16      q4, d7, d16              \n"// (1) multiply (reversed) samples right
        "vmlal.s16      q0, d10, d20             \n"// (1) multiply samples left
        "vmlal.s16      q0, d11, d21             \n"// (1) multiply samples left
        "vmlal.s16      q4, d12, d20             \n"// (1) multiply samples right
        "vmlal.s16      q4, d13, d21             \n"// (1) multiply samples right

        // moving these ARM before neon seems to be slower
        "subs           %[count], %[count], #8   \n"// (1) update loop counter
        "sub            %[sP], %[sP], #32        \n"// (0) move pointer to next set of samples

        // sP used after branch (warning)
        "bne            1b                       \n"// loop

        ASSEMBLY_ACCUMULATE_STEREO

        : [out] "=Uv" (out[0]),
          [count] "+r" (count),
          [coefsP0] "+r" (coefsP),
          [coefsN0] "+r" (coefsN),
          [sP] "+r" (sP),
          [sN] "+r" (sN)
        : [vLR] "r" (volumeLR)
        : "cc", "memory",
          "q0", "q1", "q2", "q3",
          "q4", "q5", "q6",
          "q8", "q10"
     );
#endif
}

template <>
inline void Process<1, 16>(int32_t* const out,
        int count,
        const int16_t* coefsP,
        const int16_t* coefsN,
        const int16_t* coefsP1,
        const int16_t* coefsN1,
        const int16_t* sP,
        const int16_t* sN,
        uint32_t lerpP,
        const int32_t* const volumeLR)
{
#ifdef USE_INTRINSIC
    ProcessNeonIntrinsic<1, 16, false>(out, count, coefsP, coefsN, sP, sN, volumeLR,
            lerpP, coefsP1, coefsN1);
#else

    const int CHANNELS = 1; // template specialization does not preserve params
    const int STRIDE = 16;
    sP -= CHANNELS*((STRIDE>>1)-1);
    asm (
        "vmov.32        d2[0], %[lerpP]          \n"// load the positive phase S32 Q15
        "veor           q0, q0, q0               \n"// (0 - combines+) accumulator = 0

        "1:                                      \n"

        "vld1.16        {q2}, [%[sP]]            \n"// (2+0d) load 8 16-bits mono samples
        "vld1.16        {q3}, [%[sN]]!           \n"// (2) load 8 16-bits mono samples
        "vld1.16        {q8}, [%[coefsP0]:128]!  \n"// (1) load 8 16-bits coefs
        "vld1.16        {q9}, [%[coefsP1]:128]!  \n"// (1) load 8 16-bits coefs for interpolation
        "vld1.16        {q10}, [%[coefsN1]:128]! \n"// (1) load 8 16-bits coefs
        "vld1.16        {q11}, [%[coefsN0]:128]! \n"// (1) load 8 16-bits coefs for interpolation

        "vsub.s16       q9, q9, q8               \n"// (1) interpolate (step1) 1st set of coefs
        "vsub.s16       q11, q11, q10            \n"// (1) interpolate (step1) 2nd set of coets

        "vqrdmulh.s16   q9, q9, d2[0]            \n"// (2) interpolate (step2) 1st set of coefs
        "vqrdmulh.s16   q11, q11, d2[0]          \n"// (2) interpolate (step2) 2nd set of coefs

        "vrev64.16      q2, q2                   \n"// (1) reverse s3, s2, s1, s0, s7, s6, s5, s4

        "vadd.s16       q8, q8, q9               \n"// (1+2d) interpolate (step3) 1st set
        "vadd.s16       q10, q10, q11            \n"// (1+1d) interpolate (step3) 2nd set

        // reordering the vmal to do d6, d7 before d4, d5 is slower(?)
        "vmlal.s16      q0, d4, d17              \n"// (1+0d) multiply reversed samples by coef
        "vmlal.s16      q0, d5, d16              \n"// (1) multiply reversed samples by coef
        "vmlal.s16      q0, d6, d20              \n"// (1) multiply neg samples
        "vmlal.s16      q0, d7, d21              \n"// (1) multiply neg samples

        // moving these ARM instructions before neon above seems to be slower
        "subs           %[count], %[count], #8   \n"// (1) update loop counter
        "sub            %[sP], %[sP], #16        \n"// (0) move pointer to next set of samples

        // sP used after branch (warning)
        "bne            1b                       \n"// loop

        ASSEMBLY_ACCUMULATE_MONO

        : [out]     "=Uv" (out[0]),
          [count]   "+r" (count),
          [coefsP0] "+r" (coefsP),
          [coefsN0] "+r" (coefsN),
          [coefsP1] "+r" (coefsP1),
          [coefsN1] "+r" (coefsN1),
          [sP]      "+r" (sP),
          [sN]      "+r" (sN)
        : [lerpP]   "r" (lerpP),
          [vLR]     "r" (volumeLR)
        : "cc", "memory",
          "q0", "q1", "q2", "q3",
          "q8", "q9", "q10", "q11"
    );
#endif
}

template <>
inline void Process<2, 16>(int32_t* const out,
        int count,
        const int16_t* coefsP,
        const int16_t* coefsN,
        const int16_t* coefsP1,
        const int16_t* coefsN1,
        const int16_t* sP,
        const int16_t* sN,
        uint32_t lerpP,
        const int32_t* const volumeLR)
{
#ifdef USE_INTRINSIC
    ProcessNeonIntrinsic<2, 16, false>(out, count, coefsP, coefsN, sP, sN, volumeLR,
            lerpP, coefsP1, coefsN1);
#else
    const int CHANNELS = 2; // template specialization does not preserve params
    const int STRIDE = 16;
    sP -= CHANNELS*((STRIDE>>1)-1);
    asm (
        "vmov.32        d2[0], %[lerpP]          \n"// load the positive phase
        "veor           q0, q0, q0               \n"// (1) acc_L = 0
        "veor           q4, q4, q4               \n"// (0 combines+) acc_R = 0

        "1:                                      \n"

        "vld2.16        {q2, q3}, [%[sP]]        \n"// (3+0d) load 8 16-bits stereo frames
        "vld2.16        {q5, q6}, [%[sN]]!       \n"// (3) load 8 16-bits stereo frames
        "vld1.16        {q8}, [%[coefsP0]:128]!  \n"// (1) load 8 16-bits coefs
        "vld1.16        {q9}, [%[coefsP1]:128]!  \n"// (1) load 8 16-bits coefs for interpolation
        "vld1.16        {q10}, [%[coefsN1]:128]! \n"// (1) load 8 16-bits coefs
        "vld1.16        {q11}, [%[coefsN0]:128]! \n"// (1) load 8 16-bits coefs for interpolation

        "vsub.s16       q9, q9, q8               \n"// (1) interpolate (step1) 1st set of coefs
        "vsub.s16       q11, q11, q10            \n"// (1) interpolate (step1) 2nd set of coets

        "vqrdmulh.s16   q9, q9, d2[0]            \n"// (2) interpolate (step2) 1st set of coefs
        "vqrdmulh.s16   q11, q11, d2[0]          \n"// (2) interpolate (step2) 2nd set of coefs

        "vrev64.16      q2, q2                   \n"// (1) reverse 8 samples of positive left
        "vrev64.16      q3, q3                   \n"// (1) reverse 8 samples of positive right

        "vadd.s16       q8, q8, q9               \n"// (1+1d) interpolate (step3) 1st set
        "vadd.s16       q10, q10, q11            \n"// (1+1d) interpolate (step3) 2nd set

        "vmlal.s16      q0, d4, d17              \n"// (1) multiply reversed samples left
        "vmlal.s16      q0, d5, d16              \n"// (1) multiply reversed samples left
        "vmlal.s16      q4, d6, d17              \n"// (1) multiply reversed samples right
        "vmlal.s16      q4, d7, d16              \n"// (1) multiply reversed samples right
        "vmlal.s16      q0, d10, d20             \n"// (1) multiply samples left
        "vmlal.s16      q0, d11, d21             \n"// (1) multiply samples left
        "vmlal.s16      q4, d12, d20             \n"// (1) multiply samples right
        "vmlal.s16      q4, d13, d21             \n"// (1) multiply samples right

        // moving these ARM before neon seems to be slower
        "subs           %[count], %[count], #8   \n"// (1) update loop counter
        "sub            %[sP], %[sP], #32        \n"// (0) move pointer to next set of samples

        // sP used after branch (warning)
        "bne            1b                       \n"// loop

        ASSEMBLY_ACCUMULATE_STEREO

        : [out] "=Uv" (out[0]),
          [count] "+r" (count),
          [coefsP0] "+r" (coefsP),
          [coefsN0] "+r" (coefsN),
          [coefsP1] "+r" (coefsP1),
          [coefsN1] "+r" (coefsN1),
          [sP] "+r" (sP),
          [sN] "+r" (sN)
        : [lerpP]   "r" (lerpP),
          [vLR] "r" (volumeLR)
        : "cc", "memory",
          "q0", "q1", "q2", "q3",
          "q4", "q5", "q6",
          "q8", "q9", "q10", "q11"
    );
#endif
}

template <>
inline void ProcessL<1, 16>(int32_t* const out,
        int count,
        const int32_t* coefsP,
        const int32_t* coefsN,
        const int16_t* sP,
        const int16_t* sN,
        const int32_t* const volumeLR)
{
#ifdef USE_INTRINSIC
    ProcessNeonIntrinsic<1, 16, true>(out, count, coefsP, coefsN, sP, sN, volumeLR,
            0 /*lerpP*/, NULL /*coefsP1*/, NULL /*coefsN1*/);
#else
    const int CHANNELS = 1; // template specialization does not preserve params
    const int STRIDE = 16;
    sP -= CHANNELS*((STRIDE>>1)-1);
    asm (
        "veor           q0, q0, q0                    \n"// result, initialize to 0

        "1:                                           \n"

        "vld1.16        {q2}, [%[sP]]                 \n"// load 8 16-bits mono samples
        "vld1.16        {q3}, [%[sN]]!                \n"// load 8 16-bits mono samples
        "vld1.32        {q8, q9}, [%[coefsP0]:128]!   \n"// load 8 32-bits coefs
        "vld1.32        {q10, q11}, [%[coefsN0]:128]! \n"// load 8 32-bits coefs

        "vrev64.16      q2, q2                        \n"// reverse 8 samples of the positive side

        "vshll.s16      q12, d4, #15                  \n"// extend samples to 31 bits
        "vshll.s16      q13, d5, #15                  \n"// extend samples to 31 bits

        "vshll.s16      q14, d6, #15                  \n"// extend samples to 31 bits
        "vshll.s16      q15, d7, #15                  \n"// extend samples to 31 bits

        "vqrdmulh.s32   q12, q12, q9                  \n"// multiply samples
        "vqrdmulh.s32   q13, q13, q8                  \n"// multiply samples
        "vqrdmulh.s32   q14, q14, q10                 \n"// multiply samples
        "vqrdmulh.s32   q15, q15, q11                 \n"// multiply samples

        "vadd.s32       q0, q0, q12                   \n"// accumulate result
        "vadd.s32       q13, q13, q14                 \n"// accumulate result
        "vadd.s32       q0, q0, q15                   \n"// accumulate result
        "vadd.s32       q0, q0, q13                   \n"// accumulate result

        "sub            %[sP], %[sP], #16             \n"// move pointer to next set of samples
        "subs           %[count], %[count], #8        \n"// update loop counter

        "bne            1b                            \n"// loop

        ASSEMBLY_ACCUMULATE_MONO

        : [out]     "=Uv" (out[0]),
          [count]   "+r" (count),
          [coefsP0] "+r" (coefsP),
          [coefsN0] "+r" (coefsN),
          [sP]      "+r" (sP),
          [sN]      "+r" (sN)
        : [vLR]     "r" (volumeLR)
        : "cc", "memory",
          "q0", "q1", "q2", "q3",
          "q8", "q9", "q10", "q11",
          "q12", "q13", "q14", "q15"
    );
#endif
}

template <>
inline void ProcessL<2, 16>(int32_t* const out,
        int count,
        const int32_t* coefsP,
        const int32_t* coefsN,
        const int16_t* sP,
        const int16_t* sN,
        const int32_t* const volumeLR)
{
#ifdef USE_INTRINSIC
    ProcessNeonIntrinsic<2, 16, true>(out, count, coefsP, coefsN, sP, sN, volumeLR,
            0 /*lerpP*/, NULL /*coefsP1*/, NULL /*coefsN1*/);
#else
    const int CHANNELS = 2; // template specialization does not preserve params
    const int STRIDE = 16;
    sP -= CHANNELS*((STRIDE>>1)-1);
    asm (
        "veor           q0, q0, q0                    \n"// result, initialize to 0
        "veor           q4, q4, q4                    \n"// result, initialize to 0

        "1:                                           \n"

        "vld2.16        {q2, q3}, [%[sP]]             \n"// load 8 16-bits stereo frames
        "vld2.16        {q5, q6}, [%[sN]]!            \n"// load 8 16-bits stereo frames
        "vld1.32        {q8, q9}, [%[coefsP0]:128]!   \n"// load 8 32-bits coefs
        "vld1.32        {q10, q11}, [%[coefsN0]:128]! \n"// load 8 32-bits coefs

        "vrev64.16      q2, q2                        \n"// reverse 8 samples of positive left
        "vrev64.16      q3, q3                        \n"// reverse 8 samples of positive right

        "vshll.s16      q12,  d4, #15                 \n"// extend samples to 31 bits
        "vshll.s16      q13,  d5, #15                 \n"// extend samples to 31 bits

        "vshll.s16      q14,  d10, #15                \n"// extend samples to 31 bits
        "vshll.s16      q15,  d11, #15                \n"// extend samples to 31 bits

        "vqrdmulh.s32   q12, q12, q9                  \n"// multiply samples by coef
        "vqrdmulh.s32   q13, q13, q8                  \n"// multiply samples by coef
        "vqrdmulh.s32   q14, q14, q10                 \n"// multiply samples by coef
        "vqrdmulh.s32   q15, q15, q11                 \n"// multiply samples by coef

        "vadd.s32       q0, q0, q12                   \n"// accumulate result
        "vadd.s32       q13, q13, q14                 \n"// accumulate result
        "vadd.s32       q0, q0, q15                   \n"// accumulate result
        "vadd.s32       q0, q0, q13                   \n"// accumulate result

        "vshll.s16      q12,  d6, #15                 \n"// extend samples to 31 bits
        "vshll.s16      q13,  d7, #15                 \n"// extend samples to 31 bits

        "vshll.s16      q14,  d12, #15                \n"// extend samples to 31 bits
        "vshll.s16      q15,  d13, #15                \n"// extend samples to 31 bits

        "vqrdmulh.s32   q12, q12, q9                  \n"// multiply samples by coef
        "vqrdmulh.s32   q13, q13, q8                  \n"// multiply samples by coef
        "vqrdmulh.s32   q14, q14, q10                 \n"// multiply samples by coef
        "vqrdmulh.s32   q15, q15, q11                 \n"// multiply samples by coef

        "vadd.s32       q4, q4, q12                   \n"// accumulate result
        "vadd.s32       q13, q13, q14                 \n"// accumulate result
        "vadd.s32       q4, q4, q15                   \n"// accumulate result
        "vadd.s32       q4, q4, q13                   \n"// accumulate result

        "subs           %[count], %[count], #8        \n"// update loop counter
        "sub            %[sP], %[sP], #32             \n"// move pointer to next set of samples

        "bne            1b                            \n"// loop

        ASSEMBLY_ACCUMULATE_STEREO

        : [out]     "=Uv" (out[0]),
          [count]   "+r" (count),
          [coefsP0] "+r" (coefsP),
          [coefsN0] "+r" (coefsN),
          [sP]      "+r" (sP),
          [sN]      "+r" (sN)
        : [vLR]     "r" (volumeLR)
        : "cc", "memory",
          "q0", "q1", "q2", "q3",
          "q4", "q5", "q6",
          "q8", "q9", "q10", "q11",
          "q12", "q13", "q14", "q15"
    );
#endif
}

template <>
inline void Process<1, 16>(int32_t* const out,
        int count,
        const int32_t* coefsP,
        const int32_t* coefsN,
        const int32_t* coefsP1,
        const int32_t* coefsN1,
        const int16_t* sP,
        const int16_t* sN,
        uint32_t lerpP,
        const int32_t* const volumeLR)
{
#ifdef USE_INTRINSIC
    ProcessNeonIntrinsic<1, 16, false>(out, count, coefsP, coefsN, sP, sN, volumeLR,
            lerpP, coefsP1, coefsN1);
#else
    const int CHANNELS = 1; // template specialization does not preserve params
    const int STRIDE = 16;
    sP -= CHANNELS*((STRIDE>>1)-1);
    asm (
        "vmov.32        d2[0], %[lerpP]               \n"// load the positive phase
        "veor           q0, q0, q0                    \n"// result, initialize to 0

        "1:                                           \n"

        "vld1.16        {q2}, [%[sP]]                 \n"// load 8 16-bits mono samples
        "vld1.16        {q3}, [%[sN]]!                \n"// load 8 16-bits mono samples
        "vld1.32        {q8, q9}, [%[coefsP0]:128]!   \n"// load 8 32-bits coefs
        "vld1.32        {q12, q13}, [%[coefsP1]:128]! \n"// load 8 32-bits coefs
        "vld1.32        {q10, q11}, [%[coefsN1]:128]! \n"// load 8 32-bits coefs
        "vld1.32        {q14, q15}, [%[coefsN0]:128]! \n"// load 8 32-bits coefs

        "vsub.s32       q12, q12, q8                  \n"// interpolate (step1)
        "vsub.s32       q13, q13, q9                  \n"// interpolate (step1)
        "vsub.s32       q14, q14, q10                 \n"// interpolate (step1)
        "vsub.s32       q15, q15, q11                 \n"// interpolate (step1)

        "vqrdmulh.s32   q12, q12, d2[0]               \n"// interpolate (step2)
        "vqrdmulh.s32   q13, q13, d2[0]               \n"// interpolate (step2)
        "vqrdmulh.s32   q14, q14, d2[0]               \n"// interpolate (step2)
        "vqrdmulh.s32   q15, q15, d2[0]               \n"// interpolate (step2)

        "vadd.s32       q8, q8, q12                   \n"// interpolate (step3)
        "vadd.s32       q9, q9, q13                   \n"// interpolate (step3)
        "vadd.s32       q10, q10, q14                 \n"// interpolate (step3)
        "vadd.s32       q11, q11, q15                 \n"// interpolate (step3)

        "vrev64.16      q2, q2                        \n"// reverse 8 samples of the positive side

        "vshll.s16      q12,  d4, #15                 \n"// extend samples to 31 bits
        "vshll.s16      q13,  d5, #15                 \n"// extend samples to 31 bits

        "vshll.s16      q14,  d6, #15                 \n"// extend samples to 31 bits
        "vshll.s16      q15,  d7, #15                 \n"// extend samples to 31 bits

        "vqrdmulh.s32   q12, q12, q9                  \n"// multiply samples by interpolated coef
        "vqrdmulh.s32   q13, q13, q8                  \n"// multiply samples by interpolated coef
        "vqrdmulh.s32   q14, q14, q10                 \n"// multiply samples by interpolated coef
        "vqrdmulh.s32   q15, q15, q11                 \n"// multiply samples by interpolated coef

        "vadd.s32       q0, q0, q12                   \n"// accumulate result
        "vadd.s32       q13, q13, q14                 \n"// accumulate result
        "vadd.s32       q0, q0, q15                   \n"// accumulate result
        "vadd.s32       q0, q0, q13                   \n"// accumulate result

        "sub            %[sP], %[sP], #16             \n"// move pointer to next set of samples
        "subs           %[count], %[count], #8        \n"// update loop counter

        "bne            1b                            \n"// loop

        ASSEMBLY_ACCUMULATE_MONO

        : [out]     "=Uv" (out[0]),
          [count]   "+r" (count),
          [coefsP0] "+r" (coefsP),
          [coefsN0] "+r" (coefsN),
          [coefsP1] "+r" (coefsP1),
          [coefsN1] "+r" (coefsN1),
          [sP]      "+r" (sP),
          [sN]      "+r" (sN)
        : [lerpP]   "r" (lerpP),
          [vLR]     "r" (volumeLR)
        : "cc", "memory",
          "q0", "q1", "q2", "q3",
          "q8", "q9", "q10", "q11",
          "q12", "q13", "q14", "q15"
    );
#endif
}

template <>
inline void Process<2, 16>(int32_t* const out,
        int count,
        const int32_t* coefsP,
        const int32_t* coefsN,
        const int32_t* coefsP1,
        const int32_t* coefsN1,
        const int16_t* sP,
        const int16_t* sN,
        uint32_t lerpP,
        const int32_t* const volumeLR)
{
#ifdef USE_INTRINSIC
    ProcessNeonIntrinsic<2, 16, false>(out, count, coefsP, coefsN, sP, sN, volumeLR,
            lerpP, coefsP1, coefsN1);
#else
    const int CHANNELS = 2; // template specialization does not preserve params
    const int STRIDE = 16;
    sP -= CHANNELS*((STRIDE>>1)-1);
    asm (
        "vmov.32        d2[0], %[lerpP]               \n"// load the positive phase
        "veor           q0, q0, q0                    \n"// result, initialize to 0
        "veor           q4, q4, q4                    \n"// result, initialize to 0

        "1:                                           \n"

        "vld2.16        {q2, q3}, [%[sP]]             \n"// load 8 16-bits stereo frames
        "vld2.16        {q5, q6}, [%[sN]]!            \n"// load 8 16-bits stereo frames
        "vld1.32        {q8, q9}, [%[coefsP0]:128]!   \n"// load 8 32-bits coefs
        "vld1.32        {q12, q13}, [%[coefsP1]:128]! \n"// load 8 32-bits coefs
        "vld1.32        {q10, q11}, [%[coefsN1]:128]! \n"// load 8 32-bits coefs
        "vld1.32        {q14, q15}, [%[coefsN0]:128]! \n"// load 8 32-bits coefs

        "vsub.s32       q12, q12, q8                  \n"// interpolate (step1)
        "vsub.s32       q13, q13, q9                  \n"// interpolate (step1)
        "vsub.s32       q14, q14, q10                 \n"// interpolate (step1)
        "vsub.s32       q15, q15, q11                 \n"// interpolate (step1)

        "vqrdmulh.s32   q12, q12, d2[0]               \n"// interpolate (step2)
        "vqrdmulh.s32   q13, q13, d2[0]               \n"// interpolate (step2)
        "vqrdmulh.s32   q14, q14, d2[0]               \n"// interpolate (step2)
        "vqrdmulh.s32   q15, q15, d2[0]               \n"// interpolate (step2)

        "vadd.s32       q8, q8, q12                   \n"// interpolate (step3)
        "vadd.s32       q9, q9, q13                   \n"// interpolate (step3)
        "vadd.s32       q10, q10, q14                 \n"// interpolate (step3)
        "vadd.s32       q11, q11, q15                 \n"// interpolate (step3)

        "vrev64.16      q2, q2                        \n"// reverse 8 samples of positive left
        "vrev64.16      q3, q3                        \n"// reverse 8 samples of positive right

        "vshll.s16      q12,  d4, #15                 \n"// extend samples to 31 bits
        "vshll.s16      q13,  d5, #15                 \n"// extend samples to 31 bits

        "vshll.s16      q14,  d10, #15                \n"// extend samples to 31 bits
        "vshll.s16      q15,  d11, #15                \n"// extend samples to 31 bits

        "vqrdmulh.s32   q12, q12, q9                  \n"// multiply samples by interpolated coef
        "vqrdmulh.s32   q13, q13, q8                  \n"// multiply samples by interpolated coef
        "vqrdmulh.s32   q14, q14, q10                 \n"// multiply samples by interpolated coef
        "vqrdmulh.s32   q15, q15, q11                 \n"// multiply samples by interpolated coef

        "vadd.s32       q0, q0, q12                   \n"// accumulate result
        "vadd.s32       q13, q13, q14                 \n"// accumulate result
        "vadd.s32       q0, q0, q15                   \n"// accumulate result
        "vadd.s32       q0, q0, q13                   \n"// accumulate result

        "vshll.s16      q12,  d6, #15                 \n"// extend samples to 31 bits
        "vshll.s16      q13,  d7, #15                 \n"// extend samples to 31 bits

        "vshll.s16      q14,  d12, #15                \n"// extend samples to 31 bits
        "vshll.s16      q15,  d13, #15                \n"// extend samples to 31 bits

        "vqrdmulh.s32   q12, q12, q9                  \n"// multiply samples by interpolated coef
        "vqrdmulh.s32   q13, q13, q8                  \n"// multiply samples by interpolated coef
        "vqrdmulh.s32   q14, q14, q10                 \n"// multiply samples by interpolated coef
        "vqrdmulh.s32   q15, q15, q11                 \n"// multiply samples by interpolated coef

        "vadd.s32       q4, q4, q12                   \n"// accumulate result
        "vadd.s32       q13, q13, q14                 \n"// accumulate result
        "vadd.s32       q4, q4, q15                   \n"// accumulate result
        "vadd.s32       q4, q4, q13                   \n"// accumulate result

        "subs           %[count], %[count], #8        \n"// update loop counter
        "sub            %[sP], %[sP], #32             \n"// move pointer to next set of samples

        "bne            1b                            \n"// loop

        ASSEMBLY_ACCUMULATE_STEREO

        : [out]     "=Uv" (out[0]),
          [count]   "+r" (count),
          [coefsP0] "+r" (coefsP),
          [coefsN0] "+r" (coefsN),
          [coefsP1] "+r" (coefsP1),
          [coefsN1] "+r" (coefsN1),
          [sP]      "+r" (sP),
          [sN]      "+r" (sN)
        : [lerpP]   "r" (lerpP),
          [vLR]     "r" (volumeLR)
        : "cc", "memory",
          "q0", "q1", "q2", "q3",
          "q4", "q5", "q6",
          "q8", "q9", "q10", "q11",
          "q12", "q13", "q14", "q15"
    );
#endif
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
    ProcessNeonIntrinsic<1, 16, true>(out, count, coefsP, coefsN, sP, sN, volumeLR,
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
    ProcessNeonIntrinsic<2, 16, true>(out, count, coefsP, coefsN, sP, sN, volumeLR,
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
    ProcessNeonIntrinsic<1, 16, false>(out, count, coefsP, coefsN, sP, sN, volumeLR,
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
    ProcessNeonIntrinsic<2, 16, false>(out, count, coefsP, coefsN, sP, sN, volumeLR,
            lerpP, coefsP1, coefsN1);
}

#endif //USE_NEON

} // namespace android

#endif /*ANDROID_AUDIO_RESAMPLER_FIR_PROCESS_NEON_H*/
