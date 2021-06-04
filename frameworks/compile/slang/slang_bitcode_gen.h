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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_BITCODE_GEN_H_  // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_BITCODE_GEN_H_

#include <cstdint>

namespace llvm {
  class raw_ostream;
  class Module;
}

namespace slang {

// Write out the LLVM bitcode for a module, encased in a wrapper
// containing RS version information.
void writeBitcode(llvm::raw_ostream &Out,
                  const llvm::Module &M,
                  uint32_t TargetAPI,
                  uint32_t OptimizationLevel,
                  bool GenerateDebugInfo);

} // end namespace slang

#endif  // _FRAMEWORKS_COMPILE_SLANG_SLANG_BITCODE_GEN_H_  NOLINT
