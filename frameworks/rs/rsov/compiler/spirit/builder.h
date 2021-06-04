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

#ifndef BUILDER_H
#define BUILDER_H

#include "instructions.h"
#include "module.h"

namespace android {
namespace spirit {

class Builder {
public:
  Module *MakeModule();
  EntryPointDefinition *MakeEntryPointDefinition(ExecutionModel execModel,
                                                 FunctionDefinition *func,
                                                 const char *name);
  DebugInfoSection *MakeDebugInfoSection();
  FunctionDefinition *MakeFunctionDefinition(Instruction *retType,
                                             FunctionControl ctrl,
                                             TypeFunctionInst *funcType);
  Block *MakeBlock();

#include "factory_methods_generated.h"
};

} // namespace spirit
} // namespace android

#endif // BUILDER_H
