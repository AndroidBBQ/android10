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

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>

#include <iostream>
#include <string>
#include <vector>

#include "core_defs.h"
#include "entity.h"
#include "opcodes_generated.h"
#include "types_generated.h"
#include "visitor.h"
#include "word_stream.h"

namespace android {
namespace spirit {

// Word count for a serialized operand
template <typename T> uint16_t WordCount(T) { return 1; }

inline uint16_t WordCount(PairLiteralIntegerIdRef) { return 2; }
inline uint16_t WordCount(PairIdRefLiteralInteger) { return 2; }
inline uint16_t WordCount(PairIdRefIdRef) { return 2; }
inline uint16_t WordCount(const std::string &operand) {
  return operand.length() / 4 + 1;
}

class Instruction : public Entity {
public:
  Instruction(uint32_t opCode) : mCodeAndCount(opCode) {}
  Instruction(uint32_t opCode, uint32_t fixedWordCount)
      : mCodeAndCount(opCode), mFixedWordCount(fixedWordCount) {}
  virtual ~Instruction() {}

  void accept(IVisitor *v) override;

  void setWordCount() const {
    if (mCodeAndCount.mWordCount == 0) {
      mCodeAndCount.mWordCount = getWordCount();
    }
  }
  virtual uint16_t getWordCount() const = 0;
  virtual bool hasResult() const = 0;
  virtual IdResult getId() const = 0;
  virtual void setId(IdResult) = 0;
  virtual std::vector<const IdRef *> getAllIdRefs() const = 0;

  Instruction *addExtraOperand(uint32_t word) {
    mExtraOperands.push_back(word);
    return this;
  }

  // Adds decoration to the current instruction.
  // Returns: the result OpDecorate instruction
  DecorateInst *decorate(Decoration);
  MemberDecorateInst *memberDecorate(int member, Decoration);

  bool DeserializeFirstWord(InputWordStream &IS, OpCode opcode) {
    if (IS.empty()) {
      return false;
    }

    OpCodeAndWordCount codeAndCount(*IS);

    if (codeAndCount.mOpCode != opcode) {
      return false;
    }

    mRemainingWordCount = codeAndCount.mWordCount;

    IS >> &mCodeAndCount;

    mRemainingWordCount--;

    return true;
  }

  template <typename T>
  bool DeserializeExactlyOne(InputWordStream &IS, T *operand) {
    if (IS.empty()) {
      return false;
    }

    IS >> operand;

    mRemainingWordCount -= WordCount(*operand);

    return true;
  }

  template <typename T>
  bool DeserializeOptionallyOne(InputWordStream &IS, T **operand) {
    if (mRemainingWordCount == 0) {
      return true;
    }
    *operand = new T();
    return DeserializeExactlyOne(IS, *operand);
  }

  template <typename T>
  bool DeserializeZeroOrMoreOperands(InputWordStream &IS,
                                     std::vector<T> *operands) {
    while (mRemainingWordCount > 0) {
      T tmp;
      if (!DeserializeExactlyOne(IS, &tmp)) {
        return false;
      }
      operands->push_back(tmp);
    }
    return true;
  }

  bool DeserializeExtraOperands(InputWordStream &IS) {
    return DeserializeZeroOrMoreOperands(IS, &mExtraOperands);
  }

  void SerializeExtraOperands(OutputWordStream &OS) const {
    for (uint32_t w : mExtraOperands) {
      OS << w;
    }
  }

  const std::vector<Instruction *> &getAnnotations() const {
    return mDecorations;
  }

  uint16_t getOpCode() const { return mCodeAndCount.mOpCode; }

  mutable OpCodeAndWordCount mCodeAndCount;
  uint16_t mFixedWordCount;
  uint16_t mRemainingWordCount;
  std::vector<uint32_t> mExtraOperands;
  std::vector<Instruction *> mDecorations;
};

} // namespace spirit
} // namespace android

#include "instructions_generated.h"

#endif // INSTRUCTIONS_H
