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

#ifndef ANDROID_HARDWARE_SOUNDTRIGGER_HAL_INTERFACE_H
#define ANDROID_HARDWARE_SOUNDTRIGGER_HAL_INTERFACE_H

#include <utils/RefBase.h>
#include <system/sound_trigger.h>
#include <hardware/sound_trigger.h>

namespace android {

class SoundTriggerHalInterface : public virtual RefBase
{
public:
        /* get a sound trigger HAL instance */
        static sp<SoundTriggerHalInterface> connectModule(const char *moduleName);

        virtual     ~SoundTriggerHalInterface() {}

        virtual int getProperties(struct sound_trigger_properties *properties) = 0;

        /*
         * Load a sound model. Once loaded, recognition of this model can be started and stopped.
         * Only one active recognition per model at a time. The SoundTrigger service will handle
         * concurrent recognition requests by different users/applications on the same model.
         * The implementation returns a unique handle used by other functions (unload_sound_model(),
         * start_recognition(), etc...
         */
        virtual int loadSoundModel(struct sound_trigger_sound_model *sound_model,
                                sound_model_callback_t callback,
                                void *cookie,
                                sound_model_handle_t *handle) = 0;

        /*
         * Unload a sound model. A sound model can be unloaded to make room for a new one to overcome
         * implementation limitations.
         */
        virtual int unloadSoundModel(sound_model_handle_t handle) = 0;

        /* Start recognition on a given model. Only one recognition active at a time per model.
         * Once recognition succeeds of fails, the callback is called.
         * TODO: group recognition configuration parameters into one struct and add key phrase options.
         */
        virtual int startRecognition(sound_model_handle_t handle,
                                 const struct sound_trigger_recognition_config *config,
                                 recognition_callback_t callback,
                                 void *cookie) = 0;

        /* Stop recognition on a given model.
         * The implementation does not have to call the callback when stopped via this method.
         */
        virtual int stopRecognition(sound_model_handle_t handle) = 0;

        /* Stop recognition on all models.
         * Only supported for device api versions SOUND_TRIGGER_DEVICE_API_VERSION_1_1 or above.
         * If no implementation is provided, stop_recognition will be called for each running model.
         */
        virtual int stopAllRecognitions() = 0;

        /* Get the current state of a given model.
         * Returns 0 or an error code. If successful the state will be returned asynchronously
         * via a recognition event in the callback method that was registered in the
         * startRecognition() method.
         * Only supported for device api versions SOUND_TRIGGER_DEVICE_API_VERSION_1_2 or above.
         */
        virtual int getModelState(sound_model_handle_t handle) = 0;

protected:
        SoundTriggerHalInterface() {}
};

} // namespace android

#endif // ANDROID_HARDWARE_SOUNDTRIGGER_HAL_INTERFACE_H
