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

#ifndef CORE_DEFS_H
#define CORE_DEFS_H

#include <string>

namespace android {
namespace spirit {

class Instruction;

typedef int32_t LiteralInteger;
typedef std::string LiteralString;
typedef union {
  int32_t intValue;
  int64_t longValue;
  float floatValue;
  double doubleValue;
} LiteralContextDependentNumber;
typedef uint32_t LiteralExtInstInteger;
typedef uint32_t LiteralSpecConstantOpInteger;
typedef uint32_t IdResult;

struct IdRef {
  IdRef() : mId(0), mInstruction(nullptr) {}
  IdRef(Instruction *inst);

  uint32_t mId;
  mutable Instruction *mInstruction;
};

// TODO: specialize these ref types
// TODO: should only reference type instructions
struct IdResultType : public IdRef {
  IdResultType() : IdRef() {}
  IdResultType(Instruction *inst) : IdRef(inst) {}
};

// TODO: should only reference int representing memory semeantics
struct IdMemorySemantics : public IdRef {};
// TODO: should only reference int representing scopes
struct IdScope : public IdRef {};

struct OpCodeAndWordCount {
  OpCodeAndWordCount() : mOpCode(0) {}
  OpCodeAndWordCount(uint32_t codeAndCount)
      : mOpCode((uint16_t)codeAndCount),
        mWordCount((uint32_t)(codeAndCount >> 16)) {}
  OpCodeAndWordCount(uint32_t opcode, uint32_t wordCount)
      : mOpCode((uint16_t)opcode), mWordCount((uint16_t)wordCount) {}

  operator uint32_t() const {
    return ((uint32_t)mWordCount << 16) | (uint32_t)mOpCode;
  }

  uint16_t mOpCode;
  uint16_t mWordCount;
};

} // namespace spirit
} // namespace android

#endif // CORE_DEFS_H
