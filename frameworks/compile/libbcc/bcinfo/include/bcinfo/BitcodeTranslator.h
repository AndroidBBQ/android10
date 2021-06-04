/*
 * Copyright 2011, The Android Open Source Project
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

#ifndef __ANDROID_BCINFO_BITCODETRANSLATOR_H__
#define __ANDROID_BCINFO_BITCODETRANSLATOR_H__

#include <cstddef>

namespace bcinfo {

class BitcodeTranslator {
 private:
  const char *mBitcode;
  size_t mBitcodeSize;
  const char *mTranslatedBitcode;
  size_t mTranslatedBitcodeSize;
  unsigned int mVersion;

 public:
  /**
   * Translates \p bitcode of a particular \p version to the latest version.
   *
   * \param bitcode - input bitcode string.
   * \param bitcodeSize - length of \p bitcode string (in bytes).
   * \param version - corresponding target SDK version of \p bitcode.
   */
  BitcodeTranslator(const char *bitcode, size_t bitcodeSize,
                    unsigned int version);

  ~BitcodeTranslator();

  /**
   * Translate the supplied bitcode to the latest supported version.
   *
   * \return true if the bitcode was translated successfully and false if an
   *         error occurred.
   */
  bool translate();

  /**
   * \return translated bitcode.
   */
  const char *getTranslatedBitcode() const {
    return mTranslatedBitcode;
  }

  /**
   * \return size of the translated bitcode (in bytes).
   */
  size_t getTranslatedBitcodeSize() const {
    return mTranslatedBitcodeSize;
  }
};

}  // namespace bcinfo

#endif  // __ANDROID_BCINFO_BITCODETRANSLATOR_H__
