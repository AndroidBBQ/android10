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

#define LOG_TAG "Utils"

#include "Utils.h"

#include "NeuralNetworks.h"
#include "NeuralNetworksOEM.h"
#include "OperationResolver.h"
#include "ValidateHal.h"

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>
#include <sys/system_properties.h>
#include <algorithm>
#include <unordered_map>

using ::android::hidl::allocator::V1_0::IAllocator;

namespace android {
namespace nn {

const char kVLogPropKey[] = "debug.nn.vlog";
int vLogMask = ~0;

// Split the space separated list of tags from verbose log setting and build the
// logging mask from it. note that '1' and 'all' are special cases to enable all
// verbose logging.
//
// NN API verbose logging setting comes from system property debug.nn.vlog.
// Example:
// setprop debug.nn.vlog 1 : enable all logging tags.
// setprop debug.nn.vlog "model compilation" : only enable logging for MODEL and
//                                             COMPILATION tags.
void initVLogMask() {
    vLogMask = 0;
    const std::string vLogSetting = android::base::GetProperty(kVLogPropKey, "");
    if (vLogSetting.empty()) {
        return;
    }

    std::unordered_map<std::string, int> vLogFlags = {
        {"1", -1},
        {"all", -1},
        {"model", MODEL},
        {"compilation", COMPILATION},
        {"execution", EXECUTION},
        {"cpuexe", CPUEXE},
        {"manager", MANAGER},
        {"driver", DRIVER}};

    std::vector<std::string> elements = android::base::Split(vLogSetting, " ,:");
    for (const auto& elem : elements) {
        const auto& flag = vLogFlags.find(elem);
        if (flag == vLogFlags.end()) {
            LOG(ERROR) << "Unknown trace flag: " << elem;
            continue;
        }

        if (flag->second == -1) {
            // -1 is used for the special values "1" and "all" that enable all
            // tracing.
            vLogMask = ~0;
            return;
        } else {
            vLogMask |= 1 << flag->second;
        }
    }
}

static bool isExtensionOperandType(int32_t type) {
    return static_cast<uint32_t>(type) > static_cast<uint32_t>(OperandTypeRange::BASE_MAX);
}

static bool isExtensionOperationType(ANeuralNetworksOperationType type) {
    return static_cast<uint32_t>(type) > static_cast<uint32_t>(OperationTypeRange::BASE_MAX);
}

bool isExtensionOperandType(OperandType type) {
    return isExtensionOperandType(static_cast<int32_t>(type));
}

bool isExtensionOperationType(OperationType type) {
    return isExtensionOperationType(static_cast<int32_t>(type));
}

namespace {

template <typename EntryType, uint32_t entryCount, uint32_t entryCountOEM>
EntryType tableLookup(const EntryType (&table)[entryCount],
                      const EntryType (&tableOEM)[entryCountOEM],
                      uint32_t code) {
    if (code < entryCount) {
        return table[code];
    } else if (code >= kOEMCodeBase && (code - kOEMCodeBase) < entryCountOEM) {
        return tableOEM[code - kOEMCodeBase];
    } else {
        nnAssert(!"tableLookup: bad code");
        return EntryType();
    }
}

class OperationValidationContext : public IOperationValidationContext {
    DISALLOW_IMPLICIT_CONSTRUCTORS(OperationValidationContext);

   public:
    OperationValidationContext(uint32_t inputCount, const uint32_t* inputIndexes,
                               uint32_t outputCount, const uint32_t* outputIndexes,
                               const Operand* operands, HalVersion halVersion)
        : inputCount(inputCount),
          inputIndexes(inputIndexes),
          outputCount(outputCount),
          outputIndexes(outputIndexes),
          operands(operands),
          halVersion(halVersion) {}

    HalVersion getHalVersion() const override;

    uint32_t getNumInputs() const override;
    OperandType getInputType(uint32_t index) const override;
    Shape getInputShape(uint32_t index) const override;
    const Operand::ExtraParams getInputExtraParams(uint32_t index) const override;

    uint32_t getNumOutputs() const override;
    OperandType getOutputType(uint32_t index) const override;
    Shape getOutputShape(uint32_t index) const override;

   private:
    const Operand* getInputOperand(uint32_t index) const;
    const Operand* getOutputOperand(uint32_t index) const;

    uint32_t inputCount;
    const uint32_t* inputIndexes;
    uint32_t outputCount;
    const uint32_t* outputIndexes;
    const Operand* operands;
    HalVersion halVersion;
};

HalVersion OperationValidationContext::getHalVersion() const {
    return halVersion;
}

const Operand* OperationValidationContext::getInputOperand(uint32_t index) const {
    CHECK(index < static_cast<uint32_t>(inputCount));
    return &operands[inputIndexes[index]];
}

const Operand* OperationValidationContext::getOutputOperand(uint32_t index) const {
    CHECK(index < static_cast<uint32_t>(outputCount));
    return &operands[outputIndexes[index]];
}

uint32_t OperationValidationContext::getNumInputs() const {
    return inputCount;
}

uint32_t OperationValidationContext::getNumOutputs() const {
    return outputCount;
}

OperandType OperationValidationContext::getInputType(uint32_t index) const {
    return getInputOperand(index)->type;
}

Shape OperationValidationContext::getInputShape(uint32_t index) const {
    const Operand* operand = getInputOperand(index);
    return {operand->type, operand->dimensions, operand->scale, operand->zeroPoint,
            operand->extraParams};
}

const Operand::ExtraParams OperationValidationContext::getInputExtraParams(uint32_t index) const {
    return getInputOperand(index)->extraParams;
}

OperandType OperationValidationContext::getOutputType(uint32_t index) const {
    return getOutputOperand(index)->type;
}

Shape OperationValidationContext::getOutputShape(uint32_t index) const {
    const Operand* operand = getOutputOperand(index);
    return {operand->type, operand->dimensions, operand->scale, operand->zeroPoint,
            operand->extraParams};
}

};  // anonymous namespace

#define COUNT(X) (sizeof(X) / sizeof(X[0]))

std::string getOperandTypeName(OperandType type) {
    return toString(type);
}

static std::string getOperationName(uint32_t code) {
    return getOperationName(static_cast<OperationType>(code));
}

std::string getOperationName(OperationType type) {
    return toString(type);
}

const uint32_t kSizeOfDataType[]{
        4,  // ANEURALNETWORKS_FLOAT32
        4,  // ANEURALNETWORKS_INT32
        4,  // ANEURALNETWORKS_UINT32
        4,  // ANEURALNETWORKS_TENSOR_FLOAT32
        4,  // ANEURALNETWORKS_TENSOR_INT32
        1,  // ANEURALNETWORKS_TENSOR_SYMMETRICAL_QUANT8
        1,  // ANEURALNETWORKS_BOOL
        2,  // ANEURALNETWORKS_TENSOR_QUANT16_SYMM
        2,  // ANEURALNETWORKS_TENSOR_FLOAT16
        1,  // ANEURALNETWORKS_TENSOR_BOOL8
        2,  // ANEURALNETWORKS_FLOAT16
        1,  // ANEURALNETWORKS_TENSOR_QUANT8_SYMM_PER_CHANNEL
        2,  // ANEURALNETWORKS_TENSOR_QUANT16_ASYMM
        1,  // ANEURALNETWORKS_TENSOR_QUANT8_SYMM
};

static_assert(COUNT(kSizeOfDataType) == kNumberOfDataTypes, "kSizeOfDataType is incorrect");

const bool kScalarDataType[]{
        true,   // ANEURALNETWORKS_FLOAT32
        true,   // ANEURALNETWORKS_INT32
        true,   // ANEURALNETWORKS_UINT32
        false,  // ANEURALNETWORKS_TENSOR_FLOAT32
        false,  // ANEURALNETWORKS_TENSOR_INT32
        false,  // ANEURALNETWORKS_TENSOR_SYMMETRICAL_QUANT8
        true,   // ANEURALNETWORKS_BOOL
        false,  // ANEURALNETWORKS_TENSOR_QUANT16_SYMM
        false,  // ANEURALNETWORKS_TENSOR_FLOAT16
        false,  // ANEURALNETWORKS_TENSOR_BOOL8
        true,   // ANEURALNETWORKS_FLOAT16
        false,  // ANEURALNETWORKS_TENSOR_QUANT8_SYMM_PER_CHANNEL
        false,  // ANEURALNETWORKS_TENSOR_QUANT16_ASYMM
        false,  // ANEURALNETWORKS_TENSOR_QUANT8_SYMM
};

static_assert(COUNT(kScalarDataType) == kNumberOfDataTypes, "kScalarDataType is incorrect");

const uint32_t kSizeOfDataTypeOEM[]{
        0, // ANEURALNETWORKS_OEM
        1, // ANEURALNETWORKS_TENSOR_OEM_BYTE
};

static_assert(COUNT(kSizeOfDataTypeOEM) == kNumberOfDataTypesOEM,
              "kSizeOfDataTypeOEM is incorrect");

const bool kScalarDataTypeOEM[]{
        true,  // ANEURALNETWORKS_OEM
        false, // ANEURALNETWORKS_TENSOR_OEM_BYTE
};

static_assert(COUNT(kScalarDataTypeOEM) == kNumberOfDataTypesOEM,
              "kScalarDataTypeOEM is incorrect");

bool nonExtensionOperandTypeIsScalar(int type) {
    CHECK(!isExtensionOperandType(type)) << "Extension operand types are not supported";
    return tableLookup(kScalarDataType, kScalarDataTypeOEM, type);
}

uint32_t nonExtensionOperandSizeOfData(OperandType type, const std::vector<uint32_t>& dimensions) {
    CHECK(!isExtensionOperandType(type)) << "Size of extension operand data is unknown";
    int n = static_cast<int>(type);

    uint32_t size = tableLookup(kSizeOfDataType, kSizeOfDataTypeOEM, n);

    if (tableLookup(kScalarDataType, kScalarDataTypeOEM, n) == true) {
        return size;
    }

    if (dimensions.empty()) {
        return 0;
    }

    for (auto d : dimensions) {
        size *= d;
    }
    return size;
}

bool tensorHasUnspecifiedDimensions(int type, const uint32_t* dim, uint32_t dimCount) {
    if (!isExtensionOperandType(type)) {
        CHECK(!nonExtensionOperandTypeIsScalar(type))
                << "A scalar type can never have unspecified dimensions";
    }
    return dimCount == 0 || std::find(dim, dim + dimCount, 0) != (dim + dimCount);
}

bool tensorHasUnspecifiedDimensions(const ANeuralNetworksOperandType* type) {
    return tensorHasUnspecifiedDimensions(type->type, type->dimensions, type->dimensionCount);
}

bool tensorHasUnspecifiedDimensions(const Operand& operand) {
    return tensorHasUnspecifiedDimensions(static_cast<int>(operand.type), operand.dimensions.data(),
                                          operand.dimensions.size());
}

hidl_memory allocateSharedMemory(int64_t size) {
    static const std::string type = "ashmem";
    static sp<IAllocator> allocator = IAllocator::getService(type);

    hidl_memory memory;

    // TODO: should we align memory size to nearest page? doesn't seem necessary...
    allocator->allocate(size, [&](bool success, const hidl_memory& mem) {
        if (!success) {
            LOG(ERROR) << "unable to allocate " << size << " bytes of " << type;
        } else {
            memory = mem;
        }
    });

    return memory;
}

uint32_t alignBytesNeeded(uint32_t index, size_t length) {
    uint32_t pattern;
    if (length < 2) {
        pattern = 0; // No alignment necessary
    } else if (length < 4) {
        pattern = 1; // Align on 2-byte boundary
    } else {
        pattern = 3; // Align on 4-byte boundary
    }
    uint32_t extra = (~(index - 1)) & pattern;
    return extra;
}

void logModelToInfo(const V1_0::Model& model) {
    LOG(INFO) << "V1_0::Model start";
    LOG(INFO) << "operands" << toString(model.operands);
    LOG(INFO) << "operations" << toString(model.operations);
    LOG(INFO) << "inputIndexes" << toString(model.inputIndexes);
    LOG(INFO) << "outputIndexes" << toString(model.outputIndexes);
    LOG(INFO) << "operandValues size" << model.operandValues.size();
    LOG(INFO) << "pools" << SHOW_IF_DEBUG(toString(model.pools));
}

void logModelToInfo(const V1_1::Model& model) {
    LOG(INFO) << "V1_1::Model start";
    LOG(INFO) << "operands" << toString(model.operands);
    LOG(INFO) << "operations" << toString(model.operations);
    LOG(INFO) << "inputIndexes" << toString(model.inputIndexes);
    LOG(INFO) << "outputIndexes" << toString(model.outputIndexes);
    LOG(INFO) << "operandValues size" << model.operandValues.size();
    LOG(INFO) << "pools" << SHOW_IF_DEBUG(toString(model.pools));
}

bool validateOperandSymmPerChannelQuantParams(
        const Operand& halOperand, const ANeuralNetworksSymmPerChannelQuantParams& channelQuant,
        const char* tag) {
    if (halOperand.type != OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
        return false;
    }

    NN_RET_CHECK_LT(channelQuant.channelDim, halOperand.dimensions.size()) << tag;
    NN_RET_CHECK(channelQuant.scales != nullptr) << tag;
    NN_RET_CHECK_EQ(channelQuant.scaleCount, halOperand.dimensions[channelQuant.channelDim]) << tag;
    NN_RET_CHECK_NE(halOperand.dimensions[channelQuant.channelDim], 0u)
            << tag << " channel dimension " << channelQuant.channelDim << " is underspecified";
    for (uint32_t i = 0; i < halOperand.dimensions[channelQuant.channelDim]; i++) {
        NN_RET_CHECK_GT(channelQuant.scales[i], 0.0f) << tag << " invalid scaleArray[" << i << "]";
    }
    return true;
}

static bool validateScalarDimensions(const ANeuralNetworksOperandType& type, const char* tag) {
    NN_RET_CHECK_EQ(type.dimensionCount, 0u) << tag << " invalid dimensions for scalar type";
    NN_RET_CHECK(type.dimensions == nullptr) << tag << " invalid dimensions for scalar type";
    return true;
}

static bool validateQuant8AsymmParams(const ANeuralNetworksOperandType& type, const char* tag) {
    NN_RET_CHECK(0 <= type.zeroPoint && type.zeroPoint <= 255)
            << tag << " invalid zeroPoint: " << type.zeroPoint;
    NN_RET_CHECK_GT(type.scale, 0.f) << tag << " invalid scale";
    return true;
}

static bool validateQuant8SymmParams(const ANeuralNetworksOperandType& type, const char* tag) {
    NN_RET_CHECK_EQ(type.zeroPoint, 0) << tag << " invalid zeroPoint: " << type.zeroPoint;
    NN_RET_CHECK_GT(type.scale, 0.f) << tag << " invalid scale";
    return true;
}

static bool validateQuant16AsymmParams(const ANeuralNetworksOperandType& type, const char* tag) {
    NN_RET_CHECK(0 <= type.zeroPoint && type.zeroPoint <= 65535)
            << tag << " invalid zeroPoint: " << type.zeroPoint;
    NN_RET_CHECK_GT(type.scale, 0.f) << tag << " invalid scale";
    return true;
}

static bool validateQuantSymmParams(const ANeuralNetworksOperandType& type, const char* tag) {
    NN_RET_CHECK_EQ(type.zeroPoint, 0) << tag << " zeroPoint is not zero";
    NN_RET_CHECK_GT(type.scale, 0.f) << tag << " invalid scale";
    return true;
}

static bool validateNoQuantParams(const ANeuralNetworksOperandType& type, const char* tag) {
    NN_RET_CHECK_EQ(type.zeroPoint, 0) << tag << " zeroPoint is not zero";
    NN_RET_CHECK_EQ(type.scale, 0.f) << tag << " scale is not zero";
    return true;
}

static bool validateTensorDimensions(const ANeuralNetworksOperandType& type, const char* tag,
                                     bool allowPartial) {
    if (allowPartial) {
        return true;
    }
    NN_RET_CHECK_GT(type.dimensionCount, 0u) << tag << " invalid operand dimensions";
    for (uint32_t i = 0; i < type.dimensionCount; i++) {
        NN_RET_CHECK_NE(type.dimensions[i], 0u) << tag << " invalid operand dimensions";
    }
    return true;
}

static bool validateOperandTypeHelper(
        const ANeuralNetworksOperandType& type,
        const Extension::OperandTypeInformation* const extensionOperandTypeInfo, const char* tag,
        bool allowPartial) {
    NN_RET_CHECK_EQ(type.dimensionCount == 0, type.dimensions == nullptr);
    if (isExtensionOperandType(type.type)) {
        NN_RET_CHECK(extensionOperandTypeInfo != nullptr);
        if (extensionOperandTypeInfo->isTensor) {
            NN_RET_CHECK(validateTensorDimensions(type, tag, allowPartial));
        } else {
            NN_RET_CHECK(validateScalarDimensions(type, tag));
        }
        return validateNoQuantParams(type, tag);
    }

    NN_RET_CHECK(extensionOperandTypeInfo == nullptr);
    NN_RET_CHECK(validCode(kNumberOfDataTypes, kNumberOfDataTypesOEM, type.type))
            << tag << " invalid OperandType: " << type.type;

    bool isScalar = tableLookup(kScalarDataType, kScalarDataTypeOEM, type.type);
    if (isScalar) {
        NN_RET_CHECK(validateScalarDimensions(type, tag));
        if (type.type != ANEURALNETWORKS_OEM_SCALAR) {  // Historically, we have allowed OEM types
                                                        // to use quantization parameters.
            NN_RET_CHECK(validateNoQuantParams(type, tag));
        }
    } else {
        NN_RET_CHECK(validateTensorDimensions(type, tag, allowPartial));
        if (type.type == ANEURALNETWORKS_TENSOR_QUANT8_ASYMM) {
            NN_RET_CHECK(validateQuant8AsymmParams(type, tag));
        } else if (type.type == ANEURALNETWORKS_TENSOR_QUANT8_SYMM) {
            NN_RET_CHECK(validateQuant8SymmParams(type, tag));
        } else if (type.type == ANEURALNETWORKS_TENSOR_QUANT16_ASYMM) {
            NN_RET_CHECK(validateQuant16AsymmParams(type, tag));
        } else if (type.type == ANEURALNETWORKS_TENSOR_QUANT16_SYMM) {
            NN_RET_CHECK(validateQuantSymmParams(type, tag));
        } else if (type.type == ANEURALNETWORKS_TENSOR_INT32) {
            // TODO(b/119869082): TENSOR_INT32 should not use quantization parameters.
        } else if (type.type == ANEURALNETWORKS_TENSOR_OEM_BYTE) {
            // Historically, we have allowed OEM types to use quantization parameters.
        } else {
            NN_RET_CHECK(validateNoQuantParams(type, tag));
        }
    }

    return true;
}

int validateOperandType(const ANeuralNetworksOperandType& type,
                        const Extension::OperandTypeInformation* const extensionOperandTypeInfo,
                        const char* tag, bool allowPartial) {
    return validateOperandTypeHelper(type, extensionOperandTypeInfo, tag, allowPartial)
                   ? ANEURALNETWORKS_NO_ERROR
                   : ANEURALNETWORKS_BAD_DATA;
}

int validateOperandList(uint32_t count, const uint32_t* list, uint32_t operandCount,
                        const char* tag) {
    for (uint32_t i = 0; i < count; i++) {
        if (list[i] >= operandCount) {
            LOG(ERROR) << tag << " invalid operand index at " << i << " = " << list[i]
                       << ", operandCount " << operandCount;
            return ANEURALNETWORKS_BAD_DATA;
        }
    }
    return ANEURALNETWORKS_NO_ERROR;
}

int validateOperationOperandTypes(const std::vector<Operand>& operands,
                                  uint32_t inOperandCount, const uint32_t* inOperandIndexes,
                                  const std::vector<OperandType>& inExpectedTypes,
                                  uint32_t outOperandCount, const uint32_t* outOperandIndexes,
                                  const std::vector<OperandType>& outExpectedInTypes) {
    if (inOperandCount != static_cast<uint32_t>(inExpectedTypes.size()) ||
        outOperandCount != static_cast<uint32_t>(outExpectedInTypes.size())) {
        LOG(ERROR) << "Wrong operand count: expected " << inExpectedTypes.size() << " inputs and "
                   << outExpectedInTypes.size() << " outputs,"
                   << "got " << inOperandCount << " inputs and " << outOperandCount << " outputs";
        return ANEURALNETWORKS_BAD_DATA;
    }
    for (uint32_t i = 0; i < inOperandCount; i++) {
        if (operands[inOperandIndexes[i]].type != inExpectedTypes[i]) {
            LOG(ERROR) << "Invalid input tensor type "
                       << toString(operands[inOperandIndexes[i]].type)
                       << " for input " << i << ", expected " << toString(inExpectedTypes[i]);
            return ANEURALNETWORKS_BAD_DATA;
        }
    }
    for (uint32_t i = 0; i < outOperandCount; i++) {
        if (operands[outOperandIndexes[i]].type != outExpectedInTypes[i]) {
            LOG(ERROR) << "Invalid output tensor type "
                       << toString(operands[outOperandIndexes[i]].type)
                       << " for input " << i << ", expected " << toString(outExpectedInTypes[i]);
            return ANEURALNETWORKS_BAD_DATA;
        }
    }

    return ANEURALNETWORKS_NO_ERROR;
}

static int validateHalVersion(ANeuralNetworksOperationType opType, HalVersion halVersion,
                              HalVersion minSupportedHalVersion) {
    if (halVersion < minSupportedHalVersion) {
        LOG(ERROR) << "The given inputs and outputs for operation " << getOperationName(opType)
                   << " are only supported in " << toString(minSupportedHalVersion)
                   << " and later (validating using " << toString(halVersion) << ")";
        return ANEURALNETWORKS_BAD_DATA;
    }
    return ANEURALNETWORKS_NO_ERROR;
}

int validateOperation(ANeuralNetworksOperationType opType, uint32_t inputCount,
                      const uint32_t* inputIndexes, uint32_t outputCount,
                      const uint32_t* outputIndexes, const std::vector<Operand>& operands,
                      HalVersion halVersion) {
    NN_RETURN_IF_ERROR(validateOperandList(inputCount, inputIndexes,
                                           static_cast<uint32_t>(operands.size()),
                                           "ANeuralNetworksModel_addOperation inputs"));
    NN_RETURN_IF_ERROR(validateOperandList(outputCount, outputIndexes,
                                           static_cast<uint32_t>(operands.size()),
                                           "ANeuralNetworksModel_addOperation outputs"));

    if (isExtensionOperationType(opType)) {
        if (halVersion < HalVersion::V1_2) {
            LOG(ERROR)
                    << "Extension operations are supported since HAL version 1.2, validating using "
                    << toString(halVersion);
            return ANEURALNETWORKS_BAD_DATA;
        }
        // There is no other validation we can do for an extension operation.
        return ANEURALNETWORKS_NO_ERROR;
    }

    auto logInvalidInOutNumber = [opType, inputCount, outputCount](int expIn, int expOut) {
        LOG(ERROR) << "Invalid number of input operands (" << inputCount << ", expected " << expIn
                   << ") or output operands (" << outputCount << ", expected " << expOut
                   << ") for operation " << getOperationName(opType);
    };

    switch (opType) {
        case ANEURALNETWORKS_OEM_OPERATION: {
            return ANEURALNETWORKS_NO_ERROR;
        }
        case ANEURALNETWORKS_FLOOR: {
            if (inputCount != 1 || outputCount != 1) {
                logInvalidInOutNumber(1, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
                inExpectedTypes = {OperandType::TENSOR_FLOAT32};
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {OperandType::TENSOR_FLOAT16};
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            return validateOperationOperandTypes(operands,
                                                 inputCount, inputIndexes,
                                                 inExpectedTypes,
                                                 outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_DEPTHWISE_CONV_2D: {
            if ((inputCount != 14 && inputCount != 12 && inputCount != 11 && inputCount != 9 &&
                 inputCount != 8) ||
                outputCount != 1) {
                LOG(ERROR) << "Invalid number of input operands (" << inputCount
                           << ", expected 14, 12, 11, 9 or 8) or output operands (" << outputCount
                           << ", expected 1) for operation " << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            auto filterType = operands[inputIndexes[1]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT32, OperandType::TENSOR_FLOAT32,
                        OperandType::TENSOR_FLOAT32, OperandType::INT32,
                        OperandType::INT32,          OperandType::INT32,
                        OperandType::INT32,          OperandType::INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT16, OperandType::TENSOR_FLOAT16,
                        OperandType::TENSOR_FLOAT16, OperandType::INT32,
                        OperandType::INT32,          OperandType::INT32,
                        OperandType::INT32,          OperandType::INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                if (filterType != OperandType::TENSOR_QUANT8_ASYMM &&
                    filterType != OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
                    LOG(ERROR) << "Unsupported filter tensor type for operation "
                               << getOperationName(opType);
                    return ANEURALNETWORKS_BAD_DATA;
                }

                inExpectedTypes = {
                        OperandType::TENSOR_QUANT8_ASYMM,
                        filterType,
                        OperandType::TENSOR_INT32,
                        OperandType::INT32,
                        OperandType::INT32,
                        OperandType::INT32,
                        OperandType::INT32,
                        OperandType::INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }

            // NeuralNetworks.h specifies that ANEURALNETWORKS_DEPTHWISE_CONV_2D's output must
            // meet "outputScale > inputScale * filterScale" for the operand type
            // ANEURALNETWORKS_TENSOR_QUANT8_ASYMM before API level 29. For other
            // operand types (e.g., ANEURALNETWORKS_TENSOR_FLOAT32), this constraint
            // does not apply, so by default the constraint is met.
            bool meetsQuantizedScaleConstraintBeforeV1_2 = true;
            if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                const float inputScale = operands[inputIndexes[0]].scale;
                const float filterScale = operands[inputIndexes[1]].scale;
                const float outputScale = operands[outputIndexes[0]].scale;
                meetsQuantizedScaleConstraintBeforeV1_2 = (outputScale > inputScale * filterScale);
            }

            bool withExplicitPadding = false;
            bool withLayout = false;
            bool withDilation = false;
            if (inputCount >= 9) {
                if (operands[inputIndexes[8]].type == OperandType::INT32 && inputCount >= 11) {
                    std::vector<OperandType> explicitScalarTypes(3, OperandType::INT32);
                    inExpectedTypes.insert(inExpectedTypes.end(), explicitScalarTypes.begin(),
                                           explicitScalarTypes.end());
                    withExplicitPadding = true;
                }
                int inputOffset = withExplicitPadding ? 3 : 0;
                if (inputCount >= 9 + inputOffset) {
                    inExpectedTypes.push_back(OperandType::BOOL);
                    withLayout = true;
                }
                if (inputCount == 10 + inputOffset) {
                    LOG(ERROR) << "Provided only one dilation factor value, two values are requred "
                                  "for operation "
                               << getOperationName(opType);
                    return ANEURALNETWORKS_BAD_DATA;
                }
                if (inputCount == 11 + inputOffset) {
                    inExpectedTypes.push_back(OperandType::INT32);
                    inExpectedTypes.push_back(OperandType::INT32);
                    withDilation = true;
                }
            }

            if (inputType == OperandType::TENSOR_FLOAT16 ||
                filterType == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL || withLayout ||
                withDilation || !meetsQuantizedScaleConstraintBeforeV1_2) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            } else {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
            }
            return validateOperationOperandTypes(operands,
                                                 inputCount, inputIndexes,
                                                 inExpectedTypes,
                                                 outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_LOCAL_RESPONSE_NORMALIZATION: {
            if ((inputCount != 6 && inputCount != 5) || outputCount != 1) {
                LOG(ERROR) << "Invalid number of input operands (" << inputCount
                           << ", expected 6 or 5) or output operands (" << outputCount
                           << ", expected 1) for operation " << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT32, OperandType::INT32,   OperandType::FLOAT32,
                        OperandType::FLOAT32,        OperandType::FLOAT32,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT16, OperandType::INT32,   OperandType::FLOAT16,
                        OperandType::FLOAT16,        OperandType::FLOAT16,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            if (inputCount == 6) {
                inExpectedTypes.push_back(OperandType::INT32);
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            } else if (operands[inputIndexes[0]].dimensions.size() != 4) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            }
            return validateOperationOperandTypes(operands,
                                                 inputCount, inputIndexes,
                                                 inExpectedTypes,
                                                 outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_RESHAPE: {
            if (inputCount != 2 || outputCount != 1) {
                logInvalidInOutNumber(2, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
                inExpectedTypes = {OperandType::TENSOR_FLOAT32,
                                   OperandType::TENSOR_INT32};
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {OperandType::TENSOR_FLOAT16, OperandType::TENSOR_INT32};
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
                inExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM,
                                   OperandType::TENSOR_INT32};
                outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            return validateOperationOperandTypes(operands,
                                                 inputCount, inputIndexes,
                                                 inExpectedTypes,
                                                 outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_DEPTH_TO_SPACE: {
            if ((inputCount != 3 && inputCount != 2) || outputCount != 1) {
                LOG(ERROR) << "Invalid number of input operands (" << inputCount
                           << ", expected 3 or 2) or output operands (" << outputCount
                           << ", expected 1) for operation " << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
                inExpectedTypes = {OperandType::TENSOR_FLOAT32,
                                   OperandType::INT32};
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {OperandType::TENSOR_FLOAT16, OperandType::INT32};
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
                inExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM,
                                   OperandType::INT32};
                outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            if (inputCount == 3) {
                inExpectedTypes.push_back(OperandType::BOOL);
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            } else {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
            }
            return validateOperationOperandTypes(operands,
                                                 inputCount, inputIndexes,
                                                 inExpectedTypes,
                                                 outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_SPACE_TO_DEPTH: {
            if ((inputCount != 3 && inputCount != 2) || outputCount != 1) {
                LOG(ERROR) << "Invalid number of input operands (" << inputCount
                           << ", expected 3 or 2) or output operands (" << outputCount
                           << ", expected 1) for operation " << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
                inExpectedTypes = {OperandType::TENSOR_FLOAT32,
                                   OperandType::INT32};
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {OperandType::TENSOR_FLOAT16, OperandType::INT32};
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
                inExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM,
                                   OperandType::INT32};
                outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            if (inputCount == 3) {
                inExpectedTypes.push_back(OperandType::BOOL);
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            } else {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
            }
            return validateOperationOperandTypes(operands,
                                                 inputCount, inputIndexes,
                                                 inExpectedTypes,
                                                 outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_EMBEDDING_LOOKUP: {
            if (inputCount != 2 || outputCount != 1) {
                logInvalidInOutNumber(2, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[1]].type;
            if (inputType != OperandType::TENSOR_FLOAT32 &&
                inputType != OperandType::TENSOR_INT32 &&
                inputType != OperandType::TENSOR_QUANT8_ASYMM) {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            std::vector<OperandType> inExpectedTypes = {OperandType::TENSOR_INT32,
                                                        inputType};
            std::vector<OperandType> outExpectedTypes = {inputType};
            NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
            return validateOperationOperandTypes(operands,
                                                 inputCount, inputIndexes,
                                                 inExpectedTypes,
                                                 outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_HASHTABLE_LOOKUP: {
            if (inputCount != 3 || outputCount != 2) {
                logInvalidInOutNumber(3, 2);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[2]].type;
            if (inputType != OperandType::TENSOR_FLOAT32 &&
                inputType != OperandType::TENSOR_INT32 &&
                inputType != OperandType::TENSOR_QUANT8_ASYMM) {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            std::vector<OperandType> inExpectedTypes = {OperandType::TENSOR_INT32,
                                                        OperandType::TENSOR_INT32,
                                                        inputType};
            std::vector<OperandType> outExpectedTypes = {inputType,
                                                         OperandType::TENSOR_QUANT8_ASYMM};
            NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
            return validateOperationOperandTypes(operands,
                                                 inputCount, inputIndexes,
                                                 inExpectedTypes,
                                                 outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_LSH_PROJECTION: {
            if (inputCount != 4 || outputCount != 1) {
                logInvalidInOutNumber(4, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[1]].type;
            if (inputType != OperandType::TENSOR_FLOAT16 &&
                inputType != OperandType::TENSOR_FLOAT32 &&
                inputType != OperandType::TENSOR_INT32 &&
                inputType != OperandType::TENSOR_QUANT8_ASYMM) {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto hashType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            if (hashType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT16,
                        inputType,
                        OperandType::TENSOR_FLOAT16,
                        OperandType::INT32,
                };
            } else if (hashType == OperandType::TENSOR_FLOAT32) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT32,
                        inputType,
                        OperandType::TENSOR_FLOAT32,
                        OperandType::INT32,
                };
            } else {
                LOG(ERROR) << "Unsupported hash tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            std::vector<OperandType> outExpectedTypes = {OperandType::TENSOR_INT32};
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_BIDIRECTIONAL_SEQUENCE_LSTM: {
            std::vector<OperandType> inExpectedTypes;
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> outExpectedTypes{inputType, inputType};
            std::vector<OperandType> outExpectedTypesMerged{inputType};
            if (inputType != OperandType::TENSOR_FLOAT32 &&
                inputType != OperandType::TENSOR_FLOAT16) {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));

            inExpectedTypes = {};
            for (int i = 0; i < 48; ++i) {
                inExpectedTypes.push_back(inputType);
            }
            inExpectedTypes.push_back(OperandType::INT32);
            inExpectedTypes.push_back(inputType == OperandType::TENSOR_FLOAT32
                                              ? OperandType::FLOAT32
                                              : OperandType::FLOAT16);
            inExpectedTypes.push_back(inputType == OperandType::TENSOR_FLOAT32
                                              ? OperandType::FLOAT32
                                              : OperandType::FLOAT16);
            inExpectedTypes.push_back(OperandType::BOOL);
            inExpectedTypes.push_back(OperandType::BOOL);
            for (int i = 0; i < 8; ++i) {
                inExpectedTypes.push_back(inputType);
            }

            if (inputCount != 61 || (outputCount != 1 && outputCount != 2)) {
                LOG(ERROR) << "Invalid number of input operands (" << inputCount
                           << ", expected 61) or output operands (" << outputCount
                           << ", expected 1 or 2) for operation " << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto status = validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                        inExpectedTypes, outputCount, outputIndexes,
                                                        outExpectedTypes);
            if (status != ANEURALNETWORKS_NO_ERROR) {
                status = validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                       inExpectedTypes, outputCount, outputIndexes,
                                                       outExpectedTypesMerged);
            }
            return status;
        }
        case ANEURALNETWORKS_LSTM: {
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            auto inputType = operands[inputIndexes[0]].type;
            if (inputType != OperandType::TENSOR_FLOAT32 &&
                inputType != OperandType::TENSOR_FLOAT16) {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }

            inExpectedTypes = {inputType,         inputType, inputType, inputType, inputType,
                               inputType,         inputType, inputType, inputType, inputType,
                               inputType,         inputType, inputType, inputType, inputType,
                               inputType,         inputType, inputType, inputType, inputType,
                               OperandType::INT32};
            if (inputType == OperandType::TENSOR_FLOAT32) {
                inExpectedTypes.push_back(OperandType::FLOAT32);
                inExpectedTypes.push_back(OperandType::FLOAT32);
            } else {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes.push_back(OperandType::FLOAT16);
                inExpectedTypes.push_back(OperandType::FLOAT16);
            }

            outExpectedTypes = {inputType, inputType, inputType, inputType};
            if (inputCount == 23 && outputCount == 4) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
            } else if (inputCount == 27 && outputCount == 4) {
                for (int i = 0; i < 4; ++i) {
                    inExpectedTypes.push_back(inputType);
                }
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            } else {
                LOG(ERROR) << "Invalid number of input operands (" << inputCount
                           << ", expected 23 or 27) or output operands (" << outputCount
                           << ", expected 4) for operation " << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_QUANTIZED_16BIT_LSTM: {
            if (inputCount != 15 || outputCount != 2) {
                logInvalidInOutNumber(15, 2);
                return ANEURALNETWORKS_BAD_DATA;
            }
            NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            std::vector<OperandType> inExpectedTypes = {
                    OperandType::TENSOR_QUANT8_ASYMM, OperandType::TENSOR_QUANT8_ASYMM,
                    OperandType::TENSOR_QUANT8_ASYMM, OperandType::TENSOR_QUANT8_ASYMM,
                    OperandType::TENSOR_QUANT8_ASYMM, OperandType::TENSOR_QUANT8_ASYMM,
                    OperandType::TENSOR_QUANT8_ASYMM, OperandType::TENSOR_QUANT8_ASYMM,
                    OperandType::TENSOR_QUANT8_ASYMM, OperandType::TENSOR_INT32,
                    OperandType::TENSOR_INT32,        OperandType::TENSOR_INT32,
                    OperandType::TENSOR_INT32,        OperandType::TENSOR_QUANT16_SYMM,
                    OperandType::TENSOR_QUANT8_ASYMM};
            std::vector<OperandType> outExpectedTypes = {OperandType::TENSOR_QUANT16_SYMM,
                                                         OperandType::TENSOR_QUANT8_ASYMM};
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_RANDOM_MULTINOMIAL: {
            if (inputCount != 3 || outputCount != 1) {
                logInvalidInOutNumber(3, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            OperandType inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32 ||
                inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {
                        inputType,
                        OperandType::INT32,
                        OperandType::TENSOR_INT32,
                };
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            std::vector<OperandType> outExpectedTypes = {OperandType::TENSOR_INT32};
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_RNN: {
            if (inputCount != 6 || outputCount != 2) {
                logInvalidInOutNumber(6, 2);
                return ANEURALNETWORKS_BAD_DATA;
            }
            OperandType inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT32, OperandType::TENSOR_FLOAT32,
                        OperandType::TENSOR_FLOAT32, OperandType::TENSOR_FLOAT32,
                        OperandType::TENSOR_FLOAT32, OperandType::INT32,
                };
                outExpectedTypes = {
                        OperandType::TENSOR_FLOAT32,
                        OperandType::TENSOR_FLOAT32,
                };
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT16, OperandType::TENSOR_FLOAT16,
                        OperandType::TENSOR_FLOAT16, OperandType::TENSOR_FLOAT16,
                        OperandType::TENSOR_FLOAT16, OperandType::INT32,
                };
                outExpectedTypes = {
                        OperandType::TENSOR_FLOAT16,
                        OperandType::TENSOR_FLOAT16,
                };
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_SVDF: {
            if (inputCount != 7 || outputCount != 2) {
                logInvalidInOutNumber(7, 2);
                return ANEURALNETWORKS_BAD_DATA;
            }
            OperandType inputType = operands[inputIndexes[0]].type;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_0));

            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            std::vector<OperandType> inExpectedTypes = {
                    inputType, inputType,          inputType,          inputType,
                    inputType, OperandType::INT32, OperandType::INT32,
            };
            std::vector<OperandType> outExpectedTypes = {inputType, inputType};
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_BATCH_TO_SPACE_ND: {
            if ((inputCount != 3 && inputCount != 2) || outputCount != 1) {
                LOG(ERROR) << "Invalid number of input operands (" << inputCount
                           << ", expected 3 or 2) or output operands (" << outputCount
                           << ", expected 1) for operation " << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT32,
                        OperandType::TENSOR_INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT16,
                        OperandType::TENSOR_INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                inExpectedTypes = {
                        OperandType::TENSOR_QUANT8_ASYMM,
                        OperandType::TENSOR_INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            if (inputCount == 3) {
                inExpectedTypes.push_back(OperandType::BOOL);
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            } else {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_1));
            }
            return validateOperationOperandTypes(operands,
                                                 inputCount, inputIndexes,
                                                 inExpectedTypes,
                                                 outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_SPACE_TO_BATCH_ND: {
            if ((inputCount != 4 && inputCount != 3) || outputCount != 1) {
                LOG(ERROR) << "Invalid number of input operands (" << inputCount
                           << ", expected 4 or 3) or output operands (" << outputCount
                           << ", expected 1) for operation " << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT32,
                        OperandType::TENSOR_INT32,
                        OperandType::TENSOR_INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT16,
                        OperandType::TENSOR_INT32,
                        OperandType::TENSOR_INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                if (operands[inputIndexes[0]].zeroPoint != 0) {
                    NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                }
                inExpectedTypes = {
                        OperandType::TENSOR_QUANT8_ASYMM,
                        OperandType::TENSOR_INT32,
                        OperandType::TENSOR_INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            if (inputCount == 4) {
                inExpectedTypes.push_back(OperandType::BOOL);
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            } else {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_1));
            }
            return validateOperationOperandTypes(operands,
                                                 inputCount, inputIndexes,
                                                 inExpectedTypes,
                                                 outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_PAD: {
            if (inputCount != 2 || outputCount != 1) {
                logInvalidInOutNumber(2, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_1));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT32,
                        OperandType::TENSOR_INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT16,
                        OperandType::TENSOR_INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                if (operands[inputIndexes[0]].zeroPoint == 0) {
                    NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_1));
                } else {
                    NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                }
                inExpectedTypes = {
                        OperandType::TENSOR_QUANT8_ASYMM,
                        OperandType::TENSOR_INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_PAD_V2: {
            if (inputCount != 3 || outputCount != 1) {
                logInvalidInOutNumber(3, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT32,
                        OperandType::TENSOR_INT32,
                        OperandType::FLOAT32,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT16,
                        OperandType::TENSOR_INT32,
                        OperandType::FLOAT16,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {
                        OperandType::TENSOR_QUANT8_ASYMM,
                        OperandType::TENSOR_INT32,
                        OperandType::INT32,
                };  // TODO(b/116699425): Make it UINT8.
                outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_CAST: {
            if (inputCount != 1 || outputCount != 1) {
                logInvalidInOutNumber(1, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            auto outputType = operands[outputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT16 ||
                inputType == OperandType::TENSOR_FLOAT32 ||
                inputType == OperandType::TENSOR_INT32 ||
                inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                inExpectedTypes = {inputType};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            std::vector<OperandType> outExpectedTypes;
            if (outputType == OperandType::TENSOR_FLOAT16 ||
                outputType == OperandType::TENSOR_FLOAT32 ||
                outputType == OperandType::TENSOR_INT32 ||
                outputType == OperandType::TENSOR_QUANT8_ASYMM) {
                outExpectedTypes = {outputType};
            } else {
                LOG(ERROR) << "Unsupported output tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_SQUEEZE: {
            if (inputCount != 2 || outputCount != 1) {
                logInvalidInOutNumber(2, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_1));
                inExpectedTypes = {OperandType::TENSOR_FLOAT32,
                                   OperandType::TENSOR_INT32};
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {OperandType::TENSOR_FLOAT16, OperandType::TENSOR_INT32};
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_1));
                inExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM,
                                   OperandType::TENSOR_INT32};
                outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            return validateOperationOperandTypes(operands,
                                                 inputCount, inputIndexes,
                                                 inExpectedTypes,
                                                 outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_STRIDED_SLICE: {
            if (inputCount != 7 || outputCount != 1) {
                logInvalidInOutNumber(7, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_1));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT32, OperandType::TENSOR_INT32,
                        OperandType::TENSOR_INT32,   OperandType::TENSOR_INT32,
                        OperandType::INT32,          OperandType::INT32,
                        OperandType::INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {
                        OperandType::TENSOR_FLOAT16, OperandType::TENSOR_INT32,
                        OperandType::TENSOR_INT32,   OperandType::TENSOR_INT32,
                        OperandType::INT32,          OperandType::INT32,
                        OperandType::INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_1));
                inExpectedTypes = {
                        OperandType::TENSOR_QUANT8_ASYMM,
                        OperandType::TENSOR_INT32,
                        OperandType::TENSOR_INT32,
                        OperandType::TENSOR_INT32,
                        OperandType::INT32,
                        OperandType::INT32,
                        OperandType::INT32,
                };
                outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_MEAN: {
            if (inputCount != 3 || outputCount != 1) {
                logInvalidInOutNumber(3, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_1));
                inExpectedTypes = {OperandType::TENSOR_FLOAT32,
                                   OperandType::TENSOR_INT32,
                                   OperandType::INT32};
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
                inExpectedTypes = {OperandType::TENSOR_FLOAT16, OperandType::TENSOR_INT32,
                                   OperandType::INT32};
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_1));
                inExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM,
                                   OperandType::TENSOR_INT32,
                                   OperandType::INT32};
                outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            return validateOperationOperandTypes(operands,
                                                 inputCount, inputIndexes,
                                                 inExpectedTypes,
                                                 outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_ARGMAX:
        case ANEURALNETWORKS_ARGMIN: {
            if (inputCount != 2 || outputCount != 1) {
                logInvalidInOutNumber(2, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT16 ||
                inputType == OperandType::TENSOR_FLOAT32 ||
                inputType == OperandType::TENSOR_INT32 ||
                inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                inExpectedTypes = {inputType, OperandType::INT32};
                outExpectedTypes = {OperandType::TENSOR_INT32};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_EXPAND_DIMS: {
            if (inputCount != 2 || outputCount != 1) {
                logInvalidInOutNumber(2, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT16 ||
                inputType == OperandType::TENSOR_FLOAT32 ||
                inputType == OperandType::TENSOR_INT32 ||
                inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                inExpectedTypes = {inputType, OperandType::INT32};
                outExpectedTypes = {inputType};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_SPLIT: {
            if (inputCount != 3) {
                LOG(ERROR) << "Invalid number of input operands (" << inputCount << ", expected 3)"
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            if (inputType != OperandType::TENSOR_FLOAT16 &&
                inputType != OperandType::TENSOR_FLOAT32 &&
                inputType != OperandType::TENSOR_INT32 &&
                inputType != OperandType::TENSOR_QUANT8_ASYMM) {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            std::vector<OperandType> inExpectedTypes = {inputType, OperandType::INT32,
                                                        OperandType::INT32};
            std::vector<OperandType> outExpectedTypes(outputCount, inputType);
            NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_MAXIMUM:
        case ANEURALNETWORKS_MINIMUM: {
            if (inputCount != 2 || outputCount != 1) {
                logInvalidInOutNumber(2, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            OperandType inputType = operands[inputIndexes[0]].type;
            if (inputType == OperandType::TENSOR_FLOAT16 ||
                inputType == OperandType::TENSOR_FLOAT32 ||
                inputType == OperandType::TENSOR_INT32 ||
                inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                inExpectedTypes = {inputType, inputType};
                outExpectedTypes = {inputType};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_GROUPED_CONV_2D: {
            if ((inputCount != 12 && inputCount != 9) || outputCount != 1) {
                LOG(ERROR) << "Invalid number of input operands (" << inputCount
                           << ", expected 12 or 9) or output operands (" << outputCount
                           << ", expected 1) for operation " << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            auto filterType = operands[inputIndexes[1]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT32) {
                inExpectedTypes = {OperandType::TENSOR_FLOAT32, OperandType::TENSOR_FLOAT32,
                                   OperandType::TENSOR_FLOAT32, OperandType::INT32,
                                   OperandType::INT32,          OperandType::INT32,
                                   OperandType::INT32,          OperandType::INT32};
                outExpectedTypes = {OperandType::TENSOR_FLOAT32};
            } else if (inputType == OperandType::TENSOR_FLOAT16) {
                inExpectedTypes = {OperandType::TENSOR_FLOAT16, OperandType::TENSOR_FLOAT16,
                                   OperandType::TENSOR_FLOAT16, OperandType::INT32,
                                   OperandType::INT32,          OperandType::INT32,
                                   OperandType::INT32,          OperandType::INT32};
                outExpectedTypes = {OperandType::TENSOR_FLOAT16};
            } else if (inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                if (filterType != OperandType::TENSOR_QUANT8_ASYMM &&
                    filterType != OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
                    LOG(ERROR) << "Unsupported filter tensor type for operation "
                               << getOperationName(opType);
                    return ANEURALNETWORKS_BAD_DATA;
                }

                if (filterType == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL &&
                    operands[inputIndexes[1]].extraParams.channelQuant().channelDim != 0) {
                    LOG(ERROR) << "Unsupported filter tensor channel dimension for operation "
                               << getOperationName(opType);
                    return ANEURALNETWORKS_BAD_DATA;
                }

                inExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM,
                                   filterType,
                                   OperandType::TENSOR_INT32,
                                   OperandType::INT32,
                                   OperandType::INT32,
                                   OperandType::INT32,
                                   OperandType::INT32,
                                   OperandType::INT32};
                outExpectedTypes = {OperandType::TENSOR_QUANT8_ASYMM};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }

            if (inputCount == 12) {
                std::vector<OperandType> explicitScalarTypes(3, OperandType::INT32);
                inExpectedTypes.insert(inExpectedTypes.end(), explicitScalarTypes.begin(),
                                       explicitScalarTypes.end());
            }
            inExpectedTypes.push_back(OperandType::BOOL);
            NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_TILE: {
            if (inputCount != 2 || outputCount != 1) {
                logInvalidInOutNumber(2, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT16 ||
                inputType == OperandType::TENSOR_FLOAT32 ||
                inputType == OperandType::TENSOR_INT32 ||
                inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                inExpectedTypes = {inputType, OperandType::TENSOR_INT32};
                outExpectedTypes = {inputType};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_POW: {
            if (inputCount != 2 || outputCount != 1) {
                logInvalidInOutNumber(2, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            auto inputType = operands[inputIndexes[0]].type;
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            if (inputType == OperandType::TENSOR_FLOAT16 ||
                inputType == OperandType::TENSOR_FLOAT32) {
                inExpectedTypes = {inputType, inputType};
                outExpectedTypes = {inputType};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        case ANEURALNETWORKS_TOPK_V2: {
            if (inputCount != 2 || outputCount != 2) {
                logInvalidInOutNumber(2, 1);
                return ANEURALNETWORKS_BAD_DATA;
            }
            std::vector<OperandType> inExpectedTypes;
            std::vector<OperandType> outExpectedTypes;
            OperandType inputType = operands[inputIndexes[0]].type;
            if (inputType == OperandType::TENSOR_FLOAT16 ||
                inputType == OperandType::TENSOR_FLOAT32 ||
                inputType == OperandType::TENSOR_INT32 ||
                inputType == OperandType::TENSOR_QUANT8_ASYMM) {
                inExpectedTypes = {inputType, OperandType::INT32};
                outExpectedTypes = {inputType, OperandType::TENSOR_INT32};
            } else {
                LOG(ERROR) << "Unsupported input tensor type for operation "
                           << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            NN_RETURN_IF_ERROR(validateHalVersion(opType, halVersion, HalVersion::V1_2));
            return validateOperationOperandTypes(operands, inputCount, inputIndexes,
                                                 inExpectedTypes, outputCount, outputIndexes,
                                                 outExpectedTypes);
        }
        default: {
            const OperationRegistration* operationRegistration =
                    BuiltinOperationResolver::get()->findOperation(
                            static_cast<OperationType>(opType));
            if (operationRegistration == nullptr) {
                if (0 <= opType && opType < kNumberOfOperationTypes) {
                    LOG(ERROR) << getOperationName(opType) << " not registered";
                } else {
                    LOG(ERROR) << "Operation type " << opType << " out of the range [0, "
                               << kNumberOfOperationTypes << ")";
                }
                return ANEURALNETWORKS_UNEXPECTED_NULL;
            }
            if (operationRegistration->validate == nullptr) {
                LOG(ERROR) << "Incomplete operation registration: " << getOperationName(opType);
                return ANEURALNETWORKS_UNEXPECTED_NULL;
            }
            OperationValidationContext context(inputCount, inputIndexes, outputCount, outputIndexes,
                                               operands.data(), halVersion);
            if (!operationRegistration->validate(&context)) {
                LOG(ERROR) << "Validation failed for operation " << getOperationName(opType);
                return ANEURALNETWORKS_BAD_DATA;
            }
            return ANEURALNETWORKS_NO_ERROR;
        }
    }
}

ErrorStatus convertResultCodeToErrorStatus(int resultCode) {
    switch (resultCode) {
        case ANEURALNETWORKS_NO_ERROR:
            return ErrorStatus::NONE;

        case ANEURALNETWORKS_BAD_DATA:
        case ANEURALNETWORKS_UNEXPECTED_NULL:
            return ErrorStatus::INVALID_ARGUMENT;

        case ANEURALNETWORKS_OUTPUT_INSUFFICIENT_SIZE:
            return ErrorStatus::OUTPUT_INSUFFICIENT_SIZE;

        case ANEURALNETWORKS_UNAVAILABLE_DEVICE:
            return ErrorStatus::DEVICE_UNAVAILABLE;

        default:
            LOG(ERROR) << "Unknown result code " << resultCode
                       << " mapped to ErrorStatus::GENERAL_FAILURE";
            return ErrorStatus::GENERAL_FAILURE;
        case ANEURALNETWORKS_BAD_STATE:
        case ANEURALNETWORKS_INCOMPLETE:
        case ANEURALNETWORKS_OP_FAILED:
        case ANEURALNETWORKS_OUT_OF_MEMORY:
        case ANEURALNETWORKS_UNMAPPABLE:
            return ErrorStatus::GENERAL_FAILURE;
    }
}

int convertErrorStatusToResultCode(ErrorStatus status) {
    switch (status) {
        case ErrorStatus::NONE:
            return ANEURALNETWORKS_NO_ERROR;

        case ErrorStatus::INVALID_ARGUMENT:
            return ANEURALNETWORKS_BAD_DATA;

        case ErrorStatus::OUTPUT_INSUFFICIENT_SIZE:
            return ANEURALNETWORKS_OUTPUT_INSUFFICIENT_SIZE;

        case ErrorStatus::DEVICE_UNAVAILABLE:
            return ANEURALNETWORKS_UNAVAILABLE_DEVICE;

        default:
            LOG(ERROR) << "Unknown ErrorStatus " << toString(status)
                       << " mapped to ANEURALNETWORKS_OP_FAILED";
            return ANEURALNETWORKS_OP_FAILED;
        case ErrorStatus::GENERAL_FAILURE:
            return ANEURALNETWORKS_OP_FAILED;
    }
}

// V1_2::Capabilities::operandPerformance utilities.
// The field V1_2::Capabilities::operandPerformance is a vector sorted by the
// field V1_2::Capabilities::OperandPerformance::type.

hidl_vec<Capabilities::OperandPerformance> nonExtensionOperandPerformance(PerformanceInfo perf) {
    using OpPerf = Capabilities::OperandPerformance;

    // Note: range presents enumerators in declaration order, not in numerical order.
    static constexpr ::android::hardware::hidl_enum_range<OperandType> kOperandTypeRange;

    hidl_vec<OpPerf> ret(kOperandTypeRange.end() - kOperandTypeRange.begin());

    std::transform(kOperandTypeRange.begin(), kOperandTypeRange.end(), ret.begin(),
                   [perf](OperandType type) {
                       return Capabilities::OperandPerformance{type, perf};
                   });
    std::sort(ret.begin(), ret.end(),
              [](const OpPerf& a, const OpPerf& b) { return a.type < b.type; });

    return ret;
}

void update(hidl_vec<Capabilities::OperandPerformance>* operandPerformance, OperandType type,
            PerformanceInfo perf) {
    CHECK(operandPerformance != nullptr);
    const auto it = std::lower_bound(operandPerformance->begin(), operandPerformance->end(), type,
                                     [](const Capabilities::OperandPerformance& perf,
                                        OperandType type) { return perf.type < type; });
    CHECK(it != operandPerformance->end())
            << toString(type) << " not in " << toString(*operandPerformance);
    it->info = perf;
}

PerformanceInfo lookup(const hidl_vec<Capabilities::OperandPerformance>& operandPerformance,
                       OperandType type) {
    const auto it = std::lower_bound(operandPerformance.begin(), operandPerformance.end(), type,
                                     [](const Capabilities::OperandPerformance& perf,
                                        OperandType type) { return perf.type < type; });
    if (it == operandPerformance.end()) {
        LOG(WARNING) << "No PerformanceInfo for " << toString(type);
        return {.execTime = FLT_MAX, .powerUsage = FLT_MAX};
    } else {
        return it->info;
    }
}

// Versioning

// In Android P, most data types are treated as having the same performance as TENSOR_QUANT8_ASYMM.
// This array must be in sorted order.
static const OperandType kQuantized8PerformanceConsistentWithP[] = {
        OperandType::INT32, OperandType::UINT32, OperandType::TENSOR_INT32, OperandType::OEM,
        OperandType::TENSOR_OEM_BYTE};

static bool isQuantized8PerformanceConsistentWithP(const V1_2::Capabilities& capabilities) {
    const PerformanceInfo quantized8Performance =
            lookup(capabilities.operandPerformance, OperandType::TENSOR_QUANT8_ASYMM);
    return std::all_of(std::begin(kQuantized8PerformanceConsistentWithP),
                       std::end(kQuantized8PerformanceConsistentWithP),
                       [quantized8Performance, &capabilities](OperandType type) {
                           return quantized8Performance ==
                                  lookup(capabilities.operandPerformance, type);
                       });
}

static hidl_vec<V1_2::Capabilities::OperandPerformance> makeQuantized8PerformanceConsistentWithP(
        PerformanceInfo quantized8Performance) {
    hidl_vec<V1_2::Capabilities::OperandPerformance> ret(
            sizeof(kQuantized8PerformanceConsistentWithP) /
            sizeof(kQuantized8PerformanceConsistentWithP[0]));
    std::transform(
            std::begin(kQuantized8PerformanceConsistentWithP),
            std::end(kQuantized8PerformanceConsistentWithP), ret.begin(),
            [quantized8Performance](OperandType type) -> V1_2::Capabilities::OperandPerformance {
                return {type, quantized8Performance};
            });
    return ret;
}

bool compliantWithV1_0(const V1_0::Capabilities&) {
    return true;
}

bool compliantWithV1_0(const V1_1::Capabilities& capabilities) {
    return capabilities.relaxedFloat32toFloat16Performance == capabilities.float32Performance;
}

bool compliantWithV1_0(const V1_2::Capabilities& capabilities) {
    const PerformanceInfo perfTensorFloat32 =
            lookup(capabilities.operandPerformance, OperandType::TENSOR_FLOAT32);
    const PerformanceInfo perfFloat32 =
            lookup(capabilities.operandPerformance, OperandType::FLOAT32);
    if (perfTensorFloat32 != perfFloat32 ||
        perfTensorFloat32 != capabilities.relaxedFloat32toFloat16PerformanceTensor ||
        perfFloat32 != capabilities.relaxedFloat32toFloat16PerformanceScalar) {
        return false;
    }

    return isQuantized8PerformanceConsistentWithP(capabilities);
}

bool compliantWithV1_1(const V1_0::Capabilities&) {
    return true;
}

bool compliantWithV1_1(const V1_1::Capabilities&) {
    return true;
}

bool compliantWithV1_1(const V1_2::Capabilities& capabilities) {
    if ((capabilities.relaxedFloat32toFloat16PerformanceTensor !=
         capabilities.relaxedFloat32toFloat16PerformanceScalar) ||
        (lookup(capabilities.operandPerformance, OperandType::TENSOR_FLOAT32) !=
         lookup(capabilities.operandPerformance, OperandType::FLOAT32))) {
        return false;
    }

    return isQuantized8PerformanceConsistentWithP(capabilities);
}

bool compliantWithV1_2(const V1_0::Capabilities&) {
    return true;
}

bool compliantWithV1_2(const V1_1::Capabilities&) {
    return true;
}

bool compliantWithV1_2(const V1_0::Model&) {
    return true;
}

bool compliantWithV1_0(const V1_1::Model& model) {
    // In addition to new enumeration values being introduced in V1_1::Model, a
    // new flag was introduced to indicate whether or not float32 data can be
    // calculated using float16 units. This 'relaxComputationFloat32toFloat16'
    // flag is not relevant in whether a V1_1::Model is compliant with a
    // V1_0::Model because all 1.0 drivers require strict calculation by default
    // in the P NN runtime. Even if fp16 calculations are allowed, they can
    // still be computed by a strict fp32 driver.
    return std::all_of(
            model.operations.begin(), model.operations.end(), [&model](const V1_1::Operation& op) {
                int error = validateOperation(static_cast<int32_t>(op.type), op.inputs.size(),
                                              op.inputs.size() > 0 ? op.inputs.data() : nullptr,
                                              op.outputs.size(),
                                              op.outputs.size() > 0 ? op.outputs.data() : nullptr,
                                              convertToV1_2(model.operands), HalVersion::V1_0);
                return error == ANEURALNETWORKS_NO_ERROR;
            });
}

bool compliantWithV1_1(const V1_0::Model&) {
    return true;
}

bool compliantWithV1_1(const V1_1::Model&) {
    return true;
}

static V1_0::OperationType uncheckedConvertToV1_0(V1_1::OperationType type) {
    return static_cast<V1_0::OperationType>(type);
}

static V1_1::OperationType convertToV1_1(V1_0::OperationType type) {
    return static_cast<V1_1::OperationType>(type);
}

V1_0::Capabilities convertToV1_0(const V1_0::Capabilities& capabilities) {
    return capabilities;
}

V1_0::Capabilities convertToV1_0(const V1_1::Capabilities& capabilities) {
    if (!compliantWithV1_0(capabilities)) {
        LOG(ERROR) << "Upcasting non-compliant capabilities " << toString(capabilities)
                   << " from V1_1::Capabilities to V1_0::Capabilities";
    }
    return { .float32Performance = capabilities.float32Performance,
             .quantized8Performance = capabilities.quantized8Performance };
}

V1_0::Capabilities convertToV1_0(const V1_2::Capabilities& capabilities) {
    if (!compliantWithV1_0(capabilities)) {
        LOG(ERROR) << "Upcasting non-compliant capabilities " << toString(capabilities)
                   << " from V1_2::Capabilities to V1_0::Capabilities";
    }
    return {.float32Performance =
                    lookup(capabilities.operandPerformance, OperandType::TENSOR_FLOAT32),
            .quantized8Performance =
                    lookup(capabilities.operandPerformance, OperandType::TENSOR_QUANT8_ASYMM)};
}

V1_1::Capabilities convertToV1_1(const V1_0::Capabilities& capabilities) {
    return { .float32Performance = capabilities.float32Performance,
             .quantized8Performance = capabilities.quantized8Performance,
             .relaxedFloat32toFloat16Performance = capabilities.float32Performance };
}

V1_1::Capabilities convertToV1_1(const V1_1::Capabilities& capabilities) {
    return capabilities;
}

V1_1::Capabilities convertToV1_1(const V1_2::Capabilities& capabilities) {
    if (!compliantWithV1_1(capabilities)) {
        LOG(ERROR) << "Upcasting non-compliant capabilities " << toString(capabilities)
                   << " from V1_2::Capabilities to V1_1::Capabilities";
    }
    return {.float32Performance =
                    lookup(capabilities.operandPerformance, OperandType::TENSOR_FLOAT32),
            .quantized8Performance =
                    lookup(capabilities.operandPerformance, OperandType::TENSOR_QUANT8_ASYMM),
            .relaxedFloat32toFloat16Performance =
                    capabilities.relaxedFloat32toFloat16PerformanceTensor};
}

V1_2::Capabilities convertToV1_2(const V1_0::Capabilities& capabilities) {
    V1_2::Capabilities ret = {
            .relaxedFloat32toFloat16PerformanceScalar = capabilities.float32Performance,
            .relaxedFloat32toFloat16PerformanceTensor = capabilities.float32Performance,
            .operandPerformance =
                    makeQuantized8PerformanceConsistentWithP(capabilities.quantized8Performance)};
    auto& opPerf = ret.operandPerformance;
    opPerf.resize(opPerf.size() + 2);
    opPerf[opPerf.size() - 2] = {OperandType::TENSOR_FLOAT32, capabilities.float32Performance};
    opPerf[opPerf.size() - 1] = {OperandType::FLOAT32, capabilities.float32Performance};
    using OperandPerformance = V1_2::Capabilities::OperandPerformance;
    std::sort(opPerf.begin(), opPerf.end(),
              [](const OperandPerformance& a, const OperandPerformance& b) {
                  return a.type < b.type;
              });
    return ret;
}

V1_2::Capabilities convertToV1_2(const V1_1::Capabilities& capabilities) {
    V1_2::Capabilities ret = {.relaxedFloat32toFloat16PerformanceScalar =
                                      capabilities.relaxedFloat32toFloat16Performance,
                              .relaxedFloat32toFloat16PerformanceTensor =
                                      capabilities.relaxedFloat32toFloat16Performance,
                              .operandPerformance = makeQuantized8PerformanceConsistentWithP(
                                      capabilities.quantized8Performance)};
    auto& opPerf = ret.operandPerformance;
    opPerf.resize(opPerf.size() + 2);
    opPerf[opPerf.size() - 2] = {OperandType::TENSOR_FLOAT32, capabilities.float32Performance};
    opPerf[opPerf.size() - 1] = {OperandType::FLOAT32, capabilities.float32Performance};
    using OperandPerformance = V1_2::Capabilities::OperandPerformance;
    std::sort(opPerf.begin(), opPerf.end(),
              [](const OperandPerformance& a, const OperandPerformance& b) {
                  return a.type < b.type;
              });
    return ret;
}

V1_2::Capabilities convertToV1_2(const V1_2::Capabilities& capabilities) {
    return capabilities;
}

static V1_0::Operation uncheckedConvertToV1_0(const V1_1::Operation& operation) {
    return {.type = uncheckedConvertToV1_0(operation.type),
            .inputs = operation.inputs,
            .outputs = operation.outputs};
}

static V1_1::Operation convertToV1_1(const V1_0::Operation& operation) {
    return {.type = convertToV1_1(operation.type),
            .inputs = operation.inputs,
            .outputs = operation.outputs};
}

static hidl_vec<V1_0::Operation> uncheckedConvertToV1_0(
        const hidl_vec<V1_1::Operation>& operations) {
    hidl_vec<V1_0::Operation> result(operations.size());
    std::transform(
            operations.begin(), operations.end(), result.begin(),
            [](const V1_1::Operation& operation) { return uncheckedConvertToV1_0(operation); });
    return result;
}

static hidl_vec<V1_1::Operation> convertToV1_1(const hidl_vec<V1_0::Operation>& operations) {
    hidl_vec<V1_1::Operation> result(operations.size());
    std::transform(operations.begin(), operations.end(), result.begin(),
                   [](const V1_0::Operation& operation) { return convertToV1_1(operation); });
    return result;
}

bool compliantWithV1_0(const V1_2::Operand& operand) {
    return validOperandType(static_cast<V1_0::OperandType>(operand.type)) &&
           (nonExtensionOperandTypeIsScalar(static_cast<int>(operand.type)) ||
            operand.dimensions.size() != 0);
}

V1_0::Model convertToV1_0(const V1_0::Model& model) {
    return model;
}

V1_0::Model convertToV1_0(const V1_1::Model& model) {
    if (!compliantWithV1_0(model)) {
        LOG(ERROR) << "Upcasting non-compliant model " << SHOW_IF_DEBUG(toString(model))
                   << " from V1_1::Model to V1_0::Model";
    }
    return {.operands = model.operands,
            .operations = uncheckedConvertToV1_0(model.operations),
            .inputIndexes = model.inputIndexes,
            .outputIndexes = model.outputIndexes,
            .operandValues = model.operandValues,
            .pools = model.pools};
}

V1_1::Model convertToV1_1(const V1_0::Model& model) {
    return {.operands = model.operands,
            .operations = convertToV1_1(model.operations),
            .inputIndexes = model.inputIndexes,
            .outputIndexes = model.outputIndexes,
            .operandValues = model.operandValues,
            .pools = model.pools,
            .relaxComputationFloat32toFloat16 = false};
}

V1_1::Model convertToV1_1(const V1_1::Model& model) {
    return model;
}

void logModelToInfo(const V1_2::Model& model) {
    LOG(INFO) << "V1_2::Model start";
    LOG(INFO) << "operands" << toString(model.operands);
    LOG(INFO) << "operations" << toString(model.operations);
    LOG(INFO) << "inputIndexes" << toString(model.inputIndexes);
    LOG(INFO) << "outputIndexes" << toString(model.outputIndexes);
    LOG(INFO) << "operandValues size" << model.operandValues.size();
    LOG(INFO) << "pools" << SHOW_IF_DEBUG(toString(model.pools));
}

static bool compliantWith(HalVersion version, const V1_2::Model& model,
                          std::set<uint32_t>* noncompliantOperations) {
    if (version >= HalVersion::V1_2) return true;

    // A boolean vector indicating whether each pool is compliant with the target HAL version.
    std::vector<bool> isPoolCompliant(model.pools.size(), false);
    std::transform(model.pools.begin(), model.pools.end(), isPoolCompliant.begin(),
                   [version](const hidl_memory& pool) { return validatePool(pool, version); });

    // A boolean vector indicating whether each operand is compliant with the target HAL version.
    std::vector<bool> isOperandCompliant(model.operands.size(), false);
    std::transform(model.operands.begin(), model.operands.end(), isOperandCompliant.begin(),
                   [&isPoolCompliant](const V1_2::Operand& op) {
                       // There is no V1_1::Operand -- both V1_0::Model and V1_1::Model use
                       // V1_0::Operand.
                       return compliantWithV1_0(op) &&
                              !(op.lifetime == OperandLifeTime::CONSTANT_REFERENCE &&
                                !isPoolCompliant[op.location.poolIndex]);
                   });

    auto allOperandsCompliant = [&isOperandCompliant](const hidl_vec<uint32_t>& indices) {
        return std::all_of(
                indices.begin(), indices.end(),
                [&isOperandCompliant](const uint32_t ind) { return isOperandCompliant[ind]; });
    };

    auto localValidateOperation = [&model, version,
                                   &allOperandsCompliant](const V1_2::Operation& op) {
        if (!allOperandsCompliant(op.inputs) || !allOperandsCompliant(op.outputs)) return false;
        int error = validateOperation(
                static_cast<int32_t>(op.type), op.inputs.size(),
                op.inputs.size() > 0 ? op.inputs.data() : nullptr, op.outputs.size(),
                op.outputs.size() > 0 ? op.outputs.data() : nullptr, model.operands, version);
        return error == ANEURALNETWORKS_NO_ERROR;
    };

    if (noncompliantOperations) {
        CHECK(noncompliantOperations->empty());
        for (uint32_t idx = 0; idx < model.operations.size(); ++idx) {
            if (!localValidateOperation(model.operations[idx])) {
                noncompliantOperations->insert(idx);
            }
        }
        return noncompliantOperations->empty();
    } else {
        return std::all_of(model.operations.begin(), model.operations.end(),
                           localValidateOperation);
    }
}

bool compliantWithV1_0(const V1_2::Model& model, std::set<uint32_t>* noncompliantOperations) {
    return compliantWith(HalVersion::V1_0, model, noncompliantOperations);
}

bool compliantWithV1_1(const V1_2::Model& model, std::set<uint32_t>* noncompliantOperations) {
    return compliantWith(HalVersion::V1_1, model, noncompliantOperations);
}

V1_0::OperationType uncheckedConvertToV1_0(V1_2::OperationType type) {
    return static_cast<V1_0::OperationType>(type);
}

V1_1::OperationType uncheckedConvertToV1_1(V1_2::OperationType type) {
    return static_cast<V1_1::OperationType>(type);
}

static V1_2::OperationType convertToV1_2(V1_0::OperationType type) {
    return static_cast<V1_2::OperationType>(type);
}

static V1_2::OperationType convertToV1_2(V1_1::OperationType type) {
    return static_cast<V1_2::OperationType>(type);
}

static V1_0::Operation uncheckedConvertToV1_0(const V1_2::Operation& operation) {
    return {.type = uncheckedConvertToV1_0(operation.type),
            .inputs = operation.inputs,
            .outputs = operation.outputs};
}

static V1_1::Operation uncheckedConvertToV1_1(const V1_2::Operation& operation) {
    return {.type = uncheckedConvertToV1_1(operation.type),
            .inputs = operation.inputs,
            .outputs = operation.outputs};
}

static V1_2::Operation convertToV1_2(const V1_0::Operation& operation) {
    return {.type = convertToV1_2(operation.type),
            .inputs = operation.inputs,
            .outputs = operation.outputs};
}

static V1_2::Operation convertToV1_2(const V1_1::Operation& operation) {
    return {.type = convertToV1_2(operation.type),
            .inputs = operation.inputs,
            .outputs = operation.outputs};
}

static hidl_vec<V1_0::Operation> uncheckedConvertToV1_0(
        const hidl_vec<V1_2::Operation>& operations) {
    hidl_vec<V1_0::Operation> result(operations.size());
    std::transform(
            operations.begin(), operations.end(), result.begin(),
            [](const V1_2::Operation& operation) { return uncheckedConvertToV1_0(operation); });
    return result;
}

static hidl_vec<V1_1::Operation> uncheckedConvertToV1_1(
        const hidl_vec<V1_2::Operation>& operations) {
    hidl_vec<V1_1::Operation> result(operations.size());
    std::transform(
            operations.begin(), operations.end(), result.begin(),
            [](const V1_2::Operation& operation) { return uncheckedConvertToV1_1(operation); });
    return result;
}

static hidl_vec<V1_2::Operation> convertToV1_2(const hidl_vec<V1_0::Operation>& operations) {
    hidl_vec<V1_2::Operation> result(operations.size());
    std::transform(operations.begin(), operations.end(), result.begin(),
                   [](const V1_0::Operation& operation) { return convertToV1_2(operation); });
    return result;
}

static hidl_vec<V1_2::Operation> convertToV1_2(const hidl_vec<V1_1::Operation>& operations) {
    hidl_vec<V1_2::Operation> result(operations.size());
    std::transform(operations.begin(), operations.end(), result.begin(),
                   [](const V1_1::Operation& operation) { return convertToV1_2(operation); });
    return result;
}

// We only need to convert from 1.0 and back since there wasn't any changes to
// Operand in 1.1
V1_2::OperandType convertToV1_2(const V1_0::OperandType& operandType) {
    return static_cast<V1_2::OperandType>(operandType);
}

static bool compliantWithV1_0(const V1_2::OperandType& operandType) {
    return validOperandType(static_cast<V1_0::OperandType>(operandType));
}

V1_0::OperandType convertToV1_0(const V1_2::OperandType& operandType) {
    if (!compliantWithV1_0(operandType)) {
        LOG(ERROR) << "Upcasting non-compliant operand type " << toString(operandType)
                   << " from V1_2::Operand to V1_0::Operand";
    }
    return static_cast<V1_0::OperandType>(operandType);
}

// We only need to convert from 1.0 and back since there wasn't any changes to
// Operand in 1.1
V1_2::Operand convertToV1_2(const V1_0::Operand& operand) {
    return {.type = convertToV1_2(operand.type),
            .dimensions = operand.dimensions,
            .numberOfConsumers = operand.numberOfConsumers,
            .scale = operand.scale,
            .zeroPoint = operand.zeroPoint,
            .lifetime = operand.lifetime,
            .location = operand.location};
}

V1_2::Operand convertToV1_2(const V1_2::Operand& operand) {
    return operand;
}

V1_0::Operand convertToV1_0(const V1_2::Operand& operand) {
    return {.type = convertToV1_0(operand.type),
            .dimensions = operand.dimensions,
            .numberOfConsumers = operand.numberOfConsumers,
            .scale = operand.scale,
            .zeroPoint = operand.zeroPoint,
            .lifetime = operand.lifetime,
            .location = operand.location};
}

// We only need to convert from 1.0 and back since there wasn't any changes to
// Operand in 1.1
hidl_vec<V1_2::Operand> convertToV1_2(const hidl_vec<V1_0::Operand>& operands) {
    hidl_vec<V1_2::Operand> result(operands.size());
    std::transform(operands.begin(), operands.end(), result.begin(),
                   [](const V1_0::Operand& operand) { return convertToV1_2(operand); });
    return result;
}

hidl_vec<V1_2::Operand> convertToV1_2(const hidl_vec<V1_2::Operand>& operands) {
    return operands;
}

hidl_vec<V1_0::Operand> convertToV1_0(const hidl_vec<V1_2::Operand>& operands) {
    hidl_vec<V1_0::Operand> result(operands.size());
    std::transform(operands.begin(), operands.end(), result.begin(),
                   [](const V1_2::Operand& operand) { return convertToV1_0(operand); });
    return result;
}

V1_0::Model convertToV1_0(const V1_2::Model& model) {
    if (!compliantWithV1_0(model)) {
        LOG(ERROR) << "Upcasting non-compliant model " << SHOW_IF_DEBUG(toString(model))
                   << " from V1_2::Model to V1_0::Model";
    }
    return {.operands = convertToV1_0(model.operands),
            .operations = uncheckedConvertToV1_0(model.operations),
            .inputIndexes = model.inputIndexes,
            .outputIndexes = model.outputIndexes,
            .operandValues = model.operandValues,
            .pools = model.pools};
}

V1_1::Model convertToV1_1(const V1_2::Model& model) {
    if (!compliantWithV1_1(model)) {
        LOG(ERROR) << "Upcasting non-compliant model " << SHOW_IF_DEBUG(toString(model))
                   << " from V1_2::Model to V1_1::Model";
    }
    return {.operands = convertToV1_0(model.operands),  // Operands in 1.1 and 1.0 are identical.
            .operations = uncheckedConvertToV1_1(model.operations),
            .inputIndexes = model.inputIndexes,
            .outputIndexes = model.outputIndexes,
            .operandValues = model.operandValues,
            .pools = model.pools,
            .relaxComputationFloat32toFloat16 = model.relaxComputationFloat32toFloat16};
}

V1_2::Model convertToV1_2(const V1_0::Model& model) {
    return {.operands = convertToV1_2(model.operands),
            .operations = convertToV1_2(model.operations),
            .inputIndexes = model.inputIndexes,
            .outputIndexes = model.outputIndexes,
            .operandValues = model.operandValues,
            .pools = model.pools,
            .relaxComputationFloat32toFloat16 = false};
}

V1_2::Model convertToV1_2(const V1_1::Model& model) {
    return {.operands = convertToV1_2(model.operands),
            .operations = convertToV1_2(model.operations),
            .inputIndexes = model.inputIndexes,
            .outputIndexes = model.outputIndexes,
            .operandValues = model.operandValues,
            .pools = model.pools,
            .relaxComputationFloat32toFloat16 = model.relaxComputationFloat32toFloat16};
}

V1_2::Model convertToV1_2(const V1_2::Model& model) {
    return model;
}

#ifdef NN_DEBUGGABLE
uint32_t getProp(const char* str, uint32_t defaultValue) {
    const std::string propStr = android::base::GetProperty(str, "");
    if (propStr.size() > 0) {
        return std::stoi(propStr);
    } else {
        return defaultValue;
    }
}
#endif  // NN_DEBUGGABLE

} // namespace nn
} // namespace android
