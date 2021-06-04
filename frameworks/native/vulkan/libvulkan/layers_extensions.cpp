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

#include "layers_extensions.h"

#include <alloca.h>
#include <dirent.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/prctl.h>

#include <mutex>
#include <string>
#include <vector>

#include <android/dlext.h>
#include <android-base/strings.h>
#include <cutils/properties.h>
#include <graphicsenv/GraphicsEnv.h>
#include <log/log.h>
#include <nativebridge/native_bridge.h>
#include <nativeloader/native_loader.h>
#include <utils/Trace.h>
#include <ziparchive/zip_archive.h>

// TODO(jessehall): The whole way we deal with extensions is pretty hokey, and
// not a good long-term solution. Having a hard-coded enum of extensions is
// bad, of course. Representing sets of extensions (requested, supported, etc.)
// as a bitset isn't necessarily bad, if the mapping from extension to bit were
// dynamic. Need to rethink this completely when there's a little more time.

// TODO(jessehall): This file currently builds up global data structures as it
// loads, and never cleans them up. This means we're doing heap allocations
// without going through an app-provided allocator, but worse, we'll leak those
// allocations if the loader is unloaded.
//
// We should allocate "enough" BSS space, and suballocate from there. Will
// probably want to intern strings, etc., and will need some custom/manual data
// structures.

namespace vulkan {
namespace api {

struct Layer {
    VkLayerProperties properties;
    size_t library_idx;

    // true if the layer intercepts vkCreateDevice and device commands
    bool is_global;

    std::vector<VkExtensionProperties> instance_extensions;
    std::vector<VkExtensionProperties> device_extensions;
};

namespace {

const char kSystemLayerLibraryDir[] = "/data/local/debug/vulkan";

class LayerLibrary {
   public:
    explicit LayerLibrary(const std::string& path,
                          const std::string& filename)
        : path_(path),
          filename_(filename),
          dlhandle_(nullptr),
          native_bridge_(false),
          refcount_(0) {}

    LayerLibrary(LayerLibrary&& other) noexcept
        : path_(std::move(other.path_)),
          filename_(std::move(other.filename_)),
          dlhandle_(other.dlhandle_),
          native_bridge_(other.native_bridge_),
          refcount_(other.refcount_) {
        other.dlhandle_ = nullptr;
        other.refcount_ = 0;
    }

    LayerLibrary(const LayerLibrary&) = delete;
    LayerLibrary& operator=(const LayerLibrary&) = delete;

    // these are thread-safe
    bool Open();
    void Close();

    bool EnumerateLayers(size_t library_idx,
                         std::vector<Layer>& instance_layers) const;

    void* GetGPA(const Layer& layer,
                 const char* gpa_name,
                 size_t gpa_name_len) const;

    const std::string GetFilename() { return filename_; }

   private:
    // TODO(b/79940628): remove that adapter when we could use NativeBridgeGetTrampoline
    // for native libraries.
    template<typename Func = void*>
    Func GetTrampoline(const char* name) const {
        if (native_bridge_) {
            return reinterpret_cast<Func>(android::NativeBridgeGetTrampoline(
                dlhandle_, name, nullptr, 0));
        }
        return reinterpret_cast<Func>(dlsym(dlhandle_, name));
    }

    const std::string path_;

    // Track the filename alone so we can detect duplicates
    const std::string filename_;

    std::mutex mutex_;
    void* dlhandle_;
    bool native_bridge_;
    size_t refcount_;
};

bool LayerLibrary::Open() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (refcount_++ == 0) {
        ALOGV("opening layer library '%s'", path_.c_str());
        // Libraries in the system layer library dir can't be loaded into
        // the application namespace. That causes compatibility problems, since
        // any symbol dependencies will be resolved by system libraries. They
        // can't safely use libc++_shared, for example. Which is one reason
        // (among several) we only allow them in non-user builds.
        auto app_namespace = android::GraphicsEnv::getInstance().getAppNamespace();
        if (app_namespace &&
            !android::base::StartsWith(path_, kSystemLayerLibraryDir)) {
            char* error_msg = nullptr;
            dlhandle_ = OpenNativeLibraryInNamespace(
                app_namespace, path_.c_str(), &native_bridge_, &error_msg);
            if (!dlhandle_) {
                ALOGE("failed to load layer library '%s': %s", path_.c_str(), error_msg);
                android::NativeLoaderFreeErrorMessage(error_msg);
                refcount_ = 0;
                return false;
            }
        } else {
          dlhandle_ = dlopen(path_.c_str(), RTLD_NOW | RTLD_LOCAL);
            if (!dlhandle_) {
                ALOGE("failed to load layer library '%s': %s", path_.c_str(),
                      dlerror());
                refcount_ = 0;
                return false;
            }
        }
    }
    return true;
}

void LayerLibrary::Close() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (--refcount_ == 0) {
        ALOGV("closing layer library '%s'", path_.c_str());
        char* error_msg = nullptr;
        if (!android::CloseNativeLibrary(dlhandle_, native_bridge_, &error_msg)) {
            ALOGE("failed to unload library '%s': %s", path_.c_str(), error_msg);
            android::NativeLoaderFreeErrorMessage(error_msg);
            refcount_++;
        } else {
           dlhandle_ = nullptr;
        }
    }
}

bool LayerLibrary::EnumerateLayers(size_t library_idx,
                                   std::vector<Layer>& instance_layers) const {
    PFN_vkEnumerateInstanceLayerProperties enumerate_instance_layers =
        GetTrampoline<PFN_vkEnumerateInstanceLayerProperties>(
            "vkEnumerateInstanceLayerProperties");
    PFN_vkEnumerateInstanceExtensionProperties enumerate_instance_extensions =
        GetTrampoline<PFN_vkEnumerateInstanceExtensionProperties>(
            "vkEnumerateInstanceExtensionProperties");
    if (!enumerate_instance_layers || !enumerate_instance_extensions) {
        ALOGE("layer library '%s' missing some instance enumeration functions",
              path_.c_str());
        return false;
    }

    // device functions are optional
    PFN_vkEnumerateDeviceLayerProperties enumerate_device_layers =
        GetTrampoline<PFN_vkEnumerateDeviceLayerProperties>(
            "vkEnumerateDeviceLayerProperties");
    PFN_vkEnumerateDeviceExtensionProperties enumerate_device_extensions =
        GetTrampoline<PFN_vkEnumerateDeviceExtensionProperties>(
            "vkEnumerateDeviceExtensionProperties");

    // get layer counts
    uint32_t num_instance_layers = 0;
    uint32_t num_device_layers = 0;
    VkResult result = enumerate_instance_layers(&num_instance_layers, nullptr);
    if (result != VK_SUCCESS || !num_instance_layers) {
        if (result != VK_SUCCESS) {
            ALOGE(
                "vkEnumerateInstanceLayerProperties failed for library '%s': "
                "%d",
                path_.c_str(), result);
        }
        return false;
    }
    if (enumerate_device_layers) {
        result = enumerate_device_layers(VK_NULL_HANDLE, &num_device_layers,
                                         nullptr);
        if (result != VK_SUCCESS) {
            ALOGE(
                "vkEnumerateDeviceLayerProperties failed for library '%s': %d",
                path_.c_str(), result);
            return false;
        }
    }

    // get layer properties
    VkLayerProperties* properties = static_cast<VkLayerProperties*>(alloca(
        (num_instance_layers + num_device_layers) * sizeof(VkLayerProperties)));
    result = enumerate_instance_layers(&num_instance_layers, properties);
    if (result != VK_SUCCESS) {
        ALOGE("vkEnumerateInstanceLayerProperties failed for library '%s': %d",
              path_.c_str(), result);
        return false;
    }
    if (num_device_layers > 0) {
        result = enumerate_device_layers(VK_NULL_HANDLE, &num_device_layers,
                                         properties + num_instance_layers);
        if (result != VK_SUCCESS) {
            ALOGE(
                "vkEnumerateDeviceLayerProperties failed for library '%s': %d",
                path_.c_str(), result);
            return false;
        }
    }

    // append layers to instance_layers
    size_t prev_num_instance_layers = instance_layers.size();
    instance_layers.reserve(prev_num_instance_layers + num_instance_layers);
    for (size_t i = 0; i < num_instance_layers; i++) {
        const VkLayerProperties& props = properties[i];

        Layer layer;
        layer.properties = props;
        layer.library_idx = library_idx;
        layer.is_global = false;

        uint32_t count = 0;
        result =
            enumerate_instance_extensions(props.layerName, &count, nullptr);
        if (result != VK_SUCCESS) {
            ALOGE(
                "vkEnumerateInstanceExtensionProperties(\"%s\") failed for "
                "library '%s': %d",
                props.layerName, path_.c_str(), result);
            instance_layers.resize(prev_num_instance_layers);
            return false;
        }
        layer.instance_extensions.resize(count);
        result = enumerate_instance_extensions(
            props.layerName, &count, layer.instance_extensions.data());
        if (result != VK_SUCCESS) {
            ALOGE(
                "vkEnumerateInstanceExtensionProperties(\"%s\") failed for "
                "library '%s': %d",
                props.layerName, path_.c_str(), result);
            instance_layers.resize(prev_num_instance_layers);
            return false;
        }

        for (size_t j = 0; j < num_device_layers; j++) {
            const auto& dev_props = properties[num_instance_layers + j];
            if (memcmp(&props, &dev_props, sizeof(props)) == 0) {
                layer.is_global = true;
                break;
            }
        }

        if (layer.is_global && enumerate_device_extensions) {
            result = enumerate_device_extensions(
                VK_NULL_HANDLE, props.layerName, &count, nullptr);
            if (result != VK_SUCCESS) {
                ALOGE(
                    "vkEnumerateDeviceExtensionProperties(\"%s\") failed for "
                    "library '%s': %d",
                    props.layerName, path_.c_str(), result);
                instance_layers.resize(prev_num_instance_layers);
                return false;
            }
            layer.device_extensions.resize(count);
            result = enumerate_device_extensions(
                VK_NULL_HANDLE, props.layerName, &count,
                layer.device_extensions.data());
            if (result != VK_SUCCESS) {
                ALOGE(
                    "vkEnumerateDeviceExtensionProperties(\"%s\") failed for "
                    "library '%s': %d",
                    props.layerName, path_.c_str(), result);
                instance_layers.resize(prev_num_instance_layers);
                return false;
            }
        }

        instance_layers.push_back(layer);
        ALOGD("added %s layer '%s' from library '%s'",
              (layer.is_global) ? "global" : "instance", props.layerName,
              path_.c_str());
    }

    return true;
}

void* LayerLibrary::GetGPA(const Layer& layer,
                           const char* gpa_name,
                           size_t gpa_name_len) const {
    void* gpa;
    size_t layer_name_len =
        std::max(size_t{2}, strlen(layer.properties.layerName));
    char* name = static_cast<char*>(alloca(layer_name_len + gpa_name_len + 1));
    strcpy(name, layer.properties.layerName);
    strcpy(name + layer_name_len, gpa_name);
    if (!(gpa = GetTrampoline(name))) {
        strcpy(name, "vk");
        strcpy(name + 2, gpa_name);
        gpa = GetTrampoline(name);
    }
    return gpa;
}

// ----------------------------------------------------------------------------

std::vector<LayerLibrary> g_layer_libraries;
std::vector<Layer> g_instance_layers;

void AddLayerLibrary(const std::string& path, const std::string& filename) {
    LayerLibrary library(path + "/" + filename, filename);
    if (!library.Open())
        return;

    if (!library.EnumerateLayers(g_layer_libraries.size(), g_instance_layers)) {
        library.Close();
        return;
    }

    library.Close();

    g_layer_libraries.emplace_back(std::move(library));
}

template <typename Functor>
void ForEachFileInDir(const std::string& dirname, Functor functor) {
    auto dir_deleter = [](DIR* handle) { closedir(handle); };
    std::unique_ptr<DIR, decltype(dir_deleter)> dir(opendir(dirname.c_str()),
                                                    dir_deleter);
    if (!dir) {
        // It's normal for some search directories to not exist, especially
        // /data/local/debug/vulkan.
        int err = errno;
        ALOGW_IF(err != ENOENT, "failed to open layer directory '%s': %s",
                 dirname.c_str(), strerror(err));
        return;
    }
    ALOGD("searching for layers in '%s'", dirname.c_str());
    dirent* entry;
    while ((entry = readdir(dir.get())) != nullptr)
        functor(entry->d_name);
}

template <typename Functor>
void ForEachFileInZip(const std::string& zipname,
                      const std::string& dir_in_zip,
                      Functor functor) {
    int32_t err;
    ZipArchiveHandle zip = nullptr;
    if ((err = OpenArchive(zipname.c_str(), &zip)) != 0) {
        ALOGE("failed to open apk '%s': %d", zipname.c_str(), err);
        return;
    }
    std::string prefix(dir_in_zip + "/");
    const ZipString prefix_str(prefix.c_str());
    void* iter_cookie = nullptr;
    if ((err = StartIteration(zip, &iter_cookie, &prefix_str, nullptr)) != 0) {
        ALOGE("failed to iterate entries in apk '%s': %d", zipname.c_str(),
              err);
        CloseArchive(zip);
        return;
    }
    ALOGD("searching for layers in '%s!/%s'", zipname.c_str(),
          dir_in_zip.c_str());
    ZipEntry entry;
    ZipString name;
    while (Next(iter_cookie, &entry, &name) == 0) {
        std::string filename(
            reinterpret_cast<const char*>(name.name) + prefix.length(),
            name.name_length - prefix.length());
        // only enumerate direct entries of the directory, not subdirectories
        if (filename.find('/') != filename.npos)
            continue;
        // Check whether it *may* be possible to load the library directly from
        // the APK. Loading still may fail for other reasons, but this at least
        // lets us avoid failed-to-load log messages in the typical case of
        // compressed and/or unaligned libraries.
        if (entry.method != kCompressStored || entry.offset % PAGE_SIZE != 0)
            continue;
        functor(filename);
    }
    EndIteration(iter_cookie);
    CloseArchive(zip);
}

template <typename Functor>
void ForEachFileInPath(const std::string& path, Functor functor) {
    size_t zip_pos = path.find("!/");
    if (zip_pos == std::string::npos) {
        ForEachFileInDir(path, functor);
    } else {
        ForEachFileInZip(path.substr(0, zip_pos), path.substr(zip_pos + 2),
                         functor);
    }
}

void DiscoverLayersInPathList(const std::string& pathstr) {
    ATRACE_CALL();

    std::vector<std::string> paths = android::base::Split(pathstr, ":");
    for (const auto& path : paths) {
        ForEachFileInPath(path, [&](const std::string& filename) {
            if (android::base::StartsWith(filename, "libVkLayer") &&
                android::base::EndsWith(filename, ".so")) {

                // Check to ensure we haven't seen this layer already
                // Let the first instance of the shared object be enumerated
                // We're searching for layers in following order:
                // 1. system path
                // 2. libraryPermittedPath (if enabled)
                // 3. libraryPath

                bool duplicate = false;
                for (auto& layer : g_layer_libraries) {
                    if (layer.GetFilename() == filename) {
                        ALOGV("Skipping duplicate layer %s in %s",
                              filename.c_str(), path.c_str());
                        duplicate = true;
                    }
                }

                if (!duplicate)
                    AddLayerLibrary(path, filename);
            }
        });
    }
}

const VkExtensionProperties* FindExtension(
    const std::vector<VkExtensionProperties>& extensions,
    const char* name) {
    auto it = std::find_if(extensions.cbegin(), extensions.cend(),
                           [=](const VkExtensionProperties& ext) {
                               return (strcmp(ext.extensionName, name) == 0);
                           });
    return (it != extensions.cend()) ? &*it : nullptr;
}

void* GetLayerGetProcAddr(const Layer& layer,
                          const char* gpa_name,
                          size_t gpa_name_len) {
    const LayerLibrary& library = g_layer_libraries[layer.library_idx];
    return library.GetGPA(layer, gpa_name, gpa_name_len);
}

}  // anonymous namespace

void DiscoverLayers() {
    ATRACE_CALL();

    if (android::GraphicsEnv::getInstance().isDebuggable()) {
        DiscoverLayersInPathList(kSystemLayerLibraryDir);
    }
    if (!android::GraphicsEnv::getInstance().getLayerPaths().empty())
        DiscoverLayersInPathList(android::GraphicsEnv::getInstance().getLayerPaths());
}

uint32_t GetLayerCount() {
    return static_cast<uint32_t>(g_instance_layers.size());
}

const Layer& GetLayer(uint32_t index) {
    return g_instance_layers[index];
}

const Layer* FindLayer(const char* name) {
    auto layer =
        std::find_if(g_instance_layers.cbegin(), g_instance_layers.cend(),
                     [=](const Layer& entry) {
                         return strcmp(entry.properties.layerName, name) == 0;
                     });
    return (layer != g_instance_layers.cend()) ? &*layer : nullptr;
}

const VkLayerProperties& GetLayerProperties(const Layer& layer) {
    return layer.properties;
}

bool IsLayerGlobal(const Layer& layer) {
    return layer.is_global;
}

const VkExtensionProperties* GetLayerInstanceExtensions(const Layer& layer,
                                                        uint32_t& count) {
    count = static_cast<uint32_t>(layer.instance_extensions.size());
    return layer.instance_extensions.data();
}

const VkExtensionProperties* GetLayerDeviceExtensions(const Layer& layer,
                                                      uint32_t& count) {
    count = static_cast<uint32_t>(layer.device_extensions.size());
    return layer.device_extensions.data();
}

const VkExtensionProperties* FindLayerInstanceExtension(const Layer& layer,
                                                        const char* name) {
    return FindExtension(layer.instance_extensions, name);
}

const VkExtensionProperties* FindLayerDeviceExtension(const Layer& layer,
                                                      const char* name) {
    return FindExtension(layer.device_extensions, name);
}

LayerRef GetLayerRef(const Layer& layer) {
    LayerLibrary& library = g_layer_libraries[layer.library_idx];
    return LayerRef((library.Open()) ? &layer : nullptr);
}

LayerRef::LayerRef(const Layer* layer) : layer_(layer) {}

LayerRef::~LayerRef() {
    if (layer_) {
        LayerLibrary& library = g_layer_libraries[layer_->library_idx];
        library.Close();
    }
}

LayerRef::LayerRef(LayerRef&& other) noexcept : layer_(other.layer_) {
    other.layer_ = nullptr;
}

PFN_vkGetInstanceProcAddr LayerRef::GetGetInstanceProcAddr() const {
    return layer_ ? reinterpret_cast<PFN_vkGetInstanceProcAddr>(
                        GetLayerGetProcAddr(*layer_, "GetInstanceProcAddr", 19))
                  : nullptr;
}

PFN_vkGetDeviceProcAddr LayerRef::GetGetDeviceProcAddr() const {
    return layer_ ? reinterpret_cast<PFN_vkGetDeviceProcAddr>(
                        GetLayerGetProcAddr(*layer_, "GetDeviceProcAddr", 17))
                  : nullptr;
}

}  // namespace api
}  // namespace vulkan
