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

#include "LSHProjection.h"

#include "NeuralNetworksWrapper.h"
#include "gmock/gmock-generated-matchers.h"
#include "gmock/gmock-matchers.h"
#include "gtest/gtest.h"

using ::testing::FloatNear;
using ::testing::Matcher;

namespace android {
namespace nn {
namespace wrapper {

using ::testing::ElementsAre;

#define FOR_ALL_INPUT_AND_WEIGHT_TENSORS(ACTION) \
    ACTION(Hash, float)                          \
    ACTION(Input, int)                           \
    ACTION(Weight, float)

// For all output and intermediate states
#define FOR_ALL_OUTPUT_TENSORS(ACTION) ACTION(Output, int)

class LSHProjectionOpModel {
   public:
    LSHProjectionOpModel(LSHProjectionType type, std::initializer_list<uint32_t> hash_shape,
                         std::initializer_list<uint32_t> input_shape,
                         std::initializer_list<uint32_t> weight_shape)
        : type_(type) {
        std::vector<uint32_t> inputs;

        OperandType HashTy(Type::TENSOR_FLOAT32, hash_shape);
        inputs.push_back(model_.addOperand(&HashTy));
        OperandType InputTy(Type::TENSOR_INT32, input_shape);
        inputs.push_back(model_.addOperand(&InputTy));
        OperandType WeightTy(Type::TENSOR_FLOAT32, weight_shape);
        inputs.push_back(model_.addOperand(&WeightTy));

        OperandType TypeParamTy(Type::INT32, {});
        inputs.push_back(model_.addOperand(&TypeParamTy));

        std::vector<uint32_t> outputs;

        auto multiAll = [](const std::vector<uint32_t>& dims) -> uint32_t {
            uint32_t sz = 1;
            for (uint32_t d : dims) {
                sz *= d;
            }
            return sz;
        };

        uint32_t outShapeDimension = 0;
        if (type == LSHProjectionType_SPARSE || type == LSHProjectionType_SPARSE_DEPRECATED) {
            auto it = hash_shape.begin();
            Output_.insert(Output_.end(), *it, 0.f);
            outShapeDimension = *it;
        } else {
            Output_.insert(Output_.end(), multiAll(hash_shape), 0.f);
            outShapeDimension = multiAll(hash_shape);
        }

        OperandType OutputTy(Type::TENSOR_INT32, {outShapeDimension});
        outputs.push_back(model_.addOperand(&OutputTy));

        model_.addOperation(ANEURALNETWORKS_LSH_PROJECTION, inputs, outputs);
        model_.identifyInputsAndOutputs(inputs, outputs);

        model_.finish();
    }

#define DefineSetter(X, T) \
    void Set##X(const std::vector<T>& f) { X##_.insert(X##_.end(), f.begin(), f.end()); }

    FOR_ALL_INPUT_AND_WEIGHT_TENSORS(DefineSetter);

#undef DefineSetter

    const std::vector<int>& GetOutput() const { return Output_; }

    void Invoke() {
        ASSERT_TRUE(model_.isValid());

        Compilation compilation(&model_);
        compilation.finish();
        Execution execution(&compilation);

#define SetInputOrWeight(X, T)                                                                     \
    ASSERT_EQ(                                                                                     \
            execution.setInput(LSHProjection::k##X##Tensor, X##_.data(), sizeof(T) * X##_.size()), \
            Result::NO_ERROR);

        FOR_ALL_INPUT_AND_WEIGHT_TENSORS(SetInputOrWeight);

#undef SetInputOrWeight

#define SetOutput(X, T)                                                     \
    ASSERT_EQ(execution.setOutput(LSHProjection::k##X##Tensor, X##_.data(), \
                                  sizeof(T) * X##_.size()),                 \
              Result::NO_ERROR);

        FOR_ALL_OUTPUT_TENSORS(SetOutput);

#undef SetOutput

        ASSERT_EQ(execution.setInput(LSHProjection::kTypeParam, &type_, sizeof(type_)),
                  Result::NO_ERROR);

        ASSERT_EQ(execution.compute(), Result::NO_ERROR);
    }

   private:
    Model model_;
    LSHProjectionType type_;

    std::vector<float> Hash_;
    std::vector<int> Input_;
    std::vector<float> Weight_;
    std::vector<int> Output_;
};  // namespace wrapper

TEST(LSHProjectionOpTest2, DenseWithThreeInputs) {
    LSHProjectionOpModel m(LSHProjectionType_DENSE, {4, 2}, {3, 2}, {3});

    m.SetInput({12345, 54321, 67890, 9876, -12345678, -87654321});
    m.SetHash({0.123, 0.456, -0.321, -0.654, 1.234, 5.678, -4.321, -8.765});
    m.SetWeight({0.12, 0.34, 0.56});

    m.Invoke();

    EXPECT_THAT(m.GetOutput(), ElementsAre(1, 1, 1, 0, 1, 1, 1, 0));
}

TEST(LSHProjectionOpTest2, SparseDeprecatedWithTwoInputs) {
    LSHProjectionOpModel m(LSHProjectionType_SPARSE_DEPRECATED, {4, 2}, {3, 2}, {0});

    m.SetInput({12345, 54321, 67890, 9876, -12345678, -87654321});
    m.SetHash({0.123, 0.456, -0.321, -0.654, 1.234, 5.678, -4.321, -8.765});

    m.Invoke();

    EXPECT_THAT(m.GetOutput(), ElementsAre(1, 2, 2, 0));
}

TEST(LSHProjectionOpTest2, SparseWithTwoInputs) {
    LSHProjectionOpModel m(LSHProjectionType_SPARSE, {4, 2}, {3, 2}, {0});

    m.SetInput({12345, 54321, 67890, 9876, -12345678, -87654321});
    m.SetHash({0.123, 0.456, -0.321, -0.654, 1.234, 5.678, -4.321, -8.765});

    m.Invoke();

    EXPECT_THAT(m.GetOutput(), ElementsAre(1, 6, 10, 12));
}

}  // namespace wrapper
}  // namespace nn
}  // namespace android
