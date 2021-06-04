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

#ifndef ANDROID_ML_NN_RUNTIME_BURST_BUILDER_H
#define ANDROID_ML_NN_RUNTIME_BURST_BUILDER_H

#include <atomic>
#include <memory>
#include <vector>
#include "ExecutionBurstController.h"

namespace android {
namespace nn {

class CompilationBuilder;

/*
 * TODO: Could we "hide" the per-step burst controller instance inside
 * StepExecutor? Today it's exposed as a "sibling" to StepExecutor:
 * ExecutionPlan::next both generates a StepExecutor instance and finds a
 * pointer to a burst controller; and StepExecutor::startCompute is passed a
 * pointer to a burst controller. Instead, could ExecutionPlan::next stash the
 * burst controller in the StepExecutor, so that it doesn't have to be passed
 * to any of the StepExecutor methods?
 */

class BurstBuilder {
   public:
    BurstBuilder(const CompilationBuilder* compilation,
                 std::vector<std::shared_ptr<ExecutionBurstController>> burstControllers);

    bool tryLock();
    void unlock();

    const CompilationBuilder* getCompilation() const;
    std::shared_ptr<ExecutionBurstController> getControllerAt(size_t index) const;

   private:
    std::atomic_flag mCurrentlyRunning = ATOMIC_FLAG_INIT;
    const CompilationBuilder* mCompilation;
    std::vector<std::shared_ptr<ExecutionBurstController>> mBurstControllers;
};

}  // namespace nn
}  // namespace android

#endif  // ANDROID_ML_NN_RUNTIME_BURST_BUILDER_H
