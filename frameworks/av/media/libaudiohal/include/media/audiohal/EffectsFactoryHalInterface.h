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

#ifndef ANDROID_HARDWARE_EFFECTS_FACTORY_HAL_INTERFACE_H
#define ANDROID_HARDWARE_EFFECTS_FACTORY_HAL_INTERFACE_H

#include <media/audiohal/EffectHalInterface.h>
#include <system/audio_effect.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>

namespace android {

class EffectsFactoryHalInterface : public RefBase
{
  public:
    // Returns the number of different effects in all loaded libraries.
    virtual status_t queryNumberEffects(uint32_t *pNumEffects) = 0;

    // Returns a descriptor of the next available effect.
    virtual status_t getDescriptor(uint32_t index,
            effect_descriptor_t *pDescriptor) = 0;

    virtual status_t getDescriptor(const effect_uuid_t *pEffectUuid,
            effect_descriptor_t *pDescriptor) = 0;

    // Creates an effect engine of the specified type.
    // To release the effect engine, it is necessary to release references
    // to the returned effect object.
    virtual status_t createEffect(const effect_uuid_t *pEffectUuid,
            int32_t sessionId, int32_t ioId,
            sp<EffectHalInterface> *effect) = 0;

    virtual status_t dumpEffects(int fd) = 0;

    static sp<EffectsFactoryHalInterface> create();

    virtual status_t allocateBuffer(size_t size, sp<EffectBufferHalInterface>* buffer) = 0;
    virtual status_t mirrorBuffer(void* external, size_t size,
                                  sp<EffectBufferHalInterface>* buffer) = 0;

    // Helper function to compare effect uuid to EFFECT_UUID_NULL.
    static bool isNullUuid(const effect_uuid_t *pEffectUuid);

  protected:
    // Subclasses can not be constructed directly by clients.
    EffectsFactoryHalInterface() {}

    virtual ~EffectsFactoryHalInterface() {}
};

} // namespace android

#endif // ANDROID_HARDWARE_EFFECTS_FACTORY_HAL_INTERFACE_H
