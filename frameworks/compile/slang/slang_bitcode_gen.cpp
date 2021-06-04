/*
 * Copyright 2015, The Android Open Source Project
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

#include "bcinfo/BitcodeWrapper.h"

#include "llvm/Support/raw_ostream.h"

#include "BitWriter_2_9/ReaderWriter_2_9.h"
#include "BitWriter_2_9_func/ReaderWriter_2_9_func.h"
#include "BitWriter_3_2/ReaderWriter_3_2.h"

#include "slang_assert.h"
#include "slang_bitcode_gen.h"
#include "slang_version.h"
#include "llvm/Bitcode/ReaderWriter.h"

namespace slang {

void writeBitcode(llvm::raw_ostream &Out,
                  const llvm::Module &M,
                  uint32_t TargetAPI,
                  uint32_t OptimizationLevel,
                  bool GenerateDebugInfo) {
  std::string BitcodeStr;
  llvm::raw_string_ostream Bitcode(BitcodeStr);

  // The older bitcode writers will produce invalid bitcode if the -g
  // flag is set using WriteBitcodeToFile. As such we use the ToT writer
  // when -g is set. However, this will produce a bitcode file linked to
  // this version of LLVM as the debug info format can change between
  // versions.
  // If bcc receives a bitcode file with a format of debug info
  // which is either ahead or behind the version it is expecting it will
  // fail the verification stage. Failing this stage results in the bitcode
  // loader returning null and the compiler will terminate abruptly. Bitcode
  // files with debug info are as such only capable of targeting devices with
  // LLVM libraries with the same debug info version as the version of slang
  // which was used to compile the file. This is due to the fact that LLVM
  // offers no backwards or forwards compatibility guarantee for its debug
  // bitcode. At the moment the only practical guarantee which can be made
  // is that the debug bitcode emitted by any slang will work with the bcc
  // version which was newest at the time when llvm-rs-cc was built.
  if (GenerateDebugInfo) {
    llvm::WriteBitcodeToFile(&M, Bitcode);
  } else {
    // Create the bitcode.
    switch (TargetAPI) {
    case SLANG_HC_TARGET_API:
    case SLANG_HC_MR1_TARGET_API:
    case SLANG_HC_MR2_TARGET_API: {
      // Pre-ICS targets must use the LLVM 2.9 BitcodeWriter
      llvm_2_9::WriteBitcodeToFile(&M, Bitcode);
      break;
    }
    case SLANG_ICS_TARGET_API:
    case SLANG_ICS_MR1_TARGET_API: {
      // ICS targets must use the LLVM 2.9_func BitcodeWriter
      llvm_2_9_func::WriteBitcodeToFile(&M, Bitcode);
      break;
    }
    default: {
      if (TargetAPI != SLANG_DEVELOPMENT_TARGET_API &&
          (TargetAPI < SLANG_MINIMUM_TARGET_API ||
           TargetAPI > SLANG_MAXIMUM_TARGET_API)) {
        slangAssert(false && "Invalid target API value");
      }
      // Switch to the 3.2 BitcodeWriter by default, and don't use
      // LLVM's included BitcodeWriter at all (for now).
      llvm_3_2::WriteBitcodeToFile(&M, Bitcode);
      break;
    }
    }
  }

  const uint32_t CompilerVersion = SlangVersion::CURRENT;

  // Create the bitcode wrapper.
  bcinfo::AndroidBitcodeWrapper Wrapper;
  size_t ActualWrapperLen = bcinfo::writeAndroidBitcodeWrapper(
        &Wrapper, Bitcode.str().length(), TargetAPI,
        CompilerVersion, OptimizationLevel);

  slangAssert(ActualWrapperLen > 0);

  // Write out the file.
  Out.write(reinterpret_cast<char*>(&Wrapper), ActualWrapperLen);
  Out << Bitcode.str();
}

}  // namespace slang
