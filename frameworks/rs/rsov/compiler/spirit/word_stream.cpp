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

#include "word_stream.h"

#include "file_utils.h"
#include "word_stream_impl.h"

namespace android {
namespace spirit {

InputWordStream *InputWordStream::Create() { return new WordStreamImpl(); }

InputWordStream *InputWordStream::Create(std::vector<uint32_t> &&words) {
  return new WordStreamImpl(words);
}

InputWordStream *InputWordStream::Create(const std::vector<uint32_t> &words) {
  return new WordStreamImpl(words);
}

InputWordStream *InputWordStream::Create(const std::vector<uint8_t> &bytes) {
  std::vector<uint32_t> words((uint32_t *)bytes.data(),
                              (uint32_t *)(bytes.data() + bytes.size()));
  return InputWordStream::Create(words);
}

InputWordStream *InputWordStream::Create(const char *filePath) {
  return InputWordStream::Create(readFile<uint32_t>(filePath));
}

OutputWordStream *OutputWordStream::Create() { return new WordStreamImpl(); }

WordStream *WordStream::Create() { return new WordStreamImpl(); }

WordStream *WordStream::Create(const std::vector<uint32_t> &words) {
  return new WordStreamImpl(words);
}

} // namespace spirit
} // namespace android
