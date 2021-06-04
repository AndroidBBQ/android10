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

#include "driver.h"

namespace vulkan {
namespace driver {

namespace {

// clang-format off

VKAPI_ATTR VkResult checkedCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
    if (GetData(device).hook_extensions[ProcHook::KHR_swapchain]) {
        return CreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    } else {
        Logger(device).Err(device, "VK_KHR_swapchain not enabled. vkCreateSwapchainKHR not executed.");
        return VK_SUCCESS;
    }
}

VKAPI_ATTR void checkedDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) {
    if (GetData(device).hook_extensions[ProcHook::KHR_swapchain]) {
        DestroySwapchainKHR(device, swapchain, pAllocator);
    } else {
        Logger(device).Err(device, "VK_KHR_swapchain not enabled. vkDestroySwapchainKHR not executed.");
    }
}

VKAPI_ATTR VkResult checkedGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) {
    if (GetData(device).hook_extensions[ProcHook::KHR_swapchain]) {
        return GetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    } else {
        Logger(device).Err(device, "VK_KHR_swapchain not enabled. vkGetSwapchainImagesKHR not executed.");
        return VK_SUCCESS;
    }
}

VKAPI_ATTR VkResult checkedAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) {
    if (GetData(device).hook_extensions[ProcHook::KHR_swapchain]) {
        return AcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    } else {
        Logger(device).Err(device, "VK_KHR_swapchain not enabled. vkAcquireNextImageKHR not executed.");
        return VK_SUCCESS;
    }
}

VKAPI_ATTR VkResult checkedQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    if (GetData(queue).hook_extensions[ProcHook::KHR_swapchain]) {
        return QueuePresentKHR(queue, pPresentInfo);
    } else {
        Logger(queue).Err(queue, "VK_KHR_swapchain not enabled. vkQueuePresentKHR not executed.");
        return VK_SUCCESS;
    }
}

VKAPI_ATTR VkResult checkedGetDeviceGroupPresentCapabilitiesKHR(VkDevice device, VkDeviceGroupPresentCapabilitiesKHR* pDeviceGroupPresentCapabilities) {
    if (GetData(device).hook_extensions[ProcHook::KHR_swapchain]) {
        return GetDeviceGroupPresentCapabilitiesKHR(device, pDeviceGroupPresentCapabilities);
    } else {
        Logger(device).Err(device, "VK_KHR_swapchain not enabled. vkGetDeviceGroupPresentCapabilitiesKHR not executed.");
        return VK_SUCCESS;
    }
}

VKAPI_ATTR VkResult checkedGetDeviceGroupSurfacePresentModesKHR(VkDevice device, VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR* pModes) {
    if (GetData(device).hook_extensions[ProcHook::KHR_swapchain]) {
        return GetDeviceGroupSurfacePresentModesKHR(device, surface, pModes);
    } else {
        Logger(device).Err(device, "VK_KHR_swapchain not enabled. vkGetDeviceGroupSurfacePresentModesKHR not executed.");
        return VK_SUCCESS;
    }
}

VKAPI_ATTR VkResult checkedAcquireNextImage2KHR(VkDevice device, const VkAcquireNextImageInfoKHR* pAcquireInfo, uint32_t* pImageIndex) {
    if (GetData(device).hook_extensions[ProcHook::KHR_swapchain]) {
        return AcquireNextImage2KHR(device, pAcquireInfo, pImageIndex);
    } else {
        Logger(device).Err(device, "VK_KHR_swapchain not enabled. vkAcquireNextImage2KHR not executed.");
        return VK_SUCCESS;
    }
}

VKAPI_ATTR VkResult checkedGetRefreshCycleDurationGOOGLE(VkDevice device, VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE* pDisplayTimingProperties) {
    if (GetData(device).hook_extensions[ProcHook::GOOGLE_display_timing]) {
        return GetRefreshCycleDurationGOOGLE(device, swapchain, pDisplayTimingProperties);
    } else {
        Logger(device).Err(device, "VK_GOOGLE_display_timing not enabled. vkGetRefreshCycleDurationGOOGLE not executed.");
        return VK_SUCCESS;
    }
}

VKAPI_ATTR VkResult checkedGetPastPresentationTimingGOOGLE(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pPresentationTimingCount, VkPastPresentationTimingGOOGLE* pPresentationTimings) {
    if (GetData(device).hook_extensions[ProcHook::GOOGLE_display_timing]) {
        return GetPastPresentationTimingGOOGLE(device, swapchain, pPresentationTimingCount, pPresentationTimings);
    } else {
        Logger(device).Err(device, "VK_GOOGLE_display_timing not enabled. vkGetPastPresentationTimingGOOGLE not executed.");
        return VK_SUCCESS;
    }
}

VKAPI_ATTR void checkedSetHdrMetadataEXT(VkDevice device, uint32_t swapchainCount, const VkSwapchainKHR* pSwapchains, const VkHdrMetadataEXT* pMetadata) {
    if (GetData(device).hook_extensions[ProcHook::EXT_hdr_metadata]) {
        SetHdrMetadataEXT(device, swapchainCount, pSwapchains, pMetadata);
    } else {
        Logger(device).Err(device, "VK_EXT_hdr_metadata not enabled. vkSetHdrMetadataEXT not executed.");
    }
}

VKAPI_ATTR VkResult checkedGetSwapchainStatusKHR(VkDevice device, VkSwapchainKHR swapchain) {
    if (GetData(device).hook_extensions[ProcHook::KHR_shared_presentable_image]) {
        return GetSwapchainStatusKHR(device, swapchain);
    } else {
        Logger(device).Err(device, "VK_KHR_shared_presentable_image not enabled. vkGetSwapchainStatusKHR not executed.");
        return VK_SUCCESS;
    }
}

VKAPI_ATTR VkResult checkedBindImageMemory2KHR(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfoKHR* pBindInfos) {
    if (GetData(device).hook_extensions[ProcHook::KHR_bind_memory2]) {
        return BindImageMemory2KHR(device, bindInfoCount, pBindInfos);
    } else {
        Logger(device).Err(device, "VK_KHR_bind_memory2 not enabled. vkBindImageMemory2KHR not executed.");
        return VK_SUCCESS;
    }
}

// clang-format on

const ProcHook g_proc_hooks[] = {
    // clang-format off
    {
        "vkAcquireImageANDROID",
        ProcHook::DEVICE,
        ProcHook::ANDROID_native_buffer,
        nullptr,
        nullptr,
    },
    {
        "vkAcquireNextImage2KHR",
        ProcHook::DEVICE,
        ProcHook::KHR_swapchain,
        reinterpret_cast<PFN_vkVoidFunction>(AcquireNextImage2KHR),
        reinterpret_cast<PFN_vkVoidFunction>(checkedAcquireNextImage2KHR),
    },
    {
        "vkAcquireNextImageKHR",
        ProcHook::DEVICE,
        ProcHook::KHR_swapchain,
        reinterpret_cast<PFN_vkVoidFunction>(AcquireNextImageKHR),
        reinterpret_cast<PFN_vkVoidFunction>(checkedAcquireNextImageKHR),
    },
    {
        "vkAllocateCommandBuffers",
        ProcHook::DEVICE,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(AllocateCommandBuffers),
        nullptr,
    },
    {
        "vkBindImageMemory2",
        ProcHook::DEVICE,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(BindImageMemory2),
        nullptr,
    },
    {
        "vkBindImageMemory2KHR",
        ProcHook::DEVICE,
        ProcHook::KHR_bind_memory2,
        reinterpret_cast<PFN_vkVoidFunction>(BindImageMemory2KHR),
        reinterpret_cast<PFN_vkVoidFunction>(checkedBindImageMemory2KHR),
    },
    {
        "vkCreateAndroidSurfaceKHR",
        ProcHook::INSTANCE,
        ProcHook::KHR_android_surface,
        reinterpret_cast<PFN_vkVoidFunction>(CreateAndroidSurfaceKHR),
        nullptr,
    },
    {
        "vkCreateDebugReportCallbackEXT",
        ProcHook::INSTANCE,
        ProcHook::EXT_debug_report,
        reinterpret_cast<PFN_vkVoidFunction>(CreateDebugReportCallbackEXT),
        nullptr,
    },
    {
        "vkCreateDevice",
        ProcHook::INSTANCE,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(CreateDevice),
        nullptr,
    },
    {
        "vkCreateInstance",
        ProcHook::GLOBAL,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(CreateInstance),
        nullptr,
    },
    {
        "vkCreateSwapchainKHR",
        ProcHook::DEVICE,
        ProcHook::KHR_swapchain,
        reinterpret_cast<PFN_vkVoidFunction>(CreateSwapchainKHR),
        reinterpret_cast<PFN_vkVoidFunction>(checkedCreateSwapchainKHR),
    },
    {
        "vkDebugReportMessageEXT",
        ProcHook::INSTANCE,
        ProcHook::EXT_debug_report,
        reinterpret_cast<PFN_vkVoidFunction>(DebugReportMessageEXT),
        nullptr,
    },
    {
        "vkDestroyDebugReportCallbackEXT",
        ProcHook::INSTANCE,
        ProcHook::EXT_debug_report,
        reinterpret_cast<PFN_vkVoidFunction>(DestroyDebugReportCallbackEXT),
        nullptr,
    },
    {
        "vkDestroyDevice",
        ProcHook::DEVICE,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(DestroyDevice),
        nullptr,
    },
    {
        "vkDestroyInstance",
        ProcHook::INSTANCE,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(DestroyInstance),
        nullptr,
    },
    {
        "vkDestroySurfaceKHR",
        ProcHook::INSTANCE,
        ProcHook::KHR_surface,
        reinterpret_cast<PFN_vkVoidFunction>(DestroySurfaceKHR),
        nullptr,
    },
    {
        "vkDestroySwapchainKHR",
        ProcHook::DEVICE,
        ProcHook::KHR_swapchain,
        reinterpret_cast<PFN_vkVoidFunction>(DestroySwapchainKHR),
        reinterpret_cast<PFN_vkVoidFunction>(checkedDestroySwapchainKHR),
    },
    {
        "vkEnumerateDeviceExtensionProperties",
        ProcHook::INSTANCE,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(EnumerateDeviceExtensionProperties),
        nullptr,
    },
    {
        "vkEnumerateInstanceExtensionProperties",
        ProcHook::GLOBAL,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(EnumerateInstanceExtensionProperties),
        nullptr,
    },
    {
        "vkEnumeratePhysicalDeviceGroups",
        ProcHook::INSTANCE,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(EnumeratePhysicalDeviceGroups),
        nullptr,
    },
    {
        "vkEnumeratePhysicalDevices",
        ProcHook::INSTANCE,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(EnumeratePhysicalDevices),
        nullptr,
    },
    {
        "vkGetDeviceGroupPresentCapabilitiesKHR",
        ProcHook::DEVICE,
        ProcHook::KHR_swapchain,
        reinterpret_cast<PFN_vkVoidFunction>(GetDeviceGroupPresentCapabilitiesKHR),
        reinterpret_cast<PFN_vkVoidFunction>(checkedGetDeviceGroupPresentCapabilitiesKHR),
    },
    {
        "vkGetDeviceGroupSurfacePresentModesKHR",
        ProcHook::DEVICE,
        ProcHook::KHR_swapchain,
        reinterpret_cast<PFN_vkVoidFunction>(GetDeviceGroupSurfacePresentModesKHR),
        reinterpret_cast<PFN_vkVoidFunction>(checkedGetDeviceGroupSurfacePresentModesKHR),
    },
    {
        "vkGetDeviceProcAddr",
        ProcHook::DEVICE,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(GetDeviceProcAddr),
        nullptr,
    },
    {
        "vkGetDeviceQueue",
        ProcHook::DEVICE,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(GetDeviceQueue),
        nullptr,
    },
    {
        "vkGetDeviceQueue2",
        ProcHook::DEVICE,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(GetDeviceQueue2),
        nullptr,
    },
    {
        "vkGetInstanceProcAddr",
        ProcHook::INSTANCE,
        ProcHook::EXTENSION_CORE,
        reinterpret_cast<PFN_vkVoidFunction>(GetInstanceProcAddr),
        nullptr,
    },
    {
        "vkGetPastPresentationTimingGOOGLE",
        ProcHook::DEVICE,
        ProcHook::GOOGLE_display_timing,
        reinterpret_cast<PFN_vkVoidFunction>(GetPastPresentationTimingGOOGLE),
        reinterpret_cast<PFN_vkVoidFunction>(checkedGetPastPresentationTimingGOOGLE),
    },
    {
        "vkGetPhysicalDevicePresentRectanglesKHR",
        ProcHook::INSTANCE,
        ProcHook::KHR_swapchain,
        reinterpret_cast<PFN_vkVoidFunction>(GetPhysicalDevicePresentRectanglesKHR),
        nullptr,
    },
    {
        "vkGetPhysicalDeviceSurfaceCapabilities2KHR",
        ProcHook::INSTANCE,
        ProcHook::KHR_get_surface_capabilities2,
        reinterpret_cast<PFN_vkVoidFunction>(GetPhysicalDeviceSurfaceCapabilities2KHR),
        nullptr,
    },
    {
        "vkGetPhysicalDeviceSurfaceCapabilitiesKHR",
        ProcHook::INSTANCE,
        ProcHook::KHR_surface,
        reinterpret_cast<PFN_vkVoidFunction>(GetPhysicalDeviceSurfaceCapabilitiesKHR),
        nullptr,
    },
    {
        "vkGetPhysicalDeviceSurfaceFormats2KHR",
        ProcHook::INSTANCE,
        ProcHook::KHR_get_surface_capabilities2,
        reinterpret_cast<PFN_vkVoidFunction>(GetPhysicalDeviceSurfaceFormats2KHR),
        nullptr,
    },
    {
        "vkGetPhysicalDeviceSurfaceFormatsKHR",
        ProcHook::INSTANCE,
        ProcHook::KHR_surface,
        reinterpret_cast<PFN_vkVoidFunction>(GetPhysicalDeviceSurfaceFormatsKHR),
        nullptr,
    },
    {
        "vkGetPhysicalDeviceSurfacePresentModesKHR",
        ProcHook::INSTANCE,
        ProcHook::KHR_surface,
        reinterpret_cast<PFN_vkVoidFunction>(GetPhysicalDeviceSurfacePresentModesKHR),
        nullptr,
    },
    {
        "vkGetPhysicalDeviceSurfaceSupportKHR",
        ProcHook::INSTANCE,
        ProcHook::KHR_surface,
        reinterpret_cast<PFN_vkVoidFunction>(GetPhysicalDeviceSurfaceSupportKHR),
        nullptr,
    },
    {
        "vkGetRefreshCycleDurationGOOGLE",
        ProcHook::DEVICE,
        ProcHook::GOOGLE_display_timing,
        reinterpret_cast<PFN_vkVoidFunction>(GetRefreshCycleDurationGOOGLE),
        reinterpret_cast<PFN_vkVoidFunction>(checkedGetRefreshCycleDurationGOOGLE),
    },
    {
        "vkGetSwapchainGrallocUsage2ANDROID",
        ProcHook::DEVICE,
        ProcHook::ANDROID_native_buffer,
        nullptr,
        nullptr,
    },
    {
        "vkGetSwapchainGrallocUsageANDROID",
        ProcHook::DEVICE,
        ProcHook::ANDROID_native_buffer,
        nullptr,
        nullptr,
    },
    {
        "vkGetSwapchainImagesKHR",
        ProcHook::DEVICE,
        ProcHook::KHR_swapchain,
        reinterpret_cast<PFN_vkVoidFunction>(GetSwapchainImagesKHR),
        reinterpret_cast<PFN_vkVoidFunction>(checkedGetSwapchainImagesKHR),
    },
    {
        "vkGetSwapchainStatusKHR",
        ProcHook::DEVICE,
        ProcHook::KHR_shared_presentable_image,
        reinterpret_cast<PFN_vkVoidFunction>(GetSwapchainStatusKHR),
        reinterpret_cast<PFN_vkVoidFunction>(checkedGetSwapchainStatusKHR),
    },
    {
        "vkQueuePresentKHR",
        ProcHook::DEVICE,
        ProcHook::KHR_swapchain,
        reinterpret_cast<PFN_vkVoidFunction>(QueuePresentKHR),
        reinterpret_cast<PFN_vkVoidFunction>(checkedQueuePresentKHR),
    },
    {
        "vkQueueSignalReleaseImageANDROID",
        ProcHook::DEVICE,
        ProcHook::ANDROID_native_buffer,
        nullptr,
        nullptr,
    },
    {
        "vkSetHdrMetadataEXT",
        ProcHook::DEVICE,
        ProcHook::EXT_hdr_metadata,
        reinterpret_cast<PFN_vkVoidFunction>(SetHdrMetadataEXT),
        reinterpret_cast<PFN_vkVoidFunction>(checkedSetHdrMetadataEXT),
    },
    // clang-format on
};

}  // namespace

const ProcHook* GetProcHook(const char* name) {
    const auto& begin = g_proc_hooks;
    const auto& end =
        g_proc_hooks + sizeof(g_proc_hooks) / sizeof(g_proc_hooks[0]);
    const auto hook = std::lower_bound(
        begin, end, name,
        [](const ProcHook& e, const char* n) { return strcmp(e.name, n) < 0; });
    return (hook < end && strcmp(hook->name, name) == 0) ? hook : nullptr;
}

ProcHook::Extension GetProcHookExtension(const char* name) {
    // clang-format off
    if (strcmp(name, "VK_ANDROID_native_buffer") == 0) return ProcHook::ANDROID_native_buffer;
    if (strcmp(name, "VK_EXT_debug_report") == 0) return ProcHook::EXT_debug_report;
    if (strcmp(name, "VK_EXT_hdr_metadata") == 0) return ProcHook::EXT_hdr_metadata;
    if (strcmp(name, "VK_EXT_swapchain_colorspace") == 0) return ProcHook::EXT_swapchain_colorspace;
    if (strcmp(name, "VK_GOOGLE_display_timing") == 0) return ProcHook::GOOGLE_display_timing;
    if (strcmp(name, "VK_KHR_android_surface") == 0) return ProcHook::KHR_android_surface;
    if (strcmp(name, "VK_KHR_incremental_present") == 0) return ProcHook::KHR_incremental_present;
    if (strcmp(name, "VK_KHR_shared_presentable_image") == 0) return ProcHook::KHR_shared_presentable_image;
    if (strcmp(name, "VK_KHR_surface") == 0) return ProcHook::KHR_surface;
    if (strcmp(name, "VK_KHR_swapchain") == 0) return ProcHook::KHR_swapchain;
    if (strcmp(name, "VK_KHR_get_surface_capabilities2") == 0) return ProcHook::KHR_get_surface_capabilities2;
    if (strcmp(name, "VK_KHR_get_physical_device_properties2") == 0) return ProcHook::KHR_get_physical_device_properties2;
    if (strcmp(name, "VK_ANDROID_external_memory_android_hardware_buffer") == 0) return ProcHook::ANDROID_external_memory_android_hardware_buffer;
    if (strcmp(name, "VK_KHR_bind_memory2") == 0) return ProcHook::KHR_bind_memory2;
    // clang-format on
    return ProcHook::EXTENSION_UNKNOWN;
}

#define UNLIKELY(expr) __builtin_expect((expr), 0)

#define INIT_PROC(required, obj, proc)                                 \
    do {                                                               \
        data.driver.proc =                                             \
            reinterpret_cast<PFN_vk##proc>(get_proc(obj, "vk" #proc)); \
        if (UNLIKELY(required && !data.driver.proc)) {                 \
            ALOGE("missing " #obj " proc: vk" #proc);                  \
            success = false;                                           \
        }                                                              \
    } while (0)

#define INIT_PROC_EXT(ext, required, obj, proc) \
    do {                                        \
        if (extensions[ProcHook::ext])          \
            INIT_PROC(required, obj, proc);     \
    } while (0)

bool InitDriverTable(VkInstance instance,
                     PFN_vkGetInstanceProcAddr get_proc,
                     const std::bitset<ProcHook::EXTENSION_COUNT>& extensions) {
    auto& data = GetData(instance);
    bool success = true;

    // clang-format off
    INIT_PROC(true, instance, DestroyInstance);
    INIT_PROC(true, instance, EnumeratePhysicalDevices);
    INIT_PROC(true, instance, GetInstanceProcAddr);
    INIT_PROC(true, instance, GetPhysicalDeviceProperties);
    INIT_PROC(true, instance, CreateDevice);
    INIT_PROC(true, instance, EnumerateDeviceExtensionProperties);
    INIT_PROC(false, instance, EnumeratePhysicalDeviceGroups);
    INIT_PROC(false, instance, GetPhysicalDeviceProperties2);
    INIT_PROC_EXT(EXT_debug_report, true, instance, CreateDebugReportCallbackEXT);
    INIT_PROC_EXT(EXT_debug_report, true, instance, DestroyDebugReportCallbackEXT);
    INIT_PROC_EXT(EXT_debug_report, true, instance, DebugReportMessageEXT);
    INIT_PROC_EXT(KHR_get_physical_device_properties2, true, instance, GetPhysicalDeviceProperties2KHR);
    // clang-format on

    return success;
}

bool InitDriverTable(VkDevice dev,
                     PFN_vkGetDeviceProcAddr get_proc,
                     const std::bitset<ProcHook::EXTENSION_COUNT>& extensions) {
    auto& data = GetData(dev);
    bool success = true;

    // clang-format off
    INIT_PROC(true, dev, GetDeviceProcAddr);
    INIT_PROC(true, dev, DestroyDevice);
    INIT_PROC(true, dev, GetDeviceQueue);
    INIT_PROC(true, dev, CreateImage);
    INIT_PROC(true, dev, DestroyImage);
    INIT_PROC(true, dev, AllocateCommandBuffers);
    INIT_PROC(false, dev, BindImageMemory2);
    INIT_PROC(false, dev, GetDeviceQueue2);
    INIT_PROC_EXT(ANDROID_native_buffer, false, dev, GetSwapchainGrallocUsageANDROID);
    INIT_PROC_EXT(ANDROID_native_buffer, false, dev, GetSwapchainGrallocUsage2ANDROID);
    INIT_PROC_EXT(ANDROID_native_buffer, true, dev, AcquireImageANDROID);
    INIT_PROC_EXT(ANDROID_native_buffer, true, dev, QueueSignalReleaseImageANDROID);
    INIT_PROC_EXT(KHR_bind_memory2, true, dev, BindImageMemory2KHR);
    // clang-format on

    return success;
}

}  // namespace driver
}  // namespace vulkan

// clang-format on
