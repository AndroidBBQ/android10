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

#ifndef ANDROID_HARDWARE_ISOUNDTRIGGER_SERVICE_H
#define ANDROID_HARDWARE_ISOUNDTRIGGER_SERVICE_H

#include <utils/RefBase.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <system/sound_trigger.h>

namespace android {

class ISoundTrigger;
class ISoundTriggerClient;

class ISoundTriggerHwService : public IInterface
{
public:

    DECLARE_META_INTERFACE(SoundTriggerHwService);

    virtual status_t listModules(const String16& opPackageName,
                                 struct sound_trigger_module_descriptor *modules,
                                 uint32_t *numModules) = 0;

    virtual status_t attach(const String16& opPackageName,
                            const sound_trigger_module_handle_t handle,
                            const sp<ISoundTriggerClient>& client,
                            sp<ISoundTrigger>& module) = 0;

    virtual status_t setCaptureState(bool active) = 0;
};

// ----------------------------------------------------------------------------

class BnSoundTriggerHwService: public BnInterface<ISoundTriggerHwService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

}; // namespace android

#endif //ANDROID_HARDWARE_ISOUNDTRIGGER_SERVICE_H
