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
#define LOG_TAG "C2PlatformStorePluginLoader"

#include <dlfcn.h>

#include <utils/Log.h>

#include "C2PlatformStorePluginLoader.h"

/* static */ android::Mutex C2PlatformStorePluginLoader::sMutex;
/* static */ std::unique_ptr<C2PlatformStorePluginLoader> C2PlatformStorePluginLoader::sInstance;

namespace /* unnamed */ {

constexpr const char kStorePluginPath[] = "libc2plugin_store.so";

}  // unnamed

C2PlatformStorePluginLoader::C2PlatformStorePluginLoader(const char *libPath)
    : mCreateBlockPool(nullptr) {
    mLibHandle = dlopen(libPath, RTLD_NOW | RTLD_NODELETE);
    if (mLibHandle == nullptr) {
        ALOGD("Failed to load library: %s (%s)", libPath, dlerror());
        return;
    }
    mCreateBlockPool = (CreateBlockPoolFunc)dlsym(mLibHandle, "CreateBlockPool");
    if (mCreateBlockPool == nullptr) {
        ALOGD("Failed to find symbol: CreateBlockPool (%s)", dlerror());
    }
    mCreateAllocator = (CreateAllocatorFunc)dlsym(mLibHandle, "CreateAllocator");
    if (mCreateAllocator == nullptr) {
        ALOGD("Failed to find symbol: CreateAllocator (%s)", dlerror());
    }
}

C2PlatformStorePluginLoader::~C2PlatformStorePluginLoader() {
    if (mLibHandle != nullptr) {
        ALOGV("Closing handle");
        dlclose(mLibHandle);
    }
}

c2_status_t C2PlatformStorePluginLoader::createBlockPool(
        ::C2Allocator::id_t allocatorId, ::C2BlockPool::local_id_t blockPoolId,
        std::shared_ptr<C2BlockPool>* pool) {
    if (mCreateBlockPool == nullptr) {
        ALOGD("Handle or CreateBlockPool symbol is null");
        return C2_NOT_FOUND;
    }

    std::shared_ptr<::C2BlockPool> ptr(mCreateBlockPool(allocatorId, blockPoolId));
    if (ptr) {
        *pool = ptr;
        return C2_OK;
    }
    ALOGD("Failed to CreateBlockPool by allocator id: %u", allocatorId);
    return C2_BAD_INDEX;
}

c2_status_t C2PlatformStorePluginLoader::createAllocator(
        ::C2Allocator::id_t allocatorId, std::shared_ptr<C2Allocator>* const allocator) {
    if (mCreateAllocator == nullptr) {
        ALOGD("Handle or CreateAllocator symbol is null");
        return C2_NOT_FOUND;
    }

    c2_status_t res = C2_CORRUPTED;
    allocator->reset(mCreateAllocator(allocatorId, &res));
    if (res != C2_OK) {
        ALOGD("Failed to CreateAllocator by id: %u, res: %d", allocatorId, res);
        allocator->reset();
        return res;
    }
    return C2_OK;
}

// static
const std::unique_ptr<C2PlatformStorePluginLoader>& C2PlatformStorePluginLoader::GetInstance() {
    android::Mutex::Autolock _l(sMutex);
    if (!sInstance) {
        ALOGV("Loading library");
        sInstance.reset(new C2PlatformStorePluginLoader(kStorePluginPath));
    }
    return sInstance;
}
