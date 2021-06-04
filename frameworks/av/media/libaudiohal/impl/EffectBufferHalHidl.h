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

#ifndef ANDROID_HARDWARE_EFFECT_BUFFER_HAL_HIDL_H
#define ANDROID_HARDWARE_EFFECT_BUFFER_HAL_HIDL_H

#include PATH(android/hardware/audio/effect/FILE_VERSION/types.h)
#include <android/hidl/memory/1.0/IMemory.h>
#include <hidl/HidlSupport.h>
#include <media/audiohal/EffectBufferHalInterface.h>
#include <system/audio_effect.h>

using android::hardware::hidl_memory;
using android::hidl::memory::V1_0::IMemory;

namespace android {
namespace effect {
namespace CPP_VERSION {

using namespace ::android::hardware::audio::effect::CPP_VERSION;

class EffectBufferHalHidl : public EffectBufferHalInterface
{
  public:
    static status_t allocate(size_t size, sp<EffectBufferHalInterface>* buffer);
    static status_t mirror(void* external, size_t size, sp<EffectBufferHalInterface>* buffer);

    virtual audio_buffer_t* audioBuffer();
    virtual void* externalData() const;

    virtual size_t getSize() const override { return mBufferSize; }

    virtual void setExternalData(void* external);
    virtual void setFrameCount(size_t frameCount);
    virtual bool checkFrameCountChange();

    virtual void update();
    virtual void commit();
    virtual void update(size_t size);
    virtual void commit(size_t size);

    const AudioBuffer& hidlBuffer() const { return mHidlBuffer; }

  private:
    friend class EffectBufferHalInterface;

    static uint64_t makeUniqueId();

    const size_t mBufferSize;
    bool mFrameCountChanged;
    void* mExternalData;
    AudioBuffer mHidlBuffer;
    sp<IMemory> mMemory;
    audio_buffer_t mAudioBuffer;

    // Can not be constructed directly by clients.
    explicit EffectBufferHalHidl(size_t size);

    virtual ~EffectBufferHalHidl();

    status_t init();
};

} // namespace CPP_VERSION
} // namespace effect
} // namespace android

#endif // ANDROID_HARDWARE_EFFECT_BUFFER_HAL_HIDL_H
