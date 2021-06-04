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
#ifndef LE_FX_ENGINE_DSP_CORE_DYNAMIC_RANGE_COMPRESSION_H_
#define LE_FX_ENGINE_DSP_CORE_DYNAMIC_RANGE_COMPRESSION_H_

//#define LOG_NDEBUG 0

#include "common/core/types.h"
#include "common/core/math.h"
#include "dsp/core/basic.h"
#include "dsp/core/interpolation.h"

#include <android/log.h>

namespace le_fx {

// An adaptive dynamic range compression algorithm. The gain adaptation is made
// at the logarithmic domain and it is based on a Branching-Smooth compensated
// digital peak detector with different time constants for attack and release.
class AdaptiveDynamicRangeCompression {
 public:
    AdaptiveDynamicRangeCompression();

    // Initializes the compressor using prior information. It assumes that the
    // input signal is speech from high-quality recordings that is scaled and then
    // fed to the compressor. The compressor is tuned according to the target gain
    // that is expected to be applied.
    //
    // Target gain receives values between 0.0 and 10.0. The knee threshold is
    // reduced as the target gain increases in order to fit the increased range of
    // values.
    //
    // Values between 1.0 and 2.0 will only mildly affect your signal. Higher
    // values will reduce the dynamic range of the signal to the benefit of
    // increased loudness.
    //
    // If nothing is known regarding the input, a `target_gain` of 1.0f is a
    // relatively safe choice for many signals.
    bool Initialize(float target_gain, float sampling_rate);

  // A fast version of the algorithm that uses approximate computations for the
  // log(.) and exp(.).
  float Compress(float x);

  // Stereo channel version of the compressor
  void Compress(float *x1, float *x2);

  // This version is slower than Compress(.) but faster than CompressSlow(.)
  float CompressNormalSpeed(float x);

  // A slow version of the algorithm that is easier for further developement,
  // tuning and debugging
  float CompressSlow(float x);

  // Sets knee threshold (in decibel).
  void set_knee_threshold(float decibel);

  // Sets knee threshold via the target gain using an experimentally derived
  // relationship.
  void set_knee_threshold_via_target_gain(float target_gain);

 private:
  // The minimum accepted absolute input value and it's natural logarithm. This
  // is to prevent numerical issues when the input is close to zero
  static const float kMinAbsValue;
  static const float kMinLogAbsValue;
  // Fixed-point arithmetic limits
  static const float kFixedPointLimit;
  static const float kInverseFixedPointLimit;
  // The default knee threshold in decibel. The knee threshold defines when the
  // compressor is actually starting to compress the value of the input samples
  static const float kDefaultKneeThresholdInDecibel;
  // The compression ratio is the reciprocal of the slope of the line segment
  // above the threshold (in the log-domain). The ratio controls the
  // effectiveness of the compression.
  static const float kCompressionRatio;
  // The attack time of the envelope detector
  static const float kTauAttack;
  // The release time of the envelope detector
  static const float kTauRelease;

  float sampling_rate_;
  // the internal state of the envelope detector
  float state_;
  // the latest gain factor that was applied to the input signal
  float compressor_gain_;
  // attack constant for exponential dumping
  float alpha_attack_;
  // release constant for exponential dumping
  float alpha_release_;
  float slope_;
  // The knee threshold
  float knee_threshold_;
  float knee_threshold_in_decibel_;
  // This interpolator provides the function that relates target gain to knee
  // threshold.
  sigmod::InterpolatorLinear<float> target_gain_to_knee_threshold_;

  LE_FX_DISALLOW_COPY_AND_ASSIGN(AdaptiveDynamicRangeCompression);
};

}  // namespace le_fx

#include "dsp/core/dynamic_range_compression-inl.h"

#endif  // LE_FX_ENGINE_DSP_CORE_DYNAMIC_RANGE_COMPRESSION_H_
