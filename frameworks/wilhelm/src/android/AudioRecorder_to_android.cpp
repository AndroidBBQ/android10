/*
 * Copyright (C) 2010 The Android Open Source Project
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

#include "sles_allinclusive.h"
#include "android_prompts.h"
#include "channels.h"

#include <utils/String16.h>

#include <system/audio.h>
#include <SLES/OpenSLES_Android.h>

#include <android_runtime/AndroidRuntime.h>

#define KEY_RECORDING_SOURCE_PARAMSIZE  sizeof(SLuint32)
#define KEY_RECORDING_PRESET_PARAMSIZE  sizeof(SLuint32)
#define KEY_PERFORMANCE_MODE_PARAMSIZE  sizeof(SLuint32)

//-----------------------------------------------------------------------------
// Internal utility functions
//----------------------------

SLresult audioRecorder_setPreset(CAudioRecorder* ar, SLuint32 recordPreset) {
    SLresult result = SL_RESULT_SUCCESS;

    audio_source_t newRecordSource = AUDIO_SOURCE_DEFAULT;
    switch (recordPreset) {
    case SL_ANDROID_RECORDING_PRESET_GENERIC:
        newRecordSource = AUDIO_SOURCE_DEFAULT;
        break;
    case SL_ANDROID_RECORDING_PRESET_CAMCORDER:
        newRecordSource = AUDIO_SOURCE_CAMCORDER;
        break;
    case SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION:
        newRecordSource = AUDIO_SOURCE_VOICE_RECOGNITION;
        break;
    case SL_ANDROID_RECORDING_PRESET_VOICE_COMMUNICATION:
        newRecordSource = AUDIO_SOURCE_VOICE_COMMUNICATION;
        break;
    case SL_ANDROID_RECORDING_PRESET_UNPROCESSED:
            newRecordSource = AUDIO_SOURCE_UNPROCESSED;
            break;
    case SL_ANDROID_RECORDING_PRESET_NONE:
        // it is an error to set preset "none"
    default:
        SL_LOGE(ERROR_RECORDERPRESET_SET_UNKNOWN_PRESET);
        result = SL_RESULT_PARAMETER_INVALID;
    }

    // recording preset needs to be set before the object is realized
    // (ap->mAudioRecord is supposed to be 0 until then)
    if (SL_OBJECT_STATE_UNREALIZED != ar->mObject.mState) {
        SL_LOGE(ERROR_RECORDERPRESET_REALIZED);
        result = SL_RESULT_PRECONDITIONS_VIOLATED;
    } else {
        ar->mRecordSource = newRecordSource;
    }

    return result;
}


//-----------------------------------------------------------------------------
SLresult audioRecorder_setPerformanceMode(CAudioRecorder* ar, SLuint32 mode) {
    SLresult result = SL_RESULT_SUCCESS;
    SL_LOGV("performance mode set to %d", mode);

    SLuint32 perfMode = ANDROID_PERFORMANCE_MODE_DEFAULT;
    switch (mode) {
    case SL_ANDROID_PERFORMANCE_LATENCY:
        perfMode = ANDROID_PERFORMANCE_MODE_LATENCY;
        break;
    case SL_ANDROID_PERFORMANCE_LATENCY_EFFECTS:
        perfMode = ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS;
        break;
    case SL_ANDROID_PERFORMANCE_NONE:
        perfMode = ANDROID_PERFORMANCE_MODE_NONE;
        break;
    case SL_ANDROID_PERFORMANCE_POWER_SAVING:
        perfMode = ANDROID_PERFORMANCE_MODE_POWER_SAVING;
        break;
    default:
        SL_LOGE(ERROR_CONFIG_PERF_MODE_UNKNOWN);
        result = SL_RESULT_PARAMETER_INVALID;
        break;
    }

    // performance mode needs to be set before the object is realized
    // (ar->mAudioRecord is supposed to be NULL until then)
    if (SL_OBJECT_STATE_UNREALIZED != ar->mObject.mState) {
        SL_LOGE(ERROR_CONFIG_PERF_MODE_REALIZED);
        result = SL_RESULT_PRECONDITIONS_VIOLATED;
    } else {
        ar->mPerformanceMode = perfMode;
    }

    return result;
}


SLresult audioRecorder_getPreset(CAudioRecorder* ar, SLuint32* pPreset) {
    SLresult result = SL_RESULT_SUCCESS;

    switch (ar->mRecordSource) {
    case AUDIO_SOURCE_DEFAULT:
    case AUDIO_SOURCE_MIC:
        *pPreset = SL_ANDROID_RECORDING_PRESET_GENERIC;
        break;
    case AUDIO_SOURCE_VOICE_UPLINK:
    case AUDIO_SOURCE_VOICE_DOWNLINK:
    case AUDIO_SOURCE_VOICE_CALL:
        *pPreset = SL_ANDROID_RECORDING_PRESET_NONE;
        break;
    case AUDIO_SOURCE_VOICE_RECOGNITION:
        *pPreset = SL_ANDROID_RECORDING_PRESET_VOICE_RECOGNITION;
        break;
    case AUDIO_SOURCE_CAMCORDER:
        *pPreset = SL_ANDROID_RECORDING_PRESET_CAMCORDER;
        break;
    case AUDIO_SOURCE_VOICE_COMMUNICATION:
        *pPreset = SL_ANDROID_RECORDING_PRESET_VOICE_COMMUNICATION;
        break;
    case AUDIO_SOURCE_UNPROCESSED:
        *pPreset = SL_ANDROID_RECORDING_PRESET_UNPROCESSED;
        break;
    default:
        *pPreset = SL_ANDROID_RECORDING_PRESET_NONE;
        result = SL_RESULT_INTERNAL_ERROR;
        break;
    }

    return result;
}


//-----------------------------------------------------------------------------
SLresult audioRecorder_getPerformanceMode(CAudioRecorder* ar, SLuint32 *pMode) {
    SLresult result = SL_RESULT_SUCCESS;

    switch (ar->mPerformanceMode) {
    case ANDROID_PERFORMANCE_MODE_LATENCY:
        *pMode = SL_ANDROID_PERFORMANCE_LATENCY;
        break;
    case ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS:
        *pMode = SL_ANDROID_PERFORMANCE_LATENCY_EFFECTS;
        break;
    case ANDROID_PERFORMANCE_MODE_NONE:
        *pMode = SL_ANDROID_PERFORMANCE_NONE;
        break;
    case ANDROID_PERFORMANCE_MODE_POWER_SAVING:
        *pMode = SL_ANDROID_PERFORMANCE_POWER_SAVING;
        break;
    default:
        result = SL_RESULT_INTERNAL_ERROR;
        *pMode = SL_ANDROID_PERFORMANCE_LATENCY;
        break;
    }

    return result;
}


void audioRecorder_handleNewPos_lockRecord(CAudioRecorder* ar) {
    //SL_LOGV("received event EVENT_NEW_POS from AudioRecord");
    slRecordCallback callback = NULL;
    void* callbackPContext = NULL;

    interface_lock_shared(&ar->mRecord);
    callback = ar->mRecord.mCallback;
    callbackPContext = ar->mRecord.mContext;
    interface_unlock_shared(&ar->mRecord);

    if (NULL != callback) {
        // getting this event implies SL_RECORDEVENT_HEADATNEWPOS was set in the event mask
        (*callback)(&ar->mRecord.mItf, callbackPContext, SL_RECORDEVENT_HEADATNEWPOS);
    }
}


void audioRecorder_handleMarker_lockRecord(CAudioRecorder* ar) {
    //SL_LOGV("received event EVENT_MARKER from AudioRecord");
    slRecordCallback callback = NULL;
    void* callbackPContext = NULL;

    interface_lock_shared(&ar->mRecord);
    callback = ar->mRecord.mCallback;
    callbackPContext = ar->mRecord.mContext;
    interface_unlock_shared(&ar->mRecord);

    if (NULL != callback) {
        // getting this event implies SL_RECORDEVENT_HEADATMARKER was set in the event mask
        (*callback)(&ar->mRecord.mItf, callbackPContext, SL_RECORDEVENT_HEADATMARKER);
    }
}


void audioRecorder_handleOverrun_lockRecord(CAudioRecorder* ar) {
    //SL_LOGV("received event EVENT_OVERRUN from AudioRecord");
    slRecordCallback callback = NULL;
    void* callbackPContext = NULL;

    interface_lock_shared(&ar->mRecord);
    if (ar->mRecord.mCallbackEventsMask & SL_RECORDEVENT_HEADSTALLED) {
        callback = ar->mRecord.mCallback;
        callbackPContext = ar->mRecord.mContext;
    }
    interface_unlock_shared(&ar->mRecord);

    if (NULL != callback) {
        (*callback)(&ar->mRecord.mItf, callbackPContext, SL_RECORDEVENT_HEADSTALLED);
    }
}

//-----------------------------------------------------------------------------
SLresult android_audioRecorder_checkSourceSink(CAudioRecorder* ar) {

    const SLDataSource *pAudioSrc = &ar->mDataSource.u.mSource;
    const SLDataSink   *pAudioSnk = &ar->mDataSink.u.mSink;

    const SLuint32 sinkLocatorType = *(SLuint32 *)pAudioSnk->pLocator;
    const SLuint32 sinkFormatType = *(SLuint32 *)pAudioSnk->pFormat;

    const SLuint32 *df_representation = NULL; // pointer to representation field, if it exists

    // sink must be an Android simple buffer queue with PCM data format
    switch (sinkLocatorType) {
    case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE: {
        switch (sinkFormatType) {
        case SL_ANDROID_DATAFORMAT_PCM_EX: {
            const SLAndroidDataFormat_PCM_EX *df_pcm =
                    (SLAndroidDataFormat_PCM_EX *) pAudioSnk->pFormat;
            // checkDataFormat() already checked representation
            df_representation = &df_pcm->representation;
        } // SL_ANDROID_DATAFORMAT_PCM_EX - fall through to next test.
            FALLTHROUGH_INTENDED;
        case SL_DATAFORMAT_PCM: {
            const SLDataFormat_PCM *df_pcm = (const SLDataFormat_PCM *) pAudioSnk->pFormat;
            // checkDataFormat already checked sample rate, channels, and mask
            ar->mNumChannels = df_pcm->numChannels;

            if (df_pcm->endianness != ar->mObject.mEngine->mEngine.mNativeEndianness) {
                SL_LOGE("Cannot create audio recorder: unsupported byte order %u",
                        df_pcm->endianness);
                return SL_RESULT_CONTENT_UNSUPPORTED;
            }

            ar->mSampleRateMilliHz = df_pcm->samplesPerSec; // Note: bad field name in SL ES
            SL_LOGV("AudioRecorder requested sample rate = %u mHz, %u channel(s)",
                    ar->mSampleRateMilliHz, ar->mNumChannels);

            // we don't support container size != sample depth
            if (df_pcm->containerSize != df_pcm->bitsPerSample) {
                SL_LOGE("Cannot create audio recorder: unsupported container size %u bits for "
                        "sample depth %u bits",
                        df_pcm->containerSize, (SLuint32)df_pcm->bitsPerSample);
                return SL_RESULT_CONTENT_UNSUPPORTED;
            }

            } break;
        default:
            SL_LOGE(ERROR_RECORDER_SINK_FORMAT_MUST_BE_PCM);
            return SL_RESULT_PARAMETER_INVALID;
        }   // switch (sourceFormatType)
        } break;    // case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE
    default:
        SL_LOGE(ERROR_RECORDER_SINK_MUST_BE_ANDROIDSIMPLEBUFFERQUEUE);
        return SL_RESULT_PARAMETER_INVALID;
    }   // switch (sourceLocatorType)

    // Source check:
    // only input device sources are supported
    // check it's an IO device
    if (SL_DATALOCATOR_IODEVICE != *(SLuint32 *)pAudioSrc->pLocator) {
        SL_LOGE(ERROR_RECORDER_SOURCE_MUST_BE_IODEVICE);
        return SL_RESULT_PARAMETER_INVALID;
    } else {

        // check it's an input device
        SLDataLocator_IODevice *dl_iod = (SLDataLocator_IODevice *) pAudioSrc->pLocator;
        if (SL_IODEVICE_AUDIOINPUT != dl_iod->deviceType) {
            SL_LOGE(ERROR_RECORDER_IODEVICE_MUST_BE_AUDIOINPUT);
            return SL_RESULT_PARAMETER_INVALID;
        }

        // check it's the default input device, others aren't supported here
        if (SL_DEFAULTDEVICEID_AUDIOINPUT != dl_iod->deviceID) {
            SL_LOGE(ERROR_RECORDER_INPUT_ID_MUST_BE_DEFAULT);
            return SL_RESULT_PARAMETER_INVALID;
        }
    }

    return SL_RESULT_SUCCESS;
}
//-----------------------------------------------------------------------------
static void audioRecorder_callback(int event, void* user, void *info) {
    //SL_LOGV("audioRecorder_callback(%d, %p, %p) entering", event, user, info);

    CAudioRecorder *ar = (CAudioRecorder *)user;

    if (!android::CallbackProtector::enterCbIfOk(ar->mCallbackProtector)) {
        // it is not safe to enter the callback (the track is about to go away)
        return;
    }

    void * callbackPContext = NULL;

    switch (event) {
    case android::AudioRecord::EVENT_MORE_DATA: {
        slBufferQueueCallback callback = NULL;
        android::AudioRecord::Buffer* pBuff = (android::AudioRecord::Buffer*)info;

        // push data to the buffer queue
        interface_lock_exclusive(&ar->mBufferQueue);

        if (ar->mBufferQueue.mState.count != 0) {
            assert(ar->mBufferQueue.mFront != ar->mBufferQueue.mRear);

            BufferHeader *oldFront = ar->mBufferQueue.mFront;
            BufferHeader *newFront = &oldFront[1];

            size_t availSink = oldFront->mSize - ar->mBufferQueue.mSizeConsumed;
            size_t availSource = pBuff->size;
            size_t bytesToCopy = availSink < availSource ? availSink : availSource;
            void *pDest = (char *)oldFront->mBuffer + ar->mBufferQueue.mSizeConsumed;
            memcpy(pDest, pBuff->raw, bytesToCopy);

            if (bytesToCopy < availSink) {
                // can't consume the whole or rest of the buffer in one shot
                ar->mBufferQueue.mSizeConsumed += availSource;
                // pBuff->size is already equal to bytesToCopy in this case
            } else {
                // finish pushing the buffer or push the buffer in one shot
                pBuff->size = bytesToCopy;
                ar->mBufferQueue.mSizeConsumed = 0;
                if (newFront == &ar->mBufferQueue.mArray[ar->mBufferQueue.mNumBuffers + 1]) {
                    newFront = ar->mBufferQueue.mArray;
                }
                ar->mBufferQueue.mFront = newFront;

                ar->mBufferQueue.mState.count--;
                ar->mBufferQueue.mState.playIndex++;

                // data has been copied to the buffer, and the buffer queue state has been updated
                // we will notify the client if applicable
                callback = ar->mBufferQueue.mCallback;
                // save callback data
                callbackPContext = ar->mBufferQueue.mContext;
            }
        } else { // empty queue
            // no destination to push the data
            pBuff->size = 0;
        }

        interface_unlock_exclusive(&ar->mBufferQueue);

        // notify client
        if (NULL != callback) {
            (*callback)(&ar->mBufferQueue.mItf, callbackPContext);
        }
        }
        break;

    case android::AudioRecord::EVENT_OVERRUN:
        audioRecorder_handleOverrun_lockRecord(ar);
        break;

    case android::AudioRecord::EVENT_MARKER:
        audioRecorder_handleMarker_lockRecord(ar);
        break;

    case android::AudioRecord::EVENT_NEW_POS:
        audioRecorder_handleNewPos_lockRecord(ar);
        break;

    case android::AudioRecord::EVENT_NEW_IAUDIORECORD:
        // ignore for now
        break;

    default:
        SL_LOGE("Encountered unknown AudioRecord event %d for CAudioRecord %p", event, ar);
        break;
    }

    ar->mCallbackProtector->exitCb();
}


//-----------------------------------------------------------------------------
SLresult android_audioRecorder_create(CAudioRecorder* ar) {
    SL_LOGV("android_audioRecorder_create(%p) entering", ar);

    const SLDataSource *pAudioSrc = &ar->mDataSource.u.mSource;
    const SLDataSink *pAudioSnk = &ar->mDataSink.u.mSink;
    SLresult result = SL_RESULT_SUCCESS;

    const SLuint32 sourceLocatorType = *(SLuint32 *)pAudioSrc->pLocator;
    const SLuint32 sinkLocatorType = *(SLuint32 *)pAudioSnk->pLocator;

    //  the following platform-independent fields have been initialized in CreateAudioRecorder()
    //    ar->mNumChannels
    //    ar->mSampleRateMilliHz

    if ((SL_DATALOCATOR_IODEVICE == sourceLocatorType) &&
            (SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE == sinkLocatorType)) {
        // microphone to simple buffer queue
        ar->mAndroidObjType = AUDIORECORDER_FROM_MIC_TO_PCM_BUFFERQUEUE;
        ar->mAudioRecord.clear();
        ar->mCallbackProtector = new android::CallbackProtector();
        ar->mRecordSource = AUDIO_SOURCE_DEFAULT;
        ar->mPerformanceMode = ANDROID_PERFORMANCE_MODE_DEFAULT;
    } else {
        result = SL_RESULT_CONTENT_UNSUPPORTED;
    }

    return result;
}


//-----------------------------------------------------------------------------
SLresult android_audioRecorder_setConfig(CAudioRecorder* ar, const SLchar *configKey,
        const void *pConfigValue, SLuint32 valueSize) {

    SLresult result;

    assert(NULL != ar && NULL != configKey && NULL != pConfigValue);
    if (strcmp((const char*)configKey, (const char*)SL_ANDROID_KEY_RECORDING_PRESET) == 0) {

        // recording preset
        if (KEY_RECORDING_PRESET_PARAMSIZE > valueSize) {
            SL_LOGE(ERROR_CONFIG_VALUESIZE_TOO_LOW);
            result = SL_RESULT_BUFFER_INSUFFICIENT;
        } else {
            result = audioRecorder_setPreset(ar, *(SLuint32*)pConfigValue);
        }

    } else if (strcmp((const char*)configKey, (const char*)SL_ANDROID_KEY_PERFORMANCE_MODE) == 0) {

        // performance mode
        if (KEY_PERFORMANCE_MODE_PARAMSIZE > valueSize) {
            SL_LOGE(ERROR_CONFIG_VALUESIZE_TOO_LOW);
            result = SL_RESULT_BUFFER_INSUFFICIENT;
        } else {
            result = audioRecorder_setPerformanceMode(ar, *(SLuint32*)pConfigValue);
        }
    } else {
        SL_LOGE(ERROR_CONFIG_UNKNOWN_KEY);
        result = SL_RESULT_PARAMETER_INVALID;
    }

    return result;
}


//-----------------------------------------------------------------------------
SLresult android_audioRecorder_getConfig(CAudioRecorder* ar, const SLchar *configKey,
        SLuint32* pValueSize, void *pConfigValue) {

    SLresult result;

    assert(NULL != ar && NULL != configKey && NULL != pValueSize);
    if (strcmp((const char*)configKey, (const char*)SL_ANDROID_KEY_RECORDING_PRESET) == 0) {

        // recording preset
        if (NULL == pConfigValue) {
            result = SL_RESULT_SUCCESS;
        } else if (KEY_RECORDING_PRESET_PARAMSIZE > *pValueSize) {
            SL_LOGE(ERROR_CONFIG_VALUESIZE_TOO_LOW);
            result = SL_RESULT_BUFFER_INSUFFICIENT;
        } else {
            result = audioRecorder_getPreset(ar, (SLuint32*)pConfigValue);
        }
        *pValueSize = KEY_RECORDING_PRESET_PARAMSIZE;

    } else if (strcmp((const char*)configKey, (const char*)SL_ANDROID_KEY_PERFORMANCE_MODE) == 0) {

        // performance mode
        if (NULL == pConfigValue) {
            result = SL_RESULT_SUCCESS;
        } else if (KEY_PERFORMANCE_MODE_PARAMSIZE > *pValueSize) {
            SL_LOGE(ERROR_CONFIG_VALUESIZE_TOO_LOW);
            result = SL_RESULT_BUFFER_INSUFFICIENT;
        } else {
            result = audioRecorder_getPerformanceMode(ar, (SLuint32*)pConfigValue);
        }
        *pValueSize = KEY_PERFORMANCE_MODE_PARAMSIZE;

    } else {
        SL_LOGE(ERROR_CONFIG_UNKNOWN_KEY);
        result = SL_RESULT_PARAMETER_INVALID;
    }

    return result;
}

// Called from android_audioRecorder_realize for a PCM buffer queue recorder before creating the
// AudioRecord to determine which performance modes are allowed based on effect interfaces present
static void checkAndSetPerformanceModePre(CAudioRecorder* ar)
{
    SLuint32 allowedModes = ANDROID_PERFORMANCE_MODE_ALL;
    assert(ar->mAndroidObjType == AUDIORECORDER_FROM_MIC_TO_PCM_BUFFERQUEUE);

    // no need to check the buffer queue size, application side
    // double-buffering (and more) is not a requirement for using fast tracks

    // Check a blacklist of interfaces that are incompatible with fast tracks.
    // The alternative, to check a whitelist of compatible interfaces, is
    // more maintainable but is too slow.  As a compromise, in a debug build
    // we use both methods and warn if they produce different results.
    // In release builds, we only use the blacklist method.
    // If a blacklisted interface is added after realization using
    // DynamicInterfaceManagement::AddInterface,
    // then this won't be detected but the interface will be ineffective.
    static const unsigned blacklist[] = {
        MPH_ANDROIDACOUSTICECHOCANCELLATION,
        MPH_ANDROIDAUTOMATICGAINCONTROL,
        MPH_ANDROIDNOISESUPPRESSION,
        MPH_ANDROIDEFFECT,
        // FIXME The problem with a blacklist is remembering to add new interfaces here
    };

    for (unsigned i = 0; i < sizeof(blacklist)/sizeof(blacklist[0]); ++i) {
        if (IsInterfaceInitialized(&ar->mObject, blacklist[i])) {
            uint32_t flags = 0;

            allowedModes &= ~ANDROID_PERFORMANCE_MODE_LATENCY;

            // if generic effect interface is used we don't know which effect will be used and
            // disable all low latency performance modes
            if (blacklist[i] != MPH_ANDROIDEFFECT) {
                switch (blacklist[i]) {
                case MPH_ANDROIDACOUSTICECHOCANCELLATION:
                    SL_LOGV("checkAndSetPerformanceModePre found AEC name %s",
                            ar->mAcousticEchoCancellation.mAECDescriptor.name);
                    flags = ar->mAcousticEchoCancellation.mAECDescriptor.flags;
                    break;
                case MPH_ANDROIDAUTOMATICGAINCONTROL:
                    SL_LOGV("checkAndSetPerformanceModePre found AGC name %s",
                            ar->mAutomaticGainControl.mAGCDescriptor.name);
                    flags = ar->mAutomaticGainControl.mAGCDescriptor.flags;
                    break;
                case MPH_ANDROIDNOISESUPPRESSION:
                    SL_LOGV("checkAndSetPerformanceModePre found NS name %s",
                            ar->mNoiseSuppression.mNSDescriptor.name);
                    flags = ar->mNoiseSuppression.mNSDescriptor.flags;
                    break;
                default:
                    break;
                }
            }
            if ((flags & EFFECT_FLAG_HW_ACC_TUNNEL) == 0) {
                allowedModes &= ~ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS;
                break;
            }
        }
    }
#if LOG_NDEBUG == 0
    bool blacklistResult = (
            (allowedModes &
                (ANDROID_PERFORMANCE_MODE_LATENCY|ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS)) != 0);
    bool whitelistResult = true;
    static const unsigned whitelist[] = {
        MPH_BUFFERQUEUE,
        MPH_DYNAMICINTERFACEMANAGEMENT,
        MPH_OBJECT,
        MPH_RECORD,
        MPH_ANDROIDCONFIGURATION,
        MPH_ANDROIDSIMPLEBUFFERQUEUE,
    };
    for (unsigned mph = MPH_MIN; mph < MPH_MAX; ++mph) {
        for (unsigned i = 0; i < sizeof(whitelist)/sizeof(whitelist[0]); ++i) {
            if (mph == whitelist[i]) {
                goto compatible;
            }
        }
        if (IsInterfaceInitialized(&ar->mObject, mph)) {
            whitelistResult = false;
            break;
        }
compatible: ;
    }
    if (whitelistResult != blacklistResult) {
        SL_LOGW("whitelistResult != blacklistResult");
    }
#endif
    if (ar->mPerformanceMode == ANDROID_PERFORMANCE_MODE_LATENCY) {
        if ((allowedModes & ANDROID_PERFORMANCE_MODE_LATENCY) == 0) {
            ar->mPerformanceMode = ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS;
        }
    }
    if (ar->mPerformanceMode == ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS) {
        if ((allowedModes & ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS) == 0) {
            ar->mPerformanceMode = ANDROID_PERFORMANCE_MODE_NONE;
        }
    }
}

// Called from android_audioRecorder_realize for a PCM buffer queue recorder after creating the
// AudioRecord to adjust performance mode based on actual input flags
static void checkAndSetPerformanceModePost(CAudioRecorder* ar)
{
    audio_input_flags_t flags = ar->mAudioRecord->getFlags();
    switch (ar->mPerformanceMode) {
    case ANDROID_PERFORMANCE_MODE_LATENCY:
        if ((flags & (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_RAW)) ==
                (AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_RAW)) {
            break;
        }
        ar->mPerformanceMode = ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS;
        FALLTHROUGH_INTENDED;
    case ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS:
        if ((flags & AUDIO_INPUT_FLAG_FAST) == 0) {
            ar->mPerformanceMode = ANDROID_PERFORMANCE_MODE_NONE;
        }
        break;
    case ANDROID_PERFORMANCE_MODE_NONE:
    default:
        break;
    }
}
//-----------------------------------------------------------------------------
SLresult android_audioRecorder_realize(CAudioRecorder* ar, SLboolean async) {
    SL_LOGV("android_audioRecorder_realize(%p) entering", ar);

    SLresult result = SL_RESULT_SUCCESS;

    // already checked in created and checkSourceSink
    assert(ar->mDataSink.mLocator.mLocatorType == SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE);

    const SLDataFormat_PCM *df_pcm = &ar->mDataSink.mFormat.mPCM;

    //  the following platform-independent fields have been initialized in CreateAudioRecorder()
    //    ar->mNumChannels
    //    ar->mSampleRateMilliHz

    uint32_t sampleRate = sles_to_android_sampleRate(df_pcm->samplesPerSec);

    checkAndSetPerformanceModePre(ar);

    audio_input_flags_t policy;
    switch (ar->mPerformanceMode) {
    case ANDROID_PERFORMANCE_MODE_NONE:
    case ANDROID_PERFORMANCE_MODE_POWER_SAVING:
        policy = AUDIO_INPUT_FLAG_NONE;
        break;
    case ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS:
        policy = AUDIO_INPUT_FLAG_FAST;
        break;
    case ANDROID_PERFORMANCE_MODE_LATENCY:
    default:
        policy = (audio_input_flags_t)(AUDIO_INPUT_FLAG_FAST | AUDIO_INPUT_FLAG_RAW);
        break;
    }

    SL_LOGV("Audio Record format: %dch(0x%x), %dbit, %dKHz",
            df_pcm->numChannels,
            df_pcm->channelMask,
            df_pcm->bitsPerSample,
            df_pcm->samplesPerSec / 1000000);

    // note that df_pcm->channelMask has already been validated during object creation.
    audio_channel_mask_t channelMask = sles_to_audio_input_channel_mask(df_pcm->channelMask);

    // To maintain backward compatibility with previous releases, ignore
    // channel masks that are not indexed.
    if (channelMask == AUDIO_CHANNEL_INVALID
            || audio_channel_mask_get_representation(channelMask)
                == AUDIO_CHANNEL_REPRESENTATION_POSITION) {
        channelMask = audio_channel_in_mask_from_count(df_pcm->numChannels);
        SL_LOGI("Emulating old channel mask behavior "
                "(ignoring positional mask %#x, using default mask %#x based on "
                "channel count of %d)", df_pcm->channelMask, channelMask,
                df_pcm->numChannels);
    }
    SL_LOGV("SLES channel mask %#x converted to Android mask %#x", df_pcm->channelMask,
            channelMask);

    // initialize platform-specific CAudioRecorder fields
    ar->mAudioRecord = new android::AudioRecord(
            ar->mRecordSource,     // source
            sampleRate,            // sample rate in Hertz
            sles_to_android_sampleFormat(df_pcm),               // format
            channelMask,           // channel mask
            android::String16(),   // app ops
            0,                     // frameCount
            audioRecorder_callback,// callback_t
            (void*)ar,             // user, callback data, here the AudioRecorder
            0,                     // notificationFrames
            AUDIO_SESSION_ALLOCATE,
            android::AudioRecord::TRANSFER_CALLBACK,
                                   // transfer type
            policy);               // audio_input_flags_t

    android::status_t status = ar->mAudioRecord->initCheck();
    if (android::NO_ERROR != status) {
        SL_LOGE("android_audioRecorder_realize(%p) error creating AudioRecord object; status %d",
                ar, status);
        // FIXME should return a more specific result depending on status
        result = SL_RESULT_CONTENT_UNSUPPORTED;
        ar->mAudioRecord.clear();
        return result;
    }

    // update performance mode according to actual flags granted to AudioRecord
    checkAndSetPerformanceModePost(ar);

    // If there is a JavaAudioRoutingProxy associated with this recorder, hook it up...
    JNIEnv* j_env = NULL;
    jclass clsAudioRecord = NULL;
    jmethodID midRoutingProxy_connect = NULL;
    if (ar->mAndroidConfiguration.mRoutingProxy != NULL &&
            (j_env = android::AndroidRuntime::getJNIEnv()) != NULL &&
            (clsAudioRecord = j_env->FindClass("android/media/AudioRecord")) != NULL &&
            (midRoutingProxy_connect =
                j_env->GetMethodID(clsAudioRecord, "deferred_connect", "(J)V")) != NULL) {
        j_env->ExceptionClear();
        j_env->CallVoidMethod(ar->mAndroidConfiguration.mRoutingProxy,
                              midRoutingProxy_connect,
                              ar->mAudioRecord.get());
        if (j_env->ExceptionCheck()) {
            SL_LOGE("Java exception releasing recorder routing object.");
            result = SL_RESULT_INTERNAL_ERROR;
            ar->mAudioRecord.clear();
            return result;
        }
   }

    if (ar->mPerformanceMode != ANDROID_PERFORMANCE_MODE_LATENCY) {
        audio_session_t sessionId = ar->mAudioRecord->getSessionId();
        // initialize AEC
        effect_descriptor_t *descriptor = &ar->mAcousticEchoCancellation.mAECDescriptor;
        if (memcmp(SL_IID_ANDROIDACOUSTICECHOCANCELLATION, &descriptor->type,
                   sizeof(effect_uuid_t)) == 0) {
            if ((ar->mPerformanceMode != ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS) ||
                    (descriptor->flags & EFFECT_FLAG_HW_ACC_TUNNEL)) {
                SL_LOGV("Need to initialize AEC for AudioRecorder=%p", ar);
                android_aec_init(sessionId, &ar->mAcousticEchoCancellation);
            }
        }

        // initialize AGC
        descriptor = &ar->mAutomaticGainControl.mAGCDescriptor;
        if (memcmp(SL_IID_ANDROIDAUTOMATICGAINCONTROL, &descriptor->type,
                   sizeof(effect_uuid_t)) == 0) {
            if ((ar->mPerformanceMode != ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS) ||
                    (descriptor->flags & EFFECT_FLAG_HW_ACC_TUNNEL)) {
                SL_LOGV("Need to initialize AGC for AudioRecorder=%p", ar);
                android_agc_init(sessionId, &ar->mAutomaticGainControl);
            }
        }

        // initialize NS
        descriptor = &ar->mNoiseSuppression.mNSDescriptor;
        if (memcmp(SL_IID_ANDROIDNOISESUPPRESSION, &descriptor->type,
                   sizeof(effect_uuid_t)) == 0) {
            if ((ar->mPerformanceMode != ANDROID_PERFORMANCE_MODE_LATENCY_EFFECTS) ||
                    (descriptor->flags & EFFECT_FLAG_HW_ACC_TUNNEL)) {
                SL_LOGV("Need to initialize NS for AudioRecorder=%p", ar);
                android_ns_init(sessionId, &ar->mNoiseSuppression);
            }
        }
    }

    return result;
}


//-----------------------------------------------------------------------------
/**
 * Called with a lock on AudioRecorder, and blocks until safe to destroy
 */
void android_audioRecorder_preDestroy(CAudioRecorder* ar) {
    object_unlock_exclusive(&ar->mObject);
    if (ar->mCallbackProtector != 0) {
        ar->mCallbackProtector->requestCbExitAndWait();
    }
    object_lock_exclusive(&ar->mObject);
}


//-----------------------------------------------------------------------------
void android_audioRecorder_destroy(CAudioRecorder* ar) {
    SL_LOGV("android_audioRecorder_destroy(%p) entering", ar);

    if (ar->mAudioRecord != 0) {
        ar->mAudioRecord->stop();
        ar->mAudioRecord.clear();
    }
    // explicit destructor
    ar->mAudioRecord.~sp();
    ar->mCallbackProtector.~sp();
}


//-----------------------------------------------------------------------------
void android_audioRecorder_setRecordState(CAudioRecorder* ar, SLuint32 state) {
    SL_LOGV("android_audioRecorder_setRecordState(%p, %u) entering", ar, state);

    if (ar->mAudioRecord == 0) {
        return;
    }

    switch (state) {
     case SL_RECORDSTATE_STOPPED:
         ar->mAudioRecord->stop();
         break;
     case SL_RECORDSTATE_PAUSED:
         // Note that pausing is treated like stop as this implementation only records to a buffer
         //  queue, so there is no notion of destination being "opened" or "closed" (See description
         //  of SL_RECORDSTATE in specification)
         ar->mAudioRecord->stop();
         break;
     case SL_RECORDSTATE_RECORDING:
         ar->mAudioRecord->start();
         break;
     default:
         break;
     }

}


//-----------------------------------------------------------------------------
void android_audioRecorder_useRecordEventMask(CAudioRecorder *ar) {
    IRecord *pRecordItf = &ar->mRecord;
    SLuint32 eventFlags = pRecordItf->mCallbackEventsMask;

    if (ar->mAudioRecord == 0) {
        return;
    }

    if ((eventFlags & SL_RECORDEVENT_HEADATMARKER) && (pRecordItf->mMarkerPosition != 0)) {
        ar->mAudioRecord->setMarkerPosition((uint32_t)((((int64_t)pRecordItf->mMarkerPosition
                * sles_to_android_sampleRate(ar->mSampleRateMilliHz)))/1000));
    } else {
        // clear marker
        ar->mAudioRecord->setMarkerPosition(0);
    }

    if (eventFlags & SL_RECORDEVENT_HEADATNEWPOS) {
        SL_LOGV("pos update period %d", pRecordItf->mPositionUpdatePeriod);
         ar->mAudioRecord->setPositionUpdatePeriod(
                (uint32_t)((((int64_t)pRecordItf->mPositionUpdatePeriod
                * sles_to_android_sampleRate(ar->mSampleRateMilliHz)))/1000));
    } else {
        // clear periodic update
        ar->mAudioRecord->setPositionUpdatePeriod(0);
    }

    if (eventFlags & SL_RECORDEVENT_HEADATLIMIT) {
        // FIXME support SL_RECORDEVENT_HEADATLIMIT
        SL_LOGD("[ FIXME: IRecord_SetCallbackEventsMask(SL_RECORDEVENT_HEADATLIMIT) on an "
                    "SL_OBJECTID_AUDIORECORDER to be implemented ]");
    }

    if (eventFlags & SL_RECORDEVENT_HEADMOVING) {
        // FIXME support SL_RECORDEVENT_HEADMOVING
        SL_LOGD("[ FIXME: IRecord_SetCallbackEventsMask(SL_RECORDEVENT_HEADMOVING) on an "
                "SL_OBJECTID_AUDIORECORDER to be implemented ]");
    }

    if (eventFlags & SL_RECORDEVENT_BUFFER_FULL) {
        // nothing to do for SL_RECORDEVENT_BUFFER_FULL since this will not be encountered on
        // recording to buffer queues
    }

    if (eventFlags & SL_RECORDEVENT_HEADSTALLED) {
        // nothing to do for SL_RECORDEVENT_HEADSTALLED, callback event will be checked against mask
        // when AudioRecord::EVENT_OVERRUN is encountered

    }

}


//-----------------------------------------------------------------------------
void android_audioRecorder_getPosition(CAudioRecorder *ar, SLmillisecond *pPosMsec) {
    if ((NULL == ar) || (ar->mAudioRecord == 0)) {
        *pPosMsec = 0;
    } else {
        uint32_t positionInFrames;
        ar->mAudioRecord->getPosition(&positionInFrames);
        if (ar->mSampleRateMilliHz == UNKNOWN_SAMPLERATE) {
            *pPosMsec = 0;
        } else {
            *pPosMsec = ((int64_t)positionInFrames * 1000) /
                    sles_to_android_sampleRate(ar->mSampleRateMilliHz);
        }
    }
}
