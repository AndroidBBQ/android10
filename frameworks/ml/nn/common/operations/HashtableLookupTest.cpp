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

#include "HashtableLookup.h"

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

#define FOR_ALL_INPUT_AND_WEIGHT_TENSORS(ACTION)     \
  ACTION(Lookup, int)                                \
  ACTION(Key, int)                                   \
  ACTION(Value, float)

// For all output and intermediate states
#define FOR_ALL_OUTPUT_TENSORS(ACTION) \
  ACTION(Output, float)                \
  ACTION(Hits, uint8_t)

class HashtableLookupOpModel {
 public:
    HashtableLookupOpModel(std::initializer_list<uint32_t> lookup_shape,
                           std::initializer_list<uint32_t> key_shape,
                           std::initializer_list<uint32_t> value_shape) {
    auto it_vs = value_shape.begin();
    rows_ = *it_vs++;
    features_ = *it_vs;

    std::vector<uint32_t> inputs;

    // Input and weights
    OperandType LookupTy(Type::TENSOR_INT32, lookup_shape);
    inputs.push_back(model_.addOperand(&LookupTy));

    OperandType KeyTy(Type::TENSOR_INT32, key_shape);
    inputs.push_back(model_.addOperand(&KeyTy));

    OperandType ValueTy(Type::TENSOR_FLOAT32, value_shape);
    inputs.push_back(model_.addOperand(&ValueTy));

    // Output and other intermediate state
    std::vector<uint32_t> outputs;

    std::vector<uint32_t> out_dim(lookup_shape.begin(), lookup_shape.end());
    out_dim.push_back(features_);

    OperandType OutputOpndTy(Type::TENSOR_FLOAT32, out_dim);
    outputs.push_back(model_.addOperand(&OutputOpndTy));

    OperandType HitsOpndTy(Type::TENSOR_QUANT8_ASYMM, lookup_shape, 1.f, 0);
    outputs.push_back(model_.addOperand(&HitsOpndTy));

    auto multiAll = [](const std::vector<uint32_t> &dims) -> uint32_t {
        uint32_t sz = 1;
        for (uint32_t d : dims) { sz *= d; }
        return sz;
    };

    Value_.insert(Value_.end(), multiAll(value_shape), 0.f);
    Output_.insert(Output_.end(), multiAll(out_dim), 0.f);
    Hits_.insert(Hits_.end(), multiAll(lookup_shape), 0);

    model_.addOperation(ANEURALNETWORKS_HASHTABLE_LOOKUP, inputs, outputs);
    model_.identifyInputsAndOutputs(inputs, outputs);

    model_.finish();
  }

  void Invoke() {
    ASSERT_TRUE(model_.isValid());

    Compilation compilation(&model_);
    compilation.finish();
    Execution execution(&compilation);

#define SetInputOrWeight(X, T)                                             \
  ASSERT_EQ(execution.setInput(HashtableLookup::k##X##Tensor, X##_.data(), \
                               sizeof(T) * X##_.size()),                   \
            Result::NO_ERROR);

    FOR_ALL_INPUT_AND_WEIGHT_TENSORS(SetInputOrWeight);

#undef SetInputOrWeight

#define SetOutput(X, T)                                                     \
  ASSERT_EQ(execution.setOutput(HashtableLookup::k##X##Tensor, X##_.data(), \
                               sizeof(T) * X##_.size()),                    \
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

  void SetHashtableValue(const std::function<float(uint32_t, uint32_t)>& function) {
    for (uint32_t i = 0; i < rows_; i++) {
      for (uint32_t j = 0; j < features_; j++) {
          Value_[i * features_ + j] = function(i, j);
      }
    }
  }

  const std::vector<float>& GetOutput() const { return Output_; }
  const std::vector<uint8_t>& GetHits() const { return Hits_; }

 private:
  Model model_;
  uint32_t rows_;
  uint32_t features_;

#define DefineTensor(X, T) std::vector<T> X##_;

  FOR_ALL_INPUT_AND_WEIGHT_TENSORS(DefineTensor);
  FOR_ALL_OUTPUT_TENSORS(DefineTensor);

#undef DefineTensor
};

TEST(HashtableLookupOpTest, BlackBoxTest) {
  HashtableLookupOpModel m({4}, {3}, {3, 2});

  m.SetLookup({1234, -292, -11, 0});
  m.SetKey({-11, 0, 1234});
  m.SetHashtableValue([](int i, int j) { return i + j / 10.0f; });

  m.Invoke();

  EXPECT_THAT(m.GetOutput(), ElementsAreArray(ArrayFloatNear({
                                 2.0, 2.1,  // 2-rd item
                                 0, 0,      // Not found
                                 0.0, 0.1,  // 0-th item
                                 1.0, 1.1,  // 1-st item
                             })));
  EXPECT_EQ(m.GetHits(), std::vector<uint8_t>({
                               1, 0, 1, 1,
                           }));

}

}  // namespace wrapper
}  // namespace nn
}  // namespace android
