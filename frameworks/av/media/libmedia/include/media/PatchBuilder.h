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

#ifndef ANDROID_PATCH_BUILDER_H
#define ANDROID_PATCH_BUILDER_H

#include <functional>
#include <utility>

#include <system/audio.h>
#include <utils/StrongPointer.h>

// This is a header-only utility.

namespace android {

class PatchBuilder {
  public:
    using mix_usecase_t = decltype(audio_port_config_mix_ext::usecase);

    PatchBuilder() = default;

    // All existing methods operating on audio patches take a pointer to const.
    // It's OK to construct a temporary PatchBuilder while preparing a parameter
    // to such a function because the Builder will be kept alive until the code
    // execution reaches the function call statement semicolon.
    const struct audio_patch* patch() const { return &mPatch; }

    template<typename T, typename... S>
    PatchBuilder& addSink(T&& t, S&&... s) {
        sinks().add(std::forward<T>(t), std::forward<S>(s)...);
        return *this;
    }
    // Explicit type of the second parameter allows clients to provide the struct inline.
    template<typename T>
    PatchBuilder& addSink(T&& t, const mix_usecase_t& update) {
        sinks().add(std::forward<T>(t), update);
        return *this;
    }
    template<typename T, typename... S>
    PatchBuilder& addSource(T&& t, S&&... s) {
        sources().add(std::forward<T>(t), std::forward<S>(s)...);
        return *this;
    }
    // Explicit type of the second parameter allows clients to provide the struct inline.
    template<typename T>
    PatchBuilder& addSource(T&& t, const mix_usecase_t& update) {
        sources().add(std::forward<T>(t), update);
        return *this;
    }

  private:
    struct PortCfgs {
        PortCfgs(unsigned int *countPtr, struct audio_port_config *portCfgs)
                : mCountPtr(countPtr), mPortCfgs(portCfgs) {}
        audio_port_config& add(const audio_port_config& portCfg) {
            return *advance() = portCfg;
        }
        template<typename T>
        audio_port_config& add(const sp<T>& entity) {
            audio_port_config* added = advance();
            entity->toAudioPortConfig(added);
            return *added;
        }
        template<typename T>
        void add(const sp<T>& entity, const mix_usecase_t& usecaseUpdate) {
            add(entity).ext.mix.usecase = usecaseUpdate;
        }
        template<typename T>
        void add(const sp<T>& entity,
                std::function<mix_usecase_t(const mix_usecase_t&)> usecaseUpdater) {
            mix_usecase_t* usecase = &add(entity).ext.mix.usecase;
            *usecase = usecaseUpdater(*usecase);
        }
        struct audio_port_config* advance() {
            return &mPortCfgs[(*mCountPtr)++];
        }
        unsigned int *mCountPtr;
        struct audio_port_config *mPortCfgs;
    };

    PortCfgs sinks() { return PortCfgs(&mPatch.num_sinks, mPatch.sinks); }
    PortCfgs sources() { return PortCfgs(&mPatch.num_sources, mPatch.sources); }

    struct audio_patch mPatch = {};
};

}  // namespace android

#endif  // ANDROID_PATCH_BUILDER_H
