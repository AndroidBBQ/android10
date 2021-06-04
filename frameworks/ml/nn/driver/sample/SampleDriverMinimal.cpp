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

#define LOG_TAG "SampleDriverMinimal"

#include "SampleDriver.h"

#include "HalInterfaces.h"
#include "NeuralNetworksOEM.h"
#include "Utils.h"
#include "ValidateHal.h"

#include <android-base/logging.h>
#include <hidl/LegacySupport.h>
#include <thread>

namespace android {
namespace nn {
namespace sample_driver {

class SampleDriverMinimal : public SampleDriver {
public:
    SampleDriverMinimal() : SampleDriver("sample-minimal") {}
    Return<void> getCapabilities_1_2(getCapabilities_1_2_cb cb) override;
    Return<void> getSupportedOperations_1_2(const V1_2::Model& model,
                                            getSupportedOperations_1_2_cb cb) override;
};

Return<void> SampleDriverMinimal::getCapabilities_1_2(getCapabilities_1_2_cb cb) {
    android::nn::initVLogMask();
    VLOG(DRIVER) << "getCapabilities()";

    Capabilities capabilities = {
            .relaxedFloat32toFloat16PerformanceScalar = {.execTime = 0.4f, .powerUsage = 0.5f},
            .relaxedFloat32toFloat16PerformanceTensor = {.execTime = 0.4f, .powerUsage = 0.5f},
            .operandPerformance = nonExtensionOperandPerformance({1.0f, 1.0f})};
    update(&capabilities.operandPerformance, OperandType::TENSOR_FLOAT32,
           {.execTime = 0.4f, .powerUsage = 0.5f});
    update(&capabilities.operandPerformance, OperandType::FLOAT32,
           {.execTime = 0.4f, .powerUsage = 0.5f});

    cb(ErrorStatus::NONE, capabilities);
    return Void();
}

Return<void> SampleDriverMinimal::getSupportedOperations_1_2(const V1_2::Model& model,
                                                             getSupportedOperations_1_2_cb cb) {
    VLOG(DRIVER) << "getSupportedOperations()";
    if (validateModel(model)) {
        const size_t count = model.operations.size();
        std::vector<bool> supported(count);
        // Simulate supporting just a few ops
        for (size_t i = 0; i < count; i++) {
            supported[i] = false;
            const Operation& operation = model.operations[i];
            switch (operation.type) {
                case OperationType::ADD:
                case OperationType::CONCATENATION:
                case OperationType::CONV_2D: {
                    const Operand& firstOperand = model.operands[operation.inputs[0]];
                    if (firstOperand.type == OperandType::TENSOR_FLOAT32) {
                        supported[i] = true;
                    }
                    break;
                }
                default:
                    break;
            }
        }
        cb(ErrorStatus::NONE, supported);
    } else {
        std::vector<bool> supported;
        cb(ErrorStatus::INVALID_ARGUMENT, supported);
    }
    return Void();
}

} // namespace sample_driver
} // namespace nn
} // namespace android

using android::nn::sample_driver::SampleDriverMinimal;
using android::sp;

int main() {
    sp<SampleDriverMinimal> driver(new SampleDriverMinimal());
    return driver->run();
}
