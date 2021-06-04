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

/* AndroidConfiguration implementation */

#include <android/log.h>

#include "sles_allinclusive.h"
#include <SLES/OpenSLES_Android.h>

#include <android_runtime/AndroidRuntime.h>

extern jmethodID gMidAudioTrackRoutingProxy_ctor;
extern jmethodID gMidAudioTrackRoutingProxy_release;
extern jmethodID gMidAudioRecordRoutingProxy_ctor;
extern jmethodID gMidAudioRecordRoutingProxy_release;

extern jclass gClsAudioTrackRoutingProxy;
extern jclass gClsAudioRecordRoutingProxy;

static SLresult IAndroidConfiguration_SetConfiguration(SLAndroidConfigurationItf self,
        const SLchar *configKey,
        const void *pConfigValue,
        SLuint32 valueSize)
{
    SL_ENTER_INTERFACE

    // object-specific code will check that valueSize is large enough for the key
    if (NULL == configKey || NULL == pConfigValue || valueSize == 0) {
        result = SL_RESULT_PARAMETER_INVALID;

    } else {
        IAndroidConfiguration *thiz = (IAndroidConfiguration *) self;
        interface_lock_exclusive(thiz);

        // route configuration to the appropriate object
        switch (IObjectToObjectID((thiz)->mThis)) {
        case SL_OBJECTID_AUDIORECORDER:
            SL_LOGV("SetConfiguration issued for AudioRecorder key=%s valueSize=%u",
                    configKey, valueSize);
            result = android_audioRecorder_setConfig((CAudioRecorder *) thiz->mThis, configKey,
                    pConfigValue, valueSize);
            break;
        case SL_OBJECTID_AUDIOPLAYER:
            SL_LOGV("SetConfiguration issued for AudioPlayer key=%s valueSize=%u",
                    configKey, valueSize);
            result = android_audioPlayer_setConfig((CAudioPlayer *) thiz->mThis, configKey,
                    pConfigValue, valueSize);
            break;
        default:
            result = SL_RESULT_FEATURE_UNSUPPORTED;
            break;
        }

        interface_unlock_exclusive(thiz);
    }

    SL_LEAVE_INTERFACE
}


static SLresult IAndroidConfiguration_GetConfiguration(SLAndroidConfigurationItf self,
        const SLchar *configKey,
        SLuint32 *pValueSize,
        void *pConfigValue)
{
    SL_ENTER_INTERFACE

    // non-NULL pValueSize is required, but a NULL pConfigValue is allowed, so
    // that we can report the actual value size without returning the value itself
    if (NULL == configKey || NULL == pValueSize) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IAndroidConfiguration *thiz = (IAndroidConfiguration *) self;
        interface_lock_exclusive(thiz);

        // route configuration request to the appropriate object
        switch (IObjectToObjectID((thiz)->mThis)) {
        case SL_OBJECTID_AUDIORECORDER:
            result = android_audioRecorder_getConfig((CAudioRecorder *) thiz->mThis, configKey,
                    pValueSize, pConfigValue);
            break;
        case SL_OBJECTID_AUDIOPLAYER:
            result = android_audioPlayer_getConfig((CAudioPlayer *) thiz->mThis, configKey,
                    pValueSize, pConfigValue);
            break;
        default:
            result = SL_RESULT_FEATURE_UNSUPPORTED;
            break;
        }

        interface_unlock_exclusive(thiz);
    }

    SL_LEAVE_INTERFACE
}

/*
 * Native Routing API
 */
static SLresult ValidatePlayerConfig(IAndroidConfiguration* iConfig) {
    SLresult result;

    if (iConfig->mRoutingProxy != NULL) {
        result = SL_RESULT_PRECONDITIONS_VIOLATED;
        SL_LOGE("Error creating player routing object - Routing Proxy Already Acquired.");
    }
    else {
        IObject* configObj = iConfig->mThis;                // get corresponding object
        CAudioPlayer* player = (CAudioPlayer*)configObj;    // get the native player

        switch (player->mAndroidObjType) {
            case AUDIOPLAYER_FROM_PCM_BUFFERQUEUE:
                //TODO remove these commented out lines when our comfort level is good
//                if (player->mObject.mState != SL_OBJECT_STATE_REALIZED) {
//                    // Make sure the player has been realized.
//                    result = SL_RESULT_PRECONDITIONS_VIOLATED;
//                    SL_LOGE("Error creating routing object - Player not realized.");
//                } else {
//                    android::AudioTrack* pAudioTrack = player->mAudioTrack.get();
//                    if (pAudioTrack == NULL) {
//                        result = SL_RESULT_INTERNAL_ERROR;
//                        SL_LOGE("Error creating routing object - Couldn't get AudioTrack.");
//                    } else {
                        result = SL_RESULT_SUCCESS;
//                    }
//                }
                break;

            default:
                result =  SL_RESULT_PARAMETER_INVALID;
                SL_LOGE("Error creating routing object - Player is not a buffer-queue player.");
                break;
        }
    }

    return result;
}

static SLresult AllocPlayerRoutingProxy(IAndroidConfiguration* iConfig, jobject* proxyObj) {
    SLresult result;

    IObject* configObj = iConfig->mThis;                // get corresponding object
    android::AudioTrack* pAudioTrack = ((CAudioPlayer*)configObj)->mTrackPlayer->mAudioTrack.get();

    JNIEnv* j_env = android::AndroidRuntime::getJNIEnv();

    j_env->ExceptionClear();

    jobject localObjRef =
        j_env->NewObject(gClsAudioTrackRoutingProxy,
                         gMidAudioTrackRoutingProxy_ctor,
                         (jlong)pAudioTrack /*audioTrackObjInLong*/);

    *proxyObj = j_env->NewGlobalRef(localObjRef);

    if (j_env->ExceptionCheck()) {
        SL_LOGE("Java exception creating player routing object.");
        result = SL_RESULT_INTERNAL_ERROR;
    } else {
        // stash it in the Interface object
        iConfig->mRoutingProxy = *proxyObj;
        result = SL_RESULT_SUCCESS;
    }

    return result;
}

static SLresult ValidateRecorderConfig(IAndroidConfiguration* iConfig) {
    SLresult result;

    if (iConfig->mRoutingProxy != NULL) {
        result = SL_RESULT_PRECONDITIONS_VIOLATED;
        SL_LOGE("Error creating record routing object - Routing Proxy Already Acquired.");
    } else {
        IObject* configObj = iConfig->mThis;                  // get corresponding object
        CAudioRecorder* recorder = (CAudioRecorder*)configObj;  // get the native recorder
        switch (recorder->mAndroidObjType) {
            case AUDIORECORDER_FROM_MIC_TO_PCM_BUFFERQUEUE:
                //TODO remove these commented out lines when our comfort level is good
//                if (recorder->mObject.mState != SL_OBJECT_STATE_REALIZED) {
//                    // Make sure the recorder has been realized.
//                    result = SL_RESULT_PRECONDITIONS_VIOLATED;
//                    SL_LOGE("Error creating routing object - Recorder not realized.");
//                } else {
//                    android::AudioRecord* pAudioRecord = recorder->mAudioRecord.get();
//                    if (pAudioRecord == NULL) {
//                        result = SL_RESULT_INTERNAL_ERROR;
//                        SL_LOGE("Error creating routing object - Couldn't get AudioRecord.");
//                    } else if (iConfig->mRoutingProxy != NULL) {
//                        result = SL_RESULT_PRECONDITIONS_VIOLATED;
//                        SL_LOGE("Error creating routing object - Routing Proxy Already Acquired."
//                                );
//                    } else {
                        result = SL_RESULT_SUCCESS;
//                    }
//                }
                break;

            default:
                result =  SL_RESULT_PARAMETER_INVALID;
                SL_LOGE("Error creating routing object - Recorder is not a buffer-queue recorder.");
                break;
        }
    }

    return result;
}

static SLresult AllocRecorderRoutingProxy(IAndroidConfiguration* iConfig, jobject* proxyObj) {
    SLresult result;

    IObject* configObj = iConfig->mThis;                  // get corresponding object
    android::AudioRecord* pAudioRecord = ((CAudioRecorder*)configObj)->mAudioRecord.get();

    JNIEnv* j_env = android::AndroidRuntime::getJNIEnv();

    j_env->ExceptionClear();
    jobject localObjRef =
        j_env->NewObject(gClsAudioRecordRoutingProxy,
                         gMidAudioRecordRoutingProxy_ctor,
                         (jlong)pAudioRecord /*audioRecordObjInLong*/);

    *proxyObj = j_env->NewGlobalRef(localObjRef);
    if (j_env->ExceptionCheck()) {
        SL_LOGE("Java exception creating recorder routing object.");
        result = SL_RESULT_INTERNAL_ERROR;
    } else {
        // stash it in the Interface object
        iConfig->mRoutingProxy = *proxyObj;
        result = SL_RESULT_SUCCESS;
    }

    return result;
}

/*
 * Acquires a Java proxy object, such as AudioRouting object which can be used to control
 * aspects of the associated native player or recorder.
 * Parameters:
 *   self   An SLAndroidConfigurationItf obtained from either an OpenSL ES AudioPlayer
 *          or AudioRecorder.
 *   j_env  The Java Environment pointer (passed in to the calling JNI function).
 *   proxyType Specifies the type of proxy desired. Currently only SL_ANDROID_JAVA_PROXY_ROUTING
 *          is supported.
 *   proxyObj
 *          Points to the jobject to receive the acquired Java proxy object (as a GlobalRef).
 * Returns SL_RESULT_SUCCESS is the proxy object is acquired, SL_RESULT_PARAMETER_INVALID if
 *   there is a problem with the arguments causing the function to fail,
 *   <working on this>
 *   SL_RESULT_PRECONDITIONS_VIOLATED it the AudioPlayer or AudioRecorder object associated
 *   with the ConfigurationItf has not been realized.
 */
static SLresult IAndroidConfiguration_AcquireJavaProxy(SLAndroidConfigurationItf self,
                                                       SLuint32 proxyType,
                                                       jobject* proxyObj)
{
    SL_ENTER_INTERFACE

    if (self == NULL || proxyObj == NULL || proxyType != SL_ANDROID_JAVA_PROXY_ROUTING) {
        result =  SL_RESULT_PARAMETER_INVALID;
    } else {
        IAndroidConfiguration* iConfig = (IAndroidConfiguration*)self;

        int objID = IObjectToObjectID(InterfaceToIObject(iConfig));
        switch (objID) {
        case SL_OBJECTID_AUDIOPLAYER:
            result = ValidatePlayerConfig(iConfig);
            if (result == SL_RESULT_SUCCESS) {
                result = AllocPlayerRoutingProxy(iConfig, proxyObj);
            }
            break;

        case SL_OBJECTID_AUDIORECORDER:
            result = ValidateRecorderConfig(iConfig);
            if (result == SL_RESULT_SUCCESS) {
                result = AllocRecorderRoutingProxy(iConfig, proxyObj);
            }
            break;

        default:
            result = SL_RESULT_PARAMETER_INVALID;
            break;
        }
    }

    SL_LEAVE_INTERFACE
}

/*
 * Release a Java proxy object, such as AudioRouting object, (and any resources it is holding).
 * Parameters:
 *   self   An SLAndroidConfigurationItf obtained from either an OpenSL ES AudioPlayer
 *          or AudioRecorder.
 *   j_env  The Java Environment pointer (passed in to the calling JNI function).
 *   proxyType Specifies the type of proxy object. Currently only SL_ANDROID_JAVA_PROXY_ROUTING
 *          is supported.
 * Returns SL_RESULT_SUCCESS is the proxy object is release, SL_RESULT_PARAMETER_INVALID if
 *   there is a problem with the arguments causing the function to fail,
 */
static SLresult IAndroidConfiguration_ReleaseJavaProxy(SLAndroidConfigurationItf self,
                                                       SLuint32 proxyType) {
    SL_ENTER_INTERFACE

    IAndroidConfiguration* iConfig = (IAndroidConfiguration*)self;

    if (self == NULL ||
            proxyType != SL_ANDROID_JAVA_PROXY_ROUTING ||
            iConfig->mRoutingProxy == NULL) {
        result =  SL_RESULT_PARAMETER_INVALID;
    } else {
        int objID = IObjectToObjectID(InterfaceToIObject(iConfig));
        switch (objID) {
        case SL_OBJECTID_AUDIOPLAYER:
            {
                JNIEnv* j_env = android::AndroidRuntime::getJNIEnv();

                j_env->ExceptionClear();
                j_env->CallVoidMethod(iConfig->mRoutingProxy, gMidAudioTrackRoutingProxy_release);
                if (j_env->ExceptionCheck()) {
                    SL_LOGE("Java exception releasing recorder routing object.");
                    result = SL_RESULT_INTERNAL_ERROR;
                }
                j_env->DeleteGlobalRef(iConfig->mRoutingProxy);
                iConfig->mRoutingProxy = NULL;
            }
            break;

        case SL_OBJECTID_AUDIORECORDER:
            {
                JNIEnv* j_env = android::AndroidRuntime::getJNIEnv();

                j_env->ExceptionClear();
                j_env->CallVoidMethod(iConfig->mRoutingProxy, gMidAudioRecordRoutingProxy_release);
                if (j_env->ExceptionCheck()) {
                    SL_LOGE("Java exception releasing recorder routing object.");
                    result = SL_RESULT_INTERNAL_ERROR;
                }
                j_env->DeleteGlobalRef(iConfig->mRoutingProxy);
                iConfig->mRoutingProxy = NULL;
            }
            break;
        }

        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}

static const struct SLAndroidConfigurationItf_ IAndroidConfiguration_Itf = {
    IAndroidConfiguration_SetConfiguration,
    IAndroidConfiguration_GetConfiguration,
    IAndroidConfiguration_AcquireJavaProxy,
    IAndroidConfiguration_ReleaseJavaProxy
};

void IAndroidConfiguration_init(void *self)
{
    IAndroidConfiguration *thiz = (IAndroidConfiguration *) self;
    thiz->mItf = &IAndroidConfiguration_Itf;
}

void IAndroidConfiguration_deinit(void *self)
{
    IAndroidConfiguration *thiz = (IAndroidConfiguration *) self;
    if (thiz->mRoutingProxy != NULL) {
        thiz->mItf->ReleaseJavaProxy(&thiz->mItf, SL_ANDROID_JAVA_PROXY_ROUTING);
    }
}

