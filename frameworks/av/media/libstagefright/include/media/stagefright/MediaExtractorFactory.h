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

#ifndef MEDIA_EXTRACTOR_FACTORY_H_

#define MEDIA_EXTRACTOR_FACTORY_H_

#include <stdio.h>
#include <unordered_set>

#include <android/dlext.h>
#include <media/IMediaExtractor.h>

namespace android {

class DataSource;
struct ExtractorPlugin;

class MediaExtractorFactory {
public:
    static sp<IMediaExtractor> Create(
            const sp<DataSource> &source, const char *mime = NULL);
    static sp<IMediaExtractor> CreateFromService(
            const sp<DataSource> &source, const char *mime = NULL);
    static status_t dump(int fd, const Vector<String16>& args);
    static std::unordered_set<std::string> getSupportedTypes();
    static void LoadExtractors();

private:
    static Mutex gPluginMutex;
    static std::shared_ptr<std::list<sp<ExtractorPlugin>>> gPlugins;
    static bool gPluginsRegistered;
    static bool gIgnoreVersion;

    static void RegisterExtractors(
            const char *libDirPath, const android_dlextinfo* dlextinfo,
            std::list<sp<ExtractorPlugin>> &pluginList);
    static void RegisterExtractor(
            const sp<ExtractorPlugin> &plugin, std::list<sp<ExtractorPlugin>> &pluginList);

    static void *sniff(const sp<DataSource> &source,
            float *confidence, void **meta, FreeMetaFunc *freeMeta,
            sp<ExtractorPlugin> &plugin, uint32_t *creatorVersion);
};

}  // namespace android

#endif  // MEDIA_EXTRACTOR_FACTORY_H_
