/*
 ** Copyright 2018, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#include "egl_layers.h"

#include <EGL/egl.h>
#include <android-base/file.h>
#include <android-base/strings.h>
#include <android/dlext.h>
#include <cutils/properties.h>
#include <dlfcn.h>
#include <graphicsenv/GraphicsEnv.h>
#include <log/log.h>
#include <nativebridge/native_bridge.h>
#include <nativeloader/native_loader.h>
#include <sys/prctl.h>

namespace android {

// GLES Layers
//
// - Layer discovery -
// 1. Check for debug layer list from GraphicsEnv
// 2. If none enabled, check system properties
//
// - Layer initializing -
// - AndroidGLESLayer_Initialize (provided by layer, called by loader)
// - AndroidGLESLayer_GetProcAddress (provided by layer, called by loader)
// - getNextLayerProcAddress (provided by loader, called by layer)
//
// 1. Walk through defs for egl and each gl version
// 2. Call GetLayerProcAddress passing the name and the target hook entry point
//   - This tells the layer the next point in the chain it should call
// 3. Replace the hook with the layer's entry point
//    - All entryoints will be present, anything unsupported by the driver will
//      have gl_unimplemented
//
// - Extension layering -
//  Not all functions are known to Android, so libEGL handles extensions.
//  They are looked up by applications using eglGetProcAddress
//  Layers can look them up with getNextLayerProcAddress

const int kFuncCount = sizeof(platform_impl_t) / sizeof(char*) + sizeof(egl_t) / sizeof(char*) +
        sizeof(gl_hooks_t) / sizeof(char*);

typedef struct FunctionTable {
    EGLFuncPointer x[kFuncCount];
    EGLFuncPointer& operator[](int i) { return x[i]; }
} FunctionTable;

// TODO: Move these to class
std::unordered_map<std::string, int> func_indices;
// func_indices.reserve(kFuncCount);

std::unordered_map<int, std::string> func_names;
// func_names.reserve(kFuncCount);

std::vector<FunctionTable> layer_functions;

const void* getNextLayerProcAddress(void* layer_id, const char* name) {
    // Use layer_id to find funcs for layer below current
    // This is the same key provided in AndroidGLESLayer_Initialize
    auto next_layer_funcs = reinterpret_cast<FunctionTable*>(layer_id);
    EGLFuncPointer val;

    ALOGV("getNextLayerProcAddress servicing %s", name);

    if (func_indices.find(name) == func_indices.end()) {
        // No entry for this function - it is an extension
        // call down the GPA chain directly to the impl
        ALOGV("getNextLayerProcAddress - name(%s) no func_indices entry found", name);

        // Look up which GPA we should use
        int gpaIndex = func_indices["eglGetProcAddress"];
        ALOGV("getNextLayerProcAddress - name(%s) gpaIndex(%i) <- using GPA from this index", name, gpaIndex);
        EGLFuncPointer gpaNext = (*next_layer_funcs)[gpaIndex];
        ALOGV("getNextLayerProcAddress - name(%s) gpaIndex(%i) gpaNext(%llu) <- using GPA at this address", name, gpaIndex, (unsigned long long)gpaNext);


        // Call it for the requested function
        typedef void* (*PFNEGLGETPROCADDRESSPROC)(const char*);
        PFNEGLGETPROCADDRESSPROC next = reinterpret_cast<PFNEGLGETPROCADDRESSPROC>(gpaNext);

        val = reinterpret_cast<EGLFuncPointer>(next(name));
        ALOGV("getNextLayerProcAddress - name(%s) gpaIndex(%i) gpaNext(%llu) Got back (%llu) from GPA", name, gpaIndex, (unsigned long long)gpaNext, (unsigned long long)val);

        // We should store it now, but to do that, we need to move func_idx to the class so we can
        // increment it separately
        // TODO: Move func_idx to class and store the result of GPA
        return reinterpret_cast<void*>(val);
    }

    int index = func_indices[name];
    val = (*next_layer_funcs)[index];
    ALOGV("getNextLayerProcAddress - name(%s) index(%i) entry(%llu) - Got a hit, returning known entry", name, index, (unsigned long long)val);
    return reinterpret_cast<void*>(val);
}

void SetupFuncMaps(FunctionTable& functions, char const* const* entries, EGLFuncPointer* curr,
                   int& func_idx) {
    while (*entries) {
        const char* name = *entries;

        // Some names overlap, only fill with initial entry
        // This does mean that some indices will not be used
        if (func_indices.find(name) == func_indices.end()) {
            ALOGV("SetupFuncMaps - name(%s), func_idx(%i), No entry for func_indices, assigning now", name, func_idx);
            func_names[func_idx] = name;
            func_indices[name] = func_idx;
        } else {
            ALOGV("SetupFuncMaps - name(%s), func_idx(%i), Found entry for func_indices", name, func_idx);
        }

        // Populate layer_functions once with initial value
        // These values will arrive in priority order, starting with platform entries
        if (functions[func_idx] == nullptr) {
            ALOGV("SetupFuncMaps - name(%s), func_idx(%i), No entry for functions, assigning (%llu)", name, func_idx, (unsigned long long) *curr);
            functions[func_idx] = *curr;
        } else {
            ALOGV("SetupFuncMaps - name(%s), func_idx(%i), Found entry for functions (%llu)", name, func_idx, (unsigned long long) functions[func_idx]);
        }

        entries++;
        curr++;
        func_idx++;
    }
}

LayerLoader& LayerLoader::getInstance() {
    // This function is mutex protected in egl_init_drivers_locked and eglGetProcAddressImpl
    static LayerLoader layer_loader;

    if (!layer_loader.layers_loaded_) layer_loader.LoadLayers();

    return layer_loader;
}

const char kSystemLayerLibraryDir[] = "/data/local/debug/gles";

std::string LayerLoader::GetDebugLayers() {
    // Layers can be specified at the Java level in GraphicsEnvironemnt
    // gpu_debug_layers_gles = layer1:layer2:layerN
    std::string debug_layers = android::GraphicsEnv::getInstance().getDebugLayersGLES();

    if (debug_layers.empty()) {
        // Only check system properties if Java settings are empty
        char prop[PROPERTY_VALUE_MAX];
        property_get("debug.gles.layers", prop, "");
        debug_layers = prop;
    }

    return debug_layers;
}

EGLFuncPointer LayerLoader::ApplyLayer(layer_setup_func layer_setup, const char* name,
                                       EGLFuncPointer next) {
    // Walk through our list of LayerSetup functions (they will already be in reverse order) to
    // build up a call chain from the driver

    EGLFuncPointer layer_entry = next;

    layer_entry = layer_setup(name, layer_entry);

    if (next != layer_entry) {
        ALOGV("We succeeded, replacing hook (%llu) with layer entry (%llu), for %s",
              (unsigned long long)next, (unsigned long long)layer_entry, name);
    }

    return layer_entry;
}

EGLFuncPointer LayerLoader::ApplyLayers(const char* name, EGLFuncPointer next) {
    if (!layers_loaded_ || layer_setup_.empty()) return next;

    ALOGV("ApplyLayers called for %s with next (%llu), current_layer_ (%i)", name,
          (unsigned long long)next, current_layer_);

    EGLFuncPointer val = next;

    // Only ApplyLayers for layers that have been setup, not all layers yet
    for (unsigned i = 0; i < current_layer_; i++) {
        ALOGV("ApplyLayers: Calling ApplyLayer with i = %i for %s with next (%llu)", i, name,
              (unsigned long long)next);
        val = ApplyLayer(layer_setup_[i], name, val);
    }

    ALOGV("ApplyLayers returning %llu for %s", (unsigned long long)val, name);

    return val;
}

void LayerLoader::LayerPlatformEntries(layer_setup_func layer_setup, EGLFuncPointer* curr,
                                       char const* const* entries) {
    while (*entries) {
        char const* name = *entries;

        EGLFuncPointer prev = *curr;

        // Pass the existing entry point into the layer, replace the call with return value
        *curr = ApplyLayer(layer_setup, name, *curr);

        if (prev != *curr) {
            ALOGV("LayerPlatformEntries: Replaced (%llu) with platform entry (%llu), for %s",
                  (unsigned long long)prev, (unsigned long long)*curr, name);
        } else {
            ALOGV("LayerPlatformEntries: No change(%llu) for %s, which means layer did not "
                  "intercept",
                  (unsigned long long)prev, name);
        }

        curr++;
        entries++;
    }
}

void LayerLoader::LayerDriverEntries(layer_setup_func layer_setup, EGLFuncPointer* curr,
                                     char const* const* entries) {
    while (*entries) {
        char const* name = *entries;
        EGLFuncPointer prev = *curr;

        // Only apply layers to driver entries if not handled by the platform
        if (FindPlatformImplAddr(name) == nullptr) {
            // Pass the existing entry point into the layer, replace the call with return value
            *curr = ApplyLayer(layer_setup, name, *prev);

            if (prev != *curr) {
                ALOGV("LayerDriverEntries: Replaced (%llu) with platform entry (%llu), for %s",
                      (unsigned long long)prev, (unsigned long long)*curr, name);
            }

        } else {
            ALOGV("LayerDriverEntries: Skipped (%llu) for %s", (unsigned long long)prev, name);
        }

        curr++;
        entries++;
    }
}

bool LayerLoader::Initialized() {
    return initialized_;
}

void LayerLoader::InitLayers(egl_connection_t* cnx) {
    if (!layers_loaded_) return;

    if (initialized_) return;

    if (layer_setup_.empty()) {
        initialized_ = true;
        return;
    }

    // Include the driver in layer_functions
    layer_functions.resize(layer_setup_.size() + 1);

    // Walk through the initial lists and create layer_functions[0]
    int func_idx = 0;
    char const* const* entries;
    EGLFuncPointer* curr;

    entries = platform_names;
    curr = reinterpret_cast<EGLFuncPointer*>(&cnx->platform);
    SetupFuncMaps(layer_functions[0], entries, curr, func_idx);
    ALOGV("InitLayers: func_idx after platform_names: %i", func_idx);

    entries = egl_names;
    curr = reinterpret_cast<EGLFuncPointer*>(&cnx->egl);
    SetupFuncMaps(layer_functions[0], entries, curr, func_idx);
    ALOGV("InitLayers: func_idx after egl_names: %i", func_idx);

    entries = gl_names;
    curr = reinterpret_cast<EGLFuncPointer*>(&cnx->hooks[egl_connection_t::GLESv2_INDEX]->gl);
    SetupFuncMaps(layer_functions[0], entries, curr, func_idx);
    ALOGV("InitLayers: func_idx after gl_names: %i", func_idx);

    // Walk through each layer's entry points per API, starting just above the driver
    for (current_layer_ = 0; current_layer_ < layer_setup_.size(); current_layer_++) {
        // Init the layer with a key that points to layer just below it
        layer_init_[current_layer_](reinterpret_cast<void*>(&layer_functions[current_layer_]),
                                    reinterpret_cast<PFNEGLGETNEXTLAYERPROCADDRESSPROC>(
                                            getNextLayerProcAddress));

        // Check functions implemented by the platform
        func_idx = 0;
        entries = platform_names;
        curr = reinterpret_cast<EGLFuncPointer*>(&cnx->platform);
        LayerPlatformEntries(layer_setup_[current_layer_], curr, entries);

        // Populate next function table after layers have been applied
        SetupFuncMaps(layer_functions[current_layer_ + 1], entries, curr, func_idx);

        // EGL
        entries = egl_names;
        curr = reinterpret_cast<EGLFuncPointer*>(&cnx->egl);
        LayerDriverEntries(layer_setup_[current_layer_], curr, entries);

        // Populate next function table after layers have been applied
        SetupFuncMaps(layer_functions[current_layer_ + 1], entries, curr, func_idx);

        // GLES 2+
        // NOTE: We route calls to GLESv2 hooks, not GLESv1, so layering does not support GLES 1.x
        // If it were added in the future, a different layer initialization model would be needed,
        // that defers loading GLES entrypoints until after eglMakeCurrent, so two phase
        // initialization.
        entries = gl_names;
        curr = reinterpret_cast<EGLFuncPointer*>(&cnx->hooks[egl_connection_t::GLESv2_INDEX]->gl);
        LayerDriverEntries(layer_setup_[current_layer_], curr, entries);

        // Populate next function table after layers have been applied
        SetupFuncMaps(layer_functions[current_layer_ + 1], entries, curr, func_idx);
    }

    // We only want to apply layers once
    initialized_ = true;
}

void LayerLoader::LoadLayers() {
    std::string debug_layers = GetDebugLayers();

    // If no layers are specified, we're done
    if (debug_layers.empty()) return;

    // Only enable the system search path for non-user builds
    std::string system_path;
    if (android::GraphicsEnv::getInstance().isDebuggable()) {
        system_path = kSystemLayerLibraryDir;
    }

    ALOGI("Debug layer list: %s", debug_layers.c_str());
    std::vector<std::string> layers = android::base::Split(debug_layers, ":");

    // Load the layers in reverse order so we start with the driver's entrypoint and work our way up
    for (int32_t i = layers.size() - 1; i >= 0; i--) {
        // Check each layer path for the layer
        std::vector<std::string> paths =
                android::base::Split(android::GraphicsEnv::getInstance().getLayerPaths().c_str(),
                                     ":");

        if (!system_path.empty()) {
            // Prepend the system paths so they override other layers
            auto it = paths.begin();
            paths.insert(it, system_path);
        }

        bool layer_found = false;
        for (uint32_t j = 0; j < paths.size() && !layer_found; j++) {
            std::string layer;

            ALOGI("Searching %s for GLES layers", paths[j].c_str());

            // Realpath will return null for non-existent files
            android::base::Realpath(paths[j] + "/" + layers[i], &layer);

            if (!layer.empty()) {
                layer_found = true;
                ALOGI("GLES layer found: %s", layer.c_str());

                // Load the layer
                //
                // TODO: This code is common with Vulkan loader, refactor
                //
                // Libraries in the system layer library dir can't be loaded into
                // the application namespace. That causes compatibility problems, since
                // any symbol dependencies will be resolved by system libraries. They
                // can't safely use libc++_shared, for example. Which is one reason
                // (among several) we only allow them in non-user builds.
                void* handle = nullptr;
                auto app_namespace = android::GraphicsEnv::getInstance().getAppNamespace();
                if (app_namespace && !android::base::StartsWith(layer, kSystemLayerLibraryDir)) {
                    bool native_bridge = false;
                    char* error_message = nullptr;
                    handle = OpenNativeLibraryInNamespace(
                        app_namespace, layer.c_str(), &native_bridge, &error_message);
                    if (!handle) {
                        ALOGE("Failed to load layer %s with error: %s", layer.c_str(),
                              error_message);
                        android::NativeLoaderFreeErrorMessage(error_message);
                        return;
                    }

                } else {
                    handle = dlopen(layer.c_str(), RTLD_NOW | RTLD_LOCAL);
                }

                if (handle) {
                    ALOGV("Loaded layer handle (%llu) for layer %s", (unsigned long long)handle,
                          layers[i].c_str());
                } else {
                    // If the layer is found but can't be loaded, try setenforce 0
                    const char* dlsym_error = dlerror();
                    ALOGE("Failed to load layer %s with error: %s", layer.c_str(), dlsym_error);
                    return;
                }

                // Find the layer's Initialize function
                std::string init_func = "AndroidGLESLayer_Initialize";
                ALOGV("Looking for entrypoint %s", init_func.c_str());

                layer_init_func LayerInit =
                        reinterpret_cast<layer_init_func>(dlsym(handle, init_func.c_str()));
                if (LayerInit) {
                    ALOGV("Found %s for layer %s", init_func.c_str(), layer.c_str());
                    layer_init_.push_back(LayerInit);
                } else {
                    ALOGE("Failed to dlsym %s for layer %s", init_func.c_str(), layer.c_str());
                    return;
                }

                // Find the layer's setup function
                std::string setup_func = "AndroidGLESLayer_GetProcAddress";
                ALOGV("Looking for entrypoint %s", setup_func.c_str());

                layer_setup_func LayerSetup =
                        reinterpret_cast<layer_setup_func>(dlsym(handle, setup_func.c_str()));
                if (LayerSetup) {
                    ALOGV("Found %s for layer %s", setup_func.c_str(), layer.c_str());
                    layer_setup_.push_back(LayerSetup);
                } else {
                    ALOGE("Failed to dlsym %s for layer %s", setup_func.c_str(), layer.c_str());
                    return;
                }
            }
        }
    }
    // Track this so we only attempt to load these once
    layers_loaded_ = true;
}

} // namespace android
