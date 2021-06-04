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

#include "builder.h"

#include "module.h"

namespace android {
namespace spirit {

Module *Builder::MakeModule() {
  Module *ret = new Module(this);
  ret->initialize();
  return ret;
}

EntryPointDefinition *
Builder::MakeEntryPointDefinition(ExecutionModel execModel,
                                  FunctionDefinition *func, const char *name) {
  return new EntryPointDefinition(this, execModel, func, name);
}

DebugInfoSection *Builder::MakeDebugInfoSection() {
  return new DebugInfoSection(this);
}

FunctionDefinition *
Builder::MakeFunctionDefinition(Instruction *retType, FunctionControl ctrl,
                                TypeFunctionInst *funcType) {
  FunctionInst *func = MakeFunction(retType, ctrl, funcType);
  FunctionEndInst *end = MakeFunctionEnd();
  return new FunctionDefinition(this, func, end);
}

Block *Builder::MakeBlock() { return new Block(this); }

} // namespace spirit
} // namespace android
