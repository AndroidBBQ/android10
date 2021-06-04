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

#ifndef WORD_STREAM_IMPL_H
#define WORD_STREAM_IMPL_H

#include "word_stream.h"

#include <string>

namespace android {
namespace spirit {

class WordStreamImpl : public WordStream {
public:
  WordStreamImpl();
  WordStreamImpl(const std::vector<uint32_t> &words);
  WordStreamImpl(std::vector<uint32_t> &&words);

  bool empty() const override {
    return mWords.empty() || mIter == mWords.end();
  }

  uint32_t operator*() { return *mIter; }

  WordStreamImpl &operator>>(uint32_t *RHS) override {
    *RHS = *mIter++;
    return *this;
  }

  WordStreamImpl &operator>>(LiteralContextDependentNumber *RHS) override {
    // TODO: check context in the instruction class to decide the actual size.
    return *this >> (uint32_t *)(&RHS->intValue);
  }

  WordStreamImpl &operator>>(std::string *str) override;

  std::vector<uint32_t> getWords() override { return mWords; }

  WordStreamImpl &operator<<(const uint32_t RHS) override {
    mWords.push_back(RHS);
    return *this;
  }

  WordStreamImpl &
  operator<<(const LiteralContextDependentNumber &RHS) override {
    // TODO: check context in the instruction class to decide the actual size.
    // TODO: maybe a word stream class should never take a context dependent
    // type as argument. Always take concrete types such as int32, int64, float,
    // double, etc.
    return *this << (uint32_t)(RHS.intValue);
  }
  WordStreamImpl &operator<<(const std::string &str) override;

private:
  std::vector<uint32_t> mWords;
  std::vector<uint32_t>::const_iterator mIter;
};

} // namespace spirit
} // namespace android

#endif // WORD_STREAM_IMPL_H
