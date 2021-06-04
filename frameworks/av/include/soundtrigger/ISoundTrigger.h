/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef ANDROID_HARDWARE_ISOUNDTRIGGER_H
#define ANDROID_HARDWARE_ISOUNDTRIGGER_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/IMemory.h>
#include <system/sound_trigger.h>

namespace android {

class ISoundTrigger : public IInterface
{
public:
    DECLARE_META_INTERFACE(SoundTrigger);

    virtual void detach() = 0;

    virtual status_t loadSoundModel(const sp<IMemory>& modelMemory,
                                    sound_model_handle_t *handle) = 0;

    virtual status_t unloadSoundModel(sound_model_handle_t handle) = 0;

    virtual status_t startRecognition(sound_model_handle_t handle,
                                      const sp<IMemory>& dataMemory) = 0;
    virtual status_t stopRecognition(sound_model_handle_t handle) = 0;
    virtual status_t getModelState(sound_model_handle_t handle) = 0;

};

// ----------------------------------------------------------------------------

class BnSoundTrigger: public BnInterface<ISoundTrigger>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace android

#endif //ANDROID_HARDWARE_ISOUNDTRIGGER_H
