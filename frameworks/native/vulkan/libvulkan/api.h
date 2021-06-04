/*
 * Copyright 2016 The Android Open Source Project
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

#ifndef LIBVULKAN_API_H
#define LIBVULKAN_API_H 1

#include <vulkan/vulkan.h>
#include "api_gen.h"
#include "driver.h"

namespace vulkan {
namespace api {

// clang-format off
VKAPI_ATTR VkResult CreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
VKAPI_ATTR void DestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult CreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice);
VKAPI_ATTR void DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult EnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties);
VKAPI_ATTR VkResult EnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties);
VKAPI_ATTR VkResult EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties);
VKAPI_ATTR VkResult EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties);
VKAPI_ATTR VkResult EnumerateInstanceVersion(uint32_t* pApiVersion);
// clang-format on

inline InstanceData& GetData(VkInstance instance) {
    return driver::GetData(instance).opaque_api_data;
}

inline InstanceData& GetData(VkPhysicalDevice physical_dev) {
    return driver::GetData(physical_dev).opaque_api_data;
}

inline DeviceData& GetData(VkDevice dev) {
    return driver::GetData(dev).opaque_api_data;
}

inline DeviceData& GetData(VkQueue queue) {
    return driver::GetData(queue).opaque_api_data;
}

inline DeviceData& GetData(VkCommandBuffer cmd) {
    return driver::GetData(cmd).opaque_api_data;
}

}  // namespace api
}  // namespace vulkan

#endif  // LIBVULKAN_API_H
