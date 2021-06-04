/*
 * Copyright (C) 2012 The Android Open Source Project
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

#define LOG_TAG "ResamplerCoefficients"
//#define LOG_NDEBUG 0

#include <utils/Log.h>

#include "filter_coefficients.h"

const int32_t RESAMPLE_FIR_NUM_COEF = 16;
const int32_t RESAMPLE_FIR_LERP_INT_BITS = 7;

using namespace android;

#ifdef __cplusplus
extern "C" {
#endif

const int32_t* readResamplerCoefficients(bool upSample) {

    ALOGV("readResamplerCoefficients");
    if (upSample) {
        return (const int32_t *) up_sampler_filter_coefficients;
    } else {
        return (const int32_t *) dn_sampler_filter_coefficients;
    }

}

int32_t readResampleFirNumCoeff() {
    return RESAMPLE_FIR_NUM_COEF;
}

int32_t readResampleFirLerpIntBits() {
    return RESAMPLE_FIR_LERP_INT_BITS;
}

#ifdef __cplusplus
}
#endif
