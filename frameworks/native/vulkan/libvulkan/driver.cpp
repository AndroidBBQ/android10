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

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>

#include <dlfcn.h>
#include <algorithm>
#include <array>
#include <new>

#include <log/log.h>

#include <android/dlext.h>
#include <android/hardware/configstore/1.0/ISurfaceFlingerConfigs.h>
#include <configstore/Utils.h>
#include <cutils/properties.h>
#include <graphicsenv/GraphicsEnv.h>
#include <utils/Timers.h>
#include <utils/Trace.h>
#include <utils/Vector.h>

#include "android-base/properties.h"

#include "driver.h"
#include "stubhal.h"

using namespace android::hardware::configstore;
using namespace android::hardware::configstore::V1_0;

// TODO(b/37049319) Get this from a header once one exists
extern "C" {
android_namespace_t* android_get_exported_namespace(const char*);
}

// #define ENABLE_ALLOC_CALLSTACKS 1
#if ENABLE_ALLOC_CALLSTACKS
#include <utils/CallStack.h>
#define ALOGD_CALLSTACK(...)                             \
    do {                                                 \
        ALOGD(__VA_ARGS__);                              \
        android::CallStack callstack;                    \
        callstack.update();                              \
        callstack.log(LOG_TAG, ANDROID_LOG_DEBUG, "  "); \
    } while (false)
#else
#define ALOGD_CALLSTACK(...) \
    do {                     \
    } while (false)
#endif

namespace vulkan {
namespace driver {

namespace {

class Hal {
   public:
    static bool Open();

    static const Hal& Get() { return hal_; }
    static const hwvulkan_device_t& Device() { return *Get().dev_; }

    int GetDebugReportIndex() const { return debug_report_index_; }

   private:
    Hal() : dev_(nullptr), debug_report_index_(-1) {}
    Hal(const Hal&) = delete;
    Hal& operator=(const Hal&) = delete;

    bool InitDebugReportIndex();

    static Hal hal_;

    const hwvulkan_device_t* dev_;
    int debug_report_index_;
};

class CreateInfoWrapper {
   public:
    CreateInfoWrapper(const VkInstanceCreateInfo& create_info,
                      const VkAllocationCallbacks& allocator);
    CreateInfoWrapper(VkPhysicalDevice physical_dev,
                      const VkDeviceCreateInfo& create_info,
                      const VkAllocationCallbacks& allocator);
    ~CreateInfoWrapper();

    VkResult Validate();
    void DowngradeApiVersion();

    const std::bitset<ProcHook::EXTENSION_COUNT>& GetHookExtensions() const;
    const std::bitset<ProcHook::EXTENSION_COUNT>& GetHalExtensions() const;

    explicit operator const VkInstanceCreateInfo*() const;
    explicit operator const VkDeviceCreateInfo*() const;

   private:
    struct ExtensionFilter {
        VkExtensionProperties* exts;
        uint32_t ext_count;

        const char** names;
        uint32_t name_count;
    };

    VkResult SanitizePNext();

    VkResult SanitizeLayers();
    VkResult SanitizeExtensions();

    VkResult QueryExtensionCount(uint32_t& count) const;
    VkResult EnumerateExtensions(uint32_t& count,
                                 VkExtensionProperties* props) const;
    VkResult InitExtensionFilter();
    void FilterExtension(const char* name);

    const bool is_instance_;
    const VkAllocationCallbacks& allocator_;

    VkPhysicalDevice physical_dev_;

    union {
        VkInstanceCreateInfo instance_info_;
        VkDeviceCreateInfo dev_info_;
    };

    VkApplicationInfo application_info_;

    ExtensionFilter extension_filter_;

    std::bitset<ProcHook::EXTENSION_COUNT> hook_extensions_;
    std::bitset<ProcHook::EXTENSION_COUNT> hal_extensions_;
};

Hal Hal::hal_;

void* LoadLibrary(const android_dlextinfo& dlextinfo,
                  const char* subname,
                  int subname_len) {
    ATRACE_CALL();

    const char kLibFormat[] = "vulkan.%*s.so";
    char* name = static_cast<char*>(
        alloca(sizeof(kLibFormat) + static_cast<size_t>(subname_len)));
    sprintf(name, kLibFormat, subname_len, subname);
    return android_dlopen_ext(name, RTLD_LOCAL | RTLD_NOW, &dlextinfo);
}

const std::array<const char*, 2> HAL_SUBNAME_KEY_PROPERTIES = {{
    "ro.hardware." HWVULKAN_HARDWARE_MODULE_ID,
    "ro.board.platform",
}};

int LoadDriver(android_namespace_t* library_namespace,
               const hwvulkan_module_t** module) {
    ATRACE_CALL();

    const android_dlextinfo dlextinfo = {
        .flags = ANDROID_DLEXT_USE_NAMESPACE,
        .library_namespace = library_namespace,
    };
    void* so = nullptr;
    char prop[PROPERTY_VALUE_MAX];
    for (auto key : HAL_SUBNAME_KEY_PROPERTIES) {
        int prop_len = property_get(key, prop, nullptr);
        if (prop_len > 0) {
            so = LoadLibrary(dlextinfo, prop, prop_len);
            if (so)
                break;
        }
    }
    if (!so)
        return -ENOENT;

    auto hmi = static_cast<hw_module_t*>(dlsym(so, HAL_MODULE_INFO_SYM_AS_STR));
    if (!hmi) {
        ALOGE("couldn't find symbol '%s' in HAL library: %s", HAL_MODULE_INFO_SYM_AS_STR, dlerror());
        dlclose(so);
        return -EINVAL;
    }
    if (strcmp(hmi->id, HWVULKAN_HARDWARE_MODULE_ID) != 0) {
        ALOGE("HAL id '%s' != '%s'", hmi->id, HWVULKAN_HARDWARE_MODULE_ID);
        dlclose(so);
        return -EINVAL;
    }
    hmi->dso = so;
    *module = reinterpret_cast<const hwvulkan_module_t*>(hmi);
    return 0;
}

int LoadBuiltinDriver(const hwvulkan_module_t** module) {
    ATRACE_CALL();

    auto ns = android_get_exported_namespace("sphal");
    if (!ns)
        return -ENOENT;
    android::GraphicsEnv::getInstance().setDriverToLoad(
        android::GraphicsEnv::Driver::VULKAN);
    return LoadDriver(ns, module);
}

int LoadUpdatedDriver(const hwvulkan_module_t** module) {
    ATRACE_CALL();

    auto ns = android::GraphicsEnv::getInstance().getDriverNamespace();
    if (!ns)
        return -ENOENT;
    android::GraphicsEnv::getInstance().setDriverToLoad(
        android::GraphicsEnv::Driver::VULKAN_UPDATED);
    return LoadDriver(ns, module);
}

bool Hal::Open() {
    ATRACE_CALL();

    const nsecs_t openTime = systemTime();

    ALOG_ASSERT(!hal_.dev_, "OpenHAL called more than once");

    // Use a stub device unless we successfully open a real HAL device.
    hal_.dev_ = &stubhal::kDevice;

    int result;
    const hwvulkan_module_t* module = nullptr;

    result = LoadUpdatedDriver(&module);
    if (result == -ENOENT) {
        result = LoadBuiltinDriver(&module);
        if (result != 0) {
            // -ENOENT means the sphal namespace doesn't exist, not that there
            // is a problem with the driver.
            ALOGW_IF(
                result != -ENOENT,
                "Failed to load Vulkan driver into sphal namespace. This "
                "usually means the driver has forbidden library dependencies."
                "Please fix, this will soon stop working.");
            result =
                hw_get_module(HWVULKAN_HARDWARE_MODULE_ID,
                              reinterpret_cast<const hw_module_t**>(&module));
        }
    }
    if (result != 0) {
        android::GraphicsEnv::getInstance().setDriverLoaded(
            android::GraphicsEnv::Api::API_VK, false, systemTime() - openTime);
        ALOGV("unable to load Vulkan HAL, using stub HAL (result=%d)", result);
        return true;
    }


    hwvulkan_device_t* device;
    ATRACE_BEGIN("hwvulkan module open");
    result =
        module->common.methods->open(&module->common, HWVULKAN_DEVICE_0,
                                     reinterpret_cast<hw_device_t**>(&device));
    ATRACE_END();
    if (result != 0) {
        android::GraphicsEnv::getInstance().setDriverLoaded(
            android::GraphicsEnv::Api::API_VK, false, systemTime() - openTime);
        // Any device with a Vulkan HAL should be able to open the device.
        ALOGE("failed to open Vulkan HAL device: %s (%d)", strerror(-result),
              result);
        return false;
    }

    hal_.dev_ = device;

    hal_.InitDebugReportIndex();

    android::GraphicsEnv::getInstance().setDriverLoaded(
        android::GraphicsEnv::Api::API_VK, true, systemTime() - openTime);

    return true;
}

bool Hal::InitDebugReportIndex() {
    ATRACE_CALL();

    uint32_t count;
    if (dev_->EnumerateInstanceExtensionProperties(nullptr, &count, nullptr) !=
        VK_SUCCESS) {
        ALOGE("failed to get HAL instance extension count");
        return false;
    }

    VkExtensionProperties* exts = reinterpret_cast<VkExtensionProperties*>(
        malloc(sizeof(VkExtensionProperties) * count));
    if (!exts) {
        ALOGE("failed to allocate HAL instance extension array");
        return false;
    }

    if (dev_->EnumerateInstanceExtensionProperties(nullptr, &count, exts) !=
        VK_SUCCESS) {
        ALOGE("failed to enumerate HAL instance extensions");
        free(exts);
        return false;
    }

    for (uint32_t i = 0; i < count; i++) {
        if (strcmp(exts[i].extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME) ==
            0) {
            debug_report_index_ = static_cast<int>(i);
            break;
        }
    }

    free(exts);

    return true;
}

CreateInfoWrapper::CreateInfoWrapper(const VkInstanceCreateInfo& create_info,
                                     const VkAllocationCallbacks& allocator)
    : is_instance_(true),
      allocator_(allocator),
      physical_dev_(VK_NULL_HANDLE),
      instance_info_(create_info),
      extension_filter_() {
    hook_extensions_.set(ProcHook::EXTENSION_CORE);
    hal_extensions_.set(ProcHook::EXTENSION_CORE);
}

CreateInfoWrapper::CreateInfoWrapper(VkPhysicalDevice physical_dev,
                                     const VkDeviceCreateInfo& create_info,
                                     const VkAllocationCallbacks& allocator)
    : is_instance_(false),
      allocator_(allocator),
      physical_dev_(physical_dev),
      dev_info_(create_info),
      extension_filter_() {
    hook_extensions_.set(ProcHook::EXTENSION_CORE);
    hal_extensions_.set(ProcHook::EXTENSION_CORE);
}

CreateInfoWrapper::~CreateInfoWrapper() {
    allocator_.pfnFree(allocator_.pUserData, extension_filter_.exts);
    allocator_.pfnFree(allocator_.pUserData, extension_filter_.names);
}

VkResult CreateInfoWrapper::Validate() {
    VkResult result = SanitizePNext();
    if (result == VK_SUCCESS)
        result = SanitizeLayers();
    if (result == VK_SUCCESS)
        result = SanitizeExtensions();

    return result;
}

const std::bitset<ProcHook::EXTENSION_COUNT>&
CreateInfoWrapper::GetHookExtensions() const {
    return hook_extensions_;
}

const std::bitset<ProcHook::EXTENSION_COUNT>&
CreateInfoWrapper::GetHalExtensions() const {
    return hal_extensions_;
}

CreateInfoWrapper::operator const VkInstanceCreateInfo*() const {
    return &instance_info_;
}

CreateInfoWrapper::operator const VkDeviceCreateInfo*() const {
    return &dev_info_;
}

VkResult CreateInfoWrapper::SanitizePNext() {
    const struct StructHeader {
        VkStructureType type;
        const void* next;
    } * header;

    if (is_instance_) {
        header = reinterpret_cast<const StructHeader*>(instance_info_.pNext);

        // skip leading VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFOs
        while (header &&
               header->type == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO)
            header = reinterpret_cast<const StructHeader*>(header->next);

        instance_info_.pNext = header;
    } else {
        header = reinterpret_cast<const StructHeader*>(dev_info_.pNext);

        // skip leading VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFOs
        while (header &&
               header->type == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO)
            header = reinterpret_cast<const StructHeader*>(header->next);

        dev_info_.pNext = header;
    }

    return VK_SUCCESS;
}

VkResult CreateInfoWrapper::SanitizeLayers() {
    auto& layer_names = (is_instance_) ? instance_info_.ppEnabledLayerNames
                                       : dev_info_.ppEnabledLayerNames;
    auto& layer_count = (is_instance_) ? instance_info_.enabledLayerCount
                                       : dev_info_.enabledLayerCount;

    // remove all layers
    layer_names = nullptr;
    layer_count = 0;

    return VK_SUCCESS;
}

VkResult CreateInfoWrapper::SanitizeExtensions() {
    auto& ext_names = (is_instance_) ? instance_info_.ppEnabledExtensionNames
                                     : dev_info_.ppEnabledExtensionNames;
    auto& ext_count = (is_instance_) ? instance_info_.enabledExtensionCount
                                     : dev_info_.enabledExtensionCount;
    if (!ext_count)
        return VK_SUCCESS;

    VkResult result = InitExtensionFilter();
    if (result != VK_SUCCESS)
        return result;

    for (uint32_t i = 0; i < ext_count; i++)
        FilterExtension(ext_names[i]);

    // Enable device extensions that contain physical-device commands, so that
    // vkGetInstanceProcAddr will return those physical-device commands.
    if (is_instance_) {
        hook_extensions_.set(ProcHook::KHR_swapchain);
    }

    ext_names = extension_filter_.names;
    ext_count = extension_filter_.name_count;

    return VK_SUCCESS;
}

VkResult CreateInfoWrapper::QueryExtensionCount(uint32_t& count) const {
    if (is_instance_) {
        return Hal::Device().EnumerateInstanceExtensionProperties(
            nullptr, &count, nullptr);
    } else {
        const auto& driver = GetData(physical_dev_).driver;
        return driver.EnumerateDeviceExtensionProperties(physical_dev_, nullptr,
                                                         &count, nullptr);
    }
}

VkResult CreateInfoWrapper::EnumerateExtensions(
    uint32_t& count,
    VkExtensionProperties* props) const {
    if (is_instance_) {
        return Hal::Device().EnumerateInstanceExtensionProperties(
            nullptr, &count, props);
    } else {
        const auto& driver = GetData(physical_dev_).driver;
        return driver.EnumerateDeviceExtensionProperties(physical_dev_, nullptr,
                                                         &count, props);
    }
}

VkResult CreateInfoWrapper::InitExtensionFilter() {
    // query extension count
    uint32_t count;
    VkResult result = QueryExtensionCount(count);
    if (result != VK_SUCCESS || count == 0)
        return result;

    auto& filter = extension_filter_;
    filter.exts =
        reinterpret_cast<VkExtensionProperties*>(allocator_.pfnAllocation(
            allocator_.pUserData, sizeof(VkExtensionProperties) * count,
            alignof(VkExtensionProperties),
            VK_SYSTEM_ALLOCATION_SCOPE_COMMAND));
    if (!filter.exts)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    // enumerate extensions
    result = EnumerateExtensions(count, filter.exts);
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
        return result;

    if (!count)
        return VK_SUCCESS;

    filter.ext_count = count;

    // allocate name array
    uint32_t enabled_ext_count = (is_instance_)
                                     ? instance_info_.enabledExtensionCount
                                     : dev_info_.enabledExtensionCount;
    count = std::min(filter.ext_count, enabled_ext_count);
    filter.names = reinterpret_cast<const char**>(allocator_.pfnAllocation(
        allocator_.pUserData, sizeof(const char*) * count, alignof(const char*),
        VK_SYSTEM_ALLOCATION_SCOPE_COMMAND));
    if (!filter.names)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    return VK_SUCCESS;
}

void CreateInfoWrapper::FilterExtension(const char* name) {
    auto& filter = extension_filter_;

    ProcHook::Extension ext_bit = GetProcHookExtension(name);
    if (is_instance_) {
        switch (ext_bit) {
            case ProcHook::KHR_android_surface:
            case ProcHook::KHR_surface:
            case ProcHook::EXT_swapchain_colorspace:
            case ProcHook::KHR_get_surface_capabilities2:
                hook_extensions_.set(ext_bit);
                // return now as these extensions do not require HAL support
                return;
            case ProcHook::EXT_debug_report:
                // both we and HAL can take part in
                hook_extensions_.set(ext_bit);
                break;
            case ProcHook::KHR_get_physical_device_properties2:
            case ProcHook::EXTENSION_UNKNOWN:
                // Extensions we don't need to do anything about at this level
                break;

            case ProcHook::KHR_bind_memory2:
            case ProcHook::KHR_incremental_present:
            case ProcHook::KHR_shared_presentable_image:
            case ProcHook::KHR_swapchain:
            case ProcHook::EXT_hdr_metadata:
            case ProcHook::ANDROID_external_memory_android_hardware_buffer:
            case ProcHook::ANDROID_native_buffer:
            case ProcHook::GOOGLE_display_timing:
            case ProcHook::EXTENSION_CORE:
            case ProcHook::EXTENSION_COUNT:
                // Device and meta extensions. If we ever get here it's a bug in
                // our code. But enumerating them lets us avoid having a default
                // case, and default hides other bugs.
                ALOGE(
                    "CreateInfoWrapper::FilterExtension: invalid instance "
                    "extension '%s'. FIX ME",
                    name);
                return;

            // Don't use a default case. Without it, -Wswitch will tell us
            // at compile time if someone adds a new ProcHook extension but
            // doesn't handle it above. That's a real bug that has
            // not-immediately-obvious effects.
            //
            // default:
            //     break;
        }
    } else {
        switch (ext_bit) {
            case ProcHook::KHR_swapchain:
                // map VK_KHR_swapchain to VK_ANDROID_native_buffer
                name = VK_ANDROID_NATIVE_BUFFER_EXTENSION_NAME;
                ext_bit = ProcHook::ANDROID_native_buffer;
                break;
            case ProcHook::KHR_incremental_present:
            case ProcHook::GOOGLE_display_timing:
            case ProcHook::KHR_shared_presentable_image:
                hook_extensions_.set(ext_bit);
                // return now as these extensions do not require HAL support
                return;
            case ProcHook::EXT_hdr_metadata:
            case ProcHook::KHR_bind_memory2:
                hook_extensions_.set(ext_bit);
                break;
            case ProcHook::ANDROID_external_memory_android_hardware_buffer:
            case ProcHook::EXTENSION_UNKNOWN:
                // Extensions we don't need to do anything about at this level
                break;

            case ProcHook::KHR_android_surface:
            case ProcHook::KHR_get_physical_device_properties2:
            case ProcHook::KHR_get_surface_capabilities2:
            case ProcHook::KHR_surface:
            case ProcHook::EXT_debug_report:
            case ProcHook::EXT_swapchain_colorspace:
            case ProcHook::ANDROID_native_buffer:
            case ProcHook::EXTENSION_CORE:
            case ProcHook::EXTENSION_COUNT:
                // Instance and meta extensions. If we ever get here it's a bug
                // in our code. But enumerating them lets us avoid having a
                // default case, and default hides other bugs.
                ALOGE(
                    "CreateInfoWrapper::FilterExtension: invalid device "
                    "extension '%s'. FIX ME",
                    name);
                return;

            // Don't use a default case. Without it, -Wswitch will tell us
            // at compile time if someone adds a new ProcHook extension but
            // doesn't handle it above. That's a real bug that has
            // not-immediately-obvious effects.
            //
            // default:
            //     break;
        }
    }

    for (uint32_t i = 0; i < filter.ext_count; i++) {
        const VkExtensionProperties& props = filter.exts[i];
        // ignore unknown extensions
        if (strcmp(name, props.extensionName) != 0)
            continue;

        filter.names[filter.name_count++] = name;
        if (ext_bit != ProcHook::EXTENSION_UNKNOWN) {
            if (ext_bit == ProcHook::ANDROID_native_buffer)
                hook_extensions_.set(ProcHook::KHR_swapchain);

            hal_extensions_.set(ext_bit);
        }

        break;
    }
}

void CreateInfoWrapper::DowngradeApiVersion() {
    // If pApplicationInfo is NULL, apiVersion is assumed to be 1.0:
    if (instance_info_.pApplicationInfo) {
        application_info_ = *instance_info_.pApplicationInfo;
        instance_info_.pApplicationInfo = &application_info_;
        application_info_.apiVersion = VK_API_VERSION_1_0;
    }
}

VKAPI_ATTR void* DefaultAllocate(void*,
                                 size_t size,
                                 size_t alignment,
                                 VkSystemAllocationScope) {
    void* ptr = nullptr;
    // Vulkan requires 'alignment' to be a power of two, but posix_memalign
    // additionally requires that it be at least sizeof(void*).
    int ret = posix_memalign(&ptr, std::max(alignment, sizeof(void*)), size);
    ALOGD_CALLSTACK("Allocate: size=%zu align=%zu => (%d) %p", size, alignment,
                    ret, ptr);
    return ret == 0 ? ptr : nullptr;
}

VKAPI_ATTR void* DefaultReallocate(void*,
                                   void* ptr,
                                   size_t size,
                                   size_t alignment,
                                   VkSystemAllocationScope) {
    if (size == 0) {
        free(ptr);
        return nullptr;
    }

    // TODO(jessehall): Right now we never shrink allocations; if the new
    // request is smaller than the existing chunk, we just continue using it.
    // Right now the loader never reallocs, so this doesn't matter. If that
    // changes, or if this code is copied into some other project, this should
    // probably have a heuristic to allocate-copy-free when doing so will save
    // "enough" space.
    size_t old_size = ptr ? malloc_usable_size(ptr) : 0;
    if (size <= old_size)
        return ptr;

    void* new_ptr = nullptr;
    if (posix_memalign(&new_ptr, std::max(alignment, sizeof(void*)), size) != 0)
        return nullptr;
    if (ptr) {
        memcpy(new_ptr, ptr, std::min(old_size, size));
        free(ptr);
    }
    return new_ptr;
}

VKAPI_ATTR void DefaultFree(void*, void* ptr) {
    ALOGD_CALLSTACK("Free: %p", ptr);
    free(ptr);
}

InstanceData* AllocateInstanceData(const VkAllocationCallbacks& allocator) {
    void* data_mem = allocator.pfnAllocation(
        allocator.pUserData, sizeof(InstanceData), alignof(InstanceData),
        VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
    if (!data_mem)
        return nullptr;

    return new (data_mem) InstanceData(allocator);
}

void FreeInstanceData(InstanceData* data,
                      const VkAllocationCallbacks& allocator) {
    data->~InstanceData();
    allocator.pfnFree(allocator.pUserData, data);
}

DeviceData* AllocateDeviceData(
    const VkAllocationCallbacks& allocator,
    const DebugReportCallbackList& debug_report_callbacks) {
    void* data_mem = allocator.pfnAllocation(
        allocator.pUserData, sizeof(DeviceData), alignof(DeviceData),
        VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
    if (!data_mem)
        return nullptr;

    return new (data_mem) DeviceData(allocator, debug_report_callbacks);
}

void FreeDeviceData(DeviceData* data, const VkAllocationCallbacks& allocator) {
    data->~DeviceData();
    allocator.pfnFree(allocator.pUserData, data);
}

}  // anonymous namespace

bool OpenHAL() {
    return Hal::Open();
}

const VkAllocationCallbacks& GetDefaultAllocator() {
    static const VkAllocationCallbacks kDefaultAllocCallbacks = {
        .pUserData = nullptr,
        .pfnAllocation = DefaultAllocate,
        .pfnReallocation = DefaultReallocate,
        .pfnFree = DefaultFree,
    };

    return kDefaultAllocCallbacks;
}

PFN_vkVoidFunction GetInstanceProcAddr(VkInstance instance, const char* pName) {
    const ProcHook* hook = GetProcHook(pName);
    if (!hook)
        return Hal::Device().GetInstanceProcAddr(instance, pName);

    if (!instance) {
        if (hook->type == ProcHook::GLOBAL)
            return hook->proc;

        // v0 layers expect
        //
        //   vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateDevice");
        //
        // to work.
        if (strcmp(pName, "vkCreateDevice") == 0)
            return hook->proc;

        ALOGE(
            "internal vkGetInstanceProcAddr called for %s without an instance",
            pName);

        return nullptr;
    }

    PFN_vkVoidFunction proc;

    switch (hook->type) {
        case ProcHook::INSTANCE:
            proc = (GetData(instance).hook_extensions[hook->extension])
                       ? hook->proc
                       : nullptr;
            break;
        case ProcHook::DEVICE:
            proc = (hook->extension == ProcHook::EXTENSION_CORE)
                       ? hook->proc
                       : hook->checked_proc;
            break;
        default:
            ALOGE(
                "internal vkGetInstanceProcAddr called for %s with an instance",
                pName);
            proc = nullptr;
            break;
    }

    return proc;
}

PFN_vkVoidFunction GetDeviceProcAddr(VkDevice device, const char* pName) {
    const ProcHook* hook = GetProcHook(pName);
    if (!hook)
        return GetData(device).driver.GetDeviceProcAddr(device, pName);

    if (hook->type != ProcHook::DEVICE) {
        ALOGE("internal vkGetDeviceProcAddr called for %s", pName);
        return nullptr;
    }

    return (GetData(device).hook_extensions[hook->extension]) ? hook->proc
                                                              : nullptr;
}

VkResult EnumerateInstanceExtensionProperties(
    const char* pLayerName,
    uint32_t* pPropertyCount,
    VkExtensionProperties* pProperties) {

    android::Vector<VkExtensionProperties> loader_extensions;
    loader_extensions.push_back({
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_SURFACE_SPEC_VERSION});
    loader_extensions.push_back({
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
        VK_KHR_ANDROID_SURFACE_SPEC_VERSION});
    loader_extensions.push_back({
        VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
        VK_EXT_SWAPCHAIN_COLOR_SPACE_SPEC_VERSION});
    loader_extensions.push_back({
        VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
        VK_KHR_GET_SURFACE_CAPABILITIES_2_SPEC_VERSION});

    static const VkExtensionProperties loader_debug_report_extension = {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_SPEC_VERSION,
    };

    // enumerate our extensions first
    if (!pLayerName && pProperties) {
        uint32_t count = std::min(
            *pPropertyCount, static_cast<uint32_t>(loader_extensions.size()));

        std::copy_n(loader_extensions.begin(), count, pProperties);

        if (count < loader_extensions.size()) {
            *pPropertyCount = count;
            return VK_INCOMPLETE;
        }

        pProperties += count;
        *pPropertyCount -= count;

        if (Hal::Get().GetDebugReportIndex() < 0) {
            if (!*pPropertyCount) {
                *pPropertyCount = count;
                return VK_INCOMPLETE;
            }

            pProperties[0] = loader_debug_report_extension;
            pProperties += 1;
            *pPropertyCount -= 1;
        }
    }

    ATRACE_BEGIN("driver.EnumerateInstanceExtensionProperties");
    VkResult result = Hal::Device().EnumerateInstanceExtensionProperties(
        pLayerName, pPropertyCount, pProperties);
    ATRACE_END();

    if (!pLayerName && (result == VK_SUCCESS || result == VK_INCOMPLETE)) {
        int idx = Hal::Get().GetDebugReportIndex();
        if (idx < 0) {
            *pPropertyCount += 1;
        } else if (pProperties &&
                   static_cast<uint32_t>(idx) < *pPropertyCount) {
            pProperties[idx].specVersion =
                std::min(pProperties[idx].specVersion,
                         loader_debug_report_extension.specVersion);
        }

        *pPropertyCount += loader_extensions.size();
    }

    return result;
}

bool QueryPresentationProperties(
    VkPhysicalDevice physicalDevice,
    VkPhysicalDevicePresentationPropertiesANDROID *presentation_properties)
{
    const InstanceData& data = GetData(physicalDevice);

    // GPDP2 must be present and enabled on the instance.
    if (!data.driver.GetPhysicalDeviceProperties2KHR &&
        !data.driver.GetPhysicalDeviceProperties2)
        return false;

    // Request the android-specific presentation properties via GPDP2
    VkPhysicalDeviceProperties2KHR properties = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR,
        presentation_properties,
        {}
    };

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
    presentation_properties->sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENTATION_PROPERTIES_ANDROID;
#pragma clang diagnostic pop
    presentation_properties->pNext = nullptr;
    presentation_properties->sharedImage = VK_FALSE;

    if (data.driver.GetPhysicalDeviceProperties2KHR) {
        data.driver.GetPhysicalDeviceProperties2KHR(physicalDevice,
                                                    &properties);
    } else {
        data.driver.GetPhysicalDeviceProperties2(physicalDevice, &properties);
    }

    return true;
}

VkResult EnumerateDeviceExtensionProperties(
    VkPhysicalDevice physicalDevice,
    const char* pLayerName,
    uint32_t* pPropertyCount,
    VkExtensionProperties* pProperties) {
    const InstanceData& data = GetData(physicalDevice);
    // extensions that are unconditionally exposed by the loader
    android::Vector<VkExtensionProperties> loader_extensions;
    loader_extensions.push_back({
        VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME,
        VK_KHR_INCREMENTAL_PRESENT_SPEC_VERSION});

    bool hdrBoardConfig =
        getBool<ISurfaceFlingerConfigs, &ISurfaceFlingerConfigs::hasHDRDisplay>(
            false);
    if (hdrBoardConfig) {
        loader_extensions.push_back({VK_EXT_HDR_METADATA_EXTENSION_NAME,
                                     VK_EXT_HDR_METADATA_SPEC_VERSION});
    }

    VkPhysicalDevicePresentationPropertiesANDROID presentation_properties;
    if (QueryPresentationProperties(physicalDevice, &presentation_properties) &&
        presentation_properties.sharedImage) {
        loader_extensions.push_back({
            VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME,
            VK_KHR_SHARED_PRESENTABLE_IMAGE_SPEC_VERSION});
    }

    // conditionally add VK_GOOGLE_display_timing if present timestamps are
    // supported by the driver:
    const std::string timestamp_property("service.sf.present_timestamp");
    android::base::WaitForPropertyCreation(timestamp_property);
    if (android::base::GetBoolProperty(timestamp_property, true)) {
        loader_extensions.push_back({
                VK_GOOGLE_DISPLAY_TIMING_EXTENSION_NAME,
                VK_GOOGLE_DISPLAY_TIMING_SPEC_VERSION});
    }

    // enumerate our extensions first
    if (!pLayerName && pProperties) {
        uint32_t count = std::min(
            *pPropertyCount, static_cast<uint32_t>(loader_extensions.size()));

        std::copy_n(loader_extensions.begin(), count, pProperties);

        if (count < loader_extensions.size()) {
            *pPropertyCount = count;
            return VK_INCOMPLETE;
        }

        pProperties += count;
        *pPropertyCount -= count;
    }

    ATRACE_BEGIN("driver.EnumerateDeviceExtensionProperties");
    VkResult result = data.driver.EnumerateDeviceExtensionProperties(
        physicalDevice, pLayerName, pPropertyCount, pProperties);
    ATRACE_END();

    if (pProperties) {
        // map VK_ANDROID_native_buffer to VK_KHR_swapchain
        for (uint32_t i = 0; i < *pPropertyCount; i++) {
            auto& prop = pProperties[i];

            if (strcmp(prop.extensionName,
                       VK_ANDROID_NATIVE_BUFFER_EXTENSION_NAME) != 0)
                continue;

            memcpy(prop.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                   sizeof(VK_KHR_SWAPCHAIN_EXTENSION_NAME));

            if (prop.specVersion >= 8) {
                prop.specVersion = VK_KHR_SWAPCHAIN_SPEC_VERSION;
            } else {
                prop.specVersion = 68;
            }
        }
    }

    // restore loader extension count
    if (!pLayerName && (result == VK_SUCCESS || result == VK_INCOMPLETE)) {
        *pPropertyCount += loader_extensions.size();
    }

    return result;
}

VkResult CreateInstance(const VkInstanceCreateInfo* pCreateInfo,
                        const VkAllocationCallbacks* pAllocator,
                        VkInstance* pInstance) {
    const VkAllocationCallbacks& data_allocator =
        (pAllocator) ? *pAllocator : GetDefaultAllocator();

    CreateInfoWrapper wrapper(*pCreateInfo, data_allocator);
    VkResult result = wrapper.Validate();
    if (result != VK_SUCCESS)
        return result;

    ATRACE_BEGIN("AllocateInstanceData");
    InstanceData* data = AllocateInstanceData(data_allocator);
    ATRACE_END();
    if (!data)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    data->hook_extensions |= wrapper.GetHookExtensions();

    ATRACE_BEGIN("autoDowngradeApiVersion");
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
    uint32_t api_version = ((pCreateInfo->pApplicationInfo)
                                ? pCreateInfo->pApplicationInfo->apiVersion
                                : VK_API_VERSION_1_0);
    uint32_t api_major_version = VK_VERSION_MAJOR(api_version);
    uint32_t api_minor_version = VK_VERSION_MINOR(api_version);
    uint32_t icd_api_version;
    PFN_vkEnumerateInstanceVersion pfn_enumerate_instance_version =
        reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
            Hal::Device().GetInstanceProcAddr(nullptr,
                                              "vkEnumerateInstanceVersion"));
    if (!pfn_enumerate_instance_version) {
        icd_api_version = VK_API_VERSION_1_0;
    } else {
        ATRACE_BEGIN("pfn_enumerate_instance_version");
        result = (*pfn_enumerate_instance_version)(&icd_api_version);
        ATRACE_END();
    }
    uint32_t icd_api_major_version = VK_VERSION_MAJOR(icd_api_version);
    uint32_t icd_api_minor_version = VK_VERSION_MINOR(icd_api_version);

    if ((icd_api_major_version == 1) && (icd_api_minor_version == 0) &&
        ((api_major_version > 1) || (api_minor_version > 0))) {
        api_version = VK_API_VERSION_1_0;
        wrapper.DowngradeApiVersion();
    }
#pragma clang diagnostic pop
    ATRACE_END();

    // call into the driver
    VkInstance instance;
    ATRACE_BEGIN("driver.CreateInstance");
    result = Hal::Device().CreateInstance(
        static_cast<const VkInstanceCreateInfo*>(wrapper), pAllocator,
        &instance);
    ATRACE_END();
    if (result != VK_SUCCESS) {
        FreeInstanceData(data, data_allocator);
        return result;
    }

    // initialize InstanceDriverTable
    if (!SetData(instance, *data) ||
        !InitDriverTable(instance, Hal::Device().GetInstanceProcAddr,
                         wrapper.GetHalExtensions())) {
        data->driver.DestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(
            Hal::Device().GetInstanceProcAddr(instance, "vkDestroyInstance"));
        if (data->driver.DestroyInstance)
            data->driver.DestroyInstance(instance, pAllocator);

        FreeInstanceData(data, data_allocator);

        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    data->get_device_proc_addr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
        Hal::Device().GetInstanceProcAddr(instance, "vkGetDeviceProcAddr"));
    if (!data->get_device_proc_addr) {
        data->driver.DestroyInstance(instance, pAllocator);
        FreeInstanceData(data, data_allocator);

        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    *pInstance = instance;

    return VK_SUCCESS;
}

void DestroyInstance(VkInstance instance,
                     const VkAllocationCallbacks* pAllocator) {
    InstanceData& data = GetData(instance);
    data.driver.DestroyInstance(instance, pAllocator);

    VkAllocationCallbacks local_allocator;
    if (!pAllocator) {
        local_allocator = data.allocator;
        pAllocator = &local_allocator;
    }

    FreeInstanceData(&data, *pAllocator);
}

VkResult CreateDevice(VkPhysicalDevice physicalDevice,
                      const VkDeviceCreateInfo* pCreateInfo,
                      const VkAllocationCallbacks* pAllocator,
                      VkDevice* pDevice) {
    const InstanceData& instance_data = GetData(physicalDevice);
    const VkAllocationCallbacks& data_allocator =
        (pAllocator) ? *pAllocator : instance_data.allocator;

    CreateInfoWrapper wrapper(physicalDevice, *pCreateInfo, data_allocator);
    VkResult result = wrapper.Validate();
    if (result != VK_SUCCESS)
        return result;

    ATRACE_BEGIN("AllocateDeviceData");
    DeviceData* data = AllocateDeviceData(data_allocator,
                                          instance_data.debug_report_callbacks);
    ATRACE_END();
    if (!data)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    data->hook_extensions |= wrapper.GetHookExtensions();

    // call into the driver
    VkDevice dev;
    ATRACE_BEGIN("driver.CreateDevice");
    result = instance_data.driver.CreateDevice(
        physicalDevice, static_cast<const VkDeviceCreateInfo*>(wrapper),
        pAllocator, &dev);
    ATRACE_END();
    if (result != VK_SUCCESS) {
        FreeDeviceData(data, data_allocator);
        return result;
    }

    // initialize DeviceDriverTable
    if (!SetData(dev, *data) ||
        !InitDriverTable(dev, instance_data.get_device_proc_addr,
                         wrapper.GetHalExtensions())) {
        data->driver.DestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(
            instance_data.get_device_proc_addr(dev, "vkDestroyDevice"));
        if (data->driver.DestroyDevice)
            data->driver.DestroyDevice(dev, pAllocator);

        FreeDeviceData(data, data_allocator);

        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    // sanity check ANDROID_native_buffer implementation, whose set of
    // entrypoints varies according to the spec version.
    if ((wrapper.GetHalExtensions()[ProcHook::ANDROID_native_buffer]) &&
        !data->driver.GetSwapchainGrallocUsageANDROID &&
        !data->driver.GetSwapchainGrallocUsage2ANDROID) {
        ALOGE("Driver's implementation of ANDROID_native_buffer is broken;"
              " must expose at least one of "
              "vkGetSwapchainGrallocUsageANDROID or "
              "vkGetSwapchainGrallocUsage2ANDROID");

        data->driver.DestroyDevice(dev, pAllocator);
        FreeDeviceData(data, data_allocator);

        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    VkPhysicalDeviceProperties properties;
    ATRACE_BEGIN("driver.GetPhysicalDeviceProperties");
    instance_data.driver.GetPhysicalDeviceProperties(physicalDevice,
                                                     &properties);
    ATRACE_END();

    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
        // Log that the app is hitting software Vulkan implementation
        android::GraphicsEnv::getInstance().setTargetStats(
            android::GraphicsEnv::Stats::CPU_VULKAN_IN_USE);
    }

    data->driver_device = dev;
    data->driver_version = properties.driverVersion;

    *pDevice = dev;

    return VK_SUCCESS;
}

void DestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
    DeviceData& data = GetData(device);
    data.driver.DestroyDevice(device, pAllocator);

    VkAllocationCallbacks local_allocator;
    if (!pAllocator) {
        local_allocator = data.allocator;
        pAllocator = &local_allocator;
    }

    FreeDeviceData(&data, *pAllocator);
}

VkResult EnumeratePhysicalDevices(VkInstance instance,
                                  uint32_t* pPhysicalDeviceCount,
                                  VkPhysicalDevice* pPhysicalDevices) {
    ATRACE_CALL();

    const auto& data = GetData(instance);

    VkResult result = data.driver.EnumeratePhysicalDevices(
        instance, pPhysicalDeviceCount, pPhysicalDevices);
    if ((result == VK_SUCCESS || result == VK_INCOMPLETE) && pPhysicalDevices) {
        for (uint32_t i = 0; i < *pPhysicalDeviceCount; i++)
            SetData(pPhysicalDevices[i], data);
    }

    return result;
}

VkResult EnumeratePhysicalDeviceGroups(
    VkInstance instance,
    uint32_t* pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    ATRACE_CALL();

    VkResult result = VK_SUCCESS;
    const auto& data = GetData(instance);

    if (!data.driver.EnumeratePhysicalDeviceGroups) {
        uint32_t device_count = 0;
        result = EnumeratePhysicalDevices(instance, &device_count, nullptr);
        if (result < 0)
            return result;

        if (!pPhysicalDeviceGroupProperties) {
            *pPhysicalDeviceGroupCount = device_count;
            return result;
        }

        if (!device_count) {
            *pPhysicalDeviceGroupCount = 0;
            return result;
        }
        device_count = std::min(device_count, *pPhysicalDeviceGroupCount);
        if (!device_count)
            return VK_INCOMPLETE;

        android::Vector<VkPhysicalDevice> devices;
        devices.resize(device_count);
        *pPhysicalDeviceGroupCount = device_count;
        result = EnumeratePhysicalDevices(instance, &device_count,
                                          devices.editArray());
        if (result < 0)
            return result;

        for (uint32_t i = 0; i < device_count; ++i) {
            pPhysicalDeviceGroupProperties[i].physicalDeviceCount = 1;
            pPhysicalDeviceGroupProperties[i].physicalDevices[0] = devices[i];
            pPhysicalDeviceGroupProperties[i].subsetAllocation = 0;
        }
    } else {
        result = data.driver.EnumeratePhysicalDeviceGroups(
            instance, pPhysicalDeviceGroupCount,
            pPhysicalDeviceGroupProperties);
        if ((result == VK_SUCCESS || result == VK_INCOMPLETE) &&
            *pPhysicalDeviceGroupCount && pPhysicalDeviceGroupProperties) {
            for (uint32_t i = 0; i < *pPhysicalDeviceGroupCount; i++) {
                for (uint32_t j = 0;
                     j < pPhysicalDeviceGroupProperties[i].physicalDeviceCount;
                     j++) {
                    SetData(
                        pPhysicalDeviceGroupProperties[i].physicalDevices[j],
                        data);
                }
            }
        }
    }

    return result;
}

void GetDeviceQueue(VkDevice device,
                    uint32_t queueFamilyIndex,
                    uint32_t queueIndex,
                    VkQueue* pQueue) {
    ATRACE_CALL();

    const auto& data = GetData(device);

    data.driver.GetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
    SetData(*pQueue, data);
}

void GetDeviceQueue2(VkDevice device,
                     const VkDeviceQueueInfo2* pQueueInfo,
                     VkQueue* pQueue) {
    ATRACE_CALL();

    const auto& data = GetData(device);

    data.driver.GetDeviceQueue2(device, pQueueInfo, pQueue);
    if (*pQueue != VK_NULL_HANDLE) SetData(*pQueue, data);
}

VKAPI_ATTR VkResult
AllocateCommandBuffers(VkDevice device,
                       const VkCommandBufferAllocateInfo* pAllocateInfo,
                       VkCommandBuffer* pCommandBuffers) {
    ATRACE_CALL();

    const auto& data = GetData(device);

    VkResult result = data.driver.AllocateCommandBuffers(device, pAllocateInfo,
                                                         pCommandBuffers);
    if (result == VK_SUCCESS) {
        for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; i++)
            SetData(pCommandBuffers[i], data);
    }

    return result;
}

}  // namespace driver
}  // namespace vulkan
