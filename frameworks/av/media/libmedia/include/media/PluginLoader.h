/**
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef PLUGIN_LOADER_H_
#define PLUGIN_LOADER_H_

#include "SharedLibrary.h"
#include <utils/Log.h>
#include <utils/String8.h>
#include <utils/Vector.h>

namespace android {

template <class T>
class PluginLoader {

  public:
    PluginLoader(const char *dir, const char *entry) {
        /**
         * scan all plugins in the plugin directory and add them to the
         * factories list.
         */
        String8 pluginDir(dir);

        DIR* pDir = opendir(pluginDir.string());
        if (pDir == NULL) {
            ALOGE("Failed to find plugin directory %s", pluginDir.string());
        } else {
            struct dirent* pEntry;
            while ((pEntry = readdir(pDir))) {
                String8 file(pEntry->d_name);
                if (file.getPathExtension() == ".so") {
                    String8 path = pluginDir + "/" + pEntry->d_name;
                    T *plugin = loadOne(path, entry);
                    if (plugin) {
                        factories.push(plugin);
                    }
                }
            }
            closedir(pDir);
        }
    }

    ~PluginLoader() {
        for (size_t i = 0; i < factories.size(); i++) {
            delete factories[i];
        }
    }

    T *getFactory(size_t i) const {
        return factories[i];
    }

    size_t factoryCount() const {return factories.size();}

  private:
    T* loadOne(const char *path, const char *entry) {
        sp<SharedLibrary> library = new SharedLibrary(String8(path));
        if (!library.get()) {
            ALOGE("Failed to open plugin library %s: %s", path,
                    library->lastError());
        } else {
            typedef T *(*CreateFactoryFunc)();
            CreateFactoryFunc createFactoryFunc =
                    (CreateFactoryFunc)library->lookup(entry);
            if (createFactoryFunc) {
                ALOGV("Found plugin factory entry %s in %s", entry, path);
                libraries.push(library);
                T* result = createFactoryFunc();
                return  result;
           }
        }
        return NULL;
    }

    Vector<T *> factories;
    Vector<sp<SharedLibrary> > libraries;

    PluginLoader(const PluginLoader &) = delete;
    void operator=(const PluginLoader &) = delete;
};

} // namespace android

#endif // PLUGIN_LOADER_H_

