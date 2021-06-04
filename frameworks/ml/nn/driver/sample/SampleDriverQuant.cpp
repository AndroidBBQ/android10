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

#define LOG_TAG "SampleDriverQuant"

#include "SampleDriver.h"

#include "HalInterfaces.h"
#include "Utils.h"
#include "ValidateHal.h"

#include <android-base/logging.h>
#include <hidl/LegacySupport.h>
#include <thread>

namespace android {
namespace nn {
namespace sample_driver {

class SampleDriverQuant : public SampleDriver {
public:
    SampleDriverQuant() : SampleDriver("sample-quant") {}
    Return<void> getCapabilities_1_2(getCapabilities_1_2_cb cb) override;
    Return<void> getSupportedOperations_1_2(const V1_2::Model& model,
                                            getSupportedOperations_1_2_cb cb) override;
};

Return<void> SampleDriverQuant::getCapabilities_1_2(getCapabilities_1_2_cb cb) {
    android::nn::initVLogMask();
    VLOG(DRIVER) << "getCapabilities()";

    Capabilities capabilities = {
            .relaxedFloat32toFloat16PerformanceScalar = {.execTime = 50.0f, .powerUsage = 1.0f},
            .relaxedFloat32toFloat16PerformanceTensor = {.execTime = 50.0f, .powerUsage = 1.0f},
            .operandPerformance = nonExtensionOperandPerformance({50.0f, 1.0f})};

    cb(ErrorStatus::NONE, capabilities);
    return Void();
}

Return<void> SampleDriverQuant::getSupportedOperations_1_2(const V1_2::Model& model,
                                                           getSupportedOperations_1_2_cb cb) {
    VLOG(DRIVER) << "getSupportedOperations()";
    if (validateModel(model)) {
        const size_t count = model.operations.size();
        std::vector<bool> supported(count);
        for (size_t i = 0; i < count; i++) {
            const Operation& operation = model.operations[i];
            if (operation.inputs.size() > 0) {
                const Operand& firstOperand = model.operands[operation.inputs[0]];
                supported[i] = firstOperand.type == OperandType::TENSOR_QUANT8_ASYMM;
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

using android::nn::sample_driver::SampleDriverQuant;
using android::sp;

int main() {
    sp<SampleDriverQuant> driver(new SampleDriverQuant());
    return driver->run();
}
