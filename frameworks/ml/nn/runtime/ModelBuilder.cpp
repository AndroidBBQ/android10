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

#define LOG_TAG "ModelBuilder"

#include "ModelBuilder.h"

#include "CompilationBuilder.h"
#include "GraphDump.h"
#include "Manager.h"
#include "TypeManager.h"
#include "Utils.h"
#include "ValidateHal.h"

#include <map>
#include <utility>

namespace android {
namespace nn {

// The maximum number of operands and operations that a model may have.
const uint32_t MAX_NUMBER_OF_OPERANDS = 0xFFFFFFFE;
const uint32_t MAX_NUMBER_OF_OPERATIONS = 0xFFFFFFFE;

bool ModelBuilder::badState(const char* name) {
    if (mCompletedModel) {
        LOG(ERROR) << "ANeuralNetworksModel_" << name << " can't modify after model finished";
        return true;
    }
    if (mInvalidModel) {
        LOG(ERROR) << "ANeuralNetworksModel_" << name << " can't modify an invalid model";
        return true;
    }
    return false;
}

int ModelBuilder::getExtensionType(const char* extensionName, uint16_t typeWithinExtension,
                                   int32_t* type) {
    return TypeManager::get()->getExtensionType(extensionName, typeWithinExtension, type)
                   ? ANEURALNETWORKS_NO_ERROR
                   : ANEURALNETWORKS_BAD_DATA;
}

int ModelBuilder::addOperand(const ANeuralNetworksOperandType& type) {
    if (badState("addOperand")) {
        return ANEURALNETWORKS_BAD_STATE;
    }

    OperandType operandType = static_cast<OperandType>(type.type);
    if (isExtensionOperandType(operandType) && !TypeManager::get()->areExtensionsAllowed()) {
        LOG(ERROR) << "Extensions are not supported for this process.";
        return ANEURALNETWORKS_BAD_DATA;
    }
    if (operandType == OperandType::OEM || operandType == OperandType::TENSOR_OEM_BYTE) {
        LOG(WARNING) << "OEM data type is deprecated. Use Extensions instead.";
    }

    const Extension::OperandTypeInformation* info = nullptr;
    if (isExtensionOperandType(operandType) &&
        !TypeManager::get()->getExtensionOperandTypeInfo(operandType, &info)) {
        LOG(ERROR) << "Extension operand type " << toString(operandType) << " is not registered";
        return ANEURALNETWORKS_BAD_DATA;
    }
    NN_RETURN_IF_ERROR(validateOperandType(type, info, "ANeuralNetworksModel_addOperand", true));
    size_t idx = mOperands.size();
    if (idx >= MAX_NUMBER_OF_OPERANDS) {
        LOG(ERROR) << "ANeuralNetworksModel_addOperand exceed max operands";
        return ANEURALNETWORKS_BAD_DATA;
    }

    mOperands.push_back({
            .type = operandType,
            .dimensions =
                    hidl_vec<uint32_t>(type.dimensions, type.dimensions + type.dimensionCount),
            .numberOfConsumers = 0,
            .scale = type.scale,
            .zeroPoint = type.zeroPoint,
            .lifetime = OperandLifeTime::TEMPORARY_VARIABLE,
            .location = {.poolIndex = 0, .offset = 0, .length = 0},
            .extraParams = Operand::ExtraParams(),
    });
    return ANEURALNETWORKS_NO_ERROR;
}

int ModelBuilder::setOperandValue(uint32_t index, const void* buffer, size_t length) {
    VLOG(MODEL) << __func__ << " for operand " << index << " size " << length;
    if (badState("setOperandValue")) {
        return ANEURALNETWORKS_BAD_STATE;
    }

    if (index >= operandCount()) {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandValue setting operand " << index << " of "
                   << operandCount();
        return ANEURALNETWORKS_BAD_DATA;
    }
    Operand& operand = mOperands[index];
    if (buffer == nullptr) {
        if (length) {
            LOG(ERROR) << "ANeuralNetworksModel_setOperandValue buffer is nullptr but length is "
                          "not 0";
            return ANEURALNETWORKS_BAD_DATA;
        }
        operand.lifetime = OperandLifeTime::NO_VALUE;
        // The location is unused and is set to zeros.
        operand.location = {.poolIndex = 0, .offset = 0, .length = 0};
    } else {
        if (TypeManager::get()->isTensorType(operand.type) &&
            tensorHasUnspecifiedDimensions(operand)) {
            LOG(ERROR) << "ANeuralNetworksModel_setOperandValue setting operand " << index
                       << " which has operand type that is not fully specified";
            return ANEURALNETWORKS_BAD_DATA;
        }
        if (length > 0xFFFFFFFF) {
            LOG(ERROR) << "ANeuralNetworksModel_setOperandValue value length of " << length
                       << " exceeds max size";
            return ANEURALNETWORKS_BAD_DATA;
        }
        uint32_t valueLength = static_cast<uint32_t>(length);
        if (operand.type != OperandType::OEM) {
            uint32_t neededLength = TypeManager::get()->getSizeOfData(operand);
            if (neededLength != valueLength) {
                LOG(ERROR) << "ANeuralNetworksModel_setOperandValue setting " << valueLength
                           << " bytes when needing " << neededLength;
                return ANEURALNETWORKS_BAD_DATA;
            }
        }
        if (valueLength <= ANEURALNETWORKS_MAX_SIZE_OF_IMMEDIATELY_COPIED_VALUES) {
            uint32_t existingSize = static_cast<uint32_t>(mSmallOperandValues.size());
            uint32_t extraBytes = alignBytesNeeded(existingSize, valueLength);
            mSmallOperandValues.resize(existingSize + extraBytes + valueLength);
            operand.lifetime = OperandLifeTime::CONSTANT_COPY;
            operand.location = {
                    .poolIndex = 0, .offset = existingSize + extraBytes, .length = valueLength};
            memcpy(&mSmallOperandValues[operand.location.offset], buffer, valueLength);
            VLOG(MODEL) << "Copied small value to offset " << operand.location.offset;
        } else {
            VLOG(MODEL) << "Saving large value";
            operand.lifetime = OperandLifeTime::CONSTANT_REFERENCE;
            // The values for poolIndex and offset will be set when the model is finished.
            typedef decltype(operand.location.poolIndex) PoolIndexType;
            typedef decltype(operand.location.offset) OffsetType;
            operand.location = {.poolIndex = ~PoolIndexType(0),
                                .offset = ~OffsetType(0),
                                .length = valueLength};
            // We keep track of the buffers. We'll allocate the shared memory only
            // once we know the total size, to avoid needless copies.
            mLargeOperandValues.push_back(LargeValue{.operandIndex = index, .buffer = buffer});
        }
    }
    return ANEURALNETWORKS_NO_ERROR;
}

int ModelBuilder::setOperandSymmPerChannelQuantParams(
        uint32_t index, const ANeuralNetworksSymmPerChannelQuantParams& channelQuant) {
    if (badState("setOperandSymmPerChannelQuantParams")) {
        return ANEURALNETWORKS_BAD_STATE;
    }

    if (index >= operandCount()) {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandSymmPerChannelQuantParams "
                   << "setting per-channel quantization parameters for operand " << index << " of "
                   << operandCount();
        return ANEURALNETWORKS_BAD_DATA;
    }
    Operand& operand = mOperands[index];

    if (!validateOperandSymmPerChannelQuantParams(
                operand, channelQuant,
                "ANeuralNetworksModel_setOperandSymmPerChannelQuantParams")) {
        return ANEURALNETWORKS_BAD_DATA;
    }
    switch (operand.type) {
        case OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL:
            operand.extraParams.channelQuant({
                    .scales = hidl_vec<float>(channelQuant.scales,
                                              channelQuant.scales + channelQuant.scaleCount),
                    .channelDim = channelQuant.channelDim,
            });
            break;
        default:
            LOG(ERROR) << "ANeuralNetworksModel_setOperandSymmPerChannelQuantParams "
                       << "invalid operand type " << static_cast<int32_t>(operand.type);
            return ANEURALNETWORKS_BAD_DATA;
    }
    return ANEURALNETWORKS_NO_ERROR;
}

int ModelBuilder::setOperandExtensionData(uint32_t index, const void* data, size_t length) {
    if (badState("setOperandExtensionData")) {
        return ANEURALNETWORKS_BAD_STATE;
    }

    if (index >= operandCount()) {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandExtensionData "
                   << "setting extension data for operand " << index << " of " << operandCount();
        return ANEURALNETWORKS_BAD_DATA;
    }
    Operand& operand = mOperands[index];

    if (data == nullptr && length != 0) {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandExtensionData data is nullptr but length is "
                   << length;
        return ANEURALNETWORKS_BAD_DATA;
    }
    if (data != nullptr && length == 0) {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandExtensionData data is not nullptr but length "
                   << "is zero";
        return ANEURALNETWORKS_BAD_DATA;
    }
    if (!isExtensionOperandType(operand.type)) {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandExtensionData "
                   << "setting extension data for a base operand type "
                   << static_cast<int32_t>(operand.type);
        return ANEURALNETWORKS_BAD_DATA;
    }

    if (data == nullptr) {
        operand.extraParams.none();
    } else {
        operand.extraParams.extension(
                hidl_vec<uint8_t>(reinterpret_cast<const uint8_t*>(data),
                                  reinterpret_cast<const uint8_t*>(data) + length));
    }
    return ANEURALNETWORKS_NO_ERROR;
}

int ModelBuilder::copyLargeValuesToSharedMemory() {
    VLOG(MODEL) << __func__ << " has " << mLargeOperandValues.size() << " values.";
    if (!mLargeOperandValues.empty()) {
        // Calculate the size of the shared memory needed for all the large values.
        // Also sets the offset for each value within the memory.
        size_t poolSize = 0;
        for (LargeValue& l : mLargeOperandValues) {
            Operand& operand = mOperands[l.operandIndex];
            nnAssert(operand.lifetime == OperandLifeTime::CONSTANT_REFERENCE);
            poolSize += alignBytesNeeded(poolSize, operand.location.length);
            operand.location.offset = poolSize;
            poolSize += operand.location.length;
        }

        // Allocated the shared memory.
        int n = mLargeValueMemory.create(poolSize);
        if (n != ANEURALNETWORKS_NO_ERROR) {
            return n;
        }
        uint8_t* memoryPointer = nullptr;
        n = mLargeValueMemory.getPointer(&memoryPointer);
        if (n != ANEURALNETWORKS_NO_ERROR) {
            return n;
        }
        uint32_t poolIndex = mMemories.add(&mLargeValueMemory);
        VLOG(MODEL) << "Allocated large value pool of size " << poolSize << " at index "
                    << poolIndex;

        // Copy the values to this memory.
        for (LargeValue& l : mLargeOperandValues) {
            Operand& operand = mOperands[l.operandIndex];
            operand.location.poolIndex = poolIndex;
            memcpy(memoryPointer + operand.location.offset, l.buffer, operand.location.length);
        }
    }
    return ANEURALNETWORKS_NO_ERROR;
}

int ModelBuilder::setOperandValueFromMemory(uint32_t index, const Memory* memory, uint32_t offset,
                                            size_t length) {
    VLOG(MODEL) << __func__ << " for operand " << index << " offset " << offset << " size "
                << length;
    if (badState("setOperandValueFromMemory")) {
        return ANEURALNETWORKS_BAD_STATE;
    }

    if (index >= operandCount()) {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandValueFromMemory setting operand " << index
                   << " of " << operandCount();
        return ANEURALNETWORKS_BAD_DATA;
    }
    Operand& operand = mOperands[index];
    if (TypeManager::get()->isTensorType(operand.type) && tensorHasUnspecifiedDimensions(operand)) {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandValueFromMemory setting operand " << index
                   << " which has operand type that is not fully specified";
        return ANEURALNETWORKS_BAD_DATA;
    }
    // Only BLOB format AHardwareBuffer can be used for constant data.
    if (memory->getHidlMemory().name() == "hardware_buffer") {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandValueFromMemory passed an AHardwareBuffer"
                   << " that is not in AHARDWAREBUFFER_FORMAT_BLOB format";
        return ANEURALNETWORKS_UNMAPPABLE;
    }
    uint32_t neededLength = TypeManager::get()->getSizeOfData(operand);
    if (neededLength != length) {
        LOG(ERROR) << "ANeuralNetworksModel_setOperandValueFromMemory setting " << length
                   << " bytes when needing " << neededLength;
        return ANEURALNETWORKS_BAD_DATA;
    }
    if (!memory->validateSize(offset, length)) {
        return ANEURALNETWORKS_BAD_DATA;
    }
    operand.lifetime = OperandLifeTime::CONSTANT_REFERENCE;
    operand.location = {.poolIndex = mMemories.add(memory),
                        .offset = offset,
                        .length = static_cast<uint32_t>(length)};
    return ANEURALNETWORKS_NO_ERROR;
}

int ModelBuilder::addOperation(ANeuralNetworksOperationType type, uint32_t inputCount,
                               const uint32_t* inputs, uint32_t outputCount,
                               const uint32_t* outputs) {
    if (badState("addOperation")) {
        return ANEURALNETWORKS_BAD_STATE;
    }

    OperationType operationType = static_cast<OperationType>(type);
    if (isExtensionOperationType(operationType) && !TypeManager::get()->areExtensionsAllowed()) {
        LOG(ERROR) << "Extensions are not supported for this process.";
        return ANEURALNETWORKS_BAD_DATA;
    }
    if (operationType == OperationType::OEM_OPERATION) {
        LOG(WARNING) << "OEM_OPERATION is deprecated. Use Extensions instead.";
    }

    if (!isExtensionOperationType(operationType)) {
        if (!validCode(kNumberOfOperationTypes, kNumberOfOperationTypesOEM, type)) {
            LOG(ERROR) << "ANeuralNetworksModel_addOperation invalid operation type " << type;
            return ANEURALNETWORKS_BAD_DATA;
        }
    }
    NN_RETURN_IF_ERROR(validateOperation(type, inputCount, inputs, outputCount, outputs, mOperands,
                                         HalVersion::LATEST));

    uint32_t operationIndex = operationCount();
    if (operationIndex >= MAX_NUMBER_OF_OPERATIONS) {
        LOG(ERROR) << "ANeuralNetworksModel_addOperation exceed max operations";
        return ANEURALNETWORKS_BAD_DATA;
    }

    mOperations.push_back({
            .type = operationType,
            .inputs = hidl_vec<uint32_t>(inputs, inputs + inputCount),
            .outputs = hidl_vec<uint32_t>(outputs, outputs + outputCount),
    });
    for (uint32_t i : mOperations.back().inputs) {
        mOperands[i].numberOfConsumers++;
    }
    mHasOEMOperation |= (operationType == OperationType::OEM_OPERATION);
    mHasExtensionOperation |= isExtensionOperationType(operationType);

    return ANEURALNETWORKS_NO_ERROR;
}

int ModelBuilder::identifyInputsAndOutputs(uint32_t inputCount, const uint32_t* inputs,
                                           uint32_t outputCount, const uint32_t* outputs) {
    if (badState("identifyInputsAndOutputs")) {
        return ANEURALNETWORKS_BAD_STATE;
    }

    int n = validateOperandList(inputCount, inputs, operandCount(),
                                "ANeuralNetworksModel_identifyInputsAndOutputs inputs");
    if (n != ANEURALNETWORKS_NO_ERROR) {
        return n;
    }
    n = validateOperandList(outputCount, outputs, operandCount(),
                            "ANeuralNetworksModel_identifyInputsAndOutputs outputs");
    if (n != ANEURALNETWORKS_NO_ERROR) {
        return n;
    }

    // Makes a copy of the index list, validates the arguments, and changes
    // the lifetime info of the corresponding operand.
    auto setArguments = [&](std::vector<uint32_t>* indexVector, uint32_t indexCount,
                            const uint32_t* indexList, OperandLifeTime lifetime) -> bool {
        indexVector->resize(indexCount);
        for (uint32_t i = 0; i < indexCount; i++) {
            const uint32_t operandIndex = indexList[i];
            if (operandIndex >= mOperands.size()) {
                LOG(ERROR) << "ANeuralNetworksModel_identifyInputsAndOutputs Can't set input or "
                              "output "
                              "to be "
                           << operandIndex << " as this exceeds the numbe of operands "
                           << mOperands.size();
                return false;
            }
            (*indexVector)[i] = operandIndex;
            Operand& operand = mOperands[operandIndex];
            if (operand.lifetime != OperandLifeTime::TEMPORARY_VARIABLE) {
                LOG(ERROR) << "ANeuralNetworksModel_identifyInputsAndOutputs Can't set operand "
                           << operandIndex
                           << " to be an input or output.  Check that it's not a constant or "
                              "already an input or output";
                return false;
            }
            operand.lifetime = lifetime;
        }
        return true;
    };

    if (!setArguments(&mInputIndexes, inputCount, inputs, OperandLifeTime::MODEL_INPUT) ||
        !setArguments(&mOutputIndexes, outputCount, outputs, OperandLifeTime::MODEL_OUTPUT)) {
        return ANEURALNETWORKS_BAD_DATA;
    }

    return ANEURALNETWORKS_NO_ERROR;
}

int ModelBuilder::relaxComputationFloat32toFloat16(bool allow) {
    if (badState("relaxComputationFloat32toFloat16")) {
        return ANEURALNETWORKS_BAD_STATE;
    }

    mRelaxComputationFloat32toFloat16 = allow;

    return ANEURALNETWORKS_NO_ERROR;
}

int ModelBuilder::createCompilation(CompilationBuilder** compilation,
                                    const std::vector<std::shared_ptr<Device>>& devices,
                                    bool explicitDeviceList) {
    if (!mCompletedModel || mInvalidModel) {
        LOG(ERROR) << "ANeuralNetworksCompilation_create passed an unfinished or invalid model";
        *compilation = nullptr;
        return ANEURALNETWORKS_BAD_STATE;
    }
    *compilation = new (std::nothrow) CompilationBuilder(this, devices, explicitDeviceList);
    return (*compilation ? ANEURALNETWORKS_NO_ERROR : ANEURALNETWORKS_OUT_OF_MEMORY);
}

int ModelBuilder::finish() {
    if (mCompletedModel) {
        LOG(ERROR) << "ANeuralNetworksModel_finish called more than once";
        return ANEURALNETWORKS_BAD_STATE;
    }
    if (mInvalidModel) {
        LOG(ERROR) << "ANeuralNetworksModel_finish called on an invalid model";
        return ANEURALNETWORKS_BAD_STATE;
    }

    int n = copyLargeValuesToSharedMemory();
    if (n != ANEURALNETWORKS_NO_ERROR) {
        return n;
    }

    // TODO: Modify validation so that it can be called without creating a HAL Model.
    // NOTE: Must copyLargeValuesToSharedMemory() before validation; otherwise,
    //       a CONSTANT_REFERENCE operand will not have correct .poolIndex, and
    //       validation will not work properly.
    Model modelForValidation;
    setHidlModel(&modelForValidation);
    if (!validateModel(modelForValidation)) {
        LOG(ERROR) << "ANeuralNetworksModel_finish called on invalid model";
        mInvalidModel = true;
        return ANEURALNETWORKS_BAD_DATA;
    }
    if (VLOG_IS_ON(MODEL)) {
        graphDump("ModelBuilder::finish", modelForValidation, nullptr);
    }

    // We sort the operations so that they will be in the appropriate
    // order for a single-threaded, op at a time execution.
    // TODO: we don't need this if we always run the partitioner.
    sortIntoRunOrder();
    mCompletedModel = true;
    return ANEURALNETWORKS_NO_ERROR;
}

void ModelBuilder::sortIntoRunOrder() {
    if (!mSortedOperationIndexMap.empty()) {
        LOG(ERROR) << "Operations already in run order.";
        return;
    }
    // Tracks the operations that can be executed.
    std::vector<uint32_t> opsReadyToRun;
    std::vector<Operation> runOrder;

    // Tracks how many inputs are needed for each operation to be ready to run.
    std::multimap<uint32_t, uint32_t> operandToOperations;
    std::vector<uint32_t> unknownInputCount(operationCount());
    for (uint32_t operationIndex = 0; operationIndex < operationCount(); operationIndex++) {
        uint32_t& count = unknownInputCount[operationIndex];
        count = 0;
        for (uint32_t operandIndex : mOperations[operationIndex].inputs) {
            auto lifetime = mOperands[operandIndex].lifetime;
            if (lifetime == OperandLifeTime::TEMPORARY_VARIABLE ||
                lifetime == OperandLifeTime::MODEL_OUTPUT) {
                count++;
                operandToOperations.insert(
                        std::pair<uint32_t, uint32_t>(operandIndex, operationIndex));
            }
        }
        if (count == 0) {
            opsReadyToRun.push_back(operationIndex);
        }
    }

    while (opsReadyToRun.size() > 0) {
        // Execute the next op
        int opIndex = opsReadyToRun.back();
        opsReadyToRun.pop_back();
        const Operation& operation = mOperations[opIndex];

        runOrder.push_back(mOperations[opIndex]);
        mSortedOperationIndexMap.push_back(opIndex);

        // Mark all its outputs as known.
        for (uint32_t operandIndex : operation.outputs) {
            auto range = operandToOperations.equal_range(operandIndex);
            for (auto i = range.first; i != range.second; i++) {
                uint32_t& count = unknownInputCount[i->second];
                if (--count == 0) {
                    opsReadyToRun.push_back(i->second);
                }
            }
        }
    }
    mOperations = runOrder;
}

void ModelBuilder::setHidlModel(Model* model) const {
    model->operands = mOperands;
    model->operations = mOperations;
    model->inputIndexes = mInputIndexes;
    model->outputIndexes = mOutputIndexes;
    model->operandValues = mSmallOperandValues;
    model->relaxComputationFloat32toFloat16 = mRelaxComputationFloat32toFloat16;
    model->extensionNameToPrefix = getExtensionNameToPrefixMap();

    uint32_t count = mMemories.size();
    model->pools.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        model->pools[i] = mMemories[i]->getHidlMemory();
    }
}

std::vector<Model::ExtensionNameAndPrefix> ModelBuilder::getExtensionNameToPrefixMap() const {
    std::vector<Model::ExtensionNameAndPrefix> extensionNameToPrefix;
    std::set<uint16_t> prefixSet;

    auto addExtensionWithPrefix = [&extensionNameToPrefix, &prefixSet](uint16_t prefix) {
        if (!prefixSet.insert(prefix).second) {
            return;
        }
        const Extension* extension;
        CHECK(TypeManager::get()->getExtensionInfo(prefix, &extension));
        extensionNameToPrefix.push_back({
                .name = extension->name,
                .prefix = prefix,
        });
    };

    constexpr uint8_t kLowBitsType =
            static_cast<uint8_t>(Model::ExtensionTypeEncoding::LOW_BITS_TYPE);
    for (const auto& operand : mOperands) {
        if (isExtensionOperandType(operand.type)) {
            addExtensionWithPrefix(static_cast<uint32_t>(operand.type) >> kLowBitsType);
        }
    }
    for (const auto& operation : mOperations) {
        if (isExtensionOperationType(operation.type)) {
            addExtensionWithPrefix(static_cast<uint32_t>(operation.type) >> kLowBitsType);
        }
    }
    return extensionNameToPrefix;
}

}  // namespace nn
}  // namespace android
