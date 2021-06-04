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

#include "RSAllocationUtils.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include "cxxabi.h"

#include <sstream>
#include <unordered_map>

#define DEBUG_TYPE "rs2spirv-rs-allocation-utils"

using namespace llvm;

namespace rs2spirv {

bool isRSAllocation(const GlobalVariable &GV) {
  auto *PT = cast<PointerType>(GV.getType());
  DEBUG(PT->dump());

  auto *VT = PT->getElementType();
  DEBUG(VT->dump());
  std::string TypeName;
  raw_string_ostream RSO(TypeName);
  VT->print(RSO);
  RSO.str(); // Force flush.
  DEBUG(dbgs() << "TypeName: " << TypeName << '\n');

  return TypeName.find("struct.rs_allocation") != std::string::npos;
}

bool getRSAllocationInfo(Module &M, SmallVectorImpl<RSAllocationInfo> &Allocs) {
  DEBUG(dbgs() << "getRSAllocationInfo\n");
  for (auto &GV : M.globals()) {
    if (GV.isDeclaration() || !isRSAllocation(GV))
      continue;

    Allocs.push_back({'%' + GV.getName().str(), None, &GV, -1});
  }

  return true;
}

// Collect Allocation access calls into the Calls
// Also update Allocs with assigned ID.
// After calling this function, Allocs would contain the mapping from
// GV name to the corresponding ID.
bool getRSAllocAccesses(SmallVectorImpl<RSAllocationInfo> &Allocs,
                        SmallVectorImpl<RSAllocationCallInfo> &Calls) {
  DEBUG(dbgs() << "getRSGEATCalls\n");
  DEBUG(dbgs() << "\n\n~~~~~~~~~~~~~~~~~~~~~\n\n");

  std::unordered_map<const Value *, const GlobalVariable *> Mapping;
  int id_assigned = 0;

  for (auto &A : Allocs) {
    auto *GV = A.GlobalVar;
    std::vector<User *> WorkList(GV->user_begin(), GV->user_end());
    size_t Idx = 0;

    while (Idx < WorkList.size()) {
      auto *U = WorkList[Idx];
      DEBUG(dbgs() << "Visiting ");
      DEBUG(U->dump());
      ++Idx;
      auto It = Mapping.find(U);
      if (It != Mapping.end()) {
        if (It->second == GV) {
          continue;
        } else {
          errs() << "Duplicate global mapping discovered!\n";
          errs() << "\nGlobal: ";
          GV->print(errs());
          errs() << "\nExisting mapping: ";
          It->second->print(errs());
          errs() << "\nUser: ";
          U->print(errs());
          errs() << '\n';

          return false;
        }
      }

      Mapping[U] = GV;
      DEBUG(dbgs() << "New mapping: ");
      DEBUG(U->print(dbgs()));
      DEBUG(dbgs() << " -> " << GV->getName() << '\n');

      if (auto *FCall = dyn_cast<CallInst>(U)) {
        if (auto *F = FCall->getCalledFunction()) {
          const auto FName = F->getName();
          DEBUG(dbgs() << "Discovered function call to : " << FName << '\n');
          // Treat memcpy as moves for the purpose of this analysis
          if (FName.startswith("llvm.memcpy")) {
            assert(FCall->getNumArgOperands() > 0);
            Value *CopyDest = FCall->getArgOperand(0);
            // We are interested in the users of the dest operand of
            // memcpy here
            Value *LocalCopy = CopyDest->stripPointerCasts();
            User *NewU = dyn_cast<User>(LocalCopy);
            assert(NewU);
            WorkList.push_back(NewU);
            continue;
          }

          char *demangled = __cxxabiv1::__cxa_demangle(
              FName.str().c_str(), nullptr, nullptr, nullptr);
          if (!demangled)
            continue;
          const StringRef DemangledNameRef(demangled);
          DEBUG(dbgs() << "Demangled name: " << DemangledNameRef << '\n');

          const StringRef GEAPrefix = "rsGetElementAt_";
          const StringRef SEAPrefix = "rsSetElementAt_";
          const StringRef DIMXPrefix = "rsAllocationGetDimX";
          assert(GEAPrefix.size() == SEAPrefix.size());

          const bool IsGEA = DemangledNameRef.startswith(GEAPrefix);
          const bool IsSEA = DemangledNameRef.startswith(SEAPrefix);
          const bool IsDIMX = DemangledNameRef.startswith(DIMXPrefix);

          assert(IsGEA || IsSEA || IsDIMX);
          if (!A.hasID()) {
            A.assignID(id_assigned++);
          }

          if (IsGEA || IsSEA) {
            DEBUG(dbgs() << "Found rsAlloc function!\n");

            const auto Kind =
                IsGEA ? RSAllocAccessKind::GEA : RSAllocAccessKind::SEA;

            const auto RSElementTy =
                DemangledNameRef.drop_front(GEAPrefix.size());

            Calls.push_back({A, FCall, Kind, RSElementTy.str()});
            continue;
          } else if (DemangledNameRef.startswith(GEAPrefix.drop_back()) ||
                     DemangledNameRef.startswith(SEAPrefix.drop_back())) {
            errs() << "Untyped accesses to global rs_allocations are not "
                      "supported.\n";
            return false;
          } else if (IsDIMX) {
            DEBUG(dbgs() << "Found rsAllocationGetDimX function!\n");
            const auto Kind = RSAllocAccessKind::DIMX;
            Calls.push_back({A, FCall, Kind, ""});
          }
        }
      }

      // TODO: Consider using set-like container to reduce computational
      // complexity.
      for (auto *NewU : U->users())
        if (std::find(WorkList.begin(), WorkList.end(), NewU) == WorkList.end())
          WorkList.push_back(NewU);
    }
  }

  std::unordered_map<const GlobalVariable *, std::string> GVAccessTypes;

  for (auto &Access : Calls) {
    auto AccessElemTyIt = GVAccessTypes.find(Access.RSAlloc.GlobalVar);
    if (AccessElemTyIt != GVAccessTypes.end() &&
        AccessElemTyIt->second != Access.RSElementTy) {
      errs() << "Could not infere element type for: ";
      Access.RSAlloc.GlobalVar->print(errs());
      errs() << '\n';
      return false;
    } else if (AccessElemTyIt == GVAccessTypes.end()) {
      GVAccessTypes.emplace(Access.RSAlloc.GlobalVar, Access.RSElementTy);
      Access.RSAlloc.RSElementType = Access.RSElementTy;
    }
  }

  DEBUG(dbgs() << "\n\n~~~~~~~~~~~~~~~~~~~~~\n\n");
  return true;
}

bool solidifyRSAllocAccess(Module &M, RSAllocationCallInfo CallInfo) {
  DEBUG(dbgs() << "solidifyRSAllocAccess " << CallInfo.RSAlloc.VarName << '\n');
  auto *FCall = CallInfo.FCall;
  auto *Fun = FCall->getCalledFunction();
  assert(Fun);

  StringRef FName;
  if (CallInfo.Kind == RSAllocAccessKind::DIMX)
    FName = "rsAllocationGetDimX";
  else
    FName = Fun->getName();

  std::ostringstream OSS;
  OSS << "__rsov_" << FName.str();
  // Make up uint32_t F(uint32_t)
  Type *UInt32Ty = IntegerType::get(M.getContext(), 32);
  auto *NewFT = FunctionType::get(UInt32Ty, ArrayRef<Type *>(UInt32Ty), false);

  auto *NewF = Function::Create(NewFT, // Fun->getFunctionType(),
                                Function::ExternalLinkage, OSS.str(), &M);
  FCall->setCalledFunction(NewF);
  FCall->setArgOperand(0, ConstantInt::get(UInt32Ty, 0, false));
  NewF->setAttributes(Fun->getAttributes());

  DEBUG(M.dump());

  return true;
}

} // namespace rs2spirv
