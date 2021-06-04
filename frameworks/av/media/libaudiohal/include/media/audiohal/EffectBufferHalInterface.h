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

#ifndef ANDROID_HARDWARE_EFFECT_BUFFER_HAL_INTERFACE_H
#define ANDROID_HARDWARE_EFFECT_BUFFER_HAL_INTERFACE_H

#include <system/audio_effect.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>

namespace android {

// Abstraction for an audio buffer. It may be a "mirror" for
// a buffer that the effect chain doesn't own, or a buffer owned by
// the effect chain.
// Buffers are created from EffectsFactoryHalInterface
class EffectBufferHalInterface : public RefBase
{
  public:
    virtual audio_buffer_t* audioBuffer() = 0;
    virtual void* externalData() const = 0;
    // To be used when interacting with the code that doesn't know about
    // "mirrored" buffers.
    virtual void* ptr() {
        return externalData() != nullptr ? externalData() : audioBuffer()->raw;
    }

    virtual size_t getSize() const = 0;

    virtual void setExternalData(void* external) = 0;
    virtual void setFrameCount(size_t frameCount) = 0;
    virtual bool checkFrameCountChange() = 0;  // returns whether frame count has been updated
                                               // since the last call to this method

    virtual void update() = 0;  // copies data from the external buffer, noop for allocated buffers
    virtual void commit() = 0;  // copies data to the external buffer, noop for allocated buffers
    virtual void update(size_t size) = 0;  // copies partial data from external buffer
    virtual void commit(size_t size) = 0;  // copies partial data to external buffer

  protected:
    // Subclasses can not be constructed directly by clients.
    EffectBufferHalInterface() {}

    virtual ~EffectBufferHalInterface() {}
};

} // namespace android

#endif // ANDROID_HARDWARE_EFFECT_BUFFER_HAL_INTERFACE_H
