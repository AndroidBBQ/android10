/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef RSOV_CONTEXT_H
#define RSOV_CONTEXT_H

#include <vulkan/vulkan.h>
#include <mutex>

namespace android {
namespace renderscript {

namespace rsov {

class RSoVContext {
 public:
  static RSoVContext* create();
  ~RSoVContext();

  VkDevice getDevice() const { return mDevice; }
  VkQueue getQueue() const { return mQueue; }
  VkCommandPool getCmdPool() const { return mCmdPool; }

  bool MemoryTypeFromProperties(uint32_t typeBits, VkFlags requirements_mask,
                                uint32_t* typeIndex);

 private:
  RSoVContext();

  bool Initialize(char const* const name);

  static RSoVContext* mContext;
  static std::once_flag mInitFlag;

  VkInstance mInstance;
  VkPhysicalDevice mGPU;
  VkDevice mDevice;
  VkPhysicalDeviceMemoryProperties mMemoryProperties;
  VkQueue mQueue;
  VkCommandPool mCmdPool;
};

}  // namespace rsov
}  // namespace renderscript
}  // namespace android

#endif  // RSOV_CONTEXT_H
