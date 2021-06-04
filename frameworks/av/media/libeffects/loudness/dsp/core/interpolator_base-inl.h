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

#ifndef LE_FX_ENGINE_DSP_CORE_INTERPOLATOR_BASE_INL_H_
#define LE_FX_ENGINE_DSP_CORE_INTERPOLATOR_BASE_INL_H_
#ifndef LOG_TAG
#define LOG_TAG NULL
#endif
//#define LOG_NDEBUG 0

#include <log/log.h>

#include "dsp/core/basic.h"

namespace le_fx {

namespace sigmod {

template <typename T, class Algorithm>
InterpolatorBase<T, Algorithm>::InterpolatorBase() {
  status_ = false;
  cached_index_ = 0;
  x_data_ = NULL;
  y_data_ = NULL;
  data_length_ = 0;
  own_x_data_ = false;
  x_start_offset_ = 0.0;
  last_element_index_ = -1;
  x_inverse_sampling_interval_ = 0.0;
  state_ = NULL;
}

template <typename T, class Algorithm>
InterpolatorBase<T, Algorithm>::~InterpolatorBase() {
  delete [] state_;
  if (own_x_data_) {
    delete [] x_data_;
  }
}

template <typename T, class Algorithm>
bool InterpolatorBase<T, Algorithm>::Initialize(const vector<T> &x_data,
                                                const vector<T> &y_data) {
#ifndef NDEBUG
  if (x_data.size() != y_data.size()) {
    LoggerError("InterpolatorBase::Initialize: xData size (%d) != yData size"
                  " (%d)", x_data.size(), y_data.size());
  }
#endif
  return Initialize(&x_data[0], &y_data[0], x_data.size());
}

template <typename T, class Algorithm>
bool InterpolatorBase<T, Algorithm>::Initialize(double x_start_offset,
                                                double x_sampling_interval,
                                                const vector<T> &y_data) {
  return Initialize(x_start_offset,
                    x_sampling_interval,
                    &y_data[0],
                    y_data.size());
}

template <typename T, class Algorithm>
bool InterpolatorBase<T, Algorithm>::Initialize(double x_start_offset,
                                                double x_sampling_interval,
                                                const T *y_data,
                                                int data_length) {
  // Constructs and populate x-axis data: `x_data_`
  T *x_data_tmp = new T[data_length];
  float time_offset = x_start_offset;
  for (int n = 0; n < data_length; n++) {
    x_data_tmp[n] = time_offset;
    time_offset += x_sampling_interval;
  }
  Initialize(x_data_tmp, y_data, data_length);
  // Sets-up the regularly sampled interpolation mode
  x_start_offset_ = x_start_offset;
  x_inverse_sampling_interval_ = 1.0 / x_sampling_interval;
  own_x_data_ = true;
  return status_;
}


template <typename T, class Algorithm>
bool InterpolatorBase<T, Algorithm>::Initialize(
    const T *x_data, const T *y_data, int data_length) {
  // Default settings
  cached_index_ = 0;
  data_length_ = 0;
  x_start_offset_ = 0;
  x_inverse_sampling_interval_ = 0;
  state_ = NULL;
  // Input data is externally owned
  own_x_data_ = false;
  x_data_ = x_data;
  y_data_ = y_data;
  data_length_ = data_length;
  last_element_index_ = data_length - 1;
  // Check input data sanity
  for (int n = 0; n < last_element_index_; ++n) {
    if (x_data_[n + 1] <= x_data_[n]) {
      ALOGE("InterpolatorBase::Initialize: xData are not ordered or "
              "contain equal values (X[%d] <= X[%d]) (%.5e <= %.5e)",
            n + 1, n, x_data_[n + 1], x_data_[n]);
      status_ = false;
      return false;
    }
  }
  // Pre-compute internal state by calling the corresponding function of the
  // derived class.
  status_ = static_cast<Algorithm*>(this)->SetInternalState();
  return status_;
}

template <typename T, class Algorithm>
T InterpolatorBase<T, Algorithm>::Interpolate(T x) {
#ifndef NDEBUG
  if (cached_index_ < 0 || cached_index_ > data_length_ - 2) {
    LoggerError("InterpolatorBase:Interpolate: CachedIndex_ out of bounds "
                  "[0, %d, %d]", cached_index_, data_length_ - 2);
  }
#endif
  // Search for the containing interval
  if (x <= x_data_[cached_index_]) {
    if (cached_index_ <= 0) {
      cached_index_ = 0;
      return y_data_[0];
    }
    if (x >= x_data_[cached_index_ - 1]) {
      cached_index_--;  // Fast descending
    } else {
      if (x <= x_data_[0]) {
        cached_index_ = 0;
        return y_data_[0];
      }
      cached_index_ = SearchIndex(x_data_, x, 0, cached_index_);
    }
  } else {
    if (cached_index_ >= last_element_index_) {
      cached_index_ = last_element_index_;
      return y_data_[last_element_index_];
    }
    if (x > x_data_[cached_index_ + 1]) {
      if (cached_index_ + 2 > last_element_index_) {
        cached_index_ = last_element_index_ - 1;
        return y_data_[last_element_index_];
      }
      if (x <= x_data_[cached_index_ + 2]) {
        cached_index_++;  // Fast ascending
      } else {
        if (x >= x_data_[last_element_index_]) {
          cached_index_ = last_element_index_ - 1;
          return y_data_[last_element_index_];
        }
        cached_index_ = SearchIndex(
            x_data_, x, cached_index_, last_element_index_);
      }
    }
  }
  // Compute interpolated value by calling the corresponding function of the
  // derived class.
  return static_cast<Algorithm*>(this)->MethodSpecificInterpolation(x);
}

}  // namespace sigmod

}  // namespace le_fx

#endif  // LE_FX_ENGINE_DSP_CORE_INTERPOLATOR_BASE_INL_H_
