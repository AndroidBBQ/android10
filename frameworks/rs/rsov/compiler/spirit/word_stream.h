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

#ifndef WORD_STREAM_H
#define WORD_STREAM_H

#include "core_defs.h"
#include "types_generated.h"

#include <stdint.h>

#include <string>
#include <vector>

namespace android {
namespace spirit {

struct IdRef;
class Instruction;

class InputWordStream {
public:
  static InputWordStream *Create();
  static InputWordStream *Create(std::vector<uint32_t> &&words);
  static InputWordStream *Create(const std::vector<uint32_t> &words);
  static InputWordStream *Create(const std::vector<uint8_t> &bytes);
  static InputWordStream *Create(const char *fileName);

  virtual ~InputWordStream() {}

  virtual bool empty() const = 0;
  virtual uint32_t operator*() = 0;

  virtual InputWordStream &operator>>(uint32_t *RHS) = 0;
  virtual InputWordStream &operator>>(LiteralContextDependentNumber *num) = 0;
  virtual InputWordStream &operator>>(std::string *str) = 0;

  InputWordStream &operator>>(int32_t *RHS) { return *this >> (uint32_t *)RHS; }

  InputWordStream &operator>>(OpCodeAndWordCount *codeCount) {
    uint32_t word;
    *this >> &word;
    *codeCount = word;
    return *this;
  }

  InputWordStream &operator>>(IdRef *RHS) {
    // The referred instruction will be resolved later towards the end of the
    // deserialization of the module after all instructions have been
    // deserialized.
    // It cannot be resolved here because it may be a forward reference.
    RHS->mInstruction = nullptr;
    return *this >> &RHS->mId;
    ;
  }

  InputWordStream &operator>>(PairLiteralIntegerIdRef *RHS) {
    return *this >> &RHS->mField0 >> &RHS->mField1;
  }

  InputWordStream &operator>>(PairIdRefLiteralInteger *RHS) {
    return *this >> &RHS->mField0 >> &RHS->mField1;
  }

  InputWordStream &operator>>(PairIdRefIdRef *RHS) {
    return *this >> &RHS->mField0 >> &RHS->mField1;
  }

#define HANDLE_ENUM(Enum)                                                      \
  InputWordStream &operator>>(Enum *RHS) { return *this >> (uint32_t *)RHS; }
#include "enum_dispatches_generated.h"
#undef HANDLE_ENUM
};

class OutputWordStream {
public:
  static OutputWordStream *Create();

  virtual ~OutputWordStream() {}

  virtual std::vector<uint32_t> getWords() = 0;

  virtual OutputWordStream &operator<<(const uint32_t RHS) = 0;
  virtual OutputWordStream &
  operator<<(const LiteralContextDependentNumber &RHS) = 0;
  virtual OutputWordStream &operator<<(const std::string &str) = 0;

  OutputWordStream &operator<<(const int32_t RHS) {
    return *this << (uint32_t)RHS;
  }

  OutputWordStream &operator<<(const OpCodeAndWordCount codeCount) {
    return *this << (uint32_t)codeCount;
  }

  OutputWordStream &operator<<(const IdRef &RHS) {
    return *this << RHS.mId;
  }

  OutputWordStream &operator<<(const PairLiteralIntegerIdRef &RHS) {
    return *this << RHS.mField0 << RHS.mField1;
  }

  OutputWordStream &operator<<(const PairIdRefLiteralInteger &RHS) {
    return *this << RHS.mField0 << RHS.mField1;
  }

  OutputWordStream &operator<<(const PairIdRefIdRef &RHS) {
    return *this << RHS.mField0 << RHS.mField1;
  }

#define HANDLE_ENUM(Enum)                                                      \
  OutputWordStream &operator<<(const Enum RHS) {                               \
    return *this << static_cast<uint32_t>(RHS);                                \
  }
#include "enum_dispatches_generated.h"
#undef HANDLE_ENUM
};

class WordStream : public InputWordStream, public OutputWordStream {
public:
  static WordStream *Create();
  static WordStream *Create(const std::vector<uint32_t> &words);

  virtual ~WordStream() {}
};

} // namespace spirit
} // namespace android

#endif // WORD_STREAM_H
