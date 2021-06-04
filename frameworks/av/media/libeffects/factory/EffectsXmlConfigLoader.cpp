/*
 * Copyright (C) 2017 The Android Open Source Project
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

#define LOG_TAG "EffectsFactoryConfigLoader"
//#define LOG_NDEBUG 0

#include <dlfcn.h>
#include <set>
#include <stdlib.h>
#include <string>

#include <log/log.h>

#include <media/EffectsConfig.h>

#include "EffectsConfigLoader.h"
#include "EffectsFactoryState.h"
#include "EffectsXmlConfigLoader.h"

namespace android {

using namespace effectsConfig;

/////////////////////////////////////////////////
//      Local functions
/////////////////////////////////////////////////

namespace {

/** Similarly to dlopen, looks for the provided path in LD_EFFECT_LIBRARY_PATH.
 * @return true if the library is found and set resolvedPath to its absolute path.
 *         false if not found
 */
bool resolveLibrary(const std::string& path, std::string* resolvedPath) {
    for (auto* libraryDirectory : LD_EFFECT_LIBRARY_PATH) {
        std::string candidatePath = std::string(libraryDirectory) + '/' + path;
        if (access(candidatePath.c_str(), R_OK) == 0) {
            *resolvedPath = std::move(candidatePath);
            return true;
        }
    }
    return false;
}

/** Loads a library given its relative path and stores the result in libEntry.
 * @return true on success with libEntry's path, handle and desc filled
 *         false on success with libEntry's path filled with the path of the failed lib
 * The caller MUST free the resources path (free) and handle (dlclose) if filled.
 */
bool loadLibrary(const char* relativePath, lib_entry_t* libEntry) noexcept {

    std::string absolutePath;
    if (!resolveLibrary(relativePath, &absolutePath)) {
        ALOGE("Could not find library in effect directories: %s", relativePath);
        libEntry->path = strdup(relativePath);
        return false;
    }
    const char* path = absolutePath.c_str();
    libEntry->path = strdup(path);

    // Make sure the lib is closed on early return
    std::unique_ptr<void, decltype(dlclose)*> libHandle(dlopen(path, RTLD_NOW),
                                                       dlclose);
    if (libHandle == nullptr) {
        ALOGE("Could not dlopen library %s: %s", path, dlerror());
        return false;
    }

    auto* description = static_cast<audio_effect_library_t*>(
          dlsym(libHandle.get(), AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR));
    if (description == nullptr) {
        ALOGE("Invalid effect library, failed not find symbol '%s' in %s: %s",
              AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR, path, dlerror());
        return false;
    }

    if (description->tag != AUDIO_EFFECT_LIBRARY_TAG) {
        ALOGE("Bad tag %#08x in description structure, expected %#08x for library %s",
              description->tag, AUDIO_EFFECT_LIBRARY_TAG, path);
        return false;
    }

    uint32_t majorVersion = EFFECT_API_VERSION_MAJOR(description->version);
    uint32_t expectedMajorVersion = EFFECT_API_VERSION_MAJOR(EFFECT_LIBRARY_API_VERSION);
    if (majorVersion != expectedMajorVersion) {
        ALOGE("Unsupported major version %#08x, expected %#08x for library %s",
              majorVersion, expectedMajorVersion, path);
        return false;
    }

    libEntry->handle = libHandle.release();
    libEntry->desc = description;
    return true;
}

/** Because the structures will be destroyed by c code, using new to allocate shared structure
 * is not possible. Provide a equivalent of unique_ptr for malloc/freed structure to make sure
 * they are not leaked in the c++ code.
 @{ */
struct FreeDeleter {
    void operator()(void* p) {
        free(p);
    }
};
/** unique_ptr for object created with malloc. */
template <class T>
using UniqueCPtr = std::unique_ptr<T, FreeDeleter>;

/** c version of std::make_unique. Uses malloc and free. */
template <class T>
UniqueCPtr<T> makeUniqueC() {
    T* ptr = new (malloc(sizeof(T))) T{}; // Use placement new to initialize the structure
    return UniqueCPtr<T>{ptr};
}

/** @} */

/** Push an not owned element in a list_elem link list with an optional lock. */
template <class T, class ListElem>
void listPush(T* object, ListElem** list, pthread_mutex_t* mutex = nullptr) noexcept {
    auto listElem = makeUniqueC<ListElem>();
    listElem->object = object;
    if (mutex != nullptr) {
        pthread_mutex_lock(mutex);
    }
    listElem->next = *list;
    *list = listElem.release();
    if (mutex != nullptr) {
        pthread_mutex_unlock(mutex);
    }
}

/** Push an owned element in a list_elem link list with an optional lock. */
template <class T, class ListElem>
void listPush(UniqueCPtr<T>&& object, ListElem** list, pthread_mutex_t* mutex = nullptr) noexcept {
    listPush(object.release(), list, mutex);
}

size_t loadLibraries(const effectsConfig::Libraries& libs,
                     list_elem_t** libList, pthread_mutex_t* libListLock,
                     list_elem_t** libFailedList)
{
    size_t nbSkippedElement = 0;
    for (auto& library : libs) {

        // Construct a lib entry
        auto libEntry = makeUniqueC<lib_entry_t>();
        libEntry->name = strdup(library.name.c_str());
        libEntry->effects = nullptr;
        pthread_mutex_init(&libEntry->lock, nullptr);

        if (!loadLibrary(library.path.c_str(), libEntry.get())) {
            // Register library load failure
            listPush(std::move(libEntry), libFailedList);
            ++nbSkippedElement;
            continue;
        }
        listPush(std::move(libEntry), libList, libListLock);
    }
    return nbSkippedElement;
}

/** Find a library with the given name in the given list. */
lib_entry_t* findLibrary(const char* name, list_elem_t* list) {

    while (list != nullptr) {
        auto* object = static_cast<lib_entry_t*>(list->object);
        if (strcmp(object->name, name) == 0) {
            return object;
        }
        list = list->next;
    }
    return nullptr;
}

struct UuidStr {
    /** Length of an uuid represented as string. @TODO: use a constant instead of 40. */
    char buff[40];
};

/** @return a string representing the provided uuid.
 * By not providing an output buffer, it is implicitly created in the caller context.
 * In such case the return pointer has the same lifetime as the expression containing uuidToString()
 */
char* uuidToString(const effect_uuid_t& uuid, UuidStr&& str = {}) {
    uuidToString(&uuid, str.buff, sizeof(str.buff));
    return str.buff;
}

struct LoadEffectResult {
    /** true if the effect is usable (aka, existing lib, desc, right version, unique uuid) */
    bool success = false;
    /** Set if the effect lib was found*/
    lib_entry_t* lib = nullptr;
    //* Set if the description was successfuly retrieved from the lib */
    UniqueCPtr<effect_descriptor_t> effectDesc;
};

LoadEffectResult loadEffect(const EffectImpl& effect, const std::string& name,
                            list_elem_t* libList) {
    LoadEffectResult result;

    // Find the effect library
    result.lib = findLibrary(effect.library->name.c_str(), libList);
    if (result.lib == nullptr) {
        ALOGE("Could not find library %s to load effect %s",
              effect.library->name.c_str(), name.c_str());
        return result;
    }

    result.effectDesc = makeUniqueC<effect_descriptor_t>();

    // Get the effect descriptor
    if (result.lib->desc->get_descriptor(&effect.uuid, result.effectDesc.get()) != 0) {
        ALOGE("Error querying effect %s on lib %s",
              uuidToString(effect.uuid), result.lib->name);
        result.effectDesc.reset();
        return result;
    }

    // Dump effect for debug
#if (LOG_NDEBUG==0)
    char s[512];
    dumpEffectDescriptor(result.effectDesc.get(), s, sizeof(s), 0 /* indent */);
    ALOGV("loadEffect() read descriptor %p:%s", result.effectDesc.get(), s);
#endif

    // Check effect is supported
    uint32_t expectedMajorVersion = EFFECT_API_VERSION_MAJOR(EFFECT_CONTROL_API_VERSION);
    if (EFFECT_API_VERSION_MAJOR(result.effectDesc->apiVersion) != expectedMajorVersion) {
        ALOGE("Bad API version %#08x for effect %s in lib %s, expected major %#08x",
              result.effectDesc->apiVersion, name.c_str(), result.lib->name, expectedMajorVersion);
        return result;
    }

    lib_entry_t *_;
    if (findEffect(nullptr, &effect.uuid, &_, nullptr) == 0) {
        ALOGE("Effect %s uuid %s already exist", uuidToString(effect.uuid), name.c_str());
        return result;
    }

    result.success = true;
    return result;
}

size_t loadEffects(const Effects& effects, list_elem_t* libList, list_elem_t** skippedEffects,
                   list_sub_elem_t** subEffectList) {
    size_t nbSkippedElement = 0;

    for (auto& effect : effects) {

        auto effectLoadResult = loadEffect(effect, effect.name, libList);
        if (!effectLoadResult.success) {
            if (effectLoadResult.effectDesc != nullptr) {
                listPush(std::move(effectLoadResult.effectDesc), skippedEffects);
            }
            ++nbSkippedElement;
            continue;
        }

        if (effect.isProxy) {
            auto swEffectLoadResult = loadEffect(effect.libSw, effect.name + " libsw", libList);
            auto hwEffectLoadResult = loadEffect(effect.libHw, effect.name + " libhw", libList);
            if (!swEffectLoadResult.success || !hwEffectLoadResult.success) {
                // Push the main effect in the skipped list even if only a subeffect is invalid
                // as the main effect is not usable without its subeffects.
                listPush(std::move(effectLoadResult.effectDesc), skippedEffects);
                ++nbSkippedElement;
                continue;
            }
            listPush(effectLoadResult.effectDesc.get(), subEffectList);

            // Since we return a dummy descriptor for the proxy during
            // get_descriptor call, we replace it with the corresponding
            // sw effect descriptor, but keep the Proxy UUID
            *effectLoadResult.effectDesc = *swEffectLoadResult.effectDesc;
            effectLoadResult.effectDesc->uuid = effect.uuid;

            effectLoadResult.effectDesc->flags |= EFFECT_FLAG_OFFLOAD_SUPPORTED;

            auto registerSubEffect = [subEffectList](auto&& result) {
                auto entry = makeUniqueC<sub_effect_entry_t>();
                entry->object = result.effectDesc.release();
                // lib_entry_t is stored since the sub effects are not linked to the library
                entry->lib = result.lib;
                listPush(std::move(entry), &(*subEffectList)->sub_elem);
            };
            registerSubEffect(std::move(swEffectLoadResult));
            registerSubEffect(std::move(hwEffectLoadResult));
        }

        listPush(std::move(effectLoadResult.effectDesc), &effectLoadResult.lib->effects);
    }
    return nbSkippedElement;
}

} // namespace

/////////////////////////////////////////////////
//      Interface function
/////////////////////////////////////////////////

extern "C" ssize_t EffectLoadXmlEffectConfig(const char* path)
{
    using effectsConfig::parse;
    auto result = path ? parse(path) : parse();
    if (result.parsedConfig == nullptr) {
        ALOGE("Failed to parse XML configuration file");
        return -1;
    }
    result.nbSkippedElement += loadLibraries(result.parsedConfig->libraries,
                                             &gLibraryList, &gLibLock, &gLibraryFailedList) +
                               loadEffects(result.parsedConfig->effects, gLibraryList,
                                           &gSkippedEffects, &gSubEffectList);

    ALOGE_IF(result.nbSkippedElement != 0, "%zu errors during loading of configuration: %s",
             result.nbSkippedElement,
             result.configPath.empty() ? "No config file found" : result.configPath.c_str());

    return result.nbSkippedElement;
}

} // namespace android
