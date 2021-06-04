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

#ifndef ANDROID_HARDWARE_SOUNDTRIGGER_H
#define ANDROID_HARDWARE_SOUNDTRIGGER_H

#include <binder/IBinder.h>
#include <utils/threads.h>
#include <soundtrigger/SoundTriggerCallback.h>
#include <soundtrigger/ISoundTrigger.h>
#include <soundtrigger/ISoundTriggerHwService.h>
#include <soundtrigger/ISoundTriggerClient.h>
#include <system/sound_trigger.h>

namespace android {

class MemoryDealer;

class SoundTrigger : public BnSoundTriggerClient,
                        public IBinder::DeathRecipient
{
public:

    virtual ~SoundTrigger();

    static  status_t listModules(const String16& opPackageName,
                                 struct sound_trigger_module_descriptor *modules,
                                 uint32_t *numModules);
    static  sp<SoundTrigger> attach(const String16& opPackageName,
                                    const sound_trigger_module_handle_t module,
                                    const sp<SoundTriggerCallback>& callback);

    static  status_t setCaptureState(bool active);

            void detach();

            status_t loadSoundModel(const sp<IMemory>& modelMemory,
                                            sound_model_handle_t *handle);

            status_t unloadSoundModel(sound_model_handle_t handle);

            status_t startRecognition(sound_model_handle_t handle, const sp<IMemory>& dataMemory);
            status_t stopRecognition(sound_model_handle_t handle);
            status_t getModelState(sound_model_handle_t handle);

            // BpSoundTriggerClient
            virtual void onRecognitionEvent(const sp<IMemory>& eventMemory);
            virtual void onSoundModelEvent(const sp<IMemory>& eventMemory);
            virtual void onServiceStateChange(const sp<IMemory>& eventMemory);

            //IBinder::DeathRecipient
            virtual void binderDied(const wp<IBinder>& who);

            static status_t stringToGuid(const char *str, sound_trigger_uuid_t *guid);
            static status_t guidToString(const sound_trigger_uuid_t *guid,
                                         char *str, size_t maxLen);

private:
            SoundTrigger(sound_trigger_module_handle_t module,
                            const sp<SoundTriggerCallback>&);
            static const sp<ISoundTriggerHwService> getSoundTriggerHwService();

            Mutex                               mLock;
            sp<ISoundTrigger>                   mISoundTrigger;
            sp<SoundTriggerCallback>            mCallback;
};

}; // namespace android

#endif //ANDROID_HARDWARE_SOUNDTRIGGER_H
