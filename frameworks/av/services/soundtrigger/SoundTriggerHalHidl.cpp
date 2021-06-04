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

#define LOG_TAG "SoundTriggerHalHidl"
//#define LOG_NDEBUG 0

#include <android/hidl/allocator/1.0/IAllocator.h>
#include <media/audiohal/hidl/HalDeathHandler.h>
#include <utils/Log.h>
#include "SoundTriggerHalHidl.h"
#include <hidlmemory/mapping.h>
#include <hwbinder/IPCThreadState.h>
#include <hwbinder/ProcessState.h>

namespace android {

using ::android::hardware::ProcessState;
using ::android::hardware::Return;
using ::android::hardware::Status;
using ::android::hardware::Void;
using ::android::hardware::audio::common::V2_0::AudioDevice;
using ::android::hardware::hidl_memory;
using ::android::hidl::allocator::V1_0::IAllocator;
using ::android::hidl::memory::V1_0::IMemory;

namespace {

// Backs up by the vector with the contents of shared memory.
// It is assumed that the passed hidl_vector is empty, so it's
// not cleared if the memory is a null object.
// The caller needs to keep the returned sp<IMemory> as long as
// the data is needed.
std::pair<bool, sp<IMemory>> memoryAsVector(const hidl_memory& m, hidl_vec<uint8_t>* vec) {
    sp<IMemory> memory;
    if (m.size() == 0) {
        return std::make_pair(true, memory);
    }
    memory = mapMemory(m);
    if (memory != nullptr) {
        memory->read();
        vec->setToExternal(static_cast<uint8_t*>(static_cast<void*>(memory->getPointer())),
                memory->getSize());
        return std::make_pair(true, memory);
    }
    ALOGE("%s: Could not map HIDL memory to IMemory", __func__);
    return std::make_pair(false, memory);
}

// Moves the data from the vector into allocated shared memory,
// emptying the vector.
// It is assumed that the passed hidl_memory is a null object, so it's
// not reset if the vector is empty.
// The caller needs to keep the returned sp<IMemory> as long as
// the data is needed.
std::pair<bool, sp<IMemory>> moveVectorToMemory(hidl_vec<uint8_t>* v, hidl_memory* mem) {
    sp<IMemory> memory;
    if (v->size() == 0) {
        return std::make_pair(true, memory);
    }
    sp<IAllocator> ashmem = IAllocator::getService("ashmem");
    if (ashmem == 0) {
        ALOGE("Failed to retrieve ashmem allocator service");
        return std::make_pair(false, memory);
    }
    bool success = false;
    Return<void> r = ashmem->allocate(v->size(), [&](bool s, const hidl_memory& m) {
        success = s;
        if (success) *mem = m;
    });
    if (r.isOk() && success) {
        memory = hardware::mapMemory(*mem);
        if (memory != 0) {
            memory->update();
            memcpy(memory->getPointer(), v->data(), v->size());
            memory->commit();
            v->resize(0);
            return std::make_pair(true, memory);
        } else {
            ALOGE("Failed to map allocated ashmem");
        }
    } else {
        ALOGE("Failed to allocate %llu bytes from ashmem", (unsigned long long)v->size());
    }
    return std::make_pair(false, memory);
}

}  // namespace

/* static */
sp<SoundTriggerHalInterface> SoundTriggerHalInterface::connectModule(const char *moduleName)
{
    return new SoundTriggerHalHidl(moduleName);
}

int SoundTriggerHalHidl::getProperties(struct sound_trigger_properties *properties)
{
    sp<ISoundTriggerHw> soundtrigger = getService();
    if (soundtrigger == 0) {
        return -ENODEV;
    }

    ISoundTriggerHw::Properties halProperties;
    Return<void> hidlReturn;
    int ret;
    {
        AutoMutex lock(mHalLock);
        hidlReturn = soundtrigger->getProperties([&](int rc, auto res) {
            ret = rc;
            halProperties = res;
            ALOGI("getProperties res implementor %s", res.implementor.c_str());
        });
    }

    if (hidlReturn.isOk()) {
        if (ret == 0) {
            convertPropertiesFromHal(properties, &halProperties);
        }
    } else {
        ALOGE("getProperties error %s", hidlReturn.description().c_str());
        return FAILED_TRANSACTION;
    }
    ALOGI("getProperties ret %d", ret);
    return ret;
}

int SoundTriggerHalHidl::loadSoundModel(struct sound_trigger_sound_model *sound_model,
                        sound_model_callback_t callback,
                        void *cookie,
                        sound_model_handle_t *handle)
{
    if (handle == NULL) {
        return -EINVAL;
    }

    sp<ISoundTriggerHw> soundtrigger = getService();
    if (soundtrigger == 0) {
        return -ENODEV;
    }

    uint32_t modelId;
    {
        AutoMutex lock(mLock);
        do {
            modelId = nextUniqueId();
            ALOGI("loadSoundModel modelId %u", modelId);
            sp<SoundModel> model = mSoundModels.valueFor(modelId);
            ALOGI("loadSoundModel model %p", model.get());
        } while (mSoundModels.valueFor(modelId) != 0 && modelId != 0);
    }
    LOG_ALWAYS_FATAL_IF(modelId == 0,
                        "loadSoundModel(): wrap around in sound model IDs, num loaded models %zd",
                        mSoundModels.size());

    Return<void> hidlReturn;
    int ret;
    SoundModelHandle halHandle;
    sp<V2_1_ISoundTriggerHw> soundtrigger_2_1 = toService2_1(soundtrigger);
    sp<V2_2_ISoundTriggerHw> soundtrigger_2_2 = toService2_2(soundtrigger);
    if (sound_model->type == SOUND_MODEL_TYPE_KEYPHRASE) {
        if (soundtrigger_2_2) {
            V2_2_ISoundTriggerHw::PhraseSoundModel halSoundModel;
            auto result = convertPhraseSoundModelToHal(&halSoundModel, sound_model);
            if (result.first) {
                AutoMutex lock(mHalLock);
                hidlReturn = soundtrigger_2_2->loadPhraseSoundModel_2_1(
                        halSoundModel,
                        this, modelId, [&](int32_t retval, auto res) {
                            ret = retval;
                            halHandle = res;
                        });
            } else {
                return NO_MEMORY;
            }
        } else if (soundtrigger_2_1) {
            V2_1_ISoundTriggerHw::PhraseSoundModel halSoundModel;
            auto result = convertPhraseSoundModelToHal(&halSoundModel, sound_model);
            if (result.first) {
                AutoMutex lock(mHalLock);
                hidlReturn = soundtrigger_2_1->loadPhraseSoundModel_2_1(
                        halSoundModel,
                        this, modelId, [&](int32_t retval, auto res) {
                            ret = retval;
                            halHandle = res;
                        });
            } else {
                return NO_MEMORY;
            }
        } else {
            ISoundTriggerHw::PhraseSoundModel halSoundModel;
            convertPhraseSoundModelToHal(&halSoundModel, sound_model);
            AutoMutex lock(mHalLock);
            hidlReturn = soundtrigger->loadPhraseSoundModel(
                    halSoundModel,
                    this, modelId, [&](int32_t retval, auto res) {
                        ret = retval;
                        halHandle = res;
                    });
        }
    } else {
        if (soundtrigger_2_2) {
            V2_2_ISoundTriggerHw::SoundModel halSoundModel;
            auto result = convertSoundModelToHal(&halSoundModel, sound_model);
            if (result.first) {
                AutoMutex lock(mHalLock);
                hidlReturn = soundtrigger_2_2->loadSoundModel_2_1(halSoundModel,
                        this, modelId, [&](int32_t retval, auto res) {
                            ret = retval;
                            halHandle = res;
                        });
            } else {
                return NO_MEMORY;
            }
        } else if (soundtrigger_2_1) {
            V2_1_ISoundTriggerHw::SoundModel halSoundModel;
            auto result = convertSoundModelToHal(&halSoundModel, sound_model);
            if (result.first) {
                AutoMutex lock(mHalLock);
                hidlReturn = soundtrigger_2_1->loadSoundModel_2_1(halSoundModel,
                        this, modelId, [&](int32_t retval, auto res) {
                            ret = retval;
                            halHandle = res;
                        });
            } else {
                return NO_MEMORY;
            }
        } else {
            ISoundTriggerHw::SoundModel halSoundModel;
            convertSoundModelToHal(&halSoundModel, sound_model);
            AutoMutex lock(mHalLock);
            hidlReturn = soundtrigger->loadSoundModel(halSoundModel,
                    this, modelId, [&](int32_t retval, auto res) {
                        ret = retval;
                        halHandle = res;
                    });
        }
    }

    if (hidlReturn.isOk()) {
        if (ret == 0) {
            AutoMutex lock(mLock);
            *handle = (sound_model_handle_t)modelId;
            sp<SoundModel> model = new SoundModel(*handle, callback, cookie, halHandle);
            mSoundModels.add(*handle, model);
        }
    } else {
        ALOGE("loadSoundModel error %s", hidlReturn.description().c_str());
        return FAILED_TRANSACTION;
    }

    return ret;
}

int SoundTriggerHalHidl::unloadSoundModel(sound_model_handle_t handle)
{
    sp<ISoundTriggerHw> soundtrigger = getService();
    if (soundtrigger == 0) {
        return -ENODEV;
    }

    sp<SoundModel> model = removeModel(handle);
    if (model == 0) {
        ALOGE("unloadSoundModel model not found for handle %u", handle);
        return -EINVAL;
    }

    Return<int32_t> hidlReturn(0);
    {
        AutoMutex lock(mHalLock);
        hidlReturn = soundtrigger->unloadSoundModel(model->mHalHandle);
    }

    if (!hidlReturn.isOk()) {
        ALOGE("unloadSoundModel error %s", hidlReturn.description().c_str());
        return FAILED_TRANSACTION;
    }

    return hidlReturn;
}

int SoundTriggerHalHidl::startRecognition(sound_model_handle_t handle,
                         const struct sound_trigger_recognition_config *config,
                         recognition_callback_t callback,
                         void *cookie)
{
    sp<ISoundTriggerHw> soundtrigger = getService();
    if (soundtrigger == 0) {
        return -ENODEV;
    }

    sp<SoundModel> model = getModel(handle);
    if (model == 0) {
        ALOGE("startRecognition model not found for handle %u", handle);
        return -EINVAL;
    }

    model->mRecognitionCallback = callback;
    model->mRecognitionCookie = cookie;

    sp<V2_1_ISoundTriggerHw> soundtrigger_2_1 = toService2_1(soundtrigger);
    sp<V2_2_ISoundTriggerHw> soundtrigger_2_2 = toService2_2(soundtrigger);
    Return<int32_t> hidlReturn(0);

    if (soundtrigger_2_2) {
        V2_2_ISoundTriggerHw::RecognitionConfig halConfig;
        auto result = convertRecognitionConfigToHal(&halConfig, config);
        if (result.first) {
            AutoMutex lock(mHalLock);
            hidlReturn = soundtrigger_2_2->startRecognition_2_1(
                    model->mHalHandle, halConfig, this, handle);
        } else {
            return NO_MEMORY;
        }
    } else if (soundtrigger_2_1) {
        V2_1_ISoundTriggerHw::RecognitionConfig halConfig;
        auto result = convertRecognitionConfigToHal(&halConfig, config);
        if (result.first) {
            AutoMutex lock(mHalLock);
            hidlReturn = soundtrigger_2_1->startRecognition_2_1(
                    model->mHalHandle, halConfig, this, handle);
        } else {
            return NO_MEMORY;
        }
    } else {
        ISoundTriggerHw::RecognitionConfig halConfig;
        convertRecognitionConfigToHal(&halConfig, config);
        {
            AutoMutex lock(mHalLock);
            hidlReturn = soundtrigger->startRecognition(model->mHalHandle, halConfig, this, handle);
        }
    }

    if (!hidlReturn.isOk()) {
        ALOGE("startRecognition error %s", hidlReturn.description().c_str());
        return FAILED_TRANSACTION;
    }
    return hidlReturn;
}

int SoundTriggerHalHidl::stopRecognition(sound_model_handle_t handle)
{
    sp<ISoundTriggerHw> soundtrigger = getService();
    if (soundtrigger == 0) {
        return -ENODEV;
    }

    sp<SoundModel> model = getModel(handle);
    if (model == 0) {
        ALOGE("stopRecognition model not found for handle %u", handle);
        return -EINVAL;
    }

    Return<int32_t> hidlReturn(0);
    {
        AutoMutex lock(mHalLock);
        hidlReturn = soundtrigger->stopRecognition(model->mHalHandle);
    }

    if (!hidlReturn.isOk()) {
        ALOGE("stopRecognition error %s", hidlReturn.description().c_str());
        return FAILED_TRANSACTION;
    }
    return hidlReturn;
}

int SoundTriggerHalHidl::stopAllRecognitions()
{
    sp<ISoundTriggerHw> soundtrigger = getService();
    if (soundtrigger == 0) {
        return -ENODEV;
    }

    Return<int32_t> hidlReturn(0);
    {
        AutoMutex lock(mHalLock);
        hidlReturn = soundtrigger->stopAllRecognitions();
    }

    if (!hidlReturn.isOk()) {
        ALOGE("stopAllRecognitions error %s", hidlReturn.description().c_str());
        return FAILED_TRANSACTION;
    }
    return hidlReturn;
}

int SoundTriggerHalHidl::getModelState(sound_model_handle_t handle)
{
    sp<ISoundTriggerHw> soundtrigger = getService();
    if (soundtrigger == 0) {
        return -ENODEV;
    }

    sp<V2_2_ISoundTriggerHw> soundtrigger_2_2 = toService2_2(soundtrigger);
    if (soundtrigger_2_2 == 0) {
        ALOGE("getModelState not supported");
        return -ENODEV;
    }

    sp<SoundModel> model = getModel(handle);
    if (model == 0) {
        ALOGE("getModelState model not found for handle %u", handle);
        return -EINVAL;
    }

    int ret = NO_ERROR;
    Return<int32_t> hidlReturn(0);
    {
        AutoMutex lock(mHalLock);
        hidlReturn = soundtrigger_2_2->getModelState(model->mHalHandle);
    }
    if (!hidlReturn.isOk()) {
        ALOGE("getModelState error %s", hidlReturn.description().c_str());
        ret = FAILED_TRANSACTION;
    }
    return ret;
}

SoundTriggerHalHidl::SoundTriggerHalHidl(const char *moduleName)
    : mModuleName(moduleName), mNextUniqueId(1)
{
    LOG_ALWAYS_FATAL_IF(strcmp(mModuleName, "primary") != 0,
            "Treble soundtrigger only supports primary module");
}

SoundTriggerHalHidl::~SoundTriggerHalHidl()
{
}

sp<ISoundTriggerHw> SoundTriggerHalHidl::getService()
{
    AutoMutex lock(mLock);
    if (mISoundTrigger == 0) {
        if (mModuleName == NULL) {
            mModuleName = "primary";
        }
        mISoundTrigger = ISoundTriggerHw::getService();
        if (mISoundTrigger != 0) {
            mISoundTrigger->linkToDeath(HalDeathHandler::getInstance(), 0 /*cookie*/);
        }
    }
    return mISoundTrigger;
}

sp<V2_1_ISoundTriggerHw> SoundTriggerHalHidl::toService2_1(const sp<ISoundTriggerHw>& s)
{
    auto castResult_2_1 = V2_1_ISoundTriggerHw::castFrom(s);
    return castResult_2_1.isOk() ? static_cast<sp<V2_1_ISoundTriggerHw>>(castResult_2_1) : nullptr;
}

sp<V2_2_ISoundTriggerHw> SoundTriggerHalHidl::toService2_2(const sp<ISoundTriggerHw>& s)
{
    auto castResult_2_2 = V2_2_ISoundTriggerHw::castFrom(s);
    return castResult_2_2.isOk() ? static_cast<sp<V2_2_ISoundTriggerHw>>(castResult_2_2) : nullptr;
}

sp<SoundTriggerHalHidl::SoundModel> SoundTriggerHalHidl::getModel(sound_model_handle_t handle)
{
    AutoMutex lock(mLock);
    return mSoundModels.valueFor(handle);
}

sp<SoundTriggerHalHidl::SoundModel> SoundTriggerHalHidl::removeModel(sound_model_handle_t handle)
{
    AutoMutex lock(mLock);
    sp<SoundModel> model = mSoundModels.valueFor(handle);
    mSoundModels.removeItem(handle);
    return model;
}

uint32_t SoundTriggerHalHidl::nextUniqueId()
{
    return (uint32_t) atomic_fetch_add_explicit(&mNextUniqueId,
                (uint_fast32_t) 1, memory_order_acq_rel);
}

void SoundTriggerHalHidl::convertUuidToHal(Uuid *halUuid,
                                           const sound_trigger_uuid_t *uuid)
{
    halUuid->timeLow = uuid->timeLow;
    halUuid->timeMid = uuid->timeMid;
    halUuid->versionAndTimeHigh = uuid->timeHiAndVersion;
    halUuid->variantAndClockSeqHigh = uuid->clockSeq;
    memcpy(halUuid->node.data(), &uuid->node[0], sizeof(uuid->node));
}

void SoundTriggerHalHidl::convertUuidFromHal(sound_trigger_uuid_t *uuid,
                                             const Uuid *halUuid)
{
    uuid->timeLow = halUuid->timeLow;
    uuid->timeMid = halUuid->timeMid;
    uuid->timeHiAndVersion = halUuid->versionAndTimeHigh;
    uuid->clockSeq = halUuid->variantAndClockSeqHigh;
    memcpy(&uuid->node[0], halUuid->node.data(), sizeof(uuid->node));
}

void SoundTriggerHalHidl::convertPropertiesFromHal(
        struct sound_trigger_properties *properties,
        const ISoundTriggerHw::Properties *halProperties)
{
    strlcpy(properties->implementor,
            halProperties->implementor.c_str(), SOUND_TRIGGER_MAX_STRING_LEN);
    strlcpy(properties->description,
            halProperties->description.c_str(), SOUND_TRIGGER_MAX_STRING_LEN);
    properties->version = halProperties->version;
    convertUuidFromHal(&properties->uuid, &halProperties->uuid);
    properties->max_sound_models = halProperties->maxSoundModels;
    properties->max_key_phrases = halProperties->maxKeyPhrases;
    properties->max_users = halProperties->maxUsers;
    properties->recognition_modes = halProperties->recognitionModes;
    properties->capture_transition = (bool)halProperties->captureTransition;
    properties->max_buffer_ms = halProperties->maxBufferMs;
    properties->concurrent_capture = (bool)halProperties->concurrentCapture;
    properties->trigger_in_event = (bool)halProperties->triggerInEvent;
    properties->power_consumption_mw = halProperties->powerConsumptionMw;
}

void SoundTriggerHalHidl::convertTriggerPhraseToHal(
        ISoundTriggerHw::Phrase *halTriggerPhrase,
        const struct sound_trigger_phrase *triggerPhrase)
{
    halTriggerPhrase->id = triggerPhrase->id;
    halTriggerPhrase->recognitionModes = triggerPhrase->recognition_mode;
    halTriggerPhrase->users.setToExternal((uint32_t *)&triggerPhrase->users[0], triggerPhrase->num_users);
    halTriggerPhrase->locale = triggerPhrase->locale;
    halTriggerPhrase->text = triggerPhrase->text;
}


void SoundTriggerHalHidl::convertTriggerPhrasesToHal(
        hidl_vec<ISoundTriggerHw::Phrase> *halTriggerPhrases,
        struct sound_trigger_phrase_sound_model *keyPhraseModel)
{
    halTriggerPhrases->resize(keyPhraseModel->num_phrases);
    for (unsigned int i = 0; i < keyPhraseModel->num_phrases; i++) {
        convertTriggerPhraseToHal(&(*halTriggerPhrases)[i], &keyPhraseModel->phrases[i]);
    }
}

void SoundTriggerHalHidl::convertSoundModelToHal(ISoundTriggerHw::SoundModel *halModel,
        const struct sound_trigger_sound_model *soundModel)
{
    halModel->type = (SoundModelType)soundModel->type;
    convertUuidToHal(&halModel->uuid, &soundModel->uuid);
    convertUuidToHal(&halModel->vendorUuid, &soundModel->vendor_uuid);
    halModel->data.setToExternal((uint8_t *)soundModel + soundModel->data_offset, soundModel->data_size);
}

std::pair<bool, sp<IMemory>> SoundTriggerHalHidl::convertSoundModelToHal(
        V2_1_ISoundTriggerHw::SoundModel *halModel,
        const struct sound_trigger_sound_model *soundModel)
{
    convertSoundModelToHal(&halModel->header, soundModel);
    return moveVectorToMemory(&halModel->header.data, &halModel->data);
}

void SoundTriggerHalHidl::convertPhraseSoundModelToHal(
        ISoundTriggerHw::PhraseSoundModel *halKeyPhraseModel,
        const struct sound_trigger_sound_model *soundModel)
{
    struct sound_trigger_phrase_sound_model *keyPhraseModel =
            (struct sound_trigger_phrase_sound_model *)soundModel;
    convertTriggerPhrasesToHal(&halKeyPhraseModel->phrases, keyPhraseModel);
    convertSoundModelToHal(&halKeyPhraseModel->common, soundModel);
}

std::pair<bool, sp<IMemory>> SoundTriggerHalHidl::convertPhraseSoundModelToHal(
        V2_1_ISoundTriggerHw::PhraseSoundModel *halKeyPhraseModel,
        const struct sound_trigger_sound_model *soundModel)
{
    struct sound_trigger_phrase_sound_model *keyPhraseModel =
            (struct sound_trigger_phrase_sound_model *)soundModel;
    convertTriggerPhrasesToHal(&halKeyPhraseModel->phrases, keyPhraseModel);
    return convertSoundModelToHal(&halKeyPhraseModel->common, soundModel);
}

void SoundTriggerHalHidl::convertPhraseRecognitionExtraToHal(
        PhraseRecognitionExtra *halExtra,
        const struct sound_trigger_phrase_recognition_extra *extra)
{
    halExtra->id = extra->id;
    halExtra->recognitionModes = extra->recognition_modes;
    halExtra->confidenceLevel = extra->confidence_level;
    halExtra->levels.resize(extra->num_levels);
    for (unsigned int i = 0; i < extra->num_levels; i++) {
        halExtra->levels[i].userId = extra->levels[i].user_id;
        halExtra->levels[i].levelPercent = extra->levels[i].level;
    }
}

void SoundTriggerHalHidl::convertRecognitionConfigToHal(
        ISoundTriggerHw::RecognitionConfig *halConfig,
        const struct sound_trigger_recognition_config *config)
{
    halConfig->captureHandle = config->capture_handle;
    halConfig->captureDevice = (AudioDevice)config->capture_device;
    halConfig->captureRequested = (uint32_t)config->capture_requested;

    halConfig->phrases.resize(config->num_phrases);
    for (unsigned int i = 0; i < config->num_phrases; i++) {
        convertPhraseRecognitionExtraToHal(&halConfig->phrases[i],
                                  &config->phrases[i]);
    }

    halConfig->data.setToExternal((uint8_t *)config + config->data_offset, config->data_size);
}

std::pair<bool, sp<IMemory>> SoundTriggerHalHidl::convertRecognitionConfigToHal(
        V2_1_ISoundTriggerHw::RecognitionConfig *halConfig,
        const struct sound_trigger_recognition_config *config)
{
    convertRecognitionConfigToHal(&halConfig->header, config);
    return moveVectorToMemory(&halConfig->header.data, &halConfig->data);
}


// ISoundTriggerHwCallback
::android::hardware::Return<void> SoundTriggerHalHidl::recognitionCallback(
        const V2_0_ISoundTriggerHwCallback::RecognitionEvent& halEvent,
        CallbackCookie cookie)
{
    sp<SoundModel> model;
    {
        AutoMutex lock(mLock);
        model = mSoundModels.valueFor((SoundModelHandle)cookie);
        if (model == 0) {
            return Return<void>();
        }
    }
    struct sound_trigger_recognition_event *event = convertRecognitionEventFromHal(&halEvent);
    if (event == NULL) {
        return Return<void>();
    }
    event->model = model->mHandle;
    model->mRecognitionCallback(event, model->mRecognitionCookie);

    free(event);

    return Return<void>();
}

::android::hardware::Return<void> SoundTriggerHalHidl::phraseRecognitionCallback(
        const V2_0_ISoundTriggerHwCallback::PhraseRecognitionEvent& halEvent,
        CallbackCookie cookie)
{
    sp<SoundModel> model;
    {
        AutoMutex lock(mLock);
        model = mSoundModels.valueFor((SoundModelHandle)cookie);
        if (model == 0) {
            return Return<void>();
        }
    }

    struct sound_trigger_phrase_recognition_event *event =
            convertPhraseRecognitionEventFromHal(&halEvent);
    if (event == NULL) {
        return Return<void>();
    }
    event->common.model = model->mHandle;
    model->mRecognitionCallback(&event->common, model->mRecognitionCookie);

    free(event);

    return Return<void>();
}

::android::hardware::Return<void> SoundTriggerHalHidl::soundModelCallback(
        const V2_0_ISoundTriggerHwCallback::ModelEvent& halEvent,
        CallbackCookie cookie)
{
    sp<SoundModel> model;
    {
        AutoMutex lock(mLock);
        model = mSoundModels.valueFor((SoundModelHandle)cookie);
        if (model == 0) {
            return Return<void>();
        }
    }

    struct sound_trigger_model_event *event = convertSoundModelEventFromHal(&halEvent);
    if (event == NULL) {
        return Return<void>();
    }

    event->model = model->mHandle;
    model->mSoundModelCallback(event, model->mSoundModelCookie);

    free(event);

    return Return<void>();
}

::android::hardware::Return<void> SoundTriggerHalHidl::recognitionCallback_2_1(
        const ISoundTriggerHwCallback::RecognitionEvent& event, CallbackCookie cookie) {
    // The data vector in the 'header' part of V2.1 structure is empty, thus copying is cheap.
    V2_0_ISoundTriggerHwCallback::RecognitionEvent event_2_0 = event.header;
    auto result = memoryAsVector(event.data, &event_2_0.data);
    return result.first ? recognitionCallback(event_2_0, cookie) : Void();
}

::android::hardware::Return<void> SoundTriggerHalHidl::phraseRecognitionCallback_2_1(
        const ISoundTriggerHwCallback::PhraseRecognitionEvent& event, int32_t cookie) {
    V2_0_ISoundTriggerHwCallback::PhraseRecognitionEvent event_2_0;
    // The data vector in the 'header' part of V2.1 structure is empty, thus copying is cheap.
    event_2_0.common = event.common.header;
    event_2_0.phraseExtras.setToExternal(
            const_cast<PhraseRecognitionExtra*>(event.phraseExtras.data()),
            event.phraseExtras.size());
    auto result = memoryAsVector(event.common.data, &event_2_0.common.data);
    return result.first ? phraseRecognitionCallback(event_2_0, cookie) : Void();
}

::android::hardware::Return<void> SoundTriggerHalHidl::soundModelCallback_2_1(
        const ISoundTriggerHwCallback::ModelEvent& event, CallbackCookie cookie) {
    // The data vector in the 'header' part of V2.1 structure is empty, thus copying is cheap.
    V2_0_ISoundTriggerHwCallback::ModelEvent event_2_0 = event.header;
    auto result = memoryAsVector(event.data, &event_2_0.data);
    return result.first ? soundModelCallback(event_2_0, cookie) : Void();
}


struct sound_trigger_model_event *SoundTriggerHalHidl::convertSoundModelEventFromHal(
                                              const V2_0_ISoundTriggerHwCallback::ModelEvent *halEvent)
{
    struct sound_trigger_model_event *event = (struct sound_trigger_model_event *)malloc(
            sizeof(struct sound_trigger_model_event) +
            halEvent->data.size());
    if (event == NULL) {
        return NULL;
    }

    event->status = (int)halEvent->status;
    // event->model to be set by caller
    event->data_offset = sizeof(struct sound_trigger_model_event);
    event->data_size = halEvent->data.size();
    uint8_t *dst = (uint8_t *)event + event->data_offset;
    uint8_t *src = (uint8_t *)&halEvent->data[0];
    memcpy(dst, src, halEvent->data.size());

    return event;
}

void SoundTriggerHalHidl::convertPhraseRecognitionExtraFromHal(
        struct sound_trigger_phrase_recognition_extra *extra,
        const PhraseRecognitionExtra *halExtra)
{
    extra->id = halExtra->id;
    extra->recognition_modes = halExtra->recognitionModes;
    extra->confidence_level = halExtra->confidenceLevel;

    size_t i;
    for (i = 0; i < halExtra->levels.size() && i < SOUND_TRIGGER_MAX_USERS; i++) {
        extra->levels[i].user_id = halExtra->levels[i].userId;
        extra->levels[i].level = halExtra->levels[i].levelPercent;
    }
    extra->num_levels = (unsigned int)i;
}


struct sound_trigger_phrase_recognition_event* SoundTriggerHalHidl::convertPhraseRecognitionEventFromHal(
        const V2_0_ISoundTriggerHwCallback::PhraseRecognitionEvent *halPhraseEvent)
{
    if (halPhraseEvent->common.type != SoundModelType::KEYPHRASE) {
        ALOGE("Received non-keyphrase event type as PhraseRecognitionEvent");
        return NULL;
    }
    struct sound_trigger_phrase_recognition_event *phraseEvent =
            (struct sound_trigger_phrase_recognition_event *)malloc(
                    sizeof(struct sound_trigger_phrase_recognition_event) +
                    halPhraseEvent->common.data.size());
    if (phraseEvent == NULL) {
        return NULL;
    }
    phraseEvent->common.data_offset = sizeof(sound_trigger_phrase_recognition_event);

    for (unsigned int i = 0; i < halPhraseEvent->phraseExtras.size(); i++) {
        convertPhraseRecognitionExtraFromHal(&phraseEvent->phrase_extras[i],
                                             &halPhraseEvent->phraseExtras[i]);
    }
    phraseEvent->num_phrases = halPhraseEvent->phraseExtras.size();

    fillRecognitionEventFromHal(&phraseEvent->common, &halPhraseEvent->common);
    return phraseEvent;
}

struct sound_trigger_recognition_event *SoundTriggerHalHidl::convertRecognitionEventFromHal(
        const V2_0_ISoundTriggerHwCallback::RecognitionEvent *halEvent)
{
    if (halEvent->type == SoundModelType::KEYPHRASE) {
        ALOGE("Received keyphrase event type as RecognitionEvent");
        return NULL;
    }
    struct sound_trigger_recognition_event *event;
    event = (struct sound_trigger_recognition_event *)malloc(
            sizeof(struct sound_trigger_recognition_event) + halEvent->data.size());
    if (event == NULL) {
        return NULL;
    }
    event->data_offset = sizeof(sound_trigger_recognition_event);

    fillRecognitionEventFromHal(event, halEvent);
    return event;
}

void SoundTriggerHalHidl::fillRecognitionEventFromHal(
        struct sound_trigger_recognition_event *event,
        const V2_0_ISoundTriggerHwCallback::RecognitionEvent *halEvent)
{
    event->status = (int)halEvent->status;
    event->type = (sound_trigger_sound_model_type_t)halEvent->type;
    // event->model to be set by caller
    event->capture_available = (bool)halEvent->captureAvailable;
    event->capture_session = halEvent->captureSession;
    event->capture_delay_ms = halEvent->captureDelayMs;
    event->capture_preamble_ms = halEvent->capturePreambleMs;
    event->trigger_in_data = (bool)halEvent->triggerInData;
    event->audio_config.sample_rate = halEvent->audioConfig.sampleRateHz;
    event->audio_config.channel_mask = (audio_channel_mask_t)halEvent->audioConfig.channelMask;
    event->audio_config.format = (audio_format_t)halEvent->audioConfig.format;

    event->data_size = halEvent->data.size();
    uint8_t *dst = (uint8_t *)event + event->data_offset;
    uint8_t *src = (uint8_t *)&halEvent->data[0];
    memcpy(dst, src, halEvent->data.size());
}

} // namespace android
