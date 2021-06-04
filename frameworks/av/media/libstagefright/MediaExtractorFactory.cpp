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

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaExtractorFactory"
#include <utils/Log.h>

#include <android/dlext.h>
#include <android-base/logging.h>
#include <binder/IPCThreadState.h>
#include <binder/PermissionCache.h>
#include <binder/IServiceManager.h>
#include <media/DataSource.h>
#include <media/stagefright/InterfaceUtils.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/MediaExtractorFactory.h>
#include <media/IMediaExtractor.h>
#include <media/IMediaExtractorService.h>
#include <nativeloader/dlext_namespaces.h>
#include <private/android_filesystem_config.h>
#include <cutils/properties.h>
#include <utils/String8.h>

#include <dirent.h>
#include <dlfcn.h>

namespace android {

// static
sp<IMediaExtractor> MediaExtractorFactory::Create(
        const sp<DataSource> &source, const char *mime) {
    ALOGV("MediaExtractorFactory::Create %s", mime);

    if (!property_get_bool("media.stagefright.extractremote", true)) {
        // local extractor
        ALOGW("creating media extractor in calling process");
        return CreateFromService(source, mime);
    } else {
        // remote extractor
        ALOGV("get service manager");
        sp<IBinder> binder = defaultServiceManager()->getService(String16("media.extractor"));

        if (binder != 0) {
            sp<IMediaExtractorService> mediaExService(interface_cast<IMediaExtractorService>(binder));
            sp<IMediaExtractor> ex = mediaExService->makeExtractor(
                    CreateIDataSourceFromDataSource(source), mime);
            return ex;
        } else {
            ALOGE("extractor service not running");
            return NULL;
        }
    }
    return NULL;
}

sp<IMediaExtractor> MediaExtractorFactory::CreateFromService(
        const sp<DataSource> &source, const char *mime) {

    ALOGV("MediaExtractorFactory::CreateFromService %s", mime);

    // initialize source decryption if needed
    source->DrmInitialization(nullptr /* mime */);

    void *meta = nullptr;
    void *creator = NULL;
    FreeMetaFunc freeMeta = nullptr;
    float confidence;
    sp<ExtractorPlugin> plugin;
    uint32_t creatorVersion = 0;
    creator = sniff(source, &confidence, &meta, &freeMeta, plugin, &creatorVersion);
    if (!creator) {
        ALOGV("FAILED to autodetect media content.");
        return NULL;
    }

    MediaExtractor *ex = nullptr;
    if (creatorVersion == EXTRACTORDEF_VERSION_NDK_V1 ||
            creatorVersion == EXTRACTORDEF_VERSION_NDK_V2) {
        CMediaExtractor *ret = ((CreatorFunc)creator)(source->wrap(), meta);
        if (meta != nullptr && freeMeta != nullptr) {
            freeMeta(meta);
        }
        ex = ret != nullptr ? new MediaExtractorCUnwrapper(ret) : nullptr;
    }

    ALOGV("Created an extractor '%s' with confidence %.2f",
         ex != nullptr ? ex->name() : "<null>", confidence);

    return CreateIMediaExtractorFromMediaExtractor(ex, source, plugin);
}

struct ExtractorPlugin : public RefBase {
    ExtractorDef def;
    void *libHandle;
    String8 libPath;
    String8 uuidString;

    ExtractorPlugin(ExtractorDef definition, void *handle, String8 &path)
        : def(definition), libHandle(handle), libPath(path) {
        for (size_t i = 0; i < sizeof ExtractorDef::extractor_uuid; i++) {
            uuidString.appendFormat("%02x", def.extractor_uuid.b[i]);
        }
    }
    ~ExtractorPlugin() {
        if (libHandle != nullptr) {
            ALOGV("closing handle for %s %d", libPath.c_str(), def.extractor_version);
            dlclose(libHandle);
        }
    }
};

Mutex MediaExtractorFactory::gPluginMutex;
std::shared_ptr<std::list<sp<ExtractorPlugin>>> MediaExtractorFactory::gPlugins;
bool MediaExtractorFactory::gPluginsRegistered = false;
bool MediaExtractorFactory::gIgnoreVersion = false;

// static
void *MediaExtractorFactory::sniff(
        const sp<DataSource> &source, float *confidence, void **meta,
        FreeMetaFunc *freeMeta, sp<ExtractorPlugin> &plugin, uint32_t *creatorVersion) {
    *confidence = 0.0f;
    *meta = nullptr;

    std::shared_ptr<std::list<sp<ExtractorPlugin>>> plugins;
    {
        Mutex::Autolock autoLock(gPluginMutex);
        if (!gPluginsRegistered) {
            return NULL;
        }
        plugins = gPlugins;
    }

    void *bestCreator = NULL;
    for (auto it = plugins->begin(); it != plugins->end(); ++it) {
        ALOGV("sniffing %s", (*it)->def.extractor_name);
        float newConfidence;
        void *newMeta = nullptr;
        FreeMetaFunc newFreeMeta = nullptr;

        void *curCreator = NULL;
        if ((*it)->def.def_version == EXTRACTORDEF_VERSION_NDK_V1) {
            curCreator = (void*) (*it)->def.u.v2.sniff(
                    source->wrap(), &newConfidence, &newMeta, &newFreeMeta);
        } else if ((*it)->def.def_version == EXTRACTORDEF_VERSION_NDK_V2) {
            curCreator = (void*) (*it)->def.u.v3.sniff(
                    source->wrap(), &newConfidence, &newMeta, &newFreeMeta);
        }

        if (curCreator) {
            if (newConfidence > *confidence) {
                *confidence = newConfidence;
                if (*meta != nullptr && *freeMeta != nullptr) {
                    (*freeMeta)(*meta);
                }
                *meta = newMeta;
                *freeMeta = newFreeMeta;
                plugin = *it;
                bestCreator = curCreator;
                *creatorVersion = (*it)->def.def_version;
            } else {
                if (newMeta != nullptr && newFreeMeta != nullptr) {
                    newFreeMeta(newMeta);
                }
            }
        }
    }

    return bestCreator;
}

// static
void MediaExtractorFactory::RegisterExtractor(const sp<ExtractorPlugin> &plugin,
        std::list<sp<ExtractorPlugin>> &pluginList) {
    // sanity check check struct version, uuid, name
    if (plugin->def.def_version != EXTRACTORDEF_VERSION_NDK_V1 &&
            plugin->def.def_version != EXTRACTORDEF_VERSION_NDK_V2) {
        ALOGE("don't understand extractor format %u, ignoring.", plugin->def.def_version);
        return;
    }
    if (memcmp(&plugin->def.extractor_uuid, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16) == 0) {
        ALOGE("invalid UUID, ignoring");
        return;
    }
    if (plugin->def.extractor_name == NULL || strlen(plugin->def.extractor_name) == 0) {
        ALOGE("extractors should have a name, ignoring");
        return;
    }

    for (auto it = pluginList.begin(); it != pluginList.end(); ++it) {
        if (memcmp(&((*it)->def.extractor_uuid), &plugin->def.extractor_uuid, 16) == 0) {
            // there's already an extractor with the same uuid
            if (gIgnoreVersion || (*it)->def.extractor_version < plugin->def.extractor_version) {
                // this one is newer, replace the old one
                ALOGW("replacing extractor '%s' version %u with version %u",
                        plugin->def.extractor_name,
                        (*it)->def.extractor_version,
                        plugin->def.extractor_version);
                pluginList.erase(it);
                break;
            } else {
                ALOGW("ignoring extractor '%s' version %u in favor of version %u",
                        plugin->def.extractor_name,
                        plugin->def.extractor_version,
                        (*it)->def.extractor_version);
                return;
            }
        }
    }
    ALOGV("registering extractor for %s", plugin->def.extractor_name);
    pluginList.push_back(plugin);
}

//static
void MediaExtractorFactory::RegisterExtractors(
        const char *libDirPath, const android_dlextinfo* dlextinfo,
        std::list<sp<ExtractorPlugin>> &pluginList) {
    ALOGV("search for plugins at %s", libDirPath);

    DIR *libDir = opendir(libDirPath);
    if (libDir) {
        struct dirent* libEntry;
        while ((libEntry = readdir(libDir))) {
            if (libEntry->d_name[0] == '.') {
                continue;
            }
            String8 libPath = String8(libDirPath) + "/" + libEntry->d_name;
            if (!libPath.contains("extractor.so")) {
                continue;
            }
            void *libHandle = android_dlopen_ext(
                    libPath.string(),
                    RTLD_NOW | RTLD_LOCAL, dlextinfo);
            CHECK(libHandle != nullptr)
                    << "couldn't dlopen(" << libPath.string() << ") " << strerror(errno);

            GetExtractorDef getDef =
                (GetExtractorDef) dlsym(libHandle, "GETEXTRACTORDEF");
            CHECK(getDef != nullptr)
                    << libPath.string() << " does not contain sniffer";

            ALOGV("registering sniffer for %s", libPath.string());
            RegisterExtractor(
                    new ExtractorPlugin(getDef(), libHandle, libPath), pluginList);
        }
        closedir(libDir);
    } else {
        ALOGE("couldn't opendir(%s)", libDirPath);
    }
}

static bool compareFunc(const sp<ExtractorPlugin>& first, const sp<ExtractorPlugin>& second) {
    return strcmp(first->def.extractor_name, second->def.extractor_name) < 0;
}

static std::unordered_set<std::string> gSupportedExtensions;

// static
void MediaExtractorFactory::LoadExtractors() {
    Mutex::Autolock autoLock(gPluginMutex);

    if (gPluginsRegistered) {
        return;
    }

    gIgnoreVersion = property_get_bool("debug.extractor.ignore_version", false);

    std::shared_ptr<std::list<sp<ExtractorPlugin>>> newList(new std::list<sp<ExtractorPlugin>>());

    android_namespace_t *mediaNs = android_get_exported_namespace("media");
    if (mediaNs != NULL) {
        const android_dlextinfo dlextinfo = {
            .flags = ANDROID_DLEXT_USE_NAMESPACE,
            .library_namespace = mediaNs,
        };
        RegisterExtractors("/apex/com.android.media/lib"
#ifdef __LP64__
                "64"
#endif
                "/extractors", &dlextinfo, *newList);

    } else {
        ALOGE("couldn't find media namespace.");
    }

    RegisterExtractors("/system/lib"
#ifdef __LP64__
            "64"
#endif
            "/extractors", NULL, *newList);

    newList->sort(compareFunc);
    gPlugins = newList;

    for (auto it = gPlugins->begin(); it != gPlugins->end(); ++it) {
        if ((*it)->def.def_version == EXTRACTORDEF_VERSION_NDK_V2) {
            for (size_t i = 0;; i++) {
                const char* ext = (*it)->def.u.v3.supported_types[i];
                if (ext == nullptr) {
                    break;
                }
                gSupportedExtensions.insert(std::string(ext));
            }
        }
    }

    gPluginsRegistered = true;
}

// static
std::unordered_set<std::string> MediaExtractorFactory::getSupportedTypes() {
    if (getuid() == AID_MEDIA_EX) {
        return gSupportedExtensions;
    }
    ALOGV("get service manager");
    sp<IBinder> binder = defaultServiceManager()->getService(String16("media.extractor"));

    if (binder != 0) {
        sp<IMediaExtractorService> mediaExService(interface_cast<IMediaExtractorService>(binder));
        return mediaExService->getSupportedTypes();
    }
    return std::unordered_set<std::string>();
}

status_t MediaExtractorFactory::dump(int fd, const Vector<String16>&) {
    Mutex::Autolock autoLock(gPluginMutex);
    String8 out;

    const IPCThreadState* ipc = IPCThreadState::self();
    const int pid = ipc->getCallingPid();
    const int uid = ipc->getCallingUid();
    if (!PermissionCache::checkPermission(String16("android.permission.DUMP"), pid, uid)) {
        // dumpExtractors() will append the following string.
        // out.appendFormat("Permission Denial: "
        //        "can't dump MediaExtractor from pid=%d, uid=%d\n", pid, uid);
        ALOGE("Permission Denial: can't dump MediaExtractor from pid=%d, uid=%d", pid, uid);
    } else {
        out.append("Available extractors:\n");
        if (gPluginsRegistered) {
            for (auto it = gPlugins->begin(); it != gPlugins->end(); ++it) {
                out.appendFormat("  %25s: plugin_version(%d), uuid(%s), version(%u), path(%s)",
                        (*it)->def.extractor_name,
                    (*it)->def.def_version,
                        (*it)->uuidString.c_str(),
                        (*it)->def.extractor_version,
                        (*it)->libPath.c_str());
                if ((*it)->def.def_version == EXTRACTORDEF_VERSION_NDK_V2) {
                    out.append(", supports: ");
                    for (size_t i = 0;; i++) {
                        const char* mime = (*it)->def.u.v3.supported_types[i];
                        if (mime == nullptr) {
                            break;
                        }
                        out.appendFormat("%s ", mime);
                    }
                }
                out.append("\n");
            }
            out.append("\n");
        } else {
            out.append("  (no plugins registered)\n");
        }
    }
    write(fd, out.string(), out.size());
    return OK;
}


}  // namespace android
