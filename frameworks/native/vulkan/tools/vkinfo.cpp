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

#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <sstream>
#include <vector>

#include <vulkan/vulkan.h>

namespace {

struct Options {
    bool layer_description;
    bool layer_extensions;
    bool unsupported_features;
    bool validate;
};

struct GpuInfo {
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceMemoryProperties memory;
    VkPhysicalDeviceFeatures features;
    std::vector<VkQueueFamilyProperties> queue_families;
    std::vector<VkExtensionProperties> extensions;
    std::vector<VkLayerProperties> layers;
    std::vector<std::vector<VkExtensionProperties>> layer_extensions;
};
struct VulkanInfo {
    std::vector<VkExtensionProperties> extensions;
    std::vector<VkLayerProperties> layers;
    std::vector<std::vector<VkExtensionProperties>> layer_extensions;
    std::vector<GpuInfo> gpus;
};

// ----------------------------------------------------------------------------

[[noreturn]] void die(const char* proc, VkResult result) {
    const char* result_str;
    switch (result) {
        // clang-format off
        case VK_SUCCESS: result_str = "VK_SUCCESS"; break;
        case VK_NOT_READY: result_str = "VK_NOT_READY"; break;
        case VK_TIMEOUT: result_str = "VK_TIMEOUT"; break;
        case VK_EVENT_SET: result_str = "VK_EVENT_SET"; break;
        case VK_EVENT_RESET: result_str = "VK_EVENT_RESET"; break;
        case VK_INCOMPLETE: result_str = "VK_INCOMPLETE"; break;
        case VK_ERROR_OUT_OF_HOST_MEMORY: result_str = "VK_ERROR_OUT_OF_HOST_MEMORY"; break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: result_str = "VK_ERROR_OUT_OF_DEVICE_MEMORY"; break;
        case VK_ERROR_INITIALIZATION_FAILED: result_str = "VK_ERROR_INITIALIZATION_FAILED"; break;
        case VK_ERROR_DEVICE_LOST: result_str = "VK_ERROR_DEVICE_LOST"; break;
        case VK_ERROR_MEMORY_MAP_FAILED: result_str = "VK_ERROR_MEMORY_MAP_FAILED"; break;
        case VK_ERROR_LAYER_NOT_PRESENT: result_str = "VK_ERROR_LAYER_NOT_PRESENT"; break;
        case VK_ERROR_EXTENSION_NOT_PRESENT: result_str = "VK_ERROR_EXTENSION_NOT_PRESENT"; break;
        case VK_ERROR_INCOMPATIBLE_DRIVER: result_str = "VK_ERROR_INCOMPATIBLE_DRIVER"; break;
        default: result_str = "<unknown VkResult>"; break;
            // clang-format on
    }
    fprintf(stderr, "%s failed: %s (%d)\n", proc, result_str, result);
    exit(1);
}

bool HasExtension(const std::vector<VkExtensionProperties>& extensions,
                  const char* name) {
    return std::find_if(extensions.cbegin(), extensions.cend(),
                        [=](const VkExtensionProperties& prop) {
                            return strcmp(prop.extensionName, name) == 0;
                        }) != extensions.end();
}

void EnumerateInstanceExtensions(
    const char* layer_name,
    std::vector<VkExtensionProperties>* extensions) {
    VkResult result;
    uint32_t count;
    result =
        vkEnumerateInstanceExtensionProperties(layer_name, &count, nullptr);
    if (result != VK_SUCCESS)
        die("vkEnumerateInstanceExtensionProperties (count)", result);
    do {
        extensions->resize(count);
        result = vkEnumerateInstanceExtensionProperties(layer_name, &count,
                                                        extensions->data());
    } while (result == VK_INCOMPLETE);
    if (result != VK_SUCCESS)
        die("vkEnumerateInstanceExtensionProperties (data)", result);
}

void EnumerateDeviceExtensions(VkPhysicalDevice gpu,
                               const char* layer_name,
                               std::vector<VkExtensionProperties>* extensions) {
    VkResult result;
    uint32_t count;
    result =
        vkEnumerateDeviceExtensionProperties(gpu, layer_name, &count, nullptr);
    if (result != VK_SUCCESS)
        die("vkEnumerateDeviceExtensionProperties (count)", result);
    do {
        extensions->resize(count);
        result = vkEnumerateDeviceExtensionProperties(gpu, layer_name, &count,
                                                      extensions->data());
    } while (result == VK_INCOMPLETE);
    if (result != VK_SUCCESS)
        die("vkEnumerateDeviceExtensionProperties (data)", result);
}

void GatherGpuInfo(VkPhysicalDevice gpu,
                   const Options &options,
                   GpuInfo& info) {
    VkResult result;
    uint32_t count;

    vkGetPhysicalDeviceProperties(gpu, &info.properties);
    vkGetPhysicalDeviceMemoryProperties(gpu, &info.memory);
    vkGetPhysicalDeviceFeatures(gpu, &info.features);

    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
    info.queue_families.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count,
                                             info.queue_families.data());

    result = vkEnumerateDeviceLayerProperties(gpu, &count, nullptr);
    if (result != VK_SUCCESS)
        die("vkEnumerateDeviceLayerProperties (count)", result);
    do {
        info.layers.resize(count);
        result =
            vkEnumerateDeviceLayerProperties(gpu, &count, info.layers.data());
    } while (result == VK_INCOMPLETE);
    if (result != VK_SUCCESS)
        die("vkEnumerateDeviceLayerProperties (data)", result);
    info.layer_extensions.resize(info.layers.size());

    EnumerateDeviceExtensions(gpu, nullptr, &info.extensions);
    for (size_t i = 0; i < info.layers.size(); i++) {
        EnumerateDeviceExtensions(gpu, info.layers[i].layerName,
                                  &info.layer_extensions[i]);
    }

    const std::array<const char*, 1> kDesiredExtensions = {
        {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
    };
    const char* extensions[kDesiredExtensions.size()];
    uint32_t num_extensions = 0;
    for (const auto& desired_ext : kDesiredExtensions) {
        bool available = HasExtension(info.extensions, desired_ext);
        if (options.validate) {
            for (size_t i = 0; !available && i < info.layer_extensions.size();
                 i++)
                available = HasExtension(info.layer_extensions[i], desired_ext);
        }
        if (available)
            extensions[num_extensions++] = desired_ext;
    }

    VkDevice device;
    float queue_priorities[] = {0.0};
    const VkDeviceQueueCreateInfo queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = 0,
        .queueCount = 1,
        .pQueuePriorities = queue_priorities
    };
    // clang-format off
    const char *kValidationLayers[] = {
        "VK_LAYER_GOOGLE_threading",
        "VK_LAYER_LUNARG_parameter_validation",
        "VK_LAYER_LUNARG_device_limits",
        "VK_LAYER_LUNARG_object_tracker",
        "VK_LAYER_LUNARG_image",
        "VK_LAYER_LUNARG_core_validation",
        "VK_LAYER_LUNARG_swapchain",
        "VK_LAYER_GOOGLE_unique_objects"
    };
    // clang-format on
    uint32_t num_layers = sizeof(kValidationLayers) / sizeof(char*);
    const VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_create_info,
        .enabledExtensionCount = num_extensions,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = (options.validate) ? num_layers : 0,
        .ppEnabledLayerNames = kValidationLayers,
        .pEnabledFeatures = &info.features,
    };
    result = vkCreateDevice(gpu, &create_info, nullptr, &device);
    if (result != VK_SUCCESS)
        die("vkCreateDevice", result);
    vkDestroyDevice(device, nullptr);
}

void GatherInfo(VulkanInfo* info, const Options& options) {
    VkResult result;
    uint32_t count;

    result = vkEnumerateInstanceLayerProperties(&count, nullptr);
    if (result != VK_SUCCESS)
        die("vkEnumerateInstanceLayerProperties (count)", result);
    do {
        info->layers.resize(count);
        result =
            vkEnumerateInstanceLayerProperties(&count, info->layers.data());
    } while (result == VK_INCOMPLETE);
    if (result != VK_SUCCESS)
        die("vkEnumerateInstanceLayerProperties (data)", result);
    info->layer_extensions.resize(info->layers.size());

    EnumerateInstanceExtensions(nullptr, &info->extensions);
    for (size_t i = 0; i < info->layers.size(); i++) {
        EnumerateInstanceExtensions(info->layers[i].layerName,
                                    &info->layer_extensions[i]);
    }

    const char* kDesiredExtensions[] = {
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
    };
    const char*
        extensions[sizeof(kDesiredExtensions) / sizeof(kDesiredExtensions[0])];
    uint32_t num_extensions = 0;
    for (const auto& desired_ext : kDesiredExtensions) {
        bool available = HasExtension(info->extensions, desired_ext);
        if (options.validate) {
            for (size_t i = 0; !available && i < info->layer_extensions.size();
                 i++)
                available =
                    HasExtension(info->layer_extensions[i], desired_ext);
        }
        if (available)
            extensions[num_extensions++] = desired_ext;
    }

    // clang-format off
    const char *kValidationLayers[] = {
        "VK_LAYER_GOOGLE_threading",
        "VK_LAYER_LUNARG_parameter_validation",
        "VK_LAYER_LUNARG_device_limits",
        "VK_LAYER_LUNARG_object_tracker",
        "VK_LAYER_LUNARG_image",
        "VK_LAYER_LUNARG_core_validation",
        "VK_LAYER_LUNARG_swapchain",
        "VK_LAYER_GOOGLE_unique_objects"
    };
    // clang-format on
    uint32_t num_layers = sizeof(kValidationLayers) / sizeof(char*);

    const VkApplicationInfo application_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "vkinfo",
        .applicationVersion = 0,
        .pEngineName = "vkinfo",
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_0,
    };
    const VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &application_info,
        .enabledExtensionCount = num_extensions,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = (options.validate) ? num_layers : 0,
        .ppEnabledLayerNames = kValidationLayers,
    };
    VkInstance instance;
    result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS)
        die("vkCreateInstance", result);

    uint32_t num_gpus;
    result = vkEnumeratePhysicalDevices(instance, &num_gpus, nullptr);
    if (result != VK_SUCCESS)
        die("vkEnumeratePhysicalDevices (count)", result);
    std::vector<VkPhysicalDevice> gpus(num_gpus, VK_NULL_HANDLE);
    do {
        gpus.resize(num_gpus, VK_NULL_HANDLE);
        result = vkEnumeratePhysicalDevices(instance, &num_gpus, gpus.data());
    } while (result == VK_INCOMPLETE);
    if (result != VK_SUCCESS)
        die("vkEnumeratePhysicalDevices (data)", result);

    info->gpus.resize(num_gpus);
    for (size_t i = 0; i < gpus.size(); i++)
        GatherGpuInfo(gpus[i], options, info->gpus.at(i));

    vkDestroyInstance(instance, nullptr);
}

// ----------------------------------------------------------------------------

const size_t kMaxIndent = 8;
const size_t kIndentSize = 3;
std::array<char, kMaxIndent * kIndentSize + 1> kIndent;
const char* Indent(size_t n) {
    static bool initialized = false;
    if (!initialized) {
        kIndent.fill(' ');
        kIndent.back() = '\0';
        initialized = true;
    }
    return kIndent.data() +
           (kIndent.size() - (kIndentSize * std::min(n, kMaxIndent) + 1));
}

const char* VkPhysicalDeviceTypeStr(VkPhysicalDeviceType type) {
    switch (type) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return "OTHER";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return "INTEGRATED_GPU";
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return "DISCRETE_GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return "VIRTUAL_GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return "CPU";
        default:
            return "<UNKNOWN>";
    }
}

void PrintExtensions(const std::vector<VkExtensionProperties>& extensions,
                     const Options& /*options*/,
                     size_t indent) {
    for (const auto& e : extensions)
        printf("%s%s (v%u)\n", Indent(indent), e.extensionName, e.specVersion);
}

void PrintLayers(
    const std::vector<VkLayerProperties>& layers,
    const std::vector<std::vector<VkExtensionProperties>> extensions,
    const Options& options,
    size_t indent) {
    for (size_t i = 0; i < layers.size(); i++) {
        printf("%s%s %u.%u.%u/%u\n", Indent(indent), layers[i].layerName,
               VK_VERSION_MAJOR(layers[i].specVersion),
               VK_VERSION_MINOR(layers[i].specVersion),
               VK_VERSION_PATCH(layers[i].specVersion),
               layers[i].implementationVersion);
        if (options.layer_description)
            printf("%s%s\n", Indent(indent + 1), layers[i].description);
        if (options.layer_extensions && !extensions[i].empty()) {
            if (!extensions[i].empty()) {
                printf("%sExtensions [%zu]:\n", Indent(indent + 1),
                       extensions[i].size());
                PrintExtensions(extensions[i], options, indent + 2);
            }
        }
    }
}

void PrintAllFeatures(const char* indent,
                      const VkPhysicalDeviceFeatures& features) {
    // clang-format off
    printf("%srobustBufferAccess: %s\n", indent, features.robustBufferAccess ? "YES" : "NO");
    printf("%sfullDrawIndexUint32: %s\n", indent, features.fullDrawIndexUint32 ? "YES" : "NO");
    printf("%simageCubeArray: %s\n", indent, features.imageCubeArray ? "YES" : "NO");
    printf("%sindependentBlend: %s\n", indent, features.independentBlend ? "YES" : "NO");
    printf("%sgeometryShader: %s\n", indent, features.geometryShader ? "YES" : "NO");
    printf("%stessellationShader: %s\n", indent, features.tessellationShader ? "YES" : "NO");
    printf("%ssampleRateShading: %s\n", indent, features.sampleRateShading ? "YES" : "NO");
    printf("%sdualSrcBlend: %s\n", indent, features.dualSrcBlend ? "YES" : "NO");
    printf("%slogicOp: %s\n", indent, features.logicOp ? "YES" : "NO");
    printf("%smultiDrawIndirect: %s\n", indent, features.multiDrawIndirect ? "YES" : "NO");
    printf("%sdrawIndirectFirstInstance: %s\n", indent, features.drawIndirectFirstInstance ? "YES" : "NO");
    printf("%sdepthClamp: %s\n", indent, features.depthClamp ? "YES" : "NO");
    printf("%sdepthBiasClamp: %s\n", indent, features.depthBiasClamp ? "YES" : "NO");
    printf("%sfillModeNonSolid: %s\n", indent, features.fillModeNonSolid ? "YES" : "NO");
    printf("%sdepthBounds: %s\n", indent, features.depthBounds ? "YES" : "NO");
    printf("%swideLines: %s\n", indent, features.wideLines ? "YES" : "NO");
    printf("%slargePoints: %s\n", indent, features.largePoints ? "YES" : "NO");
    printf("%salphaToOne: %s\n", indent, features.alphaToOne ? "YES" : "NO");
    printf("%smultiViewport: %s\n", indent, features.multiViewport ? "YES" : "NO");
    printf("%ssamplerAnisotropy: %s\n", indent, features.samplerAnisotropy ? "YES" : "NO");
    printf("%stextureCompressionETC2: %s\n", indent, features.textureCompressionETC2 ? "YES" : "NO");
    printf("%stextureCompressionASTC_LDR: %s\n", indent, features.textureCompressionASTC_LDR ? "YES" : "NO");
    printf("%stextureCompressionBC: %s\n", indent, features.textureCompressionBC ? "YES" : "NO");
    printf("%socclusionQueryPrecise: %s\n", indent, features.occlusionQueryPrecise ? "YES" : "NO");
    printf("%spipelineStatisticsQuery: %s\n", indent, features.pipelineStatisticsQuery ? "YES" : "NO");
    printf("%svertexPipelineStoresAndAtomics: %s\n", indent, features.vertexPipelineStoresAndAtomics ? "YES" : "NO");
    printf("%sfragmentStoresAndAtomics: %s\n", indent, features.fragmentStoresAndAtomics ? "YES" : "NO");
    printf("%sshaderTessellationAndGeometryPointSize: %s\n", indent, features.shaderTessellationAndGeometryPointSize ? "YES" : "NO");
    printf("%sshaderImageGatherExtended: %s\n", indent, features.shaderImageGatherExtended ? "YES" : "NO");
    printf("%sshaderStorageImageExtendedFormats: %s\n", indent, features.shaderStorageImageExtendedFormats ? "YES" : "NO");
    printf("%sshaderStorageImageMultisample: %s\n", indent, features.shaderStorageImageMultisample ? "YES" : "NO");
    printf("%sshaderStorageImageReadWithoutFormat: %s\n", indent, features.shaderStorageImageReadWithoutFormat ? "YES" : "NO");
    printf("%sshaderStorageImageWriteWithoutFormat: %s\n", indent, features.shaderStorageImageWriteWithoutFormat ? "YES" : "NO");
    printf("%sshaderUniformBufferArrayDynamicIndexing: %s\n", indent, features.shaderUniformBufferArrayDynamicIndexing ? "YES" : "NO");
    printf("%sshaderSampledImageArrayDynamicIndexing: %s\n", indent, features.shaderSampledImageArrayDynamicIndexing ? "YES" : "NO");
    printf("%sshaderStorageBufferArrayDynamicIndexing: %s\n", indent, features.shaderStorageBufferArrayDynamicIndexing ? "YES" : "NO");
    printf("%sshaderStorageImageArrayDynamicIndexing: %s\n", indent, features.shaderStorageImageArrayDynamicIndexing ? "YES" : "NO");
    printf("%sshaderClipDistance: %s\n", indent, features.shaderClipDistance ? "YES" : "NO");
    printf("%sshaderCullDistance: %s\n", indent, features.shaderCullDistance ? "YES" : "NO");
    printf("%sshaderFloat64: %s\n", indent, features.shaderFloat64 ? "YES" : "NO");
    printf("%sshaderInt64: %s\n", indent, features.shaderInt64 ? "YES" : "NO");
    printf("%sshaderInt16: %s\n", indent, features.shaderInt16 ? "YES" : "NO");
    printf("%sshaderResourceResidency: %s\n", indent, features.shaderResourceResidency ? "YES" : "NO");
    printf("%sshaderResourceMinLod: %s\n", indent, features.shaderResourceMinLod ? "YES" : "NO");
    printf("%ssparseBinding: %s\n", indent, features.sparseBinding ? "YES" : "NO");
    printf("%ssparseResidencyBuffer: %s\n", indent, features.sparseResidencyBuffer ? "YES" : "NO");
    printf("%ssparseResidencyImage2D: %s\n", indent, features.sparseResidencyImage2D ? "YES" : "NO");
    printf("%ssparseResidencyImage3D: %s\n", indent, features.sparseResidencyImage3D ? "YES" : "NO");
    printf("%ssparseResidency2Samples: %s\n", indent, features.sparseResidency2Samples ? "YES" : "NO");
    printf("%ssparseResidency4Samples: %s\n", indent, features.sparseResidency4Samples ? "YES" : "NO");
    printf("%ssparseResidency8Samples: %s\n", indent, features.sparseResidency8Samples ? "YES" : "NO");
    printf("%ssparseResidency16Samples: %s\n", indent, features.sparseResidency16Samples ? "YES" : "NO");
    printf("%ssparseResidencyAliased: %s\n", indent, features.sparseResidencyAliased ? "YES" : "NO");
    printf("%svariableMultisampleRate: %s\n", indent, features.variableMultisampleRate ? "YES" : "NO");
    printf("%sinheritedQueries: %s\n", indent, features.inheritedQueries ? "YES" : "NO");
    // clang-format on
}

void PrintSupportedFeatures(const char* indent,
                            const VkPhysicalDeviceFeatures& features) {
    // clang-format off
    if (features.robustBufferAccess) printf("%srobustBufferAccess\n", indent);
    if (features.fullDrawIndexUint32) printf("%sfullDrawIndexUint32\n", indent);
    if (features.imageCubeArray) printf("%simageCubeArray\n", indent);
    if (features.independentBlend) printf("%sindependentBlend\n", indent);
    if (features.geometryShader) printf("%sgeometryShader\n", indent);
    if (features.tessellationShader) printf("%stessellationShader\n", indent);
    if (features.sampleRateShading) printf("%ssampleRateShading\n", indent);
    if (features.dualSrcBlend) printf("%sdualSrcBlend\n", indent);
    if (features.logicOp) printf("%slogicOp\n", indent);
    if (features.multiDrawIndirect) printf("%smultiDrawIndirect\n", indent);
    if (features.drawIndirectFirstInstance) printf("%sdrawIndirectFirstInstance\n", indent);
    if (features.depthClamp) printf("%sdepthClamp\n", indent);
    if (features.depthBiasClamp) printf("%sdepthBiasClamp\n", indent);
    if (features.fillModeNonSolid) printf("%sfillModeNonSolid\n", indent);
    if (features.depthBounds) printf("%sdepthBounds\n", indent);
    if (features.wideLines) printf("%swideLines\n", indent);
    if (features.largePoints) printf("%slargePoints\n", indent);
    if (features.alphaToOne) printf("%salphaToOne\n", indent);
    if (features.multiViewport) printf("%smultiViewport\n", indent);
    if (features.samplerAnisotropy) printf("%ssamplerAnisotropy\n", indent);
    if (features.textureCompressionETC2) printf("%stextureCompressionETC2\n", indent);
    if (features.textureCompressionASTC_LDR) printf("%stextureCompressionASTC_LDR\n", indent);
    if (features.textureCompressionBC) printf("%stextureCompressionBC\n", indent);
    if (features.occlusionQueryPrecise) printf("%socclusionQueryPrecise\n", indent);
    if (features.pipelineStatisticsQuery) printf("%spipelineStatisticsQuery\n", indent);
    if (features.vertexPipelineStoresAndAtomics) printf("%svertexPipelineStoresAndAtomics\n", indent);
    if (features.fragmentStoresAndAtomics) printf("%sfragmentStoresAndAtomics\n", indent);
    if (features.shaderTessellationAndGeometryPointSize) printf("%sshaderTessellationAndGeometryPointSize\n", indent);
    if (features.shaderImageGatherExtended) printf("%sshaderImageGatherExtended\n", indent);
    if (features.shaderStorageImageExtendedFormats) printf("%sshaderStorageImageExtendedFormats\n", indent);
    if (features.shaderStorageImageMultisample) printf("%sshaderStorageImageMultisample\n", indent);
    if (features.shaderStorageImageReadWithoutFormat) printf("%sshaderStorageImageReadWithoutFormat\n", indent);
    if (features.shaderStorageImageWriteWithoutFormat) printf("%sshaderStorageImageWriteWithoutFormat\n", indent);
    if (features.shaderUniformBufferArrayDynamicIndexing) printf("%sshaderUniformBufferArrayDynamicIndexing\n", indent);
    if (features.shaderSampledImageArrayDynamicIndexing) printf("%sshaderSampledImageArrayDynamicIndexing\n", indent);
    if (features.shaderStorageBufferArrayDynamicIndexing) printf("%sshaderStorageBufferArrayDynamicIndexing\n", indent);
    if (features.shaderStorageImageArrayDynamicIndexing) printf("%sshaderStorageImageArrayDynamicIndexing\n", indent);
    if (features.shaderClipDistance) printf("%sshaderClipDistance\n", indent);
    if (features.shaderCullDistance) printf("%sshaderCullDistance\n", indent);
    if (features.shaderFloat64) printf("%sshaderFloat64\n", indent);
    if (features.shaderInt64) printf("%sshaderInt64\n", indent);
    if (features.shaderInt16) printf("%sshaderInt16\n", indent);
    if (features.shaderResourceResidency) printf("%sshaderResourceResidency\n", indent);
    if (features.shaderResourceMinLod) printf("%sshaderResourceMinLod\n", indent);
    if (features.sparseBinding) printf("%ssparseBinding\n", indent);
    if (features.sparseResidencyBuffer) printf("%ssparseResidencyBuffer\n", indent);
    if (features.sparseResidencyImage2D) printf("%ssparseResidencyImage2D\n", indent);
    if (features.sparseResidencyImage3D) printf("%ssparseResidencyImage3D\n", indent);
    if (features.sparseResidency2Samples) printf("%ssparseResidency2Samples\n", indent);
    if (features.sparseResidency4Samples) printf("%ssparseResidency4Samples\n", indent);
    if (features.sparseResidency8Samples) printf("%ssparseResidency8Samples\n", indent);
    if (features.sparseResidency16Samples) printf("%ssparseResidency16Samples\n", indent);
    if (features.sparseResidencyAliased) printf("%ssparseResidencyAliased\n", indent);
    if (features.variableMultisampleRate) printf("%svariableMultisampleRate\n", indent);
    if (features.inheritedQueries) printf("%sinheritedQueries\n", indent);
    // clang-format on
}

void PrintGpuInfo(const GpuInfo& info, const Options& options, size_t indent) {
    VkResult result;
    std::ostringstream strbuf;

    printf("%s\"%s\" (%s) %u.%u.%u/%#x [%04x:%04x]\n", Indent(indent),
           info.properties.deviceName,
           VkPhysicalDeviceTypeStr(info.properties.deviceType),
           VK_VERSION_MAJOR(info.properties.apiVersion),
           VK_VERSION_MINOR(info.properties.apiVersion),
           VK_VERSION_PATCH(info.properties.apiVersion),
           info.properties.driverVersion, info.properties.vendorID,
           info.properties.deviceID);

    for (uint32_t heap = 0; heap < info.memory.memoryHeapCount; heap++) {
        if ((info.memory.memoryHeaps[heap].flags &
             VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0)
            strbuf << "DEVICE_LOCAL";
        printf("%sHeap %u: %" PRIu64 " MiB (0x%" PRIx64 " B) %s\n",
               Indent(indent + 1), heap,
               info.memory.memoryHeaps[heap].size / 0x100000,
               info.memory.memoryHeaps[heap].size, strbuf.str().c_str());
        strbuf.str(std::string());

        for (uint32_t type = 0; type < info.memory.memoryTypeCount; type++) {
            if (info.memory.memoryTypes[type].heapIndex != heap)
                continue;
            VkMemoryPropertyFlags flags =
                info.memory.memoryTypes[type].propertyFlags;
            if ((flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
                strbuf << " DEVICE_LOCAL";
            if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
                strbuf << " HOST_VISIBLE";
            if ((flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0)
                strbuf << " COHERENT";
            if ((flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0)
                strbuf << " CACHED";
            if ((flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) != 0)
                strbuf << " LAZILY_ALLOCATED";
            printf("%sType %u:%s\n", Indent(indent + 2), type,
                   strbuf.str().c_str());
            strbuf.str(std::string());
        }
    }

    for (uint32_t family = 0; family < info.queue_families.size(); family++) {
        const VkQueueFamilyProperties& qprops = info.queue_families[family];
        VkQueueFlags flags = qprops.queueFlags;
        char flags_str[5];
        flags_str[0] = (flags & VK_QUEUE_GRAPHICS_BIT) ? 'G' : '_';
        flags_str[1] = (flags & VK_QUEUE_COMPUTE_BIT) ? 'C' : '_';
        flags_str[2] = (flags & VK_QUEUE_TRANSFER_BIT) ? 'T' : '_';
        flags_str[3] = (flags & VK_QUEUE_SPARSE_BINDING_BIT) ? 'S' : '_';
        flags_str[4] = '\0';
        printf(
            "%sQueue Family %u: %ux %s\n"
            "%stimestampValidBits: %ub\n"
            "%sminImageTransferGranularity: (%u,%u,%u)\n",
            Indent(indent + 1), family, qprops.queueCount, flags_str,
            Indent(indent + 2), qprops.timestampValidBits, Indent(indent + 2),
            qprops.minImageTransferGranularity.width,
            qprops.minImageTransferGranularity.height,
            qprops.minImageTransferGranularity.depth);
    }

    printf("%sFeatures:\n", Indent(indent + 1));
    if (options.unsupported_features) {
        PrintAllFeatures(Indent(indent + 2), info.features);
    } else {
        PrintSupportedFeatures(Indent(indent + 2), info.features);
    }

    printf("%sExtensions [%zu]:\n", Indent(indent + 1), info.extensions.size());
    if (!info.extensions.empty())
        PrintExtensions(info.extensions, options, indent + 2);
    printf("%sLayers [%zu]:\n", Indent(indent + 1), info.layers.size());
    if (!info.layers.empty())
        PrintLayers(info.layers, info.layer_extensions, options, indent + 2);
}

void PrintInfo(const VulkanInfo& info, const Options& options) {
    std::ostringstream strbuf;
    size_t indent = 0;

    printf("%sInstance Extensions [%zu]:\n", Indent(indent),
           info.extensions.size());
    PrintExtensions(info.extensions, options, indent + 1);
    printf("%sInstance Layers [%zu]:\n", Indent(indent), info.layers.size());
    if (!info.layers.empty())
        PrintLayers(info.layers, info.layer_extensions, options, indent + 1);

    printf("%sPhysicalDevices [%zu]:\n", Indent(indent), info.gpus.size());
    for (const auto& gpu : info.gpus)
        PrintGpuInfo(gpu, options, indent + 1);
}

const char kUsageString[] =
    "usage: vkinfo [options]\n"
    "  -v                       enable all the following verbose options\n"
    "    -layer_description     print layer description strings\n"
    "    -layer_extensions      print extensions supported by each layer\n"
    "    -unsupported_features  print all physical device features\n"
    "  -validate                enable validation layers if present\n"
    "  -debug_pause             pause at start until resumed via debugger\n";

}  // namespace

// ----------------------------------------------------------------------------

int main(int argc, char const* argv[]) {
    static volatile bool startup_pause = false;
    Options options = {
        .layer_description = false, .layer_extensions = false,
        .unsupported_features = false,
        .validate = false,
    };
    for (int argi = 1; argi < argc; argi++) {
        if (strcmp(argv[argi], "-h") == 0) {
            fputs(kUsageString, stdout);
            return 0;
        }
        if (strcmp(argv[argi], "-v") == 0) {
            options.layer_description = true;
            options.layer_extensions = true;
            options.unsupported_features = true;
        } else if (strcmp(argv[argi], "-layer_description") == 0) {
            options.layer_description = true;
        } else if (strcmp(argv[argi], "-layer_extensions") == 0) {
            options.layer_extensions = true;
        } else if (strcmp(argv[argi], "-unsupported_features") == 0) {
            options.unsupported_features = true;
        } else if (strcmp(argv[argi], "-validate") == 0) {
            options.validate = true;
        } else if (strcmp(argv[argi], "-debug_pause") == 0) {
            startup_pause = true;
        }
    }

    while (startup_pause) {
        sleep(0);
    }

    VulkanInfo info;
    GatherInfo(&info, options);
    PrintInfo(info, options);
    return 0;
}
