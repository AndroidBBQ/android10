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

#ifndef RS2SPIRV_CONTEXT_H
#define RS2SPIRV_CONTEXT_H

#include "RSAllocationUtils.h"
#include "bcinfo/MetadataExtractor.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"

#include <stdint.h>

// Declare a friend relationship in a class with a test. Used rather that
// FRIEND_TEST to avoid globally importing gtest/gtest.h into the main
// RSoV header files.
#ifdef __HOST__
#define RSOV_FRIEND_TEST(test_set_name, individual_test)                       \
  friend class test_set_name##_##individual_test##_Test
#else
#define RSOV_FRIEND_TEST(test_set_name, individual_test)
#endif // __HOST__

namespace bcinfo {
class MetadataExtractor;
}

namespace llvm {
class Module;
}

namespace rs2spirv {

// A singleton that keeps state during the compilation from RS LLVM bitcode to
// SPIR-V, which provides quick lookup of metadata and shares information
// between the passes.
class Context {
  RSOV_FRIEND_TEST(ContextTest, testInitialize);

public:
  static Context &getInstance();

  Context();

  // Initialize the internal data struture such as the slot number lookup table,
  // etc.
  bool Initialize(std::unique_ptr<bcinfo::MetadataExtractor> ME);

  // Returns the total number of exported variables
  uint32_t getNumExportVar() const { return mExportVarIndices.size(); }

  // Adds the mapping from the slot number of an exported variable to the index
  // of its field in the global buffer
  void addExportVarIndex(uint32_t slot, uint32_t index) {
    mExportVarIndices[slot] = index;
  }

  // Adds the mapping from the name of an exported variable to the index of its
  // field in the global buffer
  void addExportVarIndex(const char *varName, uint32_t index);

  // Given the slot number of an exported variable, returns the index of its
  // field in the global buffer
  uint32_t getExportVarIndex(uint32_t slot) const {
    return mExportVarIndices[slot];
  }

  void setGlobalSize(uint64_t size) {
    mGlobalSize = size;
  }

  uint64_t getGlobalSize() const {
    return mGlobalSize;
  }

  // Returns the total number of foreach kernels
  uint32_t getNumForEachKernel() const { return mForEachNameToSlot.size(); }

  // Checks if a name refers to a foreach kernel function
  bool isForEachKernel(llvm::StringRef name) const {
    return mForEachNameToSlot.count(name) != 0;
  }

  const bcinfo::MetadataExtractor &getMetadata() const { return *mMetadata; }

  llvm::SmallVectorImpl<RSAllocationInfo> &getGlobalAllocs() {
    return mGlobalAllocs;
  }

private:
  uint32_t getSlotForExportVar(const char *varName) {
    const llvm::StringRef strVarName(varName);
    auto it = mVarNameToSlot.find(strVarName);
    if (it == mVarNameToSlot.end()) {
      return std::numeric_limits<uint32_t>::max();
    }
    return it->second;
  }

  bool mInitialized;
  // RenderScript metadata embedded in the input LLVM Moduel
  std::unique_ptr<bcinfo::MetadataExtractor> mMetadata;
  // A map from exported variable names to their slot numbers
  llvm::StringMap<uint32_t> mVarNameToSlot;
  // The size of memory needed to store all global variables (static variables)
  uint64_t mGlobalSize;
  // A map from exported foreach kernel names to their slot numbers
  llvm::StringMap<uint32_t> mForEachNameToSlot;
  // These are the indices for each exported variable in the global buffer
  llvm::SmallVector<uint32_t, 8> mExportVarIndices;
  // For Global Allocations; carries global variable -> metadata offset
  // mapping from an LLVM pass to a SPIRIT pass
  llvm::SmallVector<RSAllocationInfo, 8> mGlobalAllocs;
};

} // namespace rs2spirv

#endif // RS2SPIRV_CONTEXT_H
