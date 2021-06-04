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

#include "driver.h"

namespace vulkan {
namespace driver {

DebugReportCallbackList::Node* DebugReportCallbackList::AddCallback(
    const VkDebugReportCallbackCreateInfoEXT& info,
    VkDebugReportCallbackEXT driver_handle,
    const VkAllocationCallbacks& allocator) {
    void* mem = allocator.pfnAllocation(allocator.pUserData, sizeof(Node),
                                        alignof(Node),
                                        VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    if (!mem)
        return nullptr;

    // initialize and prepend node to the list
    std::lock_guard<decltype(rwmutex_)> lock(rwmutex_);
    head_.next = new (mem) Node{head_.next, info.flags, info.pfnCallback,
                                info.pUserData, driver_handle};

    return head_.next;
}

void DebugReportCallbackList::RemoveCallback(
    Node* node,
    const VkAllocationCallbacks& allocator) {
    // remove node from the list
    {
        std::lock_guard<decltype(rwmutex_)> lock(rwmutex_);
        Node* prev = &head_;
        while (prev && prev->next != node)
            prev = prev->next;
        if (prev)
            prev->next = node->next;
    }

    allocator.pfnFree(allocator.pUserData, node);
}

void DebugReportCallbackList::Message(VkDebugReportFlagsEXT flags,
                                      VkDebugReportObjectTypeEXT object_type,
                                      uint64_t object,
                                      size_t location,
                                      int32_t message_code,
                                      const char* layer_prefix,
                                      const char* message) const {
    std::shared_lock<decltype(rwmutex_)> lock(rwmutex_);
    const Node* node = &head_;
    while ((node = node->next)) {
        if ((node->flags & flags) != 0) {
            node->callback(flags, object_type, object, location, message_code,
                           layer_prefix, message, node->user_data);
        }
    }
}

void DebugReportLogger::Message(VkDebugReportFlagsEXT flags,
                                VkDebugReportObjectTypeEXT object_type,
                                uint64_t object,
                                size_t location,
                                int32_t message_code,
                                const char* layer_prefix,
                                const char* message) const {
    const VkDebugReportCallbackCreateInfoEXT* info =
        reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT*>(
            instance_pnext_);
    while (info) {
        if (info->sType ==
                VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT &&
            (info->flags & flags) != 0) {
            info->pfnCallback(flags, object_type, object, location,
                              message_code, layer_prefix, message,
                              info->pUserData);
        }

        info = reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT*>(
            info->pNext);
    }

    if (callbacks_) {
        callbacks_->Message(flags, object_type, object, location, message_code,
                            layer_prefix, message);
    }
}

void DebugReportLogger::PrintV(VkDebugReportFlagsEXT flags,
                               VkDebugReportObjectTypeEXT object_type,
                               uint64_t object,
                               const char* format,
                               va_list ap) const {
    char buf[1024];
    int len = vsnprintf(buf, sizeof(buf), format, ap);

    // message truncated
    if (len >= static_cast<int>(sizeof(buf)))
        memcpy(buf + sizeof(buf) - 4, "...", 4);

    Message(flags, object_type, object, 0, 0, LOG_TAG, buf);
}

VkResult CreateDebugReportCallbackEXT(
    VkInstance instance,
    const VkDebugReportCallbackCreateInfoEXT* create_info,
    const VkAllocationCallbacks* allocator,
    VkDebugReportCallbackEXT* callback) {
    const auto& driver = GetData(instance).driver;
    VkDebugReportCallbackEXT driver_handle = VK_NULL_HANDLE;
    if (driver.CreateDebugReportCallbackEXT) {
        VkResult result = driver.CreateDebugReportCallbackEXT(
            instance, create_info, allocator, &driver_handle);
        if (result != VK_SUCCESS)
            return result;
    }

    auto& callbacks = GetData(instance).debug_report_callbacks;
    auto node = callbacks.AddCallback(
        *create_info, driver_handle,
        (allocator) ? *allocator : GetData(instance).allocator);
    if (!node) {
        if (driver_handle != VK_NULL_HANDLE) {
            driver.DestroyDebugReportCallbackEXT(instance, driver_handle,
                                                 allocator);
        }

        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    *callback = callbacks.GetHandle(node);

    return VK_SUCCESS;
}

void DestroyDebugReportCallbackEXT(VkInstance instance,
                                   VkDebugReportCallbackEXT callback,
                                   const VkAllocationCallbacks* allocator) {
    if (callback == VK_NULL_HANDLE)
        return;

    auto& callbacks = GetData(instance).debug_report_callbacks;
    auto node = callbacks.FromHandle(callback);
    auto driver_handle = callbacks.GetDriverHandle(node);

    callbacks.RemoveCallback(
        node, (allocator) ? *allocator : GetData(instance).allocator);

    if (driver_handle != VK_NULL_HANDLE) {
        GetData(instance).driver.DestroyDebugReportCallbackEXT(
            instance, driver_handle, allocator);
    }
}

void DebugReportMessageEXT(VkInstance instance,
                           VkDebugReportFlagsEXT flags,
                           VkDebugReportObjectTypeEXT object_type,
                           uint64_t object,
                           size_t location,
                           int32_t message_code,
                           const char* layer_prefix,
                           const char* message) {
    if (GetData(instance).driver.DebugReportMessageEXT) {
        GetData(instance).driver.DebugReportMessageEXT(
            instance, flags, object_type, object, location, message_code,
            layer_prefix, message);
    } else {
        GetData(instance).debug_report_callbacks.Message(
            flags, object_type, object, location, message_code, layer_prefix,
            message);
    }
}

}  // namespace driver
}  // namespace vulkan
