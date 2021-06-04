/*
 * Copyright 2013, The Android Open Source Project
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

#ifndef BCC_RS_SCRIPT_GROUP_FUSION_H
#define BCC_RS_SCRIPT_GROUP_FUSION_H

#include <vector>
#include <string>

namespace llvm {
class Module;
}

namespace bcc {

class Source;
class BCCContext;

/// @brief Fuse kernels
///
/// @param Context bcc context.
/// @param sources The Sources containing the kernels.
/// @param slots The slots where the kernels are located.
/// @param fusedName
/// @return True, if kernels are successfully fused. False, otherwise. It's up to
/// the caller on how to deal with unsuccessful fusion. A script group can
/// execute with either fused kernels or individual kernels.
bool fuseKernels(BCCContext& Context,
                 const std::vector<Source *>& sources,
                 const std::vector<int>& slots,
                 const std::string& fusedName,
                 llvm::Module* mergedModule);

bool renameInvoke(BCCContext& Context, const Source* source, const int slot,
                  const std::string& newName, llvm::Module* mergedModule);
}

#endif /* BCC_RS_SCRIPT_GROUP_FUSION_H */
