/*
 * Copyright 2018, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "StagefrightPluginLoader"
#include <utils/Log.h>

#include <android-base/properties.h>
#include <dlfcn.h>

#include "StagefrightPluginLoader.h"

namespace android {

/* static */ Mutex StagefrightPluginLoader::sMutex;
/* static */ std::unique_ptr<StagefrightPluginLoader> StagefrightPluginLoader::sInstance;

namespace /* unnamed */ {

constexpr const char kCCodecPluginPath[] = "libsfplugin_ccodec.so";

}  // unnamed namespace

StagefrightPluginLoader::StagefrightPluginLoader(const char *libPath) {
    if (android::base::GetIntProperty("debug.stagefright.ccodec", 1) == 0) {
        ALOGD("CCodec is disabled.");
        return;
    }
    mLibHandle = dlopen(libPath, RTLD_NOW | RTLD_NODELETE);
    if (mLibHandle == nullptr) {
        ALOGD("Failed to load library: %s (%s)", libPath, dlerror());
        return;
    }
    mCreateCodec = (CodecBase::CreateCodecFunc)dlsym(mLibHandle, "CreateCodec");
    if (mCreateCodec == nullptr) {
        ALOGD("Failed to find symbol: CreateCodec (%s)", dlerror());
    }
    mCreateBuilder = (MediaCodecListBuilderBase::CreateBuilderFunc)dlsym(
            mLibHandle, "CreateBuilder");
    if (mCreateBuilder == nullptr) {
        ALOGD("Failed to find symbol: CreateBuilder (%s)", dlerror());
    }
    mCreateInputSurface = (CodecBase::CreateInputSurfaceFunc)dlsym(
            mLibHandle, "CreateInputSurface");
    if (mCreateInputSurface == nullptr) {
        ALOGD("Failed to find symbol: CreateInputSurface (%s)", dlerror());
    }
}

StagefrightPluginLoader::~StagefrightPluginLoader() {
    if (mLibHandle != nullptr) {
        ALOGV("Closing handle");
        dlclose(mLibHandle);
    }
}

CodecBase *StagefrightPluginLoader::createCodec() {
    if (mLibHandle == nullptr || mCreateCodec == nullptr) {
        ALOGD("Handle or CreateCodec symbol is null");
        return nullptr;
    }
    return mCreateCodec();
}

MediaCodecListBuilderBase *StagefrightPluginLoader::createBuilder() {
    if (mLibHandle == nullptr || mCreateBuilder == nullptr) {
        ALOGD("Handle or CreateBuilder symbol is null");
        return nullptr;
    }
    return mCreateBuilder();
}

PersistentSurface *StagefrightPluginLoader::createInputSurface() {
    if (mLibHandle == nullptr || mCreateInputSurface == nullptr) {
        ALOGD("Handle or CreateInputSurface symbol is null");
        return nullptr;
    }
    return mCreateInputSurface();
}

//static
const std::unique_ptr<StagefrightPluginLoader> &StagefrightPluginLoader::GetCCodecInstance() {
    Mutex::Autolock _l(sMutex);
    if (!sInstance) {
        ALOGV("Loading library");
        sInstance.reset(new StagefrightPluginLoader(kCCodecPluginPath));
    }
    return sInstance;
}

}  // namespace android
