/*
 * Copyright 2016, The Android Open Source Project
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

#ifndef RS2SPIRV_RS_ALLOCATION_UTILS_H
#define RS2SPIRV_RS_ALLOCATION_UTILS_H

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"

#include <string>

namespace llvm {
class CallInst;
class GlobalVariable;
class Module;
class Type;
} // namespace llvm

namespace rs2spirv {

struct RSAllocationInfo {
  std::string VarName;
  llvm::Optional<std::string> RSElementType;
  llvm::GlobalVariable *GlobalVar;
  // Assigned unique identifier for this allocation;
  // not the slot #.
  // Represents the index of this allocation's metadata
  // in the global allocation metadata Vulkan buffer
  int ID;
  bool hasID(void) const { return ID >= 0; }
  void assignID(int no) { ID = no; }
};

enum class RSAllocAccessKind { GEA, SEA, DIMX };

struct RSAllocationCallInfo {
  RSAllocationInfo &RSAlloc;
  llvm::CallInst *FCall;
  RSAllocAccessKind Kind;
  std::string RSElementTy;
};

bool isRSAllocation(const llvm::GlobalVariable &GV);
bool getRSAllocationInfo(llvm::Module &M,
                         llvm::SmallVectorImpl<RSAllocationInfo> &Allocs);
bool getRSAllocAccesses(llvm::SmallVectorImpl<RSAllocationInfo> &Allocs,
                        llvm::SmallVectorImpl<RSAllocationCallInfo> &Calls);
bool solidifyRSAllocAccess(llvm::Module &M, RSAllocationCallInfo CallInfo);

} // namespace rs2spirv

#endif
