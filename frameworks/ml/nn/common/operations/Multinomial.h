/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef FRAMEWORKS_ML_NN_MULTINOMIAL_H
#define FRAMEWORKS_ML_NN_MULTINOMIAL_H

#include "HalOperation.h"

#include "tensorflow/lite/kernels/internal/tensor_utils.h"

#include <algorithm>
#include <cmath>

namespace android {
namespace nn {

struct RunTimeOperandInfo;
struct Shape;

class Multinomial {
   public:
    Multinomial(const android::hardware::neuralnetworks::V1_2::Operation& operation,
                std::vector<RunTimeOperandInfo>& operands);

    static bool Prepare(const hardware::neuralnetworks::V1_2::Operation& operation,
                        std::vector<RunTimeOperandInfo>& operands, Shape* outputShape);
    bool Eval();

    static constexpr int kInputTensor = 0;
    static constexpr int kSampleCountParam = 1;
    static constexpr int kRandomSeedsTensor = 2;

    static constexpr int kOutputTensor = 0;

   private:
    void EvalFloat32(const float* inputData);

    RunTimeOperandInfo* input_;
    int sample_count_;
    RunTimeOperandInfo* random_seeds_;

    RunTimeOperandInfo* output_;
};

}  // namespace nn
}  // namespace android

#endif  // FRAMEWORKS_ML_NN_MULTINOMIAL_H
