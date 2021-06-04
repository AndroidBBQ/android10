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

#include "SVDF.h"

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

static float svdf_input[] = {0.12609188,  -0.46347019, -0.89598465,
                             0.12609188,  -0.46347019, -0.89598465,

                             0.14278367,  -1.64410412, -0.75222826,
                             0.14278367,  -1.64410412, -0.75222826,

                             0.49837467,  0.19278903,  0.26584083,
                             0.49837467,  0.19278903,  0.26584083,

                             -0.11186574, 0.13164264,  -0.05349274,
                             -0.11186574, 0.13164264,  -0.05349274,

                             -0.68892461, 0.37783599,  0.18263303,
                             -0.68892461, 0.37783599,  0.18263303,

                             -0.81299269, -0.86831826, 1.43940818,
                             -0.81299269, -0.86831826, 1.43940818,

                             -1.45006323, -0.82251364, -1.69082689,
                             -1.45006323, -0.82251364, -1.69082689,

                             0.03966608,  -0.24936394, -0.77526885,
                             0.03966608,  -0.24936394, -0.77526885,

                             0.11771342,  -0.23761693, -0.65898693,
                             0.11771342,  -0.23761693, -0.65898693,

                             -0.89477462, 1.67204106,  -0.53235275,
                             -0.89477462, 1.67204106,  -0.53235275};

static float svdf_input_rank2[] = {
    0.12609188,  -0.46347019, -0.89598465,
    0.35867718,  0.36897406,  0.73463392,

    0.14278367,  -1.64410412, -0.75222826,
    -0.57290924, 0.12729003,  0.7567004,

    0.49837467,  0.19278903,  0.26584083,
    0.17660543,  0.52949083,  -0.77931279,

    -0.11186574, 0.13164264,  -0.05349274,
    -0.72674477, -0.5683046,  0.55900657,

    -0.68892461, 0.37783599,  0.18263303,
    -0.63690937, 0.44483393,  -0.71817774,

    -0.81299269, -0.86831826, 1.43940818,
    -0.95760226, 1.82078898,  0.71135032,

    -1.45006323, -0.82251364, -1.69082689,
    -1.65087092, -1.89238167, 1.54172635,

    0.03966608,  -0.24936394, -0.77526885,
    2.06740379,  -1.51439476, 1.43768692,

    0.11771342,  -0.23761693, -0.65898693,
    0.31088525,  -1.55601168, -0.87661445,

    -0.89477462, 1.67204106,  -0.53235275,
    -0.6230064,  0.29819036,  1.06939757,
};

static float svdf_golden_output[] = {
    0.014899,    -0.0517661, -0.143725, -0.00271883,
    0.014899,    -0.0517661, -0.143725, -0.00271883,

    0.068281,    -0.162217,  -0.152268, 0.00323521,
    0.068281,    -0.162217,  -0.152268, 0.00323521,

    -0.0317821,  -0.0333089, 0.0609602, 0.0333759,
    -0.0317821,  -0.0333089, 0.0609602, 0.0333759,

    -0.00623099, -0.077701,  -0.391193, -0.0136691,
    -0.00623099, -0.077701,  -0.391193, -0.0136691,

    0.201551,    -0.164607,  -0.179462, -0.0592739,
    0.201551,    -0.164607,  -0.179462, -0.0592739,

    0.0886511,   -0.0875401, -0.269283, 0.0281379,
    0.0886511,   -0.0875401, -0.269283, 0.0281379,

    -0.201174,   -0.586145,  -0.628624, -0.0330412,
    -0.201174,   -0.586145,  -0.628624, -0.0330412,

    -0.0839096,  -0.299329,  0.108746,  0.109808,
    -0.0839096,  -0.299329,  0.108746,  0.109808,

    0.419114,    -0.237824,  -0.422627, 0.175115,
    0.419114,    -0.237824,  -0.422627, 0.175115,

    0.36726,     -0.522303,  -0.456502, -0.175475,
    0.36726,     -0.522303,  -0.456502, -0.175475};

static float svdf_golden_output_rank_2[] = {
    -0.09623547, -0.10193135, 0.11083051,  -0.0347917,
    0.1141196,   0.12965347,  -0.12652366, 0.01007236,

    -0.16396809, -0.21247184, 0.11259045,  -0.04156673,
    0.10132131,  -0.06143532, -0.00924693, 0.10084561,

    0.01257364,  0.0506071,   -0.19287863, -0.07162561,
    -0.02033747, 0.22673416,  0.15487903,  0.02525555,

    -0.1411963,  -0.37054959, 0.01774767,  0.05867489,
    0.09607603,  -0.0141301,  -0.08995658, 0.12867066,

    -0.27142537, -0.16955489, 0.18521598,  -0.12528358,
    0.00331409,  0.11167502,  0.02218599,  -0.07309391,

    0.09593632,  -0.28361851, -0.0773851,  0.17199151,
    -0.00075242, 0.33691186,  -0.1536046,  0.16572715,

    -0.27916506, -0.27626723, 0.42615682,  0.3225764,
    -0.37472126, -0.55655634, -0.05013514, 0.289112,

    -0.24418658, 0.07540751,  -0.1940318,  -0.08911639,
    0.00732617,  0.46737891,  0.26449674,  0.24888524,

    -0.17225097, -0.54660404, -0.38795233, 0.08389944,
    0.07736043,  -0.28260678, 0.15666828,  1.14949894,

    -0.57454878, -0.64704704, 0.73235172,  -0.34616736,
    0.21120001,  -0.22927976, 0.02455296,  -0.35906726,
};

#define FOR_ALL_INPUT_AND_WEIGHT_TENSORS(ACTION) \
  ACTION(Input)                                  \
  ACTION(WeightsFeature)                         \
  ACTION(WeightsTime)                            \
  ACTION(Bias)                                   \
  ACTION(StateIn)

// For all output and intermediate states
#define FOR_ALL_OUTPUT_TENSORS(ACTION) \
  ACTION(StateOut)                     \
  ACTION(Output)

// Derived class of SingleOpModel, which is used to test SVDF TFLite op.
class SVDFOpModel {
 public:
  SVDFOpModel(uint32_t batches, uint32_t units, uint32_t input_size,
              uint32_t memory_size, uint32_t rank)
      : batches_(batches),
        units_(units),
        input_size_(input_size),
        memory_size_(memory_size),
        rank_(rank) {
    std::vector<std::vector<uint32_t>> input_shapes{
        {batches_, input_size_},  // Input tensor
        {units_ * rank_, input_size_},    // weights_feature tensor
        {units_ * rank_, memory_size_},   // weights_time tensor
        {units_},                  // bias tensor
        {batches_,  memory_size * units_ * rank_},   // state in tensor
    };
    std::vector<uint32_t> inputs;
    auto it = input_shapes.begin();

    // Input and weights
#define AddInput(X)                                   \
  OperandType X##OpndTy(Type::TENSOR_FLOAT32, *it++); \
  inputs.push_back(model_.addOperand(&X##OpndTy));

    FOR_ALL_INPUT_AND_WEIGHT_TENSORS(AddInput);

#undef AddInput

    // Parameters
    OperandType RankParamTy(Type::INT32, {});
    inputs.push_back(model_.addOperand(&RankParamTy));
    OperandType ActivationParamTy(Type::INT32, {});
    inputs.push_back(model_.addOperand(&ActivationParamTy));

    // Output and other intermediate state
    std::vector<std::vector<uint32_t>> output_shapes{{batches_, memory_size_ * units_ * rank_},
                                                     {batches_, units_}};
    std::vector<uint32_t> outputs;

    auto it2 = output_shapes.begin();

#define AddOutput(X)                                   \
  OperandType X##OpndTy(Type::TENSOR_FLOAT32, *it2++); \
  outputs.push_back(model_.addOperand(&X##OpndTy));

    FOR_ALL_OUTPUT_TENSORS(AddOutput);

#undef AddOutput

    Input_.insert(Input_.end(), batches_ * input_size_, 0.f);
    StateIn_.insert(StateIn_.end(), batches_ * units_ * rank_ * memory_size_, 0.f);

    auto multiAll = [](const std::vector<uint32_t> &dims) -> uint32_t {
        uint32_t sz = 1;
        for(uint32_t d:dims) { sz *= d; }
        return sz;
    };

    it2 = output_shapes.begin();

#define ReserveOutput(X) X##_.insert(X##_.end(), multiAll(*it2++), 0.f);

    FOR_ALL_OUTPUT_TENSORS(ReserveOutput);

    model_.addOperation(ANEURALNETWORKS_SVDF, inputs, outputs);
    model_.identifyInputsAndOutputs(inputs, outputs);

    model_.finish();
  }

  void Invoke() {
    ASSERT_TRUE(model_.isValid());

    Compilation compilation(&model_);
    compilation.finish();
    Execution execution(&compilation);

    StateIn_.swap(StateOut_);

#define SetInputOrWeight(X)                                                    \
  ASSERT_EQ(execution.setInput(SVDF::k##X##Tensor, X##_.data(),                \
                               sizeof(float) * X##_.size()),                   \
            Result::NO_ERROR);

    FOR_ALL_INPUT_AND_WEIGHT_TENSORS(SetInputOrWeight);

#undef SetInputOrWeight

#define SetOutput(X)                                                            \
  EXPECT_TRUE(X##_.data() != nullptr);                                          \
  ASSERT_EQ(execution.setOutput(SVDF::k##X##Tensor, X##_.data(),                \
                                sizeof(float) * X##_.size()),                   \
            Result::NO_ERROR);

    FOR_ALL_OUTPUT_TENSORS(SetOutput);

#undef SetOutput

    ASSERT_EQ(execution.setInput(SVDF::kRankParam, &rank_, sizeof(rank_)),
              Result::NO_ERROR);

    int activation = TfLiteFusedActivation::kTfLiteActNone;
    ASSERT_EQ(execution.setInput(SVDF::kActivationParam, &activation,
                                 sizeof(activation)),
              Result::NO_ERROR);

    ASSERT_EQ(execution.compute(), Result::NO_ERROR);
  }

#define DefineSetter(X)                          \
  void Set##X(const std::vector<float>& f) {     \
    X##_.insert(X##_.end(), f.begin(), f.end()); \
  }

  FOR_ALL_INPUT_AND_WEIGHT_TENSORS(DefineSetter);

#undef DefineSetter

  void SetInput(int offset, float* begin, float* end) {
    for (; begin != end; begin++, offset++) {
      Input_[offset] = *begin;
    }
  }

  // Resets the state of SVDF op by filling it with 0's.
  void ResetState() {
      std::fill(StateIn_.begin(), StateIn_.end(), 0.f);
      std::fill(StateOut_.begin(), StateOut_.end(), 0.f);
  }

  // Extracts the output tensor from the SVDF op.
  const std::vector<float>& GetOutput() const { return Output_; }

  int input_size() const { return input_size_; }
  int num_units() const { return units_; }
  int num_batches() const { return batches_; }

 private:
  Model model_;

  const uint32_t batches_;
  const uint32_t units_;
  const uint32_t input_size_;
  const uint32_t memory_size_;
  const uint32_t rank_;

#define DefineTensor(X) std::vector<float> X##_;

  FOR_ALL_INPUT_AND_WEIGHT_TENSORS(DefineTensor);
  FOR_ALL_OUTPUT_TENSORS(DefineTensor);

#undef DefineTensor
};

TEST(SVDFOpTest, BlackBoxTest) {
  SVDFOpModel svdf(/*batches=*/2, /*units=*/4, /*input_size=*/3,
                   /*memory_size=*/10, /*rank=*/1);
  svdf.SetWeightsFeature({-0.31930989, -0.36118156, 0.0079667, 0.37613347,
                          0.22197971, 0.12416199, 0.27901134, 0.27557442,
                          0.3905206, -0.36137494, -0.06634006, -0.10640851});

  svdf.SetWeightsTime(
      {-0.31930989, 0.37613347,  0.27901134,  -0.36137494, -0.36118156,
       0.22197971,  0.27557442,  -0.06634006, 0.0079667,   0.12416199,

       0.3905206,   -0.10640851, -0.0976817,  0.15294972,  0.39635518,
       -0.02702999, 0.39296314,  0.15785322,  0.21931258,  0.31053296,

       -0.36916667, 0.38031587,  -0.21580373, 0.27072677,  0.23622236,
       0.34936687,  0.18174365,  0.35907319,  -0.17493086, 0.324846,

       -0.10781813, 0.27201805,  0.14324132,  -0.23681851, -0.27115166,
       -0.01580888, -0.14943552, 0.15465137,  0.09784451,  -0.0337657});

  svdf.SetBias({});

  svdf.ResetState();
  const int svdf_num_batches = svdf.num_batches();
  const int svdf_input_size = svdf.input_size();
  const int svdf_num_units = svdf.num_units();
  const int input_sequence_size =
      sizeof(svdf_input) / sizeof(float) / (svdf_input_size * svdf_num_batches);
  // Going over each input batch, setting the input tensor, invoking the SVDF op
  // and checking the output with the expected golden values.
  for (int i = 0; i < input_sequence_size; i++) {
    float* batch_start = svdf_input + i * svdf_input_size * svdf_num_batches;
    float* batch_end = batch_start + svdf_input_size * svdf_num_batches;
    svdf.SetInput(0, batch_start, batch_end);

    svdf.Invoke();

    float* golden_start =
        svdf_golden_output + i * svdf_num_units * svdf_num_batches;
    float* golden_end = golden_start + svdf_num_units * svdf_num_batches;
    std::vector<float> expected;
    expected.insert(expected.end(), golden_start, golden_end);

    EXPECT_THAT(svdf.GetOutput(), ElementsAreArray(ArrayFloatNear(expected)));
  }
}

TEST(SVDFOpTest, BlackBoxTestRank2) {
  SVDFOpModel svdf(/*batches=*/2, /*units=*/4, /*input_size=*/3,
                   /*memory_size=*/10, /*rank=*/2);
  svdf.SetWeightsFeature({-0.31930989, 0.0079667,   0.39296314,  0.37613347,
                          0.12416199,  0.15785322,  0.27901134,  0.3905206,
                          0.21931258,  -0.36137494, -0.10640851, 0.31053296,
                          -0.36118156, -0.0976817,  -0.36916667, 0.22197971,
                          0.15294972,  0.38031587,  0.27557442,  0.39635518,
                          -0.21580373, -0.06634006, -0.02702999, 0.27072677});

  svdf.SetWeightsTime(
      {-0.31930989, 0.37613347,  0.27901134,  -0.36137494, -0.36118156,
       0.22197971,  0.27557442,  -0.06634006, 0.0079667,   0.12416199,

       0.3905206,   -0.10640851, -0.0976817,  0.15294972,  0.39635518,
       -0.02702999, 0.39296314,  0.15785322,  0.21931258,  0.31053296,

       -0.36916667, 0.38031587,  -0.21580373, 0.27072677,  0.23622236,
       0.34936687,  0.18174365,  0.35907319,  -0.17493086, 0.324846,

       -0.10781813, 0.27201805,  0.14324132,  -0.23681851, -0.27115166,
       -0.01580888, -0.14943552, 0.15465137,  0.09784451,  -0.0337657,

       -0.14884081, 0.19931212,  -0.36002168, 0.34663299,  -0.11405486,
       0.12672701,  0.39463779,  -0.07886535, -0.06384811, 0.08249187,

       -0.26816407, -0.19905911, 0.29211238,  0.31264046,  -0.28664589,
       0.05698794,  0.11613581,  0.14078894,  0.02187902,  -0.21781836,

       -0.15567942, 0.08693647,  -0.38256618, 0.36580828,  -0.22922277,
       -0.0226903,  0.12878349,  -0.28122205, -0.10850525, -0.11955214,

       0.27179423,  -0.04710215, 0.31069002,  0.22672787,  0.09580326,
       0.08682203,  0.1258215,   0.1851041,   0.29228821,  0.12366763});

  svdf.SetBias({});

  svdf.ResetState();
  const int svdf_num_batches = svdf.num_batches();
  const int svdf_input_size = svdf.input_size();
  const int svdf_num_units = svdf.num_units();
  const int input_sequence_size =
      sizeof(svdf_input_rank2) / sizeof(float) / (svdf_input_size * svdf_num_batches);
  // Going over each input batch, setting the input tensor, invoking the SVDF op
  // and checking the output with the expected golden values.
  for (int i = 0; i < input_sequence_size; i++) {
    float* batch_start = svdf_input_rank2 + i * svdf_input_size * svdf_num_batches;
    float* batch_end = batch_start + svdf_input_size * svdf_num_batches;
    svdf.SetInput(0, batch_start, batch_end);

    svdf.Invoke();

    float* golden_start =
        svdf_golden_output_rank_2 + i * svdf_num_units * svdf_num_batches;
    float* golden_end = golden_start + svdf_num_units * svdf_num_batches;
    std::vector<float> expected;
    expected.insert(expected.end(), golden_start, golden_end);

    EXPECT_THAT(svdf.GetOutput(), ElementsAreArray(ArrayFloatNear(expected)));
  }
}

}  // namespace wrapper
}  // namespace nn
}  // namespace android
