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

#include "HalInterfaces.h"
#include "NeuralNetworksWrapper.h"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"
#include "philox_random.h"
#include "simple_philox.h"

#include "unsupported/Eigen/CXX11/Tensor"

namespace android {
namespace nn {
namespace wrapper {

using ::testing::FloatNear;

constexpr int kFixedRandomSeed1 = 37;
constexpr int kFixedRandomSeed2 = 42;

class MultinomialOpModel {
   public:
    MultinomialOpModel(uint32_t batch_size, uint32_t class_size, uint32_t sample_size)
        : batch_size_(batch_size), class_size_(class_size), sample_size_(sample_size) {
        std::vector<uint32_t> inputs;
        OperandType logitsType(Type::TENSOR_FLOAT32, {batch_size_, class_size_});
        inputs.push_back(model_.addOperand(&logitsType));
        OperandType samplesType(Type::INT32, {});
        inputs.push_back(model_.addOperand(&samplesType));
        OperandType seedsType(Type::TENSOR_INT32, {2});
        inputs.push_back(model_.addOperand(&seedsType));

        std::vector<uint32_t> outputs;
        OperandType outputType(Type::TENSOR_INT32, {batch_size_, sample_size_});
        outputs.push_back(model_.addOperand(&outputType));

        model_.addOperation(ANEURALNETWORKS_RANDOM_MULTINOMIAL, inputs, outputs);
        model_.identifyInputsAndOutputs(inputs, outputs);
        model_.finish();
    }

    void Invoke() {
        ASSERT_TRUE(model_.isValid());

        Compilation compilation(&model_);
        compilation.finish();
        Execution execution(&compilation);

        tensorflow::random::PhiloxRandom rng(kFixedRandomSeed1);
        tensorflow::random::SimplePhilox srng(&rng);
        const int sample_count = batch_size_ * class_size_;
        for (int i = 0; i < sample_count; ++i) {
            input_.push_back(srng.RandDouble());
        }
        ASSERT_EQ(execution.setInput(Multinomial::kInputTensor, input_.data(),
                                     sizeof(float) * input_.size()),
                  Result::NO_ERROR);
        ASSERT_EQ(execution.setInput(Multinomial::kSampleCountParam, &sample_size_,
                                     sizeof(sample_size_)),
                  Result::NO_ERROR);

        std::vector<uint32_t> seeds{kFixedRandomSeed1, kFixedRandomSeed2};
        ASSERT_EQ(execution.setInput(Multinomial::kRandomSeedsTensor, seeds.data(),
                                     sizeof(uint32_t) * seeds.size()),
                  Result::NO_ERROR);

        output_.insert(output_.end(), batch_size_ * sample_size_, 0);
        ASSERT_EQ(execution.setOutput(Multinomial::kOutputTensor, output_.data(),
                                      sizeof(uint32_t) * output_.size()),
                  Result::NO_ERROR);

        ASSERT_EQ(execution.compute(), Result::NO_ERROR);
    }

    const std::vector<float>& GetInput() const { return input_; }
    const std::vector<uint32_t>& GetOutput() const { return output_; }

   private:
    Model model_;

    const uint32_t batch_size_;
    const uint32_t class_size_;
    const uint32_t sample_size_;

    std::vector<float> input_;
    std::vector<uint32_t> output_;
};

TEST(MultinomialOpTest, ProbabilityDeltaWithinTolerance) {
    constexpr int kBatchSize = 8;
    constexpr int kNumClasses = 10000;
    constexpr int kNumSamples = 128;
    constexpr float kMaxProbabilityDelta = 0.025;

    MultinomialOpModel multinomial(kBatchSize, kNumClasses, kNumSamples);
    multinomial.Invoke();

    std::vector<uint32_t> output = multinomial.GetOutput();
    std::vector<int> class_counts;
    class_counts.resize(kNumClasses);
    for (auto index : output) {
        class_counts[index]++;
    }

    std::vector<float> input = multinomial.GetInput();
    for (int b = 0; b < kBatchSize; ++b) {
        float probability_sum = 0;
        const int batch_index = kBatchSize * b;
        for (int i = 0; i < kNumClasses; ++i) {
            probability_sum += expf(input[batch_index + i]);
        }
        for (int i = 0; i < kNumClasses; ++i) {
            float probability =
                    static_cast<float>(class_counts[i]) / static_cast<float>(kNumSamples);
            float probability_expected = expf(input[batch_index + i]) / probability_sum;
            EXPECT_THAT(probability, FloatNear(probability_expected, kMaxProbabilityDelta));
        }
    }
}

}  // namespace wrapper
}  // namespace nn
}  // namespace android
