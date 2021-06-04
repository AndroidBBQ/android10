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

#ifndef LE_FX_ENGINE_DSP_CORE_INTERPOLATOR_BASE_H_
#define LE_FX_ENGINE_DSP_CORE_INTERPOLATOR_BASE_H_

#include "common/core/types.h"

namespace le_fx {

namespace sigmod {

// Interpolation base-class that provides the interface, while it is the derived
// class that provides the specific interpolation algorithm. The following list
// of interpolation algorithms are currently present:
//
// InterpolationSine<T>: weighted interpolation between y_data[n] and
//                       y_data[n+1] using a sin(.) weighting factor from
//                       0 to pi/4.
// InterpolationLinear<T>: linear interpolation
// InterpolationSplines<T>: spline-based interpolation
//
// Example (using derived spline-based interpolation class):
//  InterpolatorSplines<float> interp(x_data, y_data, data_length);
//  for (int n = 0; n < data_length; n++) Y[n] = interp.Interpolate(X[n]);
//
template <typename T, class Algorithm>
class InterpolatorBase {
 public:
  InterpolatorBase();
  ~InterpolatorBase();

  // Generic random-access interpolation with arbitrary spaced x-axis samples.
  // Below X[0], the interpolator returns Y[0]. Above X[data_length-1], it
  // returns Y[data_length-1].
  T Interpolate(T x);

  bool get_status() const {
    return status_;
  }

  // Initializes internal buffers.
  //  x_data: [(data_length)x1] x-axis coordinates (searching axis)
  //  y_data: [(data_length)x1] y-axis coordinates (interpolation axis)
  //  data_length: number of points
  // returns `true` if everything is ok, `false`, otherwise
  bool Initialize(const T *x_data, const T *y_data, int data_length);

  // Initializes internal buffers.
  //  x_data: x-axis coordinates (searching axis)
  //  y_data: y-axis coordinates (interpolating axis)
  // returns `true` if everything is ok, `false`, otherwise
  bool Initialize(const vector<T> &x_data, const vector<T> &y_data);

  // Initialization for regularly sampled sequences, where:
  //  x_data[i] = x_start_offset + i * x_sampling_interval
  bool Initialize(double x_start_offset,
                  double x_sampling_interval,
                  const vector<T> &y_data);

  // Initialization for regularly sampled sequences, where:
  //  x_data[i] = x_start_offset + i * x_sampling_interval
  bool Initialize(double x_start_offset,
                  double x_sampling_interval,
                  const T *y_data,
                  int data_length);

 protected:
  // Is set to false if something goes wrong, and to true if everything is ok.
  bool status_;

  // The start-index of the previously searched interval
  int cached_index_;

  // Data points
  const T *x_data_;  // Externally or internally owned, depending on own_x_data_
  const T *y_data_;  // Externally owned (always)
  int data_length_;
  // Index of the last element `data_length_ - 1` kept here for optimization
  int last_element_index_;
  bool own_x_data_;
  // For regularly-samples sequences, keep only the boundaries and the intervals
  T x_start_offset_;
  float x_inverse_sampling_interval_;

  // Algorithm state (internally owned)
  double *state_;

 private:
  LE_FX_DISALLOW_COPY_AND_ASSIGN(InterpolatorBase);
};

}  // namespace sigmod

}  // namespace le_fx

#include "dsp/core/interpolator_base-inl.h"

#endif  // LE_FX_ENGINE_DSP_CORE_INTERPOLATOR_BASE_H_
