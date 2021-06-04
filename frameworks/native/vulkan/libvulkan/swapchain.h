/*
 * Copyright 2015 The Android Open Source Project
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

#ifndef LIBVULKAN_SWAPCHAIN_H
#define LIBVULKAN_SWAPCHAIN_H 1

#include <vulkan/vulkan.h>

namespace vulkan {
namespace driver {

// clang-format off
VKAPI_ATTR VkResult CreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);
VKAPI_ATTR void DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* allocator);
VKAPI_ATTR VkResult GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice pdev, uint32_t queue_family, VkSurfaceKHR surface, VkBool32* pSupported);
VKAPI_ATTR VkResult GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice pdev, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* capabilities);
VKAPI_ATTR VkResult GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice pdev, VkSurfaceKHR surface_handle, uint32_t* count, VkSurfaceFormatKHR* formats);
VKAPI_ATTR VkResult GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice pdev, VkSurfaceKHR surface, uint32_t* count, VkPresentModeKHR* modes);
VKAPI_ATTR VkResult GetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities);
VKAPI_ATTR VkResult GetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes);
VKAPI_ATTR VkResult GetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects);
VKAPI_ATTR VkResult CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* create_info, const VkAllocationCallbacks* allocator, VkSwapchainKHR* swapchain_handle);
VKAPI_ATTR void DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain_handle, const VkAllocationCallbacks* allocator);
VKAPI_ATTR VkResult GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain_handle, uint32_t* count, VkImage* images);
VKAPI_ATTR VkResult AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain_handle, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* image_index);
VKAPI_ATTR VkResult QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* present_info);
VKAPI_ATTR VkResult AcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex);
VKAPI_ATTR VkResult GetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties);
VKAPI_ATTR VkResult GetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings);
VKAPI_ATTR VkResult GetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain);
VKAPI_ATTR void SetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pHdrMetadataEXTs);
VKAPI_ATTR VkResult GetPhysicalDeviceSurfaceCapabilities2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities);
VKAPI_ATTR VkResult GetPhysicalDeviceSurfaceFormats2KHR(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, uint32_t* pSurfaceFormatCount, VkSurfaceFormat2KHR* pSurfaceFormats);
VKAPI_ATTR VkResult BindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos);
VKAPI_ATTR VkResult BindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos);
// clang-format on

}  // namespace driver
}  // namespace vulkan

#endif  // LIBVULKAN_SWAPCHAIN_H
