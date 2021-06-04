/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef ANDROID_EGL_LAYERS_H
#define ANDROID_EGL_LAYERS_H

#include <string>
#include <unordered_map>
#include <vector>

#include <EGL/egldefs.h>

#include "egl_platform_entries.h"

typedef __eglMustCastToProperFunctionPointerType EGLFuncPointer;

namespace android {

class LayerLoader {
public:
    static LayerLoader& getInstance();
    ~LayerLoader(){};

    typedef void* (*PFNEGLGETNEXTLAYERPROCADDRESSPROC)(void*, const char*);
    typedef EGLFuncPointer (*layer_init_func)(
            const void* layer_id, PFNEGLGETNEXTLAYERPROCADDRESSPROC get_next_layer_proc_address);
    typedef EGLFuncPointer (*layer_setup_func)(const char* name, EGLFuncPointer next);

    void LoadLayers();
    void InitLayers(egl_connection_t*);
    void LayerPlatformEntries(layer_setup_func layer_setup, EGLFuncPointer*, char const* const*);
    void LayerDriverEntries(layer_setup_func layer_setup, EGLFuncPointer*, char const* const*);
    bool Initialized();
    std::string GetDebugLayers();

    EGLFuncPointer GetGpaNext(unsigned i);
    EGLFuncPointer ApplyLayer(layer_setup_func layer_setup, const char* name, EGLFuncPointer next);
    EGLFuncPointer ApplyLayers(const char* name, EGLFuncPointer next);

    std::vector<layer_init_func> layer_init_;
    std::vector<layer_setup_func> layer_setup_;

private:
    LayerLoader() : layers_loaded_(false), initialized_(false), current_layer_(0){};
    bool layers_loaded_;
    bool initialized_;
    unsigned current_layer_;
};

}; // namespace android

#endif // ANDROID_EGL_LAYERS_H
