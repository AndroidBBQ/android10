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

#ifndef LE_FX_ENGINE_DSP_CORE_INTERPOLATOR_LINEAR_H_
#define LE_FX_ENGINE_DSP_CORE_INTERPOLATOR_LINEAR_H_

#include <math.h>
#include "dsp/core/interpolator_base.h"

namespace le_fx {

namespace sigmod {

// Linear interpolation class.
//
// The main functionality of this class is provided by it's base-class, so
// please refer to: InterpolatorBase
//
// Example:
//  InterpolatorLinear<float> interp(x_data, y_data, data_length);
//  for (int n = 0; n < data_length; n++) Y[n] = interp.Interpolate(X[n]);
//
template <typename T>
class InterpolatorLinear: public InterpolatorBase<T, InterpolatorLinear<T> > {
 public:
  InterpolatorLinear() { }
  ~InterpolatorLinear() { }

 protected:
  // Provides the main implementation of the linear interpolation algorithm.
  // Assumes that: X[cached_index_] < x < X[cached_index_ + 1]
  T MethodSpecificInterpolation(T x);

  // Pre-compute internal state_ parameters.
  bool SetInternalState();

 private:
  friend class InterpolatorBase<T, InterpolatorLinear<T> >;
  typedef InterpolatorBase<T, InterpolatorLinear<T> > BaseClass;
  using BaseClass::status_;
  using BaseClass::cached_index_;
  using BaseClass::x_data_;
  using BaseClass::y_data_;
  using BaseClass::data_length_;
  using BaseClass::state_;

  LE_FX_DISALLOW_COPY_AND_ASSIGN(InterpolatorLinear<T>);
};

template <typename T>
inline T InterpolatorLinear<T>::MethodSpecificInterpolation(T x) {
  T dX = x_data_[cached_index_ + 1] - x_data_[cached_index_];
  T dY = y_data_[cached_index_ + 1] - y_data_[cached_index_];
  T dx = x - x_data_[cached_index_];
  return y_data_[cached_index_] + (dY * dx) / dX;
}

template <typename T>
bool InterpolatorLinear<T>::SetInternalState() {
  state_ = NULL;
  return true;
}

}  // namespace sigmod

}  // namespace le_fx

#endif  // LE_FX_ENGINE_DSP_CORE_INTERPOLATOR_LINEAR_H_
