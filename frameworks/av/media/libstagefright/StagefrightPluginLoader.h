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

#ifndef STAGEFRIGHT_PLUGIN_LOADER_H_

#define STAGEFRIGHT_PLUGIN_LOADER_H_

#include <media/stagefright/CodecBase.h>
#include <media/stagefright/MediaCodecListWriter.h>
#include <media/stagefright/PersistentSurface.h>
#include <utils/Mutex.h>

namespace android {

class StagefrightPluginLoader {
public:
    static const std::unique_ptr<StagefrightPluginLoader> &GetCCodecInstance();
    ~StagefrightPluginLoader();

    CodecBase *createCodec();
    MediaCodecListBuilderBase *createBuilder();
    PersistentSurface *createInputSurface();

private:
    explicit StagefrightPluginLoader(const char *libPath);

    static Mutex sMutex;
    static std::unique_ptr<StagefrightPluginLoader> sInstance;

    void *mLibHandle{nullptr};
    CodecBase::CreateCodecFunc mCreateCodec{nullptr};
    MediaCodecListBuilderBase::CreateBuilderFunc mCreateBuilder{nullptr};
    CodecBase::CreateInputSurfaceFunc mCreateInputSurface{nullptr};
};

}  // namespace android

#endif  // STAGEFRIGHT_PLUGIN_LOADER_H_
