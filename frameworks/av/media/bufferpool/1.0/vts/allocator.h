/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef VNDK_HIDL_BUFFERPOOL_V1_0_ALLOCATOR_H
#define VNDK_HIDL_BUFFERPOOL_V1_0_ALLOCATOR_H

#include <bufferpool/BufferPoolTypes.h>

using android::hardware::media::bufferpool::V1_0::ResultStatus;
using android::hardware::media::bufferpool::V1_0::implementation::
    BufferPoolAllocation;
using android::hardware::media::bufferpool::V1_0::implementation::
    BufferPoolAllocator;

// buffer allocator for the tests
class TestBufferPoolAllocator : public BufferPoolAllocator {
 public:
  TestBufferPoolAllocator() {}

  ~TestBufferPoolAllocator() override {}

  ResultStatus allocate(const std::vector<uint8_t> &params,
                        std::shared_ptr<BufferPoolAllocation> *alloc,
                        size_t *allocSize) override;

  bool compatible(const std::vector<uint8_t> &newParams,
                  const std::vector<uint8_t> &oldParams) override;

  static bool Fill(const native_handle_t *handle, const unsigned char val);

  static bool Verify(const native_handle_t *handle, const unsigned char val);

};

// retrieve buffer allocator paramters
void getTestAllocatorParams(std::vector<uint8_t> *params);

#endif  // VNDK_HIDL_BUFFERPOOL_V1_0_ALLOCATOR_H
