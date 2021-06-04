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

#ifndef ANDROID_AUDIO_RESAMPLER_FIR_OPS_H
#define ANDROID_AUDIO_RESAMPLER_FIR_OPS_H

namespace android {

#if defined(__arm__) && !defined(__thumb__)
#define USE_INLINE_ASSEMBLY (true)
#else
#define USE_INLINE_ASSEMBLY (false)
#endif

#if defined(__aarch64__) || defined(__ARM_NEON__)
#ifndef USE_NEON
#define USE_NEON (true)
#endif
#else
#define USE_NEON (false)
#endif
#if USE_NEON
#include <arm_neon.h>
#endif

#if defined(__SSSE3__)  // Should be supported in x86 ABI for both 32 & 64-bit.
#define USE_SSE (true)
#include <tmmintrin.h>
#else
#define USE_SSE (false)
#endif

template<typename T, typename U>
struct is_same
{
    static const bool value = false;
};

template<typename T>
struct is_same<T, T>  // partial specialization
{
    static const bool value = true;
};

static inline
int32_t mulRL(int left, int32_t in, uint32_t vRL)
{
#if USE_INLINE_ASSEMBLY
    int32_t out;
    if (left) {
        asm( "smultb %[out], %[in], %[vRL] \n"
             : [out]"=r"(out)
             : [in]"%r"(in), [vRL]"r"(vRL)
             : );
    } else {
        asm( "smultt %[out], %[in], %[vRL] \n"
             : [out]"=r"(out)
             : [in]"%r"(in), [vRL]"r"(vRL)
             : );
    }
    return out;
#else
    int16_t v = left ? static_cast<int16_t>(vRL) : static_cast<int16_t>(vRL>>16);
    return static_cast<int32_t>((static_cast<int64_t>(in) * v) >> 16);
#endif
}

static inline
int32_t mulAdd(int16_t in, int16_t v, int32_t a)
{
#if USE_INLINE_ASSEMBLY
    int32_t out;
    asm( "smlabb %[out], %[v], %[in], %[a] \n"
         : [out]"=r"(out)
         : [in]"%r"(in), [v]"r"(v), [a]"r"(a)
         : );
    return out;
#else
    return a + v * in;
#endif
}

static inline
int32_t mulAdd(int16_t in, int32_t v, int32_t a)
{
#if USE_INLINE_ASSEMBLY
    int32_t out;
    asm( "smlawb %[out], %[v], %[in], %[a] \n"
         : [out]"=r"(out)
         : [in]"%r"(in), [v]"r"(v), [a]"r"(a)
         : );
    return out;
#else
    return a + static_cast<int32_t>((static_cast<int64_t>(v) * in) >> 16);
#endif
}

static inline
int32_t mulAdd(int32_t in, int32_t v, int32_t a)
{
#if USE_INLINE_ASSEMBLY
    int32_t out;
    asm( "smmla %[out], %[v], %[in], %[a] \n"
         : [out]"=r"(out)
         : [in]"%r"(in), [v]"r"(v), [a]"r"(a)
         : );
    return out;
#else
    return a + static_cast<int32_t>((static_cast<int64_t>(v) * in) >> 32);
#endif
}

static inline
int32_t mulAddRL(int left, uint32_t inRL, int16_t v, int32_t a)
{
#if 0 // USE_INLINE_ASSEMBLY Seems to fail with Clang b/34110890
    int32_t out;
    if (left) {
        asm( "smlabb %[out], %[v], %[inRL], %[a] \n"
             : [out]"=r"(out)
             : [inRL]"%r"(inRL), [v]"r"(v), [a]"r"(a)
             : );
    } else {
        asm( "smlabt %[out], %[v], %[inRL], %[a] \n"
             : [out]"=r"(out)
             : [inRL]"%r"(inRL), [v]"r"(v), [a]"r"(a)
             : );
    }
    return out;
#else
    int16_t s = left ? static_cast<int16_t>(inRL) : static_cast<int16_t>(inRL>>16);
    return a + v * s;
#endif
}

static inline
int32_t mulAddRL(int left, uint32_t inRL, int32_t v, int32_t a)
{
#if 0 // USE_INLINE_ASSEMBLY Seems to fail with Clang b/34110890
    int32_t out;
    if (left) {
        asm( "smlawb %[out], %[v], %[inRL], %[a] \n"
             : [out]"=r"(out)
             : [inRL]"%r"(inRL), [v]"r"(v), [a]"r"(a)
             : );
    } else {
        asm( "smlawt %[out], %[v], %[inRL], %[a] \n"
             : [out]"=r"(out)
             : [inRL]"%r"(inRL), [v]"r"(v), [a]"r"(a)
             : );
    }
    return out;
#else
    int16_t s = left ? static_cast<int16_t>(inRL) : static_cast<int16_t>(inRL>>16);
    return a + static_cast<int32_t>((static_cast<int64_t>(v) * s) >> 16);
#endif
}

} // namespace android

#endif /*ANDROID_AUDIO_RESAMPLER_FIR_OPS_H*/
