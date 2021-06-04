/*
 * Copyright 2017, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Context.h"

#include "llvm/ADT/Triple.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <limits>

#define DEBUG_TYPE "rs2spirv-context"

namespace rs2spirv {

Context &Context::getInstance() {
  static Context c;
  return c;
}

Context::Context() : mInitialized(false) {}

bool Context::Initialize(std::unique_ptr<bcinfo::MetadataExtractor> ME) {
  if (mInitialized) {
    return true;
  }

  mMetadata = std::move(ME);

  if (!mMetadata->extract()) {
    llvm::errs() << "cannot extract metadata\n";
    return false;
  }

  const char **varNames = mMetadata->getExportVarNameList();
  size_t varCount = mMetadata->getExportVarCount();
  mExportVarIndices.resize(varCount);

  // Builds the lookup table from a variable name to its slot number
  for (size_t slot = 0; slot < varCount; slot++) {
    std::string varName(varNames[slot]);
    mVarNameToSlot.insert(std::make_pair(varName, (uint32_t)slot));
  }

  const size_t kernelCount = mMetadata->getExportForEachSignatureCount();
  const char **kernelNames = mMetadata->getExportForEachNameList();
  for (size_t slot = 0; slot < kernelCount; slot++) {
    mForEachNameToSlot.insert(std::make_pair(kernelNames[slot], slot));
  }

  mInitialized = true;

  return true;
}

void Context::addExportVarIndex(const char *varName, uint32_t index) {
  DEBUG(llvm::dbgs() << varName << " index=" << index << '\n');
  const uint32_t slot = getSlotForExportVar(varName);
  if (slot == std::numeric_limits<uint32_t>::max()) {
    return;
  }
  addExportVarIndex(slot, index);
}


} // namespace rs2spirv
