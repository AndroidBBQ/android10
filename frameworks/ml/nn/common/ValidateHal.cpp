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

#define LOG_TAG "ValidateHal"

#include "ValidateHal.h"
#include "NeuralNetworks.h"
#include "OperationsUtils.h"
#include "Tracing.h"
#include "Utils.h"

#include <android-base/logging.h>

namespace android {
namespace nn {

template <class T_Model>
struct ModelToHalVersion;
template <>
struct ModelToHalVersion<V1_0::Model> {
    static constexpr HalVersion version = HalVersion::V1_0;
};
template <>
struct ModelToHalVersion<V1_1::Model> {
    static constexpr HalVersion version = HalVersion::V1_1;
};
template <>
struct ModelToHalVersion<V1_2::Model> {
    static constexpr HalVersion version = HalVersion::V1_2;
};

class MemoryAccessVerifier {
public:
    MemoryAccessVerifier(const hidl_vec<hidl_memory>& pools)
        : mPoolCount(pools.size()), mPoolSizes(mPoolCount) {
        for (size_t i = 0; i < mPoolCount; i++) {
            mPoolSizes[i] = pools[i].size();
        }
    }
    bool validate(const DataLocation& location) {
        if (location.poolIndex >= mPoolCount) {
            LOG(ERROR) << "Invalid poolIndex " << location.poolIndex << "/" << mPoolCount;
            return false;
        }
        const size_t size = mPoolSizes[location.poolIndex];
        // Do the addition using size_t to avoid potential wrap-around problems.
        if (static_cast<size_t>(location.offset) + location.length > size) {
            LOG(ERROR) << "Reference to pool " << location.poolIndex << " with offset "
                       << location.offset << " and length " << location.length
                       << " exceeds pool size of " << size;
            return false;
        }
        return true;
    }

private:
    size_t mPoolCount;
    std::vector<size_t> mPoolSizes;
};

static bool validateOperandExtraParams(const V1_2::Operand& operand, uint32_t index) {
    switch (operand.type) {
        case OperandType::FLOAT32:
        case OperandType::INT32:
        case OperandType::UINT32:
        case OperandType::BOOL:
        case OperandType::TENSOR_FLOAT32:
        case OperandType::TENSOR_FLOAT16:
        case OperandType::TENSOR_INT32:
        case OperandType::TENSOR_QUANT8_ASYMM:
        case OperandType::TENSOR_QUANT8_SYMM:
        case OperandType::TENSOR_QUANT16_ASYMM:
        case OperandType::TENSOR_QUANT16_SYMM:
        case OperandType::TENSOR_BOOL8: {
            NN_RET_CHECK(operand.extraParams.getDiscriminator() ==
                         V1_2::Operand::ExtraParams::hidl_discriminator::none)
                    << "Operand " << index << ": Operand of type "
                    << getOperandTypeName(operand.type)
                    << " has incorrect extraParams: " << toString(operand.extraParams);
        } break;
        case OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL: {
            NN_RET_CHECK(operand.extraParams.getDiscriminator() ==
                         V1_2::Operand::ExtraParams::hidl_discriminator::channelQuant)
                    << "Operand " << index << ": Operand of type "
                    << getOperandTypeName(operand.type) << " without a Channel Quantization params";
            auto& channelQuant = operand.extraParams.channelQuant();

            size_t count = operand.dimensions.size();
            NN_RET_CHECK_LT(channelQuant.channelDim, count)
                    << "Operand " << index << ": Operand of type "
                    << getOperandTypeName(operand.type)
                    << " with an invalid channelQuant.channelDim " << channelQuant.channelDim
                    << ", must be valid dimension index in range [0, " << count << ")";
            uint32_t expected = operand.dimensions[channelQuant.channelDim];
            NN_RET_CHECK_EQ(channelQuant.scales.size(), expected)
                    << "Operand " << index << ": Operand of type "
                    << getOperandTypeName(operand.type) << " with a wrong-sized scales, "
                    << "expected " << expected << " was " << channelQuant.scales.size();
            NN_RET_CHECK_NE(expected, 0)
                    << "Operand " << index << ": Operand of type "
                    << getOperandTypeName(operand.type) << " channel dimension "
                    << channelQuant.channelDim << " is underspecified (can't be 0)";
            for (uint32_t i = 0; i < expected; ++i) {
                NN_RET_CHECK_GT(channelQuant.scales[i], .0f)
                        << "Operand " << index << ": Operand of type "
                        << getOperandTypeName(operand.type) << " with a negative value in scales["
                        << i << "]=" << channelQuant.scales[i];
            }
        } break;
        default: {
            if (isExtensionOperandType(operand.type)) {
                NN_RET_CHECK(operand.extraParams.getDiscriminator() ==
                                     V1_2::Operand::ExtraParams::hidl_discriminator::extension ||
                             operand.extraParams.getDiscriminator() ==
                                     V1_2::Operand::ExtraParams::hidl_discriminator::none)
                        << "Operand " << index << ": Extension operand of type "
                        << getOperandTypeName(operand.type)
                        << " has incorrect extraParams: " << toString(operand.extraParams);
            }
            // No validation for OEM types.
        } break;
    }
    return true;
}

template <typename VersionedOperand>
static bool validateOperands(const hidl_vec<VersionedOperand>& operands,
                             const hidl_vec<uint8_t>& operandValues,
                             const hidl_vec<hidl_memory>& pools, bool allowUnspecifiedRank) {
    uint32_t index = 0;
    MemoryAccessVerifier poolVerifier(pools);
    for (auto& versionedOperand : operands) {
        if (!validOperandType(versionedOperand.type)) {
            LOG(ERROR) << "Operand is not supported by this version: "
                       << toString(versionedOperand.type);
            return false;
        }
        // Once we are sure the operand is supported by its version, it is safe
        // to convert it to the latest version for the rest of the validations.
        V1_2::Operand operand = convertToV1_2(versionedOperand);
        // Validate type and dimensions.
        switch (operand.type) {
            case OperandType::FLOAT16:
            case OperandType::FLOAT32:
            case OperandType::INT32:
            case OperandType::UINT32:
            case OperandType::BOOL:
            case OperandType::OEM: {
                size_t count = operand.dimensions.size();
                if (count != 0) {
                    LOG(ERROR) << "Operand " << index << ": Scalar data has dimensions of rank "
                               << count;
                    return false;
                }
                break;
            }
            case OperandType::TENSOR_FLOAT16:
            case OperandType::TENSOR_FLOAT32:
            case OperandType::TENSOR_INT32:
            case OperandType::TENSOR_QUANT8_ASYMM:
            case OperandType::TENSOR_QUANT8_SYMM:
            case OperandType::TENSOR_QUANT16_ASYMM:
            case OperandType::TENSOR_QUANT16_SYMM:
            case OperandType::TENSOR_BOOL8:
            case OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL:
            case OperandType::TENSOR_OEM_BYTE: {
                if ((!allowUnspecifiedRank || operand.lifetime == OperandLifeTime::CONSTANT_COPY ||
                     operand.lifetime == OperandLifeTime::CONSTANT_REFERENCE) &&
                    operand.dimensions.size() == 0) {
                    LOG(ERROR) << "Operand " << index << ": Tensor has dimensions of rank 0";
                    return false;
                }
                break;
            }
            default: {
                if (!isExtensionOperandType(operand.type)) {
                    LOG(ERROR) << "Operand " << index << ": Invalid operand type "
                               << toString(operand.type);
                    return false;
                }
            } break;
        }

        // TODO Validate the numberOfConsumers.
        // TODO Since we have to validate it, there was no point in including it. For the next
        // release, consider removing unless we have an additional process in system space
        // that creates this value. In that case, it would not have to be validated.

        // Validate the scale.
        switch (operand.type) {
            case OperandType::FLOAT16:
            case OperandType::FLOAT32:
            case OperandType::INT32:
            case OperandType::UINT32:
            case OperandType::BOOL:
            case OperandType::TENSOR_FLOAT16:
            case OperandType::TENSOR_FLOAT32:
            case OperandType::TENSOR_BOOL8:
            case OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL:
                if (operand.scale != 0.f) {
                    LOG(ERROR) << "Operand " << index << ": Operand of type "
                               << getOperandTypeName(operand.type) << " with a non-zero scale ("
                               << operand.scale << ")";
                    return false;
                }
                break;
            case OperandType::TENSOR_INT32:
                // TENSOR_INT32 may be used with or without scale, depending on the operation.
                if (operand.scale < 0.f) {
                    LOG(ERROR) << "Operand " << index << ": Operand of type "
                               << getOperandTypeName(operand.type) << " with a negative scale";
                    return false;
                }
                break;
            case OperandType::TENSOR_QUANT8_ASYMM:
            case OperandType::TENSOR_QUANT8_SYMM:
            case OperandType::TENSOR_QUANT16_ASYMM:
            case OperandType::TENSOR_QUANT16_SYMM:
                if (operand.scale <= 0.f) {
                    LOG(ERROR) << "Operand " << index << ": Operand of type "
                               << getOperandTypeName(operand.type) << " with a non-positive scale";
                    return false;
                }
                break;
            default:
                if (isExtensionOperandType(operand.type) && operand.scale != 0.f) {
                    LOG(ERROR) << "Operand " << index << ": Operand of type "
                               << getOperandTypeName(operand.type) << " with a non-zero scale ("
                               << operand.scale << ")";
                    return false;
                }
                // No validation for OEM types.
                // TODO(b/119869082) We should have a separate type for TENSOR_INT32 with a scale.
                break;
        }

        // Validate the zeroPoint.
        switch (operand.type) {
            case OperandType::FLOAT16:
            case OperandType::FLOAT32:
            case OperandType::INT32:
            case OperandType::UINT32:
            case OperandType::BOOL:
            case OperandType::TENSOR_FLOAT16:
            case OperandType::TENSOR_FLOAT32:
            case OperandType::TENSOR_INT32:
            case OperandType::TENSOR_BOOL8:
            case OperandType::TENSOR_QUANT8_SYMM:
            case OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL:
                if (operand.zeroPoint != 0) {
                    LOG(ERROR) << "Operand " << index << ": Operand of type "
                               << getOperandTypeName(operand.type) << " with a non-zero zeroPoint "
                               << operand.zeroPoint;
                    return false;
                }
                break;
            case OperandType::TENSOR_QUANT8_ASYMM:
                if (operand.zeroPoint < 0 || operand.zeroPoint > 255) {
                    LOG(ERROR) << "Operand " << index << ": Operand of type "
                               << getOperandTypeName(operand.type) << " with an invalid zeroPoint "
                               << operand.zeroPoint << ", must be in range [0, 255]";
                    return false;
                }
                break;
            case OperandType::TENSOR_QUANT16_ASYMM:
                if (operand.zeroPoint < 0 || operand.zeroPoint > 65535) {
                    LOG(ERROR) << "Operand " << index << ": Operand of type "
                               << getOperandTypeName(operand.type) << " with an invalid zeroPoint "
                               << operand.zeroPoint << ", must be in range [0, 65535]";
                    return false;
                }
                break;
            case OperandType::TENSOR_QUANT16_SYMM:
                if (operand.zeroPoint != 0) {
                    LOG(ERROR) << "Operand " << index << ": Operand of type "
                               << getOperandTypeName(operand.type) << " with a non-zero zeroPoint "
                               << operand.zeroPoint;
                    return false;
                }
                break;
            default:
                if (isExtensionOperandType(operand.type) && operand.zeroPoint != 0) {
                    LOG(ERROR) << "Operand " << index << ": Operand of type "
                               << getOperandTypeName(operand.type) << " with a non-zero zeroPoint "
                               << operand.zeroPoint;
                    return false;
                }
                // No validation for OEM types.
                break;
        }

        NN_RET_CHECK(validateOperandExtraParams(operand, index));

        // Validate the lifetime and the location.
        const DataLocation& location = operand.location;
        switch (operand.lifetime) {
            case OperandLifeTime::CONSTANT_COPY:
                if (location.poolIndex != 0) {
                    LOG(ERROR) << "Operand " << index
                               << ": CONSTANT_COPY with a non-zero poolIndex "
                               << location.poolIndex;
                    return false;
                }
                // Do the addition using size_t to avoid potential wrap-around problems.
                if (static_cast<size_t>(location.offset) + location.length > operandValues.size()) {
                    LOG(ERROR) << "Operand " << index
                               << ": OperandValue location out of range.  Starts at "
                               << location.offset << ", length " << location.length << ", max "
                               << operandValues.size();
                    return false;
                }
                break;
            case OperandLifeTime::CONSTANT_REFERENCE:
                if (!poolVerifier.validate(location)) {
                    return false;
                }
                break;
            case OperandLifeTime::TEMPORARY_VARIABLE:
            case OperandLifeTime::MODEL_INPUT:
            case OperandLifeTime::MODEL_OUTPUT:
            case OperandLifeTime::NO_VALUE:
                if (location.poolIndex != 0 || location.offset != 0 || location.length != 0) {
                    LOG(ERROR) << "Operand " << index << ": Unexpected poolIndex "
                               << location.poolIndex << ", offset " << location.offset
                               << ", or length " << location.length << " for operand of lifetime "
                               << toString(operand.lifetime);
                    return false;
                }
                break;
            default:
                LOG(ERROR) << "Operand " << index << ": Invalid lifetime "
                           << toString(operand.lifetime);
                return false;
        }

        // For constants, validate that the length is as expected. The other lifetimes
        // expect the length to be 0. Don't validate for OEM types.
        if (operand.lifetime == OperandLifeTime::CONSTANT_REFERENCE ||
            operand.lifetime == OperandLifeTime::CONSTANT_COPY) {
            if (!isExtensionOperandType(operand.type) && operand.type != OperandType::OEM &&
                operand.type != OperandType::TENSOR_OEM_BYTE) {
                uint32_t expectedLength = nonExtensionOperandSizeOfData(operand);
                if (location.length != expectedLength) {
                    LOG(ERROR) << "Operand " << index << ": For operand " << toString(operand)
                               << " expected a size of " << expectedLength << " but got "
                               << location.length;
                    return false;
                }
            }
        }

        index++;
    }
    return true;
}

static HalVersion getHalVersion(const V1_0::Operation&) {
    return HalVersion::V1_0;
}

static HalVersion getHalVersion(const V1_1::Operation&) {
    return HalVersion::V1_1;
}

static HalVersion getHalVersion(const V1_2::Operation&) {
    return HalVersion::V1_2;
}

template <typename VersionedOperation>
static bool validateOperations(const hidl_vec<VersionedOperation>& operations,
                               const hidl_vec<Operand>& operands) {
    const size_t operandCount = operands.size();
    // This vector keeps track of whether there's an operation that writes to
    // each operand. It is used to validate that temporary variables and
    // model outputs will be written to.
    std::vector<bool> writtenTo(operandCount, false);
    for (auto& op : operations) {
        // TODO Validate the shapes and any known values. This is currently
        // done in CpuExecutor but should be done here for all drivers.
        int error = validateOperation(
                static_cast<int32_t>(op.type), op.inputs.size(),
                op.inputs.size() > 0 ? op.inputs.data() : nullptr, op.outputs.size(),
                op.outputs.size() > 0 ? op.outputs.data() : nullptr, operands, getHalVersion(op));
        if (error != ANEURALNETWORKS_NO_ERROR) {
            LOG(ERROR) << "Invalid operation " << toString(op.type);
            return false;
        }

        for (uint32_t i : op.outputs) {
            const Operand& operand = operands[i];
            if (operand.lifetime != OperandLifeTime::TEMPORARY_VARIABLE &&
                operand.lifetime != OperandLifeTime::MODEL_OUTPUT) {
                LOG(ERROR) << "Writing to an operand with incompatible lifetime "
                           << toString(operand.lifetime);
                return false;
            }

            // Check that we only write once to an operand.
            if (writtenTo[i]) {
                LOG(ERROR) << "Operand " << i << " written a second time";
                return false;
            }
            writtenTo[i] = true;
        }
    }
    for (size_t i = 0; i < operandCount; i++) {
        if (!writtenTo[i]) {
            const Operand& operand = operands[i];
            if (operand.lifetime == OperandLifeTime::TEMPORARY_VARIABLE ||
                operand.lifetime == OperandLifeTime::MODEL_OUTPUT) {
                LOG(ERROR) << "Operand " << i << " with lifetime " << toString(operand.lifetime)
                           << " is not being written to.";
                return false;
            }
        }
    }
    // TODO More whole graph verifications are possible, for example that an
    // operand is not use as input & output for the same op, and more
    // generally that it is acyclic.
    return true;
}

bool validatePool(const hidl_memory& pool, HalVersion ver) {
    const auto& name = pool.name();
    if (name != "ashmem" && name != "mmap_fd" &&
        ((ver < HalVersion::V1_2) ||
         (name != "hardware_buffer_blob" && name != "hardware_buffer"))) {
        LOG(ERROR) << "Unsupported memory type " << name;
        return false;
    }
    if (pool.handle() == nullptr) {
        LOG(ERROR) << "Memory of type " << name << " is null";
        return false;
    }
    return true;
}

static bool validatePools(const hidl_vec<hidl_memory>& pools, HalVersion ver) {
    return std::all_of(pools.begin(), pools.end(),
                       [ver](const hidl_memory& pool) { return validatePool(pool, ver); });
}

static bool validateModelInputOutputs(const hidl_vec<uint32_t> indexes,
                                      const hidl_vec<Operand>& operands, OperandLifeTime lifetime) {
    const size_t operandCount = operands.size();
    for (uint32_t i : indexes) {
        if (i >= operandCount) {
            LOG(ERROR) << "Model input or output index out of range: " << i << "/" << operandCount;
            return false;
        }
        const Operand& operand = operands[i];
        if (operand.lifetime != lifetime) {
            LOG(ERROR) << "Model input or output has lifetime of " << toString(operand.lifetime)
                       << " instead of the expected " << toString(lifetime);
            return false;
        }
    }

    std::vector<uint32_t> sortedIndexes = indexes;
    std::sort(sortedIndexes.begin(), sortedIndexes.end());
    auto adjacentI = std::adjacent_find(sortedIndexes.begin(), sortedIndexes.end());
    if (adjacentI != sortedIndexes.end()) {
        LOG(ERROR) << "Model input or output occurs multiple times: " << *adjacentI;
        return false;
    }
    return true;
}

template <class T_Model>
bool validateModel(const T_Model& model) {
    NNTRACE_FULL(NNTRACE_LAYER_UTILITY, NNTRACE_PHASE_UNSPECIFIED, "validateModel");
    HalVersion version = ModelToHalVersion<T_Model>::version;
    if (model.operations.size() == 0 || model.operands.size() == 0) {
        LOG(ERROR) << "Invalid empty model.";
        return false;
    }
    // We only need versioned operands for their validation. For all the other
    // validations we can use operands upcasted to the latest version.
    const hidl_vec<Operand> latestVersionOperands = convertToV1_2(model.operands);
    return (validateOperands(model.operands, model.operandValues, model.pools,
                             /*allowUnspecifiedRank=*/version >= HalVersion::V1_2) &&
            validateOperations(model.operations, latestVersionOperands) &&
            validateModelInputOutputs(model.inputIndexes, latestVersionOperands,
                                      OperandLifeTime::MODEL_INPUT) &&
            validateModelInputOutputs(model.outputIndexes, latestVersionOperands,
                                      OperandLifeTime::MODEL_OUTPUT) &&
            validatePools(model.pools, version));
}

template bool validateModel<V1_0::Model>(const V1_0::Model& model);
template bool validateModel<V1_1::Model>(const V1_1::Model& model);
template bool validateModel<V1_2::Model>(const V1_2::Model& model);

// Validates the arguments of a request. type is either "input" or "output" and is used
// for printing error messages. The operandIndexes is the appropriate array of input
// or output operand indexes that was passed to the ANeuralNetworksModel_identifyInputsAndOutputs.
static bool validateRequestArguments(const hidl_vec<RequestArgument>& requestArguments,
                                     const hidl_vec<uint32_t>& operandIndexes,
                                     const hidl_vec<Operand>& operands,
                                     const hidl_vec<hidl_memory>& pools, bool allowUnspecified,
                                     const char* type) {
    MemoryAccessVerifier poolVerifier(pools);
    // The request should specify as many arguments as were described in the model.
    const size_t requestArgumentCount = requestArguments.size();
    if (requestArgumentCount != operandIndexes.size()) {
        LOG(ERROR) << "Request specifies " << requestArgumentCount << " " << type
                   << "s but the model has " << operandIndexes.size();
        return false;
    }
    for (size_t requestArgumentIndex = 0; requestArgumentIndex < requestArgumentCount;
         requestArgumentIndex++) {
        const RequestArgument& requestArgument = requestArguments[requestArgumentIndex];
        const DataLocation& location = requestArgument.location;
        // Get the operand index for this argument. We extract it from the list
        // that was provided in the call to ANeuralNetworksModel_identifyInputsAndOutputs.
        // We assume in this function that the model has been validated already.
        const uint32_t operandIndex = operandIndexes[requestArgumentIndex];
        const Operand& operand = operands[operandIndex];
        if (requestArgument.hasNoValue) {
            if (location.poolIndex != 0 || location.offset != 0 || location.length != 0 ||
                requestArgument.dimensions.size() != 0) {
                LOG(ERROR) << "Request " << type << " " << requestArgumentIndex
                           << " has no value yet has details.";
                return false;
            }
        } else {
            // Validate the location.
            if (!poolVerifier.validate(location)) {
                return false;
            }
            // If the argument specified a dimension, validate it.
            uint32_t rank = requestArgument.dimensions.size();
            if (rank == 0) {
                if (!allowUnspecified) {
                    // Validate that all the dimensions are specified in the model.
                    for (size_t i = 0; i < operand.dimensions.size(); i++) {
                        if (operand.dimensions[i] == 0) {
                            LOG(ERROR) << "Model has dimension " << i
                                       << " set to 0 but the request does specify the dimension.";
                            return false;
                        }
                    }
                }
            } else {
                if (rank != operand.dimensions.size()) {
                    LOG(ERROR) << "Request " << type << " " << requestArgumentIndex
                               << " has number of dimensions (" << rank
                               << ") different than the model's (" << operand.dimensions.size()
                               << ")";
                    return false;
                }
                for (size_t i = 0; i < rank; i++) {
                    if (requestArgument.dimensions[i] != operand.dimensions[i] &&
                        operand.dimensions[i] != 0) {
                        LOG(ERROR) << "Request " << type << " " << requestArgumentIndex
                                   << " has dimension " << i << " of "
                                   << requestArgument.dimensions[i]
                                   << " different than the model's " << operand.dimensions[i];
                        return false;
                    }
                    if (requestArgument.dimensions[i] == 0 && !allowUnspecified) {
                        LOG(ERROR) << "Request " << type << " " << requestArgumentIndex
                                   << " has dimension " << i << " of zero";
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

template <class T_Model>
bool validateRequest(const Request& request, const T_Model& model) {
    HalVersion version = ModelToHalVersion<T_Model>::version;
    return (validateRequestArguments(request.inputs, model.inputIndexes,
                                     convertToV1_2(model.operands), request.pools,
                                     /*allowUnspecified=*/false, "input") &&
            validateRequestArguments(request.outputs, model.outputIndexes,
                                     convertToV1_2(model.operands), request.pools,
                                     /*allowUnspecified=*/version >= HalVersion::V1_2, "output") &&
            validatePools(request.pools, version));
}

template bool validateRequest<V1_0::Model>(const Request& request, const V1_0::Model& model);
template bool validateRequest<V1_1::Model>(const Request& request, const V1_1::Model& model);
template bool validateRequest<V1_2::Model>(const Request& request, const V1_2::Model& model);

bool validateExecutionPreference(ExecutionPreference preference) {
    return preference == ExecutionPreference::LOW_POWER ||
           preference == ExecutionPreference::FAST_SINGLE_ANSWER ||
           preference == ExecutionPreference::SUSTAINED_SPEED;
}

bool validOperandType(V1_0::OperandType operandType) {
    switch (operandType) {
        case V1_0::OperandType::FLOAT32:
        case V1_0::OperandType::INT32:
        case V1_0::OperandType::UINT32:
        case V1_0::OperandType::TENSOR_FLOAT32:
        case V1_0::OperandType::TENSOR_INT32:
        case V1_0::OperandType::TENSOR_QUANT8_ASYMM:
        case V1_0::OperandType::OEM:
        case V1_0::OperandType::TENSOR_OEM_BYTE:
            return true;
        default:
            return false;
    }
}

bool validOperandType(V1_2::OperandType operandType) {
    switch (operandType) {
        case V1_2::OperandType::FLOAT16:
        case V1_2::OperandType::FLOAT32:
        case V1_2::OperandType::INT32:
        case V1_2::OperandType::UINT32:
        case V1_2::OperandType::BOOL:
        case V1_2::OperandType::TENSOR_FLOAT16:
        case V1_2::OperandType::TENSOR_FLOAT32:
        case V1_2::OperandType::TENSOR_INT32:
        case V1_2::OperandType::TENSOR_QUANT8_ASYMM:
        case V1_2::OperandType::TENSOR_QUANT8_SYMM:
        case V1_2::OperandType::TENSOR_QUANT16_ASYMM:
        case V1_2::OperandType::TENSOR_QUANT16_SYMM:
        case V1_2::OperandType::TENSOR_BOOL8:
        case V1_2::OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL:
        case V1_2::OperandType::OEM:
        case V1_2::OperandType::TENSOR_OEM_BYTE:
            return true;
        default:
            return isExtensionOperandType(operandType);
    }
}

}  // namespace nn
}  // namespace android
