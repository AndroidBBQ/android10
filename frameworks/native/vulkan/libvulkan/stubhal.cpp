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

/* NOTE:
 * This stub HAL is only used internally by the loader when a real HAL
 * implementation is not present, in order to avoid needing "null HAL" checks
 * throughout the loader. It does not enumerate any physical devices, and is
 * only as conformant to the Vulkan and Android HAL interfaces as the loader
 * needs it to be. Do not use it as an example of a correct implementation; the
 * code in ../null_driver is better for that.
 */

#undef LOG_TAG
#define LOG_TAG "vkstub"

#include <array>
#include <bitset>
#include <mutex>

#include <log/log.h>
#include <hardware/hwvulkan.h>

#include "stubhal.h"

namespace vulkan {
namespace stubhal {

namespace {

const size_t kMaxInstances = 32;
static std::mutex g_instance_mutex;
static std::bitset<kMaxInstances> g_instance_used(false);
static std::array<hwvulkan_dispatch_t, kMaxInstances> g_instances;

[[noreturn]] VKAPI_ATTR void NoOp() {
    LOG_ALWAYS_FATAL("invalid stub function called");
}

VKAPI_ATTR VkResult
EnumerateInstanceExtensionProperties(const char* /*layer_name*/,
                                     uint32_t* count,
                                     VkExtensionProperties* /*properties*/) {
    *count = 0;
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult
EnumerateInstanceLayerProperties(uint32_t* count,
                                 VkLayerProperties* /*properties*/) {
    *count = 0;
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult CreateInstance(const VkInstanceCreateInfo* /*create_info*/,
                                   const VkAllocationCallbacks* /*allocator*/,
                                   VkInstance* instance) {
    std::lock_guard<std::mutex> lock(g_instance_mutex);
    for (size_t i = 0; i < kMaxInstances; i++) {
        if (!g_instance_used[i]) {
            g_instance_used[i] = true;
            g_instances[i].magic = HWVULKAN_DISPATCH_MAGIC;
            *instance = reinterpret_cast<VkInstance>(&g_instances[i]);
            return VK_SUCCESS;
        }
    }
    ALOGE("no more instances available (max=%zu)", kMaxInstances);
    return VK_ERROR_INITIALIZATION_FAILED;
}

VKAPI_ATTR void DestroyInstance(VkInstance instance,
                                const VkAllocationCallbacks* /*allocator*/) {
    std::lock_guard<std::mutex> lock(g_instance_mutex);
    ssize_t idx =
        reinterpret_cast<hwvulkan_dispatch_t*>(instance) - &g_instances[0];
    ALOG_ASSERT(idx >= 0 && static_cast<size_t>(idx) < g_instance_used.size(),
                "DestroyInstance: invalid instance handle");
    g_instance_used[static_cast<size_t>(idx)] = false;
}

VKAPI_ATTR VkResult EnumeratePhysicalDevices(VkInstance /*instance*/,
                                             uint32_t* count,
                                             VkPhysicalDevice* /*gpus*/) {
    *count = 0;
    return VK_SUCCESS;
}

VKAPI_ATTR VkResult
EnumeratePhysicalDeviceGroups(VkInstance /*instance*/,
                              uint32_t* count,
                              VkPhysicalDeviceGroupProperties* /*properties*/) {
    *count = 0;
    return VK_SUCCESS;
}

VKAPI_ATTR PFN_vkVoidFunction GetInstanceProcAddr(VkInstance instance,
                                                  const char* name) {
    if (strcmp(name, "vkCreateInstance") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(CreateInstance);
    if (strcmp(name, "vkDestroyInstance") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(DestroyInstance);
    if (strcmp(name, "vkEnumerateInstanceExtensionProperties") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(
            EnumerateInstanceExtensionProperties);
    if (strcmp(name, "vkEnumeratePhysicalDevices") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(EnumeratePhysicalDevices);
    if (strcmp(name, "vkEnumeratePhysicalDeviceGroups") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(
            EnumeratePhysicalDeviceGroups);
    if (strcmp(name, "vkGetInstanceProcAddr") == 0)
        return reinterpret_cast<PFN_vkVoidFunction>(GetInstanceProcAddr);
    // Per the spec, return NULL if instance is NULL.
    if (!instance)
        return nullptr;
    // None of the other Vulkan functions should ever be called, as they all
    // take a VkPhysicalDevice or other object obtained from a physical device.
    return reinterpret_cast<PFN_vkVoidFunction>(NoOp);
}

}  // anonymous namespace

const hwvulkan_device_t kDevice = {
    .common =
        {
            .tag = HARDWARE_DEVICE_TAG,
            .version = HWVULKAN_DEVICE_API_VERSION_0_1,
            .module = nullptr,
            .close = nullptr,
        },
    .EnumerateInstanceExtensionProperties =
        EnumerateInstanceExtensionProperties,
    .CreateInstance = CreateInstance,
    .GetInstanceProcAddr = GetInstanceProcAddr,
};

}  // namespace stubhal
}  // namespace vulkan
