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
#ifndef LE_FX_ENGINE_DSP_CORE_DYNAMIC_RANGE_COMPRESSION_INL_H_
#define LE_FX_ENGINE_DSP_CORE_DYNAMIC_RANGE_COMPRESSION_INL_H_

#ifndef LOG_TAG
#define LOG_TAG NULL
#endif
//#define LOG_NDEBUG 0

#include <log/log.h>

namespace le_fx {

inline void AdaptiveDynamicRangeCompression::set_knee_threshold(float decibel) {
  // Converts to 1og-base
  knee_threshold_in_decibel_ = decibel;
  knee_threshold_ = 0.1151292546497023061569109358970308676362037658691406250f *
      decibel + 10.39717719035538401328722102334722876548767089843750f;
}


inline void AdaptiveDynamicRangeCompression::set_knee_threshold_via_target_gain(
    float target_gain) {
  const float decibel = target_gain_to_knee_threshold_.Interpolate(
        target_gain);
  ALOGV("set_knee_threshold_via_target_gain: decibel =%.3fdB", decibel);
  set_knee_threshold(decibel);
}

}  // namespace le_fx


#endif  // LE_FX_ENGINE_DSP_CORE_DYNAMIC_RANGE_COMPRESSION_INL_H_
