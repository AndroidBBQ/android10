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

#include "EmbeddingLookup.h"

#include "NeuralNetworksWrapper.h"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"

using ::testing::FloatNear;
using ::testing::Matcher;

namespace android {
namespace nn {
namespace wrapper {

namespace {

std::vector<Matcher<float>> ArrayFloatNear(const std::vector<float>& values,
                                           float max_abs_error=1.e-6) {
  std::vector<Matcher<float>> matchers;
  matchers.reserve(values.size());
  for (const float& v : values) {
    matchers.emplace_back(FloatNear(v, max_abs_error));
  }
  return matchers;
}

}  // namespace

using ::testing::ElementsAreArray;

#define FOR_ALL_INPUT_AND_WEIGHT_TENSORS(ACTION) \
  ACTION(Value, float)                           \
  ACTION(Lookup, int)

// For all output and intermediate states
#define FOR_ALL_OUTPUT_TENSORS(ACTION) \
  ACTION(Output, float)

class EmbeddingLookupOpModel {
 public:
  EmbeddingLookupOpModel(std::initializer_list<uint32_t> index_shape,
                         std::initializer_list<uint32_t> weight_shape) {
    auto it = weight_shape.begin();
    rows_ = *it++;
    columns_ = *it++;
    features_ = *it;

    std::vector<uint32_t> inputs;

    OperandType LookupTy(Type::TENSOR_INT32, index_shape);
    inputs.push_back(model_.addOperand(&LookupTy));

    OperandType ValueTy(Type::TENSOR_FLOAT32, weight_shape);
    inputs.push_back(model_.addOperand(&ValueTy));

    std::vector<uint32_t> outputs;

    OperandType OutputOpndTy(Type::TENSOR_FLOAT32, weight_shape);
    outputs.push_back(model_.addOperand(&OutputOpndTy));

    auto multiAll = [](const std::vector<uint32_t> &dims) -> uint32_t {
        uint32_t sz = 1;
        for (uint32_t d : dims) { sz *= d; }
        return sz;
    };

    Value_.insert(Value_.end(), multiAll(weight_shape), 0.f);
    Output_.insert(Output_.end(), multiAll(weight_shape), 0.f);

    model_.addOperation(ANEURALNETWORKS_EMBEDDING_LOOKUP, inputs, outputs);
    model_.identifyInputsAndOutputs(inputs, outputs);

    model_.finish();
  }

  void Invoke() {
    ASSERT_TRUE(model_.isValid());

    Compilation compilation(&model_);
    compilation.finish();
    Execution execution(&compilation);

#define SetInputOrWeight(X, T)                                               \
  ASSERT_EQ(execution.setInput(EmbeddingLookup::k##X##Tensor, X##_.data(),   \
                               sizeof(T) * X##_.size()),                     \
            Result::NO_ERROR);

    FOR_ALL_INPUT_AND_WEIGHT_TENSORS(SetInputOrWeight);

#undef SetInputOrWeight

#define SetOutput(X, T)                                                       \
  ASSERT_EQ(execution.setOutput(EmbeddingLookup::k##X##Tensor, X##_.data(),   \
                                sizeof(T) * X##_.size()),                     \
            Result::NO_ERROR);

    FOR_ALL_OUTPUT_TENSORS(SetOutput);

#undef SetOutput

    ASSERT_EQ(execution.compute(), Result::NO_ERROR);
  }

#define DefineSetter(X, T)                       \
  void Set##X(const std::vector<T>& f) {         \
    X##_.insert(X##_.end(), f.begin(), f.end()); \
  }

  FOR_ALL_INPUT_AND_WEIGHT_TENSORS(DefineSetter);

#undef DefineSetter

  void Set3DWeightMatrix(const std::function<float(int, int, int)>& function) {
    for (uint32_t i = 0; i < rows_; i++) {
      for (uint32_t j = 0; j < columns_; j++) {
        for (uint32_t k = 0; k < features_; k++) {
          Value_[(i * columns_ + j) * features_ + k] = function(i, j, k);
        }
      }
    }
  }

  const std::vector<float> &GetOutput() const { return Output_; }

 private:
  Model model_;
  uint32_t rows_;
  uint32_t columns_;
  uint32_t features_;

#define DefineTensor(X, T) std::vector<T> X##_;

  FOR_ALL_INPUT_AND_WEIGHT_TENSORS(DefineTensor);
  FOR_ALL_OUTPUT_TENSORS(DefineTensor);

#undef DefineTensor
};

// TODO: write more tests that exercise the details of the op, such as
// lookup errors and variable input shapes.
TEST(EmbeddingLookupOpTest, SimpleTest) {
  EmbeddingLookupOpModel m({3}, {3, 2, 4});
  m.SetLookup({1, 0, 2});
  m.Set3DWeightMatrix(
      [](int i, int j, int k) { return i + j / 10.0f + k / 100.0f; });

  m.Invoke();

  EXPECT_THAT(m.GetOutput(),
              ElementsAreArray(ArrayFloatNear({
                  1.00, 1.01, 1.02, 1.03, 1.10, 1.11, 1.12, 1.13,  // Row 1
                  0.00, 0.01, 0.02, 0.03, 0.10, 0.11, 0.12, 0.13,  // Row 0
                  2.00, 2.01, 2.02, 2.03, 2.10, 2.11, 2.12, 2.13,  // Row 2
              })));
}

}  // namespace wrapper
}  // namespace nn
}  // namespace android
