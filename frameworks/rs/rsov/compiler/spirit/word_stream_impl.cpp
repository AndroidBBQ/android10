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

#include "word_stream_impl.h"

namespace android {
namespace spirit {

WordStreamImpl::WordStreamImpl() {}

WordStreamImpl::WordStreamImpl(const std::vector<uint32_t> &words)
    : mWords(words), mIter(mWords.begin()) {}

WordStreamImpl::WordStreamImpl(std::vector<uint32_t> &&words)
    : mWords(words), mIter(mWords.begin()) {}

WordStreamImpl &WordStreamImpl::operator<<(const std::string &str) {
  const size_t len = str.length();
  const uint32_t *begin = (uint32_t *)str.c_str();
  const uint32_t *end = begin + (len / 4);
  mWords.insert(mWords.end(), begin, end);

  uint32_t lastWord = *end;
  uint32_t mask = 0xFF;
  bool clear = false;
  for (int i = 0; i < 4; i++, mask <<= 8) {
    if (clear) {
      lastWord &= ~mask;
    } else {
      clear = ((lastWord & mask) == 0);
    }
  }
  mWords.push_back(lastWord);
  return *this;
}

WordStreamImpl &WordStreamImpl::operator>>(std::string *str) {
  const char *s = (const char *)&*mIter;
  str->assign(s);
  while (*mIter++ & 0xFF000000) {
  }
  return *this;
}

} // namespace spirit
} // namespace android
