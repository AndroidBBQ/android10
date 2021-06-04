/*
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

#include <android/hardware/audio/effect/2.0/IEffectsFactory.h>
#include <android/hardware/audio/effect/4.0/IEffectsFactory.h>
#include <android/hardware/audio/effect/5.0/IEffectsFactory.h>

#include <libaudiohal/FactoryHalHidl.h>

namespace android {

// static
sp<EffectsFactoryHalInterface> EffectsFactoryHalInterface::create() {
    if (hardware::audio::effect::V5_0::IEffectsFactory::getService() != nullptr) {
        return effect::V5_0::createEffectsFactoryHal();
    }
    if (hardware::audio::effect::V4_0::IEffectsFactory::getService() != nullptr) {
        return effect::V4_0::createEffectsFactoryHal();
    }
    if (hardware::audio::effect::V2_0::IEffectsFactory::getService() != nullptr) {
        return effect::V2_0::createEffectsFactoryHal();
    }
    return nullptr;
}

// static
bool EffectsFactoryHalInterface::isNullUuid(const effect_uuid_t *pEffectUuid) {
    return memcmp(pEffectUuid, EFFECT_UUID_NULL, sizeof(effect_uuid_t)) == 0;
}

} // namespace android
