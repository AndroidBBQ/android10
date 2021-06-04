/*
**
** Copyright (C) 2014, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_TAG "SoundTrigger"
//#define LOG_NDEBUG 0

#include <utils/Log.h>
#include <utils/threads.h>
#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <binder/IMemory.h>

#include <soundtrigger/SoundTrigger.h>
#include <soundtrigger/ISoundTrigger.h>
#include <soundtrigger/ISoundTriggerHwService.h>
#include <soundtrigger/ISoundTriggerClient.h>
#include <soundtrigger/SoundTriggerCallback.h>

namespace android {

namespace {
    sp<ISoundTriggerHwService> gSoundTriggerHwService;
    const int                  kSoundTriggerHwServicePollDelay = 500000; // 0.5s
    const char*                kSoundTriggerHwServiceName      = "media.sound_trigger_hw";
    Mutex                      gLock;

    class DeathNotifier : public IBinder::DeathRecipient
    {
    public:
        DeathNotifier() {
        }

        virtual void binderDied(const wp<IBinder>& who __unused) {
            ALOGV("binderDied");
            Mutex::Autolock _l(gLock);
            gSoundTriggerHwService.clear();
            ALOGW("Sound trigger service died!");
        }
    };

    sp<DeathNotifier>         gDeathNotifier;
}; // namespace anonymous

const sp<ISoundTriggerHwService> SoundTrigger::getSoundTriggerHwService()
{
    Mutex::Autolock _l(gLock);
    if (gSoundTriggerHwService.get() == 0) {
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder;
        do {
            binder = sm->getService(String16(kSoundTriggerHwServiceName));
            if (binder != 0) {
                break;
            }
            ALOGW("SoundTriggerHwService not published, waiting...");
            usleep(kSoundTriggerHwServicePollDelay);
        } while(true);
        if (gDeathNotifier == NULL) {
            gDeathNotifier = new DeathNotifier();
        }
        binder->linkToDeath(gDeathNotifier);
        gSoundTriggerHwService = interface_cast<ISoundTriggerHwService>(binder);
    }
    ALOGE_IF(gSoundTriggerHwService == 0, "no SoundTriggerHwService!?");
    return gSoundTriggerHwService;
}

// Static methods
status_t SoundTrigger::listModules(const String16& opPackageName,
                                   struct sound_trigger_module_descriptor *modules,
                                   uint32_t *numModules)
{
    ALOGV("listModules()");
    const sp<ISoundTriggerHwService> service = getSoundTriggerHwService();
    if (service == 0) {
        return NO_INIT;
    }
    return service->listModules(opPackageName, modules, numModules);
}

sp<SoundTrigger> SoundTrigger::attach(const String16& opPackageName,
                                      const sound_trigger_module_handle_t module,
                                      const sp<SoundTriggerCallback>& callback)
{
    ALOGV("attach()");
    sp<SoundTrigger> soundTrigger;
    const sp<ISoundTriggerHwService> service = getSoundTriggerHwService();
    if (service == 0) {
        return soundTrigger;
    }
    soundTrigger = new SoundTrigger(module, callback);
    status_t status = service->attach(opPackageName, module, soundTrigger,
                                      soundTrigger->mISoundTrigger);

    if (status == NO_ERROR && soundTrigger->mISoundTrigger != 0) {
        IInterface::asBinder(soundTrigger->mISoundTrigger)->linkToDeath(soundTrigger);
    } else {
        ALOGW("Error %d connecting to sound trigger service", status);
        soundTrigger.clear();
    }
    return soundTrigger;
}


status_t SoundTrigger::setCaptureState(bool active)
{
    ALOGV("setCaptureState(%d)", active);
    const sp<ISoundTriggerHwService> service = getSoundTriggerHwService();
    if (service == 0) {
        return NO_INIT;
    }
    return service->setCaptureState(active);
}

// SoundTrigger
SoundTrigger::SoundTrigger(sound_trigger_module_handle_t /*module*/,
                                 const sp<SoundTriggerCallback>& callback)
    : mCallback(callback)
{
}

SoundTrigger::~SoundTrigger()
{
    if (mISoundTrigger != 0) {
        mISoundTrigger->detach();
    }
}


void SoundTrigger::detach() {
    ALOGV("detach()");
    Mutex::Autolock _l(mLock);
    mCallback.clear();
    if (mISoundTrigger != 0) {
        mISoundTrigger->detach();
        IInterface::asBinder(mISoundTrigger)->unlinkToDeath(this);
        mISoundTrigger = 0;
    }
}

status_t SoundTrigger::loadSoundModel(const sp<IMemory>& modelMemory,
                                sound_model_handle_t *handle)
{
    Mutex::Autolock _l(mLock);
    if (mISoundTrigger == 0) {
        return NO_INIT;
    }

    return mISoundTrigger->loadSoundModel(modelMemory, handle);
}

status_t SoundTrigger::unloadSoundModel(sound_model_handle_t handle)
{
    Mutex::Autolock _l(mLock);
    if (mISoundTrigger == 0) {
        return NO_INIT;
    }
    return mISoundTrigger->unloadSoundModel(handle);
}

status_t SoundTrigger::startRecognition(sound_model_handle_t handle,
                                        const sp<IMemory>& dataMemory)
{
    Mutex::Autolock _l(mLock);
    if (mISoundTrigger == 0) {
        return NO_INIT;
    }
    return mISoundTrigger->startRecognition(handle, dataMemory);
}

status_t SoundTrigger::stopRecognition(sound_model_handle_t handle)
{
    Mutex::Autolock _l(mLock);
    if (mISoundTrigger == 0) {
        return NO_INIT;
    }
    return mISoundTrigger->stopRecognition(handle);
}

status_t SoundTrigger::getModelState(sound_model_handle_t handle)
{
    Mutex::Autolock _l(mLock);
    if (mISoundTrigger == 0) {
        return NO_INIT;
    }
    return mISoundTrigger->getModelState(handle);
}

// BpSoundTriggerClient
void SoundTrigger::onRecognitionEvent(const sp<IMemory>& eventMemory)
{
    Mutex::Autolock _l(mLock);
    if (eventMemory == 0 || eventMemory->pointer() == NULL) {
        return;
    }

    if (mCallback != 0) {
        mCallback->onRecognitionEvent(
                (struct sound_trigger_recognition_event *)eventMemory->pointer());
    }
}

void SoundTrigger::onSoundModelEvent(const sp<IMemory>& eventMemory)
{
    Mutex::Autolock _l(mLock);
    if (eventMemory == 0 || eventMemory->pointer() == NULL) {
        return;
    }

    if (mCallback != 0) {
        mCallback->onSoundModelEvent(
                (struct sound_trigger_model_event *)eventMemory->pointer());
    }
}

void SoundTrigger::onServiceStateChange(const sp<IMemory>& eventMemory)
{
    Mutex::Autolock _l(mLock);
    if (eventMemory == 0 || eventMemory->pointer() == NULL) {
        return;
    }

    if (mCallback != 0) {
        mCallback->onServiceStateChange(
                *((sound_trigger_service_state_t *)eventMemory->pointer()));
    }
}

//IBinder::DeathRecipient
void SoundTrigger::binderDied(const wp<IBinder>& who __unused) {
    Mutex::Autolock _l(mLock);
    ALOGW("SoundTrigger server binder Died ");
    mISoundTrigger = 0;
    if (mCallback != 0) {
        mCallback->onServiceDied();
    }
}

status_t SoundTrigger::stringToGuid(const char *str, sound_trigger_uuid_t *guid)
{
    if (str == NULL || guid == NULL) {
        return BAD_VALUE;
    }

    int tmp[10];

    if (sscanf(str, "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
            tmp, tmp+1, tmp+2, tmp+3, tmp+4, tmp+5, tmp+6, tmp+7, tmp+8, tmp+9) < 10) {
        return BAD_VALUE;
    }
    guid->timeLow = (uint32_t)tmp[0];
    guid->timeMid = (uint16_t)tmp[1];
    guid->timeHiAndVersion = (uint16_t)tmp[2];
    guid->clockSeq = (uint16_t)tmp[3];
    guid->node[0] = (uint8_t)tmp[4];
    guid->node[1] = (uint8_t)tmp[5];
    guid->node[2] = (uint8_t)tmp[6];
    guid->node[3] = (uint8_t)tmp[7];
    guid->node[4] = (uint8_t)tmp[8];
    guid->node[5] = (uint8_t)tmp[9];

    return NO_ERROR;
}

status_t SoundTrigger::guidToString(const sound_trigger_uuid_t *guid, char *str, size_t maxLen)
{
    if (guid == NULL || str == NULL) {
        return BAD_VALUE;
    }

    snprintf(str, maxLen, "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
            guid->timeLow,
            guid->timeMid,
            guid->timeHiAndVersion,
            guid->clockSeq,
            guid->node[0],
            guid->node[1],
            guid->node[2],
            guid->node[3],
            guid->node[4],
            guid->node[5]);

    return NO_ERROR;
}

}; // namespace android
