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

#define LOG_TAG "BurstBuilder"

#include "BurstBuilder.h"

#include "CompilationBuilder.h"
#include "ExecutionBurstController.h"

namespace android {
namespace nn {

BurstBuilder::BurstBuilder(const CompilationBuilder* compilation,
                           std::vector<std::shared_ptr<ExecutionBurstController>> burstControllers)
    : mCompilation(compilation), mBurstControllers(std::move(burstControllers)) {}

bool BurstBuilder::tryLock() {
    const bool alreadyRunning = mCurrentlyRunning.test_and_set();
    return !alreadyRunning;
}

void BurstBuilder::unlock() {
    mCurrentlyRunning.clear();
}

const CompilationBuilder* BurstBuilder::getCompilation() const {
    return mCompilation;
}

std::shared_ptr<ExecutionBurstController> BurstBuilder::getControllerAt(size_t index) const {
    return index < mBurstControllers.size() ? mBurstControllers[index] : nullptr;
}

}  // namespace nn
}  // namespace android
