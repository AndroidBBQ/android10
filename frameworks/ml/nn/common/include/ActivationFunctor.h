/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef ANDROID_ML_NN_ACTIVATION_FUNCTOR_H
#define ANDROID_ML_NN_ACTIVATION_FUNCTOR_H

#include "android/log.h"

#include <algorithm>
#include <cmath>

enum ActivationFn {
    kActivationNone = 0,
    kActivationRelu,
    kActivationRelu1,
    kActivationRelu6,
    kActivationTanh,
    kActivationSignBit,
    kActivationSigmoid,
};

class ActivationFunctor {
 public:
  explicit ActivationFunctor(ActivationFn act) : act_(act) {}

  float operator()(float a) const {
    switch (act_) {
      case kActivationNone:
        return a;
      case kActivationRelu:
        return a < 0.f ? 0.f : a;
      case kActivationRelu6:
        return std::max(0.f, std::min(a, 6.f));
      case kActivationTanh:
        return std::tanh(a);
      case kActivationSigmoid:
        return 1.0f / (1.0f + std::exp(-a));
      default:
        __android_log_print(ANDROID_LOG_ERROR, "NN API",
                            "Invalid enum value for activation function: 0x%0X",
                            act_);
        abort();
    }
  }

 private:
  ActivationFn act_;
};

#endif  // ANDROID_ML_NN_ACTIVATION_FUNCTOR_H
