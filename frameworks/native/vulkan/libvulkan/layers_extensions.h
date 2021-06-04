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

#ifndef LIBVULKAN_LAYERS_EXTENSIONS_H
#define LIBVULKAN_LAYERS_EXTENSIONS_H 1

#include <vulkan/vulkan.h>

namespace vulkan {
namespace api {

struct Layer;

class LayerRef {
   public:
    explicit LayerRef(const Layer* layer);
    LayerRef(LayerRef&& other) noexcept;
    ~LayerRef();
    LayerRef(const LayerRef&) = delete;
    LayerRef& operator=(const LayerRef&) = delete;

    // provides bool-like behavior
    // NOLINTNEXTLINE(google-explicit-constructor)
    operator const Layer*() const { return layer_; }

    PFN_vkGetInstanceProcAddr GetGetInstanceProcAddr() const;
    PFN_vkGetDeviceProcAddr GetGetDeviceProcAddr() const;

   private:
    const Layer* layer_;
};

void DiscoverLayers();

uint32_t GetLayerCount();
const Layer& GetLayer(uint32_t index);
const Layer* FindLayer(const char* name);

const VkLayerProperties& GetLayerProperties(const Layer& layer);
bool IsLayerGlobal(const Layer& layer);
const VkExtensionProperties* GetLayerInstanceExtensions(const Layer& layer,
                                                        uint32_t& count);
const VkExtensionProperties* GetLayerDeviceExtensions(const Layer& layer,
                                                      uint32_t& count);

const VkExtensionProperties* FindLayerInstanceExtension(const Layer& layer,
                                                        const char* name);
const VkExtensionProperties* FindLayerDeviceExtension(const Layer& layer,
                                                      const char* name);

LayerRef GetLayerRef(const Layer& layer);

}  // namespace api
}  // namespace vulkan

#endif  // LIBVULKAN_LAYERS_EXTENSIONS_H
