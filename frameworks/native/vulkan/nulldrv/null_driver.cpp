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

#include <hardware/hwvulkan.h>

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <algorithm>
#include <array>

#include <log/log.h>

#include "null_driver_gen.h"

using namespace null_driver;

struct VkPhysicalDevice_T {
    hwvulkan_dispatch_t dispatch;
};

struct VkInstance_T {
    hwvulkan_dispatch_t dispatch;
    VkAllocationCallbacks allocator;
    VkPhysicalDevice_T physical_device;
    uint64_t next_callback_handle;
};

struct VkQueue_T {
    hwvulkan_dispatch_t dispatch;
};

struct VkCommandBuffer_T {
    hwvulkan_dispatch_t dispatch;
};

namespace {
// Handles for non-dispatchable objects are either pointers, or arbitrary
// 64-bit non-zero values. We only use pointers when we need to keep state for
// the object even in a null driver. For the rest, we form a handle as:
//   [63:63] = 1 to distinguish from pointer handles*
//   [62:56] = non-zero handle type enum value
//   [55: 0] = per-handle-type incrementing counter
// * This works because virtual addresses with the high bit set are reserved
// for kernel data in all ABIs we run on.
//
// We never reclaim handles on vkDestroy*. It's not even necessary for us to
// have distinct handles for live objects, and practically speaking we won't
// ever create 2^56 objects of the same type from a single VkDevice in a null
// driver.
//
// Using a namespace here instead of 'enum class' since we want scoped
// constants but also want implicit conversions to integral types.
namespace HandleType {
enum Enum {
    kBufferView,
    kDebugReportCallbackEXT,
    kDescriptorPool,
    kDescriptorSet,
    kDescriptorSetLayout,
    kEvent,
    kFence,
    kFramebuffer,
    kImageView,
    kPipeline,
    kPipelineCache,
    kPipelineLayout,
    kQueryPool,
    kRenderPass,
    kSampler,
    kSemaphore,
    kShaderModule,

    kNumTypes
};
}  // namespace HandleType

const VkDeviceSize kMaxDeviceMemory = 0x10000000;  // 256 MiB, arbitrary

}  // anonymous namespace

struct VkDevice_T {
    hwvulkan_dispatch_t dispatch;
    VkAllocationCallbacks allocator;
    VkInstance_T* instance;
    VkQueue_T queue;
    std::array<uint64_t, HandleType::kNumTypes> next_handle;
};

// -----------------------------------------------------------------------------
// Declare HAL_MODULE_INFO_SYM early so it can be referenced by nulldrv_device
// later.

namespace {
int OpenDevice(const hw_module_t* module, const char* id, hw_device_t** device);
hw_module_methods_t nulldrv_module_methods = {.open = OpenDevice};
}  // namespace

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
__attribute__((visibility("default"))) hwvulkan_module_t HAL_MODULE_INFO_SYM = {
    .common =
        {
            .tag = HARDWARE_MODULE_TAG,
            .module_api_version = HWVULKAN_MODULE_API_VERSION_0_1,
            .hal_api_version = HARDWARE_HAL_API_VERSION,
            .id = HWVULKAN_HARDWARE_MODULE_ID,
            .name = "Null Vulkan Driver",
            .author = "The Android Open Source Project",
            .methods = &nulldrv_module_methods,
        },
};
#pragma clang diagnostic pop

// -----------------------------------------------------------------------------

namespace {

int CloseDevice(struct hw_device_t* /*device*/) {
    // nothing to do - opening a device doesn't allocate any resources
    return 0;
}

hwvulkan_device_t nulldrv_device = {
    .common =
        {
            .tag = HARDWARE_DEVICE_TAG,
            .version = HWVULKAN_DEVICE_API_VERSION_0_1,
            .module = &HAL_MODULE_INFO_SYM.common,
            .close = CloseDevice,
        },
    .EnumerateInstanceExtensionProperties =
        EnumerateInstanceExtensionProperties,
    .CreateInstance = CreateInstance,
    .GetInstanceProcAddr = GetInstanceProcAddr};

int OpenDevice(const hw_module_t* /*module*/,
               const char* id,
               hw_device_t** device) {
    if (strcmp(id, HWVULKAN_DEVICE_0) == 0) {
        *device = &nulldrv_device.common;
        return 0;
    }
    return -ENOENT;
}

VkInstance_T* GetInstanceFromPhysicalDevice(
    VkPhysicalDevice_T* physical_device) {
    return reinterpret_cast<VkInstance_T*>(
        reinterpret_cast<uintptr_t>(physical_device) -
        offsetof(VkInstance_T, physical_device));
}

uint64_t AllocHandle(uint64_t type, uint64_t* next_handle) {
    const uint64_t kHandleMask = (UINT64_C(1) << 56) - 1;
    ALOGE_IF(*next_handle == kHandleMask,
             "non-dispatchable handles of type=%" PRIu64
             " are about to overflow",
             type);
    return (UINT64_C(1) << 63) | ((type & 0x7) << 56) |
           ((*next_handle)++ & kHandleMask);
}

template <class Handle>
Handle AllocHandle(VkInstance instance, HandleType::Enum type) {
    return reinterpret_cast<Handle>(
        AllocHandle(type, &instance->next_callback_handle));
}

template <class Handle>
Handle AllocHandle(VkDevice device, HandleType::Enum type) {
    return reinterpret_cast<Handle>(
        AllocHandle(type, &device->next_handle[type]));
}

VKAPI_ATTR void* DefaultAllocate(void*,
                                 size_t size,
                                 size_t alignment,
                                 VkSystemAllocationScope) {
    void* ptr = nullptr;
    // Vulkan requires 'alignment' to be a power of two, but posix_memalign
    // additionally requires that it be at least sizeof(void*).
    int ret = posix_memalign(&ptr, std::max(alignment, sizeof(void*)), size);
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
    // The null driver never reallocs, so this doesn't matter. If that changes,
    // or if this code is copied into some other project, this should probably
    // have a heuristic to allocate-copy-free when doing so will save "enough"
    // space.
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
    free(ptr);
}

const VkAllocationCallbacks kDefaultAllocCallbacks = {
    .pUserData = nullptr,
    .pfnAllocation = DefaultAllocate,
    .pfnReallocation = DefaultReallocate,
    .pfnFree = DefaultFree,
};

}  // namespace

namespace null_driver {

#define DEFINE_OBJECT_HANDLE_CONVERSION(T)              \
    T* Get##T##FromHandle(Vk##T h);                     \
    T* Get##T##FromHandle(Vk##T h) {                    \
        return reinterpret_cast<T*>(uintptr_t(h));      \
    }                                                   \
    Vk##T GetHandleTo##T(const T* obj);                 \
    Vk##T GetHandleTo##T(const T* obj) {                \
        return Vk##T(reinterpret_cast<uintptr_t>(obj)); \
    }

// -----------------------------------------------------------------------------
// Global

VKAPI_ATTR
VkResult EnumerateInstanceVersion(uint32_t* pApiVersion) {
    *pApiVersion = VK_API_VERSION_1_1;
    return VK_SUCCESS;
}

VKAPI_ATTR
VkResult EnumerateInstanceExtensionProperties(
    const char* layer_name,
    uint32_t* count,
    VkExtensionProperties* properties) {
    if (layer_name) {
        ALOGW(
            "Driver vkEnumerateInstanceExtensionProperties shouldn't be called "
            "with a layer name ('%s')",
            layer_name);
    }

    const VkExtensionProperties kExtensions[] = {
        {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION}};
    const uint32_t kExtensionsCount =
        sizeof(kExtensions) / sizeof(kExtensions[0]);

    if (!properties || *count > kExtensionsCount)
        *count = kExtensionsCount;
    if (properties)
        std::copy(kExtensions, kExtensions + *count, properties);
    return *count < kExtensionsCount ? VK_INCOMPLETE : VK_SUCCESS;
}

VKAPI_ATTR
VkResult CreateInstance(const VkInstanceCreateInfo* create_info,
                        const VkAllocationCallbacks* allocator,
                        VkInstance* out_instance) {
    if (!allocator)
        allocator = &kDefaultAllocCallbacks;

    VkInstance_T* instance =
        static_cast<VkInstance_T*>(allocator->pfnAllocation(
            allocator->pUserData, sizeof(VkInstance_T), alignof(VkInstance_T),
            VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE));
    if (!instance)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    instance->dispatch.magic = HWVULKAN_DISPATCH_MAGIC;
    instance->allocator = *allocator;
    instance->physical_device.dispatch.magic = HWVULKAN_DISPATCH_MAGIC;
    instance->next_callback_handle = 0;

    for (uint32_t i = 0; i < create_info->enabledExtensionCount; i++) {
        if (strcmp(create_info->ppEnabledExtensionNames[i],
                   VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0) {
            ALOGV("instance extension '%s' requested",
                  create_info->ppEnabledExtensionNames[i]);
        } else if (strcmp(create_info->ppEnabledExtensionNames[i],
                   VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0) {
            ALOGV("instance extension '%s' requested",
                  create_info->ppEnabledExtensionNames[i]);
        } else {
            ALOGW("unsupported extension '%s' requested",
                  create_info->ppEnabledExtensionNames[i]);
        }
    }

    *out_instance = instance;
    return VK_SUCCESS;
}

VKAPI_ATTR
PFN_vkVoidFunction GetInstanceProcAddr(VkInstance instance, const char* name) {
    return instance ? GetInstanceProcAddr(name) : GetGlobalProcAddr(name);
}

VKAPI_ATTR
PFN_vkVoidFunction GetDeviceProcAddr(VkDevice, const char* name) {
    return GetInstanceProcAddr(name);
}

// -----------------------------------------------------------------------------
// Instance

void DestroyInstance(VkInstance instance,
                     const VkAllocationCallbacks* /*allocator*/) {
    instance->allocator.pfnFree(instance->allocator.pUserData, instance);
}

// -----------------------------------------------------------------------------
// PhysicalDevice

VkResult EnumeratePhysicalDevices(VkInstance instance,
                                  uint32_t* physical_device_count,
                                  VkPhysicalDevice* physical_devices) {
    if (!physical_devices)
        *physical_device_count = 1;
    else if (*physical_device_count == 0)
        return VK_INCOMPLETE;
    else {
        physical_devices[0] = &instance->physical_device;
        *physical_device_count = 1;
    }
    return VK_SUCCESS;
}

VkResult EnumerateDeviceLayerProperties(VkPhysicalDevice /*gpu*/,
                                        uint32_t* count,
                                        VkLayerProperties* /*properties*/) {
    ALOGW("Driver vkEnumerateDeviceLayerProperties shouldn't be called");
    *count = 0;
    return VK_SUCCESS;
}

VkResult EnumerateDeviceExtensionProperties(VkPhysicalDevice /*gpu*/,
                                            const char* layer_name,
                                            uint32_t* count,
                                            VkExtensionProperties* properties) {
    if (layer_name) {
        ALOGW(
            "Driver vkEnumerateDeviceExtensionProperties shouldn't be called "
            "with a layer name ('%s')",
            layer_name);
        *count = 0;
        return VK_SUCCESS;
    }

    const VkExtensionProperties kExtensions[] = {
        {VK_ANDROID_NATIVE_BUFFER_EXTENSION_NAME,
         VK_ANDROID_NATIVE_BUFFER_SPEC_VERSION}};
    const uint32_t kExtensionsCount =
        sizeof(kExtensions) / sizeof(kExtensions[0]);

    if (!properties || *count > kExtensionsCount)
        *count = kExtensionsCount;
    if (properties)
        std::copy(kExtensions, kExtensions + *count, properties);
    return *count < kExtensionsCount ? VK_INCOMPLETE : VK_SUCCESS;
}

void GetPhysicalDeviceProperties(VkPhysicalDevice,
                                 VkPhysicalDeviceProperties* properties) {
    properties->apiVersion = VK_MAKE_VERSION(1, 0, VK_HEADER_VERSION);
    properties->driverVersion = VK_MAKE_VERSION(0, 0, 1);
    properties->vendorID = 0;
    properties->deviceID = 0;
    properties->deviceType = VK_PHYSICAL_DEVICE_TYPE_OTHER;
    strcpy(properties->deviceName, "Android Vulkan Null Driver");
    memset(properties->pipelineCacheUUID, 0,
           sizeof(properties->pipelineCacheUUID));
    properties->limits = VkPhysicalDeviceLimits{
        4096,     // maxImageDimension1D
        4096,     // maxImageDimension2D
        256,      // maxImageDimension3D
        4096,     // maxImageDimensionCube
        256,      // maxImageArrayLayers
        65536,    // maxTexelBufferElements
        16384,    // maxUniformBufferRange
        1 << 27,  // maxStorageBufferRange
        128,      // maxPushConstantsSize
        4096,     // maxMemoryAllocationCount
        4000,     // maxSamplerAllocationCount
        1,        // bufferImageGranularity
        0,        // sparseAddressSpaceSize
        4,        // maxBoundDescriptorSets
        16,       // maxPerStageDescriptorSamplers
        12,       // maxPerStageDescriptorUniformBuffers
        4,        // maxPerStageDescriptorStorageBuffers
        16,       // maxPerStageDescriptorSampledImages
        4,        // maxPerStageDescriptorStorageImages
        4,        // maxPerStageDescriptorInputAttachments
        128,      // maxPerStageResources
        96,       // maxDescriptorSetSamplers
        72,       // maxDescriptorSetUniformBuffers
        8,        // maxDescriptorSetUniformBuffersDynamic
        24,       // maxDescriptorSetStorageBuffers
        4,        // maxDescriptorSetStorageBuffersDynamic
        96,       // maxDescriptorSetSampledImages
        24,       // maxDescriptorSetStorageImages
        4,        // maxDescriptorSetInputAttachments
        16,       // maxVertexInputAttributes
        16,       // maxVertexInputBindings
        2047,     // maxVertexInputAttributeOffset
        2048,     // maxVertexInputBindingStride
        64,       // maxVertexOutputComponents
        0,        // maxTessellationGenerationLevel
        0,        // maxTessellationPatchSize
        0,        // maxTessellationControlPerVertexInputComponents
        0,        // maxTessellationControlPerVertexOutputComponents
        0,        // maxTessellationControlPerPatchOutputComponents
        0,        // maxTessellationControlTotalOutputComponents
        0,        // maxTessellationEvaluationInputComponents
        0,        // maxTessellationEvaluationOutputComponents
        0,        // maxGeometryShaderInvocations
        0,        // maxGeometryInputComponents
        0,        // maxGeometryOutputComponents
        0,        // maxGeometryOutputVertices
        0,        // maxGeometryTotalOutputComponents
        64,       // maxFragmentInputComponents
        4,        // maxFragmentOutputAttachments
        0,        // maxFragmentDualSrcAttachments
        4,        // maxFragmentCombinedOutputResources
        16384,    // maxComputeSharedMemorySize
        {65536, 65536, 65536},  // maxComputeWorkGroupCount[3]
        128,                    // maxComputeWorkGroupInvocations
        {128, 128, 64},         // maxComputeWorkGroupSize[3]
        4,                      // subPixelPrecisionBits
        4,                      // subTexelPrecisionBits
        4,                      // mipmapPrecisionBits
        UINT32_MAX,             // maxDrawIndexedIndexValue
        1,                      // maxDrawIndirectCount
        2,                      // maxSamplerLodBias
        1,                      // maxSamplerAnisotropy
        1,                      // maxViewports
        {4096, 4096},           // maxViewportDimensions[2]
        {-8192.0f, 8191.0f},    // viewportBoundsRange[2]
        0,                      // viewportSubPixelBits
        64,                     // minMemoryMapAlignment
        256,                    // minTexelBufferOffsetAlignment
        256,                    // minUniformBufferOffsetAlignment
        256,                    // minStorageBufferOffsetAlignment
        -8,                     // minTexelOffset
        7,                      // maxTexelOffset
        0,                      // minTexelGatherOffset
        0,                      // maxTexelGatherOffset
        0.0f,                   // minInterpolationOffset
        0.0f,                   // maxInterpolationOffset
        0,                      // subPixelInterpolationOffsetBits
        4096,                   // maxFramebufferWidth
        4096,                   // maxFramebufferHeight
        256,                    // maxFramebufferLayers
        VK_SAMPLE_COUNT_1_BIT |
            VK_SAMPLE_COUNT_4_BIT,  // framebufferColorSampleCounts
        VK_SAMPLE_COUNT_1_BIT |
            VK_SAMPLE_COUNT_4_BIT,  // framebufferDepthSampleCounts
        VK_SAMPLE_COUNT_1_BIT |
            VK_SAMPLE_COUNT_4_BIT,  // framebufferStencilSampleCounts
        VK_SAMPLE_COUNT_1_BIT |
            VK_SAMPLE_COUNT_4_BIT,  // framebufferNoAttachmentsSampleCounts
        4,                          // maxColorAttachments
        VK_SAMPLE_COUNT_1_BIT |
            VK_SAMPLE_COUNT_4_BIT,  // sampledImageColorSampleCounts
        VK_SAMPLE_COUNT_1_BIT,      // sampledImageIntegerSampleCounts
        VK_SAMPLE_COUNT_1_BIT |
            VK_SAMPLE_COUNT_4_BIT,  // sampledImageDepthSampleCounts
        VK_SAMPLE_COUNT_1_BIT |
            VK_SAMPLE_COUNT_4_BIT,  // sampledImageStencilSampleCounts
        VK_SAMPLE_COUNT_1_BIT,      // storageImageSampleCounts
        1,                          // maxSampleMaskWords
        VK_TRUE,                    // timestampComputeAndGraphics
        1,                          // timestampPeriod
        0,                          // maxClipDistances
        0,                          // maxCullDistances
        0,                          // maxCombinedClipAndCullDistances
        2,                          // discreteQueuePriorities
        {1.0f, 1.0f},               // pointSizeRange[2]
        {1.0f, 1.0f},               // lineWidthRange[2]
        0.0f,                       // pointSizeGranularity
        0.0f,                       // lineWidthGranularity
        VK_TRUE,                    // strictLines
        VK_TRUE,                    // standardSampleLocations
        1,                          // optimalBufferCopyOffsetAlignment
        1,                          // optimalBufferCopyRowPitchAlignment
        64,                         // nonCoherentAtomSize
    };
}

void GetPhysicalDeviceProperties2KHR(VkPhysicalDevice physical_device,
                                  VkPhysicalDeviceProperties2KHR* properties) {
    GetPhysicalDeviceProperties(physical_device, &properties->properties);

    while (properties->pNext) {
        properties = reinterpret_cast<VkPhysicalDeviceProperties2KHR *>(properties->pNext);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
        switch ((VkFlags)properties->sType) {
        case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENTATION_PROPERTIES_ANDROID: {
            VkPhysicalDevicePresentationPropertiesANDROID *presentation_properties =
                reinterpret_cast<VkPhysicalDevicePresentationPropertiesANDROID *>(properties);
#pragma clang diagnostic pop

                // Claim that we do all the right things for the loader to
                // expose KHR_shared_presentable_image on our behalf.
                presentation_properties->sharedImage = VK_TRUE;
            } break;

        default:
            // Silently ignore other extension query structs
            break;
        }
    }
}

void GetPhysicalDeviceQueueFamilyProperties(
    VkPhysicalDevice,
    uint32_t* count,
    VkQueueFamilyProperties* properties) {
    if (!properties || *count > 1)
        *count = 1;
    if (properties && *count == 1) {
        properties->queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT |
                                 VK_QUEUE_TRANSFER_BIT;
        properties->queueCount = 1;
        properties->timestampValidBits = 64;
        properties->minImageTransferGranularity = VkExtent3D{1, 1, 1};
    }
}

void GetPhysicalDeviceQueueFamilyProperties2KHR(VkPhysicalDevice physical_device, uint32_t* count, VkQueueFamilyProperties2KHR* properties) {
    // note: even though multiple structures, this is safe to forward in this
    // case since we only expose one queue family.
    GetPhysicalDeviceQueueFamilyProperties(physical_device, count, properties ? &properties->queueFamilyProperties : nullptr);
}

void GetPhysicalDeviceMemoryProperties(
    VkPhysicalDevice,
    VkPhysicalDeviceMemoryProperties* properties) {
    properties->memoryTypeCount = 1;
    properties->memoryTypes[0].propertyFlags =
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
        VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    properties->memoryTypes[0].heapIndex = 0;
    properties->memoryHeapCount = 1;
    properties->memoryHeaps[0].size = kMaxDeviceMemory;
    properties->memoryHeaps[0].flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;
}

void GetPhysicalDeviceMemoryProperties2KHR(VkPhysicalDevice physical_device, VkPhysicalDeviceMemoryProperties2KHR* properties) {
    GetPhysicalDeviceMemoryProperties(physical_device, &properties->memoryProperties);
}

void GetPhysicalDeviceFeatures(VkPhysicalDevice /*gpu*/,
                               VkPhysicalDeviceFeatures* features) {
    *features = VkPhysicalDeviceFeatures{
        VK_TRUE,   // robustBufferAccess
        VK_FALSE,  // fullDrawIndexUint32
        VK_FALSE,  // imageCubeArray
        VK_FALSE,  // independentBlend
        VK_FALSE,  // geometryShader
        VK_FALSE,  // tessellationShader
        VK_FALSE,  // sampleRateShading
        VK_FALSE,  // dualSrcBlend
        VK_FALSE,  // logicOp
        VK_FALSE,  // multiDrawIndirect
        VK_FALSE,  // drawIndirectFirstInstance
        VK_FALSE,  // depthClamp
        VK_FALSE,  // depthBiasClamp
        VK_FALSE,  // fillModeNonSolid
        VK_FALSE,  // depthBounds
        VK_FALSE,  // wideLines
        VK_FALSE,  // largePoints
        VK_FALSE,  // alphaToOne
        VK_FALSE,  // multiViewport
        VK_FALSE,  // samplerAnisotropy
        VK_FALSE,  // textureCompressionETC2
        VK_FALSE,  // textureCompressionASTC_LDR
        VK_FALSE,  // textureCompressionBC
        VK_FALSE,  // occlusionQueryPrecise
        VK_FALSE,  // pipelineStatisticsQuery
        VK_FALSE,  // vertexPipelineStoresAndAtomics
        VK_FALSE,  // fragmentStoresAndAtomics
        VK_FALSE,  // shaderTessellationAndGeometryPointSize
        VK_FALSE,  // shaderImageGatherExtended
        VK_FALSE,  // shaderStorageImageExtendedFormats
        VK_FALSE,  // shaderStorageImageMultisample
        VK_FALSE,  // shaderStorageImageReadWithoutFormat
        VK_FALSE,  // shaderStorageImageWriteWithoutFormat
        VK_FALSE,  // shaderUniformBufferArrayDynamicIndexing
        VK_FALSE,  // shaderSampledImageArrayDynamicIndexing
        VK_FALSE,  // shaderStorageBufferArrayDynamicIndexing
        VK_FALSE,  // shaderStorageImageArrayDynamicIndexing
        VK_FALSE,  // shaderClipDistance
        VK_FALSE,  // shaderCullDistance
        VK_FALSE,  // shaderFloat64
        VK_FALSE,  // shaderInt64
        VK_FALSE,  // shaderInt16
        VK_FALSE,  // shaderResourceResidency
        VK_FALSE,  // shaderResourceMinLod
        VK_FALSE,  // sparseBinding
        VK_FALSE,  // sparseResidencyBuffer
        VK_FALSE,  // sparseResidencyImage2D
        VK_FALSE,  // sparseResidencyImage3D
        VK_FALSE,  // sparseResidency2Samples
        VK_FALSE,  // sparseResidency4Samples
        VK_FALSE,  // sparseResidency8Samples
        VK_FALSE,  // sparseResidency16Samples
        VK_FALSE,  // sparseResidencyAliased
        VK_FALSE,  // variableMultisampleRate
        VK_FALSE,  // inheritedQueries
    };
}

void GetPhysicalDeviceFeatures2KHR(VkPhysicalDevice physical_device, VkPhysicalDeviceFeatures2KHR* features) {
    GetPhysicalDeviceFeatures(physical_device, &features->features);
}

// -----------------------------------------------------------------------------
// Device

VkResult CreateDevice(VkPhysicalDevice physical_device,
                      const VkDeviceCreateInfo* create_info,
                      const VkAllocationCallbacks* allocator,
                      VkDevice* out_device) {
    VkInstance_T* instance = GetInstanceFromPhysicalDevice(physical_device);
    if (!allocator)
        allocator = &instance->allocator;
    VkDevice_T* device = static_cast<VkDevice_T*>(allocator->pfnAllocation(
        allocator->pUserData, sizeof(VkDevice_T), alignof(VkDevice_T),
        VK_SYSTEM_ALLOCATION_SCOPE_DEVICE));
    if (!device)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    device->dispatch.magic = HWVULKAN_DISPATCH_MAGIC;
    device->allocator = *allocator;
    device->instance = instance;
    device->queue.dispatch.magic = HWVULKAN_DISPATCH_MAGIC;
    std::fill(device->next_handle.begin(), device->next_handle.end(),
              UINT64_C(0));

    for (uint32_t i = 0; i < create_info->enabledExtensionCount; i++) {
        if (strcmp(create_info->ppEnabledExtensionNames[i],
                   VK_ANDROID_NATIVE_BUFFER_EXTENSION_NAME) == 0) {
            ALOGV("Enabling " VK_ANDROID_NATIVE_BUFFER_EXTENSION_NAME);
        }
    }

    *out_device = device;
    return VK_SUCCESS;
}

void DestroyDevice(VkDevice device,
                   const VkAllocationCallbacks* /*allocator*/) {
    if (!device)
        return;
    device->allocator.pfnFree(device->allocator.pUserData, device);
}

void GetDeviceQueue(VkDevice device, uint32_t, uint32_t, VkQueue* queue) {
    *queue = &device->queue;
}

// -----------------------------------------------------------------------------
// CommandPool

struct CommandPool {
    typedef VkCommandPool HandleType;
    VkAllocationCallbacks allocator;
};
DEFINE_OBJECT_HANDLE_CONVERSION(CommandPool)

VkResult CreateCommandPool(VkDevice device,
                           const VkCommandPoolCreateInfo* /*create_info*/,
                           const VkAllocationCallbacks* allocator,
                           VkCommandPool* cmd_pool) {
    if (!allocator)
        allocator = &device->allocator;
    CommandPool* pool = static_cast<CommandPool*>(allocator->pfnAllocation(
        allocator->pUserData, sizeof(CommandPool), alignof(CommandPool),
        VK_SYSTEM_ALLOCATION_SCOPE_OBJECT));
    if (!pool)
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    pool->allocator = *allocator;
    *cmd_pool = GetHandleToCommandPool(pool);
    return VK_SUCCESS;
}

void DestroyCommandPool(VkDevice /*device*/,
                        VkCommandPool cmd_pool,
                        const VkAllocationCallbacks* /*allocator*/) {
    CommandPool* pool = GetCommandPoolFromHandle(cmd_pool);
    pool->allocator.pfnFree(pool->allocator.pUserData, pool);
}

// -----------------------------------------------------------------------------
// CmdBuffer

VkResult AllocateCommandBuffers(VkDevice /*device*/,
                                const VkCommandBufferAllocateInfo* alloc_info,
                                VkCommandBuffer* cmdbufs) {
    VkResult result = VK_SUCCESS;
    CommandPool& pool = *GetCommandPoolFromHandle(alloc_info->commandPool);
    std::fill(cmdbufs, cmdbufs + alloc_info->commandBufferCount, nullptr);
    for (uint32_t i = 0; i < alloc_info->commandBufferCount; i++) {
        cmdbufs[i] =
            static_cast<VkCommandBuffer_T*>(pool.allocator.pfnAllocation(
                pool.allocator.pUserData, sizeof(VkCommandBuffer_T),
                alignof(VkCommandBuffer_T), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT));
        if (!cmdbufs[i]) {
            result = VK_ERROR_OUT_OF_HOST_MEMORY;
            break;
        }
        cmdbufs[i]->dispatch.magic = HWVULKAN_DISPATCH_MAGIC;
    }
    if (result != VK_SUCCESS) {
        for (uint32_t i = 0; i < alloc_info->commandBufferCount; i++) {
            if (!cmdbufs[i])
                break;
            pool.allocator.pfnFree(pool.allocator.pUserData, cmdbufs[i]);
        }
    }
    return result;
}

void FreeCommandBuffers(VkDevice /*device*/,
                        VkCommandPool cmd_pool,
                        uint32_t count,
                        const VkCommandBuffer* cmdbufs) {
    CommandPool& pool = *GetCommandPoolFromHandle(cmd_pool);
    for (uint32_t i = 0; i < count; i++)
        pool.allocator.pfnFree(pool.allocator.pUserData, cmdbufs[i]);
}

// -----------------------------------------------------------------------------
// DeviceMemory

struct DeviceMemory {
    typedef VkDeviceMemory HandleType;
    VkDeviceSize size;
    alignas(16) uint8_t data[0];
};
DEFINE_OBJECT_HANDLE_CONVERSION(DeviceMemory)

VkResult AllocateMemory(VkDevice device,
                        const VkMemoryAllocateInfo* alloc_info,
                        const VkAllocationCallbacks* allocator,
                        VkDeviceMemory* mem_handle) {
    if (SIZE_MAX - sizeof(DeviceMemory) <= alloc_info->allocationSize)
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    if (!allocator)
        allocator = &device->allocator;

    size_t size = sizeof(DeviceMemory) + size_t(alloc_info->allocationSize);
    DeviceMemory* mem = static_cast<DeviceMemory*>(allocator->pfnAllocation(
        allocator->pUserData, size, alignof(DeviceMemory),
        VK_SYSTEM_ALLOCATION_SCOPE_OBJECT));
    if (!mem)
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    mem->size = size;
    *mem_handle = GetHandleToDeviceMemory(mem);
    return VK_SUCCESS;
}

void FreeMemory(VkDevice device,
                VkDeviceMemory mem_handle,
                const VkAllocationCallbacks* allocator) {
    if (!allocator)
        allocator = &device->allocator;
    DeviceMemory* mem = GetDeviceMemoryFromHandle(mem_handle);
    allocator->pfnFree(allocator->pUserData, mem);
}

VkResult MapMemory(VkDevice,
                   VkDeviceMemory mem_handle,
                   VkDeviceSize offset,
                   VkDeviceSize,
                   VkMemoryMapFlags,
                   void** out_ptr) {
    DeviceMemory* mem = GetDeviceMemoryFromHandle(mem_handle);
    *out_ptr = &mem->data[0] + offset;
    return VK_SUCCESS;
}

// -----------------------------------------------------------------------------
// Buffer

struct Buffer {
    typedef VkBuffer HandleType;
    VkDeviceSize size;
};
DEFINE_OBJECT_HANDLE_CONVERSION(Buffer)

VkResult CreateBuffer(VkDevice device,
                      const VkBufferCreateInfo* create_info,
                      const VkAllocationCallbacks* allocator,
                      VkBuffer* buffer_handle) {
    ALOGW_IF(create_info->size > kMaxDeviceMemory,
             "CreateBuffer: requested size 0x%" PRIx64
             " exceeds max device memory size 0x%" PRIx64,
             create_info->size, kMaxDeviceMemory);
    if (!allocator)
        allocator = &device->allocator;
    Buffer* buffer = static_cast<Buffer*>(allocator->pfnAllocation(
        allocator->pUserData, sizeof(Buffer), alignof(Buffer),
        VK_SYSTEM_ALLOCATION_SCOPE_OBJECT));
    if (!buffer)
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    buffer->size = create_info->size;
    *buffer_handle = GetHandleToBuffer(buffer);
    return VK_SUCCESS;
}

void GetBufferMemoryRequirements(VkDevice,
                                 VkBuffer buffer_handle,
                                 VkMemoryRequirements* requirements) {
    Buffer* buffer = GetBufferFromHandle(buffer_handle);
    requirements->size = buffer->size;
    requirements->alignment = 16;  // allow fast Neon/SSE memcpy
    requirements->memoryTypeBits = 0x1;
}

void DestroyBuffer(VkDevice device,
                   VkBuffer buffer_handle,
                   const VkAllocationCallbacks* allocator) {
    if (!allocator)
        allocator = &device->allocator;
    Buffer* buffer = GetBufferFromHandle(buffer_handle);
    allocator->pfnFree(allocator->pUserData, buffer);
}

// -----------------------------------------------------------------------------
// Image

struct Image {
    typedef VkImage HandleType;
    VkDeviceSize size;
};
DEFINE_OBJECT_HANDLE_CONVERSION(Image)

VkResult CreateImage(VkDevice device,
                     const VkImageCreateInfo* create_info,
                     const VkAllocationCallbacks* allocator,
                     VkImage* image_handle) {
    if (create_info->imageType != VK_IMAGE_TYPE_2D ||
        create_info->format != VK_FORMAT_R8G8B8A8_UNORM ||
        create_info->mipLevels != 1) {
        ALOGE("CreateImage: not yet implemented: type=%d format=%d mips=%u",
              create_info->imageType, create_info->format,
              create_info->mipLevels);
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    VkDeviceSize size =
        VkDeviceSize(create_info->extent.width * create_info->extent.height) *
        create_info->arrayLayers * create_info->samples * 4u;
    ALOGW_IF(size > kMaxDeviceMemory,
             "CreateImage: image size 0x%" PRIx64
             " exceeds max device memory size 0x%" PRIx64,
             size, kMaxDeviceMemory);

    if (!allocator)
        allocator = &device->allocator;
    Image* image = static_cast<Image*>(allocator->pfnAllocation(
        allocator->pUserData, sizeof(Image), alignof(Image),
        VK_SYSTEM_ALLOCATION_SCOPE_OBJECT));
    if (!image)
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    image->size = size;
    *image_handle = GetHandleToImage(image);
    return VK_SUCCESS;
}

void GetImageMemoryRequirements(VkDevice,
                                VkImage image_handle,
                                VkMemoryRequirements* requirements) {
    Image* image = GetImageFromHandle(image_handle);
    requirements->size = image->size;
    requirements->alignment = 16;  // allow fast Neon/SSE memcpy
    requirements->memoryTypeBits = 0x1;
}

void DestroyImage(VkDevice device,
                  VkImage image_handle,
                  const VkAllocationCallbacks* allocator) {
    if (!allocator)
        allocator = &device->allocator;
    Image* image = GetImageFromHandle(image_handle);
    allocator->pfnFree(allocator->pUserData, image);
}

VkResult GetSwapchainGrallocUsageANDROID(VkDevice,
                                         VkFormat,
                                         VkImageUsageFlags,
                                         int* grallocUsage) {
    // The null driver never reads or writes the gralloc buffer
    *grallocUsage = 0;
    return VK_SUCCESS;
}

VkResult GetSwapchainGrallocUsage2ANDROID(VkDevice,
                                          VkFormat,
                                          VkImageUsageFlags,
                                          VkSwapchainImageUsageFlagsANDROID,
                                          uint64_t* grallocConsumerUsage,
                                          uint64_t* grallocProducerUsage) {
    // The null driver never reads or writes the gralloc buffer
    *grallocConsumerUsage = 0;
    *grallocProducerUsage = 0;
    return VK_SUCCESS;
}

VkResult AcquireImageANDROID(VkDevice,
                             VkImage,
                             int fence,
                             VkSemaphore,
                             VkFence) {
    close(fence);
    return VK_SUCCESS;
}

VkResult QueueSignalReleaseImageANDROID(VkQueue,
                                        uint32_t,
                                        const VkSemaphore*,
                                        VkImage,
                                        int* fence) {
    *fence = -1;
    return VK_SUCCESS;
}

// -----------------------------------------------------------------------------
// No-op types

VkResult CreateBufferView(VkDevice device,
                          const VkBufferViewCreateInfo*,
                          const VkAllocationCallbacks* /*allocator*/,
                          VkBufferView* view) {
    *view = AllocHandle<VkBufferView>(device, HandleType::kBufferView);
    return VK_SUCCESS;
}

VkResult CreateDescriptorPool(VkDevice device,
                              const VkDescriptorPoolCreateInfo*,
                              const VkAllocationCallbacks* /*allocator*/,
                              VkDescriptorPool* pool) {
    *pool = AllocHandle<VkDescriptorPool>(device, HandleType::kDescriptorPool);
    return VK_SUCCESS;
}

VkResult AllocateDescriptorSets(VkDevice device,
                                const VkDescriptorSetAllocateInfo* alloc_info,
                                VkDescriptorSet* descriptor_sets) {
    for (uint32_t i = 0; i < alloc_info->descriptorSetCount; i++)
        descriptor_sets[i] =
            AllocHandle<VkDescriptorSet>(device, HandleType::kDescriptorSet);
    return VK_SUCCESS;
}

VkResult CreateDescriptorSetLayout(VkDevice device,
                                   const VkDescriptorSetLayoutCreateInfo*,
                                   const VkAllocationCallbacks* /*allocator*/,
                                   VkDescriptorSetLayout* layout) {
    *layout = AllocHandle<VkDescriptorSetLayout>(
        device, HandleType::kDescriptorSetLayout);
    return VK_SUCCESS;
}

VkResult CreateEvent(VkDevice device,
                     const VkEventCreateInfo*,
                     const VkAllocationCallbacks* /*allocator*/,
                     VkEvent* event) {
    *event = AllocHandle<VkEvent>(device, HandleType::kEvent);
    return VK_SUCCESS;
}

VkResult CreateFence(VkDevice device,
                     const VkFenceCreateInfo*,
                     const VkAllocationCallbacks* /*allocator*/,
                     VkFence* fence) {
    *fence = AllocHandle<VkFence>(device, HandleType::kFence);
    return VK_SUCCESS;
}

VkResult CreateFramebuffer(VkDevice device,
                           const VkFramebufferCreateInfo*,
                           const VkAllocationCallbacks* /*allocator*/,
                           VkFramebuffer* framebuffer) {
    *framebuffer = AllocHandle<VkFramebuffer>(device, HandleType::kFramebuffer);
    return VK_SUCCESS;
}

VkResult CreateImageView(VkDevice device,
                         const VkImageViewCreateInfo*,
                         const VkAllocationCallbacks* /*allocator*/,
                         VkImageView* view) {
    *view = AllocHandle<VkImageView>(device, HandleType::kImageView);
    return VK_SUCCESS;
}

VkResult CreateGraphicsPipelines(VkDevice device,
                                 VkPipelineCache,
                                 uint32_t count,
                                 const VkGraphicsPipelineCreateInfo*,
                                 const VkAllocationCallbacks* /*allocator*/,
                                 VkPipeline* pipelines) {
    for (uint32_t i = 0; i < count; i++)
        pipelines[i] = AllocHandle<VkPipeline>(device, HandleType::kPipeline);
    return VK_SUCCESS;
}

VkResult CreateComputePipelines(VkDevice device,
                                VkPipelineCache,
                                uint32_t count,
                                const VkComputePipelineCreateInfo*,
                                const VkAllocationCallbacks* /*allocator*/,
                                VkPipeline* pipelines) {
    for (uint32_t i = 0; i < count; i++)
        pipelines[i] = AllocHandle<VkPipeline>(device, HandleType::kPipeline);
    return VK_SUCCESS;
}

VkResult CreatePipelineCache(VkDevice device,
                             const VkPipelineCacheCreateInfo*,
                             const VkAllocationCallbacks* /*allocator*/,
                             VkPipelineCache* cache) {
    *cache = AllocHandle<VkPipelineCache>(device, HandleType::kPipelineCache);
    return VK_SUCCESS;
}

VkResult CreatePipelineLayout(VkDevice device,
                              const VkPipelineLayoutCreateInfo*,
                              const VkAllocationCallbacks* /*allocator*/,
                              VkPipelineLayout* layout) {
    *layout =
        AllocHandle<VkPipelineLayout>(device, HandleType::kPipelineLayout);
    return VK_SUCCESS;
}

VkResult CreateQueryPool(VkDevice device,
                         const VkQueryPoolCreateInfo*,
                         const VkAllocationCallbacks* /*allocator*/,
                         VkQueryPool* pool) {
    *pool = AllocHandle<VkQueryPool>(device, HandleType::kQueryPool);
    return VK_SUCCESS;
}

VkResult CreateRenderPass(VkDevice device,
                          const VkRenderPassCreateInfo*,
                          const VkAllocationCallbacks* /*allocator*/,
                          VkRenderPass* renderpass) {
    *renderpass = AllocHandle<VkRenderPass>(device, HandleType::kRenderPass);
    return VK_SUCCESS;
}

VkResult CreateSampler(VkDevice device,
                       const VkSamplerCreateInfo*,
                       const VkAllocationCallbacks* /*allocator*/,
                       VkSampler* sampler) {
    *sampler = AllocHandle<VkSampler>(device, HandleType::kSampler);
    return VK_SUCCESS;
}

VkResult CreateSemaphore(VkDevice device,
                         const VkSemaphoreCreateInfo*,
                         const VkAllocationCallbacks* /*allocator*/,
                         VkSemaphore* semaphore) {
    *semaphore = AllocHandle<VkSemaphore>(device, HandleType::kSemaphore);
    return VK_SUCCESS;
}

VkResult CreateShaderModule(VkDevice device,
                            const VkShaderModuleCreateInfo*,
                            const VkAllocationCallbacks* /*allocator*/,
                            VkShaderModule* module) {
    *module = AllocHandle<VkShaderModule>(device, HandleType::kShaderModule);
    return VK_SUCCESS;
}

VkResult CreateDebugReportCallbackEXT(VkInstance instance,
                                      const VkDebugReportCallbackCreateInfoEXT*,
                                      const VkAllocationCallbacks*,
                                      VkDebugReportCallbackEXT* callback) {
    *callback = AllocHandle<VkDebugReportCallbackEXT>(
        instance, HandleType::kDebugReportCallbackEXT);
    return VK_SUCCESS;
}

// -----------------------------------------------------------------------------
// No-op entrypoints

// clang-format off
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

void GetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties) {
    ALOGV("TODO: vk%s", __FUNCTION__);
}

void GetPhysicalDeviceFormatProperties2KHR(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2KHR* pFormatProperties) {
    ALOGV("TODO: vk%s", __FUNCTION__);
}

VkResult GetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties* pImageFormatProperties) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

VkResult GetPhysicalDeviceImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                    const VkPhysicalDeviceImageFormatInfo2KHR* pImageFormatInfo,
                                                    VkImageFormatProperties2KHR* pImageFormatProperties) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

VkResult EnumerateInstanceLayerProperties(uint32_t* pCount, VkLayerProperties* pProperties) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

VkResult QueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmitInfo, VkFence fence) {
    return VK_SUCCESS;
}

VkResult QueueWaitIdle(VkQueue queue) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

VkResult DeviceWaitIdle(VkDevice device) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

void UnmapMemory(VkDevice device, VkDeviceMemory mem) {
}

VkResult FlushMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

VkResult InvalidateMappedMemoryRanges(VkDevice device, uint32_t memRangeCount, const VkMappedMemoryRange* pMemRanges) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

void GetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize* pCommittedMemoryInBytes) {
    ALOGV("TODO: vk%s", __FUNCTION__);
}

VkResult BindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory mem, VkDeviceSize memOffset) {
    return VK_SUCCESS;
}

VkResult BindImageMemory(VkDevice device, VkImage image, VkDeviceMemory mem, VkDeviceSize memOffset) {
    return VK_SUCCESS;
}

void GetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t* pNumRequirements, VkSparseImageMemoryRequirements* pSparseMemoryRequirements) {
    ALOGV("TODO: vk%s", __FUNCTION__);
}

void GetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t* pNumProperties, VkSparseImageFormatProperties* pProperties) {
    ALOGV("TODO: vk%s", __FUNCTION__);
}

void GetPhysicalDeviceSparseImageFormatProperties2KHR(VkPhysicalDevice physicalDevice,
                                                      VkPhysicalDeviceSparseImageFormatInfo2KHR const* pInfo,
                                                      unsigned int* pNumProperties,
                                                      VkSparseImageFormatProperties2KHR* pProperties) {
    ALOGV("TODO: vk%s", __FUNCTION__);
}


VkResult QueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo* pBindInfo, VkFence fence) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

void DestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* allocator) {
}

VkResult ResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) {
    return VK_SUCCESS;
}

VkResult GetFenceStatus(VkDevice device, VkFence fence) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

VkResult WaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) {
    return VK_SUCCESS;
}

void DestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* allocator) {
}

void DestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks* allocator) {
}

VkResult GetEventStatus(VkDevice device, VkEvent event) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

VkResult SetEvent(VkDevice device, VkEvent event) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

VkResult ResetEvent(VkDevice device, VkEvent event) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

void DestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks* allocator) {
}

VkResult GetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t startQuery, uint32_t queryCount, size_t dataSize, void* pData, VkDeviceSize stride, VkQueryResultFlags flags) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

void DestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks* allocator) {
}

void GetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource* pSubresource, VkSubresourceLayout* pLayout) {
    ALOGV("TODO: vk%s", __FUNCTION__);
}

void DestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* allocator) {
}

void DestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks* allocator) {
}

void DestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks* allocator) {
}

VkResult GetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t* pDataSize, void* pData) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

VkResult MergePipelineCaches(VkDevice device, VkPipelineCache destCache, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

void DestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks* allocator) {
}

void DestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks* allocator) {
}

void DestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks* allocator) {
}

void DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks* allocator) {
}

void DestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks* allocator) {
}

VkResult ResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

void UpdateDescriptorSets(VkDevice device, uint32_t writeCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t copyCount, const VkCopyDescriptorSet* pDescriptorCopies) {
    ALOGV("TODO: vk%s", __FUNCTION__);
}

VkResult FreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t count, const VkDescriptorSet* pDescriptorSets) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

void DestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks* allocator) {
}

void DestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks* allocator) {
}

void GetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D* pGranularity) {
    ALOGV("TODO: vk%s", __FUNCTION__);
}

VkResult ResetCommandPool(VkDevice device, VkCommandPool cmdPool, VkCommandPoolResetFlags flags) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

VkResult BeginCommandBuffer(VkCommandBuffer cmdBuffer, const VkCommandBufferBeginInfo* pBeginInfo) {
    return VK_SUCCESS;
}

VkResult EndCommandBuffer(VkCommandBuffer cmdBuffer) {
    return VK_SUCCESS;
}

VkResult ResetCommandBuffer(VkCommandBuffer cmdBuffer, VkCommandBufferResetFlags flags) {
    ALOGV("TODO: vk%s", __FUNCTION__);
    return VK_SUCCESS;
}

void CmdBindPipeline(VkCommandBuffer cmdBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) {
}

void CmdSetViewport(VkCommandBuffer cmdBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) {
}

void CmdSetScissor(VkCommandBuffer cmdBuffer, uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors) {
}

void CmdSetLineWidth(VkCommandBuffer cmdBuffer, float lineWidth) {
}

void CmdSetDepthBias(VkCommandBuffer cmdBuffer, float depthBias, float depthBiasClamp, float slopeScaledDepthBias) {
}

void CmdSetBlendConstants(VkCommandBuffer cmdBuffer, const float blendConst[4]) {
}

void CmdSetDepthBounds(VkCommandBuffer cmdBuffer, float minDepthBounds, float maxDepthBounds) {
}

void CmdSetStencilCompareMask(VkCommandBuffer cmdBuffer, VkStencilFaceFlags faceMask, uint32_t stencilCompareMask) {
}

void CmdSetStencilWriteMask(VkCommandBuffer cmdBuffer, VkStencilFaceFlags faceMask, uint32_t stencilWriteMask) {
}

void CmdSetStencilReference(VkCommandBuffer cmdBuffer, VkStencilFaceFlags faceMask, uint32_t stencilReference) {
}

void CmdBindDescriptorSets(VkCommandBuffer cmdBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t setCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
}

void CmdBindIndexBuffer(VkCommandBuffer cmdBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType) {
}

void CmdBindVertexBuffers(VkCommandBuffer cmdBuffer, uint32_t startBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets) {
}

void CmdDraw(VkCommandBuffer cmdBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
}

void CmdDrawIndexed(VkCommandBuffer cmdBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
}

void CmdDrawIndirect(VkCommandBuffer cmdBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count, uint32_t stride) {
}

void CmdDrawIndexedIndirect(VkCommandBuffer cmdBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t count, uint32_t stride) {
}

void CmdDispatch(VkCommandBuffer cmdBuffer, uint32_t x, uint32_t y, uint32_t z) {
}

void CmdDispatchIndirect(VkCommandBuffer cmdBuffer, VkBuffer buffer, VkDeviceSize offset) {
}

void CmdCopyBuffer(VkCommandBuffer cmdBuffer, VkBuffer srcBuffer, VkBuffer destBuffer, uint32_t regionCount, const VkBufferCopy* pRegions) {
}

void CmdCopyImage(VkCommandBuffer cmdBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkImageCopy* pRegions) {
}

void CmdBlitImage(VkCommandBuffer cmdBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter) {
}

void CmdCopyBufferToImage(VkCommandBuffer cmdBuffer, VkBuffer srcBuffer, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
}

void CmdCopyImageToBuffer(VkCommandBuffer cmdBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer destBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions) {
}

void CmdUpdateBuffer(VkCommandBuffer cmdBuffer, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize dataSize, const void* pData) {
}

void CmdFillBuffer(VkCommandBuffer cmdBuffer, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize fillSize, uint32_t data) {
}

void CmdClearColorImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
}

void CmdClearDepthStencilImage(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges) {
}

void CmdClearAttachments(VkCommandBuffer cmdBuffer, uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects) {
}

void CmdResolveImage(VkCommandBuffer cmdBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage destImage, VkImageLayout destImageLayout, uint32_t regionCount, const VkImageResolve* pRegions) {
}

void CmdSetEvent(VkCommandBuffer cmdBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
}

void CmdResetEvent(VkCommandBuffer cmdBuffer, VkEvent event, VkPipelineStageFlags stageMask) {
}

void CmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
}

void CmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers) {
}

void CmdBeginQuery(VkCommandBuffer cmdBuffer, VkQueryPool queryPool, uint32_t slot, VkQueryControlFlags flags) {
}

void CmdEndQuery(VkCommandBuffer cmdBuffer, VkQueryPool queryPool, uint32_t slot) {
}

void CmdResetQueryPool(VkCommandBuffer cmdBuffer, VkQueryPool queryPool, uint32_t startQuery, uint32_t queryCount) {
}

void CmdWriteTimestamp(VkCommandBuffer cmdBuffer, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t slot) {
}

void CmdCopyQueryPoolResults(VkCommandBuffer cmdBuffer, VkQueryPool queryPool, uint32_t startQuery, uint32_t queryCount, VkBuffer destBuffer, VkDeviceSize destOffset, VkDeviceSize destStride, VkQueryResultFlags flags) {
}

void CmdPushConstants(VkCommandBuffer cmdBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t start, uint32_t length, const void* values) {
}

void CmdBeginRenderPass(VkCommandBuffer cmdBuffer, const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents) {
}

void CmdNextSubpass(VkCommandBuffer cmdBuffer, VkSubpassContents contents) {
}

void CmdEndRenderPass(VkCommandBuffer cmdBuffer) {
}

void CmdExecuteCommands(VkCommandBuffer cmdBuffer, uint32_t cmdBuffersCount, const VkCommandBuffer* pCmdBuffers) {
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
}

void DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage) {
}

VkResult BindBufferMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindBufferMemoryInfo* pBindInfos) {
    return VK_SUCCESS;
}

VkResult BindImageMemory2(VkDevice device, uint32_t bindInfoCount, const VkBindImageMemoryInfo* pBindInfos) {
    return VK_SUCCESS;
}

void GetDeviceGroupPeerMemoryFeatures(VkDevice device, uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags* pPeerMemoryFeatures) {
}

void CmdSetDeviceMask(VkCommandBuffer commandBuffer, uint32_t deviceMask) {
}

void CmdDispatchBase(VkCommandBuffer commandBuffer, uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
}

VkResult EnumeratePhysicalDeviceGroups(VkInstance instance, uint32_t* pPhysicalDeviceGroupCount, VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties) {
    return VK_SUCCESS;
}

void GetImageMemoryRequirements2(VkDevice device, const VkImageMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
}

void GetBufferMemoryRequirements2(VkDevice device, const VkBufferMemoryRequirementsInfo2* pInfo, VkMemoryRequirements2* pMemoryRequirements) {
}

void GetImageSparseMemoryRequirements2(VkDevice device, const VkImageSparseMemoryRequirementsInfo2* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2* pSparseMemoryRequirements) {
}

void GetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
}

void GetPhysicalDeviceProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2* pProperties) {
}

void GetPhysicalDeviceFormatProperties2(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties2* pFormatProperties) {
}

VkResult GetPhysicalDeviceImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo, VkImageFormatProperties2* pImageFormatProperties) {
    return VK_SUCCESS;
}

void GetPhysicalDeviceQueueFamilyProperties2(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties2* pQueueFamilyProperties) {
}

void GetPhysicalDeviceMemoryProperties2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties2* pMemoryProperties) {
}

void GetPhysicalDeviceSparseImageFormatProperties2(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo, uint32_t* pPropertyCount, VkSparseImageFormatProperties2* pProperties) {
}

void TrimCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolTrimFlags flags) {
}

void GetDeviceQueue2(VkDevice device, const VkDeviceQueueInfo2* pQueueInfo, VkQueue* pQueue) {
}

VkResult CreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSamplerYcbcrConversion* pYcbcrConversion) {
    return VK_SUCCESS;
}

void DestroySamplerYcbcrConversion(VkDevice device, VkSamplerYcbcrConversion ycbcrConversion, const VkAllocationCallbacks* pAllocator) {
}

VkResult CreateDescriptorUpdateTemplate(VkDevice device, const VkDescriptorUpdateTemplateCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate) {
    return VK_SUCCESS;
}

void DestroyDescriptorUpdateTemplate(VkDevice device, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const VkAllocationCallbacks* pAllocator) {
}

void UpdateDescriptorSetWithTemplate(VkDevice device, VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, const void* pData) {
}

void GetPhysicalDeviceExternalBufferProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo, VkExternalBufferProperties* pExternalBufferProperties) {
}

void GetPhysicalDeviceExternalFenceProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo, VkExternalFenceProperties* pExternalFenceProperties) {
}

void GetPhysicalDeviceExternalSemaphoreProperties(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo, VkExternalSemaphoreProperties* pExternalSemaphoreProperties) {
}

void GetDescriptorSetLayoutSupport(VkDevice device, const VkDescriptorSetLayoutCreateInfo* pCreateInfo, VkDescriptorSetLayoutSupport* pSupport) {
}

#pragma clang diagnostic pop
// clang-format on

}  // namespace null_driver
