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

#define LOG_TAG "CpuExecutor"

#include "CpuExecutor.h"

#include "NeuralNetworks.h"
#include "OperationResolver.h"
#include "Operations.h"
#include "OperationsUtils.h"
#include "Tracing.h"

#include "Eigen/Core"
// b/109953668, disable OpenMP
#ifdef NNAPI_OPENMP
#include <omp.h>
#endif  // NNAPI_OPENMP
#include <android/hardware_buffer.h>
#include <sys/mman.h>

namespace android {
namespace nn {

namespace {

class OperationExecutionContext : public IOperationExecutionContext {
    DISALLOW_IMPLICIT_CONSTRUCTORS(OperationExecutionContext);

   public:
    OperationExecutionContext(const Operation* operation, RunTimeOperandInfo* operands)
        : operation(operation), operands(operands) {}

    uint32_t getNumInputs() const override;
    OperandType getInputType(uint32_t index) const override;
    Shape getInputShape(uint32_t index) const override;
    const void* getInputBuffer(uint32_t index) const override;
    const Operand::ExtraParams getInputExtraParams(uint32_t index) const override;

    uint32_t getNumOutputs() const override;
    OperandType getOutputType(uint32_t index) const override;
    Shape getOutputShape(uint32_t index) const override;
    void* getOutputBuffer(uint32_t index) override;

    // Return false on failure and store the result code.
    // Use getResultCode() to retrieve it at the end of the operation execution.
    bool setOutputShape(uint32_t index, const Shape& shape) override;
    int getResultCode() const;

    bool isOmittedInput(uint32_t index) const override;
    bool isOmittedOutput(uint32_t index) const override;

    // Return false if any of inputs or outputs is omitted, i.e. has lifetime of NO_VALUE.
    bool checkNoOmittedOperand() const;
    // Return false if any of inputs has dimension 0.
    bool checkNoZeroSizedInput() const;

   private:
    const RunTimeOperandInfo* getInputInfo(uint32_t index) const;
    const RunTimeOperandInfo* getOutputInfo(uint32_t index) const;
    RunTimeOperandInfo* getOutputInfo(uint32_t index);

    const Operation* operation;
    RunTimeOperandInfo* operands;

    int result = ANEURALNETWORKS_NO_ERROR;
};

const RunTimeOperandInfo* OperationExecutionContext::getInputInfo(uint32_t index) const {
    CHECK(index < operation->inputs.size());
    return &operands[operation->inputs[index]];
}

const RunTimeOperandInfo* OperationExecutionContext::getOutputInfo(uint32_t index) const {
    CHECK(index < operation->outputs.size());
    return &operands[operation->outputs[index]];
}

RunTimeOperandInfo* OperationExecutionContext::getOutputInfo(uint32_t index) {
    CHECK(index < operation->outputs.size());
    return &operands[operation->outputs[index]];
}

OperandType OperationExecutionContext::getInputType(uint32_t index) const {
    return getInputInfo(index)->type;
}

Shape OperationExecutionContext::getInputShape(uint32_t index) const {
    return getInputInfo(index)->shape();
}

const void* OperationExecutionContext::getInputBuffer(uint32_t index) const {
    return getInputInfo(index)->buffer;
}

const Operand::ExtraParams OperationExecutionContext::getInputExtraParams(uint32_t index) const {
    return getInputInfo(index)->extraParams;
}

OperandType OperationExecutionContext::getOutputType(uint32_t index) const {
    return getOutputInfo(index)->type;
}

Shape OperationExecutionContext::getOutputShape(uint32_t index) const {
    return getOutputInfo(index)->shape();
}

void* OperationExecutionContext::getOutputBuffer(uint32_t index) {
    return getOutputInfo(index)->buffer;
}

uint32_t OperationExecutionContext::getNumInputs() const {
    return operation->inputs.size();
}

uint32_t OperationExecutionContext::getNumOutputs() const {
    return operation->outputs.size();
}

int OperationExecutionContext::getResultCode() const {
    return result;
}

// TODO: Return error code directly once we've fully integrated OperationResolver with all ops.
// Updates the RunTimeOperandInfo with the newly calculated shape.
// Allocate the buffer if we need to.
bool setInfoAndAllocateIfNeeded(RunTimeOperandInfo* info, const Shape& shape, int* result) {
    // For user-provided model output operands, the parameters must match the Shape
    // calculated from the preparation step.
    if (info->lifetime == OperandLifeTime::MODEL_OUTPUT) {
        if (info->type != shape.type) {
            LOG(ERROR) << "Invalid type for model output";
            *result = ANEURALNETWORKS_OP_FAILED;
            return false;
        }
        if (info->type == OperandType::TENSOR_QUANT8_ASYMM) {
            if (info->scale != shape.scale) {
                LOG(ERROR) << "Invalid scale for model output";
                *result = ANEURALNETWORKS_OP_FAILED;
                return false;
            }
            if (info->zeroPoint != shape.offset) {
                LOG(ERROR) << "Invalid zeroPoint for model output";
                *result = ANEURALNETWORKS_OP_FAILED;
                return false;
            }
        }
        if (info->extraParams != shape.extraParams) {
            LOG(ERROR) << "Invalid extraParams for model output";
            *result = ANEURALNETWORKS_OP_FAILED;
            return false;
        }
    }

    std::vector<uint32_t> combined;
    if (!combineDimensions(shape.dimensions, info->dimensions, &combined)) {
        LOG(ERROR) << "Invalid dimensions for model operand";
        *result = ANEURALNETWORKS_OP_FAILED;
        return false;
    }
    info->dimensions = combined;
    info->type = shape.type;
    info->scale = shape.scale;
    info->zeroPoint = shape.offset;
    info->extraParams = shape.extraParams;

    // Allocate the buffer only if the combined dimension is fully specified
    if (info->lifetime == OperandLifeTime::TEMPORARY_VARIABLE && info->buffer == nullptr) {
        if (isExtensionOperandType(info->type)) {
            LOG(ERROR) << "Cannot allocate a temporary variable of an extension type";
            *result = ANEURALNETWORKS_OP_FAILED;
            return false;
        }
        uint32_t length = nonExtensionOperandSizeOfData(info->type, info->dimensions);
        if (length > 0) {
            info->buffer = new uint8_t[length];
            if (info->buffer == nullptr) {
                *result = ANEURALNETWORKS_OUT_OF_MEMORY;
                return false;
            }
            info->length = length;
        }
    }
    if (!info->isSufficient()) {
        uint32_t length = nonExtensionOperandSizeOfData(info->type, info->dimensions);
        LOG(ERROR) << "Insufficient size for model operand: require = " << length
                   << ", provided = " << info->length;
        *result = ANEURALNETWORKS_OUTPUT_INSUFFICIENT_SIZE;
        return false;
    }
    *result = ANEURALNETWORKS_NO_ERROR;
    return true;
}

bool OperationExecutionContext::setOutputShape(uint32_t index, const Shape& shape) {
    return setInfoAndAllocateIfNeeded(getOutputInfo(index), shape, &result);
}

bool OperationExecutionContext::isOmittedInput(uint32_t index) const {
    return getInputInfo(index)->lifetime == OperandLifeTime::NO_VALUE;
}

bool OperationExecutionContext::isOmittedOutput(uint32_t index) const {
    return getOutputInfo(index)->lifetime == OperandLifeTime::NO_VALUE;
}

bool OperationExecutionContext::checkNoOmittedOperand() const {
    for (uint32_t i = 0; i < operation->inputs.size(); i++) {
        NN_RET_CHECK(!isOmittedInput(i)) << getOperationName(operation->type) << " input operand "
                                         << i << " is required but missing.";
    }
    for (uint32_t i = 0; i < operation->outputs.size(); i++) {
        NN_RET_CHECK(!isOmittedOutput(i)) << getOperationName(operation->type) << " output operand "
                                          << i << " is required but missing.";
    }
    return true;
}

bool OperationExecutionContext::checkNoZeroSizedInput() const {
    for (uint32_t i = 0; i < operation->inputs.size(); i++) {
        if (isOmittedInput(i)) continue;
        for (uint32_t j = 0; j < getInputInfo(i)->dimensions.size(); j++) {
            NN_RET_CHECK_NE(getInputInfo(i)->dimensions[j], 0)
                    << getOperationName(operation->type)
                    << " does not support zero-sized tensor, but input " << i << " dimension " << j
                    << " is 0.";
        }
    }
    return true;
}

}  // namespace

// Used to keep a pointer to a memory pool.
//
// In the case of an "mmap_fd" pool, owns the mmap region
// returned by getBuffer() -- i.e., that region goes away
// when the RunTimePoolInfo is destroyed or is assigned to.
class RunTimePoolInfo::RunTimePoolInfoImpl {
   public:
    RunTimePoolInfoImpl(const hidl_memory& hidlMemory, uint8_t* buffer, const sp<IMemory>& memory,
                        const sp<GraphicBuffer>& graphicBuffer);

    // rule of five...
    ~RunTimePoolInfoImpl();
    RunTimePoolInfoImpl(const RunTimePoolInfoImpl&) = delete;
    RunTimePoolInfoImpl(RunTimePoolInfoImpl&&) noexcept = delete;
    RunTimePoolInfoImpl& operator=(const RunTimePoolInfoImpl&) = delete;
    RunTimePoolInfoImpl& operator=(RunTimePoolInfoImpl&&) noexcept = delete;

    uint8_t* getBuffer() const { return mBuffer; }

    bool update() const;

    hidl_memory getHidlMemory() const { return mHidlMemory; }

   private:
    const hidl_memory mHidlMemory;     // always used
    uint8_t* const mBuffer = nullptr;  // always used
    const sp<IMemory> mMemory;         // only used when hidlMemory.name() == "ashmem"
    const sp<GraphicBuffer>
            mGraphicBuffer;  // only used when hidlMemory.name() == "hardware_buffer_blob"
};

RunTimePoolInfo::RunTimePoolInfoImpl::RunTimePoolInfoImpl(const hidl_memory& hidlMemory,
                                                          uint8_t* buffer,
                                                          const sp<IMemory>& memory,
                                                          const sp<GraphicBuffer>& graphicBuffer)
    : mHidlMemory(hidlMemory), mBuffer(buffer), mMemory(memory), mGraphicBuffer(graphicBuffer) {}

RunTimePoolInfo::RunTimePoolInfoImpl::~RunTimePoolInfoImpl() {
    if (mBuffer == nullptr) {
        return;
    }

    const std::string memType = mHidlMemory.name();
    if (memType == "ashmem") {
        // nothing to do
    } else if (memType == "mmap_fd") {
        const size_t size = mHidlMemory.size();
        if (munmap(mBuffer, size)) {
            LOG(ERROR) << "RunTimePoolInfoImpl::~RunTimePoolInfo(): Can't munmap";
        }
    } else if (memType == "hardware_buffer_blob") {
        mGraphicBuffer->unlock();
    } else if (memType == "") {
        // Represents a POINTER argument; nothing to do
    } else {
        LOG(ERROR) << "RunTimePoolInfoImpl::~RunTimePoolInfoImpl(): unsupported hidl_memory type";
    }
}

// Making sure the output data are correctly updated after execution.
bool RunTimePoolInfo::RunTimePoolInfoImpl::update() const {
    const std::string memType = mHidlMemory.name();
    if (memType == "ashmem") {
        mMemory->commit();
        return true;
    }
    if (memType == "mmap_fd") {
        int prot = mHidlMemory.handle()->data[1];
        if (prot & PROT_WRITE) {
            const size_t size = mHidlMemory.size();
            return msync(mBuffer, size, MS_SYNC) == 0;
        }
    }
    // No-op for other types of memory.
    return true;
}

// TODO: short term, make share memory mapping and updating a utility function.
// TODO: long term, implement mmap_fd as a hidl IMemory service.
std::optional<RunTimePoolInfo> RunTimePoolInfo::createFromHidlMemory(
        const hidl_memory& hidlMemory) {
    uint8_t* buffer = nullptr;
    sp<IMemory> memory;
    sp<GraphicBuffer> graphicBuffer;

    const auto& memType = hidlMemory.name();
    if (memType == "ashmem") {
        memory = mapMemory(hidlMemory);
        if (memory == nullptr) {
            LOG(ERROR) << "Can't map shared memory.";
            return std::nullopt;
        }
        memory->update();
        buffer = reinterpret_cast<uint8_t*>(static_cast<void*>(memory->getPointer()));
        if (buffer == nullptr) {
            LOG(ERROR) << "Can't access shared memory.";
            return std::nullopt;
        }
    } else if (memType == "mmap_fd") {
        size_t size = hidlMemory.size();
        int fd = hidlMemory.handle()->data[0];
        int prot = hidlMemory.handle()->data[1];
        size_t offset = getSizeFromInts(hidlMemory.handle()->data[2], hidlMemory.handle()->data[3]);
        buffer = static_cast<uint8_t*>(mmap(nullptr, size, prot, MAP_SHARED, fd, offset));
        if (buffer == MAP_FAILED) {
            LOG(ERROR) << "RunTimePoolInfo::set(): Can't mmap the file descriptor.";
            return std::nullopt;
        }
    } else if (memType == "hardware_buffer_blob") {
        auto handle = hidlMemory.handle();
        auto format = AHARDWAREBUFFER_FORMAT_BLOB;
        auto usage = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
        const uint32_t width = hidlMemory.size();
        const uint32_t height = 1;  // height is always 1 for BLOB mode AHardwareBuffer.
        const uint32_t layers = 1;  // layers is always 1 for BLOB mode AHardwareBuffer.
        const uint32_t stride = hidlMemory.size();
        graphicBuffer = new GraphicBuffer(handle, GraphicBuffer::HandleWrapMethod::CLONE_HANDLE,
                                          width, height, format, layers, usage, stride);
        void* gBuffer = nullptr;
        int32_t outBytesPerPixel, outBytesPerStride;
        status_t status =
                graphicBuffer->lock(usage, &gBuffer, &outBytesPerPixel, &outBytesPerStride);
        if (status != NO_ERROR) {
            LOG(ERROR) << "RunTimePoolInfo Can't lock the AHardwareBuffer.";
            return std::nullopt;
        }
        buffer = static_cast<uint8_t*>(gBuffer);
    } else {
        LOG(ERROR) << "RunTimePoolInfo::set(): unsupported hidl_memory type";
        return std::nullopt;
    }

    const auto impl =
            std::make_shared<const RunTimePoolInfoImpl>(hidlMemory, buffer, memory, graphicBuffer);
    return {RunTimePoolInfo(impl)};
}

RunTimePoolInfo RunTimePoolInfo::createFromExistingBuffer(uint8_t* buffer) {
    const auto impl =
            std::make_shared<const RunTimePoolInfoImpl>(hidl_memory{}, buffer, nullptr, nullptr);
    return {impl};
}

RunTimePoolInfo::RunTimePoolInfo(const std::shared_ptr<const RunTimePoolInfoImpl>& impl)
    : mImpl(impl) {}

uint8_t* RunTimePoolInfo::getBuffer() const {
    return mImpl->getBuffer();
}

bool RunTimePoolInfo::update() const {
    return mImpl->update();
}

hidl_memory RunTimePoolInfo::getHidlMemory() const {
    return mImpl->getHidlMemory();
}

bool setRunTimePoolInfosFromHidlMemories(std::vector<RunTimePoolInfo>* poolInfos,
                                         const hidl_vec<hidl_memory>& pools) {
    CHECK(poolInfos != nullptr);
    poolInfos->clear();
    poolInfos->reserve(pools.size());
    for (const auto& pool : pools) {
        if (std::optional<RunTimePoolInfo> poolInfo = RunTimePoolInfo::createFromHidlMemory(pool)) {
            poolInfos->push_back(*poolInfo);
        } else {
            LOG(ERROR) << "Could not map pools";
            poolInfos->clear();
            return false;
        }
    }
    return true;
}

template <typename T>
inline bool convertToNhwcImpl(T* to, const T* from, const std::vector<uint32_t>& fromDim) {
    uint32_t spatialSize = fromDim[2] * fromDim[3];
    for (uint32_t n = 0; n < fromDim[0]; n++) {
        for (uint32_t hw = 0; hw < spatialSize; hw++) {
            for (uint32_t c = 0; c < fromDim[1]; c++) {
                uint32_t fromIndex = n * fromDim[1] * spatialSize + c * spatialSize + hw;
                *to++ = from[fromIndex];
            }
        }
    }
    return true;
}

template <typename T>
inline bool convertFromNhwcImpl(T* to, const T* from, const std::vector<uint32_t>& fromDim) {
    uint32_t spatialSize = fromDim[1] * fromDim[2];
    for (uint32_t n = 0; n < fromDim[0]; n++) {
        for (uint32_t c = 0; c < fromDim[3]; c++) {
            for (uint32_t hw = 0; hw < spatialSize; hw++) {
                uint32_t fromIndex = n * spatialSize * fromDim[3] + hw * fromDim[3] + c;
                *to++ = from[fromIndex];
            }
        }
    }
    return true;
}

static bool convertToNhwc(RunTimeOperandInfo& to, const RunTimeOperandInfo& from,
                          std::unique_ptr<uint8_t[]>& ptr_guard, bool data_layout) {
    int result;
    if (from.dimensions.size() != 4) {
        LOG(ERROR) << "Error converting a non-4-D tensor to NHWC layout";
        return false;
    }
    to.lifetime = OperandLifeTime::TEMPORARY_VARIABLE;
    if (data_layout) {
        // convert dimensions
        Shape inShape = from.shape();
        auto& fromDim = from.dimensions;
        inShape.dimensions = {fromDim[0], fromDim[2], fromDim[3], fromDim[1]};
        // allocate buffer
        to.buffer = nullptr;
        if (!setInfoAndAllocateIfNeeded(&to, inShape, &result)) {
            return false;
        }
        ptr_guard.reset(to.buffer);
        // convert value
        if (from.type == OperandType::TENSOR_FLOAT32) {
            return convertToNhwcImpl<float>(reinterpret_cast<float*>(to.buffer),
                                            reinterpret_cast<const float*>(from.buffer), fromDim);
        } else if (from.type == OperandType::TENSOR_FLOAT16) {
            return convertToNhwcImpl<_Float16>(reinterpret_cast<_Float16*>(to.buffer),
                                               reinterpret_cast<const _Float16*>(from.buffer),
                                               fromDim);
        } else if (from.type == OperandType::TENSOR_QUANT8_ASYMM) {
            return convertToNhwcImpl<uint8_t>(reinterpret_cast<uint8_t*>(to.buffer),
                                              reinterpret_cast<const uint8_t*>(from.buffer),
                                              fromDim);
        } else {
            LOG(ERROR) << "Unsupported data type";
            return false;
        }
    } else {
        to = from;
    }
    return true;
}

static bool convertFromNhwc(RunTimeOperandInfo& to, const RunTimeOperandInfo& from,
                            bool data_layout, int* result) {
    if (from.dimensions.size() != 4) {
        LOG(ERROR) << "Error converting a non-4-D tensor from NHWC layout";
        return false;
    }
    if (data_layout) {
        // convert dimensions
        Shape outShape = from.shape();
        auto& fromDim = from.dimensions;
        outShape.dimensions = {fromDim[0], fromDim[3], fromDim[1], fromDim[2]};
        // allocate buffer
        if (!setInfoAndAllocateIfNeeded(&to, outShape, result)) {
            return false;
        }
        // convert value
        if (from.type == OperandType::TENSOR_FLOAT32) {
            return convertFromNhwcImpl<float>(reinterpret_cast<float*>(to.buffer),
                                              reinterpret_cast<const float*>(from.buffer), fromDim);
        } else if (from.type == OperandType::TENSOR_FLOAT16) {
            return convertFromNhwcImpl<_Float16>(reinterpret_cast<_Float16*>(to.buffer),
                                                 reinterpret_cast<const _Float16*>(from.buffer),
                                                 fromDim);
        } else if (from.type == OperandType::TENSOR_QUANT8_ASYMM) {
            return convertFromNhwcImpl<uint8_t>(reinterpret_cast<uint8_t*>(to.buffer),
                                                reinterpret_cast<const uint8_t*>(from.buffer),
                                                fromDim);
        } else {
            LOG(ERROR) << "Unsupported data type";
            return false;
        }
    } else {
        Shape outShape = from.shape();
        to.buffer = from.buffer;
        to.length = from.length;
        if (!setInfoAndAllocateIfNeeded(&to, outShape, result)) {
            return false;
        }
    }
    return true;
}

// Ignore the .pools entry in model and request.  This will have been taken care of
// by the caller.
int CpuExecutor::run(const Model& model, const Request& request,
                     const std::vector<RunTimePoolInfo>& modelPoolInfos,
                     const std::vector<RunTimePoolInfo>& requestPoolInfos) {
    NNTRACE_CPU(NNTRACE_PHASE_EXECUTION, "run");
    VLOG(CPUEXE) << "CpuExecutor::run() with request(" << SHOW_IF_DEBUG(toString(request)) << ")";

    // b/109953668, disable OpenMP
#ifdef NNAPI_OPENMP
    ScopedOpenmpSettings openMpSettings;
#endif  // NNAPI_OPENMP

    mModel = &model;
    mRequest = &request;  // TODO check if mRequest is needed
    initializeRunTimeInfo(modelPoolInfos, requestPoolInfos);
    // The model has serialized the operation in execution order.
    for (const auto& operation : model.operations) {
        int n = executeOperation(operation);
        if (n != ANEURALNETWORKS_NO_ERROR) {
            finish(n);
            return n;
        }
    }
    for (auto& runtimeInfo : modelPoolInfos) {
        runtimeInfo.update();
    }
    for (auto& runtimeInfo : requestPoolInfos) {
        runtimeInfo.update();
    }
    finish(ANEURALNETWORKS_NO_ERROR);
    VLOG(CPUEXE) << "Completed run normally";
    return ANEURALNETWORKS_NO_ERROR;
}

bool CpuExecutor::initializeRunTimeInfo(const std::vector<RunTimePoolInfo>& modelPoolInfos,
                                        const std::vector<RunTimePoolInfo>& requestPoolInfos) {
    VLOG(CPUEXE) << "CpuExecutor::initializeRunTimeInfo";
    const size_t count = mModel->operands.size();
    mOperands.resize(count);

    // Start by setting the runtime info to what's in the model.
    for (size_t i = 0; i < count; i++) {
        const Operand& from = mModel->operands[i];
        RunTimeOperandInfo& to = mOperands[i];
        to.type = from.type;
        to.dimensions = from.dimensions;
        to.scale = from.scale;
        to.zeroPoint = from.zeroPoint;
        to.length = from.location.length;
        to.lifetime = from.lifetime;
        to.extraParams = from.extraParams;
        switch (from.lifetime) {
            case OperandLifeTime::TEMPORARY_VARIABLE:
                to.buffer = nullptr;
                to.numberOfUsesLeft = from.numberOfConsumers;
                break;
            case OperandLifeTime::CONSTANT_COPY:
                to.buffer = const_cast<uint8_t*>(&mModel->operandValues[from.location.offset]);
                to.numberOfUsesLeft = 0;
                break;
            case OperandLifeTime::CONSTANT_REFERENCE: {
                auto poolIndex = from.location.poolIndex;
                nnAssert(poolIndex < modelPoolInfos.size());
                auto& r = modelPoolInfos[poolIndex];
                to.buffer = r.getBuffer() + from.location.offset;
                to.numberOfUsesLeft = 0;
                break;
            }
            case OperandLifeTime::MODEL_INPUT:
            case OperandLifeTime::MODEL_OUTPUT:
            case OperandLifeTime::NO_VALUE:
                to.buffer = nullptr;
                to.numberOfUsesLeft = 0;
                break;
            default:
                nnAssert(false);
                break;
        }
    }

    // Adjust the runtime info for the arguments passed to the model,
    // modifying the buffer location, and possibly the dimensions.
    auto updateForArguments = [this, &requestPoolInfos](
                                      const std::vector<uint32_t>& indexes,
                                      const hidl_vec<RequestArgument>& arguments) {
        nnAssert(indexes.size() == arguments.size());
        for (size_t i = 0; i < indexes.size(); i++) {
            const uint32_t operandIndex = indexes[i];
            const RequestArgument& from = arguments[i];
            RunTimeOperandInfo& to = mOperands[operandIndex];
            if (from.dimensions.size() > 0) {
                // It's the responsibility of the caller to validate that
                // from.dimensions only modifies the dimensions that were
                // unspecified in the model.  That's the case in SampleDriver.cpp
                // with the call to validateRequest().
                // TODO make sure that's the case for the default CPU path.
                to.dimensions = from.dimensions;
            }
            if (from.hasNoValue) {
                to.lifetime = OperandLifeTime::NO_VALUE;
                nnAssert(to.buffer == nullptr);
                to.length = 0;
            } else {
                auto poolIndex = from.location.poolIndex;
                nnAssert(poolIndex < requestPoolInfos.size());
                auto& r = requestPoolInfos[poolIndex];
                to.buffer = r.getBuffer() + from.location.offset;
                to.length = from.location.length;
            }
        }
    };
    updateForArguments(mModel->inputIndexes, mRequest->inputs);
    updateForArguments(mModel->outputIndexes, mRequest->outputs);

    return true;
}

void CpuExecutor::freeNoLongerUsedOperands(const std::vector<uint32_t>& inputs) {
    for (uint32_t i : inputs) {
        auto& info = mOperands[i];
        // Check if it's a static or model input/output.
        if (info.numberOfUsesLeft == 0) {
            continue;
        }
        info.numberOfUsesLeft--;
        if (info.numberOfUsesLeft == 0 && info.buffer != nullptr) {
            delete[] info.buffer;
            info.buffer = nullptr;
        }
    }
}

int CpuExecutor::executeOperation(const Operation& operation) {
    // VLOG(CPUEXE) << "CpuExecutor::executeOperation(" << toString(operation) << ")";
    const hidl_vec<uint32_t>& ins = operation.inputs;
    const hidl_vec<uint32_t>& outs = operation.outputs;
    bool success = false;
    int result = ANEURALNETWORKS_NO_ERROR;

    // Function to verify that the number of input and output parameters
    // matches what is expected.  Also checks that all the parameters have
    // values. This function is to be used only for operations that do not
    // accept optional arguments.
    // TODO Have a version that works for optional arguments.
    auto allParametersPresent = [&operation, &ins, &outs, this](size_t requiredIns,
                                                                size_t requiredOuts) -> bool {
        auto verify = [&operation, this](size_t requiredCount, const hidl_vec<uint32_t>& indexes,
                                         const char* type) -> bool {
            size_t actualCount = indexes.size();
            if (actualCount != requiredCount) {
                LOG(ERROR) << getOperationName(operation.type) << ": Invalid number of " << type
                           << " operands. Got " << actualCount << " of " << requiredCount;
                return false;
            }
            for (size_t i = 0; i < actualCount; i++) {
                if (mOperands[indexes[i]].lifetime == OperandLifeTime::NO_VALUE) {
                    LOG(ERROR) << getOperationName(operation.type) << " " << type << " operand "
                               << i << " is required but missing.";
                    return false;
                }
            }
            return true;
        };

        auto verifyNoZeroSizedInputs = [&operation, this](const hidl_vec<uint32_t>& indexes) {
            for (size_t i = 0; i < indexes.size(); i++) {
                for (size_t j = 0; j < mOperands[indexes[i]].dimensions.size(); j++) {
                    if (mOperands[indexes[i]].dimensions[j] == 0) {
                        LOG(ERROR) << getOperationName(operation.type)
                                   << " does not support zero-sized tensor, but input " << i
                                   << " dimension " << j << " is zero.";
                        return false;
                    }
                }
            }
            return true;
        };

        return verify(requiredIns, ins, "in") && verify(requiredOuts, outs, "out") &&
               verifyNoZeroSizedInputs(ins);
    };

    switch (operation.type) {
        case OperationType::OEM_OPERATION: {
            LOG(ERROR) << "OEM operation not supported for CPU execution";
            success = false;
        } break;
        case OperationType::FLOOR: {
            if (!allParametersPresent(1, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            if (!floorPrepare(input.shape(), &outShape) ||
                !setInfoAndAllocateIfNeeded(&output, outShape, &result)) {
                break;
            }
            if (input.type == OperandType::TENSOR_FLOAT32) {
                success = floorFloat32(reinterpret_cast<const float*>(input.buffer),
                                       reinterpret_cast<float*>(output.buffer), outShape);
            } else if (input.type == OperandType::TENSOR_FLOAT16) {
                success = floorFloat16(reinterpret_cast<const _Float16*>(input.buffer),
                                       reinterpret_cast<_Float16*>(output.buffer), outShape);
            }
        } break;
        case OperationType::DEPTHWISE_CONV_2D: {
            const size_t inCount = ins.size();
            if ((inCount != 14 && inCount != 12 && inCount != 11 && inCount != 9 && inCount != 8) ||
                !allParametersPresent(inCount, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            const RunTimeOperandInfo& filter = mOperands[ins[1]];
            const RunTimeOperandInfo& bias = mOperands[ins[2]];

            int32_t padding_left, padding_right;
            int32_t padding_top, padding_bottom;
            int32_t padding_implicit = 0;
            int32_t stride_width, stride_height;
            int32_t dilation_width_factor = 1, dilation_height_factor = 1;
            int32_t depth_multiplier;
            int32_t activation;
            bool data_layout = false;
            bool useImplicitPadding = false;

            if ((inCount >= 9 && mOperands[ins[8]].type == OperandType::BOOL) || inCount == 8) {
                padding_implicit = getScalarData<int32_t>(mOperands[ins[3]]);
                stride_width = getScalarData<int32_t>(mOperands[ins[4]]);
                stride_height = getScalarData<int32_t>(mOperands[ins[5]]);
                depth_multiplier = getScalarData<int32_t>(mOperands[ins[6]]);
                activation = getScalarData<int32_t>(mOperands[ins[7]]);
                if (inCount >= 9) {
                    data_layout = getScalarData<bool>(mOperands[ins[8]]);
                }
                if (inCount == 11) {
                    dilation_width_factor = getScalarData<int32_t>(mOperands[ins[9]]);
                    dilation_height_factor = getScalarData<int32_t>(mOperands[ins[10]]);
                }
                useImplicitPadding = true;
            } else if (inCount >= 11 && mOperands[ins[8]].type == OperandType::INT32) {
                padding_left = getScalarData<int32_t>(mOperands[ins[3]]);
                padding_right = getScalarData<int32_t>(mOperands[ins[4]]);
                padding_top = getScalarData<int32_t>(mOperands[ins[5]]);
                padding_bottom = getScalarData<int32_t>(mOperands[ins[6]]);
                stride_width = getScalarData<int32_t>(mOperands[ins[7]]);
                stride_height = getScalarData<int32_t>(mOperands[ins[8]]);
                depth_multiplier = getScalarData<int32_t>(mOperands[ins[9]]);
                activation = getScalarData<int32_t>(mOperands[ins[10]]);
                if (inCount >= 12) {
                    data_layout = getScalarData<bool>(mOperands[ins[11]]);
                }
                if (inCount == 14) {
                    dilation_width_factor = getScalarData<int32_t>(mOperands[ins[12]]);
                    dilation_height_factor = getScalarData<int32_t>(mOperands[ins[13]]);
                }
            } else {
                return ANEURALNETWORKS_BAD_DATA;
            }

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            RunTimeOperandInfo input_tmp, output_tmp;
            std::unique_ptr<uint8_t[]> input_tmp_guard, output_tmp_guard;
            if (!convertToNhwc(input_tmp, input, input_tmp_guard, data_layout)) {
                success = false;
                break;
            }
            output_tmp.lifetime = OperandLifeTime::TEMPORARY_VARIABLE;
            output_tmp.buffer = data_layout ? nullptr : output.buffer;
            output_tmp.length = data_layout ? 0 : output.length;

            if (useImplicitPadding) {
                Shape inputShape = input_tmp.shape();
                Shape filterShape = filter.shape();
                int32_t input_width = getSizeOfDimension(inputShape, 2);
                int32_t input_height = getSizeOfDimension(inputShape, 1);
                int32_t filter_width = getSizeOfDimension(filterShape, 2);
                int32_t filter_height = getSizeOfDimension(filterShape, 1);
                calculateExplicitPadding(input_width, stride_width, dilation_width_factor,
                                         filter_width, padding_implicit, &padding_left,
                                         &padding_right);
                calculateExplicitPadding(input_height, stride_height, dilation_height_factor,
                                         filter_height, padding_implicit, &padding_top,
                                         &padding_bottom);
            }

            if (!depthwiseConvPrepare(input_tmp.shape(), filter.shape(), bias.shape(), padding_left,
                                      padding_right, padding_top, padding_bottom, stride_width,
                                      stride_height, depth_multiplier, dilation_width_factor,
                                      dilation_height_factor, &outShape) ||
                !setInfoAndAllocateIfNeeded(&output_tmp, outShape, &result)) {
                if (!data_layout) output.dimensions = output_tmp.dimensions;
                success = false;
                break;
            }
            if (input_tmp.type == OperandType::TENSOR_FLOAT32) {
                success = depthwiseConvFloat32(
                        reinterpret_cast<const float*>(input_tmp.buffer), input_tmp.shape(),
                        reinterpret_cast<const float*>(filter.buffer), filter.shape(),
                        reinterpret_cast<const float*>(bias.buffer), bias.shape(), padding_left,
                        padding_right, padding_top, padding_bottom, stride_width, stride_height,
                        dilation_width_factor, dilation_height_factor, depth_multiplier, activation,
                        reinterpret_cast<float*>(output_tmp.buffer), outShape);
            } else if (input_tmp.type == OperandType::TENSOR_FLOAT16) {
                success = depthwiseConvFloat16(
                        reinterpret_cast<const _Float16*>(input_tmp.buffer), input_tmp.shape(),
                        reinterpret_cast<const _Float16*>(filter.buffer), filter.shape(),
                        reinterpret_cast<const _Float16*>(bias.buffer), bias.shape(), padding_left,
                        padding_right, padding_top, padding_bottom, stride_width, stride_height,
                        dilation_width_factor, dilation_height_factor, depth_multiplier, activation,
                        reinterpret_cast<_Float16*>(output_tmp.buffer), outShape);
            } else if (input_tmp.type == OperandType::TENSOR_QUANT8_ASYMM) {
                if (filter.type == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
                    success = depthwiseConvQuant8PerChannel(
                            reinterpret_cast<const uint8_t*>(input_tmp.buffer), input_tmp.shape(),
                            reinterpret_cast<const int8_t*>(filter.buffer), filter.shape(),
                            filter.extraParams.channelQuant().scales.data(),
                            reinterpret_cast<const int32_t*>(bias.buffer), bias.shape(),
                            padding_left, padding_right, padding_top, padding_bottom, stride_width,
                            stride_height, dilation_width_factor, dilation_height_factor,
                            depth_multiplier, activation,
                            reinterpret_cast<uint8_t*>(output_tmp.buffer), outShape);
                } else if (filter.type == OperandType::TENSOR_QUANT8_ASYMM) {
                    success = depthwiseConvQuant8(
                            reinterpret_cast<const uint8_t*>(input_tmp.buffer), input_tmp.shape(),
                            reinterpret_cast<const uint8_t*>(filter.buffer), filter.shape(),
                            reinterpret_cast<const int32_t*>(bias.buffer), bias.shape(),
                            padding_left, padding_right, padding_top, padding_bottom, stride_width,
                            stride_height, dilation_width_factor, dilation_height_factor,
                            depth_multiplier, activation,
                            reinterpret_cast<uint8_t*>(output_tmp.buffer), outShape);
                }
            }
            if (data_layout) {
                output_tmp_guard.reset(output_tmp.buffer);
            }
            if (!success || !convertFromNhwc(output, output_tmp, data_layout, &result)) {
                success = false;
                break;
            }
        } break;
        case OperationType::LOCAL_RESPONSE_NORMALIZATION: {
            const size_t inCount = ins.size();
            if ((inCount != 6 && inCount != 5) || !allParametersPresent(inCount, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            int32_t radius = getScalarData<int32_t>(mOperands[ins[1]]);
            float bias = (input.type == OperandType::TENSOR_FLOAT16)
                                 ? getScalarData<_Float16>(mOperands[ins[2]])
                                 : getScalarData<float>(mOperands[ins[2]]);
            float alpha = (input.type == OperandType::TENSOR_FLOAT16)
                                  ? getScalarData<_Float16>(mOperands[ins[3]])
                                  : getScalarData<float>(mOperands[ins[3]]);
            float beta = (input.type == OperandType::TENSOR_FLOAT16)
                                 ? getScalarData<_Float16>(mOperands[ins[4]])
                                 : getScalarData<float>(mOperands[ins[4]]);
            const int32_t axis = inCount == 6 ? getScalarData<int32_t>(mOperands[ins[5]]) : -1;

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            if (!genericNormalizationPrepare(input.shape(), &outShape) ||
                !setInfoAndAllocateIfNeeded(&output, outShape, &result)) {
                success = false;
                break;
            }
            if (input.type == OperandType::TENSOR_FLOAT32) {
                success = localResponseNormFloat32(
                        reinterpret_cast<const float*>(input.buffer), input.shape(), radius, bias,
                        alpha, beta, axis, reinterpret_cast<float*>(output.buffer), outShape);
            } else if (input.type == OperandType::TENSOR_FLOAT16) {
                success = localResponseNormFloat16(reinterpret_cast<const _Float16*>(input.buffer),
                                                   input.shape(), radius, bias, alpha, beta, axis,
                                                   reinterpret_cast<_Float16*>(output.buffer),
                                                   outShape);
            }
        } break;
        case OperationType::RESHAPE: {
            if (!allParametersPresent(2, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            const RunTimeOperandInfo& targetShape = mOperands[ins[1]];

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            success = reshapePrepare(input.shape(),
                                     reinterpret_cast<const int32_t*>(targetShape.buffer),
                                     getNumberOfElements(targetShape.shape()), &outShape) &&
                      setInfoAndAllocateIfNeeded(&output, outShape, &result) &&
                      copyData(input.buffer, input.shape(), output.buffer, outShape);
        } break;
        case OperationType::DEPTH_TO_SPACE: {
            const size_t inCount = ins.size();
            if ((inCount != 3 && inCount != 2) || !allParametersPresent(inCount, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            int32_t blockSize = getScalarData<int32_t>(mOperands[ins[1]]);
            bool data_layout = inCount == 3 ? getScalarData<bool>(mOperands[ins[2]]) : false;

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            RunTimeOperandInfo input_tmp, output_tmp;
            std::unique_ptr<uint8_t[]> input_tmp_guard, output_tmp_guard;
            if (!convertToNhwc(input_tmp, input, input_tmp_guard, data_layout)) {
                success = false;
                break;
            }
            output_tmp.lifetime = OperandLifeTime::TEMPORARY_VARIABLE;
            output_tmp.buffer = data_layout ? nullptr : output.buffer;
            output_tmp.length = data_layout ? 0 : output.length;
            if (!depthToSpacePrepare(input_tmp.shape(), blockSize, &outShape) ||
                !setInfoAndAllocateIfNeeded(&output_tmp, outShape, &result)) {
                if (!data_layout) output.dimensions = output_tmp.dimensions;
                break;
            }
            switch (input_tmp.type) {
                case OperandType::TENSOR_FLOAT32: {
                    success = depthToSpaceGeneric(
                            reinterpret_cast<const float*>(input_tmp.buffer), input_tmp.shape(),
                            blockSize, reinterpret_cast<float*>(output_tmp.buffer), outShape);
                    break;
                }
                case OperandType::TENSOR_FLOAT16: {
                    success = depthToSpaceGeneric(
                            reinterpret_cast<const _Float16*>(input_tmp.buffer), input_tmp.shape(),
                            blockSize, reinterpret_cast<_Float16*>(output_tmp.buffer), outShape);
                    break;
                }
                case OperandType::TENSOR_QUANT8_ASYMM: {
                    success = depthToSpaceGeneric(
                            reinterpret_cast<const uint8_t*>(input_tmp.buffer), input_tmp.shape(),
                            blockSize, reinterpret_cast<uint8_t*>(output_tmp.buffer), outShape);
                    break;
                }
                default: {
                    LOG(ERROR) << "Unsupported data type";
                    success = false;
                }
            }
            if (data_layout) {
                output_tmp_guard.reset(output_tmp.buffer);
            }
            if (!success || !convertFromNhwc(output, output_tmp, data_layout, &result)) {
                success = false;
                break;
            }
        } break;
        case OperationType::SPACE_TO_DEPTH: {
            const size_t inCount = ins.size();
            if ((inCount != 3 && inCount != 2) || !allParametersPresent(inCount, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            int32_t blockSize = getScalarData<int32_t>(mOperands[ins[1]]);
            bool data_layout = inCount == 3 ? getScalarData<bool>(mOperands[ins[2]]) : false;

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            RunTimeOperandInfo input_tmp, output_tmp;
            std::unique_ptr<uint8_t[]> input_tmp_guard, output_tmp_guard;
            if (!convertToNhwc(input_tmp, input, input_tmp_guard, data_layout)) {
                success = false;
                break;
            }
            output_tmp.lifetime = OperandLifeTime::TEMPORARY_VARIABLE;
            output_tmp.buffer = data_layout ? nullptr : output.buffer;
            output_tmp.length = data_layout ? 0 : output.length;

            if (!spaceToDepthPrepare(input_tmp.shape(), blockSize, &outShape) ||
                !setInfoAndAllocateIfNeeded(&output_tmp, outShape, &result)) {
                if (!data_layout) output.dimensions = output_tmp.dimensions;
                break;
            }
            switch (input_tmp.type) {
                case OperandType::TENSOR_FLOAT32: {
                    success = spaceToDepthGeneric(
                            reinterpret_cast<const float*>(input_tmp.buffer), input_tmp.shape(),
                            blockSize, reinterpret_cast<float*>(output_tmp.buffer), outShape);
                    break;
                }
                case OperandType::TENSOR_FLOAT16: {
                    success = spaceToDepthGeneric(
                            reinterpret_cast<const _Float16*>(input_tmp.buffer), input_tmp.shape(),
                            blockSize, reinterpret_cast<_Float16*>(output_tmp.buffer), outShape);
                    break;
                }
                case OperandType::TENSOR_QUANT8_ASYMM: {
                    success = spaceToDepthGeneric(
                            reinterpret_cast<const uint8_t*>(input_tmp.buffer), input_tmp.shape(),
                            blockSize, reinterpret_cast<uint8_t*>(output_tmp.buffer), outShape);
                    break;
                }
                default: {
                    LOG(ERROR) << "Unsupported data type";
                    success = false;
                }
            }
            if (data_layout) {
                output_tmp_guard.reset(output_tmp.buffer);
            }
            if (!success || !convertFromNhwc(output, output_tmp, data_layout, &result)) {
                success = false;
                break;
            }
        } break;
        case OperationType::EMBEDDING_LOOKUP: {
            const RunTimeOperandInfo& values = mOperands[ins[EmbeddingLookup::kValueTensor]];
            const RunTimeOperandInfo& lookups = mOperands[ins[EmbeddingLookup::kLookupTensor]];
            RunTimeOperandInfo& output = mOperands[outs[EmbeddingLookup::kOutputTensor]];

            Shape outputShape;
            EmbeddingLookup lookup(operation, mOperands);

            success = embeddingLookupPrepare(values.shape(), lookups.shape(), &outputShape) &&
                      setInfoAndAllocateIfNeeded(&output, outputShape, &result) && lookup.Eval();
        } break;
        case OperationType::HASHTABLE_LOOKUP: {
            const RunTimeOperandInfo& lookups = mOperands[ins[HashtableLookup::kLookupTensor]];
            const RunTimeOperandInfo& keys = mOperands[ins[HashtableLookup::kKeyTensor]];
            const RunTimeOperandInfo& values = mOperands[ins[HashtableLookup::kValueTensor]];

            RunTimeOperandInfo& output = mOperands[outs[HashtableLookup::kOutputTensor]];
            RunTimeOperandInfo& hits = mOperands[outs[HashtableLookup::kHitsTensor]];

            Shape outputShape, hitShape;
            HashtableLookup lookup(operation, mOperands);

            success = hashtableLookupPrepare(lookups.shape(), keys.shape(), values.shape(),
                                             &outputShape, &hitShape) &&
                      setInfoAndAllocateIfNeeded(&output, outputShape, &result) &&
                      setInfoAndAllocateIfNeeded(&hits, hitShape, &result) && lookup.Eval();
        } break;
        case OperationType::LSH_PROJECTION: {
            RunTimeOperandInfo& output = mOperands[outs[LSHProjection::kOutputTensor]];
            Shape outputShape;
            if (!LSHProjection::Prepare(operation, mOperands, &outputShape) ||
                !setInfoAndAllocateIfNeeded(&output, outputShape, &result)) {
                break;
            }

            LSHProjection lsh(operation, mOperands);
            const RunTimeOperandInfo& hash = mOperands[ins[LSHProjection::kHashTensor]];
            switch (hash.type) {
                case OperandType::TENSOR_FLOAT32: {
                    success = lsh.Eval<float>();
                    break;
                }
                case OperandType::TENSOR_FLOAT16: {
                    success = lsh.Eval<_Float16>();
                    break;
                }
                default: {
                    success = false;
                    LOG(ERROR) << "Unsupported data type";
                }
            }
        } break;
        case OperationType::BIDIRECTIONAL_SEQUENCE_LSTM: {
            const auto merge_outputs = getScalarData<bool>(
                    mOperands[ins[BidirectionalSequenceLSTM::kMergeOutputsParam]]);
            RunTimeOperandInfo& fwOutput =
                    mOperands[outs[BidirectionalSequenceLSTM::kFwOutputTensor]];
            Shape fwOutputShape, bwOutputShape;

            BidirectionalSequenceLSTM lstm(operation, mOperands);
            success = lstm.Prepare(operation, mOperands, &fwOutputShape, &bwOutputShape) &&
                      setInfoAndAllocateIfNeeded(&fwOutput, fwOutputShape, &result);
            if (!merge_outputs) {
                RunTimeOperandInfo& bwOutput =
                        mOperands[outs[BidirectionalSequenceLSTM::kBwOutputTensor]];
                success = success && setInfoAndAllocateIfNeeded(&bwOutput, bwOutputShape, &result);
            }
            success = success && lstm.Eval();
        } break;
        case OperationType::LSTM: {
            RunTimeOperandInfo& scratch = mOperands[outs[LSTMCell::kScratchBufferTensor]];
            RunTimeOperandInfo& outputStateOut = mOperands[outs[LSTMCell::kOutputStateOutTensor]];
            RunTimeOperandInfo& cellStateOut = mOperands[outs[LSTMCell::kCellStateOutTensor]];
            RunTimeOperandInfo& output = mOperands[outs[LSTMCell::kOutputTensor]];

            Shape scratchShape, outputStateShape, cellStateShape, outputShape;
            LSTMCell lstm_cell(operation, mOperands);

            success = lstm_cell.Prepare(operation, mOperands, &scratchShape, &outputStateShape,
                                        &cellStateShape, &outputShape) &&
                      setInfoAndAllocateIfNeeded(&scratch, scratchShape, &result) &&
                      setInfoAndAllocateIfNeeded(&outputStateOut, outputStateShape, &result) &&
                      setInfoAndAllocateIfNeeded(&cellStateOut, cellStateShape, &result) &&
                      setInfoAndAllocateIfNeeded(&output, outputShape, &result) && lstm_cell.Eval();
        } break;
        case OperationType::RANDOM_MULTINOMIAL: {
            const RunTimeOperandInfo& lookups = mOperands[ins[HashtableLookup::kLookupTensor]];
            const RunTimeOperandInfo& keys = mOperands[ins[HashtableLookup::kKeyTensor]];
            const RunTimeOperandInfo& values = mOperands[ins[HashtableLookup::kValueTensor]];
            RunTimeOperandInfo& output = mOperands[outs[Multinomial::kOutputTensor]];

            Shape outputShape;
            Multinomial multinomial(operation, mOperands);

            success = Multinomial::Prepare(operation, mOperands, &outputShape) &&
                      setInfoAndAllocateIfNeeded(&output, outputShape, &result) &&
                      multinomial.Eval();
        } break;
        case OperationType::RNN: {
            RunTimeOperandInfo& hiddenStateOut = mOperands[outs[RNN::kHiddenStateOutTensor]];
            RunTimeOperandInfo& output = mOperands[outs[RNN::kOutputTensor]];

            Shape hiddenStateShape, outputShape;
            RNN rnn_cell(operation, mOperands);

            success = RNN::Prepare(operation, mOperands, &hiddenStateShape, &outputShape) &&
                      setInfoAndAllocateIfNeeded(&hiddenStateOut, hiddenStateShape, &result) &&
                      setInfoAndAllocateIfNeeded(&output, outputShape, &result) && rnn_cell.Eval();
        } break;
        case OperationType::SVDF: {
            RunTimeOperandInfo& stateOut = mOperands[outs[SVDF::kStateOutTensor]];
            RunTimeOperandInfo& output = mOperands[outs[SVDF::kOutputTensor]];

            Shape stateShape, outputShape;
            SVDF svdf(operation, mOperands);

            success = SVDF::Prepare(operation, mOperands, &stateShape, &outputShape) &&
                      setInfoAndAllocateIfNeeded(&stateOut, stateShape, &result) &&
                      setInfoAndAllocateIfNeeded(&output, outputShape, &result) && svdf.Eval();
        } break;
        case OperationType::BATCH_TO_SPACE_ND: {
            const size_t inCount = ins.size();
            if ((inCount != 3 && inCount != 2) || !allParametersPresent(inCount, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            const RunTimeOperandInfo& blockSize = mOperands[ins[1]];
            bool data_layout = inCount == 3 ? getScalarData<bool>(mOperands[ins[2]]) : false;

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            RunTimeOperandInfo input_tmp, output_tmp;
            std::unique_ptr<uint8_t[]> input_tmp_guard, output_tmp_guard;
            if (!convertToNhwc(input_tmp, input, input_tmp_guard, data_layout)) {
                success = false;
                break;
            }
            output_tmp.lifetime = OperandLifeTime::TEMPORARY_VARIABLE;
            output_tmp.buffer = data_layout ? nullptr : output.buffer;
            output_tmp.length = data_layout ? 0 : output.length;

            if (!batchToSpacePrepare(input_tmp.shape(),
                                     reinterpret_cast<const int32_t*>(blockSize.buffer),
                                     blockSize.shape(), &outShape) ||
                !setInfoAndAllocateIfNeeded(&output_tmp, outShape, &result)) {
                if (!data_layout) output.dimensions = output_tmp.dimensions;
                break;
            }
            switch (input_tmp.type) {
                case OperandType::TENSOR_FLOAT32: {
                    success = batchToSpaceGeneric(
                            reinterpret_cast<const float*>(input_tmp.buffer), input_tmp.shape(),
                            reinterpret_cast<const int32_t*>(blockSize.buffer),
                            reinterpret_cast<float*>(output_tmp.buffer), outShape);
                    break;
                }
                case OperandType::TENSOR_FLOAT16: {
                    success = batchToSpaceGeneric(
                            reinterpret_cast<const _Float16*>(input_tmp.buffer), input_tmp.shape(),
                            reinterpret_cast<const int32_t*>(blockSize.buffer),
                            reinterpret_cast<_Float16*>(output_tmp.buffer), outShape);
                    break;
                }
                case OperandType::TENSOR_QUANT8_ASYMM: {
                    success = batchToSpaceGeneric(
                            reinterpret_cast<const uint8_t*>(input_tmp.buffer), input_tmp.shape(),
                            reinterpret_cast<const int32_t*>(blockSize.buffer),
                            reinterpret_cast<uint8_t*>(output_tmp.buffer), outShape);
                    break;
                }
                default: {
                    LOG(ERROR) << "Unsupported data type";
                    success = false;
                }
            }
            if (data_layout) {
                output_tmp_guard.reset(output_tmp.buffer);
            }
            if (!success || !convertFromNhwc(output, output_tmp, data_layout, &result)) {
                success = false;
                break;
            }
        } break;
        case OperationType::SPACE_TO_BATCH_ND: {
            const size_t inCount = ins.size();
            if ((inCount != 4 && inCount != 3) || !allParametersPresent(inCount, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            const RunTimeOperandInfo& blockSize = mOperands[ins[1]];
            const RunTimeOperandInfo& paddings = mOperands[ins[2]];
            bool data_layout = inCount == 4 ? getScalarData<bool>(mOperands[ins[3]]) : false;

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            RunTimeOperandInfo input_tmp, output_tmp;
            std::unique_ptr<uint8_t[]> input_tmp_guard, output_tmp_guard;
            if (!convertToNhwc(input_tmp, input, input_tmp_guard, data_layout)) {
                success = false;
                break;
            }
            output_tmp.lifetime = OperandLifeTime::TEMPORARY_VARIABLE;
            output_tmp.buffer = data_layout ? nullptr : output.buffer;
            output_tmp.length = data_layout ? 0 : output.length;

            if (!spaceToBatchPrepare(
                        input_tmp.shape(), reinterpret_cast<const int32_t*>(blockSize.buffer),
                        blockSize.shape(), reinterpret_cast<const int32_t*>(paddings.buffer),
                        paddings.shape(), &outShape) ||
                !setInfoAndAllocateIfNeeded(&output_tmp, outShape, &result)) {
                if (!data_layout) output.dimensions = output_tmp.dimensions;
                break;
            }
            switch (input_tmp.type) {
                case OperandType::TENSOR_FLOAT32: {
                    success = spaceToBatchGeneric(
                            reinterpret_cast<const float*>(input_tmp.buffer), input_tmp.shape(),
                            reinterpret_cast<const int32_t*>(blockSize.buffer),
                            reinterpret_cast<const int32_t*>(paddings.buffer), paddings.shape(),
                            reinterpret_cast<float*>(output_tmp.buffer), outShape);
                    break;
                }
                case OperandType::TENSOR_FLOAT16: {
                    success = spaceToBatchGeneric(
                            reinterpret_cast<const _Float16*>(input_tmp.buffer), input_tmp.shape(),
                            reinterpret_cast<const int32_t*>(blockSize.buffer),
                            reinterpret_cast<const int32_t*>(paddings.buffer), paddings.shape(),
                            reinterpret_cast<_Float16*>(output_tmp.buffer), outShape);
                    break;
                }
                case OperandType::TENSOR_QUANT8_ASYMM: {
                    success = spaceToBatchGeneric(
                            reinterpret_cast<const uint8_t*>(input_tmp.buffer), input_tmp.shape(),
                            reinterpret_cast<const int32_t*>(blockSize.buffer),
                            reinterpret_cast<const int32_t*>(paddings.buffer), paddings.shape(),
                            reinterpret_cast<uint8_t*>(output_tmp.buffer), outShape);
                    break;
                }
                default: {
                    LOG(ERROR) << "Unsupported data type";
                    success = false;
                }
            }
            if (data_layout) {
                output_tmp_guard.reset(output_tmp.buffer);
            }
            if (!success || !convertFromNhwc(output, output_tmp, data_layout, &result)) {
                success = false;
                break;
            }
        } break;
        case OperationType::PAD:
        case OperationType::PAD_V2: {
            const bool isV2 = operation.type == OperationType::PAD_V2;
            if (!allParametersPresent(isV2 ? 3 : 2, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            const RunTimeOperandInfo& paddings = mOperands[ins[1]];

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            if (!padPrepare(input.shape(), reinterpret_cast<const int32_t*>(paddings.buffer),
                            paddings.shape(), &outShape) ||
                !setInfoAndAllocateIfNeeded(&output, outShape, &result)) {
                break;
            }
            if (input.type == OperandType::TENSOR_FLOAT32) {
                float pad_value = isV2 ? getScalarData<float>(mOperands[ins[2]]) : 0;
                success = padGeneric(reinterpret_cast<const float*>(input.buffer), input.shape(),
                                     reinterpret_cast<const int32_t*>(paddings.buffer), pad_value,
                                     reinterpret_cast<float*>(output.buffer), outShape);
            } else if (input.type == OperandType::TENSOR_FLOAT16) {
                _Float16 pad_value = isV2 ? getScalarData<_Float16>(mOperands[ins[2]]) : 0;
                success = padGeneric(reinterpret_cast<const _Float16*>(input.buffer), input.shape(),
                                     reinterpret_cast<const int32_t*>(paddings.buffer),
                                     static_cast<_Float16>(pad_value),
                                     reinterpret_cast<_Float16*>(output.buffer), outShape);
            } else if (input.type == OperandType::TENSOR_QUANT8_ASYMM) {
                uint8_t pad_value =
                        isV2 ? getScalarData<uint8_t>(mOperands[ins[2]]) : outShape.offset;
                success = padGeneric(input.buffer, input.shape(),
                                     reinterpret_cast<const int32_t*>(paddings.buffer), pad_value,
                                     output.buffer, outShape);
            }
        } break;
        case OperationType::CAST: {
            if (!allParametersPresent(1, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            success = cast::prepare(input.shape(), &outShape) &&
                      setInfoAndAllocateIfNeeded(&output, outShape, &result) &&
                      cast::eval(input.buffer, input.shape(), output.buffer, outShape);
        } break;
        case OperationType::SQUEEZE: {
            if (ins.size() != 2 || outs.size() != 1 ||
                mOperands[ins[0]].lifetime == OperandLifeTime::NO_VALUE ||
                mOperands[outs[0]].lifetime == OperandLifeTime::NO_VALUE) {
                LOG(ERROR) << "Wrong input/output count or lifetime for SQUEEZE op.";
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            const RunTimeOperandInfo& squeezeDims = mOperands[ins[1]];

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            success = squeezePrepare(input.shape(),
                                     reinterpret_cast<const int32_t*>(squeezeDims.buffer),
                                     squeezeDims.shape(), &outShape) &&
                      setInfoAndAllocateIfNeeded(&output, outShape, &result) &&
                      copyData(input.buffer, input.shape(), output.buffer, outShape);
        } break;
        case OperationType::STRIDED_SLICE: {
            if (!allParametersPresent(7, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            const RunTimeOperandInfo& begins = mOperands[ins[1]];
            const RunTimeOperandInfo& ends = mOperands[ins[2]];
            const RunTimeOperandInfo& strides = mOperands[ins[3]];
            int32_t beginMask = getScalarData<int32_t>(mOperands[ins[4]]);
            int32_t endMask = getScalarData<int32_t>(mOperands[ins[5]]);
            int32_t shrinkAxisMask = getScalarData<int32_t>(mOperands[ins[6]]);

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            success =
                    stridedSlicePrepare(
                            input.shape(), reinterpret_cast<const int32_t*>(begins.buffer),
                            begins.shape(), reinterpret_cast<const int32_t*>(ends.buffer),
                            ends.shape(), reinterpret_cast<const int32_t*>(strides.buffer),
                            strides.shape(), beginMask, endMask, shrinkAxisMask, &outShape) &&
                    setInfoAndAllocateIfNeeded(&output, outShape, &result) &&
                    stridedSliceGeneric(input.buffer, input.shape(),
                                        reinterpret_cast<const int32_t*>(begins.buffer),
                                        reinterpret_cast<const int32_t*>(ends.buffer),
                                        reinterpret_cast<const int32_t*>(strides.buffer), beginMask,
                                        endMask, shrinkAxisMask, output.buffer, outShape);
        } break;
        case OperationType::MEAN: {
            if (!allParametersPresent(3, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            const RunTimeOperandInfo& axis = mOperands[ins[1]];
            int32_t keepDims = getScalarData<int32_t>(mOperands[ins[2]]);

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            if (!meanPrepare(input.shape(), reinterpret_cast<const int32_t*>(axis.buffer),
                             axis.shape(), keepDims > 0, &outShape) ||
                !setInfoAndAllocateIfNeeded(&output, outShape, &result)) {
                break;
            }
            if (input.type == OperandType::TENSOR_FLOAT16) {
                success = meanFloat16(reinterpret_cast<_Float16*>(input.buffer), input.shape(),
                                      reinterpret_cast<const int32_t*>(axis.buffer), axis.shape(),
                                      keepDims > 0, reinterpret_cast<_Float16*>(output.buffer),
                                      outShape);
            } else if (input.type == OperandType::TENSOR_FLOAT32) {
                success = meanGeneric<float, float>(
                        reinterpret_cast<float*>(input.buffer), input.shape(),
                        reinterpret_cast<const int32_t*>(axis.buffer), axis.shape(), keepDims > 0,
                        reinterpret_cast<float*>(output.buffer), outShape);
            } else if (input.type == OperandType::TENSOR_QUANT8_ASYMM) {
                success = meanGeneric<uint8_t, int32_t>(
                        reinterpret_cast<uint8_t*>(input.buffer), input.shape(),
                        reinterpret_cast<const int32_t*>(axis.buffer), axis.shape(), keepDims > 0,
                        reinterpret_cast<uint8_t*>(output.buffer), outShape);
            }
        } break;
        case OperationType::ARGMAX:
        case OperationType::ARGMIN: {
            if (!allParametersPresent(2, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            int32_t axis = getScalarData<int32_t>(mOperands[ins[1]]);

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            const bool isArgMin = operation.type == OperationType::ARGMIN;
            success = argMinMaxPrepare(input.shape(), axis, &outShape) &&
                      setInfoAndAllocateIfNeeded(&output, outShape, &result) &&
                      argMinMaxGeneric(input.buffer, input.shape(), axis, isArgMin, output.buffer,
                                       outShape);
        } break;
        case OperationType::EXPAND_DIMS: {
            if (!allParametersPresent(2, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            int32_t axis = getScalarData<int32_t>(mOperands[ins[1]]);

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            success = expand_dims::prepare(input.shape(), axis, &outShape) &&
                      setInfoAndAllocateIfNeeded(&output, outShape, &result) &&
                      expand_dims::eval(input.buffer, input.shape(), axis, output.buffer, outShape);
        } break;
        case OperationType::SPLIT: {
            if (ins.size() != 3) {
                LOG(ERROR) << "Wrong input count";
                return ANEURALNETWORKS_BAD_DATA;
            }

            const RunTimeOperandInfo& input = mOperands[ins[0]];
            const int32_t axis = getScalarData<int32_t>(mOperands[ins[1]]);
            const int32_t numOutputs = getScalarData<int32_t>(mOperands[ins[2]]);

            if (numOutputs != outs.size()) {
                return ANEURALNETWORKS_BAD_DATA;
            }

            std::vector<Shape> outputShapes(numOutputs);
            for (int i = 0; i < numOutputs; ++i) {
                outputShapes[i] = mOperands[outs[i]].shape();
            }

            success = splitPrepare(input.shape(), axis, numOutputs, &outputShapes);
            for (int i = 0; i < numOutputs; ++i) {
                success = success && setInfoAndAllocateIfNeeded(&(mOperands[outs[i]]),
                                                                outputShapes[i], &result);
            }
            switch (input.type) {
                case OperandType::TENSOR_FLOAT16: {
                    std::vector<_Float16*> outputDataPtrs(numOutputs);
                    for (int i = 0; i < numOutputs; ++i) {
                        outputDataPtrs[i] = reinterpret_cast<_Float16*>(mOperands[outs[i]].buffer);
                    }
                    success = success &&
                              splitFloat16(reinterpret_cast<const _Float16*>(input.buffer),
                                           input.shape(), axis, &outputDataPtrs, outputShapes);
                } break;
                case OperandType::TENSOR_FLOAT32: {
                    std::vector<float*> outputDataPtrs(numOutputs);
                    for (int i = 0; i < numOutputs; ++i) {
                        outputDataPtrs[i] = reinterpret_cast<float*>(mOperands[outs[i]].buffer);
                    }
                    success = success &&
                              splitFloat32(reinterpret_cast<const float*>(input.buffer),
                                           input.shape(), axis, &outputDataPtrs, outputShapes);
                } break;
                case OperandType::TENSOR_INT32: {
                    std::vector<int32_t*> outputDataPtrs(numOutputs);
                    for (int i = 0; i < numOutputs; ++i) {
                        outputDataPtrs[i] = reinterpret_cast<int32_t*>(mOperands[outs[i]].buffer);
                    }
                    success = success &&
                              splitInt32(reinterpret_cast<const int32_t*>(input.buffer),
                                         input.shape(), axis, &outputDataPtrs, outputShapes);
                } break;
                case OperandType::TENSOR_QUANT8_ASYMM: {
                    std::vector<uint8_t*> outputDataPtrs(numOutputs);
                    for (int i = 0; i < numOutputs; ++i) {
                        outputDataPtrs[i] = reinterpret_cast<uint8_t*>(mOperands[outs[i]].buffer);
                    }
                    success = success &&
                              splitQuant8(reinterpret_cast<const uint8_t*>(input.buffer),
                                          input.shape(), axis, &outputDataPtrs, outputShapes);
                } break;
                default: {
                    return ANEURALNETWORKS_BAD_DATA;
                }
            }
        } break;
        case OperationType::MAXIMUM:
        case OperationType::MINIMUM: {
            if (!allParametersPresent(2, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& in1 = mOperands[ins[0]];
            const RunTimeOperandInfo& in2 = mOperands[ins[1]];

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outputShape = output.shape();

            const bool isMinimum = operation.type == OperationType::MINIMUM;
            success = maximum_minimum::prepare(in1.shape(), in2.shape(), &outputShape) &&
                      setInfoAndAllocateIfNeeded(&output, outputShape, &result) &&
                      maximum_minimum::eval(in1.buffer, in1.shape(), in2.buffer, in2.shape(),
                                            isMinimum, output.buffer, outputShape);
        } break;
        case OperationType::GROUPED_CONV_2D: {
            const size_t inCount = ins.size();
            if ((inCount != 12 && inCount != 9) || !allParametersPresent(inCount, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            const RunTimeOperandInfo& filter = mOperands[ins[1]];
            const RunTimeOperandInfo& bias = mOperands[ins[2]];

            int32_t padding_left, padding_right;
            int32_t padding_top, padding_bottom;
            int32_t padding_implicit = 0;
            int32_t stride_width, stride_height;
            int32_t numGroups;
            int32_t activation;
            bool data_layout = false;

            if (inCount == 12) {
                padding_left = getScalarData<int32_t>(mOperands[ins[3]]);
                padding_right = getScalarData<int32_t>(mOperands[ins[4]]);
                padding_top = getScalarData<int32_t>(mOperands[ins[5]]);
                padding_bottom = getScalarData<int32_t>(mOperands[ins[6]]);
                stride_width = getScalarData<int32_t>(mOperands[ins[7]]);
                stride_height = getScalarData<int32_t>(mOperands[ins[8]]);
                numGroups = getScalarData<int32_t>(mOperands[ins[9]]);
                activation = getScalarData<int32_t>(mOperands[ins[10]]);
                data_layout = getScalarData<bool>(mOperands[ins[11]]);
            } else {
                padding_implicit = getScalarData<int32_t>(mOperands[ins[3]]);
                stride_width = getScalarData<int32_t>(mOperands[ins[4]]);
                stride_height = getScalarData<int32_t>(mOperands[ins[5]]);
                numGroups = getScalarData<int32_t>(mOperands[ins[6]]);
                activation = getScalarData<int32_t>(mOperands[ins[7]]);
                data_layout = getScalarData<bool>(mOperands[ins[8]]);
            }

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            RunTimeOperandInfo input_tmp, output_tmp;
            std::unique_ptr<uint8_t[]> input_tmp_guard, output_tmp_guard;
            if (!convertToNhwc(input_tmp, input, input_tmp_guard, data_layout)) {
                success = false;
                break;
            }
            output_tmp.lifetime = OperandLifeTime::TEMPORARY_VARIABLE;
            output_tmp.buffer = data_layout ? nullptr : output.buffer;
            output_tmp.length = data_layout ? 0 : output.length;

            if (inCount == 9) {
                Shape inputShape = input_tmp.shape();
                Shape filterShape = filter.shape();
                int32_t input_width = getSizeOfDimension(inputShape, 2);
                int32_t input_height = getSizeOfDimension(inputShape, 1);
                int32_t filter_width = getSizeOfDimension(filterShape, 2);
                int32_t filter_height = getSizeOfDimension(filterShape, 1);
                calculateExplicitPadding(input_width, stride_width, filter_width, padding_implicit,
                                         &padding_left, &padding_right);
                calculateExplicitPadding(input_height, stride_height, filter_height,
                                         padding_implicit, &padding_top, &padding_bottom);
            }

            if (!groupedConvPrepare(input_tmp.shape(), filter.shape(), bias.shape(), padding_left,
                                    padding_right, padding_top, padding_bottom, stride_width,
                                    stride_height, numGroups, &outShape) ||
                !setInfoAndAllocateIfNeeded(&output_tmp, outShape, &result)) {
                if (!data_layout) output.dimensions = output_tmp.dimensions;
                success = false;
                break;
            }

            if (input_tmp.type == OperandType::TENSOR_FLOAT32) {
                success = groupedConvFloat32(
                        reinterpret_cast<const float*>(input_tmp.buffer), input_tmp.shape(),
                        reinterpret_cast<const float*>(filter.buffer), filter.shape(),
                        reinterpret_cast<const float*>(bias.buffer), bias.shape(), padding_left,
                        padding_right, padding_top, padding_bottom, stride_width, stride_height,
                        numGroups, activation, reinterpret_cast<float*>(output_tmp.buffer),
                        outShape);
            } else if (input_tmp.type == OperandType::TENSOR_FLOAT16) {
                success = groupedConvFloat16(
                        reinterpret_cast<const _Float16*>(input_tmp.buffer), input_tmp.shape(),
                        reinterpret_cast<const _Float16*>(filter.buffer), filter.shape(),
                        reinterpret_cast<const _Float16*>(bias.buffer), bias.shape(), padding_left,
                        padding_right, padding_top, padding_bottom, stride_width, stride_height,
                        numGroups, activation, reinterpret_cast<_Float16*>(output_tmp.buffer),
                        outShape);
            } else if (input_tmp.type == OperandType::TENSOR_QUANT8_ASYMM) {
                if (filter.type == OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL) {
                    success = groupedConvQuant8PerChannel(
                            reinterpret_cast<const uint8_t*>(input_tmp.buffer), input_tmp.shape(),
                            reinterpret_cast<const int8_t*>(filter.buffer), filter.shape(),
                            filter.extraParams.channelQuant().scales.data(),
                            reinterpret_cast<const int32_t*>(bias.buffer), bias.shape(),
                            padding_left, padding_right, padding_top, padding_bottom, stride_width,
                            stride_height, numGroups, activation,
                            reinterpret_cast<uint8_t*>(output_tmp.buffer), outShape);
                } else if (filter.type == OperandType::TENSOR_QUANT8_ASYMM) {
                    success = groupedConvQuant8(
                            reinterpret_cast<const uint8_t*>(input_tmp.buffer), input_tmp.shape(),
                            reinterpret_cast<const uint8_t*>(filter.buffer), filter.shape(),
                            reinterpret_cast<const int32_t*>(bias.buffer), bias.shape(),
                            padding_left, padding_right, padding_top, padding_bottom, stride_width,
                            stride_height, numGroups, activation,
                            reinterpret_cast<uint8_t*>(output_tmp.buffer), outShape);
                }
            }

            if (data_layout) {
                output_tmp_guard.reset(output_tmp.buffer);
            }
            if (!success || !convertFromNhwc(output, output_tmp, data_layout, &result)) {
                success = false;
                break;
            }
        } break;
        case OperationType::TILE: {
            if (!allParametersPresent(2, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            const RunTimeOperandInfo& multiples = mOperands[ins[1]];

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            success =
                    tile::prepare(input.shape(), reinterpret_cast<const int32_t*>(multiples.buffer),
                                  multiples.shape(), &outShape) &&
                    setInfoAndAllocateIfNeeded(&output, outShape, &result) &&
                    tile::eval(input.buffer, input.shape(),
                               reinterpret_cast<const int32_t*>(multiples.buffer), output.buffer,
                               outShape);
        } break;
        case OperationType::QUANTIZED_16BIT_LSTM: {
            if (!allParametersPresent(15, 2)) {
                return ANEURALNETWORKS_BAD_DATA;
            }

            RunTimeOperandInfo& cellStateOut =
                    mOperands[outs[QuantizedLSTMCell::kCellStateOutTensor]];
            RunTimeOperandInfo& output = mOperands[outs[QuantizedLSTMCell::kOutputTensor]];

            Shape cellStateOutShape, outputShape;
            QuantizedLSTMCell quantizedLSTMCell(operation, mOperands);

            success = QuantizedLSTMCell::prepare(operation, mOperands, &cellStateOutShape,
                                                 &outputShape) &&
                      setInfoAndAllocateIfNeeded(&cellStateOut, cellStateOutShape, &result) &&
                      setInfoAndAllocateIfNeeded(&output, outputShape, &result) &&
                      quantizedLSTMCell.eval();
        } break;
        case OperationType::POW: {
            if (!allParametersPresent(2, 1)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& base = mOperands[ins[0]];
            const RunTimeOperandInfo& exponent = mOperands[ins[1]];

            RunTimeOperandInfo& output = mOperands[outs[0]];
            Shape outShape = output.shape();

            success = pow::prepare(base.shape(), exponent.shape(), &outShape) &&
                      setInfoAndAllocateIfNeeded(&output, outShape, &result) &&
                      pow::eval(base.buffer, base.shape(), exponent.buffer, exponent.shape(),
                                output.buffer, outShape);
        } break;
        case OperationType::TOPK_V2: {
            if (!allParametersPresent(2, 2)) {
                return ANEURALNETWORKS_BAD_DATA;
            }
            const RunTimeOperandInfo& input = mOperands[ins[0]];
            int32_t k = getScalarData<int32_t>(mOperands[ins[1]]);

            RunTimeOperandInfo& values = mOperands[outs[0]];
            Shape valuesShape = values.shape();
            RunTimeOperandInfo& indices = mOperands[outs[1]];
            Shape indicesShape = indices.shape();

            success = topk_v2::prepare(input.shape(), k, &valuesShape, &indicesShape) &&
                      setInfoAndAllocateIfNeeded(&values, valuesShape, &result) &&
                      setInfoAndAllocateIfNeeded(&indices, indicesShape, &result) &&
                      topk_v2::eval(input.buffer, input.shape(), k, values.buffer, valuesShape,
                                    indices.buffer, indicesShape);
        } break;
        default: {
            const OperationRegistration* operationRegistration =
                    mOperationResolver->findOperation(operation.type);
            if (operationRegistration == nullptr) {
                LOG(ERROR) << getOperationName(operation.type) << " not registered";
            } else if (operationRegistration->prepare == nullptr ||
                       operationRegistration->execute == nullptr) {
                LOG(ERROR) << "Incomplete operation registration: "
                           << getOperationName(operation.type);
            } else {
                OperationExecutionContext context(&operation, mOperands.data());
                success = operationRegistration->flags.allowOmittedOperand ||
                          context.checkNoOmittedOperand();
                success = success && (operationRegistration->flags.allowZeroSizedInput ||
                                      context.checkNoZeroSizedInput());
                success = success && operationRegistration->prepare(&context) &&
                          operationRegistration->execute(&context);
                result = context.getResultCode();
            }
        }
    }
    if (!success && result == ANEURALNETWORKS_NO_ERROR) {
        result = ANEURALNETWORKS_OP_FAILED;
    }
    if (result != ANEURALNETWORKS_NO_ERROR) {
        LOG(ERROR) << getOperationName(operation.type) << " failed.";
        return result;
    }

    freeNoLongerUsedOperands(ins);
    return ANEURALNETWORKS_NO_ERROR;
}

void CpuExecutor::finish(int result) {
    // Free allocated temporary operands.
    for (auto& info : mOperands) {
        if (info.lifetime == OperandLifeTime::TEMPORARY_VARIABLE && info.buffer != nullptr) {
            delete[] info.buffer;
            info.buffer = nullptr;
        }
    }

    // Only report the output shapes when the result code is NO_ERROR or
    // OUTPUT_INSUFFICIENT_SIZE.
    if (result == ANEURALNETWORKS_NO_ERROR || result == ANEURALNETWORKS_OUTPUT_INSUFFICIENT_SIZE) {
        const auto& outputs = mModel->outputIndexes;
        mOutputShapes.resize(outputs.size());
        for (uint32_t i = 0; i < outputs.size(); i++) {
            const uint32_t operandIndex = outputs[i];
            RunTimeOperandInfo& from = mOperands[operandIndex];
            mOutputShapes[i].dimensions = from.dimensions;
            mOutputShapes[i].isSufficient = from.isSufficient();
        }
    } else {
        mOutputShapes.clear();
    }

    mModel = nullptr;
    mRequest = nullptr;
    mFinished = true;
}

// b/109953668, disable OpenMP
#ifdef NNAPI_OPENMP
ScopedOpenmpSettings::ScopedOpenmpSettings() {
    mBlocktimeInitial = kmp_get_blocktime();
    kmp_set_blocktime(20);  // ms, see b/109645291

#if NNAPI_LIMIT_CPU_THREADS
    // Code not yet enabled. Choosing the number of threads to be based on
    // benchmarking. See longer comment by the class declaration.
    mMaxThreadsInitial = Eigen::nbThreads();
    const int nProcs = omp_get_num_procs();
    int threads = nProcs;
    if (nProcs >= 8) {
        threads = nProcs - 4;
    } else if (nProcs >= 4) {
        threads = nProcs - 2;
    }
    Eigen::setNbThreads(threads);
#endif
}

ScopedOpenmpSettings::~ScopedOpenmpSettings() {
    kmp_set_blocktime(mBlocktimeInitial);
#if NNAPI_LIMIT_CPU_THREADS
    Eigen::setNbThreads(mMaxThreadsInitial);
#endif
}
#endif  // NNAPI_OPENMP

}  // namespace nn
}  // namespace android
