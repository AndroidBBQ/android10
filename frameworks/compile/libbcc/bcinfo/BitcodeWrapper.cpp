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

#include "bcinfo/BitcodeWrapper.h"
#include "bcinfo/Wrap/bitcode_wrapperer.h"
#include "bcinfo/Wrap/in_memory_wrapper_input.h"

#include "llvm/Bitcode/ReaderWriter.h"

#include <cstdlib>
#include <cstring>

namespace bcinfo {

BitcodeWrapper::BitcodeWrapper(const char *bitcode, size_t bitcodeSize)
    : mFileType(BC_NOT_BC), mBitcode(bitcode),
      mBitcodeSize(bitcodeSize),
      mHeaderVersion(0), mTargetAPI(0), mCompilerVersion(0),
      mOptimizationLevel(3) {
  InMemoryWrapperInput inMem(mBitcode, mBitcodeSize);
  BitcodeWrapperer wrapperer(&inMem, nullptr);
  if (wrapperer.IsInputBitcodeWrapper()) {
    mFileType = BC_WRAPPER;
    mHeaderVersion = wrapperer.getAndroidHeaderVersion();
    mTargetAPI = wrapperer.getAndroidTargetAPI();
    mCompilerVersion = wrapperer.getAndroidCompilerVersion();
    mOptimizationLevel = wrapperer.getAndroidOptimizationLevel();
  } else if (wrapperer.IsInputBitcodeFile()) {
    mFileType = BC_RAW;
  }
}


BitcodeWrapper::~BitcodeWrapper() {
  return;
}


bool BitcodeWrapper::unwrap() {
  return mFileType != BC_NOT_BC;
}

}  // namespace bcinfo

