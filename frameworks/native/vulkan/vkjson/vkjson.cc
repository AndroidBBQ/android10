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

#include "vkjson.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <cmath>
#include <cinttypes>
#include <cstdio>
#include <limits>
#include <memory>
#include <sstream>
#include <type_traits>
#include <utility>

#include <json/json.h>

namespace {

inline bool IsIntegral(double value) {
#if defined(ANDROID)
  // Android NDK doesn't provide std::trunc yet
  return trunc(value) == value;
#else
  return std::trunc(value) == value;
#endif
}

template <typename T> struct EnumTraits;
template <> struct EnumTraits<VkPhysicalDeviceType> {
  static uint32_t min() { return VK_PHYSICAL_DEVICE_TYPE_BEGIN_RANGE; }
  static uint32_t max() { return VK_PHYSICAL_DEVICE_TYPE_END_RANGE; }
  static bool exist(uint32_t e) { return e >= min() && e <= max(); }
};

template <> struct EnumTraits<VkFormat> {
  static bool exist(uint32_t e) {
    switch (e) {
      case VK_FORMAT_UNDEFINED:
      case VK_FORMAT_R4G4_UNORM_PACK8:
      case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
      case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
      case VK_FORMAT_R5G6B5_UNORM_PACK16:
      case VK_FORMAT_B5G6R5_UNORM_PACK16:
      case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
      case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
      case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
      case VK_FORMAT_R8_UNORM:
      case VK_FORMAT_R8_SNORM:
      case VK_FORMAT_R8_USCALED:
      case VK_FORMAT_R8_SSCALED:
      case VK_FORMAT_R8_UINT:
      case VK_FORMAT_R8_SINT:
      case VK_FORMAT_R8_SRGB:
      case VK_FORMAT_R8G8_UNORM:
      case VK_FORMAT_R8G8_SNORM:
      case VK_FORMAT_R8G8_USCALED:
      case VK_FORMAT_R8G8_SSCALED:
      case VK_FORMAT_R8G8_UINT:
      case VK_FORMAT_R8G8_SINT:
      case VK_FORMAT_R8G8_SRGB:
      case VK_FORMAT_R8G8B8_UNORM:
      case VK_FORMAT_R8G8B8_SNORM:
      case VK_FORMAT_R8G8B8_USCALED:
      case VK_FORMAT_R8G8B8_SSCALED:
      case VK_FORMAT_R8G8B8_UINT:
      case VK_FORMAT_R8G8B8_SINT:
      case VK_FORMAT_R8G8B8_SRGB:
      case VK_FORMAT_B8G8R8_UNORM:
      case VK_FORMAT_B8G8R8_SNORM:
      case VK_FORMAT_B8G8R8_USCALED:
      case VK_FORMAT_B8G8R8_SSCALED:
      case VK_FORMAT_B8G8R8_UINT:
      case VK_FORMAT_B8G8R8_SINT:
      case VK_FORMAT_B8G8R8_SRGB:
      case VK_FORMAT_R8G8B8A8_UNORM:
      case VK_FORMAT_R8G8B8A8_SNORM:
      case VK_FORMAT_R8G8B8A8_USCALED:
      case VK_FORMAT_R8G8B8A8_SSCALED:
      case VK_FORMAT_R8G8B8A8_UINT:
      case VK_FORMAT_R8G8B8A8_SINT:
      case VK_FORMAT_R8G8B8A8_SRGB:
      case VK_FORMAT_B8G8R8A8_UNORM:
      case VK_FORMAT_B8G8R8A8_SNORM:
      case VK_FORMAT_B8G8R8A8_USCALED:
      case VK_FORMAT_B8G8R8A8_SSCALED:
      case VK_FORMAT_B8G8R8A8_UINT:
      case VK_FORMAT_B8G8R8A8_SINT:
      case VK_FORMAT_B8G8R8A8_SRGB:
      case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
      case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
      case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
      case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
      case VK_FORMAT_A8B8G8R8_UINT_PACK32:
      case VK_FORMAT_A8B8G8R8_SINT_PACK32:
      case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
      case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
      case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
      case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
      case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
      case VK_FORMAT_A2R10G10B10_UINT_PACK32:
      case VK_FORMAT_A2R10G10B10_SINT_PACK32:
      case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
      case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
      case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
      case VK_FORMAT_A2B10G10R10_UINT_PACK32:
      case VK_FORMAT_A2B10G10R10_SINT_PACK32:
      case VK_FORMAT_R16_UNORM:
      case VK_FORMAT_R16_SNORM:
      case VK_FORMAT_R16_USCALED:
      case VK_FORMAT_R16_SSCALED:
      case VK_FORMAT_R16_UINT:
      case VK_FORMAT_R16_SINT:
      case VK_FORMAT_R16_SFLOAT:
      case VK_FORMAT_R16G16_UNORM:
      case VK_FORMAT_R16G16_SNORM:
      case VK_FORMAT_R16G16_USCALED:
      case VK_FORMAT_R16G16_SSCALED:
      case VK_FORMAT_R16G16_UINT:
      case VK_FORMAT_R16G16_SINT:
      case VK_FORMAT_R16G16_SFLOAT:
      case VK_FORMAT_R16G16B16_UNORM:
      case VK_FORMAT_R16G16B16_SNORM:
      case VK_FORMAT_R16G16B16_USCALED:
      case VK_FORMAT_R16G16B16_SSCALED:
      case VK_FORMAT_R16G16B16_UINT:
      case VK_FORMAT_R16G16B16_SINT:
      case VK_FORMAT_R16G16B16_SFLOAT:
      case VK_FORMAT_R16G16B16A16_UNORM:
      case VK_FORMAT_R16G16B16A16_SNORM:
      case VK_FORMAT_R16G16B16A16_USCALED:
      case VK_FORMAT_R16G16B16A16_SSCALED:
      case VK_FORMAT_R16G16B16A16_UINT:
      case VK_FORMAT_R16G16B16A16_SINT:
      case VK_FORMAT_R16G16B16A16_SFLOAT:
      case VK_FORMAT_R32_UINT:
      case VK_FORMAT_R32_SINT:
      case VK_FORMAT_R32_SFLOAT:
      case VK_FORMAT_R32G32_UINT:
      case VK_FORMAT_R32G32_SINT:
      case VK_FORMAT_R32G32_SFLOAT:
      case VK_FORMAT_R32G32B32_UINT:
      case VK_FORMAT_R32G32B32_SINT:
      case VK_FORMAT_R32G32B32_SFLOAT:
      case VK_FORMAT_R32G32B32A32_UINT:
      case VK_FORMAT_R32G32B32A32_SINT:
      case VK_FORMAT_R32G32B32A32_SFLOAT:
      case VK_FORMAT_R64_UINT:
      case VK_FORMAT_R64_SINT:
      case VK_FORMAT_R64_SFLOAT:
      case VK_FORMAT_R64G64_UINT:
      case VK_FORMAT_R64G64_SINT:
      case VK_FORMAT_R64G64_SFLOAT:
      case VK_FORMAT_R64G64B64_UINT:
      case VK_FORMAT_R64G64B64_SINT:
      case VK_FORMAT_R64G64B64_SFLOAT:
      case VK_FORMAT_R64G64B64A64_UINT:
      case VK_FORMAT_R64G64B64A64_SINT:
      case VK_FORMAT_R64G64B64A64_SFLOAT:
      case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
      case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
      case VK_FORMAT_D16_UNORM:
      case VK_FORMAT_X8_D24_UNORM_PACK32:
      case VK_FORMAT_D32_SFLOAT:
      case VK_FORMAT_S8_UINT:
      case VK_FORMAT_D16_UNORM_S8_UINT:
      case VK_FORMAT_D24_UNORM_S8_UINT:
      case VK_FORMAT_D32_SFLOAT_S8_UINT:
      case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
      case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
      case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
      case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
      case VK_FORMAT_BC2_UNORM_BLOCK:
      case VK_FORMAT_BC2_SRGB_BLOCK:
      case VK_FORMAT_BC3_UNORM_BLOCK:
      case VK_FORMAT_BC3_SRGB_BLOCK:
      case VK_FORMAT_BC4_UNORM_BLOCK:
      case VK_FORMAT_BC4_SNORM_BLOCK:
      case VK_FORMAT_BC5_UNORM_BLOCK:
      case VK_FORMAT_BC5_SNORM_BLOCK:
      case VK_FORMAT_BC6H_UFLOAT_BLOCK:
      case VK_FORMAT_BC6H_SFLOAT_BLOCK:
      case VK_FORMAT_BC7_UNORM_BLOCK:
      case VK_FORMAT_BC7_SRGB_BLOCK:
      case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
      case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
      case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
      case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
      case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
      case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
      case VK_FORMAT_EAC_R11_UNORM_BLOCK:
      case VK_FORMAT_EAC_R11_SNORM_BLOCK:
      case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
      case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
      case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
      case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
      case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
      case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
      case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
      case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
      case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
      case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
      case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
      case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
      case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
      case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
      case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
      case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
      case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
      case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
      case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
      case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
      case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
      case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
      case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
      case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
      case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
      case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
      case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
      case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
      case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
      case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
      case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
      case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
      case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
      case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
      case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
      case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
      case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
      case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
      case VK_FORMAT_G8B8G8R8_422_UNORM_KHR:
      case VK_FORMAT_B8G8R8G8_422_UNORM_KHR:
      case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR:
      case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR:
      case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR:
      case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR:
      case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR:
      case VK_FORMAT_R10X6_UNORM_PACK16_KHR:
      case VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR:
      case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR:
      case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR:
      case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR:
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR:
      case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR:
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR:
      case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR:
      case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR:
      case VK_FORMAT_R12X4_UNORM_PACK16_KHR:
      case VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR:
      case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR:
      case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR:
      case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR:
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR:
      case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR:
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR:
      case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR:
      case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR:
      case VK_FORMAT_G16B16G16R16_422_UNORM_KHR:
      case VK_FORMAT_B16G16R16G16_422_UNORM_KHR:
      case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR:
      case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR:
      case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR:
      case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR:
      case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR:
        return true;
    }
    return false;
  }
};

template <>
struct EnumTraits<VkPointClippingBehavior> {
  static uint32_t min() { return VK_POINT_CLIPPING_BEHAVIOR_BEGIN_RANGE; }
  static uint32_t max() { return VK_POINT_CLIPPING_BEHAVIOR_END_RANGE; }
  static bool exist(uint32_t e) { return e >= min() && e <= max(); }
};

template <>
struct EnumTraits<VkExternalFenceHandleTypeFlagBits> {
  static bool exist(uint32_t e) {
    switch (e) {
      case VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT:
      case VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT:
      case VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT:
      case VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT:
        return true;
    }
    return false;
  }
};

template <>
struct EnumTraits<VkExternalSemaphoreHandleTypeFlagBits> {
  static bool exist(uint32_t e) {
    switch (e) {
      case VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT:
      case VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT:
      case VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT:
      case VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT:
      case VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT:
        return true;
    }
    return false;
  }
};

template <>
struct EnumTraits<VkDriverIdKHR> {
  static uint32_t min() { return VK_DRIVER_ID_BEGIN_RANGE_KHR; }
  static uint32_t max() { return VK_DRIVER_ID_END_RANGE_KHR; }
  static bool exist(uint32_t e) { return e >= min() && e <= max(); }
};

// VkSparseImageFormatProperties

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkExtent3D* extents) {
  return
    visitor->Visit("width", &extents->width) &&
    visitor->Visit("height", &extents->height) &&
    visitor->Visit("depth", &extents->depth);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkImageFormatProperties* properties) {
  return
    visitor->Visit("maxExtent", &properties->maxExtent) &&
    visitor->Visit("maxMipLevels", &properties->maxMipLevels) &&
    visitor->Visit("maxArrayLayers", &properties->maxArrayLayers) &&
    visitor->Visit("sampleCounts", &properties->sampleCounts) &&
    visitor->Visit("maxResourceSize", &properties->maxResourceSize);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkPhysicalDeviceLimits* limits) {
  return
    visitor->Visit("maxImageDimension1D", &limits->maxImageDimension1D) &&
    visitor->Visit("maxImageDimension2D", &limits->maxImageDimension2D) &&
    visitor->Visit("maxImageDimension3D", &limits->maxImageDimension3D) &&
    visitor->Visit("maxImageDimensionCube", &limits->maxImageDimensionCube) &&
    visitor->Visit("maxImageArrayLayers", &limits->maxImageArrayLayers) &&
    visitor->Visit("maxTexelBufferElements", &limits->maxTexelBufferElements) &&
    visitor->Visit("maxUniformBufferRange", &limits->maxUniformBufferRange) &&
    visitor->Visit("maxStorageBufferRange", &limits->maxStorageBufferRange) &&
    visitor->Visit("maxPushConstantsSize", &limits->maxPushConstantsSize) &&
    visitor->Visit("maxMemoryAllocationCount", &limits->maxMemoryAllocationCount) &&
    visitor->Visit("maxSamplerAllocationCount", &limits->maxSamplerAllocationCount) &&
    visitor->Visit("bufferImageGranularity", &limits->bufferImageGranularity) &&
    visitor->Visit("sparseAddressSpaceSize", &limits->sparseAddressSpaceSize) &&
    visitor->Visit("maxBoundDescriptorSets", &limits->maxBoundDescriptorSets) &&
    visitor->Visit("maxPerStageDescriptorSamplers", &limits->maxPerStageDescriptorSamplers) &&
    visitor->Visit("maxPerStageDescriptorUniformBuffers", &limits->maxPerStageDescriptorUniformBuffers) &&
    visitor->Visit("maxPerStageDescriptorStorageBuffers", &limits->maxPerStageDescriptorStorageBuffers) &&
    visitor->Visit("maxPerStageDescriptorSampledImages", &limits->maxPerStageDescriptorSampledImages) &&
    visitor->Visit("maxPerStageDescriptorStorageImages", &limits->maxPerStageDescriptorStorageImages) &&
    visitor->Visit("maxPerStageDescriptorInputAttachments", &limits->maxPerStageDescriptorInputAttachments) &&
    visitor->Visit("maxPerStageResources", &limits->maxPerStageResources) &&
    visitor->Visit("maxDescriptorSetSamplers", &limits->maxDescriptorSetSamplers) &&
    visitor->Visit("maxDescriptorSetUniformBuffers", &limits->maxDescriptorSetUniformBuffers) &&
    visitor->Visit("maxDescriptorSetUniformBuffersDynamic", &limits->maxDescriptorSetUniformBuffersDynamic) &&
    visitor->Visit("maxDescriptorSetStorageBuffers", &limits->maxDescriptorSetStorageBuffers) &&
    visitor->Visit("maxDescriptorSetStorageBuffersDynamic", &limits->maxDescriptorSetStorageBuffersDynamic) &&
    visitor->Visit("maxDescriptorSetSampledImages", &limits->maxDescriptorSetSampledImages) &&
    visitor->Visit("maxDescriptorSetStorageImages", &limits->maxDescriptorSetStorageImages) &&
    visitor->Visit("maxDescriptorSetInputAttachments", &limits->maxDescriptorSetInputAttachments) &&
    visitor->Visit("maxVertexInputAttributes", &limits->maxVertexInputAttributes) &&
    visitor->Visit("maxVertexInputBindings", &limits->maxVertexInputBindings) &&
    visitor->Visit("maxVertexInputAttributeOffset", &limits->maxVertexInputAttributeOffset) &&
    visitor->Visit("maxVertexInputBindingStride", &limits->maxVertexInputBindingStride) &&
    visitor->Visit("maxVertexOutputComponents", &limits->maxVertexOutputComponents) &&
    visitor->Visit("maxTessellationGenerationLevel", &limits->maxTessellationGenerationLevel) &&
    visitor->Visit("maxTessellationPatchSize", &limits->maxTessellationPatchSize) &&
    visitor->Visit("maxTessellationControlPerVertexInputComponents", &limits->maxTessellationControlPerVertexInputComponents) &&
    visitor->Visit("maxTessellationControlPerVertexOutputComponents", &limits->maxTessellationControlPerVertexOutputComponents) &&
    visitor->Visit("maxTessellationControlPerPatchOutputComponents", &limits->maxTessellationControlPerPatchOutputComponents) &&
    visitor->Visit("maxTessellationControlTotalOutputComponents", &limits->maxTessellationControlTotalOutputComponents) &&
    visitor->Visit("maxTessellationEvaluationInputComponents", &limits->maxTessellationEvaluationInputComponents) &&
    visitor->Visit("maxTessellationEvaluationOutputComponents", &limits->maxTessellationEvaluationOutputComponents) &&
    visitor->Visit("maxGeometryShaderInvocations", &limits->maxGeometryShaderInvocations) &&
    visitor->Visit("maxGeometryInputComponents", &limits->maxGeometryInputComponents) &&
    visitor->Visit("maxGeometryOutputComponents", &limits->maxGeometryOutputComponents) &&
    visitor->Visit("maxGeometryOutputVertices", &limits->maxGeometryOutputVertices) &&
    visitor->Visit("maxGeometryTotalOutputComponents", &limits->maxGeometryTotalOutputComponents) &&
    visitor->Visit("maxFragmentInputComponents", &limits->maxFragmentInputComponents) &&
    visitor->Visit("maxFragmentOutputAttachments", &limits->maxFragmentOutputAttachments) &&
    visitor->Visit("maxFragmentDualSrcAttachments", &limits->maxFragmentDualSrcAttachments) &&
    visitor->Visit("maxFragmentCombinedOutputResources", &limits->maxFragmentCombinedOutputResources) &&
    visitor->Visit("maxComputeSharedMemorySize", &limits->maxComputeSharedMemorySize) &&
    visitor->Visit("maxComputeWorkGroupCount", &limits->maxComputeWorkGroupCount) &&
    visitor->Visit("maxComputeWorkGroupInvocations", &limits->maxComputeWorkGroupInvocations) &&
    visitor->Visit("maxComputeWorkGroupSize", &limits->maxComputeWorkGroupSize) &&
    visitor->Visit("subPixelPrecisionBits", &limits->subPixelPrecisionBits) &&
    visitor->Visit("subTexelPrecisionBits", &limits->subTexelPrecisionBits) &&
    visitor->Visit("mipmapPrecisionBits", &limits->mipmapPrecisionBits) &&
    visitor->Visit("maxDrawIndexedIndexValue", &limits->maxDrawIndexedIndexValue) &&
    visitor->Visit("maxDrawIndirectCount", &limits->maxDrawIndirectCount) &&
    visitor->Visit("maxSamplerLodBias", &limits->maxSamplerLodBias) &&
    visitor->Visit("maxSamplerAnisotropy", &limits->maxSamplerAnisotropy) &&
    visitor->Visit("maxViewports", &limits->maxViewports) &&
    visitor->Visit("maxViewportDimensions", &limits->maxViewportDimensions) &&
    visitor->Visit("viewportBoundsRange", &limits->viewportBoundsRange) &&
    visitor->Visit("viewportSubPixelBits", &limits->viewportSubPixelBits) &&
    visitor->Visit("minMemoryMapAlignment", &limits->minMemoryMapAlignment) &&
    visitor->Visit("minTexelBufferOffsetAlignment", &limits->minTexelBufferOffsetAlignment) &&
    visitor->Visit("minUniformBufferOffsetAlignment", &limits->minUniformBufferOffsetAlignment) &&
    visitor->Visit("minStorageBufferOffsetAlignment", &limits->minStorageBufferOffsetAlignment) &&
    visitor->Visit("minTexelOffset", &limits->minTexelOffset) &&
    visitor->Visit("maxTexelOffset", &limits->maxTexelOffset) &&
    visitor->Visit("minTexelGatherOffset", &limits->minTexelGatherOffset) &&
    visitor->Visit("maxTexelGatherOffset", &limits->maxTexelGatherOffset) &&
    visitor->Visit("minInterpolationOffset", &limits->minInterpolationOffset) &&
    visitor->Visit("maxInterpolationOffset", &limits->maxInterpolationOffset) &&
    visitor->Visit("subPixelInterpolationOffsetBits", &limits->subPixelInterpolationOffsetBits) &&
    visitor->Visit("maxFramebufferWidth", &limits->maxFramebufferWidth) &&
    visitor->Visit("maxFramebufferHeight", &limits->maxFramebufferHeight) &&
    visitor->Visit("maxFramebufferLayers", &limits->maxFramebufferLayers) &&
    visitor->Visit("framebufferColorSampleCounts", &limits->framebufferColorSampleCounts) &&
    visitor->Visit("framebufferDepthSampleCounts", &limits->framebufferDepthSampleCounts) &&
    visitor->Visit("framebufferStencilSampleCounts", &limits->framebufferStencilSampleCounts) &&
    visitor->Visit("framebufferNoAttachmentsSampleCounts", &limits->framebufferNoAttachmentsSampleCounts) &&
    visitor->Visit("maxColorAttachments", &limits->maxColorAttachments) &&
    visitor->Visit("sampledImageColorSampleCounts", &limits->sampledImageColorSampleCounts) &&
    visitor->Visit("sampledImageIntegerSampleCounts", &limits->sampledImageIntegerSampleCounts) &&
    visitor->Visit("sampledImageDepthSampleCounts", &limits->sampledImageDepthSampleCounts) &&
    visitor->Visit("sampledImageStencilSampleCounts", &limits->sampledImageStencilSampleCounts) &&
    visitor->Visit("storageImageSampleCounts", &limits->storageImageSampleCounts) &&
    visitor->Visit("maxSampleMaskWords", &limits->maxSampleMaskWords) &&
    visitor->Visit("timestampComputeAndGraphics", &limits->timestampComputeAndGraphics) &&
    visitor->Visit("timestampPeriod", &limits->timestampPeriod) &&
    visitor->Visit("maxClipDistances", &limits->maxClipDistances) &&
    visitor->Visit("maxCullDistances", &limits->maxCullDistances) &&
    visitor->Visit("maxCombinedClipAndCullDistances", &limits->maxCombinedClipAndCullDistances) &&
    visitor->Visit("discreteQueuePriorities", &limits->discreteQueuePriorities) &&
    visitor->Visit("pointSizeRange", &limits->pointSizeRange) &&
    visitor->Visit("lineWidthRange", &limits->lineWidthRange) &&
    visitor->Visit("pointSizeGranularity", &limits->pointSizeGranularity) &&
    visitor->Visit("lineWidthGranularity", &limits->lineWidthGranularity) &&
    visitor->Visit("strictLines", &limits->strictLines) &&
    visitor->Visit("standardSampleLocations", &limits->standardSampleLocations) &&
    visitor->Visit("optimalBufferCopyOffsetAlignment", &limits->optimalBufferCopyOffsetAlignment) &&
    visitor->Visit("optimalBufferCopyRowPitchAlignment", &limits->optimalBufferCopyRowPitchAlignment) &&
    visitor->Visit("nonCoherentAtomSize", &limits->nonCoherentAtomSize);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDeviceSparseProperties* properties) {
  return
    visitor->Visit("residencyStandard2DBlockShape", &properties->residencyStandard2DBlockShape) &&
    visitor->Visit("residencyStandard2DMultisampleBlockShape", &properties->residencyStandard2DMultisampleBlockShape) &&
    visitor->Visit("residencyStandard3DBlockShape", &properties->residencyStandard3DBlockShape) &&
    visitor->Visit("residencyAlignedMipSize", &properties->residencyAlignedMipSize) &&
    visitor->Visit("residencyNonResidentStrict", &properties->residencyNonResidentStrict);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDeviceProperties* properties) {
  return
    visitor->Visit("apiVersion", &properties->apiVersion) &&
    visitor->Visit("driverVersion", &properties->driverVersion) &&
    visitor->Visit("vendorID", &properties->vendorID) &&
    visitor->Visit("deviceID", &properties->deviceID) &&
    visitor->Visit("deviceType", &properties->deviceType) &&
    visitor->Visit("deviceName", &properties->deviceName) &&
    visitor->Visit("pipelineCacheUUID", &properties->pipelineCacheUUID) &&
    visitor->Visit("limits", &properties->limits) &&
    visitor->Visit("sparseProperties", &properties->sparseProperties);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkPhysicalDeviceFeatures* features) {
  return
    visitor->Visit("robustBufferAccess", &features->robustBufferAccess) &&
    visitor->Visit("fullDrawIndexUint32", &features->fullDrawIndexUint32) &&
    visitor->Visit("imageCubeArray", &features->imageCubeArray) &&
    visitor->Visit("independentBlend", &features->independentBlend) &&
    visitor->Visit("geometryShader", &features->geometryShader) &&
    visitor->Visit("tessellationShader", &features->tessellationShader) &&
    visitor->Visit("sampleRateShading", &features->sampleRateShading) &&
    visitor->Visit("dualSrcBlend", &features->dualSrcBlend) &&
    visitor->Visit("logicOp", &features->logicOp) &&
    visitor->Visit("multiDrawIndirect", &features->multiDrawIndirect) &&
    visitor->Visit("drawIndirectFirstInstance", &features->drawIndirectFirstInstance) &&
    visitor->Visit("depthClamp", &features->depthClamp) &&
    visitor->Visit("depthBiasClamp", &features->depthBiasClamp) &&
    visitor->Visit("fillModeNonSolid", &features->fillModeNonSolid) &&
    visitor->Visit("depthBounds", &features->depthBounds) &&
    visitor->Visit("wideLines", &features->wideLines) &&
    visitor->Visit("largePoints", &features->largePoints) &&
    visitor->Visit("alphaToOne", &features->alphaToOne) &&
    visitor->Visit("multiViewport", &features->multiViewport) &&
    visitor->Visit("samplerAnisotropy", &features->samplerAnisotropy) &&
    visitor->Visit("textureCompressionETC2", &features->textureCompressionETC2) &&
    visitor->Visit("textureCompressionASTC_LDR", &features->textureCompressionASTC_LDR) &&
    visitor->Visit("textureCompressionBC", &features->textureCompressionBC) &&
    visitor->Visit("occlusionQueryPrecise", &features->occlusionQueryPrecise) &&
    visitor->Visit("pipelineStatisticsQuery", &features->pipelineStatisticsQuery) &&
    visitor->Visit("vertexPipelineStoresAndAtomics", &features->vertexPipelineStoresAndAtomics) &&
    visitor->Visit("fragmentStoresAndAtomics", &features->fragmentStoresAndAtomics) &&
    visitor->Visit("shaderTessellationAndGeometryPointSize", &features->shaderTessellationAndGeometryPointSize) &&
    visitor->Visit("shaderImageGatherExtended", &features->shaderImageGatherExtended) &&
    visitor->Visit("shaderStorageImageExtendedFormats", &features->shaderStorageImageExtendedFormats) &&
    visitor->Visit("shaderStorageImageMultisample", &features->shaderStorageImageMultisample) &&
    visitor->Visit("shaderStorageImageReadWithoutFormat", &features->shaderStorageImageReadWithoutFormat) &&
    visitor->Visit("shaderStorageImageWriteWithoutFormat", &features->shaderStorageImageWriteWithoutFormat) &&
    visitor->Visit("shaderUniformBufferArrayDynamicIndexing", &features->shaderUniformBufferArrayDynamicIndexing) &&
    visitor->Visit("shaderSampledImageArrayDynamicIndexing", &features->shaderSampledImageArrayDynamicIndexing) &&
    visitor->Visit("shaderStorageBufferArrayDynamicIndexing", &features->shaderStorageBufferArrayDynamicIndexing) &&
    visitor->Visit("shaderStorageImageArrayDynamicIndexing", &features->shaderStorageImageArrayDynamicIndexing) &&
    visitor->Visit("shaderClipDistance", &features->shaderClipDistance) &&
    visitor->Visit("shaderCullDistance", &features->shaderCullDistance) &&
    visitor->Visit("shaderFloat64", &features->shaderFloat64) &&
    visitor->Visit("shaderInt64", &features->shaderInt64) &&
    visitor->Visit("shaderInt16", &features->shaderInt16) &&
    visitor->Visit("shaderResourceResidency", &features->shaderResourceResidency) &&
    visitor->Visit("shaderResourceMinLod", &features->shaderResourceMinLod) &&
    visitor->Visit("sparseBinding", &features->sparseBinding) &&
    visitor->Visit("sparseResidencyBuffer", &features->sparseResidencyBuffer) &&
    visitor->Visit("sparseResidencyImage2D", &features->sparseResidencyImage2D) &&
    visitor->Visit("sparseResidencyImage3D", &features->sparseResidencyImage3D) &&
    visitor->Visit("sparseResidency2Samples", &features->sparseResidency2Samples) &&
    visitor->Visit("sparseResidency4Samples", &features->sparseResidency4Samples) &&
    visitor->Visit("sparseResidency8Samples", &features->sparseResidency8Samples) &&
    visitor->Visit("sparseResidency16Samples", &features->sparseResidency16Samples) &&
    visitor->Visit("sparseResidencyAliased", &features->sparseResidencyAliased) &&
    visitor->Visit("variableMultisampleRate", &features->variableMultisampleRate) &&
    visitor->Visit("inheritedQueries", &features->inheritedQueries);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkJsonExtDriverProperties* properties) {
  return visitor->Visit("driverPropertiesKHR",
                        &properties->driver_properties_khr);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDeviceDriverPropertiesKHR* properties) {
  return visitor->Visit("driverID", &properties->driverID) &&
         visitor->Visit("driverName", &properties->driverName) &&
         visitor->Visit("driverInfo", &properties->driverInfo) &&
         visitor->Visit("conformanceVersion", &properties->conformanceVersion);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkConformanceVersionKHR* version) {
  return visitor->Visit("major", &version->major) &&
         visitor->Visit("minor", &version->minor) &&
         visitor->Visit("subminor", &version->subminor) &&
         visitor->Visit("patch", &version->patch);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkJsonExtVariablePointerFeatures* features) {
  return visitor->Visit("variablePointerFeaturesKHR",
                        &features->variable_pointer_features_khr);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkMemoryType* type) {
  return
    visitor->Visit("propertyFlags", &type->propertyFlags) &&
    visitor->Visit("heapIndex", &type->heapIndex);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkMemoryHeap* heap) {
  return
    visitor->Visit("size", &heap->size) &&
    visitor->Visit("flags", &heap->flags);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkPhysicalDeviceMemoryProperties* properties) {
  return
    visitor->Visit("memoryTypeCount", &properties->memoryTypeCount) &&
    visitor->VisitArray("memoryTypes", properties->memoryTypeCount, &properties->memoryTypes) &&
    visitor->Visit("memoryHeapCount", &properties->memoryHeapCount) &&
    visitor->VisitArray("memoryHeaps", properties->memoryHeapCount, &properties->memoryHeaps);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDeviceSubgroupProperties* properties) {
  return visitor->Visit("subgroupSize", &properties->subgroupSize) &&
         visitor->Visit("supportedStages", &properties->supportedStages) &&
         visitor->Visit("supportedOperations",
                        &properties->supportedOperations) &&
         visitor->Visit("quadOperationsInAllStages",
                        &properties->quadOperationsInAllStages);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDevicePointClippingProperties* properties) {
  return visitor->Visit("pointClippingBehavior",
                        &properties->pointClippingBehavior);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDeviceMultiviewProperties* properties) {
  return visitor->Visit("maxMultiviewViewCount",
                        &properties->maxMultiviewViewCount) &&
         visitor->Visit("maxMultiviewInstanceIndex",
                        &properties->maxMultiviewInstanceIndex);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDeviceIDProperties* properties) {
  return visitor->Visit("deviceUUID", &properties->deviceUUID) &&
         visitor->Visit("driverUUID", &properties->driverUUID) &&
         visitor->Visit("deviceLUID", &properties->deviceLUID) &&
         visitor->Visit("deviceNodeMask", &properties->deviceNodeMask) &&
         visitor->Visit("deviceLUIDValid", &properties->deviceLUIDValid);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDeviceMaintenance3Properties* properties) {
  return visitor->Visit("maxPerSetDescriptors",
                        &properties->maxPerSetDescriptors) &&
         visitor->Visit("maxMemoryAllocationSize",
                        &properties->maxMemoryAllocationSize);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDevice16BitStorageFeatures* features) {
  return visitor->Visit("storageBuffer16BitAccess",
                        &features->storageBuffer16BitAccess) &&
         visitor->Visit("uniformAndStorageBuffer16BitAccess",
                        &features->uniformAndStorageBuffer16BitAccess) &&
         visitor->Visit("storagePushConstant16",
                        &features->storagePushConstant16) &&
         visitor->Visit("storageInputOutput16",
                        &features->storageInputOutput16);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDeviceMultiviewFeatures* features) {
  return visitor->Visit("multiview", &features->multiview) &&
         visitor->Visit("multiviewGeometryShader",
                        &features->multiviewGeometryShader) &&
         visitor->Visit("multiviewTessellationShader",
                        &features->multiviewTessellationShader);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDeviceVariablePointerFeatures* features) {
  return visitor->Visit("variablePointersStorageBuffer",
                        &features->variablePointersStorageBuffer) &&
         visitor->Visit("variablePointers", &features->variablePointers);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDeviceProtectedMemoryFeatures* features) {
  return visitor->Visit("protectedMemory", &features->protectedMemory);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDeviceSamplerYcbcrConversionFeatures* features) {
  return visitor->Visit("samplerYcbcrConversion",
                        &features->samplerYcbcrConversion);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkPhysicalDeviceShaderDrawParameterFeatures* features) {
  return visitor->Visit("shaderDrawParameters",
                        &features->shaderDrawParameters);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkExternalFenceProperties* properties) {
  return visitor->Visit("exportFromImportedHandleTypes",
                        &properties->exportFromImportedHandleTypes) &&
         visitor->Visit("compatibleHandleTypes",
                        &properties->compatibleHandleTypes) &&
         visitor->Visit("externalFenceFeatures",
                        &properties->externalFenceFeatures);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor,
                    VkExternalSemaphoreProperties* properties) {
  return visitor->Visit("exportFromImportedHandleTypes",
                        &properties->exportFromImportedHandleTypes) &&
         visitor->Visit("compatibleHandleTypes",
                        &properties->compatibleHandleTypes) &&
         visitor->Visit("externalSemaphoreFeatures",
                        &properties->externalSemaphoreFeatures);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkQueueFamilyProperties* properties) {
  return
    visitor->Visit("queueFlags", &properties->queueFlags) &&
    visitor->Visit("queueCount", &properties->queueCount) &&
    visitor->Visit("timestampValidBits", &properties->timestampValidBits) &&
    visitor->Visit("minImageTransferGranularity", &properties->minImageTransferGranularity);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkExtensionProperties* properties) {
  return
    visitor->Visit("extensionName", &properties->extensionName) &&
    visitor->Visit("specVersion", &properties->specVersion);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkLayerProperties* properties) {
  return
    visitor->Visit("layerName", &properties->layerName) &&
    visitor->Visit("specVersion", &properties->specVersion) &&
    visitor->Visit("implementationVersion", &properties->implementationVersion) &&
    visitor->Visit("description", &properties->description);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkFormatProperties* properties) {
  return
    visitor->Visit("linearTilingFeatures", &properties->linearTilingFeatures) &&
    visitor->Visit("optimalTilingFeatures", &properties->optimalTilingFeatures) &&
    visitor->Visit("bufferFeatures", &properties->bufferFeatures);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkJsonLayer* layer) {
  return visitor->Visit("properties", &layer->properties) &&
         visitor->Visit("extensions", &layer->extensions);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkJsonDeviceGroup* device_group) {
  return visitor->Visit("devices", &device_group->device_inds) &&
         visitor->Visit("subsetAllocation",
                        &device_group->properties.subsetAllocation);
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkJsonDevice* device) {
  bool ret = true;
  switch (device->properties.apiVersion ^
          VK_VERSION_PATCH(device->properties.apiVersion)) {
    case VK_API_VERSION_1_1:
      ret &=
          visitor->Visit("subgroupProperties", &device->subgroup_properties) &&
          visitor->Visit("pointClippingProperties",
                         &device->point_clipping_properties) &&
          visitor->Visit("multiviewProperties",
                         &device->multiview_properties) &&
          visitor->Visit("idProperties", &device->id_properties) &&
          visitor->Visit("maintenance3Properties",
                         &device->maintenance3_properties) &&
          visitor->Visit("16bitStorageFeatures",
                         &device->bit16_storage_features) &&
          visitor->Visit("multiviewFeatures", &device->multiview_features) &&
          visitor->Visit("variablePointerFeatures",
                         &device->variable_pointer_features) &&
          visitor->Visit("protectedMemoryFeatures",
                         &device->protected_memory_features) &&
          visitor->Visit("samplerYcbcrConversionFeatures",
                         &device->sampler_ycbcr_conversion_features) &&
          visitor->Visit("shaderDrawParameterFeatures",
                         &device->shader_draw_parameter_features) &&
          visitor->Visit("externalFenceProperties",
                         &device->external_fence_properties) &&
          visitor->Visit("externalSemaphoreProperties",
                         &device->external_semaphore_properties);
      FALLTHROUGH_INTENDED;
    case VK_API_VERSION_1_0:
      ret &= visitor->Visit("properties", &device->properties) &&
             visitor->Visit("features", &device->features) &&
             visitor->Visit("memory", &device->memory) &&
             visitor->Visit("queues", &device->queues) &&
             visitor->Visit("extensions", &device->extensions) &&
             visitor->Visit("layers", &device->layers) &&
             visitor->Visit("formats", &device->formats);
      if (device->ext_driver_properties.reported) {
        ret &= visitor->Visit("VK_KHR_driver_properties",
                            &device->ext_driver_properties);
      }
      if (device->ext_variable_pointer_features.reported) {
        ret &= visitor->Visit("VK_KHR_variable_pointers",
                            &device->ext_variable_pointer_features);
      }
  }
  return ret;
}

template <typename Visitor>
inline bool Iterate(Visitor* visitor, VkJsonInstance* instance) {
  bool ret = true;
  switch (instance->api_version ^ VK_VERSION_PATCH(instance->api_version)) {
    case VK_API_VERSION_1_1:
      ret &= visitor->Visit("deviceGroups", &instance->device_groups);
      FALLTHROUGH_INTENDED;
    case VK_API_VERSION_1_0:
      ret &= visitor->Visit("layers", &instance->layers) &&
             visitor->Visit("extensions", &instance->extensions) &&
             visitor->Visit("devices", &instance->devices);
  }
  return ret;
}

template <typename T>
using EnableForArithmetic =
    typename std::enable_if<std::is_arithmetic<T>::value, void>::type;

template <typename T>
using EnableForStruct =
    typename std::enable_if<std::is_class<T>::value, void>::type;

template <typename T>
using EnableForEnum =
    typename std::enable_if<std::is_enum<T>::value, void>::type;

template <typename T, typename = EnableForStruct<T>, typename = void>
Json::Value ToJsonValue(const T& value);

template <typename T, typename = EnableForArithmetic<T>>
inline Json::Value ToJsonValue(const T& value) {
  return Json::Value(static_cast<double>(value));
}

inline Json::Value ToJsonValue(const uint64_t& value) {
  char string[19] = {0};  // "0x" + 16 digits + terminal \0
  snprintf(string, sizeof(string), "0x%016" PRIx64, value);
  return Json::Value(string);
}

template <typename T, typename = EnableForEnum<T>, typename = void,
          typename = void>
inline Json::Value ToJsonValue(const T& value) {
  return Json::Value(static_cast<double>(value));
}

template <typename T>
inline Json::Value ArrayToJsonValue(uint32_t count, const T* values) {
  Json::Value array(Json::arrayValue);
  for (unsigned int i = 0; i < count; ++i) array.append(ToJsonValue(values[i]));
  return array;
}

template <typename T, unsigned int N>
inline Json::Value ToJsonValue(const T (&value)[N]) {
  return ArrayToJsonValue(N, value);
}

template <size_t N>
inline Json::Value ToJsonValue(const char (&value)[N]) {
  assert(strlen(value) < N);
  return Json::Value(value);
}

template <typename T>
inline Json::Value ToJsonValue(const std::vector<T>& value) {
  assert(value.size() <= std::numeric_limits<uint32_t>::max());
  return ArrayToJsonValue(static_cast<uint32_t>(value.size()), value.data());
}

template <typename F, typename S>
inline Json::Value ToJsonValue(const std::pair<F, S>& value) {
  Json::Value array(Json::arrayValue);
  array.append(ToJsonValue(value.first));
  array.append(ToJsonValue(value.second));
  return array;
}

template <typename F, typename S>
inline Json::Value ToJsonValue(const std::map<F, S>& value) {
  Json::Value array(Json::arrayValue);
  for (auto& kv : value) array.append(ToJsonValue(kv));
  return array;
}

class JsonWriterVisitor {
 public:
  JsonWriterVisitor() : object_(Json::objectValue) {}

  ~JsonWriterVisitor() {}

  template <typename T> bool Visit(const char* key, const T* value) {
    object_[key] = ToJsonValue(*value);
    return true;
  }

  template <typename T, uint32_t N>
  bool VisitArray(const char* key, uint32_t count, const T (*value)[N]) {
    assert(count <= N);
    object_[key] = ArrayToJsonValue(count, *value);
    return true;
  }

  Json::Value get_object() const { return object_; }

 private:
  Json::Value object_;
};

template <typename Visitor, typename T>
inline void VisitForWrite(Visitor* visitor, const T& t) {
  Iterate(visitor, const_cast<T*>(&t));
}

template <typename T, typename /*= EnableForStruct<T>*/, typename /*= void*/>
Json::Value ToJsonValue(const T& value) {
  JsonWriterVisitor visitor;
  VisitForWrite(&visitor, value);
  return visitor.get_object();
}

template <typename T, typename = EnableForStruct<T>>
bool AsValue(Json::Value* json_value, T* t);

inline bool AsValue(Json::Value* json_value, int32_t* value) {
  if (json_value->type() != Json::realValue) return false;
  double d = json_value->asDouble();
  if (!IsIntegral(d) ||
      d < static_cast<double>(std::numeric_limits<int32_t>::min()) ||
      d > static_cast<double>(std::numeric_limits<int32_t>::max()))
    return false;
  *value = static_cast<int32_t>(d);
  return true;
}

inline bool AsValue(Json::Value* json_value, uint64_t* value) {
  if (json_value->type() != Json::stringValue) return false;
  int result =
      std::sscanf(json_value->asString().c_str(), "0x%016" PRIx64, value);
  return result == 1;
}

inline bool AsValue(Json::Value* json_value, uint32_t* value) {
  if (json_value->type() != Json::realValue) return false;
  double d = json_value->asDouble();
  if (!IsIntegral(d) || d < 0.0 ||
      d > static_cast<double>(std::numeric_limits<uint32_t>::max()))
    return false;
  *value = static_cast<uint32_t>(d);
  return true;
}

inline bool AsValue(Json::Value* json_value, uint8_t* value) {
  uint32_t value32 = 0;
  AsValue(json_value, &value32);
  if (value32 > std::numeric_limits<uint8_t>::max())
    return false;
  *value = static_cast<uint8_t>(value32);
  return true;
}

inline bool AsValue(Json::Value* json_value, float* value) {
  if (json_value->type() != Json::realValue) return false;
  *value = static_cast<float>(json_value->asDouble());
  return true;
}

template <typename T>
inline bool AsArray(Json::Value* json_value, uint32_t count, T* values) {
  if (json_value->type() != Json::arrayValue || json_value->size() != count)
    return false;
  for (uint32_t i = 0; i < count; ++i) {
    if (!AsValue(&(*json_value)[i], values + i)) return false;
  }
  return true;
}

template <typename T, unsigned int N>
inline bool AsValue(Json::Value* json_value, T (*value)[N]) {
  return AsArray(json_value, N, *value);
}

template <size_t N>
inline bool AsValue(Json::Value* json_value, char (*value)[N]) {
  if (json_value->type() != Json::stringValue) return false;
  size_t len = json_value->asString().length();
  if (len >= N)
    return false;
  memcpy(*value, json_value->asString().c_str(), len);
  memset(*value + len, 0, N-len);
  return true;
}

template <typename T, typename = EnableForEnum<T>, typename = void>
inline bool AsValue(Json::Value* json_value, T* t) {
  uint32_t value = 0;
  if (!AsValue(json_value, &value))
      return false;
  if (!EnumTraits<T>::exist(value)) return false;
  *t = static_cast<T>(value);
  return true;
}

template <typename T>
inline bool AsValue(Json::Value* json_value, std::vector<T>* value) {
  if (json_value->type() != Json::arrayValue) return false;
  int size = json_value->size();
  value->resize(size);
  return AsArray(json_value, size, value->data());
}

template <typename F, typename S>
inline bool AsValue(Json::Value* json_value, std::pair<F, S>* value) {
  if (json_value->type() != Json::arrayValue || json_value->size() != 2)
    return false;
  return AsValue(&(*json_value)[0], &value->first) &&
         AsValue(&(*json_value)[1], &value->second);
}

template <typename F, typename S>
inline bool AsValue(Json::Value* json_value, std::map<F, S>* value) {
  if (json_value->type() != Json::arrayValue) return false;
  int size = json_value->size();
  for (int i = 0; i < size; ++i) {
    std::pair<F, S> elem;
    if (!AsValue(&(*json_value)[i], &elem)) return false;
    if (!value->insert(elem).second)
      return false;
  }
  return true;
}

template <typename T>
bool ReadValue(Json::Value* object, const char* key, T* value,
               std::string* errors) {
  Json::Value json_value = (*object)[key];
  if (!json_value) {
    if (errors)
      *errors = std::string(key) + " missing.";
    return false;
  }
  if (AsValue(&json_value, value)) return true;
  if (errors)
    *errors = std::string("Wrong type for ") + std::string(key) + ".";
  return false;
}

template <typename Visitor, typename T>
inline bool VisitForRead(Visitor* visitor, T* t) {
  return Iterate(visitor, t);
}

class JsonReaderVisitor {
 public:
  JsonReaderVisitor(Json::Value* object, std::string* errors)
      : object_(object), errors_(errors) {}

  template <typename T> bool Visit(const char* key, T* value) const {
    return ReadValue(object_, key, value, errors_);
  }

  template <typename T, uint32_t N>
  bool VisitArray(const char* key, uint32_t count, T (*value)[N]) {
    if (count > N)
      return false;
    Json::Value json_value = (*object_)[key];
    if (!json_value) {
      if (errors_)
        *errors_ = std::string(key) + " missing.";
      return false;
    }
    if (AsArray(&json_value, count, *value)) return true;
    if (errors_)
      *errors_ = std::string("Wrong type for ") + std::string(key) + ".";
    return false;
  }


 private:
  Json::Value* object_;
  std::string* errors_;
};

template <typename T, typename /*= EnableForStruct<T>*/>
bool AsValue(Json::Value* json_value, T* t) {
  if (json_value->type() != Json::objectValue) return false;
  JsonReaderVisitor visitor(json_value, nullptr);
  return VisitForRead(&visitor, t);
}


template <typename T> std::string VkTypeToJson(const T& t) {
  JsonWriterVisitor visitor;
  VisitForWrite(&visitor, t);
  return visitor.get_object().toStyledString();
}

template <typename T> bool VkTypeFromJson(const std::string& json,
                                          T* t,
                                          std::string* errors) {
  *t = T();
  Json::Value object(Json::objectValue);
  Json::Reader reader;
  reader.parse(json, object, false);
  if (!object) {
    if (errors) errors->assign(reader.getFormatedErrorMessages());
    return false;
  }
  return AsValue(&object, t);
}

}  // anonymous namespace

std::string VkJsonInstanceToJson(const VkJsonInstance& instance) {
  return VkTypeToJson(instance);
}

bool VkJsonInstanceFromJson(const std::string& json,
                            VkJsonInstance* instance,
                            std::string* errors) {
  return VkTypeFromJson(json, instance, errors);
}

std::string VkJsonDeviceToJson(const VkJsonDevice& device) {
  return VkTypeToJson(device);
}

bool VkJsonDeviceFromJson(const std::string& json,
                          VkJsonDevice* device,
                          std::string* errors) {
  return VkTypeFromJson(json, device, errors);
};

std::string VkJsonImageFormatPropertiesToJson(
    const VkImageFormatProperties& properties) {
  return VkTypeToJson(properties);
}

bool VkJsonImageFormatPropertiesFromJson(const std::string& json,
                                         VkImageFormatProperties* properties,
                                         std::string* errors) {
  return VkTypeFromJson(json, properties, errors);
};
