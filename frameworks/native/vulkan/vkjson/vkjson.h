///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015-2016 The Khronos Group Inc.
// Copyright (c) 2015-2016 Valve Corporation
// Copyright (c) 2015-2016 LunarG, Inc.
// Copyright (c) 2015-2016 Google, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
///////////////////////////////////////////////////////////////////////////////

#ifndef VKJSON_H_
#define VKJSON_H_

#include <vulkan/vulkan.h>
#include <string.h>

#include <map>
#include <string>
#include <vector>

#ifdef WIN32
#undef min
#undef max
#endif

#ifndef VK_API_VERSION_1_0
#define VK_API_VERSION_1_0 VK_MAKE_VERSION(1, 0, 0)
#endif

#ifndef VK_API_VERSION_1_1
#define VK_API_VERSION_1_1 VK_MAKE_VERSION(1, 1, 0)
#endif

/*
 * Annotation to tell clang that we intend to fall through from one case to
 * another in a switch. Sourced from android-base/macros.h.
 */
#define FALLTHROUGH_INTENDED [[clang::fallthrough]]

struct VkJsonLayer {
  VkLayerProperties properties;
  std::vector<VkExtensionProperties> extensions;
};

struct VkJsonExtDriverProperties {
  VkJsonExtDriverProperties() {
    reported = false;
    memset(&driver_properties_khr, 0,
           sizeof(VkPhysicalDeviceDriverPropertiesKHR));
  }
  bool reported;
  VkPhysicalDeviceDriverPropertiesKHR driver_properties_khr;
};

struct VkJsonExtVariablePointerFeatures {
  VkJsonExtVariablePointerFeatures() {
    reported = false;
    memset(&variable_pointer_features_khr, 0,
           sizeof(VkPhysicalDeviceVariablePointerFeaturesKHR));
  }
  bool reported;
  VkPhysicalDeviceVariablePointerFeaturesKHR variable_pointer_features_khr;
};

struct VkJsonDevice {
  VkJsonDevice() {
    memset(&properties, 0, sizeof(VkPhysicalDeviceProperties));
    memset(&features, 0, sizeof(VkPhysicalDeviceFeatures));
    memset(&memory, 0, sizeof(VkPhysicalDeviceMemoryProperties));
    memset(&subgroup_properties, 0, sizeof(VkPhysicalDeviceSubgroupProperties));
    memset(&point_clipping_properties, 0,
           sizeof(VkPhysicalDevicePointClippingProperties));
    memset(&multiview_properties, 0,
           sizeof(VkPhysicalDeviceMultiviewProperties));
    memset(&id_properties, 0, sizeof(VkPhysicalDeviceIDProperties));
    memset(&maintenance3_properties, 0,
           sizeof(VkPhysicalDeviceMaintenance3Properties));
    memset(&bit16_storage_features, 0,
           sizeof(VkPhysicalDevice16BitStorageFeatures));
    memset(&multiview_features, 0, sizeof(VkPhysicalDeviceMultiviewFeatures));
    memset(&variable_pointer_features, 0,
           sizeof(VkPhysicalDeviceVariablePointerFeatures));
    memset(&protected_memory_features, 0,
           sizeof(VkPhysicalDeviceProtectedMemoryFeatures));
    memset(&sampler_ycbcr_conversion_features, 0,
           sizeof(VkPhysicalDeviceSamplerYcbcrConversionFeatures));
    memset(&shader_draw_parameter_features, 0,
           sizeof(VkPhysicalDeviceShaderDrawParameterFeatures));
  }
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  VkJsonExtDriverProperties ext_driver_properties;
  VkJsonExtVariablePointerFeatures ext_variable_pointer_features;
  VkPhysicalDeviceMemoryProperties memory;
  std::vector<VkQueueFamilyProperties> queues;
  std::vector<VkExtensionProperties> extensions;
  std::vector<VkLayerProperties> layers;
  std::map<VkFormat, VkFormatProperties> formats;
  VkPhysicalDeviceSubgroupProperties subgroup_properties;
  VkPhysicalDevicePointClippingProperties point_clipping_properties;
  VkPhysicalDeviceMultiviewProperties multiview_properties;
  VkPhysicalDeviceIDProperties id_properties;
  VkPhysicalDeviceMaintenance3Properties maintenance3_properties;
  VkPhysicalDevice16BitStorageFeatures bit16_storage_features;
  VkPhysicalDeviceMultiviewFeatures multiview_features;
  VkPhysicalDeviceVariablePointerFeatures variable_pointer_features;
  VkPhysicalDeviceProtectedMemoryFeatures protected_memory_features;
  VkPhysicalDeviceSamplerYcbcrConversionFeatures
      sampler_ycbcr_conversion_features;
  VkPhysicalDeviceShaderDrawParameterFeatures shader_draw_parameter_features;
  std::map<VkExternalFenceHandleTypeFlagBits, VkExternalFenceProperties>
      external_fence_properties;
  std::map<VkExternalSemaphoreHandleTypeFlagBits, VkExternalSemaphoreProperties>
      external_semaphore_properties;
};

struct VkJsonDeviceGroup {
  VkJsonDeviceGroup() {
    memset(&properties, 0, sizeof(VkPhysicalDeviceGroupProperties));
  }
  VkPhysicalDeviceGroupProperties properties;
  std::vector<uint32_t> device_inds;
};

struct VkJsonInstance {
  VkJsonInstance() : api_version(0) {}
  uint32_t api_version;
  std::vector<VkJsonLayer> layers;
  std::vector<VkExtensionProperties> extensions;
  std::vector<VkJsonDevice> devices;
  std::vector<VkJsonDeviceGroup> device_groups;
};

VkJsonInstance VkJsonGetInstance();
std::string VkJsonInstanceToJson(const VkJsonInstance& instance);
bool VkJsonInstanceFromJson(const std::string& json,
                            VkJsonInstance* instance,
                            std::string* errors);

VkJsonDevice VkJsonGetDevice(VkInstance instance,
                             VkPhysicalDevice device,
                             uint32_t instanceExtensionCount,
                             const char* const* instanceExtensions);
std::string VkJsonDeviceToJson(const VkJsonDevice& device);
bool VkJsonDeviceFromJson(const std::string& json,
                          VkJsonDevice* device,
                          std::string* errors);

std::string VkJsonImageFormatPropertiesToJson(
    const VkImageFormatProperties& properties);
bool VkJsonImageFormatPropertiesFromJson(const std::string& json,
                                         VkImageFormatProperties* properties,
                                         std::string* errors);

// Backward-compatibility aliases
typedef VkJsonDevice VkJsonAllProperties;
inline VkJsonAllProperties VkJsonGetAllProperties(
    VkPhysicalDevice physicalDevice) {
  return VkJsonGetDevice(VK_NULL_HANDLE, physicalDevice, 0, nullptr);
}
inline std::string VkJsonAllPropertiesToJson(
    const VkJsonAllProperties& properties) {
  return VkJsonDeviceToJson(properties);
}
inline bool VkJsonAllPropertiesFromJson(const std::string& json,
                                        VkJsonAllProperties* properties,
                                        std::string* errors) {
  return VkJsonDeviceFromJson(json, properties, errors);
}

#endif  // VKJSON_H_
