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

#include <algorithm>
#include <iostream>
#include <string>

#include "clang/AST/APValue.h"

#include "slang_assert.h"
#include "slang_rs_export_foreach.h"
#include "slang_rs_export_func.h"
#include "slang_rs_export_reduce.h"
#include "slang_rs_export_type.h"
#include "slang_rs_export_var.h"
#include "slang_rs_reflection.h"
#include "slang_rs_reflection_state.h"

#include "bcinfo/MetadataExtractor.h"

namespace slang {

static bool equal(const clang::APValue &a, const clang::APValue &b) {
  if (a.getKind() != b.getKind())
    return false;
  switch (a.getKind()) {
    case clang::APValue::Float:
      return a.getFloat().bitwiseIsEqual(b.getFloat());
    case clang::APValue::Int:
      return a.getInt() == b.getInt();
    case clang::APValue::Vector: {
      unsigned NumElements = a.getVectorLength();
      if (NumElements != b.getVectorLength())
        return false;
      for (unsigned i = 0; i < NumElements; ++i) {
        if (!equal(a.getVectorElt(i), b.getVectorElt(i)))
          return false;
      }
      return true;
    }
    default:
      slangAssert(false && "unexpected APValue kind");
      return false;
  }
}

ReflectionState::~ReflectionState() {
  slangAssert(mState==S_Initial || mState==S_ClosedJava64 || mState==S_Bad);
  delete mStringSet;
}

void ReflectionState::openJava32(size_t NumFiles) {
  if (kDisabled)
    return;
  slangAssert(mState==S_Initial);
  mState = S_OpenJava32;
  mStringSet = new llvm::StringSet<>;
  mFiles.BeginCollecting(NumFiles);
}

void ReflectionState::closeJava32() {
  if (kDisabled)
    return;
  slangAssert(mState==S_OpenJava32 && (mForEachOpen < 0) && !mOutputClassOpen && (mRecordsState != RS_Open));
  mState = S_ClosedJava32;
  mRSC = nullptr;
}

void ReflectionState::openJava64() {
  if (kDisabled)
    return;
  slangAssert(mState==S_ClosedJava32);
  mState = S_OpenJava64;
  mFiles.BeginUsing();
}

void ReflectionState::closeJava64() {
  if (kDisabled)
    return;
  slangAssert(mState==S_OpenJava64 && (mForEachOpen < 0) && !mOutputClassOpen && (mRecordsState != RS_Open));
  mState = S_ClosedJava64;
  mRSC = nullptr;
}

llvm::StringRef ReflectionState::canon(const std::string &String) {
  slangAssert(isCollecting());
  // NOTE: llvm::StringSet does not permit the empty string as a member
  return String.empty() ? llvm::StringRef() : mStringSet->insert(String).first->getKey();
}

std::string ReflectionState::getUniqueTypeName(const RSExportType *T) {
  return RSReflectionJava::GetTypeName(T, RSReflectionJava::TypeNamePseudoC);
}

void ReflectionState::nextFile(const RSContext *RSC,
                               const std::string &PackageName,
                               const std::string &RSSourceFileName) {
  slangAssert(!isClosed());
  if (!isActive())
    return;

  mRSC = RSC;

  slangAssert(mRecordsState != RS_Open);
  mRecordsState = RS_Initial;

  if (isCollecting()) {
    File &file = mFiles.CollectNext();
    file.mPackageName = PackageName;
    file.mRSSourceFileName = RSSourceFileName;
  }
  if (isUsing()) {
    File &file = mFiles.UseNext();
    slangAssert(file.mRSSourceFileName == RSSourceFileName);
    if (file.mPackageName != PackageName)
      mRSC->ReportError("in file '%0' Java package name is '%1' for 32-bit targets "
                        "but '%2' for 64-bit targets")
          << RSSourceFileName << file.mPackageName << PackageName;
  }
}

void ReflectionState::dump() {
  const size_t NumFiles = mFiles.Size();
  for (int i = 0; i < NumFiles; ++i) {
    const File &file = mFiles[i];
    std::cout << "file = \"" << file.mRSSourceFileName << "\", "
              << "package = \"" << file.mPackageName << "\"" << std::endl;

    // NOTE: "StringMap iteration order, however, is not guaranteed to
    // be deterministic".  So sort before dumping.
    typedef const llvm::StringMap<File::Record>::MapEntryTy *RecordsEntryTy;
    std::vector<RecordsEntryTy> Records;
    Records.reserve(file.mRecords.size());
    for (auto I = file.mRecords.begin(), E = file.mRecords.end(); I != E; I++)
      Records.push_back(&(*I));
    std::sort(Records.begin(), Records.end(),
              [](RecordsEntryTy a, RecordsEntryTy b) { return a->getKey().compare(b->getKey())==-1; });
    for (auto Record : Records) {
      const auto &Val = Record->getValue();
      std::cout << "  (Record) name=\"" << Record->getKey().str() << "\""
                << " allocSize=" << Val.mAllocSize
                << " postPadding=" << Val.mPostPadding
                << " ordinary=" << Val.mOrdinary
                << " matchedByName=" << Val.mMatchedByName
                << std::endl;
      const size_t NumFields = Val.mFieldCount;
      for (int fieldIdx = 0; fieldIdx < NumFields; ++fieldIdx) {
        const auto &field = Val.mFields[fieldIdx];
        std::cout << "    (Field) name=\"" << field.mName << "\" ("
                  << field.mPrePadding << ", \"" << field.mType.str()
                  << "\"(" << field.mStoreSize << ")@" << field.mOffset
                  << ", " << field.mPostPadding << ")" << std::endl;
      }
    }

    const size_t NumVars = file.mVariables.Size();
    for (int varIdx = 0; varIdx < NumVars; ++varIdx) {
      const auto &var = file.mVariables[varIdx];
      std::cout << "  (Var) name=\"" << var.mName << "\" type=\"" << var.mType.str()
                << "\" const=" << var.mIsConst << " initialized=" << (var.mInitializerCount != 0)
                << " allocSize=" << var.mAllocSize << std::endl;
    }

    for (int feIdx = 0; feIdx < file.mForEachCount; ++feIdx) {
      const auto &fe = file.mForEaches[feIdx];
      std::cout << "  (ForEach) ordinal=" << feIdx << " state=";
      switch (fe.mState) {
        case File::ForEach::S_Initial:
          std::cout << "initial" << std::endl;
          continue;
        case File::ForEach::S_Collected:
          std::cout << "collected";
          break;
        case File::ForEach::S_UseMatched:
          std::cout << "usematched";
          break;
        default:
          std::cout << fe.mState;
          break;
      }
      std::cout << " name=\"" << fe.mName << "\" kernel=" << fe.mIsKernel
                << " hasOut=" << fe.mHasOut << " out=\"" << fe.mOut.str()
                << "\" metadata=0x" << std::hex << fe.mSignatureMetadata << std::dec
                << std::endl;
      const size_t NumIns = fe.mIns.Size();
      for (int insIdx = 0; insIdx < NumIns; ++insIdx)
        std::cout << "    (In) " << fe.mIns[insIdx].str() << std::endl;
      const size_t NumParams = fe.mParams.Size();
      for (int paramsIdx = 0; paramsIdx < NumParams; ++paramsIdx)
        std::cout << "    (Param) " << fe.mParams[paramsIdx].str() << std::endl;
    }

    for (auto feBad : mForEachesBad) {
      std::cout << "  (ForEachBad) ordinal=" << feBad->getOrdinal()
                << " name=\"" << feBad->getName() << "\""
                << std::endl;
    }

    const size_t NumInvokables = file.mInvokables.Size();
    for (int invIdx = 0; invIdx < NumInvokables; ++invIdx) {
      const auto &inv = file.mInvokables[invIdx];
      std::cout << "  (Invokable) name=\"" << inv.mName << "\"" << std::endl;
      const size_t NumParams = inv.mParamCount;
      for (int paramsIdx = 0; paramsIdx < NumParams; ++paramsIdx)
        std::cout << "    (Param) " << inv.mParams[paramsIdx].str() << std::endl;
    }

    const size_t NumReduces = file.mReduces.Size();
    for (int redIdx = 0; redIdx < NumReduces; ++redIdx) {
      const auto &red = file.mReduces[redIdx];
      std::cout << "  (Reduce) name=\"" << red.mName
                << "\" result=\"" << red.mResult.str()
                << "\" exportable=" << red.mIsExportable
                << std::endl;
      const size_t NumIns = red.mAccumInCount;
      for (int insIdx = 0; insIdx < NumIns; ++insIdx)
        std::cout << "    (In) " << red.mAccumIns[insIdx].str() << std::endl;
    }
  }
}

// ForEach /////////////////////////////////////////////////////////////////////////////////////

void ReflectionState::beginForEaches(size_t Count) {
  slangAssert(!isClosed());
  if (!isActive())
    return;

  if (isCollecting()) {
    auto &file = mFiles.Current();
    file.mForEaches = new File::ForEach[Count];
    file.mForEachCount = Count;
  }
  if (isUsing()) {
    slangAssert(mForEachesBad.empty());
    mNumForEachesMatchedByOrdinal = 0;
  }
}

// Keep this in sync with RSReflectionJava::genExportForEach().
void ReflectionState::beginForEach(const RSExportForEach *EF) {
  slangAssert(!isClosed() && (mForEachOpen < 0));
  if (!isActive())
    return;

  const bool IsKernel = EF->isKernelStyle();
  const std::string& Name = EF->getName();
  const unsigned Ordinal = EF->getOrdinal();
  const size_t InCount = EF->getInTypes().size();
  const size_t ParamCount = EF->params_count();

  const RSExportType *OET = EF->getOutType();
  if (OET && !IsKernel) {
    slangAssert(OET->getClass() == RSExportType::ExportClassPointer);
    OET = static_cast<const RSExportPointerType *>(OET)->getPointeeType();
  }
  const std::string OutType = (OET ? getUniqueTypeName(OET) : "");
  const bool HasOut = (EF->hasOut() || EF->hasReturn());

  mForEachOpen = Ordinal;
  mForEachFatal = true;  // we'll set this to false if everything looks ok

  auto &file = mFiles.Current();
  auto &foreaches = file.mForEaches;
  if (isCollecting()) {
    slangAssert(Ordinal < file.mForEachCount);
    auto &foreach = foreaches[Ordinal];
    slangAssert(foreach.mState == File::ForEach::S_Initial);
    foreach.mState = File::ForEach::S_Collected;
    foreach.mName = Name;
    foreach.mIns.BeginCollecting(InCount);
    foreach.mParams.BeginCollecting(ParamCount);
    foreach.mOut = canon(OutType);
    foreach.mHasOut = HasOut;
    foreach.mSignatureMetadata = 0;
    foreach.mIsKernel = IsKernel;
  }
  if (isUsing()) {
    if (Ordinal >= file.mForEachCount) {
      mForEachesBad.push_back(EF);
      return;
    }

    auto &foreach = foreaches[Ordinal];
    slangAssert(foreach.mState == File::ForEach::S_Collected);
    foreach.mState = File::ForEach::S_UseMatched;
    ++mNumForEachesMatchedByOrdinal;

    if (foreach.mName != Name) {
      // Order matters because it determines slot number
      mForEachesBad.push_back(EF);
      return;
    }

    // At this point, we have matching ordinal and matching name.

    if (foreach.mIsKernel != IsKernel) {
      mRSC->ReportError(EF->getLocation(),
                        "foreach kernel '%0' has __attribute__((kernel)) for %select{32|64}1-bit targets "
                        "but not for %select{64|32}1-bit targets")
          << Name << IsKernel;
      return;
    }

    if ((foreach.mHasOut != HasOut) || !foreach.mOut.equals(OutType)) {
      // There are several different patterns we need to handle:
      // (1) Two different non-void* output types
      // (2) One non-void* output type, one void* output type
      // (3) One non-void* output type, one no-output
      // (4) One void* output type, one no-output
      if (foreach.mHasOut && HasOut) {
        if (foreach.mOut.size() && OutType.size()) {
          // (1) Two different non-void* output types
          mRSC->ReportError(EF->getLocation(),
                            "foreach kernel '%0' has output type '%1' for 32-bit targets "
                            "but output type '%2' for 64-bit targets")
              << Name << foreach.mOut.str() << OutType;
        } else {
          // (2) One non-void* return type, one void* output type
          const bool hasTyped64 = OutType.size();
          mRSC->ReportError(EF->getLocation(),
                            "foreach kernel '%0' has output type '%1' for %select{32|64}2-bit targets "
                            "but has untyped output for %select{64|32}2-bit targets")
              << Name << (foreach.mOut.str() + OutType) << hasTyped64;
        }
      } else {
        const std::string CombinedOutType = (foreach.mOut.str() + OutType);
        if (CombinedOutType.size()) {
          // (3) One non-void* output type, one no-output
          mRSC->ReportError(EF->getLocation(),
                            "foreach kernel '%0' has output type '%1' for %select{32|64}2-bit targets "
                            "but no output for %select{64|32}2-bit targets")
              << Name << CombinedOutType << HasOut;
        } else {
          // (4) One void* output type, one no-output
          mRSC->ReportError(EF->getLocation(),
                            "foreach kernel '%0' has untyped output for %select{32|64}1-bit targets "
                            "but no output for %select{64|32}1-bit targets")
              << Name << HasOut;
        }
      }
    }

    bool BadCount = false;
    if (foreach.mIns.Size() != InCount) {
      mRSC->ReportError(EF->getLocation(),
                        "foreach kernel '%0' has %1 input%s1 for 32-bit targets "
                        "but %2 input%s2 for 64-bit targets")
          << Name << unsigned(foreach.mIns.Size()) << unsigned(InCount);
      BadCount = true;
    }
    if (foreach.mParams.Size() != ParamCount) {
      mRSC->ReportError(EF->getLocation(),
                        "foreach kernel '%0' has %1 usrData parameter%s1 for 32-bit targets "
                        "but %2 usrData parameter%s2 for 64-bit targets")
          << Name << unsigned(foreach.mParams.Size()) << unsigned(ParamCount);
      BadCount = true;
    }

    if (BadCount)
      return;

    foreach.mIns.BeginUsing();
    foreach.mParams.BeginUsing();
  }

  mForEachFatal = false;
}

void ReflectionState::addForEachIn(const RSExportForEach *EF, const RSExportType *Type) {
  slangAssert(!isClosed());
  if (!isActive())
    return;

  slangAssert(mForEachOpen == EF->getOrdinal());

  // Type may be nullptr in the case of void*.  See RSExportForEach::Create().
  if (Type && !EF->isKernelStyle()) {
    slangAssert(Type->getClass() == RSExportType::ExportClassPointer);
    Type = static_cast<const RSExportPointerType *>(Type)->getPointeeType();
  }
  const std::string TypeName = (Type ? getUniqueTypeName(Type) : std::string());

  auto &ins = mFiles.Current().mForEaches[EF->getOrdinal()].mIns;
  if (isCollecting()) {
    ins.CollectNext() = canon(TypeName);
  }
  if (isUsing()) {
    if (mForEachFatal)
      return;

    if (!ins.UseNext().equals(TypeName)) {
      if (ins.Current().size() && TypeName.size()) {
        mRSC->ReportError(EF->getLocation(),
                          "%ordinal0 input of foreach kernel '%1' "
                          "has type '%2' for 32-bit targets "
                          "but type '%3' for 64-bit targets")
            << unsigned(ins.CurrentIdx() + 1)
            << EF->getName()
            << ins.Current().str()
            << TypeName;
      } else {
        const bool hasType64 = TypeName.size();
        mRSC->ReportError(EF->getLocation(),
                          "%ordinal0 input of foreach kernel '%1' "
                          "has type '%2' for %select{32|64}3-bit targets "
                          "but is untyped for %select{64|32}3-bit targets")
            << unsigned(ins.CurrentIdx() + 1)
            << EF->getName()
            << (ins.Current().str() + TypeName)
            << hasType64;
      }
    }
  }
}

void ReflectionState::addForEachParam(const RSExportForEach *EF, const RSExportType *Type) {
  slangAssert(!isClosed());
  if (!isActive())
    return;

  slangAssert(mForEachOpen == EF->getOrdinal());

  const std::string TypeName = getUniqueTypeName(Type);

  auto &params = mFiles.Current().mForEaches[EF->getOrdinal()].mParams;
  if (isCollecting()) {
    params.CollectNext() = canon(TypeName);
  }
  if (isUsing()) {
    if (mForEachFatal)
      return;

    if (!params.UseNext().equals(TypeName)) {
      mRSC->ReportError(EF->getLocation(),
                        "%ordinal0 usrData parameter of foreach kernel '%1' "
                        "has type '%2' for 32-bit targets "
                        "but type '%3' for 64-bit targets")
          << unsigned(params.CurrentIdx() + 1)
          << EF->getName()
          << params.Current().str()
          << TypeName;
    }
  }
}

void ReflectionState::addForEachSignatureMetadata(const RSExportForEach *EF, unsigned Metadata) {
  slangAssert(!isClosed());
  if (!isActive())
    return;

  slangAssert(mForEachOpen == EF->getOrdinal());

  // These are properties in the metadata that we need to check.
  const unsigned SpecialParameterBits = bcinfo::MD_SIG_X|bcinfo::MD_SIG_Y|bcinfo::MD_SIG_Z|bcinfo::MD_SIG_Ctxt;

#ifndef __DISABLE_ASSERTS
  {
    // These are properties in the metadata that we already check in
    // some other way.
    const unsigned BoringBits = bcinfo::MD_SIG_In|bcinfo::MD_SIG_Out|bcinfo::MD_SIG_Usr|bcinfo::MD_SIG_Kernel;

    slangAssert((Metadata & ~(SpecialParameterBits | BoringBits)) == 0);
  }
#endif

  auto &mSignatureMetadata = mFiles.Current().mForEaches[EF->getOrdinal()].mSignatureMetadata;
  if (isCollecting()) {
    mSignatureMetadata = Metadata;
  }
  if (isUsing()) {
    if (mForEachFatal)
      return;

    if ((mSignatureMetadata & SpecialParameterBits) != (Metadata & SpecialParameterBits)) {
      mRSC->ReportError(EF->getLocation(),
                        "foreach kernel '%0' has different special parameters "
                        "for 32-bit targets than for 64-bit targets")
          << EF->getName();
    }
  }
}

void ReflectionState::endForEach() {
  slangAssert(!isClosed());
  if (!isActive())
    return;

  slangAssert(mForEachOpen >= 0);
  if (isUsing() && !mForEachFatal) {
    slangAssert(mFiles.Current().mForEaches[mForEachOpen].mIns.isFinished());
    slangAssert(mFiles.Current().mForEaches[mForEachOpen].mParams.isFinished());
  }

  mForEachOpen = -1;
}

void ReflectionState::endForEaches() {
  slangAssert(mForEachOpen < 0);
  if (!isUsing())
    return;

  const auto &file = mFiles.Current();

  if (!mForEachesBad.empty()) {
    std::sort(mForEachesBad.begin(), mForEachesBad.end(),
         [](const RSExportForEach *a, const RSExportForEach *b) { return a->getOrdinal() < b->getOrdinal(); });
    // Note that after the sort, all kernels that are bad because of
    // name mismatch precede all kernels that are bad because of
    // too-high ordinal.

    // 32-bit and 64-bit compiles need to see foreach kernels in the
    // same order, because of slot number assignment.  Once we see the
    // first name mismatch in the sequence of foreach kernels, it
    // doesn't make sense to issue further diagnostics regarding
    // foreach kernels except those that still happen to match by name
    // and ordinal (we already handled those diagnostics between
    // beginForEach() and endForEach()).
    bool ForEachesOrderFatal = false;

    for (const RSExportForEach *EF : mForEachesBad) {
      if (EF->getOrdinal() >= file.mForEachCount) {
        mRSC->ReportError(EF->getLocation(),
                          "foreach kernel '%0' is only present for 64-bit targets")
            << EF->getName();
      } else {
        mRSC->ReportError(EF->getLocation(),
                          "%ordinal0 foreach kernel is '%1' for 32-bit targets "
                          "but '%2' for 64-bit targets")
            << (EF->getOrdinal() + 1)
            << mFiles.Current().mForEaches[EF->getOrdinal()].mName
            << EF->getName();
        ForEachesOrderFatal = true;
        break;
      }
    }

    mForEachesBad.clear();

    if (ForEachesOrderFatal)
      return;
  }

  if (mNumForEachesMatchedByOrdinal == file.mForEachCount)
    return;
  for (unsigned ord = 0; ord < file.mForEachCount; ord++) {
    const auto &fe = file.mForEaches[ord];
    if (fe.mState == File::ForEach::S_Collected) {
      mRSC->ReportError("in file '%0' foreach kernel '%1' is only present for 32-bit targets")
          << file.mRSSourceFileName << fe.mName;
    }
  }
}

// Invokable ///////////////////////////////////////////////////////////////////////////////////

// Keep this in sync with RSReflectionJava::genExportFunction().
void ReflectionState::declareInvokable(const RSExportFunc *EF) {
  slangAssert(!isClosed());
  if (!isActive())
    return;

  const std::string& Name = EF->getName(/*Mangle=*/false);
  const size_t ParamCount = EF->getNumParameters();

  auto &invokables = mFiles.Current().mInvokables;
  if (isCollecting()) {
    auto &invokable = invokables.CollectNext();
    invokable.mName = Name;
    invokable.mParamCount = ParamCount;
    if (EF->hasParam()) {
      unsigned FieldIdx = 0;
      invokable.mParams = new llvm::StringRef[ParamCount];
      for (RSExportFunc::const_param_iterator I = EF->params_begin(),
                                              E = EF->params_end();
           I != E; I++, FieldIdx++) {
        invokable.mParams[FieldIdx] = canon(getUniqueTypeName((*I)->getType()));
      }
    }
  }
  if (isUsing()) {
    if (mInvokablesOrderFatal)
      return;

    if (invokables.isFinished()) {
      // This doesn't actually break reflection, but that's a
      // coincidence of the fact that we reflect during the 64-bit
      // compilation pass rather than the 32-bit compilation pass, and
      // of the fact that the "extra" invokable(s) are at the end.
      mRSC->ReportError(EF->getLocation(),
                        "invokable function '%0' is only present for 64-bit targets")
          << Name;
      return;
    }

    auto &invokable = invokables.UseNext();

    if (invokable.mName != Name) {
      // Order matters because it determines slot number
      mRSC->ReportError(EF->getLocation(),
                        "%ordinal0 invokable function is '%1' for 32-bit targets "
                        "but '%2' for 64-bit targets")
          << unsigned(invokables.CurrentIdx() + 1)
          << invokable.mName
          << Name;
      mInvokablesOrderFatal = true;
      return;
    }

    if (invokable.mParamCount != ParamCount) {
      mRSC->ReportError(EF->getLocation(),
                        "invokable function '%0' has %1 parameter%s1 for 32-bit targets "
                        "but %2 parameter%s2 for 64-bit targets")
          << Name << unsigned(invokable.mParamCount) << unsigned(ParamCount);
      return;
    }
    if (EF->hasParam()) {
      unsigned FieldIdx = 0;
      for (RSExportFunc::const_param_iterator I = EF->params_begin(),
                                              E = EF->params_end();
           I != E; I++, FieldIdx++) {
        const std::string Type = getUniqueTypeName((*I)->getType());
        if (!invokable.mParams[FieldIdx].equals(Type)) {
          mRSC->ReportError(EF->getLocation(),
                            "%ordinal0 parameter of invokable function '%1' "
                            "has type '%2' for 32-bit targets "
                            "but type '%3' for 64-bit targets")
              << (FieldIdx + 1)
              << Name
              << invokable.mParams[FieldIdx].str()
              << Type;
        }
      }
    }
  }
}

void ReflectionState::endInvokables() {
  if (!isUsing() || mInvokablesOrderFatal)
    return;

  auto &invokables = mFiles.Current().mInvokables;
  while (!invokables.isFinished()) {
    const auto &invokable = invokables.UseNext();
    mRSC->ReportError("in file '%0' invokable function '%1' is only present for 32-bit targets")
        << mFiles.Current().mRSSourceFileName << invokable.mName;
  }
}

// Record //////////////////////////////////////////////////////////////////////////////////////

void ReflectionState::beginRecords() {
  slangAssert(!isClosed());
  if (!isActive())
    return;

  slangAssert(mRecordsState != RS_Open);
  mRecordsState = RS_Open;
  mNumRecordsMatchedByName = 0;
}

void ReflectionState::endRecords() {
  slangAssert(!isClosed());
  if (!isActive())
    return;

  slangAssert(mRecordsState == RS_Open);
  mRecordsState = RS_Closed;

  if (isUsing()) {
    const File &file = mFiles.Current();
    if (mNumRecordsMatchedByName == file.mRecords.size())
      return;
    // NOTE: "StringMap iteration order, however, is not guaranteed to
    // be deterministic".  So sort by name before reporting.
    // Alternatively, if we record additional information, we could
    // sort by source location or by order in which we discovered the
    // need to export.
    std::vector<llvm::StringRef> Non64RecordNames;
    for (auto I = file.mRecords.begin(), E = file.mRecords.end(); I != E; I++)
      if (!I->getValue().mMatchedByName && I->getValue().mOrdinary)
        Non64RecordNames.push_back(I->getKey());
    std::sort(Non64RecordNames.begin(), Non64RecordNames.end(),
              [](llvm::StringRef a, llvm::StringRef b) { return a.compare(b)==-1; });
    for (auto N : Non64RecordNames)
      mRSC->ReportError("in file '%0' structure '%1' is exported only for 32-bit targets")
          << file.mRSSourceFileName << N.str();
  }
}

void ReflectionState::declareRecord(const RSExportRecordType *ERT, bool Ordinary) {
  slangAssert(!isClosed());
  if (!isActive())
    return;

  slangAssert(mRecordsState == RS_Open);

  auto &records = mFiles.Current().mRecords;
  if (isCollecting()) {
    // Keep struct/field layout in sync with
    // RSReflectionJava::genPackVarOfType() and
    // RSReflectionJavaElementBuilder::genAddElement()

    // Save properties of record

    const size_t FieldCount = ERT->fields_size();
    File::Record::Field *Fields = new File::Record::Field[FieldCount];

    size_t Pos = 0;  // Relative position of field within record
    unsigned FieldIdx = 0;
    for (RSExportRecordType::const_field_iterator I = ERT->fields_begin(), E = ERT->fields_end();
         I != E; I++, FieldIdx++) {
      const RSExportRecordType::Field *FieldExport = *I;
      size_t FieldOffset = FieldExport->getOffsetInParent();
      const RSExportType *T = FieldExport->getType();
      size_t FieldStoreSize = T->getStoreSize();
      size_t FieldAllocSize = T->getAllocSize();

      slangAssert(FieldOffset >= Pos);
      slangAssert(FieldAllocSize >= FieldStoreSize);

      auto &FieldState = Fields[FieldIdx];
      FieldState.mName = FieldExport->getName();
      FieldState.mType = canon(getUniqueTypeName(T));
      FieldState.mPrePadding = FieldOffset - Pos;
      FieldState.mPostPadding = FieldAllocSize - FieldStoreSize;
      FieldState.mOffset = FieldOffset;
      FieldState.mStoreSize = FieldStoreSize;

      Pos = FieldOffset + FieldAllocSize;
    }

    slangAssert(ERT->getAllocSize() >= Pos);

    // Insert record into map

    slangAssert(records.find(ERT->getName()) == records.end());
    File::Record &record = records[ERT->getName()];
    record.mFields = Fields;
    record.mFieldCount = FieldCount;
    record.mPostPadding = ERT->getAllocSize() - Pos;
    record.mAllocSize = ERT->getAllocSize();
    record.mOrdinary = Ordinary;
    record.mMatchedByName = false;
  }
  if (isUsing()) {
    if (!Ordinary)
      return;

    const auto RIT = records.find(ERT->getName());
    if (RIT == records.end()) {
      // This doesn't actually break reflection, but that's a
      // coincidence of the fact that we reflect during the 64-bit
      // compilation pass rather than the 32-bit compilation pass, so
      // a record that's only classified as exported during the 64-bit
      // compilation pass doesn't cause any problems.
      mRSC->ReportError(ERT->getLocation(), "structure '%0' is exported only for 64-bit targets")
          << ERT->getName();
      return;
    }
    File::Record &record = RIT->getValue();
    record.mMatchedByName = true;
    ++mNumRecordsMatchedByName;
    slangAssert(record.mOrdinary);

    if (ERT->fields_size() != record.mFieldCount) {
      mRSC->ReportError(ERT->getLocation(),
                        "exported structure '%0' has %1 field%s1 for 32-bit targets "
                        "but %2 field%s2 for 64-bit targets")
          << ERT->getName() << unsigned(record.mFieldCount) << unsigned(ERT->fields_size());
      return;
    }

    // Note that we are deliberately NOT comparing layout properties
    // (such as Field offsets and sizes, or Record allocation size);
    // we need to tolerate layout differences between 32-bit
    // compilation and 64-bit compilation.

    unsigned FieldIdx = 0;
    for (RSExportRecordType::const_field_iterator I = ERT->fields_begin(), E = ERT->fields_end();
         I != E; I++, FieldIdx++) {
      const RSExportRecordType::Field &FieldExport = **I;
      const File::Record::Field &FieldState = record.mFields[FieldIdx];
      if (FieldState.mName != FieldExport.getName()) {
        mRSC->ReportError(ERT->getLocation(),
                          "%ordinal0 field of exported structure '%1' "
                          "is '%2' for 32-bit targets "
                          "but '%3' for 64-bit targets")
            << (FieldIdx + 1) << ERT->getName() << FieldState.mName << FieldExport.getName();
        return;
      }
      const std::string FieldExportType = getUniqueTypeName(FieldExport.getType());
      if (!FieldState.mType.equals(FieldExportType)) {
        mRSC->ReportError(ERT->getLocation(),
                          "field '%0' of exported structure '%1' "
                          "has type '%2' for 32-bit targets "
                          "but type '%3' for 64-bit targets")
            << FieldState.mName << ERT->getName() << FieldState.mType.str() << FieldExportType;
      }
    }
  }
}

ReflectionState::Record32
ReflectionState::getRecord32(const RSExportRecordType *ERT) {
  if (isUsing()) {
    const auto &Records = mFiles.Current().mRecords;
    const auto RIT = Records.find(ERT->getName());
    if (RIT != Records.end())
      return Record32(&RIT->getValue());
  }
  return Record32();
}

// Reduce //////////////////////////////////////////////////////////////////////////////////////

void ReflectionState::declareReduce(const RSExportReduce *ER, bool IsExportable) {
  slangAssert(!isClosed());
  if (!isActive())
    return;

  auto &reduces = mFiles.Current().mReduces;
  if (isCollecting()) {
    auto &reduce = reduces.CollectNext();
    reduce.mName = ER->getNameReduce();

    const auto &InTypes = ER->getAccumulatorInTypes();
    const size_t InTypesSize = InTypes.size();
    reduce.mAccumInCount = InTypesSize;
    reduce.mAccumIns = new llvm::StringRef[InTypesSize];
    unsigned InTypesIdx = 0;
    for (const auto &InType : InTypes)
      reduce.mAccumIns[InTypesIdx++] = canon(getUniqueTypeName(InType));

    reduce.mResult = canon(getUniqueTypeName(ER->getResultType()));
    reduce.mIsExportable = IsExportable;
  }
  if (isUsing()) {
    if (mReducesOrderFatal)
      return;

    const std::string& Name = ER->getNameReduce();

    if (reduces.isFinished()) {
      // This doesn't actually break reflection, but that's a
      // coincidence of the fact that we reflect during the 64-bit
      // compilation pass rather than the 32-bit compilation pass, and
      // of the fact that the "extra" reduction kernel(s) are at the
      // end.
      mRSC->ReportError(ER->getLocation(),
                        "reduction kernel '%0' is only present for 64-bit targets")
          << Name;
      return;
    }

    auto &reduce = reduces.UseNext();

    if (reduce.mName != Name) {
      // Order matters because it determines slot number.  We might be
      // able to tolerate certain cases if we ignore non-exportable
      // kernels in the two sequences (32-bit and 64-bit) -- non-exportable
      // kernels do not take up slot numbers.
      mRSC->ReportError(ER->getLocation(),
                        "%ordinal0 reduction kernel is '%1' for 32-bit targets "
                        "but '%2' for 64-bit targets")
          << unsigned(reduces.CurrentIdx() + 1)
          << reduce.mName
          << Name;
      mReducesOrderFatal = true;
      return;
    }

    // If at least one of the two kernels (32-bit or 64-bit) is not
    // exporable, then there will be no reflection for that kernel,
    // and so any mismatch in result type or in inputs is irrelevant.
    // However, we may make more kernels exportable in the future.
    // Therefore, we'll forbid mismatches anyway.

    if (reduce.mIsExportable != IsExportable) {
      mRSC->ReportError(ER->getLocation(),
                        "reduction kernel '%0' is reflected in Java only for %select{32|64}1-bit targets")
          << reduce.mName
          << IsExportable;
    }

    const std::string ResultType = getUniqueTypeName(ER->getResultType());
    if (!reduce.mResult.equals(ResultType)) {
      mRSC->ReportError(ER->getLocation(),
                        "reduction kernel '%0' has result type '%1' for 32-bit targets "
                        "but result type '%2' for 64-bit targets")
          << reduce.mName << reduce.mResult.str() << ResultType;
    }

    const auto &InTypes = ER->getAccumulatorInTypes();
    if (reduce.mAccumInCount != InTypes.size()) {
      mRSC->ReportError(ER->getLocation(),
                        "reduction kernel '%0' has %1 input%s1 for 32-bit targets "
                        "but %2 input%s2 for 64-bit targets")
          << Name << unsigned(reduce.mAccumInCount) << unsigned(InTypes.size());
      return;
    }
    unsigned FieldIdx = 0;
    for (const auto &InType : InTypes) {
      const std::string InTypeName = getUniqueTypeName(InType);
      const llvm::StringRef StateInTypeName = reduce.mAccumIns[FieldIdx++];
      if (!StateInTypeName.equals(InTypeName)) {
        mRSC->ReportError(ER->getLocation(),
                          "%ordinal0 input of reduction kernel '%1' "
                          "has type '%2' for 32-bit targets "
                          "but type '%3' for 64-bit targets")
            << FieldIdx
            << Name
            << StateInTypeName.str()
            << InTypeName;
      }
    }
  }
}

void ReflectionState::endReduces() {
  if (!isUsing() || mReducesOrderFatal)
    return;

  auto &reduces = mFiles.Current().mReduces;
  while (!reduces.isFinished()) {
    const auto &reduce = reduces.UseNext();
    mRSC->ReportError("in file '%0' reduction kernel '%1' is only present for 32-bit targets")
        << mFiles.Current().mRSSourceFileName << reduce.mName;
  }
}

// Variable ////////////////////////////////////////////////////////////////////////////////////

// Keep this in sync with initialization handling in
// RSReflectionJava::genScriptClassConstructor().
ReflectionState::Val32 ReflectionState::declareVariable(const RSExportVar *EV) {
  slangAssert(!isClosed());
  if (!isActive())
    return NoVal32();

  auto &variables = mFiles.Current().mVariables;
  if (isCollecting()) {
    auto &variable = variables.CollectNext();
    variable.mName = EV->getName();
    variable.mType = canon(getUniqueTypeName(EV->getType()));
    variable.mAllocSize = EV->getType()->getAllocSize();
    variable.mIsConst = EV->isConst();
    if (!EV->getInit().isUninit()) {
      variable.mInitializerCount = 1;
      variable.mInitializers = new clang::APValue[1];
      variable.mInitializers[0] = EV->getInit();
    } else if (EV->getArraySize()) {
      variable.mInitializerCount = EV->getNumInits();
      variable.mInitializers = new clang::APValue[variable.mInitializerCount];
      for (size_t i = 0; i < variable.mInitializerCount; ++i)
        variable.mInitializers[i] = EV->getInitArray(i);
    } else {
      variable.mInitializerCount = 0;
    }
    return NoVal32();
  }

  /*-- isUsing() -----------------------------------------------------------*/

  slangAssert(isUsing());

  if (mVariablesOrderFatal)
    return NoVal32();

  if (variables.isFinished()) {
    // This doesn't actually break reflection, but that's a
    // coincidence of the fact that we reflect during the 64-bit
    // compilation pass rather than the 32-bit compilation pass, and
    // of the fact that the "extra" variable(s) are at the end.
    mRSC->ReportError(EV->getLocation(), "global variable '%0' is only present for 64-bit targets")
        << EV->getName();
    return NoVal32();
  }

  const auto &variable = variables.UseNext();

  if (variable.mName != EV->getName()) {
    // Order matters because it determines slot number
    mRSC->ReportError(EV->getLocation(),
                      "%ordinal0 global variable is '%1' for 32-bit targets "
                      "but '%2' for 64-bit targets")
        << unsigned(variables.CurrentIdx() + 1)
        << variable.mName
        << EV->getName();
    mVariablesOrderFatal = true;
    return NoVal32();
  }

  const std::string TypeName = getUniqueTypeName(EV->getType());

  if (!variable.mType.equals(TypeName)) {
    mRSC->ReportError(EV->getLocation(),
                      "global variable '%0' has type '%1' for 32-bit targets "
                      "but type '%2' for 64-bit targets")
        << EV->getName()
        << variable.mType.str()
        << TypeName;
    return NoVal32();
  }

  if (variable.mIsConst != EV->isConst()) {
    mRSC->ReportError(EV->getLocation(),
                      "global variable '%0' has inconsistent 'const' qualification "
                      "between 32-bit targets and 64-bit targets")
        << EV->getName();
    return NoVal32();
  }

  // NOTE: Certain syntactically different but semantically
  // equivalent initialization patterns are unnecessarily rejected
  // as errors.
  //
  // Background:
  //
  // . A vector initialized with a scalar value is treated
  //   by reflection as if all elements of the vector are
  //   initialized with the scalar value.
  // . A vector may be initialized with a vector of greater
  //   length; reflection ignores the extra initializers.
  // . If only the beginning of a vector is explicitly
  //   initialized, reflection treats it as if trailing elements are
  //   initialized to zero (by issuing explicit assignments to those
  //   trailing elements).
  // . If only the beginning of an array is explicitly initialized,
  //   reflection treats it as if trailing elements are initialized
  //   to zero (by Java rules for newly-created arrays).
  //
  // Unnecessarily rejected as errors:
  //
  // . One compile initializes a vector with a scalar, and
  //   another initializes it with a vector whose elements
  //   are the scalar, as in
  //
  //     int2 x =
  //     #ifdef __LP64__
  //       1
  //     #else
  //       { 1, 1 }
  //     #endif
  //
  // . Compiles initialize a vector with vectors of different
  //   lengths, but the initializers agree up to the length
  //   of the variable being initialized, as in
  //
  //     int2 x = { 1, 2
  //     #ifdef __LP64__
  //       3
  //     #else
  //       4
  //     #endif
  //     };
  //
  // . Two compiles agree with the initializer for a vector or
  //   array, except that one has some number of explicit trailing
  //   zeroes, as in
  //
  //     int x[4] = { 3, 2, 1
  //     #ifdef __LP64__
  //       , 0
  //     #endif
  //     };

  bool MismatchedInitializers = false;
  if (!EV->getInit().isUninit()) {
    // Use phase has a scalar initializer.
    // Make sure that Collect phase had a matching scalar initializer.
    if ((variable.mInitializerCount != 1) ||
        !equal(variable.mInitializers[0], EV->getInit()))
      MismatchedInitializers = true;
  } else if (EV->getArraySize()) {
    const size_t UseSize = EV->getNumInits();
    if (variable.mInitializerCount != UseSize)
      MismatchedInitializers = true;
    else {
      for (int i = 0; i < UseSize; ++i)
        if (!equal(variable.mInitializers[i], EV->getInitArray(i))) {
          MismatchedInitializers = true;
          break;
        }
    }
  } else if (variable.mInitializerCount != 0) {
    // Use phase does not have a scalar initializer, variable is not
    // an array, and Collect phase has an initializer.  This is an error.
    MismatchedInitializers = true;
  }

  if (MismatchedInitializers) {
    mRSC->ReportError(EV->getLocation(),
                      "global variable '%0' is initialized differently for 32-bit targets "
                      "than for 64-bit targets")
        << EV->getName();
    return NoVal32();
  }

  return Val32(true, variable.mAllocSize);
}

void ReflectionState::endVariables() {
  if (!isUsing() || mVariablesOrderFatal)
    return;

  auto &variables = mFiles.Current().mVariables;
  while (!variables.isFinished()) {
    const auto &variable = variables.UseNext();
    mRSC->ReportError("in file '%0' global variable '%1' is only present for 32-bit targets")
        << mFiles.Current().mRSSourceFileName << variable.mName;
  }
}

}  // namespace slang
