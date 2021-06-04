/*
 * Copyright 2018 The Android Open Source Project
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

#include <renderengine/mock/Image.h>

namespace android {
namespace renderengine {
namespace mock {

// The Google Mock documentation recommends explicit non-header instantiations
// for better compile time performance.
Image::Image() = default;
Image::~Image() = default;

} // namespace mock
} // namespace renderengine
} // namespace android
