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
//#define LOG_NDEBUG 0

#include <cmath>

#include "common/core/math.h"
#include "common/core/types.h"
#include "dsp/core/basic.h"
#include "dsp/core/interpolation.h"
#include "dsp/core/dynamic_range_compression.h"

#include <android/log.h>

namespace le_fx {

// Definitions for static const class members declared in
// dynamic_range_compression.h.
const float AdaptiveDynamicRangeCompression::kMinAbsValue = 0.000001f;
const float AdaptiveDynamicRangeCompression::kMinLogAbsValue =
    0.032766999999999997517097227728299912996590137481689453125f;
const float AdaptiveDynamicRangeCompression::kFixedPointLimit = 32767.0f;
const float AdaptiveDynamicRangeCompression::kInverseFixedPointLimit =
    1.0f / AdaptiveDynamicRangeCompression::kFixedPointLimit;
const float AdaptiveDynamicRangeCompression::kDefaultKneeThresholdInDecibel =
    -8.0f;
const float AdaptiveDynamicRangeCompression::kCompressionRatio = 7.0f;
const float AdaptiveDynamicRangeCompression::kTauAttack = 0.001f;
const float AdaptiveDynamicRangeCompression::kTauRelease = 0.015f;

AdaptiveDynamicRangeCompression::AdaptiveDynamicRangeCompression() {
  static const float kTargetGain[] = {
      1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
  static const float kKneeThreshold[] = {
      -8.0f, -8.0f, -8.5f, -9.0f, -10.0f };
  target_gain_to_knee_threshold_.Initialize(
      &kTargetGain[0], &kKneeThreshold[0],
      sizeof(kTargetGain) / sizeof(kTargetGain[0]));
}

bool AdaptiveDynamicRangeCompression::Initialize(
        float target_gain, float sampling_rate) {
  set_knee_threshold_via_target_gain(target_gain);
  sampling_rate_ = sampling_rate;
  state_ = 0.0f;
  compressor_gain_ = 1.0f;
  if (kTauAttack > 0.0f) {
    const float taufs = kTauAttack * sampling_rate_;
    alpha_attack_ = std::exp(-1.0f / taufs);
  } else {
    alpha_attack_ = 0.0f;
  }
  if (kTauRelease > 0.0f) {
    const float taufs = kTauRelease * sampling_rate_;
    alpha_release_ = std::exp(-1.0f / taufs);
  } else {
    alpha_release_ = 0.0f;
  }
  // Feed-forward topology
  slope_ = 1.0f / kCompressionRatio - 1.0f;
  return true;
}

float AdaptiveDynamicRangeCompression::Compress(float x) {
  const float max_abs_x = std::max(std::fabs(x), kMinLogAbsValue);
  const float max_abs_x_dB = math::fast_log(max_abs_x);
  // Subtract Threshold from log-encoded input to get the amount of overshoot
  const float overshoot = max_abs_x_dB - knee_threshold_;
  // Hard half-wave rectifier
  const float rect = std::max(overshoot, 0.0f);
  // Multiply rectified overshoot with slope
  const float cv = rect * slope_;
  const float prev_state = state_;
  if (cv <= state_) {
    state_ = alpha_attack_ * state_ + (1.0f - alpha_attack_) * cv;
  } else {
    state_ = alpha_release_ * state_ + (1.0f - alpha_release_) * cv;
  }
  compressor_gain_ *=
      math::ExpApproximationViaTaylorExpansionOrder5(state_ - prev_state);
  x *= compressor_gain_;
  if (x > kFixedPointLimit) {
    return kFixedPointLimit;
  }
  if (x < -kFixedPointLimit) {
    return -kFixedPointLimit;
  }
  return x;
}

void AdaptiveDynamicRangeCompression::Compress(float *x1, float *x2) {
  // Taking the maximum amplitude of both channels
  const float max_abs_x = std::max(std::fabs(*x1),
    std::max(std::fabs(*x2), kMinLogAbsValue));
  const float max_abs_x_dB = math::fast_log(max_abs_x);
  // Subtract Threshold from log-encoded input to get the amount of overshoot
  const float overshoot = max_abs_x_dB - knee_threshold_;
  // Hard half-wave rectifier
  const float rect = std::max(overshoot, 0.0f);
  // Multiply rectified overshoot with slope
  const float cv = rect * slope_;
  const float prev_state = state_;
  if (cv <= state_) {
    state_ = alpha_attack_ * state_ + (1.0f - alpha_attack_) * cv;
  } else {
    state_ = alpha_release_ * state_ + (1.0f - alpha_release_) * cv;
  }
  compressor_gain_ *=
      math::ExpApproximationViaTaylorExpansionOrder5(state_ - prev_state);
  *x1 *= compressor_gain_;
  if (*x1 > kFixedPointLimit) {
    *x1 = kFixedPointLimit;
  }
  if (*x1 < -kFixedPointLimit) {
    *x1 = -kFixedPointLimit;
  }
  *x2 *= compressor_gain_;
  if (*x2 > kFixedPointLimit) {
    *x2 = kFixedPointLimit;
  }
  if (*x2 < -kFixedPointLimit) {
    *x2 = -kFixedPointLimit;
  }
}

}  // namespace le_fx

