/*
 * Copyright 2011-2012, The Android Open Source Project
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

#include "bcinfo/MetadataExtractor.h"

#include "bcinfo/BitcodeWrapper.h"
#include "rsDefines.h"

#define LOG_TAG "bcinfo"
#include <log/log.h>

#include "Assert.h"

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/MemoryBuffer.h"

#ifdef __ANDROID__
#include "Properties.h"
#endif

#include <cstdlib>

namespace bcinfo {

namespace {

llvm::StringRef getStringOperand(const llvm::Metadata *node) {
  if (auto *mds = llvm::dyn_cast_or_null<const llvm::MDString>(node)) {
    return mds->getString();
  }
  return llvm::StringRef();
}

bool extractUIntFromMetadataString(uint32_t *value,
    const llvm::Metadata *m) {
  llvm::StringRef SigString = getStringOperand(m);
  if (SigString != "") {
    if (!SigString.getAsInteger(10, *value)) {
      return true;
    }
  }
  return false;
}

const char *createStringFromValue(llvm::Metadata *m) {
  auto ref = getStringOperand(m);
  char *c = new char[ref.size() + 1];
  memcpy(c, ref.data(), ref.size());
  c[ref.size()] = '\0';
  return c;
}

const char *createStringFromOptionalValue(llvm::MDNode *n, unsigned opndNum) {
  llvm::Metadata *opnd;
  if (opndNum >= n->getNumOperands() || !(opnd = n->getOperand(opndNum)))
    return nullptr;
  return createStringFromValue(opnd);
}

// Collect metadata from NamedMDNodes that contain a list of names
// (strings).
//
// Inputs:
//
// NamedMetadata - An LLVM metadata node, each of whose operands have
// a string as their first entry
//
// NameList - A reference that will hold an allocated array of strings
//
// Count - A reference that will hold the length of the allocated
// array of strings
//
// Return value:
//
// Return true on success, false on error.
//
// Upon success, the function sets NameList to an array of strings
// corresponding the names found in the metadata. The function sets
// Count to the number of entries in NameList.
//
// An error occurs if one of the metadata operands doesn't have a
// first entry.
bool populateNameMetadata(const llvm::NamedMDNode *NameMetadata,
                          const char **&NameList, size_t &Count) {
  if (!NameMetadata) {
    NameList = nullptr;
    Count = 0;
    return true;
  }

  Count = NameMetadata->getNumOperands();
  if (!Count) {
    NameList = nullptr;
    return true;
  }

  NameList = new const char *[Count];

  for (size_t i = 0; i < Count; i++) {
    llvm::MDNode *Name = NameMetadata->getOperand(i);
    if (Name && Name->getNumOperands() > 0) {
      NameList[i] = createStringFromValue(Name->getOperand(0));
    } else {
      ALOGE("Metadata operand does not contain a name string");
      for (size_t AllocatedIndex = 0; AllocatedIndex < i; AllocatedIndex++) {
        delete [] NameList[AllocatedIndex];
      }
      delete [] NameList;
      NameList = nullptr;
      Count = 0;

      return false;
    }
  }

  return true;
}

} // end anonymous namespace

// Name of metadata node where pragma info resides (should be synced with
// slang.cpp)
static const llvm::StringRef PragmaMetadataName = "#pragma";

// Name of metadata node where exported variable names reside (should be
// synced with slang_rs_metadata.h)
static const llvm::StringRef ExportVarMetadataName = "#rs_export_var";

// Name of metadata node where exported function names reside (should be
// synced with slang_rs_metadata.h)
static const llvm::StringRef ExportFuncMetadataName = "#rs_export_func";

// Name of metadata node where exported ForEach name information resides
// (should be synced with slang_rs_metadata.h)
static const llvm::StringRef ExportForEachNameMetadataName =
    "#rs_export_foreach_name";

// Name of metadata node where exported ForEach signature information resides
// (should be synced with slang_rs_metadata.h)
static const llvm::StringRef ExportForEachMetadataName = "#rs_export_foreach";

// Name of metadata node where exported general reduce information resides
// (should be synced with slang_rs_metadata.h)
static const llvm::StringRef ExportReduceMetadataName = "#rs_export_reduce";

// Name of metadata node where RS object slot info resides (should be
// synced with slang_rs_metadata.h)
static const llvm::StringRef ObjectSlotMetadataName = "#rs_object_slots";

static const llvm::StringRef ThreadableMetadataName = "#rs_is_threadable";

// Name of metadata node where the checksum for this build is stored.  (should
// be synced with libbcc/lib/Core/Source.cpp)
static const llvm::StringRef ChecksumMetadataName = "#rs_build_checksum";

// Name of metadata node which contains a list of compile units that have debug
// metadata. If this is null then there is no debug metadata in the compile
// unit.
static const llvm::StringRef DebugInfoMetadataName = "llvm.dbg.cu";

const char MetadataExtractor::kWrapperMetadataName[] = "#rs_wrapper";

MetadataExtractor::MetadataExtractor(const char *bitcode, size_t bitcodeSize)
    : mModule(nullptr), mBitcode(bitcode), mBitcodeSize(bitcodeSize),
      mExportVarCount(0), mExportFuncCount(0), mExportForEachSignatureCount(0),
      mExportReduceCount(0), mExportVarNameList(nullptr),
      mExportFuncNameList(nullptr), mExportForEachNameList(nullptr),
      mExportForEachSignatureList(nullptr),
      mExportForEachInputCountList(nullptr),
      mExportReduceList(nullptr),
      mPragmaCount(0), mPragmaKeyList(nullptr), mPragmaValueList(nullptr),
      mObjectSlotCount(0), mObjectSlotList(nullptr),
      mRSFloatPrecision(RS_FP_Full), mIsThreadable(true),
      mBuildChecksum(nullptr), mHasDebugInfo(false) {
  BitcodeWrapper wrapper(bitcode, bitcodeSize);
  mCompilerVersion = wrapper.getCompilerVersion();
  mOptimizationLevel = wrapper.getOptimizationLevel();
}

MetadataExtractor::MetadataExtractor(const llvm::Module *module)
    : mModule(module), mBitcode(nullptr), mBitcodeSize(0),
      mExportVarCount(0), mExportFuncCount(0), mExportForEachSignatureCount(0),
      mExportReduceCount(0), mExportVarNameList(nullptr),
      mExportFuncNameList(nullptr), mExportForEachNameList(nullptr),
      mExportForEachSignatureList(nullptr),
      mExportForEachInputCountList(nullptr),
      mExportReduceList(nullptr),
      mPragmaCount(0), mPragmaKeyList(nullptr), mPragmaValueList(nullptr),
      mObjectSlotCount(0), mObjectSlotList(nullptr),
      mRSFloatPrecision(RS_FP_Full), mIsThreadable(true),
      mBuildChecksum(nullptr) {
  const llvm::NamedMDNode *const wrapperMDNode = module->getNamedMetadata(kWrapperMetadataName);
  bccAssert((wrapperMDNode != nullptr) && (wrapperMDNode->getNumOperands() == 1));
  const llvm::MDNode *const wrapperMDTuple = wrapperMDNode->getOperand(0);

  bool success = true;
  success &= extractUIntFromMetadataString(&mCompilerVersion, wrapperMDTuple->getOperand(0));
  success &= extractUIntFromMetadataString(&mOptimizationLevel, wrapperMDTuple->getOperand(1));
  bccAssert(success);
}


MetadataExtractor::~MetadataExtractor() {
  if (mExportVarNameList) {
    for (size_t i = 0; i < mExportVarCount; i++) {
        delete [] mExportVarNameList[i];
        mExportVarNameList[i] = nullptr;
    }
  }
  delete [] mExportVarNameList;
  mExportVarNameList = nullptr;

  if (mExportFuncNameList) {
    for (size_t i = 0; i < mExportFuncCount; i++) {
        delete [] mExportFuncNameList[i];
        mExportFuncNameList[i] = nullptr;
    }
  }
  delete [] mExportFuncNameList;
  mExportFuncNameList = nullptr;

  if (mExportForEachNameList) {
    for (size_t i = 0; i < mExportForEachSignatureCount; i++) {
        delete [] mExportForEachNameList[i];
        mExportForEachNameList[i] = nullptr;
    }
  }
  delete [] mExportForEachNameList;
  mExportForEachNameList = nullptr;

  delete [] mExportForEachSignatureList;
  mExportForEachSignatureList = nullptr;

  delete [] mExportForEachInputCountList;
  mExportForEachInputCountList = nullptr;

  delete [] mExportReduceList;
  mExportReduceList = nullptr;

  for (size_t i = 0; i < mPragmaCount; i++) {
    if (mPragmaKeyList) {
      delete [] mPragmaKeyList[i];
      mPragmaKeyList[i] = nullptr;
    }
    if (mPragmaValueList) {
      delete [] mPragmaValueList[i];
      mPragmaValueList[i] = nullptr;
    }
  }
  delete [] mPragmaKeyList;
  mPragmaKeyList = nullptr;
  delete [] mPragmaValueList;
  mPragmaValueList = nullptr;

  delete [] mObjectSlotList;
  mObjectSlotList = nullptr;

  delete [] mBuildChecksum;

  return;
}


bool MetadataExtractor::populateObjectSlotMetadata(
    const llvm::NamedMDNode *ObjectSlotMetadata) {
  if (!ObjectSlotMetadata) {
    return true;
  }

  mObjectSlotCount = ObjectSlotMetadata->getNumOperands();

  if (!mObjectSlotCount) {
    return true;
  }

  std::unique_ptr<uint32_t[]> TmpSlotList(new uint32_t[mObjectSlotCount]());
  for (size_t i = 0; i < mObjectSlotCount; i++) {
    llvm::MDNode *ObjectSlot = ObjectSlotMetadata->getOperand(i);
    if (ObjectSlot != nullptr && ObjectSlot->getNumOperands() == 1) {
      if (!extractUIntFromMetadataString(&TmpSlotList[i], ObjectSlot->getOperand(0))) {
        ALOGE("Non-integer object slot value");
        return false;
      }
    } else {
      ALOGE("Corrupt object slot information");
      return false;
    }
  }

  delete [] mObjectSlotList;
  mObjectSlotList = TmpSlotList.release();
  return true;
}


void MetadataExtractor::populatePragmaMetadata(
    const llvm::NamedMDNode *PragmaMetadata) {
  if (!PragmaMetadata) {
    return;
  }

  mPragmaCount = PragmaMetadata->getNumOperands();
  if (!mPragmaCount) {
    return;
  }

  const char **TmpKeyList = new const char*[mPragmaCount];
  const char **TmpValueList = new const char*[mPragmaCount];

  for (size_t i = 0; i < mPragmaCount; i++) {
    llvm::MDNode *Pragma = PragmaMetadata->getOperand(i);
    if (Pragma != nullptr && Pragma->getNumOperands() == 2) {
      llvm::Metadata *PragmaKeyMDS = Pragma->getOperand(0);
      TmpKeyList[i] = createStringFromValue(PragmaKeyMDS);
      llvm::Metadata *PragmaValueMDS = Pragma->getOperand(1);
      TmpValueList[i] = createStringFromValue(PragmaValueMDS);
    }
  }

  mPragmaKeyList = TmpKeyList;
  mPragmaValueList = TmpValueList;

  // Check to see if we have any FP precision-related pragmas.
  std::string Relaxed("rs_fp_relaxed");
  std::string Imprecise("rs_fp_imprecise");
  std::string Full("rs_fp_full");
  bool RelaxedPragmaSeen = false;
  bool FullPragmaSeen = false;
  for (size_t i = 0; i < mPragmaCount; i++) {
    if (!Relaxed.compare(mPragmaKeyList[i])) {
      RelaxedPragmaSeen = true;
    } else if (!Imprecise.compare(mPragmaKeyList[i])) {
      ALOGW("rs_fp_imprecise is deprecated.  Assuming rs_fp_relaxed instead.");
      RelaxedPragmaSeen = true;
    } else if (!Full.compare(mPragmaKeyList[i])) {
      FullPragmaSeen = true;
    }
  }

  if (RelaxedPragmaSeen && FullPragmaSeen) {
    ALOGE("Full and relaxed precision specified at the same time!");
  }
  mRSFloatPrecision = RelaxedPragmaSeen ? RS_FP_Relaxed : RS_FP_Full;

#ifdef __ANDROID__
  // Provide an override for precsiion via adb shell setprop
  // adb shell setprop debug.rs.precision rs_fp_full
  // adb shell setprop debug.rs.precision rs_fp_relaxed
  // adb shell setprop debug.rs.precision rs_fp_imprecise
  char PrecisionPropBuf[PROP_VALUE_MAX];
  const std::string PrecisionPropName("debug.rs.precision");
  property_get("debug.rs.precision", PrecisionPropBuf, "");
  if (PrecisionPropBuf[0]) {
    if (!Relaxed.compare(PrecisionPropBuf)) {
      ALOGI("Switching to RS FP relaxed mode via setprop");
      mRSFloatPrecision = RS_FP_Relaxed;
    } else if (!Imprecise.compare(PrecisionPropBuf)) {
      ALOGW("Switching to RS FP relaxed mode via setprop. rs_fp_imprecise was "
            "specified but is deprecated ");
      mRSFloatPrecision = RS_FP_Relaxed;
    } else if (!Full.compare(PrecisionPropBuf)) {
      ALOGI("Switching to RS FP full mode via setprop");
      mRSFloatPrecision = RS_FP_Full;
    } else {
      ALOGE("Unrecognized debug.rs.precision %s", PrecisionPropBuf);
    }
  }
#endif
}

uint32_t MetadataExtractor::calculateNumInputs(const llvm::Function *Function,
                                               uint32_t Signature) {

  if (hasForEachSignatureIn(Signature)) {
    uint32_t OtherCount = 0;

    OtherCount += hasForEachSignatureUsrData(Signature);
    OtherCount += hasForEachSignatureX(Signature);
    OtherCount += hasForEachSignatureY(Signature);
    OtherCount += hasForEachSignatureZ(Signature);
    OtherCount += hasForEachSignatureCtxt(Signature);
    OtherCount += hasForEachSignatureOut(Signature) &&
                  Function->getReturnType()->isVoidTy();

    return Function->arg_size() - OtherCount;

  } else {
    return 0;
  }
}


bool MetadataExtractor::populateForEachMetadata(
    const llvm::NamedMDNode *Names,
    const llvm::NamedMDNode *Signatures) {
  if (!Names && !Signatures && mCompilerVersion == 0) {
    // Handle legacy case for pre-ICS bitcode that doesn't contain a metadata
    // section for ForEach. We generate a full signature for a "root" function
    // which means that we need to set the bottom 5 bits in the mask.
    mExportForEachSignatureCount = 1;
    char **TmpNameList = new char*[mExportForEachSignatureCount];
    size_t RootLen = strlen(kRoot) + 1;
    TmpNameList[0] = new char[RootLen];
    strncpy(TmpNameList[0], kRoot, RootLen);

    uint32_t *TmpSigList = new uint32_t[mExportForEachSignatureCount];
    TmpSigList[0] = 0x1f;

    mExportForEachNameList = (const char**)TmpNameList;
    mExportForEachSignatureList = TmpSigList;
    return true;
  }

  if (Signatures) {
    mExportForEachSignatureCount = Signatures->getNumOperands();
    if (!mExportForEachSignatureCount) {
      return true;
    }
  } else {
    mExportForEachSignatureCount = 0;
    mExportForEachSignatureList = nullptr;
    return true;
  }

  std::unique_ptr<uint32_t[]> TmpSigList(new uint32_t[mExportForEachSignatureCount]);
  std::unique_ptr<const char *[]> TmpNameList(new const char*[mExportForEachSignatureCount]);
  std::unique_ptr<uint32_t[]> TmpInputCountList(new uint32_t[mExportForEachSignatureCount]);

  for (size_t i = 0; i < mExportForEachSignatureCount; i++) {
    llvm::MDNode *SigNode = Signatures->getOperand(i);
    if (SigNode != nullptr && SigNode->getNumOperands() == 1) {
      if (!extractUIntFromMetadataString(&TmpSigList[i], SigNode->getOperand(0))) {
        ALOGE("Non-integer signature value");
        return false;
      }
    } else {
      ALOGE("Corrupt signature information");
      return false;
    }
  }

  if (Names) {
    for (size_t i = 0; i < mExportForEachSignatureCount; i++) {
      llvm::MDNode *Name = Names->getOperand(i);
      if (Name != nullptr && Name->getNumOperands() == 1) {
        TmpNameList[i] = createStringFromValue(Name->getOperand(0));

        // Note that looking up the function by name can fail: One of
        // the uses of MetadataExtractor is as part of the
        // RSEmbedInfoPass, which bcc_compat runs sufficiently late in
        // the phase order that RSKernelExpandPass has already run and
        // the original (UNexpanded) kernel function (TmpNameList[i])
        // may have been deleted as having no references (if it has
        // been inlined into the expanded kernel function and is
        // otherwise unreferenced).
        llvm::Function *Func =
            mModule->getFunction(llvm::StringRef(TmpNameList[i]));

        TmpInputCountList[i] = (Func != nullptr) ?
          calculateNumInputs(Func, TmpSigList[i]) : 0;
      }
    }
  } else {
    if (mExportForEachSignatureCount != 1) {
      ALOGE("mExportForEachSignatureCount = %zu, but should be 1",
            mExportForEachSignatureCount);
    }
    char *RootName = new char[5];
    strncpy(RootName, "root", 5);
    TmpNameList[0] = RootName;
  }

  delete [] mExportForEachNameList;
  mExportForEachNameList = TmpNameList.release();

  delete [] mExportForEachSignatureList;
  mExportForEachSignatureList = TmpSigList.release();

  delete [] mExportForEachInputCountList;
  mExportForEachInputCountList = TmpInputCountList.release();

  return true;
}


bool MetadataExtractor::populateReduceMetadata(const llvm::NamedMDNode *ReduceMetadata) {
  mExportReduceCount = 0;
  mExportReduceList = nullptr;

  if (!ReduceMetadata || !(mExportReduceCount = ReduceMetadata->getNumOperands()))
    return true;

  std::unique_ptr<Reduce[]> TmpReduceList(new Reduce[mExportReduceCount]);

  for (size_t i = 0; i < mExportReduceCount; i++) {
    llvm::MDNode *Node = ReduceMetadata->getOperand(i);
    if (!Node || Node->getNumOperands() < 3) {
      ALOGE("Missing reduce metadata");
      return false;
    }

    TmpReduceList[i].mReduceName = createStringFromValue(Node->getOperand(0));

    if (!extractUIntFromMetadataString(&TmpReduceList[i].mAccumulatorDataSize,
                                       Node->getOperand(1))) {
      ALOGE("Non-integer accumulator data size value in reduce metadata");
      return false;
    }

    llvm::MDNode *AccumulatorNode = llvm::dyn_cast<llvm::MDNode>(Node->getOperand(2));
    if (!AccumulatorNode || AccumulatorNode->getNumOperands() != 2) {
      ALOGE("Malformed accumulator node in reduce metadata");
      return false;
    }
    TmpReduceList[i].mAccumulatorName = createStringFromValue(AccumulatorNode->getOperand(0));
    if (!extractUIntFromMetadataString(&TmpReduceList[i].mSignature,
                                       AccumulatorNode->getOperand(1))) {
      ALOGE("Non-integer signature value in reduce metadata");
      return false;
    }
    // Note that looking up the function by name can fail: One of the
    // uses of MetadataExtractor is as part of the RSEmbedInfoPass,
    // which bcc_compat runs sufficiently late in the phase order that
    // RSKernelExpandPass has already run and the original
    // (UNexpanded) accumulator function (mAccumulatorName) may have
    // been deleted as having no references (if it has been inlined
    // into the expanded accumulator function and is otherwise
    // unreferenced).
    llvm::Function *Func =
        mModule->getFunction(llvm::StringRef(TmpReduceList[i].mAccumulatorName));
    // Why calculateNumInputs() - 1?  The "-1" is because we don't
    // want to treat the accumulator argument as an input.
    TmpReduceList[i].mInputCount = (Func ? calculateNumInputs(Func, TmpReduceList[i].mSignature) - 1 : 0);

    TmpReduceList[i].mInitializerName = createStringFromOptionalValue(Node, 3);
    TmpReduceList[i].mCombinerName = createStringFromOptionalValue(Node, 4);
    TmpReduceList[i].mOutConverterName = createStringFromOptionalValue(Node, 5);
    TmpReduceList[i].mHalterName = createStringFromOptionalValue(Node, 6);
  }

  mExportReduceList = TmpReduceList.release();
  return true;
}

void MetadataExtractor::readThreadableFlag(
    const llvm::NamedMDNode *ThreadableMetadata) {

  // Scripts are threadable by default.  If we read a valid metadata value for
  // 'ThreadableMetadataName' and it is set to 'no', we mark script as non
  // threadable.  All other exception paths retain the default value.

  mIsThreadable = true;
  if (ThreadableMetadata == nullptr)
    return;

  llvm::MDNode *mdNode = ThreadableMetadata->getOperand(0);
  if (mdNode == nullptr)
    return;

  llvm::Metadata *mdValue = mdNode->getOperand(0);
  if (mdValue == nullptr)
    return;

  if (getStringOperand(mdValue) == "no")
    mIsThreadable = false;
}

void MetadataExtractor::readBuildChecksumMetadata(
    const llvm::NamedMDNode *ChecksumMetadata) {

  if (ChecksumMetadata == nullptr)
    return;

  llvm::MDNode *mdNode = ChecksumMetadata->getOperand(0);
  if (mdNode == nullptr)
    return;

  llvm::Metadata *mdValue = mdNode->getOperand(0);
  if (mdValue == nullptr)
    return;

  mBuildChecksum = createStringFromValue(mdValue);
}

bool MetadataExtractor::extract() {
  if (!(mBitcode && mBitcodeSize) && !mModule) {
    ALOGE("Invalid/empty bitcode/module");
    return false;
  }

  std::unique_ptr<llvm::LLVMContext> mContext;
  bool shouldNullModule = false;

  if (!mModule) {
    mContext.reset(new llvm::LLVMContext());
    std::unique_ptr<llvm::MemoryBuffer> MEM(
      llvm::MemoryBuffer::getMemBuffer(
        llvm::StringRef(mBitcode, mBitcodeSize), "", false));
    std::string error;

    llvm::ErrorOr<std::unique_ptr<llvm::Module> > errval =
        llvm::parseBitcodeFile(MEM.get()->getMemBufferRef(), *mContext);
    if (std::error_code ec = errval.getError()) {
        ALOGE("Could not parse bitcode file");
        ALOGE("%s", ec.message().c_str());
        return false;
    }

    mModule = errval.get().release();
    shouldNullModule = true;
  }

  const llvm::NamedMDNode *ExportVarMetadata =
      mModule->getNamedMetadata(ExportVarMetadataName);
  const llvm::NamedMDNode *ExportFuncMetadata =
      mModule->getNamedMetadata(ExportFuncMetadataName);
  const llvm::NamedMDNode *ExportForEachNameMetadata =
      mModule->getNamedMetadata(ExportForEachNameMetadataName);
  const llvm::NamedMDNode *ExportForEachMetadata =
      mModule->getNamedMetadata(ExportForEachMetadataName);
  const llvm::NamedMDNode *ExportReduceMetadata =
      mModule->getNamedMetadata(ExportReduceMetadataName);
  const llvm::NamedMDNode *PragmaMetadata =
      mModule->getNamedMetadata(PragmaMetadataName);
  const llvm::NamedMDNode *ObjectSlotMetadata =
      mModule->getNamedMetadata(ObjectSlotMetadataName);
  const llvm::NamedMDNode *ThreadableMetadata =
      mModule->getNamedMetadata(ThreadableMetadataName);
  const llvm::NamedMDNode *ChecksumMetadata =
      mModule->getNamedMetadata(ChecksumMetadataName);
  const llvm::NamedMDNode *DebugInfoMetadata =
      mModule->getNamedMetadata(DebugInfoMetadataName);

  if (!populateNameMetadata(ExportVarMetadata, mExportVarNameList,
                            mExportVarCount)) {
    ALOGE("Could not populate export variable metadata");
    goto err;
  }

  if (!populateNameMetadata(ExportFuncMetadata, mExportFuncNameList,
                            mExportFuncCount)) {
    ALOGE("Could not populate export function metadata");
    goto err;
  }

  if (!populateForEachMetadata(ExportForEachNameMetadata,
                               ExportForEachMetadata)) {
    ALOGE("Could not populate ForEach signature metadata");
    goto err;
  }

  if (!populateReduceMetadata(ExportReduceMetadata)) {
    ALOGE("Could not populate export general reduction metadata");
    goto err;
  }

  populatePragmaMetadata(PragmaMetadata);

  if (!populateObjectSlotMetadata(ObjectSlotMetadata)) {
    ALOGE("Could not populate object slot metadata");
    goto err;
  }

  readThreadableFlag(ThreadableMetadata);
  readBuildChecksumMetadata(ChecksumMetadata);

  mHasDebugInfo = DebugInfoMetadata != nullptr;

  if (shouldNullModule) {
    mModule = nullptr;
  }
  return true;

err:
  if (shouldNullModule) {
    mModule = nullptr;
  }
  return false;
}

}  // namespace bcinfo
