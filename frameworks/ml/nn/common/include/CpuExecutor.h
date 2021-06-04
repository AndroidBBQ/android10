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

#ifndef ANDROID_ML_NN_COMMON_CPU_EXECUTOR_H
#define ANDROID_ML_NN_COMMON_CPU_EXECUTOR_H

#include "HalInterfaces.h"
#include "OperationResolver.h"
#include "OperationsUtils.h"
#include "Utils.h"

#include <android-base/macros.h>
#include <ui/GraphicBuffer.h>
#include <algorithm>
#include <optional>
#include <vector>

namespace android {
namespace nn {

// Information we maintain about each operand during execution that
// may change during execution.
struct RunTimeOperandInfo {
    // TODO Storing the type here is redundant, as it won't change during execution.
    OperandType type;
    // The type and dimensions of the operand.  The dimensions can
    // change at runtime.  We include the type because it's useful
    // to pass together with the dimension to the functions implementing
    // the operators.
    //
    // A dimension being zero has different meanings for different operands at different stages:
    // - Model inputs:
    //   * Specified in model: implies "dynamic", and must be fully-specified in request.
    //   * Specified in request: illegal.
    // - Constant operands: illegal.
    // - Model outputs and internal operands:
    //   * Before evaluation: implies unknown and to be deduced from execution.
    //   * After evaluation:
    //     - If isSufficient reports true: the tensor is zero-sized.
    //     - Otherwise: implies unknown.
    std::vector<uint32_t> dimensions;

    float scale;
    int32_t zeroPoint;
    // Where the operand's data is stored.  Check the corresponding
    // location information in the model to figure out if this points
    // to memory we have allocated for an temporary operand.
    uint8_t* buffer;
    // The length of the buffer.
    uint32_t length;
    // Whether this is a temporary variable, a model input, a constant, etc.
    OperandLifeTime lifetime;
    // Keeps track of how many operations have yet to make use
    // of this temporary variable.  When the count is decremented to 0,
    // we free the buffer.  For non-temporary variables, this count is
    // always 0.
    uint32_t numberOfUsesLeft;

    Operand::ExtraParams extraParams;

    Shape shape() const {
        return {
                .type = type,
                .dimensions = dimensions,
                .scale = scale,
                .offset = zeroPoint,
                .extraParams = extraParams,
        };
    }

    bool isSufficient() const {
        if (isExtensionOperandType(type)) {
            // We don't know sizes of extension types.
            return true;
        }
        return length >= nonExtensionOperandSizeOfData(type, dimensions);
    }
};

// Used to keep a pointer to each of the memory pools.
//
// RunTimePoolInfo references a region of memory. Other RunTimePoolInfo objects
// may reference the same region of memory by either:
// (1) copying an existing RunTimePoolInfo object, or
// (2) creating multiple RunTimePoolInfo objects from the same memory resource
//     (e.g., "createFromHidlMemory" or "createFromExistingBuffer")
//
// If the underlying region of memory is mapped by "createFromHidlMemory", the
// mapping will be sustained until it is no longer referenced by any
// RunTimePoolInfo objects.
class RunTimePoolInfo {
   public:
    static std::optional<RunTimePoolInfo> createFromHidlMemory(const hidl_memory& hidlMemory);
    static RunTimePoolInfo createFromExistingBuffer(uint8_t* buffer);

    uint8_t* getBuffer() const;
    bool update() const;
    hidl_memory getHidlMemory() const;

   private:
    class RunTimePoolInfoImpl;
    RunTimePoolInfo(const std::shared_ptr<const RunTimePoolInfoImpl>& impl);

    std::shared_ptr<const RunTimePoolInfoImpl> mImpl;
};

bool setRunTimePoolInfosFromHidlMemories(std::vector<RunTimePoolInfo>* poolInfos,
                                         const hidl_vec<hidl_memory>& pools);

// This class is used to execute a model on the CPU.
class CpuExecutor {
   public:
    // This constructor allows clients of CpuExecutor to provide custom CPU
    // operation implementations. It is used by a sample driver to test
    // extension support.
    //
    // Note that it is not possible to provide custom CPU implementations for
    // non-OperationResolver operations (b/124041202).
    //
    // The operation resolver must outlive the executor.
    explicit CpuExecutor(const IOperationResolver* operationResolver)
        : mOperationResolver(operationResolver) {}

    CpuExecutor() : CpuExecutor(BuiltinOperationResolver::get()) {}

    // Executes the model. The results will be stored at the locations
    // specified in the constructor.
    // The model must outlive the executor.  We prevent it from being modified
    // while this is executing.
    int run(const Model& model, const Request& request,
            const std::vector<RunTimePoolInfo>& modelPoolInfos,
            const std::vector<RunTimePoolInfo>& requestPoolInfos);

    const std::vector<OutputShape>& getOutputShapes() const {
        CHECK(mFinished) << "getOutputShapes() called by an unfinished CpuExecutor.";
        return mOutputShapes;
    }

   private:
    bool initializeRunTimeInfo(const std::vector<RunTimePoolInfo>& modelPoolInfos,
                               const std::vector<RunTimePoolInfo>& requestPoolInfos);
    // Runs one operation of the graph.
    int executeOperation(const Operation& entry);
    // Decrement the usage count for the operands listed.  Frees the memory
    // allocated for any temporary variable with a count of zero.
    void freeNoLongerUsedOperands(const std::vector<uint32_t>& inputs);

    // Frees the memory allocated for any temporary variable, and sets the
    // output operand shapes returning to the runtime.
    void finish(int result);

    // The model and the request that we'll execute. Only valid while run()
    // is being executed.
    const Model* mModel = nullptr;
    const Request* mRequest = nullptr;

    // We're copying the list of all the dimensions from the model, as
    // these may be modified when we run the operations.  Since we're
    // making a full copy, the indexes used in the operand description
    // stay valid.
    //    std::vector<uint32_t> mDimensions;
    // Runtime information about all the operands.
    std::vector<RunTimeOperandInfo> mOperands;

    // The output operand shapes returning to the runtime.
    std::vector<OutputShape> mOutputShapes;

    // Whether execution is finished and mOutputShapes is ready
    bool mFinished = false;

    const IOperationResolver* mOperationResolver;
};

// Class for setting reasonable OpenMP threading settings. (OpenMP is used by
// the Eigen matrix library.)
//
// Currently sets a low blocktime: the time OpenMP threads busy-wait for more
// work before going to sleep. See b/79159165, https://reviews.llvm.org/D18577.
// The default is 200ms, we set to 20ms here, see b/109645291. This keeps the
// cores enabled throughout inference computation without too much extra power
// consumption afterwards.
//
// The OpenMP settings are thread-local (applying only to worker threads formed
// from that thread), see https://software.intel.com/en-us/node/522688 and
// http://lists.llvm.org/pipermail/openmp-dev/2016-July/001432.html. This class
// ensures that within the scope in which an object is instantiated we use the
// right settings (scopes may be nested), as long as no other library changes
// them.  (Note that in current NNAPI usage only one instance is used in the
// CpuExecutor thread).
//
// TODO(mikie): consider also setting the number of threads used. Using as many
// threads as there are cores results in more variable performance: if we don't
// get all cores for our threads, the latency is doubled as we wait for one core
// to do twice the amount of work. Reality is complicated though as not all
// cores are the same. Decision to be based on benchmarking against a
// representative set of workloads and devices. I'm keeping the code here for
// reference.
// b/109953668, disable OpenMP
#ifdef NNAPI_OPENMP
class ScopedOpenmpSettings {
public:
    ScopedOpenmpSettings();
    ~ScopedOpenmpSettings();
    DISALLOW_COPY_AND_ASSIGN(ScopedOpenmpSettings);
private:
    int mBlocktimeInitial;
#if NNAPI_LIMIT_CPU_THREADS
    int mMaxThreadsInitial;
#endif
};
#endif  // NNAPI_OPENMP


namespace {

template <typename T>
T getScalarData(const RunTimeOperandInfo& info) {
    // TODO: Check buffer is at least as long as size of data.
    T* data = reinterpret_cast<T*>(info.buffer);
    return data[0];
}

inline bool IsNullInput(const RunTimeOperandInfo *input) {
    return input->lifetime == OperandLifeTime::NO_VALUE;
}

inline int NumInputsWithValues(const Operation &operation,
                               std::vector<RunTimeOperandInfo> &operands) {
  const std::vector<uint32_t> &inputs = operation.inputs;
  return std::count_if(inputs.begin(), inputs.end(),
                       [&operands](uint32_t i) {
                         return !IsNullInput(&operands[i]);
                       });
}

inline int NumOutputs(const Operation &operation) {
  return operation.outputs.size();
}

inline size_t NumDimensions(const RunTimeOperandInfo *operand) {
  return operand->shape().dimensions.size();
}

inline uint32_t SizeOfDimension(const RunTimeOperandInfo *operand, int i) {
  return operand->shape().dimensions[i];
}

inline RunTimeOperandInfo *GetInput(const Operation &operation,
                                    std::vector<RunTimeOperandInfo> &operands,
                                    int index) {
  return &operands[operation.inputs[index]];
}

inline RunTimeOperandInfo *GetOutput(const Operation &operation,
                                     std::vector<RunTimeOperandInfo> &operands,
                                     int index) {
  return &operands[operation.outputs[index]];
}

}  // anonymous namespace

} // namespace nn
} // namespace android

#endif // ANDROID_ML_NN_COMMON_CPU_EXECUTOR_H
