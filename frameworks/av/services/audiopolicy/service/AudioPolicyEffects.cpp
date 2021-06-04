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

#define LOG_TAG "AudioPolicyEffects"
//#define LOG_NDEBUG 0

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <cutils/misc.h>
#include <media/AudioEffect.h>
#include <media/EffectsConfig.h>
#include <mediautils/ServiceUtilities.h>
#include <system/audio.h>
#include <system/audio_effects/audio_effects_conf.h>
#include <utils/Vector.h>
#include <utils/SortedVector.h>
#include <cutils/config_utils.h>
#include <binder/IPCThreadState.h>
#include "AudioPolicyEffects.h"

namespace android {

// ----------------------------------------------------------------------------
// AudioPolicyEffects Implementation
// ----------------------------------------------------------------------------

AudioPolicyEffects::AudioPolicyEffects()
{
    status_t loadResult = loadAudioEffectXmlConfig();
    if (loadResult < 0) {
        ALOGW("Failed to load XML effect configuration, fallback to .conf");
        // load automatic audio effect modules
        if (access(AUDIO_EFFECT_VENDOR_CONFIG_FILE, R_OK) == 0) {
            loadAudioEffectConfig(AUDIO_EFFECT_VENDOR_CONFIG_FILE);
        } else if (access(AUDIO_EFFECT_DEFAULT_CONFIG_FILE, R_OK) == 0) {
            loadAudioEffectConfig(AUDIO_EFFECT_DEFAULT_CONFIG_FILE);
        }
    } else if (loadResult > 0) {
        ALOGE("Effect config is partially invalid, skipped %d elements", loadResult);
    }
}


AudioPolicyEffects::~AudioPolicyEffects()
{
    size_t i = 0;
    // release audio input processing resources
    for (i = 0; i < mInputSources.size(); i++) {
        delete mInputSources.valueAt(i);
    }
    mInputSources.clear();

    for (i = 0; i < mInputSessions.size(); i++) {
        mInputSessions.valueAt(i)->mEffects.clear();
        delete mInputSessions.valueAt(i);
    }
    mInputSessions.clear();

    // release audio output processing resources
    for (i = 0; i < mOutputStreams.size(); i++) {
        delete mOutputStreams.valueAt(i);
    }
    mOutputStreams.clear();

    for (i = 0; i < mOutputSessions.size(); i++) {
        mOutputSessions.valueAt(i)->mEffects.clear();
        delete mOutputSessions.valueAt(i);
    }
    mOutputSessions.clear();
}


status_t AudioPolicyEffects::addInputEffects(audio_io_handle_t input,
                             audio_source_t inputSource,
                             audio_session_t audioSession)
{
    status_t status = NO_ERROR;

    // create audio pre processors according to input source
    audio_source_t aliasSource = (inputSource == AUDIO_SOURCE_HOTWORD) ?
                                    AUDIO_SOURCE_VOICE_RECOGNITION : inputSource;

    Mutex::Autolock _l(mLock);
    ssize_t index = mInputSources.indexOfKey(aliasSource);
    if (index < 0) {
        ALOGV("addInputEffects(): no processing needs to be attached to this source");
        return status;
    }
    ssize_t idx = mInputSessions.indexOfKey(audioSession);
    EffectVector *sessionDesc;
    if (idx < 0) {
        sessionDesc = new EffectVector(audioSession);
        mInputSessions.add(audioSession, sessionDesc);
    } else {
        // EffectVector is existing and we just need to increase ref count
        sessionDesc = mInputSessions.valueAt(idx);
    }
    sessionDesc->mRefCount++;

    ALOGV("addInputEffects(): input: %d, refCount: %d", input, sessionDesc->mRefCount);
    if (sessionDesc->mRefCount == 1) {
        int64_t token = IPCThreadState::self()->clearCallingIdentity();
        Vector <EffectDesc *> effects = mInputSources.valueAt(index)->mEffects;
        for (size_t i = 0; i < effects.size(); i++) {
            EffectDesc *effect = effects[i];
            sp<AudioEffect> fx = new AudioEffect(NULL, String16("android"), &effect->mUuid, -1, 0,
                                                 0, audioSession, input);
            status_t status = fx->initCheck();
            if (status != NO_ERROR && status != ALREADY_EXISTS) {
                ALOGW("addInputEffects(): failed to create Fx %s on source %d",
                      effect->mName, (int32_t)aliasSource);
                // fx goes out of scope and strong ref on AudioEffect is released
                continue;
            }
            for (size_t j = 0; j < effect->mParams.size(); j++) {
                fx->setParameter(effect->mParams[j]);
            }
            ALOGV("addInputEffects(): added Fx %s on source: %d",
                  effect->mName, (int32_t)aliasSource);
            sessionDesc->mEffects.add(fx);
        }
        sessionDesc->setProcessorEnabled(true);
        IPCThreadState::self()->restoreCallingIdentity(token);
    }
    return status;
}


status_t AudioPolicyEffects::releaseInputEffects(audio_io_handle_t input,
                                                 audio_session_t audioSession)
{
    status_t status = NO_ERROR;

    Mutex::Autolock _l(mLock);
    ssize_t index = mInputSessions.indexOfKey(audioSession);
    if (index < 0) {
        return status;
    }
    EffectVector *sessionDesc = mInputSessions.valueAt(index);
    sessionDesc->mRefCount--;
    ALOGV("releaseInputEffects(): input: %d, refCount: %d", input, sessionDesc->mRefCount);
    if (sessionDesc->mRefCount == 0) {
        sessionDesc->setProcessorEnabled(false);
        delete sessionDesc;
        mInputSessions.removeItemsAt(index);
        ALOGV("releaseInputEffects(): all effects released");
    }
    return status;
}

status_t AudioPolicyEffects::queryDefaultInputEffects(audio_session_t audioSession,
                                                      effect_descriptor_t *descriptors,
                                                      uint32_t *count)
{
    status_t status = NO_ERROR;

    Mutex::Autolock _l(mLock);
    size_t index;
    for (index = 0; index < mInputSessions.size(); index++) {
        if (mInputSessions.valueAt(index)->mSessionId == audioSession) {
            break;
        }
    }
    if (index == mInputSessions.size()) {
        *count = 0;
        return BAD_VALUE;
    }
    Vector< sp<AudioEffect> > effects = mInputSessions.valueAt(index)->mEffects;

    for (size_t i = 0; i < effects.size(); i++) {
        effect_descriptor_t desc = effects[i]->descriptor();
        if (i < *count) {
            descriptors[i] = desc;
        }
    }
    if (effects.size() > *count) {
        status = NO_MEMORY;
    }
    *count = effects.size();
    return status;
}


status_t AudioPolicyEffects::queryDefaultOutputSessionEffects(audio_session_t audioSession,
                         effect_descriptor_t *descriptors,
                         uint32_t *count)
{
    status_t status = NO_ERROR;

    Mutex::Autolock _l(mLock);
    size_t index;
    for (index = 0; index < mOutputSessions.size(); index++) {
        if (mOutputSessions.valueAt(index)->mSessionId == audioSession) {
            break;
        }
    }
    if (index == mOutputSessions.size()) {
        *count = 0;
        return BAD_VALUE;
    }
    Vector< sp<AudioEffect> > effects = mOutputSessions.valueAt(index)->mEffects;

    for (size_t i = 0; i < effects.size(); i++) {
        effect_descriptor_t desc = effects[i]->descriptor();
        if (i < *count) {
            descriptors[i] = desc;
        }
    }
    if (effects.size() > *count) {
        status = NO_MEMORY;
    }
    *count = effects.size();
    return status;
}


status_t AudioPolicyEffects::addOutputSessionEffects(audio_io_handle_t output,
                         audio_stream_type_t stream,
                         audio_session_t audioSession)
{
    status_t status = NO_ERROR;

    Mutex::Autolock _l(mLock);
    // create audio processors according to stream
    // FIXME: should we have specific post processing settings for internal streams?
    // default to media for now.
    if (stream >= AUDIO_STREAM_PUBLIC_CNT) {
        stream = AUDIO_STREAM_MUSIC;
    }
    ssize_t index = mOutputStreams.indexOfKey(stream);
    if (index < 0) {
        ALOGV("addOutputSessionEffects(): no output processing needed for this stream");
        return NO_ERROR;
    }

    ssize_t idx = mOutputSessions.indexOfKey(audioSession);
    EffectVector *procDesc;
    if (idx < 0) {
        procDesc = new EffectVector(audioSession);
        mOutputSessions.add(audioSession, procDesc);
    } else {
        // EffectVector is existing and we just need to increase ref count
        procDesc = mOutputSessions.valueAt(idx);
    }
    procDesc->mRefCount++;

    ALOGV("addOutputSessionEffects(): session: %d, refCount: %d",
          audioSession, procDesc->mRefCount);
    if (procDesc->mRefCount == 1) {
        // make sure effects are associated to audio server even if we are executing a binder call
        int64_t token = IPCThreadState::self()->clearCallingIdentity();
        Vector <EffectDesc *> effects = mOutputStreams.valueAt(index)->mEffects;
        for (size_t i = 0; i < effects.size(); i++) {
            EffectDesc *effect = effects[i];
            sp<AudioEffect> fx = new AudioEffect(NULL, String16("android"), &effect->mUuid, 0, 0, 0,
                                                 audioSession, output);
            status_t status = fx->initCheck();
            if (status != NO_ERROR && status != ALREADY_EXISTS) {
                ALOGE("addOutputSessionEffects(): failed to create Fx  %s on session %d",
                      effect->mName, audioSession);
                // fx goes out of scope and strong ref on AudioEffect is released
                continue;
            }
            ALOGV("addOutputSessionEffects(): added Fx %s on session: %d for stream: %d",
                  effect->mName, audioSession, (int32_t)stream);
            procDesc->mEffects.add(fx);
        }

        procDesc->setProcessorEnabled(true);
        IPCThreadState::self()->restoreCallingIdentity(token);
    }
    return status;
}

status_t AudioPolicyEffects::releaseOutputSessionEffects(audio_io_handle_t output,
                         audio_stream_type_t stream,
                         audio_session_t audioSession)
{
    status_t status = NO_ERROR;
    (void) output; // argument not used for now
    (void) stream; // argument not used for now

    Mutex::Autolock _l(mLock);
    ssize_t index = mOutputSessions.indexOfKey(audioSession);
    if (index < 0) {
        ALOGV("releaseOutputSessionEffects: no output processing was attached to this stream");
        return NO_ERROR;
    }

    EffectVector *procDesc = mOutputSessions.valueAt(index);
    procDesc->mRefCount--;
    ALOGV("releaseOutputSessionEffects(): session: %d, refCount: %d",
          audioSession, procDesc->mRefCount);
    if (procDesc->mRefCount == 0) {
        procDesc->setProcessorEnabled(false);
        procDesc->mEffects.clear();
        delete procDesc;
        mOutputSessions.removeItemsAt(index);
        ALOGV("releaseOutputSessionEffects(): output processing released from session: %d",
              audioSession);
    }
    return status;
}

status_t AudioPolicyEffects::addSourceDefaultEffect(const effect_uuid_t *type,
                                                    const String16& opPackageName,
                                                    const effect_uuid_t *uuid,
                                                    int32_t priority,
                                                    audio_source_t source,
                                                    audio_unique_id_t* id)
{
    if (uuid == NULL || type == NULL) {
        ALOGE("addSourceDefaultEffect(): Null uuid or type uuid pointer");
        return BAD_VALUE;
    }

    // HOTWORD, FM_TUNER and ECHO_REFERENCE are special case sources > MAX.
    if (source < AUDIO_SOURCE_DEFAULT ||
            (source > AUDIO_SOURCE_MAX &&
             source != AUDIO_SOURCE_HOTWORD &&
             source != AUDIO_SOURCE_FM_TUNER &&
             source != AUDIO_SOURCE_ECHO_REFERENCE)) {
        ALOGE("addSourceDefaultEffect(): Unsupported source type %d", source);
        return BAD_VALUE;
    }

    // Check that |uuid| or |type| corresponds to an effect on the system.
    effect_descriptor_t descriptor = {};
    status_t res = AudioEffect::getEffectDescriptor(
            uuid, type, EFFECT_FLAG_TYPE_PRE_PROC, &descriptor);
    if (res != OK) {
        ALOGE("addSourceDefaultEffect(): Failed to find effect descriptor matching uuid/type.");
        return res;
    }

    // Only pre-processing effects can be added dynamically as source defaults.
    if ((descriptor.flags & EFFECT_FLAG_TYPE_MASK) != EFFECT_FLAG_TYPE_PRE_PROC) {
        ALOGE("addSourceDefaultEffect(): Desired effect cannot be attached "
              "as a source default effect.");
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mLock);

    // Find the EffectDescVector for the given source type, or create a new one if necessary.
    ssize_t index = mInputSources.indexOfKey(source);
    EffectDescVector *desc = NULL;
    if (index < 0) {
        // No effects for this source type yet.
        desc = new EffectDescVector();
        mInputSources.add(source, desc);
    } else {
        desc = mInputSources.valueAt(index);
    }

    // Create a new effect and add it to the vector.
    res = AudioEffect::newEffectUniqueId(id);
    if (res != OK) {
        ALOGE("addSourceDefaultEffect(): failed to get new unique id.");
        return res;
    }
    EffectDesc *effect = new EffectDesc(
            descriptor.name, *type, opPackageName, *uuid, priority, *id);
    desc->mEffects.add(effect);
    // TODO(b/71813697): Support setting params as well.

    // TODO(b/71814300): Retroactively attach to any existing sources of the given type.
    // This requires tracking the source type of each session id in addition to what is
    // already being tracked.

    return NO_ERROR;
}

status_t AudioPolicyEffects::addStreamDefaultEffect(const effect_uuid_t *type,
                                                    const String16& opPackageName,
                                                    const effect_uuid_t *uuid,
                                                    int32_t priority,
                                                    audio_usage_t usage,
                                                    audio_unique_id_t* id)
{
    if (uuid == NULL || type == NULL) {
        ALOGE("addStreamDefaultEffect(): Null uuid or type uuid pointer");
        return BAD_VALUE;
    }
    audio_stream_type_t stream = AudioSystem::attributesToStreamType(attributes_initializer(usage));

    if (stream < AUDIO_STREAM_MIN || stream >= AUDIO_STREAM_PUBLIC_CNT) {
        ALOGE("addStreamDefaultEffect(): Unsupported stream type %d", stream);
        return BAD_VALUE;
    }

    // Check that |uuid| or |type| corresponds to an effect on the system.
    effect_descriptor_t descriptor = {};
    status_t res = AudioEffect::getEffectDescriptor(
            uuid, type, EFFECT_FLAG_TYPE_INSERT, &descriptor);
    if (res != OK) {
        ALOGE("addStreamDefaultEffect(): Failed to find effect descriptor matching uuid/type.");
        return res;
    }

    // Only insert effects can be added dynamically as stream defaults.
    if ((descriptor.flags & EFFECT_FLAG_TYPE_MASK) != EFFECT_FLAG_TYPE_INSERT) {
        ALOGE("addStreamDefaultEffect(): Desired effect cannot be attached "
              "as a stream default effect.");
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mLock);

    // Find the EffectDescVector for the given stream type, or create a new one if necessary.
    ssize_t index = mOutputStreams.indexOfKey(stream);
    EffectDescVector *desc = NULL;
    if (index < 0) {
        // No effects for this stream type yet.
        desc = new EffectDescVector();
        mOutputStreams.add(stream, desc);
    } else {
        desc = mOutputStreams.valueAt(index);
    }

    // Create a new effect and add it to the vector.
    res = AudioEffect::newEffectUniqueId(id);
    if (res != OK) {
        ALOGE("addStreamDefaultEffect(): failed to get new unique id.");
        return res;
    }
    EffectDesc *effect = new EffectDesc(
            descriptor.name, *type, opPackageName, *uuid, priority, *id);
    desc->mEffects.add(effect);
    // TODO(b/71813697): Support setting params as well.

    // TODO(b/71814300): Retroactively attach to any existing streams of the given type.
    // This requires tracking the stream type of each session id in addition to what is
    // already being tracked.

    return NO_ERROR;
}

status_t AudioPolicyEffects::removeSourceDefaultEffect(audio_unique_id_t id)
{
    if (id == AUDIO_UNIQUE_ID_ALLOCATE) {
        // ALLOCATE is not a unique identifier, but rather a reserved value indicating
        // a real id has not been assigned. For default effects, this value is only used
        // by system-owned defaults from the loaded config, which cannot be removed.
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mLock);

    // Check each source type.
    size_t numSources = mInputSources.size();
    for (size_t i = 0; i < numSources; ++i) {
        // Check each effect for each source.
        EffectDescVector* descVector = mInputSources[i];
        for (auto desc = descVector->mEffects.begin(); desc != descVector->mEffects.end(); ++desc) {
            if ((*desc)->mId == id) {
                // Found it!
                // TODO(b/71814300): Remove from any sources the effect was attached to.
                descVector->mEffects.erase(desc);
                // Handles are unique; there can only be one match, so return early.
                return NO_ERROR;
            }
        }
    }

    // Effect wasn't found, so it's been trivially removed successfully.
    return NO_ERROR;
}

status_t AudioPolicyEffects::removeStreamDefaultEffect(audio_unique_id_t id)
{
    if (id == AUDIO_UNIQUE_ID_ALLOCATE) {
        // ALLOCATE is not a unique identifier, but rather a reserved value indicating
        // a real id has not been assigned. For default effects, this value is only used
        // by system-owned defaults from the loaded config, which cannot be removed.
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mLock);

    // Check each stream type.
    size_t numStreams = mOutputStreams.size();
    for (size_t i = 0; i < numStreams; ++i) {
        // Check each effect for each stream.
        EffectDescVector* descVector = mOutputStreams[i];
        for (auto desc = descVector->mEffects.begin(); desc != descVector->mEffects.end(); ++desc) {
            if ((*desc)->mId == id) {
                // Found it!
                // TODO(b/71814300): Remove from any streams the effect was attached to.
                descVector->mEffects.erase(desc);
                // Handles are unique; there can only be one match, so return early.
                return NO_ERROR;
            }
        }
    }

    // Effect wasn't found, so it's been trivially removed successfully.
    return NO_ERROR;
}

void AudioPolicyEffects::EffectVector::setProcessorEnabled(bool enabled)
{
    for (size_t i = 0; i < mEffects.size(); i++) {
        mEffects.itemAt(i)->setEnabled(enabled);
    }
}


// ----------------------------------------------------------------------------
// Audio processing configuration
// ----------------------------------------------------------------------------

/*static*/ const char * const AudioPolicyEffects::kInputSourceNames[AUDIO_SOURCE_CNT -1] = {
    MIC_SRC_TAG,
    VOICE_UL_SRC_TAG,
    VOICE_DL_SRC_TAG,
    VOICE_CALL_SRC_TAG,
    CAMCORDER_SRC_TAG,
    VOICE_REC_SRC_TAG,
    VOICE_COMM_SRC_TAG,
    UNPROCESSED_SRC_TAG,
    VOICE_PERFORMANCE_SRC_TAG
};

// returns the audio_source_t enum corresponding to the input source name or
// AUDIO_SOURCE_CNT is no match found
/*static*/ audio_source_t AudioPolicyEffects::inputSourceNameToEnum(const char *name)
{
    int i;
    for (i = AUDIO_SOURCE_MIC; i < AUDIO_SOURCE_CNT; i++) {
        if (strcmp(name, kInputSourceNames[i - AUDIO_SOURCE_MIC]) == 0) {
            ALOGV("inputSourceNameToEnum found source %s %d", name, i);
            break;
        }
    }
    return (audio_source_t)i;
}

const char *AudioPolicyEffects::kStreamNames[AUDIO_STREAM_PUBLIC_CNT+1] = {
    AUDIO_STREAM_DEFAULT_TAG,
    AUDIO_STREAM_VOICE_CALL_TAG,
    AUDIO_STREAM_SYSTEM_TAG,
    AUDIO_STREAM_RING_TAG,
    AUDIO_STREAM_MUSIC_TAG,
    AUDIO_STREAM_ALARM_TAG,
    AUDIO_STREAM_NOTIFICATION_TAG,
    AUDIO_STREAM_BLUETOOTH_SCO_TAG,
    AUDIO_STREAM_ENFORCED_AUDIBLE_TAG,
    AUDIO_STREAM_DTMF_TAG,
    AUDIO_STREAM_TTS_TAG
};

// returns the audio_stream_t enum corresponding to the output stream name or
// AUDIO_STREAM_PUBLIC_CNT is no match found
audio_stream_type_t AudioPolicyEffects::streamNameToEnum(const char *name)
{
    int i;
    for (i = AUDIO_STREAM_DEFAULT; i < AUDIO_STREAM_PUBLIC_CNT; i++) {
        if (strcmp(name, kStreamNames[i - AUDIO_STREAM_DEFAULT]) == 0) {
            ALOGV("streamNameToEnum found stream %s %d", name, i);
            break;
        }
    }
    return (audio_stream_type_t)i;
}

// ----------------------------------------------------------------------------
// Audio Effect Config parser
// ----------------------------------------------------------------------------

size_t AudioPolicyEffects::growParamSize(char **param,
                                         size_t size,
                                         size_t *curSize,
                                         size_t *totSize)
{
    // *curSize is at least sizeof(effect_param_t) + 2 * sizeof(int)
    size_t pos = ((*curSize - 1 ) / size + 1) * size;

    if (pos + size > *totSize) {
        while (pos + size > *totSize) {
            *totSize += ((*totSize + 7) / 8) * 4;
        }
        char *newParam = (char *)realloc(*param, *totSize);
        if (newParam == NULL) {
            ALOGE("%s realloc error for size %zu", __func__, *totSize);
            return 0;
        }
        *param = newParam;
    }
    *curSize = pos + size;
    return pos;
}


size_t AudioPolicyEffects::readParamValue(cnode *node,
                                          char **param,
                                          size_t *curSize,
                                          size_t *totSize)
{
    size_t len = 0;
    size_t pos;

    if (strncmp(node->name, SHORT_TAG, sizeof(SHORT_TAG) + 1) == 0) {
        pos = growParamSize(param, sizeof(short), curSize, totSize);
        if (pos == 0) {
            goto exit;
        }
        *(short *)(*param + pos) = (short)atoi(node->value);
        ALOGV("readParamValue() reading short %d", *(short *)(*param + pos));
        len = sizeof(short);
    } else if (strncmp(node->name, INT_TAG, sizeof(INT_TAG) + 1) == 0) {
        pos = growParamSize(param, sizeof(int), curSize, totSize);
        if (pos == 0) {
            goto exit;
        }
        *(int *)(*param + pos) = atoi(node->value);
        ALOGV("readParamValue() reading int %d", *(int *)(*param + pos));
        len = sizeof(int);
    } else if (strncmp(node->name, FLOAT_TAG, sizeof(FLOAT_TAG) + 1) == 0) {
        pos = growParamSize(param, sizeof(float), curSize, totSize);
        if (pos == 0) {
            goto exit;
        }
        *(float *)(*param + pos) = (float)atof(node->value);
        ALOGV("readParamValue() reading float %f",*(float *)(*param + pos));
        len = sizeof(float);
    } else if (strncmp(node->name, BOOL_TAG, sizeof(BOOL_TAG) + 1) == 0) {
        pos = growParamSize(param, sizeof(bool), curSize, totSize);
        if (pos == 0) {
            goto exit;
        }
        if (strncmp(node->value, "true", strlen("true") + 1) == 0) {
            *(bool *)(*param + pos) = true;
        } else {
            *(bool *)(*param + pos) = false;
        }
        ALOGV("readParamValue() reading bool %s",
              *(bool *)(*param + pos) ? "true" : "false");
        len = sizeof(bool);
    } else if (strncmp(node->name, STRING_TAG, sizeof(STRING_TAG) + 1) == 0) {
        len = strnlen(node->value, EFFECT_STRING_LEN_MAX);
        if (*curSize + len + 1 > *totSize) {
            *totSize = *curSize + len + 1;
            char *newParam = (char *)realloc(*param, *totSize);
            if (newParam == NULL) {
                len = 0;
                ALOGE("%s realloc error for string len %zu", __func__, *totSize);
                goto exit;
            }
            *param = newParam;
        }
        strncpy(*param + *curSize, node->value, len);
        *curSize += len;
        (*param)[*curSize] = '\0';
        ALOGV("readParamValue() reading string %s", *param + *curSize - len);
    } else {
        ALOGW("readParamValue() unknown param type %s", node->name);
    }
exit:
    return len;
}

effect_param_t *AudioPolicyEffects::loadEffectParameter(cnode *root)
{
    cnode *param;
    cnode *value;
    size_t curSize = sizeof(effect_param_t);
    size_t totSize = sizeof(effect_param_t) + 2 * sizeof(int);
    effect_param_t *fx_param = (effect_param_t *)malloc(totSize);

    if (fx_param == NULL) {
        ALOGE("%s malloc error for effect structure of size %zu",
              __func__, totSize);
        return NULL;
    }

    param = config_find(root, PARAM_TAG);
    value = config_find(root, VALUE_TAG);
    if (param == NULL && value == NULL) {
        // try to parse simple parameter form {int int}
        param = root->first_child;
        if (param != NULL) {
            // Note: that a pair of random strings is read as 0 0
            int *ptr = (int *)fx_param->data;
#if LOG_NDEBUG == 0
            int *ptr2 = (int *)((char *)param + sizeof(effect_param_t));
            ALOGV("loadEffectParameter() ptr %p ptr2 %p", ptr, ptr2);
#endif
            *ptr++ = atoi(param->name);
            *ptr = atoi(param->value);
            fx_param->psize = sizeof(int);
            fx_param->vsize = sizeof(int);
            return fx_param;
        }
    }
    if (param == NULL || value == NULL) {
        ALOGW("loadEffectParameter() invalid parameter description %s",
              root->name);
        goto error;
    }

    fx_param->psize = 0;
    param = param->first_child;
    while (param) {
        ALOGV("loadEffectParameter() reading param of type %s", param->name);
        size_t size =
                readParamValue(param, (char **)&fx_param, &curSize, &totSize);
        if (size == 0) {
            goto error;
        }
        fx_param->psize += size;
        param = param->next;
    }

    // align start of value field on 32 bit boundary
    curSize = ((curSize - 1 ) / sizeof(int) + 1) * sizeof(int);

    fx_param->vsize = 0;
    value = value->first_child;
    while (value) {
        ALOGV("loadEffectParameter() reading value of type %s", value->name);
        size_t size =
                readParamValue(value, (char **)&fx_param, &curSize, &totSize);
        if (size == 0) {
            goto error;
        }
        fx_param->vsize += size;
        value = value->next;
    }

    return fx_param;

error:
    free(fx_param);
    return NULL;
}

void AudioPolicyEffects::loadEffectParameters(cnode *root, Vector <effect_param_t *>& params)
{
    cnode *node = root->first_child;
    while (node) {
        ALOGV("loadEffectParameters() loading param %s", node->name);
        effect_param_t *param = loadEffectParameter(node);
        if (param != NULL) {
            params.add(param);
        }
        node = node->next;
    }
}


AudioPolicyEffects::EffectDescVector *AudioPolicyEffects::loadEffectConfig(
                                                            cnode *root,
                                                            const Vector <EffectDesc *>& effects)
{
    cnode *node = root->first_child;
    if (node == NULL) {
        ALOGW("loadInputSource() empty element %s", root->name);
        return NULL;
    }
    EffectDescVector *desc = new EffectDescVector();
    while (node) {
        size_t i;

        for (i = 0; i < effects.size(); i++) {
            if (strncmp(effects[i]->mName, node->name, EFFECT_STRING_LEN_MAX) == 0) {
                ALOGV("loadEffectConfig() found effect %s in list", node->name);
                break;
            }
        }
        if (i == effects.size()) {
            ALOGV("loadEffectConfig() effect %s not in list", node->name);
            node = node->next;
            continue;
        }
        EffectDesc *effect = new EffectDesc(*effects[i]);   // deep copy
        loadEffectParameters(node, effect->mParams);
        ALOGV("loadEffectConfig() adding effect %s uuid %08x",
              effect->mName, effect->mUuid.timeLow);
        desc->mEffects.add(effect);
        node = node->next;
    }
    if (desc->mEffects.size() == 0) {
        ALOGW("loadEffectConfig() no valid effects found in config %s", root->name);
        delete desc;
        return NULL;
    }
    return desc;
}

status_t AudioPolicyEffects::loadInputEffectConfigurations(cnode *root,
                                                           const Vector <EffectDesc *>& effects)
{
    cnode *node = config_find(root, PREPROCESSING_TAG);
    if (node == NULL) {
        return -ENOENT;
    }
    node = node->first_child;
    while (node) {
        audio_source_t source = inputSourceNameToEnum(node->name);
        if (source == AUDIO_SOURCE_CNT) {
            ALOGW("loadInputSources() invalid input source %s", node->name);
            node = node->next;
            continue;
        }
        ALOGV("loadInputSources() loading input source %s", node->name);
        EffectDescVector *desc = loadEffectConfig(node, effects);
        if (desc == NULL) {
            node = node->next;
            continue;
        }
        mInputSources.add(source, desc);
        node = node->next;
    }
    return NO_ERROR;
}

status_t AudioPolicyEffects::loadStreamEffectConfigurations(cnode *root,
                                                            const Vector <EffectDesc *>& effects)
{
    cnode *node = config_find(root, OUTPUT_SESSION_PROCESSING_TAG);
    if (node == NULL) {
        return -ENOENT;
    }
    node = node->first_child;
    while (node) {
        audio_stream_type_t stream = streamNameToEnum(node->name);
        if (stream == AUDIO_STREAM_PUBLIC_CNT) {
            ALOGW("loadStreamEffectConfigurations() invalid output stream %s", node->name);
            node = node->next;
            continue;
        }
        ALOGV("loadStreamEffectConfigurations() loading output stream %s", node->name);
        EffectDescVector *desc = loadEffectConfig(node, effects);
        if (desc == NULL) {
            node = node->next;
            continue;
        }
        mOutputStreams.add(stream, desc);
        node = node->next;
    }
    return NO_ERROR;
}

AudioPolicyEffects::EffectDesc *AudioPolicyEffects::loadEffect(cnode *root)
{
    cnode *node = config_find(root, UUID_TAG);
    if (node == NULL) {
        return NULL;
    }
    effect_uuid_t uuid;
    if (AudioEffect::stringToGuid(node->value, &uuid) != NO_ERROR) {
        ALOGW("loadEffect() invalid uuid %s", node->value);
        return NULL;
    }
    return new EffectDesc(root->name, uuid);
}

status_t AudioPolicyEffects::loadEffects(cnode *root, Vector <EffectDesc *>& effects)
{
    cnode *node = config_find(root, EFFECTS_TAG);
    if (node == NULL) {
        return -ENOENT;
    }
    node = node->first_child;
    while (node) {
        ALOGV("loadEffects() loading effect %s", node->name);
        EffectDesc *effect = loadEffect(node);
        if (effect == NULL) {
            node = node->next;
            continue;
        }
        effects.add(effect);
        node = node->next;
    }
    return NO_ERROR;
}

status_t AudioPolicyEffects::loadAudioEffectXmlConfig() {
    auto result = effectsConfig::parse();
    if (result.parsedConfig == nullptr) {
        return -ENOENT;
    }

    auto loadProcessingChain = [](auto& processingChain, auto& streams) {
        for (auto& stream : processingChain) {
            auto effectDescs = std::make_unique<EffectDescVector>();
            for (auto& effect : stream.effects) {
                effectDescs->mEffects.add(
                        new EffectDesc{effect.get().name.c_str(), effect.get().uuid});
            }
            streams.add(stream.type, effectDescs.release());
        }
    };
    loadProcessingChain(result.parsedConfig->preprocess, mInputSources);
    loadProcessingChain(result.parsedConfig->postprocess, mOutputStreams);
    // Casting from ssize_t to status_t is probably safe, there should not be more than 2^31 errors
    return result.nbSkippedElement;
}

status_t AudioPolicyEffects::loadAudioEffectConfig(const char *path)
{
    cnode *root;
    char *data;

    data = (char *)load_file(path, NULL);
    if (data == NULL) {
        return -ENODEV;
    }
    root = config_node("", "");
    config_load(root, data);

    Vector <EffectDesc *> effects;
    loadEffects(root, effects);
    loadInputEffectConfigurations(root, effects);
    loadStreamEffectConfigurations(root, effects);

    for (size_t i = 0; i < effects.size(); i++) {
        delete effects[i];
    }

    config_free(root);
    free(root);
    free(data);

    return NO_ERROR;
}


} // namespace android
