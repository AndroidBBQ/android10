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

#ifndef WRAPPER_H
#define WRAPPER_H

#include <stdint.h>

namespace llvm {
class Module;
}

namespace android {
namespace spirit {

class Builder;
class Instruction;
class Module;
class Pass;
class VariableInst;

// TODO: avoid exposing these methods while still unit testing them
void AddHeader(Module *m);
VariableInst *AddBuffer(Instruction *elementType, uint32_t binding, Builder &b,
                        Module *m);

bool AddWrapper(const char *name, const uint32_t signature,
                const uint32_t numInput, Builder &b, Module *m);

// Find the LLVM generated buffer for all non-allocation glboal variables, and
// correctly decorate it and its type with annotations for binding numbers, etc.
bool DecorateGlobalBuffer(llvm::Module &M, Builder &b, Module *m);

} // namespace spirit
} // namespace android

namespace rs2spirv {

android::spirit::Pass* CreateWrapperPass(const llvm::Module &LLVMModule);

} // namespace rs2spirv

#endif
