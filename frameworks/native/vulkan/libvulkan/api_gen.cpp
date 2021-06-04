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

// WARNING: This file is generated. See ../README.md for instructions.

#include <string.h>

#include <algorithm>

#include <log/log.h>

// to catch mismatches between vulkan.h and this file
#undef VK_NO_PROTOTYPES
#include "api.h"

namespace vulkan {
namespace api {

#define UNLIKELY(expr) __builtin_expect((expr), 0)

#define INIT_PROC(required, obj, proc)                                 \
    do {                                                               \
        data.dispatch.proc =                                           \
            reinterpret_cast<PFN_vk##proc>(get_proc(obj, "vk" #proc)); \
        if (UNLIKELY(required && !data.dispatch.proc)) {               \
            ALOGE("missing " #obj " proc: vk" #proc);                  \
            success = false;                                           \
        }                                                              \
    } while (0)

// Exported extension functions may be invoked even when their extensions
// are disabled.  Dispatch to stubs when that happens.
#define INIT_PROC_EXT(ext, required, obj, proc)  \
    do {                                         \
        if (extensions[driver::ProcHook::ext])   \
            INIT_PROC(required, obj, proc);      \
        else                                     \
            data.dispatch.proc = disabled##proc; \
    } while (0)

namespace {

// clang-format off

VKAPI_ATTR void disabledDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR, const VkAllocationCallbacks*) {
    driver::Logger(instance).Err(instance, "VK_KHR_surface not enabled. Exported vkDestroySurfaceKHR not executed.");
}

VKAPI_ATTR VkResult disabledGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t, VkSurfaceKHR, VkBool32*) {
    driver::Logger(physicalDevice).Err(physicalDevice, "VK_KHR_surface not enabled. Exported vkGetPhysicalDeviceSurfaceSupportKHR not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult disabledGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR, VkSurfaceCapabilitiesKHR*) {
    driver::Logger(physicalDevice).Err(physicalDevice, "VK_KHR_surface not enabled. Exported vkGetPhysicalDeviceSurfaceCapabilitiesKHR not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult disabledGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR, uint32_t*, VkSurfaceFormatKHR*) {
    driver::Logger(physicalDevice).Err(physicalDevice, "VK_KHR_surface not enabled. Exported vkGetPhysicalDeviceSurfaceFormatsKHR not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult disabledGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR, uint32_t*, VkPresentModeKHR*) {
    driver::Logger(physicalDevice).Err(physicalDevice, "VK_KHR_surface not enabled. Exported vkGetPhysicalDeviceSurfacePresentModesKHR not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult disabledCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR*, const VkAllocationCallbacks*, VkSwapchainKHR*) {
    driver::Logger(device).Err(device, "VK_KHR_swapchain not enabled. Exported vkCreateSwapchainKHR not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR void disabledDestroySwapchainKHR(VkDevice device, VkSwapchainKHR, const VkAllocationCallbacks*) {
    driver::Logger(device).Err(device, "VK_KHR_swapchain not enabled. Exported vkDestroySwapchainKHR not executed.");
}

VKAPI_ATTR VkResult disabledGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR, uint32_t*, VkImage*) {
    driver::Logger(device).Err(device, "VK_KHR_swapchain not enabled. Exported vkGetSwapchainImagesKHR not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult disabledAcquireNextImageKHR(VkDevice device, VkSwapchainKHR, uint64_t, VkSemaphore, VkFence, uint32_t*) {
    driver::Logger(device).Err(device, "VK_KHR_swapchain not enabled. Exported vkAcquireNextImageKHR not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult disabledQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR*) {
    driver::Logger(queue).Err(queue, "VK_KHR_swapchain not enabled. Exported vkQueuePresentKHR not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult disabledGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR*) {
    driver::Logger(device).Err(device, "VK_KHR_swapchain not enabled. Exported vkGetDeviceGroupPresentCapabilitiesKHR not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult disabledGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR, VkDeviceGroupPresentModeFlagsKHR*) {
    driver::Logger(device).Err(device, "VK_KHR_swapchain not enabled. Exported vkGetDeviceGroupSurfacePresentModesKHR not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult disabledGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR, uint32_t*, VkRect2D*) {
    driver::Logger(physicalDevice).Err(physicalDevice, "VK_KHR_swapchain not enabled. Exported vkGetPhysicalDevicePresentRectanglesKHR not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult disabledAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR*, uint32_t*) {
    driver::Logger(device).Err(device, "VK_KHR_swapchain not enabled. Exported vkAcquireNextImage2KHR not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult disabledCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR*, const VkAllocationCallbacks*, VkSurfaceKHR*) {
    driver::Logger(instance).Err(instance, "VK_KHR_android_surface not enabled. Exported vkCreateAndroidSurfaceKHR not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult disabledGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer*, VkAndroidHardwareBufferPropertiesANDROID*) {
    driver::Logger(device).Err(device, "VK_ANDROID_external_memory_android_hardware_buffer not enabled. Exported vkGetAndroidHardwareBufferPropertiesANDROID not executed.");
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult disabledGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID*, struct AHardwareBuffer**) {
    driver::Logger(device).Err(device, "VK_ANDROID_external_memory_android_hardware_buffer not enabled. Exported vkGetMemoryAndroidHardwareBufferANDROID not executed.");
    return VK_SUCCESS;
}

// clang-format on

}  // namespace

bool InitDispatchTable(
    VkInstance instance,
    PFN_vkGetInstanceProcAddr get_proc,
    const std::bitset<driver::ProcHook::EXTENSION_COUNT>& extensions) {
    auto& data = GetData(instance);
    bool success = true;

    // clang-format off
    INIT_PROC(true, instance, DestroyInstance);
    INIT_PROC(true, instance, EnumeratePhysicalDevices);
    INIT_PROC(true, instance, GetInstanceProcAddr);
    INIT_PROC(true, instance, GetPhysicalDeviceProperties);
    INIT_PROC(true, instance, GetPhysicalDeviceQueueFamilyProperties);
    INIT_PROC(true, instance, GetPhysicalDeviceMemoryProperties);
    INIT_PROC(true, instance, GetPhysicalDeviceFeatures);
    INIT_PROC(true, instance, GetPhysicalDeviceFormatProperties);
    INIT_PROC(true, instance, GetPhysicalDeviceImageFormatProperties);
    INIT_PROC(true, instance, CreateDevice);
    INIT_PROC(true, instance, EnumerateDeviceExtensionProperties);
    INIT_PROC(true, instance, GetPhysicalDeviceSparseImageFormatProperties);
    INIT_PROC(false, instance, EnumeratePhysicalDeviceGroups);
    INIT_PROC(false, instance, GetPhysicalDeviceFeatures2);
    INIT_PROC(false, instance, GetPhysicalDeviceProperties2);
    INIT_PROC(false, instance, GetPhysicalDeviceFormatProperties2);
    INIT_PROC(false, instance, GetPhysicalDeviceImageFormatProperties2);
    INIT_PROC(false, instance, GetPhysicalDeviceQueueFamilyProperties2);
    INIT_PROC(false, instance, GetPhysicalDeviceMemoryProperties2);
    INIT_PROC(false, instance, GetPhysicalDeviceSparseImageFormatProperties2);
    INIT_PROC(false, instance, GetPhysicalDeviceExternalBufferProperties);
    INIT_PROC(false, instance, GetPhysicalDeviceExternalFenceProperties);
    INIT_PROC(false, instance, GetPhysicalDeviceExternalSemaphoreProperties);
    INIT_PROC_EXT(KHR_surface, true, instance, DestroySurfaceKHR);
    INIT_PROC_EXT(KHR_surface, true, instance, GetPhysicalDeviceSurfaceSupportKHR);
    INIT_PROC_EXT(KHR_surface, true, instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
    INIT_PROC_EXT(KHR_surface, true, instance, GetPhysicalDeviceSurfaceFormatsKHR);
    INIT_PROC_EXT(KHR_surface, true, instance, GetPhysicalDeviceSurfacePresentModesKHR);
    INIT_PROC_EXT(KHR_swapchain, false, instance, GetPhysicalDevicePresentRectanglesKHR);
    INIT_PROC_EXT(KHR_android_surface, true, instance, CreateAndroidSurfaceKHR);
    // clang-format on

    return success;
}

bool InitDispatchTable(
    VkDevice dev,
    PFN_vkGetDeviceProcAddr get_proc,
    const std::bitset<driver::ProcHook::EXTENSION_COUNT>& extensions) {
    auto& data = GetData(dev);
    bool success = true;

    // clang-format off
    INIT_PROC(true, dev, GetDeviceProcAddr);
    INIT_PROC(true, dev, DestroyDevice);
    INIT_PROC(true, dev, GetDeviceQueue);
    INIT_PROC(true, dev, QueueSubmit);
    INIT_PROC(true, dev, QueueWaitIdle);
    INIT_PROC(true, dev, DeviceWaitIdle);
    INIT_PROC(true, dev, AllocateMemory);
    INIT_PROC(true, dev, FreeMemory);
    INIT_PROC(true, dev, MapMemory);
    INIT_PROC(true, dev, UnmapMemory);
    INIT_PROC(true, dev, FlushMappedMemoryRanges);
    INIT_PROC(true, dev, InvalidateMappedMemoryRanges);
    INIT_PROC(true, dev, GetDeviceMemoryCommitment);
    INIT_PROC(true, dev, GetBufferMemoryRequirements);
    INIT_PROC(true, dev, BindBufferMemory);
    INIT_PROC(true, dev, GetImageMemoryRequirements);
    INIT_PROC(true, dev, BindImageMemory);
    INIT_PROC(true, dev, GetImageSparseMemoryRequirements);
    INIT_PROC(true, dev, QueueBindSparse);
    INIT_PROC(true, dev, CreateFence);
    INIT_PROC(true, dev, DestroyFence);
    INIT_PROC(true, dev, ResetFences);
    INIT_PROC(true, dev, GetFenceStatus);
    INIT_PROC(true, dev, WaitForFences);
    INIT_PROC(true, dev, CreateSemaphore);
    INIT_PROC(true, dev, DestroySemaphore);
    INIT_PROC(true, dev, CreateEvent);
    INIT_PROC(true, dev, DestroyEvent);
    INIT_PROC(true, dev, GetEventStatus);
    INIT_PROC(true, dev, SetEvent);
    INIT_PROC(true, dev, ResetEvent);
    INIT_PROC(true, dev, CreateQueryPool);
    INIT_PROC(true, dev, DestroyQueryPool);
    INIT_PROC(true, dev, GetQueryPoolResults);
    INIT_PROC(true, dev, CreateBuffer);
    INIT_PROC(true, dev, DestroyBuffer);
    INIT_PROC(true, dev, CreateBufferView);
    INIT_PROC(true, dev, DestroyBufferView);
    INIT_PROC(true, dev, CreateImage);
    INIT_PROC(true, dev, DestroyImage);
    INIT_PROC(true, dev, GetImageSubresourceLayout);
    INIT_PROC(true, dev, CreateImageView);
    INIT_PROC(true, dev, DestroyImageView);
    INIT_PROC(true, dev, CreateShaderModule);
    INIT_PROC(true, dev, DestroyShaderModule);
    INIT_PROC(true, dev, CreatePipelineCache);
    INIT_PROC(true, dev, DestroyPipelineCache);
    INIT_PROC(true, dev, GetPipelineCacheData);
    INIT_PROC(true, dev, MergePipelineCaches);
    INIT_PROC(true, dev, CreateGraphicsPipelines);
    INIT_PROC(true, dev, CreateComputePipelines);
    INIT_PROC(true, dev, DestroyPipeline);
    INIT_PROC(true, dev, CreatePipelineLayout);
    INIT_PROC(true, dev, DestroyPipelineLayout);
    INIT_PROC(true, dev, CreateSampler);
    INIT_PROC(true, dev, DestroySampler);
    INIT_PROC(true, dev, CreateDescriptorSetLayout);
    INIT_PROC(true, dev, DestroyDescriptorSetLayout);
    INIT_PROC(true, dev, CreateDescriptorPool);
    INIT_PROC(true, dev, DestroyDescriptorPool);
    INIT_PROC(true, dev, ResetDescriptorPool);
    INIT_PROC(true, dev, AllocateDescriptorSets);
    INIT_PROC(true, dev, FreeDescriptorSets);
    INIT_PROC(true, dev, UpdateDescriptorSets);
    INIT_PROC(true, dev, CreateFramebuffer);
    INIT_PROC(true, dev, DestroyFramebuffer);
    INIT_PROC(true, dev, CreateRenderPass);
    INIT_PROC(true, dev, DestroyRenderPass);
    INIT_PROC(true, dev, GetRenderAreaGranularity);
    INIT_PROC(true, dev, CreateCommandPool);
    INIT_PROC(true, dev, DestroyCommandPool);
    INIT_PROC(true, dev, ResetCommandPool);
    INIT_PROC(true, dev, AllocateCommandBuffers);
    INIT_PROC(true, dev, FreeCommandBuffers);
    INIT_PROC(true, dev, BeginCommandBuffer);
    INIT_PROC(true, dev, EndCommandBuffer);
    INIT_PROC(true, dev, ResetCommandBuffer);
    INIT_PROC(true, dev, CmdBindPipeline);
    INIT_PROC(true, dev, CmdSetViewport);
    INIT_PROC(true, dev, CmdSetScissor);
    INIT_PROC(true, dev, CmdSetLineWidth);
    INIT_PROC(true, dev, CmdSetDepthBias);
    INIT_PROC(true, dev, CmdSetBlendConstants);
    INIT_PROC(true, dev, CmdSetDepthBounds);
    INIT_PROC(true, dev, CmdSetStencilCompareMask);
    INIT_PROC(true, dev, CmdSetStencilWriteMask);
    INIT_PROC(true, dev, CmdSetStencilReference);
    INIT_PROC(true, dev, CmdBindDescriptorSets);
    INIT_PROC(true, dev, CmdBindIndexBuffer);
    INIT_PROC(true, dev, CmdBindVertexBuffers);
    INIT_PROC(true, dev, CmdDraw);
    INIT_PROC(true, dev, CmdDrawIndexed);
    INIT_PROC(true, dev, CmdDrawIndirect);
    INIT_PROC(true, dev, CmdDrawIndexedIndirect);
    INIT_PROC(true, dev, CmdDispatch);
    INIT_PROC(true, dev, CmdDispatchIndirect);
    INIT_PROC(true, dev, CmdCopyBuffer);
    INIT_PROC(true, dev, CmdCopyImage);
    INIT_PROC(true, dev, CmdBlitImage);
    INIT_PROC(true, dev, CmdCopyBufferToImage);
    INIT_PROC(true, dev, CmdCopyImageToBuffer);
    INIT_PROC(true, dev, CmdUpdateBuffer);
    INIT_PROC(true, dev, CmdFillBuffer);
    INIT_PROC(true, dev, CmdClearColorImage);
    INIT_PROC(true, dev, CmdClearDepthStencilImage);
    INIT_PROC(true, dev, CmdClearAttachments);
    INIT_PROC(true, dev, CmdResolveImage);
    INIT_PROC(true, dev, CmdSetEvent);
    INIT_PROC(true, dev, CmdResetEvent);
    INIT_PROC(true, dev, CmdWaitEvents);
    INIT_PROC(true, dev, CmdPipelineBarrier);
    INIT_PROC(true, dev, CmdBeginQuery);
    INIT_PROC(true, dev, CmdEndQuery);
    INIT_PROC(true, dev, CmdResetQueryPool);
    INIT_PROC(true, dev, CmdWriteTimestamp);
    INIT_PROC(true, dev, CmdCopyQueryPoolResults);
    INIT_PROC(true, dev, CmdPushConstants);
    INIT_PROC(true, dev, CmdBeginRenderPass);
    INIT_PROC(true, dev, CmdNextSubpass);
    INIT_PROC(true, dev, CmdEndRenderPass);
    INIT_PROC(true, dev, CmdExecuteCommands);
    INIT_PROC(false, dev, BindBufferMemory2);
    INIT_PROC(false, dev, BindImageMemory2);
    INIT_PROC(false, dev, GetDeviceGroupPeerMemoryFeatures);
    INIT_PROC(false, dev, CmdSetDeviceMask);
    INIT_PROC(false, dev, CmdDispatchBase);
    INIT_PROC(false, dev, GetImageMemoryRequirements2);
    INIT_PROC(false, dev, GetBufferMemoryRequirements2);
    INIT_PROC(false, dev, GetImageSparseMemoryRequirements2);
    INIT_PROC(false, dev, TrimCommandPool);
    INIT_PROC(false, dev, GetDeviceQueue2);
    INIT_PROC(false, dev, CreateSamplerYcbcrConversion);
    INIT_PROC(false, dev, DestroySamplerYcbcrConversion);
    INIT_PROC(false, dev, CreateDescriptorUpdateTemplate);
    INIT_PROC(false, dev, DestroyDescriptorUpdateTemplate);
    INIT_PROC(false, dev, UpdateDescriptorSetWithTemplate);
    INIT_PROC(false, dev, GetDescriptorSetLayoutSupport);
    INIT_PROC_EXT(KHR_swapchain, true, dev, CreateSwapchainKHR);
    INIT_PROC_EXT(KHR_swapchain, true, dev, DestroySwapchainKHR);
    INIT_PROC_EXT(KHR_swapchain, true, dev, GetSwapchainImagesKHR);
    INIT_PROC_EXT(KHR_swapchain, true, dev, AcquireNextImageKHR);
    INIT_PROC_EXT(KHR_swapchain, true, dev, QueuePresentKHR);
    INIT_PROC_EXT(KHR_swapchain, false, dev, GetDeviceGroupPresentCapabilitiesKHR);
    INIT_PROC_EXT(KHR_swapchain, false, dev, GetDeviceGroupSurfacePresentModesKHR);
    INIT_PROC_EXT(KHR_swapchain, false, dev, AcquireNextImage2KHR);
    INIT_PROC_EXT(ANDROID_external_memory_android_hardware_buffer, false, dev, GetAndroidHardwareBufferPropertiesANDROID);
    INIT_PROC_EXT(ANDROID_external_memory_android_hardware_buffer, false, dev, GetMemoryAndroidHardwareBufferANDROID);
    // clang-format on

    return success;
}

// clang-format off

namespace {

// forward declarations needed by GetInstanceProcAddr and GetDeviceProcAddr
VKAPI_ATTR VkResult EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices);
VKAPI_ATTR PFN_vkVoidFunction GetDeviceProcAddr(VkDevice device, const char* pName);
VKAPI_ATTR PFN_vkVoidFunction GetInstanceProcAddr(VkInstance instance, const char* pName);
VKAPI_ATTR void GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties);
VKAPI_ATTR void GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties);
VKAPI_ATTR void GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties);
VKAPI_ATTR void GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures);
VKAPI_ATTR void GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties);
VKAPI_ATTR VkResult GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties);
VKAPI_ATTR void GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue);
VKAPI_ATTR VkResult QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
VKAPI_ATTR VkResult QueueWaitIdle(VkQueue queue);
VKAPI_ATTR VkResult DeviceWaitIdle(VkDevice device);
VKAPI_ATTR VkResult AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory);
VKAPI_ATTR void FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult MapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
VKAPI_ATTR void UnmapMemory(VkDevice device, VkDeviceMemory memory);
VKAPI_ATTR VkResult FlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges);
VKAPI_ATTR VkResult InvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges);
VKAPI_ATTR void GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes);
VKAPI_ATTR void GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements);
VKAPI_ATTR VkResult BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);
VKAPI_ATTR void GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements);
VKAPI_ATTR VkResult BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset);
VKAPI_ATTR void GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements);
VKAPI_ATTR void GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties);
VKAPI_ATTR VkResult QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence);
VKAPI_ATTR VkResult CreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence);
VKAPI_ATTR void DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences);
VKAPI_ATTR VkResult GetFenceStatus(VkDevice device, VkFence fence);
VKAPI_ATTR VkResult WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout);
VKAPI_ATTR VkResult CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore);
VKAPI_ATTR void DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult CreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent);
VKAPI_ATTR void DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult GetEventStatus(VkDevice device, VkEvent event);
VKAPI_ATTR VkResult SetEvent(VkDevice device, VkEvent event);
VKAPI_ATTR VkResult ResetEvent(VkDevice device, VkEvent event);
VKAPI_ATTR VkResult CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool);
VKAPI_ATTR void DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags);
VKAPI_ATTR VkResult CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer);
VKAPI_ATTR void DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult CreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView);
VKAPI_ATTR void DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage);
VKAPI_ATTR void DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR void GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout);
VKAPI_ATTR VkResult CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView);
VKAPI_ATTR void DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule);
VKAPI_ATTR void DestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache);
VKAPI_ATTR void DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData);
VKAPI_ATTR VkResult MergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches);
VKAPI_ATTR VkResult CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
VKAPI_ATTR VkResult CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines);
VKAPI_ATTR void DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout);
VKAPI_ATTR void DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult CreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler);
VKAPI_ATTR void DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout);
VKAPI_ATTR void DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool);
VKAPI_ATTR void DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags);
VKAPI_ATTR VkResult AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets);
VKAPI_ATTR VkResult FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets);
VKAPI_ATTR void UpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies);
VKAPI_ATTR VkResult CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer);
VKAPI_ATTR void DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass);
VKAPI_ATTR void DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR void GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity);
VKAPI_ATTR VkResult CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool);
VKAPI_ATTR void DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags);
VKAPI_ATTR VkResult AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers);
VKAPI_ATTR void FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);
VKAPI_ATTR VkResult BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo);
VKAPI_ATTR VkResult EndCommandBuffer(VkCommandBuffer commandBuffer);
VKAPI_ATTR VkResult ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
VKAPI_ATTR void CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
VKAPI_ATTR void CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports);
VKAPI_ATTR void CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors);
VKAPI_ATTR void CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth);
VKAPI_ATTR void CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
VKAPI_ATTR void CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]);
VKAPI_ATTR void CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds);
VKAPI_ATTR void CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask);
VKAPI_ATTR void CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask);
VKAPI_ATTR void CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference);
VKAPI_ATTR void CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets);
VKAPI_ATTR void CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
VKAPI_ATTR void CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
VKAPI_ATTR void CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
VKAPI_ATTR void CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
VKAPI_ATTR void CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
VKAPI_ATTR void CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
VKAPI_ATTR void CmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
VKAPI_ATTR void CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);
VKAPI_ATTR void CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);
VKAPI_ATTR void CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions);
VKAPI_ATTR void CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter);
VKAPI_ATTR void CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions);
VKAPI_ATTR void CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);
VKAPI_ATTR void CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData);
VKAPI_ATTR void CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);
VKAPI_ATTR void CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges);
VKAPI_ATTR void CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges);
VKAPI_ATTR void CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects);
VKAPI_ATTR void CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions);
VKAPI_ATTR void CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
VKAPI_ATTR void CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);
VKAPI_ATTR void CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
VKAPI_ATTR void CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
VKAPI_ATTR void CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags);
VKAPI_ATTR void CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query);
VKAPI_ATTR void CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
VKAPI_ATTR void CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query);
VKAPI_ATTR void CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags);
VKAPI_ATTR void CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues);
VKAPI_ATTR void CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents);
VKAPI_ATTR void CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);
VKAPI_ATTR void CmdEndRenderPass(VkCommandBuffer commandBuffer);
VKAPI_ATTR void CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);
VKAPI_ATTR VkResult BindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos);
VKAPI_ATTR VkResult BindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos);
VKAPI_ATTR void GetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures);
VKAPI_ATTR void CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask);
VKAPI_ATTR void CmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
VKAPI_ATTR VkResult EnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties);
VKAPI_ATTR void GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements);
VKAPI_ATTR void GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements);
VKAPI_ATTR void GetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements);
VKAPI_ATTR void GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures);
VKAPI_ATTR void GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties);
VKAPI_ATTR void GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties);
VKAPI_ATTR VkResult GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties);
VKAPI_ATTR void GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties);
VKAPI_ATTR void GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties);
VKAPI_ATTR void GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties);
VKAPI_ATTR void TrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags);
VKAPI_ATTR void GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue);
VKAPI_ATTR VkResult CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion);
VKAPI_ATTR void DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult CreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate);
VKAPI_ATTR void DestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR void UpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData);
VKAPI_ATTR void GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties);
VKAPI_ATTR void GetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties);
VKAPI_ATTR void GetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties);
VKAPI_ATTR void GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport);
VKAPI_ATTR void DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported);
VKAPI_ATTR VkResult GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
VKAPI_ATTR VkResult GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
VKAPI_ATTR VkResult GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes);
VKAPI_ATTR VkResult CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);
VKAPI_ATTR void DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkResult GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages);
VKAPI_ATTR VkResult AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
VKAPI_ATTR VkResult QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
VKAPI_ATTR VkResult GetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities);
VKAPI_ATTR VkResult GetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes);
VKAPI_ATTR VkResult GetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects);
VKAPI_ATTR VkResult AcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex);
VKAPI_ATTR VkResult CreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
VKAPI_ATTR VkResult GetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties);
VKAPI_ATTR VkResult GetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo, struct AHardwareBuffer** pBuffer);

VKAPI_ATTR VkResult EnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) {
    return GetData(instance).dispatch.EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
}

VKAPI_ATTR PFN_vkVoidFunction GetDeviceProcAddr(VkDevice device, const char* pName) {
    if (device == VK_NULL_HANDLE) {
        ALOGE("invalid vkGetDeviceProcAddr(VK_NULL_HANDLE, ...) call");
        return nullptr;
    }

    static const char* const known_non_device_names[] = {
        "vkCreateAndroidSurfaceKHR",
        "vkCreateDebugReportCallbackEXT",
        "vkCreateDebugUtilsMessengerEXT",
        "vkCreateDevice",
        "vkCreateInstance",
        "vkDebugReportMessageEXT",
        "vkDestroyDebugReportCallbackEXT",
        "vkDestroyDebugUtilsMessengerEXT",
        "vkDestroyInstance",
        "vkDestroySurfaceKHR",
        "vkEnumerateDeviceExtensionProperties",
        "vkEnumerateDeviceLayerProperties",
        "vkEnumerateInstanceExtensionProperties",
        "vkEnumerateInstanceLayerProperties",
        "vkEnumerateInstanceVersion",
        "vkEnumeratePhysicalDeviceGroups",
        "vkEnumeratePhysicalDeviceGroupsKHR",
        "vkEnumeratePhysicalDevices",
        "vkGetDisplayModeProperties2KHR",
        "vkGetDisplayPlaneCapabilities2KHR",
        "vkGetInstanceProcAddr",
        "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT",
        "vkGetPhysicalDeviceDisplayPlaneProperties2KHR",
        "vkGetPhysicalDeviceDisplayProperties2KHR",
        "vkGetPhysicalDeviceExternalBufferProperties",
        "vkGetPhysicalDeviceExternalBufferPropertiesKHR",
        "vkGetPhysicalDeviceExternalFenceProperties",
        "vkGetPhysicalDeviceExternalFencePropertiesKHR",
        "vkGetPhysicalDeviceExternalImageFormatPropertiesNV",
        "vkGetPhysicalDeviceExternalSemaphoreProperties",
        "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR",
        "vkGetPhysicalDeviceFeatures",
        "vkGetPhysicalDeviceFeatures2",
        "vkGetPhysicalDeviceFeatures2KHR",
        "vkGetPhysicalDeviceFormatProperties",
        "vkGetPhysicalDeviceFormatProperties2",
        "vkGetPhysicalDeviceFormatProperties2KHR",
        "vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX",
        "vkGetPhysicalDeviceImageFormatProperties",
        "vkGetPhysicalDeviceImageFormatProperties2",
        "vkGetPhysicalDeviceImageFormatProperties2KHR",
        "vkGetPhysicalDeviceMemoryProperties",
        "vkGetPhysicalDeviceMemoryProperties2",
        "vkGetPhysicalDeviceMemoryProperties2KHR",
        "vkGetPhysicalDeviceMultisamplePropertiesEXT",
        "vkGetPhysicalDevicePresentRectanglesKHR",
        "vkGetPhysicalDeviceProperties",
        "vkGetPhysicalDeviceProperties2",
        "vkGetPhysicalDeviceProperties2KHR",
        "vkGetPhysicalDeviceQueueFamilyProperties",
        "vkGetPhysicalDeviceQueueFamilyProperties2",
        "vkGetPhysicalDeviceQueueFamilyProperties2KHR",
        "vkGetPhysicalDeviceSparseImageFormatProperties",
        "vkGetPhysicalDeviceSparseImageFormatProperties2",
        "vkGetPhysicalDeviceSparseImageFormatProperties2KHR",
        "vkGetPhysicalDeviceSurfaceCapabilities2KHR",
        "vkGetPhysicalDeviceSurfaceCapabilitiesKHR",
        "vkGetPhysicalDeviceSurfaceFormats2KHR",
        "vkGetPhysicalDeviceSurfaceFormatsKHR",
        "vkGetPhysicalDeviceSurfacePresentModesKHR",
        "vkGetPhysicalDeviceSurfaceSupportKHR",
        "vkSubmitDebugUtilsMessageEXT",
    };
    // clang-format on
    constexpr size_t count =
        sizeof(known_non_device_names) / sizeof(known_non_device_names[0]);
    if (!pName ||
        std::binary_search(
            known_non_device_names, known_non_device_names + count, pName,
            [](const char* a, const char* b) { return (strcmp(a, b) < 0); })) {
        vulkan::driver::Logger(device).Err(
            device, "invalid vkGetDeviceProcAddr(%p, \"%s\") call", device,
            (pName) ? pName : "(null)");
        return nullptr;
    }
    // clang-format off

    if (strcmp(pName, "vkGetDeviceProcAddr") == 0) return reinterpret_cast<PFN_vkVoidFunction>(GetDeviceProcAddr);
    if (strcmp(pName, "vkDestroyDevice") == 0) return reinterpret_cast<PFN_vkVoidFunction>(DestroyDevice);

    return GetData(device).dispatch.GetDeviceProcAddr(device, pName);
}

VKAPI_ATTR PFN_vkVoidFunction GetInstanceProcAddr(VkInstance instance, const char* pName) {
    // global functions
    if (instance == VK_NULL_HANDLE) {
        if (strcmp(pName, "vkCreateInstance") == 0) return reinterpret_cast<PFN_vkVoidFunction>(CreateInstance);
        if (strcmp(pName, "vkEnumerateInstanceLayerProperties") == 0) return reinterpret_cast<PFN_vkVoidFunction>(EnumerateInstanceLayerProperties);
        if (strcmp(pName, "vkEnumerateInstanceExtensionProperties") == 0) return reinterpret_cast<PFN_vkVoidFunction>(EnumerateInstanceExtensionProperties);
        if (strcmp(pName, "vkEnumerateInstanceVersion") == 0) return reinterpret_cast<PFN_vkVoidFunction>(EnumerateInstanceVersion);

        ALOGE("invalid vkGetInstanceProcAddr(VK_NULL_HANDLE, \"%s\") call", pName);
        return nullptr;
    }

    static const struct Hook {
        const char* name;
        PFN_vkVoidFunction proc;
    } hooks[] = {
        { "vkAcquireNextImage2KHR", reinterpret_cast<PFN_vkVoidFunction>(AcquireNextImage2KHR) },
        { "vkAcquireNextImageKHR", reinterpret_cast<PFN_vkVoidFunction>(AcquireNextImageKHR) },
        { "vkAllocateCommandBuffers", reinterpret_cast<PFN_vkVoidFunction>(AllocateCommandBuffers) },
        { "vkAllocateDescriptorSets", reinterpret_cast<PFN_vkVoidFunction>(AllocateDescriptorSets) },
        { "vkAllocateMemory", reinterpret_cast<PFN_vkVoidFunction>(AllocateMemory) },
        { "vkBeginCommandBuffer", reinterpret_cast<PFN_vkVoidFunction>(BeginCommandBuffer) },
        { "vkBindBufferMemory", reinterpret_cast<PFN_vkVoidFunction>(BindBufferMemory) },
        { "vkBindBufferMemory2", reinterpret_cast<PFN_vkVoidFunction>(BindBufferMemory2) },
        { "vkBindImageMemory", reinterpret_cast<PFN_vkVoidFunction>(BindImageMemory) },
        { "vkBindImageMemory2", reinterpret_cast<PFN_vkVoidFunction>(BindImageMemory2) },
        { "vkCmdBeginQuery", reinterpret_cast<PFN_vkVoidFunction>(CmdBeginQuery) },
        { "vkCmdBeginRenderPass", reinterpret_cast<PFN_vkVoidFunction>(CmdBeginRenderPass) },
        { "vkCmdBindDescriptorSets", reinterpret_cast<PFN_vkVoidFunction>(CmdBindDescriptorSets) },
        { "vkCmdBindIndexBuffer", reinterpret_cast<PFN_vkVoidFunction>(CmdBindIndexBuffer) },
        { "vkCmdBindPipeline", reinterpret_cast<PFN_vkVoidFunction>(CmdBindPipeline) },
        { "vkCmdBindVertexBuffers", reinterpret_cast<PFN_vkVoidFunction>(CmdBindVertexBuffers) },
        { "vkCmdBlitImage", reinterpret_cast<PFN_vkVoidFunction>(CmdBlitImage) },
        { "vkCmdClearAttachments", reinterpret_cast<PFN_vkVoidFunction>(CmdClearAttachments) },
        { "vkCmdClearColorImage", reinterpret_cast<PFN_vkVoidFunction>(CmdClearColorImage) },
        { "vkCmdClearDepthStencilImage", reinterpret_cast<PFN_vkVoidFunction>(CmdClearDepthStencilImage) },
        { "vkCmdCopyBuffer", reinterpret_cast<PFN_vkVoidFunction>(CmdCopyBuffer) },
        { "vkCmdCopyBufferToImage", reinterpret_cast<PFN_vkVoidFunction>(CmdCopyBufferToImage) },
        { "vkCmdCopyImage", reinterpret_cast<PFN_vkVoidFunction>(CmdCopyImage) },
        { "vkCmdCopyImageToBuffer", reinterpret_cast<PFN_vkVoidFunction>(CmdCopyImageToBuffer) },
        { "vkCmdCopyQueryPoolResults", reinterpret_cast<PFN_vkVoidFunction>(CmdCopyQueryPoolResults) },
        { "vkCmdDispatch", reinterpret_cast<PFN_vkVoidFunction>(CmdDispatch) },
        { "vkCmdDispatchBase", reinterpret_cast<PFN_vkVoidFunction>(CmdDispatchBase) },
        { "vkCmdDispatchIndirect", reinterpret_cast<PFN_vkVoidFunction>(CmdDispatchIndirect) },
        { "vkCmdDraw", reinterpret_cast<PFN_vkVoidFunction>(CmdDraw) },
        { "vkCmdDrawIndexed", reinterpret_cast<PFN_vkVoidFunction>(CmdDrawIndexed) },
        { "vkCmdDrawIndexedIndirect", reinterpret_cast<PFN_vkVoidFunction>(CmdDrawIndexedIndirect) },
        { "vkCmdDrawIndirect", reinterpret_cast<PFN_vkVoidFunction>(CmdDrawIndirect) },
        { "vkCmdEndQuery", reinterpret_cast<PFN_vkVoidFunction>(CmdEndQuery) },
        { "vkCmdEndRenderPass", reinterpret_cast<PFN_vkVoidFunction>(CmdEndRenderPass) },
        { "vkCmdExecuteCommands", reinterpret_cast<PFN_vkVoidFunction>(CmdExecuteCommands) },
        { "vkCmdFillBuffer", reinterpret_cast<PFN_vkVoidFunction>(CmdFillBuffer) },
        { "vkCmdNextSubpass", reinterpret_cast<PFN_vkVoidFunction>(CmdNextSubpass) },
        { "vkCmdPipelineBarrier", reinterpret_cast<PFN_vkVoidFunction>(CmdPipelineBarrier) },
        { "vkCmdPushConstants", reinterpret_cast<PFN_vkVoidFunction>(CmdPushConstants) },
        { "vkCmdResetEvent", reinterpret_cast<PFN_vkVoidFunction>(CmdResetEvent) },
        { "vkCmdResetQueryPool", reinterpret_cast<PFN_vkVoidFunction>(CmdResetQueryPool) },
        { "vkCmdResolveImage", reinterpret_cast<PFN_vkVoidFunction>(CmdResolveImage) },
        { "vkCmdSetBlendConstants", reinterpret_cast<PFN_vkVoidFunction>(CmdSetBlendConstants) },
        { "vkCmdSetDepthBias", reinterpret_cast<PFN_vkVoidFunction>(CmdSetDepthBias) },
        { "vkCmdSetDepthBounds", reinterpret_cast<PFN_vkVoidFunction>(CmdSetDepthBounds) },
        { "vkCmdSetDeviceMask", reinterpret_cast<PFN_vkVoidFunction>(CmdSetDeviceMask) },
        { "vkCmdSetEvent", reinterpret_cast<PFN_vkVoidFunction>(CmdSetEvent) },
        { "vkCmdSetLineWidth", reinterpret_cast<PFN_vkVoidFunction>(CmdSetLineWidth) },
        { "vkCmdSetScissor", reinterpret_cast<PFN_vkVoidFunction>(CmdSetScissor) },
        { "vkCmdSetStencilCompareMask", reinterpret_cast<PFN_vkVoidFunction>(CmdSetStencilCompareMask) },
        { "vkCmdSetStencilReference", reinterpret_cast<PFN_vkVoidFunction>(CmdSetStencilReference) },
        { "vkCmdSetStencilWriteMask", reinterpret_cast<PFN_vkVoidFunction>(CmdSetStencilWriteMask) },
        { "vkCmdSetViewport", reinterpret_cast<PFN_vkVoidFunction>(CmdSetViewport) },
        { "vkCmdUpdateBuffer", reinterpret_cast<PFN_vkVoidFunction>(CmdUpdateBuffer) },
        { "vkCmdWaitEvents", reinterpret_cast<PFN_vkVoidFunction>(CmdWaitEvents) },
        { "vkCmdWriteTimestamp", reinterpret_cast<PFN_vkVoidFunction>(CmdWriteTimestamp) },
        { "vkCreateBuffer", reinterpret_cast<PFN_vkVoidFunction>(CreateBuffer) },
        { "vkCreateBufferView", reinterpret_cast<PFN_vkVoidFunction>(CreateBufferView) },
        { "vkCreateCommandPool", reinterpret_cast<PFN_vkVoidFunction>(CreateCommandPool) },
        { "vkCreateComputePipelines", reinterpret_cast<PFN_vkVoidFunction>(CreateComputePipelines) },
        { "vkCreateDescriptorPool", reinterpret_cast<PFN_vkVoidFunction>(CreateDescriptorPool) },
        { "vkCreateDescriptorSetLayout", reinterpret_cast<PFN_vkVoidFunction>(CreateDescriptorSetLayout) },
        { "vkCreateDescriptorUpdateTemplate", reinterpret_cast<PFN_vkVoidFunction>(CreateDescriptorUpdateTemplate) },
        { "vkCreateDevice", reinterpret_cast<PFN_vkVoidFunction>(CreateDevice) },
        { "vkCreateEvent", reinterpret_cast<PFN_vkVoidFunction>(CreateEvent) },
        { "vkCreateFence", reinterpret_cast<PFN_vkVoidFunction>(CreateFence) },
        { "vkCreateFramebuffer", reinterpret_cast<PFN_vkVoidFunction>(CreateFramebuffer) },
        { "vkCreateGraphicsPipelines", reinterpret_cast<PFN_vkVoidFunction>(CreateGraphicsPipelines) },
        { "vkCreateImage", reinterpret_cast<PFN_vkVoidFunction>(CreateImage) },
        { "vkCreateImageView", reinterpret_cast<PFN_vkVoidFunction>(CreateImageView) },
        { "vkCreateInstance", nullptr },
        { "vkCreatePipelineCache", reinterpret_cast<PFN_vkVoidFunction>(CreatePipelineCache) },
        { "vkCreatePipelineLayout", reinterpret_cast<PFN_vkVoidFunction>(CreatePipelineLayout) },
        { "vkCreateQueryPool", reinterpret_cast<PFN_vkVoidFunction>(CreateQueryPool) },
        { "vkCreateRenderPass", reinterpret_cast<PFN_vkVoidFunction>(CreateRenderPass) },
        { "vkCreateSampler", reinterpret_cast<PFN_vkVoidFunction>(CreateSampler) },
        { "vkCreateSamplerYcbcrConversion", reinterpret_cast<PFN_vkVoidFunction>(CreateSamplerYcbcrConversion) },
        { "vkCreateSemaphore", reinterpret_cast<PFN_vkVoidFunction>(CreateSemaphore) },
        { "vkCreateShaderModule", reinterpret_cast<PFN_vkVoidFunction>(CreateShaderModule) },
        { "vkCreateSwapchainKHR", reinterpret_cast<PFN_vkVoidFunction>(CreateSwapchainKHR) },
        { "vkDestroyBuffer", reinterpret_cast<PFN_vkVoidFunction>(DestroyBuffer) },
        { "vkDestroyBufferView", reinterpret_cast<PFN_vkVoidFunction>(DestroyBufferView) },
        { "vkDestroyCommandPool", reinterpret_cast<PFN_vkVoidFunction>(DestroyCommandPool) },
        { "vkDestroyDescriptorPool", reinterpret_cast<PFN_vkVoidFunction>(DestroyDescriptorPool) },
        { "vkDestroyDescriptorSetLayout", reinterpret_cast<PFN_vkVoidFunction>(DestroyDescriptorSetLayout) },
        { "vkDestroyDescriptorUpdateTemplate", reinterpret_cast<PFN_vkVoidFunction>(DestroyDescriptorUpdateTemplate) },
        { "vkDestroyDevice", reinterpret_cast<PFN_vkVoidFunction>(DestroyDevice) },
        { "vkDestroyEvent", reinterpret_cast<PFN_vkVoidFunction>(DestroyEvent) },
        { "vkDestroyFence", reinterpret_cast<PFN_vkVoidFunction>(DestroyFence) },
        { "vkDestroyFramebuffer", reinterpret_cast<PFN_vkVoidFunction>(DestroyFramebuffer) },
        { "vkDestroyImage", reinterpret_cast<PFN_vkVoidFunction>(DestroyImage) },
        { "vkDestroyImageView", reinterpret_cast<PFN_vkVoidFunction>(DestroyImageView) },
        { "vkDestroyInstance", reinterpret_cast<PFN_vkVoidFunction>(DestroyInstance) },
        { "vkDestroyPipeline", reinterpret_cast<PFN_vkVoidFunction>(DestroyPipeline) },
        { "vkDestroyPipelineCache", reinterpret_cast<PFN_vkVoidFunction>(DestroyPipelineCache) },
        { "vkDestroyPipelineLayout", reinterpret_cast<PFN_vkVoidFunction>(DestroyPipelineLayout) },
        { "vkDestroyQueryPool", reinterpret_cast<PFN_vkVoidFunction>(DestroyQueryPool) },
        { "vkDestroyRenderPass", reinterpret_cast<PFN_vkVoidFunction>(DestroyRenderPass) },
        { "vkDestroySampler", reinterpret_cast<PFN_vkVoidFunction>(DestroySampler) },
        { "vkDestroySamplerYcbcrConversion", reinterpret_cast<PFN_vkVoidFunction>(DestroySamplerYcbcrConversion) },
        { "vkDestroySemaphore", reinterpret_cast<PFN_vkVoidFunction>(DestroySemaphore) },
        { "vkDestroyShaderModule", reinterpret_cast<PFN_vkVoidFunction>(DestroyShaderModule) },
        { "vkDestroySwapchainKHR", reinterpret_cast<PFN_vkVoidFunction>(DestroySwapchainKHR) },
        { "vkDeviceWaitIdle", reinterpret_cast<PFN_vkVoidFunction>(DeviceWaitIdle) },
        { "vkEndCommandBuffer", reinterpret_cast<PFN_vkVoidFunction>(EndCommandBuffer) },
        { "vkEnumerateDeviceExtensionProperties", reinterpret_cast<PFN_vkVoidFunction>(EnumerateDeviceExtensionProperties) },
        { "vkEnumerateDeviceLayerProperties", reinterpret_cast<PFN_vkVoidFunction>(EnumerateDeviceLayerProperties) },
        { "vkEnumerateInstanceExtensionProperties", nullptr },
        { "vkEnumerateInstanceLayerProperties", nullptr },
        { "vkEnumerateInstanceVersion", nullptr },
        { "vkFlushMappedMemoryRanges", reinterpret_cast<PFN_vkVoidFunction>(FlushMappedMemoryRanges) },
        { "vkFreeCommandBuffers", reinterpret_cast<PFN_vkVoidFunction>(FreeCommandBuffers) },
        { "vkFreeDescriptorSets", reinterpret_cast<PFN_vkVoidFunction>(FreeDescriptorSets) },
        { "vkFreeMemory", reinterpret_cast<PFN_vkVoidFunction>(FreeMemory) },
        { "vkGetAndroidHardwareBufferPropertiesANDROID", reinterpret_cast<PFN_vkVoidFunction>(GetAndroidHardwareBufferPropertiesANDROID) },
        { "vkGetBufferMemoryRequirements", reinterpret_cast<PFN_vkVoidFunction>(GetBufferMemoryRequirements) },
        { "vkGetBufferMemoryRequirements2", reinterpret_cast<PFN_vkVoidFunction>(GetBufferMemoryRequirements2) },
        { "vkGetDescriptorSetLayoutSupport", reinterpret_cast<PFN_vkVoidFunction>(GetDescriptorSetLayoutSupport) },
        { "vkGetDeviceGroupPeerMemoryFeatures", reinterpret_cast<PFN_vkVoidFunction>(GetDeviceGroupPeerMemoryFeatures) },
        { "vkGetDeviceGroupPresentCapabilitiesKHR", reinterpret_cast<PFN_vkVoidFunction>(GetDeviceGroupPresentCapabilitiesKHR) },
        { "vkGetDeviceGroupSurfacePresentModesKHR", reinterpret_cast<PFN_vkVoidFunction>(GetDeviceGroupSurfacePresentModesKHR) },
        { "vkGetDeviceMemoryCommitment", reinterpret_cast<PFN_vkVoidFunction>(GetDeviceMemoryCommitment) },
        { "vkGetDeviceProcAddr", reinterpret_cast<PFN_vkVoidFunction>(GetDeviceProcAddr) },
        { "vkGetDeviceQueue", reinterpret_cast<PFN_vkVoidFunction>(GetDeviceQueue) },
        { "vkGetDeviceQueue2", reinterpret_cast<PFN_vkVoidFunction>(GetDeviceQueue2) },
        { "vkGetEventStatus", reinterpret_cast<PFN_vkVoidFunction>(GetEventStatus) },
        { "vkGetFenceStatus", reinterpret_cast<PFN_vkVoidFunction>(GetFenceStatus) },
        { "vkGetImageMemoryRequirements", reinterpret_cast<PFN_vkVoidFunction>(GetImageMemoryRequirements) },
        { "vkGetImageMemoryRequirements2", reinterpret_cast<PFN_vkVoidFunction>(GetImageMemoryRequirements2) },
        { "vkGetImageSparseMemoryRequirements", reinterpret_cast<PFN_vkVoidFunction>(GetImageSparseMemoryRequirements) },
        { "vkGetImageSparseMemoryRequirements2", reinterpret_cast<PFN_vkVoidFunction>(GetImageSparseMemoryRequirements2) },
        { "vkGetImageSubresourceLayout", reinterpret_cast<PFN_vkVoidFunction>(GetImageSubresourceLayout) },
        { "vkGetInstanceProcAddr", reinterpret_cast<PFN_vkVoidFunction>(GetInstanceProcAddr) },
        { "vkGetMemoryAndroidHardwareBufferANDROID", reinterpret_cast<PFN_vkVoidFunction>(GetMemoryAndroidHardwareBufferANDROID) },
        { "vkGetPipelineCacheData", reinterpret_cast<PFN_vkVoidFunction>(GetPipelineCacheData) },
        { "vkGetQueryPoolResults", reinterpret_cast<PFN_vkVoidFunction>(GetQueryPoolResults) },
        { "vkGetRenderAreaGranularity", reinterpret_cast<PFN_vkVoidFunction>(GetRenderAreaGranularity) },
        { "vkGetSwapchainImagesKHR", reinterpret_cast<PFN_vkVoidFunction>(GetSwapchainImagesKHR) },
        { "vkInvalidateMappedMemoryRanges", reinterpret_cast<PFN_vkVoidFunction>(InvalidateMappedMemoryRanges) },
        { "vkMapMemory", reinterpret_cast<PFN_vkVoidFunction>(MapMemory) },
        { "vkMergePipelineCaches", reinterpret_cast<PFN_vkVoidFunction>(MergePipelineCaches) },
        { "vkQueueBindSparse", reinterpret_cast<PFN_vkVoidFunction>(QueueBindSparse) },
        { "vkQueuePresentKHR", reinterpret_cast<PFN_vkVoidFunction>(QueuePresentKHR) },
        { "vkQueueSubmit", reinterpret_cast<PFN_vkVoidFunction>(QueueSubmit) },
        { "vkQueueWaitIdle", reinterpret_cast<PFN_vkVoidFunction>(QueueWaitIdle) },
        { "vkResetCommandBuffer", reinterpret_cast<PFN_vkVoidFunction>(ResetCommandBuffer) },
        { "vkResetCommandPool", reinterpret_cast<PFN_vkVoidFunction>(ResetCommandPool) },
        { "vkResetDescriptorPool", reinterpret_cast<PFN_vkVoidFunction>(ResetDescriptorPool) },
        { "vkResetEvent", reinterpret_cast<PFN_vkVoidFunction>(ResetEvent) },
        { "vkResetFences", reinterpret_cast<PFN_vkVoidFunction>(ResetFences) },
        { "vkSetEvent", reinterpret_cast<PFN_vkVoidFunction>(SetEvent) },
        { "vkTrimCommandPool", reinterpret_cast<PFN_vkVoidFunction>(TrimCommandPool) },
        { "vkUnmapMemory", reinterpret_cast<PFN_vkVoidFunction>(UnmapMemory) },
        { "vkUpdateDescriptorSetWithTemplate", reinterpret_cast<PFN_vkVoidFunction>(UpdateDescriptorSetWithTemplate) },
        { "vkUpdateDescriptorSets", reinterpret_cast<PFN_vkVoidFunction>(UpdateDescriptorSets) },
        { "vkWaitForFences", reinterpret_cast<PFN_vkVoidFunction>(WaitForFences) },
    };
    // clang-format on
    constexpr size_t count = sizeof(hooks) / sizeof(hooks[0]);
    auto hook = std::lower_bound(
        hooks, hooks + count, pName,
        [](const Hook& h, const char* n) { return strcmp(h.name, n) < 0; });
    if (hook < hooks + count && strcmp(hook->name, pName) == 0) {
        if (!hook->proc) {
            vulkan::driver::Logger(instance).Err(
                instance, "invalid vkGetInstanceProcAddr(%p, \"%s\") call",
                instance, pName);
        }
        return hook->proc;
    }
    // clang-format off

    return GetData(instance).dispatch.GetInstanceProcAddr(instance, pName);
}

VKAPI_ATTR void GetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceProperties(physicalDevice, pProperties);
}

VKAPI_ATTR void GetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKAPI_ATTR void GetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

VKAPI_ATTR void GetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

VKAPI_ATTR void GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

VKAPI_ATTR VkResult GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties) {
    return GetData(physicalDevice).dispatch.GetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}

VKAPI_ATTR void GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    GetData(device).dispatch.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

VKAPI_ATTR VkResult QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    return GetData(queue).dispatch.QueueSubmit(queue, submitCount, pSubmits, fence);
}

VKAPI_ATTR VkResult QueueWaitIdle(VkQueue queue) {
    return GetData(queue).dispatch.QueueWaitIdle(queue);
}

VKAPI_ATTR VkResult DeviceWaitIdle(VkDevice device) {
    return GetData(device).dispatch.DeviceWaitIdle(device);
}

VKAPI_ATTR VkResult AllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
    return GetData(device).dispatch.AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}

VKAPI_ATTR void FreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.FreeMemory(device, memory, pAllocator);
}

VKAPI_ATTR VkResult MapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) {
    return GetData(device).dispatch.MapMemory(device, memory, offset, size, flags, ppData);
}

VKAPI_ATTR void UnmapMemory(VkDevice device, VkDeviceMemory memory) {
    GetData(device).dispatch.UnmapMemory(device, memory);
}

VKAPI_ATTR VkResult FlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) {
    return GetData(device).dispatch.FlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

VKAPI_ATTR VkResult InvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) {
    return GetData(device).dispatch.InvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

VKAPI_ATTR void GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) {
    GetData(device).dispatch.GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

VKAPI_ATTR void GetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
    GetData(device).dispatch.GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

VKAPI_ATTR VkResult BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    return GetData(device).dispatch.BindBufferMemory(device, buffer, memory, memoryOffset);
}

VKAPI_ATTR void GetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) {
    GetData(device).dispatch.GetImageMemoryRequirements(device, image, pMemoryRequirements);
}

VKAPI_ATTR VkResult BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    return GetData(device).dispatch.BindImageMemory(device, image, memory, memoryOffset);
}

VKAPI_ATTR void GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
    GetData(device).dispatch.GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VKAPI_ATTR void GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}

VKAPI_ATTR VkResult QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) {
    return GetData(queue).dispatch.QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}

VKAPI_ATTR VkResult CreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    return GetData(device).dispatch.CreateFence(device, pCreateInfo, pAllocator, pFence);
}

VKAPI_ATTR void DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyFence(device, fence, pAllocator);
}

VKAPI_ATTR VkResult ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) {
    return GetData(device).dispatch.ResetFences(device, fenceCount, pFences);
}

VKAPI_ATTR VkResult GetFenceStatus(VkDevice device, VkFence fence) {
    return GetData(device).dispatch.GetFenceStatus(device, fence);
}

VKAPI_ATTR VkResult WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) {
    return GetData(device).dispatch.WaitForFences(device, fenceCount, pFences, waitAll, timeout);
}

VKAPI_ATTR VkResult CreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) {
    return GetData(device).dispatch.CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
}

VKAPI_ATTR void DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroySemaphore(device, semaphore, pAllocator);
}

VKAPI_ATTR VkResult CreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) {
    return GetData(device).dispatch.CreateEvent(device, pCreateInfo, pAllocator, pEvent);
}

VKAPI_ATTR void DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyEvent(device, event, pAllocator);
}

VKAPI_ATTR VkResult GetEventStatus(VkDevice device, VkEvent event) {
    return GetData(device).dispatch.GetEventStatus(device, event);
}

VKAPI_ATTR VkResult SetEvent(VkDevice device, VkEvent event) {
    return GetData(device).dispatch.SetEvent(device, event);
}

VKAPI_ATTR VkResult ResetEvent(VkDevice device, VkEvent event) {
    return GetData(device).dispatch.ResetEvent(device, event);
}

VKAPI_ATTR VkResult CreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) {
    return GetData(device).dispatch.CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
}

VKAPI_ATTR void DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyQueryPool(device, queryPool, pAllocator);
}

VKAPI_ATTR VkResult GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) {
    return GetData(device).dispatch.GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}

VKAPI_ATTR VkResult CreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) {
    return GetData(device).dispatch.CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}

VKAPI_ATTR void DestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyBuffer(device, buffer, pAllocator);
}

VKAPI_ATTR VkResult CreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView) {
    return GetData(device).dispatch.CreateBufferView(device, pCreateInfo, pAllocator, pView);
}

VKAPI_ATTR void DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyBufferView(device, bufferView, pAllocator);
}

VKAPI_ATTR VkResult CreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) {
    return GetData(device).dispatch.CreateImage(device, pCreateInfo, pAllocator, pImage);
}

VKAPI_ATTR void DestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyImage(device, image, pAllocator);
}

VKAPI_ATTR void GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) {
    GetData(device).dispatch.GetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

VKAPI_ATTR VkResult CreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) {
    return GetData(device).dispatch.CreateImageView(device, pCreateInfo, pAllocator, pView);
}

VKAPI_ATTR void DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyImageView(device, imageView, pAllocator);
}

VKAPI_ATTR VkResult CreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) {
    return GetData(device).dispatch.CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
}

VKAPI_ATTR void DestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyShaderModule(device, shaderModule, pAllocator);
}

VKAPI_ATTR VkResult CreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) {
    return GetData(device).dispatch.CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
}

VKAPI_ATTR void DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyPipelineCache(device, pipelineCache, pAllocator);
}

VKAPI_ATTR VkResult GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) {
    return GetData(device).dispatch.GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
}

VKAPI_ATTR VkResult MergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches) {
    return GetData(device).dispatch.MergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
}

VKAPI_ATTR VkResult CreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    return GetData(device).dispatch.CreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VKAPI_ATTR VkResult CreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    return GetData(device).dispatch.CreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VKAPI_ATTR void DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyPipeline(device, pipeline, pAllocator);
}

VKAPI_ATTR VkResult CreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) {
    return GetData(device).dispatch.CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
}

VKAPI_ATTR void DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

VKAPI_ATTR VkResult CreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) {
    return GetData(device).dispatch.CreateSampler(device, pCreateInfo, pAllocator, pSampler);
}

VKAPI_ATTR void DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroySampler(device, sampler, pAllocator);
}

VKAPI_ATTR VkResult CreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) {
    return GetData(device).dispatch.CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
}

VKAPI_ATTR void DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

VKAPI_ATTR VkResult CreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) {
    return GetData(device).dispatch.CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
}

VKAPI_ATTR void DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyDescriptorPool(device, descriptorPool, pAllocator);
}

VKAPI_ATTR VkResult ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) {
    return GetData(device).dispatch.ResetDescriptorPool(device, descriptorPool, flags);
}

VKAPI_ATTR VkResult AllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets) {
    return GetData(device).dispatch.AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
}

VKAPI_ATTR VkResult FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) {
    return GetData(device).dispatch.FreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
}

VKAPI_ATTR void UpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) {
    GetData(device).dispatch.UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

VKAPI_ATTR VkResult CreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) {
    return GetData(device).dispatch.CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
}

VKAPI_ATTR void DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyFramebuffer(device, framebuffer, pAllocator);
}

VKAPI_ATTR VkResult CreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    return GetData(device).dispatch.CreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
}

VKAPI_ATTR void DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyRenderPass(device, renderPass, pAllocator);
}

VKAPI_ATTR void GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    GetData(device).dispatch.GetRenderAreaGranularity(device, renderPass, pGranularity);
}

VKAPI_ATTR VkResult CreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) {
    return GetData(device).dispatch.CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
}

VKAPI_ATTR void DestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyCommandPool(device, commandPool, pAllocator);
}

VKAPI_ATTR VkResult ResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) {
    return GetData(device).dispatch.ResetCommandPool(device, commandPool, flags);
}

VKAPI_ATTR VkResult AllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) {
    return GetData(device).dispatch.AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
}

VKAPI_ATTR void FreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    GetData(device).dispatch.FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

VKAPI_ATTR VkResult BeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) {
    return GetData(commandBuffer).dispatch.BeginCommandBuffer(commandBuffer, pBeginInfo);
}

VKAPI_ATTR VkResult EndCommandBuffer(VkCommandBuffer commandBuffer) {
    return GetData(commandBuffer).dispatch.EndCommandBuffer(commandBuffer);
}

VKAPI_ATTR VkResult ResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
    return GetData(commandBuffer).dispatch.ResetCommandBuffer(commandBuffer, flags);
}

VKAPI_ATTR void CmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
    GetData(commandBuffer).dispatch.CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

VKAPI_ATTR void CmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) {
    GetData(commandBuffer).dispatch.CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

VKAPI_ATTR void CmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) {
    GetData(commandBuffer).dispatch.CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

VKAPI_ATTR void CmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    GetData(commandBuffer).dispatch.CmdSetLineWidth(commandBuffer, lineWidth);
}

VKAPI_ATTR void CmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
    GetData(commandBuffer).dispatch.CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

VKAPI_ATTR void CmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    GetData(commandBuffer).dispatch.CmdSetBlendConstants(commandBuffer, blendConstants);
}

VKAPI_ATTR void CmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    GetData(commandBuffer).dispatch.CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

VKAPI_ATTR void CmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) {
    GetData(commandBuffer).dispatch.CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

VKAPI_ATTR void CmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) {
    GetData(commandBuffer).dispatch.CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

VKAPI_ATTR void CmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) {
    GetData(commandBuffer).dispatch.CmdSetStencilReference(commandBuffer, faceMask, reference);
}

VKAPI_ATTR void CmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
    GetData(commandBuffer).dispatch.CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

VKAPI_ATTR void CmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) {
    GetData(commandBuffer).dispatch.CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

VKAPI_ATTR void CmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
    GetData(commandBuffer).dispatch.CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

VKAPI_ATTR void CmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    GetData(commandBuffer).dispatch.CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

VKAPI_ATTR void CmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    GetData(commandBuffer).dispatch.CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

VKAPI_ATTR void CmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    GetData(commandBuffer).dispatch.CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

VKAPI_ATTR void CmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    GetData(commandBuffer).dispatch.CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

VKAPI_ATTR void CmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    GetData(commandBuffer).dispatch.CmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

VKAPI_ATTR void CmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    GetData(commandBuffer).dispatch.CmdDispatchIndirect(commandBuffer, buffer, offset);
}

VKAPI_ATTR void CmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
    GetData(commandBuffer).dispatch.CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

VKAPI_ATTR void CmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) {
    GetData(commandBuffer).dispatch.CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

VKAPI_ATTR void CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) {
    GetData(commandBuffer).dispatch.CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

VKAPI_ATTR void CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    GetData(commandBuffer).dispatch.CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

VKAPI_ATTR void CmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    GetData(commandBuffer).dispatch.CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

VKAPI_ATTR void CmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) {
    GetData(commandBuffer).dispatch.CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

VKAPI_ATTR void CmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) {
    GetData(commandBuffer).dispatch.CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

VKAPI_ATTR void CmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    GetData(commandBuffer).dispatch.CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

VKAPI_ATTR void CmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    GetData(commandBuffer).dispatch.CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

VKAPI_ATTR void CmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) {
    GetData(commandBuffer).dispatch.CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

VKAPI_ATTR void CmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) {
    GetData(commandBuffer).dispatch.CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

VKAPI_ATTR void CmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    GetData(commandBuffer).dispatch.CmdSetEvent(commandBuffer, event, stageMask);
}

VKAPI_ATTR void CmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    GetData(commandBuffer).dispatch.CmdResetEvent(commandBuffer, event, stageMask);
}

VKAPI_ATTR void CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    GetData(commandBuffer).dispatch.CmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

VKAPI_ATTR void CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    GetData(commandBuffer).dispatch.CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

VKAPI_ATTR void CmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) {
    GetData(commandBuffer).dispatch.CmdBeginQuery(commandBuffer, queryPool, query, flags);
}

VKAPI_ATTR void CmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) {
    GetData(commandBuffer).dispatch.CmdEndQuery(commandBuffer, queryPool, query);
}

VKAPI_ATTR void CmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    GetData(commandBuffer).dispatch.CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

VKAPI_ATTR void CmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) {
    GetData(commandBuffer).dispatch.CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

VKAPI_ATTR void CmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) {
    GetData(commandBuffer).dispatch.CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

VKAPI_ATTR void CmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) {
    GetData(commandBuffer).dispatch.CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

VKAPI_ATTR void CmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) {
    GetData(commandBuffer).dispatch.CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}

VKAPI_ATTR void CmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    GetData(commandBuffer).dispatch.CmdNextSubpass(commandBuffer, contents);
}

VKAPI_ATTR void CmdEndRenderPass(VkCommandBuffer commandBuffer) {
    GetData(commandBuffer).dispatch.CmdEndRenderPass(commandBuffer);
}

VKAPI_ATTR void CmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    GetData(commandBuffer).dispatch.CmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}

VKAPI_ATTR VkResult BindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
    return GetData(device).dispatch.BindBufferMemory2(device, bindInfoCount, pBindInfos);
}

VKAPI_ATTR VkResult BindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    return GetData(device).dispatch.BindImageMemory2(device, bindInfoCount, pBindInfos);
}

VKAPI_ATTR void GetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    GetData(device).dispatch.GetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

VKAPI_ATTR void CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    GetData(commandBuffer).dispatch.CmdSetDeviceMask(commandBuffer, deviceMask);
}

VKAPI_ATTR void CmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    GetData(commandBuffer).dispatch.CmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

VKAPI_ATTR VkResult EnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    return GetData(instance).dispatch.EnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}

VKAPI_ATTR void GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    GetData(device).dispatch.GetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    GetData(device).dispatch.GetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

VKAPI_ATTR void GetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    GetData(device).dispatch.GetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

VKAPI_ATTR void GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
}

VKAPI_ATTR void GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceProperties2(physicalDevice, pProperties);
}

VKAPI_ATTR void GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
}

VKAPI_ATTR VkResult GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) {
    return GetData(physicalDevice).dispatch.GetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
}

VKAPI_ATTR void GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

VKAPI_ATTR void GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
}

VKAPI_ATTR void GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

VKAPI_ATTR void TrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    GetData(device).dispatch.TrimCommandPool(device, commandPool, flags);
}

VKAPI_ATTR void GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {
    GetData(device).dispatch.GetDeviceQueue2(device, pQueueInfo, pQueue);
}

VKAPI_ATTR VkResult CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) {
    return GetData(device).dispatch.CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
}

VKAPI_ATTR void DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}

VKAPI_ATTR VkResult CreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    return GetData(device).dispatch.CreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}

VKAPI_ATTR void DestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
}

VKAPI_ATTR void UpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
    GetData(device).dispatch.UpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
}

VKAPI_ATTR void GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}

VKAPI_ATTR void GetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}

VKAPI_ATTR void GetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    GetData(physicalDevice).dispatch.GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}

VKAPI_ATTR void GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {
    GetData(device).dispatch.GetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
}

VKAPI_ATTR void DestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) {
    GetData(instance).dispatch.DestroySurfaceKHR(instance, surface, pAllocator);
}

VKAPI_ATTR VkResult GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) {
    return GetData(physicalDevice).dispatch.GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
}

VKAPI_ATTR VkResult GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
    return GetData(physicalDevice).dispatch.GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
}

VKAPI_ATTR VkResult GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) {
    return GetData(physicalDevice).dispatch.GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
}

VKAPI_ATTR VkResult GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
    return GetData(physicalDevice).dispatch.GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
}

VKAPI_ATTR VkResult CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
    return GetData(device).dispatch.CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
}

VKAPI_ATTR void DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) {
    GetData(device).dispatch.DestroySwapchainKHR(device, swapchain, pAllocator);
}

VKAPI_ATTR VkResult GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) {
    return GetData(device).dispatch.GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
}

VKAPI_ATTR VkResult AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) {
    return GetData(device).dispatch.AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
}

VKAPI_ATTR VkResult QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    return GetData(queue).dispatch.QueuePresentKHR(queue, pPresentInfo);
}

VKAPI_ATTR VkResult GetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) {
    return GetData(device).dispatch.GetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
}

VKAPI_ATTR VkResult GetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes) {
    return GetData(device).dispatch.GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
}

VKAPI_ATTR VkResult GetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects) {
    return GetData(physicalDevice).dispatch.GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
}

VKAPI_ATTR VkResult AcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex) {
    return GetData(device).dispatch.AcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
}

VKAPI_ATTR VkResult CreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return GetData(instance).dispatch.CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

VKAPI_ATTR VkResult GetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties) {
    return GetData(device).dispatch.GetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
}

VKAPI_ATTR VkResult GetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo, struct AHardwareBuffer** pBuffer) {
    return GetData(device).dispatch.GetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
}


}  // anonymous namespace

// clang-format on

}  // namespace api
}  // namespace vulkan

// clang-format off

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) {
    return vulkan::api::CreateInstance(pCreateInfo, pAllocator, pInstance);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyInstance(instance, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) {
    return vulkan::api::EnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
}

__attribute__((visibility("default")))
VKAPI_ATTR PFN_vkVoidFunction vkGetDeviceProcAddr(VkDevice device, const char* pName) {
    return vulkan::api::GetDeviceProcAddr(device, pName);
}

__attribute__((visibility("default")))
VKAPI_ATTR PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
    return vulkan::api::GetInstanceProcAddr(instance, pName);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
    vulkan::api::GetPhysicalDeviceProperties(physicalDevice, pProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) {
    vulkan::api::GetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    vulkan::api::GetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    vulkan::api::GetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties) {
    vulkan::api::GetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties) {
    return vulkan::api::GetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    return vulkan::api::CreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyDevice(device, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkEnumerateInstanceLayerProperties(uint32_t* pPropertyCount, VkLayerProperties* pProperties) {
    return vulkan::api::EnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkEnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    return vulkan::api::EnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pPropertyCount, VkLayerProperties* pProperties) {
    return vulkan::api::EnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    return vulkan::api::EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    vulkan::api::GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    return vulkan::api::QueueSubmit(queue, submitCount, pSubmits, fence);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkQueueWaitIdle(VkQueue queue) {
    return vulkan::api::QueueWaitIdle(queue);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkDeviceWaitIdle(VkDevice device) {
    return vulkan::api::DeviceWaitIdle(device);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
    return vulkan::api::AllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::FreeMemory(device, memory, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) {
    return vulkan::api::MapMemory(device, memory, offset, size, flags, ppData);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkUnmapMemory(VkDevice device, VkDeviceMemory memory) {
    vulkan::api::UnmapMemory(device, memory);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) {
    return vulkan::api::FlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange* pMemoryRanges) {
    return vulkan::api::InvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) {
    vulkan::api::GetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
    vulkan::api::GetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    return vulkan::api::BindBufferMemory(device, buffer, memory, memoryOffset);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements* pMemoryRequirements) {
    vulkan::api::GetImageMemoryRequirements(device, image, pMemoryRequirements);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    return vulkan::api::BindImageMemory(device, image, memory, memoryOffset);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
    vulkan::api::GetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pPropertyCount, VkSparseImageFormatProperties* pProperties) {
    vulkan::api::GetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) {
    return vulkan::api::QueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    return vulkan::api::CreateFence(device, pCreateInfo, pAllocator, pFence);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyFence(device, fence, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) {
    return vulkan::api::ResetFences(device, fenceCount, pFences);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetFenceStatus(VkDevice device, VkFence fence) {
    return vulkan::api::GetFenceStatus(device, fence);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) {
    return vulkan::api::WaitForFences(device, fenceCount, pFences, waitAll, timeout);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) {
    return vulkan::api::CreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroySemaphore(device, semaphore, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateEvent(VkDevice device, const VkEventCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkEvent* pEvent) {
    return vulkan::api::CreateEvent(device, pCreateInfo, pAllocator, pEvent);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyEvent(device, event, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetEventStatus(VkDevice device, VkEvent event) {
    return vulkan::api::GetEventStatus(device, event);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkSetEvent(VkDevice device, VkEvent event) {
    return vulkan::api::SetEvent(device, event);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkResetEvent(VkDevice device, VkEvent event) {
    return vulkan::api::ResetEvent(device, event);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkQueryPool* pQueryPool) {
    return vulkan::api::CreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyQueryPool(device, queryPool, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) {
    return vulkan::api::GetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) {
    return vulkan::api::CreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyBuffer(device, buffer, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBufferView* pView) {
    return vulkan::api::CreateBufferView(device, pCreateInfo, pAllocator, pView);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyBufferView(device, bufferView, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateImage(VkDevice device, const VkImageCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImage* pImage) {
    return vulkan::api::CreateImage(device, pCreateInfo, pAllocator, pImage);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyImage(device, image, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) {
    vulkan::api::GetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) {
    return vulkan::api::CreateImageView(device, pCreateInfo, pAllocator, pView);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyImageView(device, imageView, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkShaderModule* pShaderModule) {
    return vulkan::api::CreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyShaderModule(device, shaderModule, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineCache* pPipelineCache) {
    return vulkan::api::CreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyPipelineCache(device, pipelineCache, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) {
    return vulkan::api::GetPipelineCacheData(device, pipelineCache, pDataSize, pData);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches) {
    return vulkan::api::MergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    return vulkan::api::CreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkComputePipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    return vulkan::api::CreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyPipeline(device, pipeline, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkPipelineLayout* pPipelineLayout) {
    return vulkan::api::CreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) {
    return vulkan::api::CreateSampler(device, pCreateInfo, pAllocator, pSampler);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroySampler(device, sampler, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorSetLayout* pSetLayout) {
    return vulkan::api::CreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorPool* pDescriptorPool) {
    return vulkan::api::CreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyDescriptorPool(device, descriptorPool, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) {
    return vulkan::api::ResetDescriptorPool(device, descriptorPool, flags);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo* pAllocateInfo, VkDescriptorSet* pDescriptorSets) {
    return vulkan::api::AllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets) {
    return vulkan::api::FreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount, const VkCopyDescriptorSet* pDescriptorCopies) {
    vulkan::api::UpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFramebuffer* pFramebuffer) {
    return vulkan::api::CreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyFramebuffer(device, framebuffer, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
    return vulkan::api::CreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyRenderPass(device, renderPass, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    vulkan::api::GetRenderAreaGranularity(device, renderPass, pGranularity);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) {
    return vulkan::api::CreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyCommandPool(device, commandPool, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags) {
    return vulkan::api::ResetCommandPool(device, commandPool, flags);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) {
    return vulkan::api::AllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    vulkan::api::FreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) {
    return vulkan::api::BeginCommandBuffer(commandBuffer, pBeginInfo);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkEndCommandBuffer(VkCommandBuffer commandBuffer) {
    return vulkan::api::EndCommandBuffer(commandBuffer);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags) {
    return vulkan::api::ResetCommandBuffer(commandBuffer, flags);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
    vulkan::api::CmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) {
    vulkan::api::CmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) {
    vulkan::api::CmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth) {
    vulkan::api::CmdSetLineWidth(commandBuffer, lineWidth);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor) {
    vulkan::api::CmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4]) {
    vulkan::api::CmdSetBlendConstants(commandBuffer, blendConstants);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds) {
    vulkan::api::CmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask) {
    vulkan::api::CmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask) {
    vulkan::api::CmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t reference) {
    vulkan::api::CmdSetStencilReference(commandBuffer, faceMask, reference);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
    vulkan::api::CmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) {
    vulkan::api::CmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
    vulkan::api::CmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    vulkan::api::CmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    vulkan::api::CmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    vulkan::api::CmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    vulkan::api::CmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    vulkan::api::CmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset) {
    vulkan::api::CmdDispatchIndirect(commandBuffer, buffer, offset);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
    vulkan::api::CmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) {
    vulkan::api::CmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) {
    vulkan::api::CmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    vulkan::api::CmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
    vulkan::api::CmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData) {
    vulkan::api::CmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data) {
    vulkan::api::CmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    vulkan::api::CmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
    vulkan::api::CmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) {
    vulkan::api::CmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) {
    vulkan::api::CmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    vulkan::api::CmdSetEvent(commandBuffer, event, stageMask);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
    vulkan::api::CmdResetEvent(commandBuffer, event, stageMask);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    vulkan::api::CmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
    vulkan::api::CmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags) {
    vulkan::api::CmdBeginQuery(commandBuffer, queryPool, query, flags);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query) {
    vulkan::api::CmdEndQuery(commandBuffer, queryPool, query);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount) {
    vulkan::api::CmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query) {
    vulkan::api::CmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags) {
    vulkan::api::CmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues) {
    vulkan::api::CmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) {
    vulkan::api::CmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents) {
    vulkan::api::CmdNextSubpass(commandBuffer, contents);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    vulkan::api::CmdEndRenderPass(commandBuffer);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers) {
    vulkan::api::CmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkEnumerateInstanceVersion(uint32_t* pApiVersion) {
    return vulkan::api::EnumerateInstanceVersion(pApiVersion);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkBindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
    return vulkan::api::BindBufferMemory2(device, bindInfoCount, pBindInfos);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkBindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    return vulkan::api::BindImageMemory2(device, bindInfoCount, pBindInfos);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
    vulkan::api::GetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
    vulkan::api::CmdSetDeviceMask(commandBuffer, deviceMask);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkCmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
    vulkan::api::CmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkEnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    return vulkan::api::EnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    vulkan::api::GetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
    vulkan::api::GetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
    vulkan::api::GetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    vulkan::api::GetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
    vulkan::api::GetPhysicalDeviceProperties2(physicalDevice, pProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {
    vulkan::api::GetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) {
    return vulkan::api::GetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
    vulkan::api::GetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
    vulkan::api::GetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
    vulkan::api::GetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkTrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
    vulkan::api::TrimCommandPool(device, commandPool, flags);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {
    vulkan::api::GetDeviceQueue2(device, pQueueInfo, pQueue);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) {
    return vulkan::api::CreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    return vulkan::api::CreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkUpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
    vulkan::api::UpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {
    vulkan::api::GetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {
    vulkan::api::GetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
    vulkan::api::GetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkGetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {
    vulkan::api::GetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroySurfaceKHR(instance, surface, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) {
    return vulkan::api::GetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
    return vulkan::api::GetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) {
    return vulkan::api::GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
    return vulkan::api::GetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
    return vulkan::api::CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
}

__attribute__((visibility("default")))
VKAPI_ATTR void vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) {
    vulkan::api::DestroySwapchainKHR(device, swapchain, pAllocator);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) {
    return vulkan::api::GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) {
    return vulkan::api::AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    return vulkan::api::QueuePresentKHR(queue, pPresentInfo);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) {
    return vulkan::api::GetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes) {
    return vulkan::api::GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetPhysicalDevicePresentRectanglesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pRectCount, VkRect2D* pRects) {
    return vulkan::api::GetPhysicalDevicePresentRectanglesKHR(physicalDevice, surface, pRectCount, pRects);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex) {
    return vulkan::api::AcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    return vulkan::api::CreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetAndroidHardwareBufferPropertiesANDROID(VkDevice device, const struct AHardwareBuffer* buffer, VkAndroidHardwareBufferPropertiesANDROID* pProperties) {
    return vulkan::api::GetAndroidHardwareBufferPropertiesANDROID(device, buffer, pProperties);
}

__attribute__((visibility("default")))
VKAPI_ATTR VkResult vkGetMemoryAndroidHardwareBufferANDROID(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID* pInfo, struct AHardwareBuffer** pBuffer) {
    return vulkan::api::GetMemoryAndroidHardwareBufferANDROID(device, pInfo, pBuffer);
}

// clang-format on
