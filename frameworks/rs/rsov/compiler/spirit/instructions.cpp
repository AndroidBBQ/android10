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

#include "instructions.h"

#include <iostream>

#include "visitor.h"

namespace android {
namespace spirit {

IdRef::IdRef(Instruction *inst) : mId(inst->getId()), mInstruction(inst) {}

void Instruction::accept(IVisitor *v) { v->visit(this); }

DecorateInst *Instruction::decorate(Decoration decor) {
  if (mCodeAndCount.mOpCode == OpDecorate) {
    return nullptr;
  }

  DecorateInst *decorInst = new DecorateInst(this, decor);

  mDecorations.push_back(decorInst);

  return decorInst;
}

MemberDecorateInst *Instruction::memberDecorate(int member, Decoration decor) {
  if (mCodeAndCount.mOpCode != OpTypeStruct) {
    std::cerr << "OpMemberDecorate applied to non-OpTypeStruct instructions\n";
    return nullptr;
  }

  MemberDecorateInst *decorInst = new MemberDecorateInst(this, member, decor);

  mDecorations.push_back(decorInst);

  return decorInst;
}

} // namespace spirit
} // namespace android
