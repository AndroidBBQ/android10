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

#include "bcinfo/BitcodeTranslator.h"

#include "bcinfo/BitcodeWrapper.h"

#include "BitReader_2_7/BitReader_2_7.h"
#include "BitReader_3_0/BitReader_3_0.h"

#include "BitWriter_3_2/ReaderWriter_3_2.h"

#include "StripUnkAttr/strip_unknown_attributes.h"

#define LOG_TAG "bcinfo"
#include <log/log.h>

#include "llvm/Bitcode/BitstreamWriter.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdlib>
#include <climits>

namespace bcinfo {

/**
 * Define minimum and maximum target API versions. These correspond to the
 * same API levels used by the standard Android SDK.
 *
 * LLVM 2.7
 *  11 - Honeycomb
 *  12 - Honeycomb MR1
 *  13 - Honeycomb MR2
 *
 * LLVM 3.0
 *  14 - Ice Cream Sandwich
 *  15 - Ice Cream Sandwich MR1
 *
 * LLVM 3.1
 *  16 - Ice Cream Sandwich MR2
 */
static const unsigned int kMinimumAPIVersion     = 11;
static const unsigned int kMaximumAPIVersion     = RS_VERSION;
static const unsigned int kCurrentAPIVersion     = 10000;
static const unsigned int kDevelopmentAPIVersion = UINT_MAX;

/**
 * The minimum version which does not require translation (i.e. is already
 * compatible with LLVM's default bitcode reader).
 */
static const unsigned int kMinimumUntranslatedVersion = 16;
static const unsigned int kMinimumCompatibleVersion_LLVM_3_0 = 14;
static const unsigned int kMinimumCompatibleVersion_LLVM_2_7 = 11;


static void stripUnknownAttributes(llvm::Module *M) {
  for (llvm::Function &F : *M)
    slang::stripUnknownAttributes(F);
}

BitcodeTranslator::BitcodeTranslator(const char *bitcode, size_t bitcodeSize,
                                     unsigned int version)
    : mBitcode(bitcode), mBitcodeSize(bitcodeSize), mTranslatedBitcode(nullptr),
      mTranslatedBitcodeSize(0), mVersion(version) {
  return;
}


BitcodeTranslator::~BitcodeTranslator() {
  if (mVersion < kMinimumUntranslatedVersion) {
    // We didn't actually do a translation in the alternate case, so deleting
    // the bitcode would be improper.
    delete [] mTranslatedBitcode;
  }
  mTranslatedBitcode = nullptr;
  return;
}


bool BitcodeTranslator::translate() {
  if (!mBitcode || !mBitcodeSize) {
    ALOGE("Invalid/empty bitcode");
    return false;
  }

  BitcodeWrapper BCWrapper(mBitcode, mBitcodeSize);
  if (BCWrapper.getTargetAPI() != mVersion) {
    ALOGE("Bitcode wrapper (%u) and translator (%u) disagree about target API",
          BCWrapper.getTargetAPI(), mVersion);
  }

  if ((mVersion != kDevelopmentAPIVersion) &&
      (mVersion != kCurrentAPIVersion)     &&
       ((mVersion < kMinimumAPIVersion) ||
        (mVersion > kMaximumAPIVersion))) {
    ALOGE("Invalid API version: %u is out of range ('%u' - '%u')", mVersion,
         kMinimumAPIVersion, kMaximumAPIVersion);
    return false;
  }

  // We currently don't need to transcode any API version higher than 14 or
  // the current API version (i.e. 10000)
  if (mVersion >= kMinimumUntranslatedVersion) {
    mTranslatedBitcode = mBitcode;
    mTranslatedBitcodeSize = mBitcodeSize;
    return true;
  }

  // Do the actual transcoding by invoking a 2.7-era bitcode reader that can
  // then write the bitcode back out in a more modern (acceptable) version.
  std::unique_ptr<llvm::LLVMContext> mContext(new llvm::LLVMContext());
  std::unique_ptr<llvm::MemoryBuffer> MEM(
    llvm::MemoryBuffer::getMemBuffer(
      llvm::StringRef(mBitcode, mBitcodeSize), "", false));
  std::string error;
  llvm::ErrorOr<llvm::MemoryBufferRef> MBOrErr = MEM->getMemBufferRef();

  llvm::ErrorOr<llvm::Module *> MOrErr(nullptr);

  if (mVersion >= kMinimumCompatibleVersion_LLVM_3_0) {
    MOrErr = llvm_3_0::parseBitcodeFile(*MBOrErr, *mContext);
  } else if (mVersion >= kMinimumCompatibleVersion_LLVM_2_7) {
    MOrErr = llvm_2_7::parseBitcodeFile(*MBOrErr, *mContext);
  } else {
    ALOGE("No compatible bitcode reader for API version %d", mVersion);
    return false;
  }

  if (std::error_code EC = MOrErr.getError()) {
    ALOGE("Could not parse bitcode file");
    ALOGE("%s", EC.message().c_str());
    return false;
  }

  // Module ownership is handled by the context, so we don't need to free it.
  llvm::Module *module = MOrErr.get();

  stripUnknownAttributes(module);

  std::string Buffer;

  llvm::raw_string_ostream OS(Buffer);
  // Use the LLVM 3.2 bitcode writer, instead of the top-of-tree version.
  llvm_3_2::WriteBitcodeToFile(module, OS);
  OS.flush();

  AndroidBitcodeWrapper wrapper;
  size_t actualWrapperLen = writeAndroidBitcodeWrapper(
      &wrapper, Buffer.size(), kMinimumUntranslatedVersion,
      BCWrapper.getCompilerVersion(), BCWrapper.getOptimizationLevel());
  if (!actualWrapperLen) {
    ALOGE("Couldn't produce bitcode wrapper!");
    return false;
  }

  mTranslatedBitcodeSize = actualWrapperLen + Buffer.size();
  char *c = new char[mTranslatedBitcodeSize];
  memcpy(c, &wrapper, actualWrapperLen);
  memcpy(c + actualWrapperLen, Buffer.c_str(), Buffer.size());

  mTranslatedBitcode = c;

  return true;
}

}  // namespace bcinfo
