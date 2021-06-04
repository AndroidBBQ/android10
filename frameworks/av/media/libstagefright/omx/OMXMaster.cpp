/*
 * Copyright (C) 2009 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "OMXMaster"
#include <utils/Log.h>

#include <media/stagefright/omx/OMXMaster.h>
#include <media/stagefright/omx/SoftOMXPlugin.h>
#include <media/stagefright/foundation/ADebug.h>

#include <vndksupport/linker.h>

#include <dlfcn.h>
#include <fcntl.h>

namespace android {

OMXMaster::OMXMaster() {

    pid_t pid = getpid();
    char filename[20];
    snprintf(filename, sizeof(filename), "/proc/%d/comm", pid);
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
      ALOGW("couldn't determine process name");
      strlcpy(mProcessName, "<unknown>", sizeof(mProcessName));
    } else {
      ssize_t len = read(fd, mProcessName, sizeof(mProcessName));
      if (len < 2) {
        ALOGW("couldn't determine process name");
        strlcpy(mProcessName, "<unknown>", sizeof(mProcessName));
      } else {
        // the name is newline terminated, so erase the newline
        mProcessName[len - 1] = 0;
      }
      close(fd);
    }

    addVendorPlugin();
    addPlatformPlugin();
}

OMXMaster::~OMXMaster() {
    clearPlugins();
}

void OMXMaster::addVendorPlugin() {
    addPlugin("libstagefrighthw.so");
}

void OMXMaster::addPlatformPlugin() {
    addPlugin("libstagefright_softomx_plugin.so");
}

void OMXMaster::addPlugin(const char *libname) {
    void *libHandle = android_load_sphal_library(libname, RTLD_NOW);

    if (libHandle == NULL) {
        return;
    }

    typedef OMXPluginBase *(*CreateOMXPluginFunc)();
    CreateOMXPluginFunc createOMXPlugin =
        (CreateOMXPluginFunc)dlsym(
                libHandle, "createOMXPlugin");
    if (!createOMXPlugin)
        createOMXPlugin = (CreateOMXPluginFunc)dlsym(
                libHandle, "_ZN7android15createOMXPluginEv");

    OMXPluginBase *plugin = nullptr;
    if (createOMXPlugin) {
        plugin = (*createOMXPlugin)();
    }

    if (plugin) {
        mPlugins.push_back({ plugin, libHandle });
        addPlugin(plugin);
    } else {
        android_unload_sphal_library(libHandle);
    }
}

void OMXMaster::addPlugin(OMXPluginBase *plugin) {
    Mutex::Autolock autoLock(mLock);

    OMX_U32 index = 0;

    char name[128];
    OMX_ERRORTYPE err;
    while ((err = plugin->enumerateComponents(
                    name, sizeof(name), index++)) == OMX_ErrorNone) {
        String8 name8(name);

        if (mPluginByComponentName.indexOfKey(name8) >= 0) {
            ALOGE("A component of name '%s' already exists, ignoring this one.",
                 name8.string());

            continue;
        }

        mPluginByComponentName.add(name8, plugin);
    }

    if (err != OMX_ErrorNoMore) {
        ALOGE("OMX plugin failed w/ error 0x%08x after registering %zu "
             "components", err, mPluginByComponentName.size());
    }
}

void OMXMaster::clearPlugins() {
    Mutex::Autolock autoLock(mLock);

    mPluginByComponentName.clear();
    mPluginByInstance.clear();

    typedef void (*DestroyOMXPluginFunc)(OMXPluginBase*);
    for (const Plugin &plugin : mPlugins) {
        DestroyOMXPluginFunc destroyOMXPlugin =
            (DestroyOMXPluginFunc)dlsym(
                    plugin.mLibHandle, "destroyOMXPlugin");
        if (destroyOMXPlugin)
            destroyOMXPlugin(plugin.mOmx);
        else
            delete plugin.mOmx;

        android_unload_sphal_library(plugin.mLibHandle);
    }

    mPlugins.clear();
}

OMX_ERRORTYPE OMXMaster::makeComponentInstance(
        const char *name,
        const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData,
        OMX_COMPONENTTYPE **component) {
    ALOGI("makeComponentInstance(%s) in %s process", name, mProcessName);
    Mutex::Autolock autoLock(mLock);

    *component = NULL;

    ssize_t index = mPluginByComponentName.indexOfKey(String8(name));

    if (index < 0) {
        return OMX_ErrorInvalidComponentName;
    }

    OMXPluginBase *plugin = mPluginByComponentName.valueAt(index);
    OMX_ERRORTYPE err =
        plugin->makeComponentInstance(name, callbacks, appData, component);

    if (err != OMX_ErrorNone) {
        return err;
    }

    mPluginByInstance.add(*component, plugin);

    return err;
}

OMX_ERRORTYPE OMXMaster::destroyComponentInstance(
        OMX_COMPONENTTYPE *component) {
    Mutex::Autolock autoLock(mLock);

    ssize_t index = mPluginByInstance.indexOfKey(component);

    if (index < 0) {
        return OMX_ErrorBadParameter;
    }

    OMXPluginBase *plugin = mPluginByInstance.valueAt(index);
    mPluginByInstance.removeItemsAt(index);

    return plugin->destroyComponentInstance(component);
}

OMX_ERRORTYPE OMXMaster::enumerateComponents(
        OMX_STRING name,
        size_t size,
        OMX_U32 index) {
    Mutex::Autolock autoLock(mLock);

    size_t numComponents = mPluginByComponentName.size();

    if (index >= numComponents) {
        return OMX_ErrorNoMore;
    }

    const String8 &name8 = mPluginByComponentName.keyAt(index);

    CHECK(size >= 1 + name8.size());
    strcpy(name, name8.string());

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OMXMaster::getRolesOfComponent(
        const char *name,
        Vector<String8> *roles) {
    Mutex::Autolock autoLock(mLock);

    roles->clear();

    ssize_t index = mPluginByComponentName.indexOfKey(String8(name));

    if (index < 0) {
        return OMX_ErrorInvalidComponentName;
    }

    OMXPluginBase *plugin = mPluginByComponentName.valueAt(index);
    return plugin->getRolesOfComponent(name, roles);
}

}  // namespace android
