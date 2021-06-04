/*
 * Copyright 2011-2012, The Android Open Source Project
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

#ifndef __ANDROID_BCINFO_BITCODEWRAPPER_H__
#define __ANDROID_BCINFO_BITCODEWRAPPER_H__

#include "bcinfo/Wrap/BCHeaderField.h"

#include <cstddef>
#include <stdint.h>

namespace bcinfo {

struct AndroidBitcodeWrapper {
  uint32_t Magic;
  uint32_t Version;
  uint32_t BitcodeOffset;
  uint32_t BitcodeSize;
  uint32_t HeaderVersion;
  uint32_t TargetAPI;
  uint32_t PNaClVersion;
  uint16_t CompilerVersionTag;
  uint16_t CompilerVersionLen;
  uint32_t CompilerVersion;
  uint16_t OptimizationLevelTag;
  uint16_t OptimizationLevelLen;
  uint32_t OptimizationLevel;
};

enum BCFileType {
  BC_NOT_BC = 0,
  BC_WRAPPER = 1,
  BC_RAW = 2
};

class BitcodeWrapper {
 private:
  enum BCFileType mFileType;
  const char *mBitcode;
  size_t mBitcodeSize;

  uint32_t mHeaderVersion;
  uint32_t mTargetAPI;
  uint32_t mCompilerVersion;
  uint32_t mOptimizationLevel;

 public:
  /**
   * Reads wrapper information from \p bitcode.
   *
   * \param bitcode - input bitcode string.
   * \param bitcodeSize - length of \p bitcode string (in bytes).
   */
  BitcodeWrapper(const char *bitcode, size_t bitcodeSize);

  ~BitcodeWrapper();

  /**
   * Attempt to unwrap the target bitcode. This function is \deprecated.
   *
   * \return true on success and false if an error occurred.
   */
  bool unwrap();

  /**
   * \return type of bitcode file.
   */
  enum BCFileType getBCFileType() const {
    return mFileType;
  }

  /**
   * \return header version of bitcode wrapper.
   */
  uint32_t getHeaderVersion() const {
    return mHeaderVersion;
  }

  /**
   * \return target API version for this bitcode.
   */
  uint32_t getTargetAPI() const {
    return mTargetAPI;
  }

  /**
   * \return compiler version that generated this bitcode.
   */
  uint32_t getCompilerVersion() const {
    return mCompilerVersion;
  }

  /**
   * \return compiler optimization level for this bitcode.
   */
  uint32_t getOptimizationLevel() const {
    return mOptimizationLevel;
  }

};

/**
 * Helper function to emit just the bitcode wrapper returning the number of
 * bytes that were written.
 *
 * \param wrapper - where to write header information into.
 * \param bitcodeSize - size of bitcode in bytes.
 * \param targetAPI - target API version for this bitcode.
 * \param compilerVersion - compiler version that generated this bitcode.
 * \param optimizationLevel - compiler optimization level for this bitcode.
 *
 * \return number of wrapper bytes written into the \p buffer.
 */
static inline size_t writeAndroidBitcodeWrapper(AndroidBitcodeWrapper *wrapper,
    size_t bitcodeSize, uint32_t targetAPI, uint32_t compilerVersion,
    uint32_t optimizationLevel) {
  if (!wrapper) {
    return 0;
  }

  wrapper->Magic = 0x0B17C0DE;
  wrapper->Version = 0;
  wrapper->BitcodeOffset = sizeof(*wrapper);
  wrapper->BitcodeSize = bitcodeSize;
  wrapper->HeaderVersion = 0;
  wrapper->TargetAPI = targetAPI;
  wrapper->PNaClVersion = 0;
  wrapper->CompilerVersionTag = BCHeaderField::kAndroidCompilerVersion;
  wrapper->CompilerVersionLen = 4;
  wrapper->CompilerVersion = compilerVersion;
  wrapper->OptimizationLevelTag = BCHeaderField::kAndroidOptimizationLevel;
  wrapper->OptimizationLevelLen = 4;
  wrapper->OptimizationLevel = optimizationLevel;

  return sizeof(*wrapper);
}

}  // namespace bcinfo

#endif  // __ANDROID_BCINFO_BITCODEWRAPPER_H__
