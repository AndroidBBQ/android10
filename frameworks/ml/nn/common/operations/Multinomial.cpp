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

#include "Multinomial.h"

#include "CpuExecutor.h"
#include "CpuOperationUtils.h"
#include "HalInterfaces.h"
#include "Tracing.h"

#include "guarded_philox_random.h"
#include "philox_random.h"
#include "simple_philox.h"

#include "unsupported/Eigen/CXX11/Tensor"

namespace android {
namespace nn {

namespace {

template <typename T>
inline T* GetBuffer(RunTimeOperandInfo* operand) {
    return reinterpret_cast<T*>(operand->buffer);
}

template <typename T>
inline const T* GetBuffer(const RunTimeOperandInfo* operand) {
    return reinterpret_cast<const T*>(operand->buffer);
}

}  // namespace

Multinomial::Multinomial(const Operation& operation, std::vector<RunTimeOperandInfo>& operands) {
    NNTRACE_TRANS("Multinomial::Multinomial");
    input_ = GetInput(operation, operands, kInputTensor);
    sample_count_ = getScalarData<int>(*GetInput(operation, operands, kSampleCountParam));
    random_seeds_ = GetInput(operation, operands, kRandomSeedsTensor);

    output_ = GetOutput(operation, operands, kOutputTensor);
}

bool Multinomial::Prepare(const Operation& operation, std::vector<RunTimeOperandInfo>& operands,
                          Shape* outputShape) {
    NNTRACE_TRANS("Multinomial::Prepare");
    NN_CHECK_EQ(NumInputsWithValues(operation, operands), 3);
    NN_CHECK_EQ(NumOutputs(operation), 1);

    const RunTimeOperandInfo* input = GetInput(operation, operands, Multinomial::kInputTensor);
    const Shape& inputShape = input->shape();

    const uint32_t batch_size = SizeOfDimension(input, 0);
    const uint32_t sample_count =
            getScalarData<int>(*GetInput(operation, operands, kSampleCountParam));

    outputShape->type = OperandType::TENSOR_INT32;
    outputShape->dimensions = {batch_size, sample_count};
    outputShape->offset = inputShape.offset;
    outputShape->scale = inputShape.scale;

    return true;
}

bool Multinomial::Eval() {
    NNTRACE_COMP("Multinomial::Eval");
    switch (input_->type) {
        case OperandType::TENSOR_FLOAT16: {
            std::vector<float> inputDataFloat32(getNumberOfElements(input_->shape()));
            convertFloat16ToFloat32(GetBuffer<_Float16>(input_), &inputDataFloat32);
            EvalFloat32(inputDataFloat32.data());
            break;
        }
        case OperandType::TENSOR_FLOAT32: {
            EvalFloat32(GetBuffer<float>(input_));
            break;
        }
        default: {
            LOG(ERROR) << "Unsupported data type: " << static_cast<int>(input_->type);
            return false;
        }
    }
    return true;
}

void Multinomial::EvalFloat32(const float* inputData) {
    const int batch_size = SizeOfDimension(input_, 0);
    const int class_size = SizeOfDimension(input_, 1);

    tensorflow::GuardedPhiloxRandom random_generator;
    int32_t* seeds = GetBuffer<int32_t>(random_seeds_);
    random_generator.Init(seeds[0], seeds[1]);

    // PhiloxRandom produces results as 4 32-bit integers.
    int sample_count_aligned = (sample_count_ + 3) / 4 * 4;
    // The CPU operation uses 64-bit double values, so two results per sample.
    sample_count_aligned *= 2;
    auto random_generator_reserved =
            random_generator.ReserveRandomOutputs(batch_size * sample_count_aligned, 256);
    tensorflow::random::SimplePhilox simple_philox(&random_generator_reserved);

    for (uint64_t b = 0; b < batch_size; ++b) {
        const float* input_ptr_batch = inputData + b * class_size;
        float max = std::numeric_limits<float>::lowest();
        for (uint64_t j = 0; j < class_size; ++j) {
            if (Eigen::numext::isfinite(input_ptr_batch[j])) {
                max = std::max(max, input_ptr_batch[j]);
            }
        }
        const double batch_max = static_cast<double>(max);
        double total = 0;
        std::vector<double> cdf;
        cdf.resize(class_size);
        for (uint64_t j = 0; j < class_size; ++j) {
            if (Eigen::numext::isfinite(static_cast<float>(input_ptr_batch[j]))) {
                total += exp(static_cast<double>(input_ptr_batch[j]) - batch_max);
            }
            cdf[j] = total;
        }

        auto* output_ptr_batch = GetBuffer<int32_t>(output_) + b * sample_count_;
        for (uint64_t j = 0; j < sample_count_; ++j) {
            const double target = simple_philox.RandDouble() * total;
            auto found_iter = std::upper_bound(cdf.begin(), cdf.end(), target);
            output_ptr_batch[j] = std::distance(cdf.begin(), found_iter);
        }
    }
}

}  // namespace nn
}  // namespace android
