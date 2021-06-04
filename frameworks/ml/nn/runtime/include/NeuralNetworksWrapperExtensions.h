/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef ANDROID_ML_NN_RUNTIME_NEURAL_NETWORKS_WRAPPER_EXTENSIONS_H
#define ANDROID_ML_NN_RUNTIME_NEURAL_NETWORKS_WRAPPER_EXTENSIONS_H

#include "NeuralNetworksExtensions.h"
#include "NeuralNetworksWrapper.h"

#include <variant>

namespace android {
namespace nn {
namespace extension_wrapper {

using wrapper::SymmPerChannelQuantParams;
using wrapper::Type;

struct ExtensionOperandParams {
    std::vector<uint8_t> data;

    ExtensionOperandParams(std::vector<uint8_t> data) : data(std::move(data)) {}

    template <typename T>
    ExtensionOperandParams(const T& data)
        : ExtensionOperandParams(
                  std::vector(reinterpret_cast<const uint8_t*>(&data),
                              reinterpret_cast<const uint8_t*>(&data) + sizeof(data))) {
        static_assert(std::is_trivially_copyable<T>::value, "data must be trivially copyable");
    }
};

struct OperandType {
    using ExtraParams =
            std::variant<std::monostate, SymmPerChannelQuantParams, ExtensionOperandParams>;

    ANeuralNetworksOperandType operandType;
    std::vector<uint32_t> dimensions;
    ExtraParams extraParams;

    OperandType(const OperandType& other)
        : operandType(other.operandType),
          dimensions(other.dimensions),
          extraParams(other.extraParams) {
        operandType.dimensions = dimensions.size() > 0 ? dimensions.data() : nullptr;
    }

    OperandType& operator=(const OperandType& other) {
        if (this != &other) {
            operandType = other.operandType;
            dimensions = other.dimensions;
            extraParams = other.extraParams;
            operandType.dimensions = dimensions.size() > 0 ? dimensions.data() : nullptr;
        }
        return *this;
    }

    OperandType(Type type, std::vector<uint32_t> d, float scale = 0.0f, int32_t zeroPoint = 0,
                ExtraParams&& extraParams = std::monostate())
        : dimensions(std::move(d)), extraParams(std::move(extraParams)) {
        operandType = {
                .type = static_cast<int32_t>(type),
                .dimensionCount = static_cast<uint32_t>(dimensions.size()),
                .dimensions = dimensions.size() > 0 ? dimensions.data() : nullptr,
                .scale = scale,
                .zeroPoint = zeroPoint,
        };
    }

    OperandType(Type type, std::vector<uint32_t> dimensions, float scale, int32_t zeroPoint,
                SymmPerChannelQuantParams&& channelQuant)
        : OperandType(type, dimensions, scale, zeroPoint, ExtraParams(std::move(channelQuant))) {}

    OperandType(Type type, std::vector<uint32_t> dimensions, ExtraParams&& extraParams)
        : OperandType(type, dimensions, 0.0f, 0, std::move(extraParams)) {}
};

class Model : public wrapper::Model {
   public:
    using wrapper::Model::Model;  // Inherit constructors.

    int32_t getExtensionOperandType(const char* extensionName, uint16_t typeWithinExtension) {
        int32_t result;
        if (ANeuralNetworksModel_getExtensionOperandType(mModel, extensionName, typeWithinExtension,
                                                         &result) != ANEURALNETWORKS_NO_ERROR) {
            mValid = false;
        }
        return result;
    }

    ANeuralNetworksOperationType getExtensionOperationType(const char* extensionName,
                                                           uint16_t typeWithinExtension) {
        ANeuralNetworksOperationType result;
        if (ANeuralNetworksModel_getExtensionOperationType(mModel, extensionName,
                                                           typeWithinExtension,
                                                           &result) != ANEURALNETWORKS_NO_ERROR) {
            mValid = false;
        }
        return result;
    }

    uint32_t addOperand(const OperandType* type) {
        if (ANeuralNetworksModel_addOperand(mModel, &(type->operandType)) !=
            ANEURALNETWORKS_NO_ERROR) {
            mValid = false;
        }
        if (std::holds_alternative<SymmPerChannelQuantParams>(type->extraParams)) {
            const auto& channelQuant = std::get<SymmPerChannelQuantParams>(type->extraParams);
            if (ANeuralNetworksModel_setOperandSymmPerChannelQuantParams(
                        mModel, mNextOperandId, &channelQuant.params) != ANEURALNETWORKS_NO_ERROR) {
                mValid = false;
            }
        } else if (std::holds_alternative<ExtensionOperandParams>(type->extraParams)) {
            const auto& extension = std::get<ExtensionOperandParams>(type->extraParams);
            if (ANeuralNetworksModel_setOperandExtensionData(
                        mModel, mNextOperandId, extension.data.data(), extension.data.size()) !=
                ANEURALNETWORKS_NO_ERROR) {
                mValid = false;
            }
        }
        return mNextOperandId++;
    }
};

}  // namespace extension_wrapper

namespace wrapper {

using ExtensionModel = extension_wrapper::Model;
using ExtensionOperandType = extension_wrapper::OperandType;
using ExtensionOperandParams = extension_wrapper::ExtensionOperandParams;

}  // namespace wrapper
}  // namespace nn
}  // namespace android

#endif  //  ANDROID_ML_NN_RUNTIME_NEURAL_NETWORKS_WRAPPER_EXTENSIONS_H
