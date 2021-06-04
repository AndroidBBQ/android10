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

/* Engine implementation */

#include <endian.h>
#include "sles_allinclusive.h"


/* Utility functions */

static SLresult initializeBufferQueueMembers(CAudioPlayer *ap) {
    // inline allocation of circular mArray, up to a typical max
    if (BUFFER_HEADER_TYPICAL >= ap->mBufferQueue.mNumBuffers) {
        ap->mBufferQueue.mArray = ap->mBufferQueue.mTypical;
    } else {
        // Avoid possible integer overflow during multiplication; this arbitrary
        // maximum is big enough to not interfere with real applications, but
        // small enough to not overflow.
        if (ap->mBufferQueue.mNumBuffers >= 256) {
            return SL_RESULT_MEMORY_FAILURE;
        }
        ap->mBufferQueue.mArray = (BufferHeader *)
                malloc((ap->mBufferQueue.mNumBuffers + 1) * sizeof(BufferHeader));
        if (NULL == ap->mBufferQueue.mArray) {
            return SL_RESULT_MEMORY_FAILURE;
        }
    }
    ap->mBufferQueue.mFront = ap->mBufferQueue.mArray;
    ap->mBufferQueue.mRear = ap->mBufferQueue.mArray;
    return SL_RESULT_SUCCESS;
}

#ifdef ANDROID
static SLresult initializeAndroidBufferQueueMembers(CAudioPlayer *ap) {
    // Avoid possible integer overflow during multiplication; this arbitrary
    // maximum is big enough to not interfere with real applications, but
    // small enough to not overflow.
    if (ap->mAndroidBufferQueue.mNumBuffers >= 256) {
        return SL_RESULT_MEMORY_FAILURE;
    }
    ap->mAndroidBufferQueue.mBufferArray = (AdvancedBufferHeader *)
            malloc( (ap->mAndroidBufferQueue.mNumBuffers + 1) * sizeof(AdvancedBufferHeader));
    if (NULL == ap->mAndroidBufferQueue.mBufferArray) {
        return SL_RESULT_MEMORY_FAILURE;
    } else {

        // initialize ABQ buffer type
        // assert below has been checked in android_audioPlayer_checkSourceSink
        assert(SL_DATAFORMAT_MIME == ap->mDataSource.mFormat.mFormatType);
        switch (ap->mDataSource.mFormat.mMIME.containerType) {
          case SL_CONTAINERTYPE_MPEG_TS:
            ap->mAndroidBufferQueue.mBufferType = kAndroidBufferTypeMpeg2Ts;
            break;
          case SL_CONTAINERTYPE_AAC:
          case SL_CONTAINERTYPE_RAW: {
            const char* mime = (char*)ap->mDataSource.mFormat.mMIME.mimeType;
            if ((mime != NULL) && !(strcasecmp(mime, (const char *)SL_ANDROID_MIME_AACADTS) &&
                    strcasecmp(mime, ANDROID_MIME_AACADTS_ANDROID_FRAMEWORK))) {
                ap->mAndroidBufferQueue.mBufferType = kAndroidBufferTypeAacadts;
            } else {
                ap->mAndroidBufferQueue.mBufferType = kAndroidBufferTypeInvalid;
                SL_LOGE("CreateAudioPlayer: Invalid buffer type in Android Buffer Queue");
                return SL_RESULT_CONTENT_UNSUPPORTED;
            }
          } break;
          default:
            ap->mAndroidBufferQueue.mBufferType = kAndroidBufferTypeInvalid;
            SL_LOGE("CreateAudioPlayer: Invalid buffer type in Android Buffer Queue");
            return SL_RESULT_CONTENT_UNSUPPORTED;
        }

        ap->mAndroidBufferQueue.mFront = ap->mAndroidBufferQueue.mBufferArray;
        ap->mAndroidBufferQueue.mRear  = ap->mAndroidBufferQueue.mBufferArray;
    }

    return SL_RESULT_SUCCESS;
}
#endif


static SLresult IEngine_CreateLEDDevice(SLEngineItf self, SLObjectItf *pDevice, SLuint32 deviceID,
    SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    SL_ENTER_INTERFACE

#if USE_PROFILES & USE_PROFILES_OPTIONAL
    if ((NULL == pDevice) || (SL_DEFAULTDEVICEID_LED != deviceID)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pDevice = NULL;
        unsigned exposedMask;
        const ClassTable *pCLEDDevice_class = objectIDtoClass(SL_OBJECTID_LEDDEVICE);
        if (NULL == pCLEDDevice_class) {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        } else {
            result = checkInterfaces(pCLEDDevice_class, numInterfaces, pInterfaceIds,
                pInterfaceRequired, &exposedMask, NULL);
        }
        if (SL_RESULT_SUCCESS == result) {
            CLEDDevice *thiz = (CLEDDevice *) construct(pCLEDDevice_class, exposedMask, self);
            if (NULL == thiz) {
                result = SL_RESULT_MEMORY_FAILURE;
            } else {
                thiz->mDeviceID = deviceID;
                IObject_Publish(&thiz->mObject);
                // return the new LED object
                *pDevice = &thiz->mObject.mItf;
            }
        }
    }
#else
    result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif

    SL_LEAVE_INTERFACE
}


static SLresult IEngine_CreateVibraDevice(SLEngineItf self, SLObjectItf *pDevice, SLuint32 deviceID,
    SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    SL_ENTER_INTERFACE

#if USE_PROFILES & USE_PROFILES_OPTIONAL
    if ((NULL == pDevice) || (SL_DEFAULTDEVICEID_VIBRA != deviceID)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pDevice = NULL;
        unsigned exposedMask;
        const ClassTable *pCVibraDevice_class = objectIDtoClass(SL_OBJECTID_VIBRADEVICE);
        if (NULL == pCVibraDevice_class) {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        } else {
            result = checkInterfaces(pCVibraDevice_class, numInterfaces,
                pInterfaceIds, pInterfaceRequired, &exposedMask, NULL);
        }
        if (SL_RESULT_SUCCESS == result) {
            CVibraDevice *thiz = (CVibraDevice *) construct(pCVibraDevice_class, exposedMask, self);
            if (NULL == thiz) {
                result = SL_RESULT_MEMORY_FAILURE;
            } else {
                thiz->mDeviceID = deviceID;
                IObject_Publish(&thiz->mObject);
                // return the new vibra object
                *pDevice = &thiz->mObject.mItf;
            }
        }
    }
#else
    result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif

    SL_LEAVE_INTERFACE
}


static SLresult IEngine_CreateAudioPlayer(SLEngineItf self, SLObjectItf *pPlayer,
    SLDataSource *pAudioSrc, SLDataSink *pAudioSnk, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    SL_ENTER_INTERFACE

    if (NULL == pPlayer) {
       result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pPlayer = NULL;
        unsigned exposedMask, requiredMask;
        const ClassTable *pCAudioPlayer_class = objectIDtoClass(SL_OBJECTID_AUDIOPLAYER);
        assert(NULL != pCAudioPlayer_class);
        result = checkInterfaces(pCAudioPlayer_class, numInterfaces,
            pInterfaceIds, pInterfaceRequired, &exposedMask, &requiredMask);
        if (SL_RESULT_SUCCESS == result) {

            // Construct our new AudioPlayer instance
            CAudioPlayer *thiz = (CAudioPlayer *) construct(pCAudioPlayer_class, exposedMask, self);
            if (NULL == thiz) {
                result = SL_RESULT_MEMORY_FAILURE;
            } else {

                do {

                    // Initialize private fields not associated with an interface

                    // Default data source in case of failure in checkDataSource
                    thiz->mDataSource.mLocator.mLocatorType = SL_DATALOCATOR_NULL;
                    thiz->mDataSource.mFormat.mFormatType = SL_DATAFORMAT_NULL;

                    // Default data sink in case of failure in checkDataSink
                    thiz->mDataSink.mLocator.mLocatorType = SL_DATALOCATOR_NULL;
                    thiz->mDataSink.mFormat.mFormatType = SL_DATAFORMAT_NULL;

                    // Default is no per-channel mute or solo
                    thiz->mMuteMask = 0;
                    thiz->mSoloMask = 0;

                    // Will be set soon for PCM buffer queues, or later by platform-specific code
                    // during Realize or Prefetch
                    thiz->mNumChannels = UNKNOWN_NUMCHANNELS;
                    thiz->mSampleRateMilliHz = UNKNOWN_SAMPLERATE;

                    // More default values, in case destructor needs to be called early
                    thiz->mDirectLevel = 0; // no attenuation
#ifdef USE_OUTPUTMIXEXT
                    thiz->mTrack = NULL;
                    thiz->mGains[0] = 1.0f;
                    thiz->mGains[1] = 1.0f;
                    thiz->mDestroyRequested = SL_BOOLEAN_FALSE;
#endif
#ifdef USE_SNDFILE
                    thiz->mSndFile.mPathname = NULL;
                    thiz->mSndFile.mSNDFILE = NULL;
                    memset(&thiz->mSndFile.mSfInfo, 0, sizeof(SF_INFO));
                    memset(&thiz->mSndFile.mMutex, 0, sizeof(pthread_mutex_t));
                    thiz->mSndFile.mEOF = SL_BOOLEAN_FALSE;
                    thiz->mSndFile.mWhich = 0;
                    memset(thiz->mSndFile.mBuffer, 0, sizeof(thiz->mSndFile.mBuffer));
#endif
#ifdef ANDROID
                    // placement new (explicit constructor)
                    // FIXME unnecessary once those fields are encapsulated in one class, rather
                    //   than a structure
                    //###(void) new (&thiz->mAudioTrack) android::sp<android::AudioTrack>();
                    (void) new (&thiz->mTrackPlayer) android::sp<android::TrackPlayerBase>();
                    (void) new (&thiz->mCallbackProtector)
                            android::sp<android::CallbackProtector>();
                    (void) new (&thiz->mAuxEffect) android::sp<android::AudioEffect>();
                    (void) new (&thiz->mAPlayer) android::sp<android::GenericPlayer>();
                    // Android-specific POD fields are initialized in android_audioPlayer_create,
                    // and assume calloc or memset 0 during allocation
#endif

                    // Check the source and sink parameters against generic constraints,
                    // and make a local copy of all parameters in case other application threads
                    // change memory concurrently.

                    result = checkDataSource("pAudioSrc", pAudioSrc, &thiz->mDataSource,
                            DATALOCATOR_MASK_URI | DATALOCATOR_MASK_ADDRESS |
                            DATALOCATOR_MASK_BUFFERQUEUE
#ifdef ANDROID
                            | DATALOCATOR_MASK_ANDROIDFD | DATALOCATOR_MASK_ANDROIDSIMPLEBUFFERQUEUE
                            | DATALOCATOR_MASK_ANDROIDBUFFERQUEUE
#endif
                            , DATAFORMAT_MASK_MIME | DATAFORMAT_MASK_PCM | DATAFORMAT_MASK_PCM_EX);

                    if (SL_RESULT_SUCCESS != result) {
                        break;
                    }

                    result = checkDataSink("pAudioSnk", pAudioSnk, &thiz->mDataSink,
                            DATALOCATOR_MASK_OUTPUTMIX                  // for playback
#ifdef ANDROID
                            | DATALOCATOR_MASK_ANDROIDSIMPLEBUFFERQUEUE // for decode to a BQ
                            | DATALOCATOR_MASK_BUFFERQUEUE              // for decode to a BQ
#endif
                            , DATAFORMAT_MASK_NULL
#ifdef ANDROID
                            | DATAFORMAT_MASK_PCM | DATAFORMAT_MASK_PCM_EX  // for decode to PCM
#endif
                            );
                    if (SL_RESULT_SUCCESS != result) {
                        break;
                    }

                    // It would be unsafe to ever refer to the application pointers again
                    pAudioSrc = NULL;
                    pAudioSnk = NULL;

                    // Check that the requested interfaces are compatible with data source and sink
                    result = checkSourceSinkVsInterfacesCompatibility(&thiz->mDataSource,
                            &thiz->mDataSink, pCAudioPlayer_class, requiredMask);
                    if (SL_RESULT_SUCCESS != result) {
                        break;
                    }

                    // copy the buffer queue count from source locator (for playback) / from the
                    // sink locator (for decode on ANDROID build) to the buffer queue interface
                    // we have already range-checked the value down to a smaller width
                    SLuint16 nbBuffers = 0;
                    bool usesAdvancedBufferHeaders = false;
                    bool usesSimpleBufferQueue = false;
                    // creating an AudioPlayer which decodes AAC ADTS buffers to a PCM buffer queue
                    //  will cause usesAdvancedBufferHeaders and usesSimpleBufferQueue to be true
                    switch (thiz->mDataSource.mLocator.mLocatorType) {
                    case SL_DATALOCATOR_BUFFERQUEUE:
#ifdef ANDROID
                    case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE:
#endif
                        usesSimpleBufferQueue = true;
                        nbBuffers = (SLuint16) thiz->mDataSource.mLocator.mBufferQueue.numBuffers;
                        assert(SL_DATAFORMAT_PCM == thiz->mDataSource.mFormat.mFormatType
                                || SL_ANDROID_DATAFORMAT_PCM_EX
                                    == thiz->mDataSource.mFormat.mFormatType);
                        thiz->mNumChannels = thiz->mDataSource.mFormat.mPCM.numChannels;
                        thiz->mSampleRateMilliHz = thiz->mDataSource.mFormat.mPCM.samplesPerSec;
                        break;
#ifdef ANDROID
                    case SL_DATALOCATOR_ANDROIDBUFFERQUEUE:
                        usesAdvancedBufferHeaders = true;
                        nbBuffers = (SLuint16) thiz->mDataSource.mLocator.mABQ.numBuffers;
                        thiz->mAndroidBufferQueue.mNumBuffers = nbBuffers;
                        break;
#endif
                    default:
                        nbBuffers = 0;
                        break;
                    }
#ifdef ANDROID
                    switch (thiz->mDataSink.mLocator.mLocatorType) {
                    case SL_DATALOCATOR_BUFFERQUEUE:
                    case SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE:
                        usesSimpleBufferQueue = true;
                        nbBuffers = thiz->mDataSink.mLocator.mBufferQueue.numBuffers;
                        assert(SL_DATAFORMAT_PCM == thiz->mDataSink.mFormat.mFormatType
                                || SL_ANDROID_DATAFORMAT_PCM_EX
                                    == thiz->mDataSink.mFormat.mFormatType);
                        // FIXME The values specified by the app are meaningless. We get the
                        // real values from the decoder.  But the data sink checks currently require
                        // that the app specify these useless values.  Needs doc/fix.
                        // Instead use the "unknown" values, as needed by prepare completion.
                        // thiz->mNumChannels = thiz->mDataSink.mFormat.mPCM.numChannels;
                        // thiz->mSampleRateMilliHz = thiz->mDataSink.mFormat.mPCM.samplesPerSec;
                        thiz->mNumChannels = UNKNOWN_NUMCHANNELS;
                        thiz->mSampleRateMilliHz = UNKNOWN_SAMPLERATE;
                        break;
                    default:
                        // leave nbBuffers unchanged
                        break;
                    }
#endif
                    thiz->mBufferQueue.mNumBuffers = nbBuffers;

                    // check the audio source and sink parameters against platform support
#ifdef ANDROID
                    result = android_audioPlayer_checkSourceSink(thiz);
                    if (SL_RESULT_SUCCESS != result) {
                        break;
                    }
#endif

#ifdef USE_SNDFILE
                    result = SndFile_checkAudioPlayerSourceSink(thiz);
                    if (SL_RESULT_SUCCESS != result) {
                        break;
                    }
#endif

#ifdef USE_OUTPUTMIXEXT
                    result = IOutputMixExt_checkAudioPlayerSourceSink(thiz);
                    if (SL_RESULT_SUCCESS != result) {
                        break;
                    }
#endif

                    // Allocate memory for buffer queue
                    if (usesAdvancedBufferHeaders) {
#ifdef ANDROID
                        // locator is SL_DATALOCATOR_ANDROIDBUFFERQUEUE
                        result = initializeAndroidBufferQueueMembers(thiz);
#else
                        assert(false);
#endif
                    }

                    if (usesSimpleBufferQueue) {
                        // locator is SL_DATALOCATOR_BUFFERQUEUE
                        //         or SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE
                        result = initializeBufferQueueMembers(thiz);
                    }

                    // used to store the data source of our audio player
                    thiz->mDynamicSource.mDataSource = &thiz->mDataSource.u.mSource;

                    // platform-specific initialization
#ifdef ANDROID
                    android_audioPlayer_create(thiz);
#endif

                } while (0);

                if (SL_RESULT_SUCCESS != result) {
                    IObject_Destroy(&thiz->mObject.mItf);
                } else {
                    IObject_Publish(&thiz->mObject);
                    // return the new audio player object
                    *pPlayer = &thiz->mObject.mItf;
                }

            }
        }

    }

    SL_LEAVE_INTERFACE
}


static SLresult IEngine_CreateAudioRecorder(SLEngineItf self, SLObjectItf *pRecorder,
    SLDataSource *pAudioSrc, SLDataSink *pAudioSnk, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    SL_ENTER_INTERFACE

#if (USE_PROFILES & USE_PROFILES_OPTIONAL) || defined(ANDROID)
    if (NULL == pRecorder) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pRecorder = NULL;
        unsigned exposedMask;
        const ClassTable *pCAudioRecorder_class = objectIDtoClass(SL_OBJECTID_AUDIORECORDER);
        if (NULL == pCAudioRecorder_class) {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        } else {
            result = checkInterfaces(pCAudioRecorder_class, numInterfaces,
                    pInterfaceIds, pInterfaceRequired, &exposedMask, NULL);
        }

        if (SL_RESULT_SUCCESS == result) {

            // Construct our new AudioRecorder instance
            CAudioRecorder *thiz = (CAudioRecorder *) construct(pCAudioRecorder_class, exposedMask,
                    self);
            if (NULL == thiz) {
                result = SL_RESULT_MEMORY_FAILURE;
            } else {

                do {

                    // Initialize fields not associated with any interface

                    // Default data source in case of failure in checkDataSource
                    thiz->mDataSource.mLocator.mLocatorType = SL_DATALOCATOR_NULL;
                    thiz->mDataSource.mFormat.mFormatType = SL_DATAFORMAT_NULL;

                    // Default data sink in case of failure in checkDataSink
                    thiz->mDataSink.mLocator.mLocatorType = SL_DATALOCATOR_NULL;
                    thiz->mDataSink.mFormat.mFormatType = SL_DATAFORMAT_NULL;

                    // These fields are set to real values by
                    // android_audioRecorder_checkSourceSink.  Note that the data sink is
                    // always PCM buffer queue, so we know the channel count and sample rate early.
                    thiz->mNumChannels = UNKNOWN_NUMCHANNELS;
                    thiz->mSampleRateMilliHz = UNKNOWN_SAMPLERATE;
#ifdef ANDROID
                    // placement new (explicit constructor)
                    // FIXME unnecessary once those fields are encapsulated in one class, rather
                    //   than a structure
                    (void) new (&thiz->mAudioRecord) android::sp<android::AudioRecord>();
                    (void) new (&thiz->mCallbackProtector)
                            android::sp<android::CallbackProtector>();
                    thiz->mRecordSource = AUDIO_SOURCE_DEFAULT;
#endif

                    // Check the source and sink parameters, and make a local copy of all parameters
                    result = checkDataSource("pAudioSrc", pAudioSrc, &thiz->mDataSource,
                            DATALOCATOR_MASK_IODEVICE, DATAFORMAT_MASK_NULL);
                    if (SL_RESULT_SUCCESS != result) {
                        break;
                    }
                    result = checkDataSink("pAudioSnk", pAudioSnk, &thiz->mDataSink,
                            DATALOCATOR_MASK_URI
#ifdef ANDROID
                            | DATALOCATOR_MASK_ANDROIDSIMPLEBUFFERQUEUE
#endif
                            , DATAFORMAT_MASK_MIME | DATAFORMAT_MASK_PCM | DATAFORMAT_MASK_PCM_EX
                    );
                    if (SL_RESULT_SUCCESS != result) {
                        break;
                    }

                    // It would be unsafe to ever refer to the application pointers again
                    pAudioSrc = NULL;
                    pAudioSnk = NULL;

                    // check the audio source and sink parameters against platform support
#ifdef ANDROID
                    result = android_audioRecorder_checkSourceSink(thiz);
                    if (SL_RESULT_SUCCESS != result) {
                        SL_LOGE("Cannot create AudioRecorder: invalid source or sink");
                        break;
                    }
#endif

#ifdef ANDROID
                    // Allocate memory for buffer queue
                    SLuint32 locatorType = thiz->mDataSink.mLocator.mLocatorType;
                    if (locatorType == SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE) {
                        thiz->mBufferQueue.mNumBuffers =
                            thiz->mDataSink.mLocator.mBufferQueue.numBuffers;
                        // inline allocation of circular Buffer Queue mArray, up to a typical max
                        if (BUFFER_HEADER_TYPICAL >= thiz->mBufferQueue.mNumBuffers) {
                            thiz->mBufferQueue.mArray = thiz->mBufferQueue.mTypical;
                        } else {
                            // Avoid possible integer overflow during multiplication; this arbitrary
                            // maximum is big enough to not interfere with real applications, but
                            // small enough to not overflow.
                            if (thiz->mBufferQueue.mNumBuffers >= 256) {
                                result = SL_RESULT_MEMORY_FAILURE;
                                break;
                            }
                            thiz->mBufferQueue.mArray = (BufferHeader *) malloc((thiz->mBufferQueue.
                                    mNumBuffers + 1) * sizeof(BufferHeader));
                            if (NULL == thiz->mBufferQueue.mArray) {
                                result = SL_RESULT_MEMORY_FAILURE;
                                break;
                            }
                        }
                        thiz->mBufferQueue.mFront = thiz->mBufferQueue.mArray;
                        thiz->mBufferQueue.mRear = thiz->mBufferQueue.mArray;
                    }
#endif

                    // platform-specific initialization
#ifdef ANDROID
                    android_audioRecorder_create(thiz);
#endif

                } while (0);

                if (SL_RESULT_SUCCESS != result) {
                    IObject_Destroy(&thiz->mObject.mItf);
                } else {
                    IObject_Publish(&thiz->mObject);
                    // return the new audio recorder object
                    *pRecorder = &thiz->mObject.mItf;
                }
            }

        }

    }
#else
    result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif

    SL_LEAVE_INTERFACE
}


static SLresult IEngine_CreateMidiPlayer(SLEngineItf self, SLObjectItf *pPlayer,
    SLDataSource *pMIDISrc, SLDataSource *pBankSrc, SLDataSink *pAudioOutput,
    SLDataSink *pVibra, SLDataSink *pLEDArray, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    SL_ENTER_INTERFACE

#if USE_PROFILES & (USE_PROFILES_GAME | USE_PROFILES_PHONE)
    if ((NULL == pPlayer) || (NULL == pMIDISrc) || (NULL == pAudioOutput)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pPlayer = NULL;
        unsigned exposedMask;
        const ClassTable *pCMidiPlayer_class = objectIDtoClass(SL_OBJECTID_MIDIPLAYER);
        if (NULL == pCMidiPlayer_class) {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        } else {
            result = checkInterfaces(pCMidiPlayer_class, numInterfaces,
                pInterfaceIds, pInterfaceRequired, &exposedMask, NULL);
        }
        if (SL_RESULT_SUCCESS == result) {
            CMidiPlayer *thiz = (CMidiPlayer *) construct(pCMidiPlayer_class, exposedMask, self);
            if (NULL == thiz) {
                result = SL_RESULT_MEMORY_FAILURE;
            } else {
#if 0
                "pMIDISrc", pMIDISrc, URI | MIDIBUFFERQUEUE, NONE
                "pBankSrc", pBanksrc, NULL | URI | ADDRESS, NULL
                "pAudioOutput", pAudioOutput, OUTPUTMIX, NULL
                "pVibra", pVibra, NULL | IODEVICE, NULL
                "pLEDArray", pLEDArray, NULL | IODEVICE, NULL
#endif
                // a fake value - why not use value from IPlay_init? what does CT check for?
                thiz->mPlay.mDuration = 0;
                IObject_Publish(&thiz->mObject);
                // return the new MIDI player object
                *pPlayer = &thiz->mObject.mItf;
            }
        }
    }
#else
    result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif

    SL_LEAVE_INTERFACE
}


static SLresult IEngine_CreateListener(SLEngineItf self, SLObjectItf *pListener,
    SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    SL_ENTER_INTERFACE

#if USE_PROFILES & USE_PROFILES_GAME
    if (NULL == pListener) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pListener = NULL;
        unsigned exposedMask;
        const ClassTable *pCListener_class = objectIDtoClass(SL_OBJECTID_LISTENER);
        if (NULL == pCListener_class) {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        } else {
            result = checkInterfaces(pCListener_class, numInterfaces,
                pInterfaceIds, pInterfaceRequired, &exposedMask, NULL);
        }
        if (SL_RESULT_SUCCESS == result) {
            CListener *thiz = (CListener *) construct(pCListener_class, exposedMask, self);
            if (NULL == thiz) {
                result = SL_RESULT_MEMORY_FAILURE;
            } else {
                IObject_Publish(&thiz->mObject);
                // return the new 3D listener object
                *pListener = &thiz->mObject.mItf;
            }
        }
    }
#else
    result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif

    SL_LEAVE_INTERFACE
}


static SLresult IEngine_Create3DGroup(SLEngineItf self, SLObjectItf *pGroup, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    SL_ENTER_INTERFACE

#if USE_PROFILES & USE_PROFILES_GAME
    if (NULL == pGroup) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pGroup = NULL;
        unsigned exposedMask;
        const ClassTable *pC3DGroup_class = objectIDtoClass(SL_OBJECTID_3DGROUP);
        if (NULL == pC3DGroup_class) {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        } else {
            result = checkInterfaces(pC3DGroup_class, numInterfaces,
                pInterfaceIds, pInterfaceRequired, &exposedMask, NULL);
        }
        if (SL_RESULT_SUCCESS == result) {
            C3DGroup *thiz = (C3DGroup *) construct(pC3DGroup_class, exposedMask, self);
            if (NULL == thiz) {
                result = SL_RESULT_MEMORY_FAILURE;
            } else {
                thiz->mMemberMask = 0;
                IObject_Publish(&thiz->mObject);
                // return the new 3D group object
                *pGroup = &thiz->mObject.mItf;
            }
        }
    }
#else
    result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif

    SL_LEAVE_INTERFACE
}


static SLresult IEngine_CreateOutputMix(SLEngineItf self, SLObjectItf *pMix, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    SL_ENTER_INTERFACE

    if (NULL == pMix) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pMix = NULL;
        unsigned exposedMask;
        const ClassTable *pCOutputMix_class = objectIDtoClass(SL_OBJECTID_OUTPUTMIX);
        assert(NULL != pCOutputMix_class);
        result = checkInterfaces(pCOutputMix_class, numInterfaces,
            pInterfaceIds, pInterfaceRequired, &exposedMask, NULL);
        if (SL_RESULT_SUCCESS == result) {
            COutputMix *thiz = (COutputMix *) construct(pCOutputMix_class, exposedMask, self);
            if (NULL == thiz) {
                result = SL_RESULT_MEMORY_FAILURE;
            } else {
#ifdef ANDROID
                android_outputMix_create(thiz);
#endif
#ifdef USE_SDL
                IEngine *thisEngine = &thiz->mObject.mEngine->mEngine;
                interface_lock_exclusive(thisEngine);
                bool unpause = false;
                if (NULL == thisEngine->mOutputMix) {
                    thisEngine->mOutputMix = thiz;
                    unpause = true;
                }
                interface_unlock_exclusive(thisEngine);
#endif
                IObject_Publish(&thiz->mObject);
#ifdef USE_SDL
                if (unpause) {
                    // Enable SDL_callback to be called periodically by SDL's internal thread
                    SDL_PauseAudio(0);
                }
#endif
                // return the new output mix object
                *pMix = &thiz->mObject.mItf;
            }
        }
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEngine_CreateMetadataExtractor(SLEngineItf self, SLObjectItf *pMetadataExtractor,
    SLDataSource *pDataSource, SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds,
    const SLboolean *pInterfaceRequired)
{
    SL_ENTER_INTERFACE

#if USE_PROFILES & (USE_PROFILES_GAME | USE_PROFILES_MUSIC)
    if (NULL == pMetadataExtractor) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pMetadataExtractor = NULL;
        unsigned exposedMask;
        const ClassTable *pCMetadataExtractor_class =
            objectIDtoClass(SL_OBJECTID_METADATAEXTRACTOR);
        if (NULL == pCMetadataExtractor_class) {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        } else {
            result = checkInterfaces(pCMetadataExtractor_class, numInterfaces,
                pInterfaceIds, pInterfaceRequired, &exposedMask, NULL);
        }
        if (SL_RESULT_SUCCESS == result) {
            CMetadataExtractor *thiz = (CMetadataExtractor *)
                construct(pCMetadataExtractor_class, exposedMask, self);
            if (NULL == thiz) {
                result = SL_RESULT_MEMORY_FAILURE;
            } else {
#if 0
                "pDataSource", pDataSource, NONE, NONE
#endif
                IObject_Publish(&thiz->mObject);
                // return the new metadata extractor object
                *pMetadataExtractor = &thiz->mObject.mItf;
                result = SL_RESULT_SUCCESS;
            }
        }
    }
#else
    result = SL_RESULT_FEATURE_UNSUPPORTED;
#endif

    SL_LEAVE_INTERFACE
}


static SLresult IEngine_CreateExtensionObject(SLEngineItf self, SLObjectItf *pObject,
    void *pParameters, SLuint32 objectID, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    SL_ENTER_INTERFACE

    if (NULL == pObject) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pObject = NULL;
        result = SL_RESULT_FEATURE_UNSUPPORTED;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEngine_QueryNumSupportedInterfaces(SLEngineItf self,
    SLuint32 objectID, SLuint32 *pNumSupportedInterfaces)
{
    SL_ENTER_INTERFACE

    if (NULL == pNumSupportedInterfaces) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        const ClassTable *clazz = objectIDtoClass(objectID);
        if (NULL == clazz) {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        } else {
            SLuint32 count = 0;
            SLuint32 i;
            for (i = 0; i < clazz->mInterfaceCount; ++i) {
                switch (clazz->mInterfaces[i].mInterface) {
                case INTERFACE_IMPLICIT:
                case INTERFACE_IMPLICIT_PREREALIZE:
                case INTERFACE_EXPLICIT:
                case INTERFACE_EXPLICIT_PREREALIZE:
                case INTERFACE_DYNAMIC:
                    ++count;
                    break;
                case INTERFACE_UNAVAILABLE:
                    break;
                default:
                    assert(false);
                    break;
                }
            }
            *pNumSupportedInterfaces = count;
            result = SL_RESULT_SUCCESS;
        }
    }

    SL_LEAVE_INTERFACE;
}


static SLresult IEngine_QuerySupportedInterfaces(SLEngineItf self,
    SLuint32 objectID, SLuint32 index, SLInterfaceID *pInterfaceId)
{
    SL_ENTER_INTERFACE

    if (NULL == pInterfaceId) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pInterfaceId = NULL;
        const ClassTable *clazz = objectIDtoClass(objectID);
        if (NULL == clazz) {
            result = SL_RESULT_FEATURE_UNSUPPORTED;
        } else {
            result = SL_RESULT_PARAMETER_INVALID; // will be reset later
            SLuint32 i;
            for (i = 0; i < clazz->mInterfaceCount; ++i) {
                switch (clazz->mInterfaces[i].mInterface) {
                case INTERFACE_IMPLICIT:
                case INTERFACE_IMPLICIT_PREREALIZE:
                case INTERFACE_EXPLICIT:
                case INTERFACE_EXPLICIT_PREREALIZE:
                case INTERFACE_DYNAMIC:
                    break;
                case INTERFACE_UNAVAILABLE:
                    continue;
                default:
                    assert(false);
                    break;
                }
                if (index == 0) {
                    *pInterfaceId = &SL_IID_array[clazz->mInterfaces[i].mMPH];
                    result = SL_RESULT_SUCCESS;
                    break;
                }
                --index;
            }
        }
    }

    SL_LEAVE_INTERFACE
};


static const char * const extensionNames[] = {
#ifdef ANDROID
#define _(n) #n
#define __(n) _(n)
    "ANDROID_SDK_LEVEL_" __(PLATFORM_SDK_VERSION),
#undef _
#undef __
#else
    "WILHELM_DESKTOP",
#endif
};


static SLresult IEngine_QueryNumSupportedExtensions(SLEngineItf self, SLuint32 *pNumExtensions)
{
    SL_ENTER_INTERFACE

    if (NULL == pNumExtensions) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pNumExtensions = sizeof(extensionNames) / sizeof(extensionNames[0]);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEngine_QuerySupportedExtension(SLEngineItf self,
    SLuint32 index, SLchar *pExtensionName, SLint16 *pNameLength)
{
    SL_ENTER_INTERFACE

    if (NULL == pNameLength) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        size_t actualNameLength;
        unsigned numExtensions = sizeof(extensionNames) / sizeof(extensionNames[0]);
        if (index >= numExtensions) {
            actualNameLength = 0;
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
            const char *extensionName = extensionNames[index];
            actualNameLength = strlen(extensionName) + 1;
            if (NULL == pExtensionName) {
                // application is querying the name length in order to allocate a buffer
                result = SL_RESULT_SUCCESS;
            } else {
                SLint16 availableNameLength = *pNameLength;
                if (0 >= availableNameLength) {
                    // there is not even room for the terminating NUL
                    result = SL_RESULT_BUFFER_INSUFFICIENT;
                } else if (actualNameLength > (size_t) availableNameLength) {
                    // "no invalid strings are written. That is, the null-terminator always exists"
                    memcpy(pExtensionName, extensionName, (size_t) availableNameLength - 1);
                    pExtensionName[(size_t) availableNameLength - 1] = '\0';
                    result = SL_RESULT_BUFFER_INSUFFICIENT;
                } else {
                    memcpy(pExtensionName, extensionName, actualNameLength);
                    result = SL_RESULT_SUCCESS;
                }
            }
        }
        *pNameLength = actualNameLength;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IEngine_IsExtensionSupported(SLEngineItf self,
    const SLchar *pExtensionName, SLboolean *pSupported)
{
    SL_ENTER_INTERFACE

    if (NULL == pSupported) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        SLboolean isSupported = SL_BOOLEAN_FALSE;
        if (NULL == pExtensionName) {
            result = SL_RESULT_PARAMETER_INVALID;
        } else {
            unsigned numExtensions = sizeof(extensionNames) / sizeof(extensionNames[0]);
            unsigned i;
            for (i = 0; i < numExtensions; ++i) {
                if (!strcmp((const char *) pExtensionName, extensionNames[i])) {
                    isSupported = SL_BOOLEAN_TRUE;
                    break;
                }
            }
            result = SL_RESULT_SUCCESS;
        }
        *pSupported = isSupported;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLEngineItf_ IEngine_Itf = {
    IEngine_CreateLEDDevice,
    IEngine_CreateVibraDevice,
    IEngine_CreateAudioPlayer,
    IEngine_CreateAudioRecorder,
    IEngine_CreateMidiPlayer,
    IEngine_CreateListener,
    IEngine_Create3DGroup,
    IEngine_CreateOutputMix,
    IEngine_CreateMetadataExtractor,
    IEngine_CreateExtensionObject,
    IEngine_QueryNumSupportedInterfaces,
    IEngine_QuerySupportedInterfaces,
    IEngine_QueryNumSupportedExtensions,
    IEngine_QuerySupportedExtension,
    IEngine_IsExtensionSupported
};

void IEngine_init(void *self)
{
    IEngine *thiz = (IEngine *) self;
    thiz->mItf = &IEngine_Itf;
    // mLossOfControlGlobal is initialized in slCreateEngine
#ifdef USE_SDL
    thiz->mOutputMix = NULL;
#endif
    thiz->mInstanceCount = 1; // ourself
    thiz->mInstanceMask = 0;
    thiz->mChangedMask = 0;
    unsigned i;
    for (i = 0; i < MAX_INSTANCE; ++i) {
        thiz->mInstances[i] = NULL;
    }
    thiz->mShutdown = SL_BOOLEAN_FALSE;
    thiz->mShutdownAck = SL_BOOLEAN_FALSE;
#if _BYTE_ORDER == _BIG_ENDIAN
    thiz->mNativeEndianness = SL_BYTEORDER_BIGENDIAN;
#else
    thiz->mNativeEndianness = SL_BYTEORDER_LITTLEENDIAN;
#endif
}

void IEngine_deinit(void *self)
{
}


// OpenMAX AL Engine


static XAresult IEngine_CreateCameraDevice(XAEngineItf self, XAObjectItf *pDevice,
        XAuint32 deviceID, XAuint32 numInterfaces, const XAInterfaceID *pInterfaceIds,
        const XAboolean *pInterfaceRequired)
{
    XA_ENTER_INTERFACE

    //IXAEngine *thiz = (IXAEngine *) self;
    result = SL_RESULT_FEATURE_UNSUPPORTED;

    XA_LEAVE_INTERFACE
}


static XAresult IEngine_CreateRadioDevice(XAEngineItf self, XAObjectItf *pDevice,
        XAuint32 numInterfaces, const XAInterfaceID *pInterfaceIds,
        const XAboolean *pInterfaceRequired)
{
    XA_ENTER_INTERFACE

    //IXAEngine *thiz = (IXAEngine *) self;
    result = SL_RESULT_FEATURE_UNSUPPORTED;

    XA_LEAVE_INTERFACE
}


static XAresult IXAEngine_CreateLEDDevice(XAEngineItf self, XAObjectItf *pDevice, XAuint32 deviceID,
        XAuint32 numInterfaces, const XAInterfaceID *pInterfaceIds,
        const XAboolean *pInterfaceRequired)
{
    // forward to OpenSL ES
    return IEngine_CreateLEDDevice(&((CEngine *) ((IXAEngine *) self)->mThis)->mEngine.mItf,
            (SLObjectItf *) pDevice, deviceID, numInterfaces, (const SLInterfaceID *) pInterfaceIds,
            (const SLboolean *) pInterfaceRequired);
}


static XAresult IXAEngine_CreateVibraDevice(XAEngineItf self, XAObjectItf *pDevice,
        XAuint32 deviceID, XAuint32 numInterfaces, const XAInterfaceID *pInterfaceIds,
        const XAboolean *pInterfaceRequired)
{
    // forward to OpenSL ES
    return IEngine_CreateVibraDevice(&((CEngine *) ((IXAEngine *) self)->mThis)->mEngine.mItf,
            (SLObjectItf *) pDevice, deviceID, numInterfaces, (const SLInterfaceID *) pInterfaceIds,
            (const SLboolean *) pInterfaceRequired);
}


static XAresult IEngine_CreateMediaPlayer(XAEngineItf self, XAObjectItf *pPlayer,
        XADataSource *pDataSrc, XADataSource *pBankSrc, XADataSink *pAudioSnk,
        XADataSink *pImageVideoSnk, XADataSink *pVibra, XADataSink *pLEDArray,
        XAuint32 numInterfaces, const XAInterfaceID *pInterfaceIds,
        const XAboolean *pInterfaceRequired)
{
    XA_ENTER_INTERFACE

    if (NULL == pPlayer) {
        result = XA_RESULT_PARAMETER_INVALID;
    } else {
        *pPlayer = NULL;
        unsigned exposedMask;
        const ClassTable *pCMediaPlayer_class = objectIDtoClass(XA_OBJECTID_MEDIAPLAYER);
        assert(NULL != pCMediaPlayer_class);
        result = checkInterfaces(pCMediaPlayer_class, numInterfaces,
                (const SLInterfaceID *) pInterfaceIds, pInterfaceRequired, &exposedMask, NULL);
        if (XA_RESULT_SUCCESS == result) {

            // Construct our new MediaPlayer instance
            CMediaPlayer *thiz = (CMediaPlayer *) construct(pCMediaPlayer_class, exposedMask,
                    &((CEngine *) ((IXAEngine *) self)->mThis)->mEngine.mItf);
            if (NULL == thiz) {
                result = XA_RESULT_MEMORY_FAILURE;
            } else {

                do {

                    // Initialize private fields not associated with an interface

                    // Default data source in case of failure in checkDataSource
                    thiz->mDataSource.mLocator.mLocatorType = SL_DATALOCATOR_NULL;
                    thiz->mDataSource.mFormat.mFormatType = XA_DATAFORMAT_NULL;

                    // Default andio and image sink in case of failure in checkDataSink
                    thiz->mAudioSink.mLocator.mLocatorType = XA_DATALOCATOR_NULL;
                    thiz->mAudioSink.mFormat.mFormatType = XA_DATAFORMAT_NULL;
                    thiz->mImageVideoSink.mLocator.mLocatorType = XA_DATALOCATOR_NULL;
                    thiz->mImageVideoSink.mFormat.mFormatType = XA_DATAFORMAT_NULL;

                    // More default values, in case destructor needs to be called early
                    thiz->mNumChannels = UNKNOWN_NUMCHANNELS;

#ifdef ANDROID
                    // placement new (explicit constructor)
                    // FIXME unnecessary once those fields are encapsulated in one class, rather
                    //   than a structure
                    (void) new (&thiz->mAVPlayer) android::sp<android::GenericPlayer>();
                    (void) new (&thiz->mCallbackProtector)
                            android::sp<android::CallbackProtector>();
                    // Android-specific POD fields are initialized in android_Player_create,
                    // and assume calloc or memset 0 during allocation
#endif

                    // Check the source and sink parameters against generic constraints

                    result = checkDataSource("pDataSrc", (const SLDataSource *) pDataSrc,
                            &thiz->mDataSource, DATALOCATOR_MASK_URI
#ifdef ANDROID
                            | DATALOCATOR_MASK_ANDROIDFD
                            | DATALOCATOR_MASK_ANDROIDBUFFERQUEUE
#endif
                            , DATAFORMAT_MASK_MIME);
                    if (XA_RESULT_SUCCESS != result) {
                        break;
                    }

                    result = checkDataSource("pBankSrc", (const SLDataSource *) pBankSrc,
                            &thiz->mBankSource, DATALOCATOR_MASK_NULL | DATALOCATOR_MASK_URI |
                            DATALOCATOR_MASK_ADDRESS, DATAFORMAT_MASK_NULL);
                    if (XA_RESULT_SUCCESS != result) {
                        break;
                    }

                    result = checkDataSink("pAudioSnk", (const SLDataSink *) pAudioSnk,
                            &thiz->mAudioSink, DATALOCATOR_MASK_OUTPUTMIX, DATAFORMAT_MASK_NULL);
                    if (XA_RESULT_SUCCESS != result) {
                        break;
                    }

                    result = checkDataSink("pImageVideoSnk", (const SLDataSink *) pImageVideoSnk,
                            &thiz->mImageVideoSink,
                            DATALOCATOR_MASK_NULL | DATALOCATOR_MASK_NATIVEDISPLAY,
                            DATAFORMAT_MASK_NULL);
                    if (XA_RESULT_SUCCESS != result) {
                        break;
                    }

                    result = checkDataSink("pVibra", (const SLDataSink *) pVibra, &thiz->mVibraSink,
                            DATALOCATOR_MASK_NULL | DATALOCATOR_MASK_IODEVICE,
                            DATAFORMAT_MASK_NULL);
                    if (XA_RESULT_SUCCESS != result) {
                        break;
                    }

                    result = checkDataSink("pLEDArray", (const SLDataSink *) pLEDArray,
                            &thiz->mLEDArraySink, DATALOCATOR_MASK_NULL | DATALOCATOR_MASK_IODEVICE,
                            DATAFORMAT_MASK_NULL);
                    if (XA_RESULT_SUCCESS != result) {
                        break;
                    }

                    // Unsafe to ever refer to application pointers again
                    pDataSrc = NULL;
                    pBankSrc = NULL;
                    pAudioSnk = NULL;
                    pImageVideoSnk = NULL;
                    pVibra = NULL;
                    pLEDArray = NULL;

                    // Check that the requested interfaces are compatible with the data source
                    // FIXME implement

                    // check the source and sink parameters against platform support
#ifdef ANDROID
                    result = android_Player_checkSourceSink(thiz);
                    if (XA_RESULT_SUCCESS != result) {
                        break;
                    }
#endif

#ifdef ANDROID
                    // AndroidBufferQueue-specific initialization
                    if (XA_DATALOCATOR_ANDROIDBUFFERQUEUE ==
                            thiz->mDataSource.mLocator.mLocatorType) {
                        XAuint16 nbBuffers = (XAuint16) thiz->mDataSource.mLocator.mABQ.numBuffers;

                        // Avoid possible integer overflow during multiplication; this arbitrary
                        // maximum is big enough to not interfere with real applications, but
                        // small enough to not overflow.
                        if (nbBuffers >= 256) {
                            result = SL_RESULT_MEMORY_FAILURE;
                            break;
                        }

                        // initialize ABQ buffer type
                        // assert below has been checked in android_audioPlayer_checkSourceSink
                        assert(XA_DATAFORMAT_MIME == thiz->mDataSource.mFormat.mFormatType);
                        if (XA_CONTAINERTYPE_MPEG_TS ==
                                thiz->mDataSource.mFormat.mMIME.containerType) {
                            thiz->mAndroidBufferQueue.mBufferType = kAndroidBufferTypeMpeg2Ts;

                            // Set the container type for the StreamInformation interface
                            XAMediaContainerInformation *containerInfo =
                                    (XAMediaContainerInformation*)
                                        // always storing container info at index 0, as per spec
                                        &thiz->mStreamInfo.mStreamInfoTable.itemAt(0).containerInfo;
                            containerInfo->containerType = XA_CONTAINERTYPE_MPEG_TS;
                            // there are no streams at this stage
                            containerInfo->numStreams = 0;

                        } else {
                            thiz->mAndroidBufferQueue.mBufferType = kAndroidBufferTypeInvalid;
                            SL_LOGE("Invalid buffer type in Android Buffer Queue");
                            result = SL_RESULT_CONTENT_UNSUPPORTED;
                        }

                        // initialize ABQ memory
                        thiz->mAndroidBufferQueue.mBufferArray = (AdvancedBufferHeader *)
                                    malloc( (nbBuffers + 1) * sizeof(AdvancedBufferHeader));
                        if (NULL == thiz->mAndroidBufferQueue.mBufferArray) {
                            result = SL_RESULT_MEMORY_FAILURE;
                            break;
                        } else {
                            thiz->mAndroidBufferQueue.mFront =
                                    thiz->mAndroidBufferQueue.mBufferArray;
                            thiz->mAndroidBufferQueue.mRear =
                                    thiz->mAndroidBufferQueue.mBufferArray;
                        }

                        thiz->mAndroidBufferQueue.mNumBuffers = nbBuffers;

                    }
#endif

                    // used to store the data source of our audio player
                    thiz->mDynamicSource.mDataSource = &thiz->mDataSource.u.mSource;

                    // platform-specific initialization
#ifdef ANDROID
                    android_Player_create(thiz);
#endif

                } while (0);

                if (XA_RESULT_SUCCESS != result) {
                    IObject_Destroy(&thiz->mObject.mItf);
                } else {
                    IObject_Publish(&thiz->mObject);
                    // return the new media player object
                    *pPlayer = (XAObjectItf) &thiz->mObject.mItf;
                }

            }
        }

    }

    XA_LEAVE_INTERFACE
}


static XAresult IEngine_CreateMediaRecorder(XAEngineItf self, XAObjectItf *pRecorder,
        XADataSource *pAudioSrc, XADataSource *pImageVideoSrc,
        XADataSink *pDataSnk, XAuint32 numInterfaces, const XAInterfaceID *pInterfaceIds,
        const XAboolean *pInterfaceRequired)
{
    XA_ENTER_INTERFACE

    //IXAEngine *thiz = (IXAEngine *) self;
    result = SL_RESULT_FEATURE_UNSUPPORTED;

#if 0
    "pAudioSrc", pAudioSrc,
    "pImageVideoSrc", pImageVideoSrc,
    "pDataSink", pDataSnk,
#endif

    XA_LEAVE_INTERFACE
}


static XAresult IXAEngine_CreateOutputMix(XAEngineItf self, XAObjectItf *pMix,
        XAuint32 numInterfaces, const XAInterfaceID *pInterfaceIds,
        const XAboolean *pInterfaceRequired)
{
    // forward to OpenSL ES
    return IEngine_CreateOutputMix(&((CEngine *) ((IXAEngine *) self)->mThis)->mEngine.mItf,
            (SLObjectItf *) pMix, numInterfaces, (const SLInterfaceID *) pInterfaceIds,
            (const SLboolean *) pInterfaceRequired);
}


static XAresult IXAEngine_CreateMetadataExtractor(XAEngineItf self, XAObjectItf *pMetadataExtractor,
            XADataSource *pDataSource, XAuint32 numInterfaces,
            const XAInterfaceID *pInterfaceIds, const XAboolean *pInterfaceRequired)
{
    // forward to OpenSL ES
    return IEngine_CreateMetadataExtractor(&((CEngine *) ((IXAEngine *) self)->mThis)->mEngine.mItf,
            (SLObjectItf *) pMetadataExtractor, (SLDataSource *) pDataSource, numInterfaces,
            (const SLInterfaceID *) pInterfaceIds, (const SLboolean *) pInterfaceRequired);
}


static XAresult IXAEngine_CreateExtensionObject(XAEngineItf self, XAObjectItf *pObject,
            void *pParameters, XAuint32 objectID, XAuint32 numInterfaces,
            const XAInterfaceID *pInterfaceIds, const XAboolean *pInterfaceRequired)
{
    // forward to OpenSL ES
    return IEngine_CreateExtensionObject(&((CEngine *) ((IXAEngine *) self)->mThis)->mEngine.mItf,
            (SLObjectItf *) pObject, pParameters, objectID, numInterfaces,
            (const SLInterfaceID *) pInterfaceIds, (const SLboolean *) pInterfaceRequired);
}


static XAresult IEngine_GetImplementationInfo(XAEngineItf self, XAuint32 *pMajor, XAuint32 *pMinor,
        XAuint32 *pStep, /* XAuint32 nImplementationTextSize, */ const XAchar *pImplementationText)
{
    XA_ENTER_INTERFACE

    //IXAEngine *thiz = (IXAEngine *) self;
    result = SL_RESULT_FEATURE_UNSUPPORTED;

    XA_LEAVE_INTERFACE
}


static XAresult IXAEngine_QuerySupportedProfiles(XAEngineItf self, XAint16 *pProfilesSupported)
{
    XA_ENTER_INTERFACE

    if (NULL == pProfilesSupported) {
        result = XA_RESULT_PARAMETER_INVALID;
    } else {
#if 1
        *pProfilesSupported = 0;
        // the code below was copied from OpenSL ES and needs to be adapted for OpenMAX AL.
#else
        // The generic implementation doesn't implement any of the profiles, they shouldn't be
        // declared as supported. Also exclude the fake profiles BASE and OPTIONAL.
        *pProfilesSupported = USE_PROFILES &
                (USE_PROFILES_GAME | USE_PROFILES_MUSIC | USE_PROFILES_PHONE);
#endif
        result = XA_RESULT_SUCCESS;
    }

    XA_LEAVE_INTERFACE
}


static XAresult IXAEngine_QueryNumSupportedInterfaces(XAEngineItf self, XAuint32 objectID,
        XAuint32 *pNumSupportedInterfaces)
{
    // forward to OpenSL ES
    return IEngine_QueryNumSupportedInterfaces(
            &((CEngine *) ((IXAEngine *) self)->mThis)->mEngine.mItf, objectID,
            pNumSupportedInterfaces);
}


static XAresult IXAEngine_QuerySupportedInterfaces(XAEngineItf self, XAuint32 objectID,
        XAuint32 index, XAInterfaceID *pInterfaceId)
{
    // forward to OpenSL ES
    return IEngine_QuerySupportedInterfaces(
            &((CEngine *) ((IXAEngine *) self)->mThis)->mEngine.mItf, objectID, index,
            (SLInterfaceID *) pInterfaceId);
}


static XAresult IXAEngine_QueryNumSupportedExtensions(XAEngineItf self, XAuint32 *pNumExtensions)
{
    // forward to OpenSL ES
    return IEngine_QueryNumSupportedExtensions(
            &((CEngine *) ((IXAEngine *) self)->mThis)->mEngine.mItf, pNumExtensions);
}


static XAresult IXAEngine_QuerySupportedExtension(XAEngineItf self, XAuint32 index,
        XAchar *pExtensionName, XAint16 *pNameLength)
{
    // forward to OpenSL ES
    return IEngine_QuerySupportedExtension(&((CEngine *) ((IXAEngine *) self)->mThis)->mEngine.mItf,
            index, pExtensionName, (SLint16 *) pNameLength);
}


static XAresult IXAEngine_IsExtensionSupported(XAEngineItf self, const XAchar *pExtensionName,
        XAboolean *pSupported)
{
    // forward to OpenSL ES
    return IEngine_IsExtensionSupported(&((CEngine *) ((IXAEngine *) self)->mThis)->mEngine.mItf,
            pExtensionName, pSupported);
}


static XAresult IXAEngine_QueryLEDCapabilities(XAEngineItf self, XAuint32 *pIndex,
        XAuint32 *pLEDDeviceID, XALEDDescriptor *pDescriptor)
{
    // forward to OpenSL ES EngineCapabilities
    return (XAresult) IEngineCapabilities_QueryLEDCapabilities(
            &((CEngine *) ((IXAEngine *) self)->mThis)->mEngineCapabilities.mItf, pIndex,
            pLEDDeviceID, (SLLEDDescriptor *) pDescriptor);
}


static XAresult IXAEngine_QueryVibraCapabilities(XAEngineItf self, XAuint32 *pIndex,
        XAuint32 *pVibraDeviceID, XAVibraDescriptor *pDescriptor)
{
    // forward to OpenSL ES EngineCapabilities
    return (XAresult) IEngineCapabilities_QueryVibraCapabilities(
            &((CEngine *) ((IXAEngine *) self)->mThis)->mEngineCapabilities.mItf, pIndex,
            pVibraDeviceID, (SLVibraDescriptor *) pDescriptor);
}


// OpenMAX AL engine v-table

static const struct XAEngineItf_ IXAEngine_Itf = {
    IEngine_CreateCameraDevice,
    IEngine_CreateRadioDevice,
    IXAEngine_CreateLEDDevice,
    IXAEngine_CreateVibraDevice,
    IEngine_CreateMediaPlayer,
    IEngine_CreateMediaRecorder,
    IXAEngine_CreateOutputMix,
    IXAEngine_CreateMetadataExtractor,
    IXAEngine_CreateExtensionObject,
    IEngine_GetImplementationInfo,
    IXAEngine_QuerySupportedProfiles,
    IXAEngine_QueryNumSupportedInterfaces,
    IXAEngine_QuerySupportedInterfaces,
    IXAEngine_QueryNumSupportedExtensions,
    IXAEngine_QuerySupportedExtension,
    IXAEngine_IsExtensionSupported,
    IXAEngine_QueryLEDCapabilities,
    IXAEngine_QueryVibraCapabilities
};


void IXAEngine_init(void *self)
{
    IXAEngine *thiz = (IXAEngine *) self;
    thiz->mItf = &IXAEngine_Itf;
}


void IXAEngine_deinit(void *self)
{
}
