/*
 * Copyright 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cstdint>
#include <vector>

namespace annotation_util {

typedef uint64_t AnnotationId;
typedef std::vector<uint8_t> SerializedAnnotation;
constexpr int kKeyError = -1;
constexpr AnnotationId kAnnotationError = -1;
constexpr uint64_t kStreamError = -1;

// Returns kAnnotationError if unsuccessful
AnnotationId DecodeAnnotationSerialization(const SerializedAnnotation &ser,
                                           const std::vector<int>& radix_mult);

// Returns 0 if successful, 1 if not
int SerializeAnnotationId(uint64_t id, SerializedAnnotation& ser,
                          const std::vector<int>& radix_mult);

void SetUpAnnotationRadixes( std::vector<int>& radix_mult,
                             const std::vector<int>& enum_sizes);

} // namespace annotation_util
