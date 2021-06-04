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

/* Classes vs. interfaces */

#include "sles_allinclusive.h"


#if USE_PROFILES & USE_PROFILES_GAME

// 3DGroup class

static const struct iid_vtable _3DGroup_interfaces[INTERFACES_3DGroup] = {
    {MPH_OBJECT, INTERFACE_IMPLICIT_PREREALIZE, offsetof(C3DGroup, mObject)},
    {MPH_DYNAMICINTERFACEMANAGEMENT, INTERFACE_IMPLICIT,
        offsetof(C3DGroup, mDynamicInterfaceManagement)},
    {MPH_3DLOCATION, INTERFACE_IMPLICIT, offsetof(C3DGroup, m3DLocation)},
    {MPH_3DDOPPLER, INTERFACE_DYNAMIC_GAME, offsetof(C3DGroup, m3DDoppler)},
    {MPH_3DSOURCE, INTERFACE_EXPLICIT_GAME, offsetof(C3DGroup, m3DSource)},
    {MPH_3DMACROSCOPIC, INTERFACE_OPTIONAL, offsetof(C3DGroup, m3DMacroscopic)},
};

static const ClassTable C3DGroup_class = {
    _3DGroup_interfaces,
    INTERFACES_3DGroup,
    MPH_to_3DGroup,
    "3DGroup",
    sizeof(C3DGroup),
    SL_OBJECTID_3DGROUP,
    0,      // OpenMAX AL object ID
    NULL,
    NULL,
    NULL,
    C3DGroup_PreDestroy
};

#endif


// AudioPlayer class

static const struct iid_vtable AudioPlayer_interfaces[INTERFACES_AudioPlayer] = {
    {MPH_OBJECT, INTERFACE_IMPLICIT_PREREALIZE, offsetof(CAudioPlayer, mObject)},
    {MPH_DYNAMICINTERFACEMANAGEMENT, INTERFACE_IMPLICIT,
        offsetof(CAudioPlayer, mDynamicInterfaceManagement)},
    {MPH_PLAY, INTERFACE_IMPLICIT, offsetof(CAudioPlayer, mPlay)},
    {MPH_3DDOPPLER, INTERFACE_DYNAMIC_GAME, offsetof(CAudioPlayer, m3DDoppler)},
    {MPH_3DGROUPING, INTERFACE_EXPLICIT_GAME, offsetof(CAudioPlayer, m3DGrouping)},
    {MPH_3DLOCATION, INTERFACE_EXPLICIT_GAME, offsetof(CAudioPlayer, m3DLocation)},
    {MPH_3DSOURCE, INTERFACE_EXPLICIT_GAME, offsetof(CAudioPlayer, m3DSource)},
    {MPH_BUFFERQUEUE, INTERFACE_EXPLICIT, offsetof(CAudioPlayer, mBufferQueue)},
    {MPH_EFFECTSEND, INTERFACE_EXPLICIT, offsetof(CAudioPlayer, mEffectSend)},
    {MPH_MUTESOLO, INTERFACE_EXPLICIT, offsetof(CAudioPlayer, mMuteSolo)},
    {MPH_METADATAEXTRACTION, INTERFACE_DYNAMIC,
        offsetof(CAudioPlayer, mMetadataExtraction)},
    {MPH_METADATATRAVERSAL, INTERFACE_DYNAMIC_GAME_MUSIC,
        offsetof(CAudioPlayer, mMetadataTraversal)},
    {MPH_PREFETCHSTATUS, INTERFACE_EXPLICIT, offsetof(CAudioPlayer, mPrefetchStatus)},
    {MPH_RATEPITCH, INTERFACE_DYNAMIC_GAME, offsetof(CAudioPlayer, mRatePitch)},
    {MPH_SEEK, INTERFACE_EXPLICIT, offsetof(CAudioPlayer, mSeek)},
    // The base Volume interface is explicit, but portions are only for Game and Music profiles
    {MPH_VOLUME, INTERFACE_EXPLICIT, offsetof(CAudioPlayer, mVolume)},
    {MPH_3DMACROSCOPIC, INTERFACE_OPTIONAL, offsetof(CAudioPlayer, m3DMacroscopic)},
    {MPH_BASSBOOST, INTERFACE_DYNAMIC, offsetof(CAudioPlayer, mBassBoost)},
    {MPH_DYNAMICSOURCE, INTERFACE_OPTIONAL, offsetof(CAudioPlayer, mDynamicSource)},
    {MPH_ENVIRONMENTALREVERB, INTERFACE_DYNAMIC,
        offsetof(CAudioPlayer, mEnvironmentalReverb)},
    {MPH_EQUALIZER, INTERFACE_DYNAMIC, offsetof(CAudioPlayer, mEqualizer)},
    {MPH_PITCH, INTERFACE_DYNAMIC_OPTIONAL, offsetof(CAudioPlayer, mPitch)},
    {MPH_PRESETREVERB, INTERFACE_DYNAMIC, offsetof(CAudioPlayer, mPresetReverb)},
    {MPH_PLAYBACKRATE, INTERFACE_DYNAMIC, offsetof(CAudioPlayer, mPlaybackRate)},
    {MPH_VIRTUALIZER, INTERFACE_DYNAMIC, offsetof(CAudioPlayer, mVirtualizer)},
    {MPH_VISUALIZATION, INTERFACE_OPTIONAL, offsetof(CAudioPlayer, mVisualization)},
#ifdef ANDROID
    {MPH_ANDROIDEFFECT, INTERFACE_EXPLICIT, offsetof(CAudioPlayer, mAndroidEffect)},
    {MPH_ANDROIDEFFECTSEND, INTERFACE_EXPLICIT, offsetof(CAudioPlayer, mAndroidEffectSend)},
    {MPH_ANDROIDCONFIGURATION, INTERFACE_EXPLICIT_PREREALIZE,
            offsetof(CAudioPlayer, mAndroidConfiguration)},
    {MPH_ANDROIDBUFFERQUEUESOURCE, INTERFACE_EXPLICIT, offsetof(CAudioPlayer, mAndroidBufferQueue)},
#endif
};

static const ClassTable CAudioPlayer_class = {
    AudioPlayer_interfaces,
    INTERFACES_AudioPlayer,
    MPH_to_AudioPlayer,
    "AudioPlayer",
    sizeof(CAudioPlayer),
    SL_OBJECTID_AUDIOPLAYER,
    0,      // OpenMAX AL object ID
    CAudioPlayer_Realize,
    CAudioPlayer_Resume,
    CAudioPlayer_Destroy,
    CAudioPlayer_PreDestroy
};


#if (USE_PROFILES & USE_PROFILES_OPTIONAL) || defined(ANDROID)

// AudioRecorder class

static const struct iid_vtable AudioRecorder_interfaces[INTERFACES_AudioRecorder] = {
    {MPH_OBJECT, INTERFACE_IMPLICIT_PREREALIZE, offsetof(CAudioRecorder, mObject)},
    {MPH_DYNAMICINTERFACEMANAGEMENT, INTERFACE_IMPLICIT,
        offsetof(CAudioRecorder, mDynamicInterfaceManagement)},
    {MPH_RECORD, INTERFACE_IMPLICIT, offsetof(CAudioRecorder, mRecord)},
    {MPH_AUDIOENCODER, INTERFACE_EXPLICIT, offsetof(CAudioRecorder, mAudioEncoder)},
    {MPH_BASSBOOST, INTERFACE_DYNAMIC_OPTIONAL, offsetof(CAudioRecorder, mBassBoost)},
    {MPH_DYNAMICSOURCE, INTERFACE_OPTIONAL, offsetof(CAudioRecorder, mDynamicSource)},
    {MPH_EQUALIZER, INTERFACE_DYNAMIC_OPTIONAL, offsetof(CAudioRecorder, mEqualizer)},
    {MPH_VISUALIZATION, INTERFACE_OPTIONAL, offsetof(CAudioRecorder, mVisualization)},
    {MPH_VOLUME, INTERFACE_OPTIONAL, offsetof(CAudioRecorder, mVolume)},
#ifdef ANDROID
    {MPH_ANDROIDSIMPLEBUFFERQUEUE, INTERFACE_EXPLICIT, offsetof(CAudioRecorder, mBufferQueue)},
    {MPH_ANDROIDCONFIGURATION, INTERFACE_EXPLICIT_PREREALIZE,
            offsetof(CAudioRecorder, mAndroidConfiguration)},
    /* marked explicit for Android implementation, not an SL ES explicit interface */
    {MPH_ANDROIDACOUSTICECHOCANCELLATION, INTERFACE_EXPLICIT, offsetof(CAudioRecorder,
                                                              mAcousticEchoCancellation)},
    /* marked explicit for Android implementation, not an SL ES explicit interface */
    {MPH_ANDROIDAUTOMATICGAINCONTROL, INTERFACE_EXPLICIT, offsetof(CAudioRecorder,
                                                                   mAutomaticGainControl)},
    /* marked explicit for Android implementation, not an SL ES explicit interface */
    {MPH_ANDROIDNOISESUPPRESSION, INTERFACE_EXPLICIT, offsetof(CAudioRecorder, mNoiseSuppression)},
#endif
};

static const ClassTable CAudioRecorder_class = {
    AudioRecorder_interfaces,
    INTERFACES_AudioRecorder,
    MPH_to_AudioRecorder,
    "AudioRecorder",
    sizeof(CAudioRecorder),
    SL_OBJECTID_AUDIORECORDER,
    0,      // OpenMAX AL object ID
    CAudioRecorder_Realize,
    CAudioRecorder_Resume,
    CAudioRecorder_Destroy,
    CAudioRecorder_PreDestroy
};

#endif


// Engine class

static const struct iid_vtable Engine_interfaces[INTERFACES_Engine] = {
    {MPH_OBJECT, INTERFACE_IMPLICIT_PREREALIZE, offsetof(CEngine, mObject)},
    {MPH_DYNAMICINTERFACEMANAGEMENT, INTERFACE_IMPLICIT,
        offsetof(CEngine, mDynamicInterfaceManagement)},
    {MPH_ENGINE, INTERFACE_IMPLICIT, offsetof(CEngine, mEngine)},
    {MPH_ENGINECAPABILITIES, INTERFACE_IMPLICIT_BASE, offsetof(CEngine, mEngineCapabilities)},
    {MPH_THREADSYNC, INTERFACE_IMPLICIT_BASE, offsetof(CEngine, mThreadSync)},
    {MPH_AUDIOIODEVICECAPABILITIES, INTERFACE_IMPLICIT_BASE,
        offsetof(CEngine, mAudioIODeviceCapabilities)},
    {MPH_AUDIODECODERCAPABILITIES, INTERFACE_EXPLICIT_BASE,
        offsetof(CEngine, mAudioDecoderCapabilities)},
    {MPH_AUDIOENCODERCAPABILITIES, INTERFACE_EXPLICIT_BASE,
        offsetof(CEngine, mAudioEncoderCapabilities)},
    {MPH_3DCOMMIT, INTERFACE_EXPLICIT_GAME, offsetof(CEngine, m3DCommit)},
    {MPH_DEVICEVOLUME, INTERFACE_OPTIONAL, offsetof(CEngine, mDeviceVolume)},
    {MPH_XAENGINE, INTERFACE_IMPLICIT, offsetof(CEngine, mXAEngine)},
#ifdef ANDROID
    {MPH_ANDROIDEFFECTCAPABILITIES, INTERFACE_EXPLICIT,
        offsetof(CEngine, mAndroidEffectCapabilities)},
#endif
    {MPH_XAVIDEODECODERCAPABILITIES, INTERFACE_EXPLICIT,
        offsetof(CEngine, mVideoDecoderCapabilities)},
};

static const ClassTable CEngine_class = {
    Engine_interfaces,
    INTERFACES_Engine,
    MPH_to_Engine,
    "Engine",
    sizeof(CEngine),
    SL_OBJECTID_ENGINE,
    XA_OBJECTID_ENGINE,
    CEngine_Realize,
    CEngine_Resume,
    CEngine_Destroy,
    CEngine_PreDestroy
};


#if USE_PROFILES & USE_PROFILES_OPTIONAL

// LEDDevice class

static const struct iid_vtable LEDDevice_interfaces[INTERFACES_LEDDevice] = {
    {MPH_OBJECT, INTERFACE_IMPLICIT_PREREALIZE, offsetof(CLEDDevice, mObject)},
    {MPH_DYNAMICINTERFACEMANAGEMENT, INTERFACE_IMPLICIT,
        offsetof(CLEDDevice, mDynamicInterfaceManagement)},
    {MPH_LED, INTERFACE_IMPLICIT, offsetof(CLEDDevice, mLEDArray)},
};

static const ClassTable CLEDDevice_class = {
    LEDDevice_interfaces,
    INTERFACES_LEDDevice,
    MPH_to_LEDDevice,
    "LEDDevice",
    sizeof(CLEDDevice),
    SL_OBJECTID_LEDDEVICE,
    XA_OBJECTID_LEDDEVICE,
    NULL,
    NULL,
    NULL,
    NULL
};

#endif


#if USE_PROFILES & USE_PROFILES_GAME

// Listener class

static const struct iid_vtable Listener_interfaces[INTERFACES_Listener] = {
    {MPH_OBJECT, INTERFACE_IMPLICIT_PREREALIZE, offsetof(CListener, mObject)},
    {MPH_DYNAMICINTERFACEMANAGEMENT, INTERFACE_IMPLICIT,
        offsetof(CListener, mDynamicInterfaceManagement)},
    {MPH_3DDOPPLER, INTERFACE_DYNAMIC_GAME, offsetof(CListener, m3DDoppler)},
    {MPH_3DLOCATION, INTERFACE_EXPLICIT_GAME, offsetof(CListener, m3DLocation)},
};

static const ClassTable CListener_class = {
    Listener_interfaces,
    INTERFACES_Listener,
    MPH_to_Listener,
    "Listener",
    sizeof(CListener),
    SL_OBJECTID_LISTENER,
    0,      // OpenMAX AL object ID
    NULL,
    NULL,
    NULL,
    NULL
};

#endif


#if USE_PROFILES & (USE_PROFILES_GAME | USE_PROFILES_MUSIC)

// MetadataExtractor class

static const struct iid_vtable MetadataExtractor_interfaces[INTERFACES_MetadataExtractor] = {
    {MPH_OBJECT, INTERFACE_IMPLICIT_PREREALIZE, offsetof(CMetadataExtractor, mObject)},
    {MPH_DYNAMICINTERFACEMANAGEMENT, INTERFACE_IMPLICIT,
        offsetof(CMetadataExtractor, mDynamicInterfaceManagement)},
    {MPH_DYNAMICSOURCE, INTERFACE_IMPLICIT, offsetof(CMetadataExtractor, mDynamicSource)},
    {MPH_METADATAEXTRACTION, INTERFACE_IMPLICIT, offsetof(CMetadataExtractor, mMetadataExtraction)},
    {MPH_METADATATRAVERSAL, INTERFACE_IMPLICIT, offsetof(CMetadataExtractor, mMetadataTraversal)},
};

static const ClassTable CMetadataExtractor_class = {
    MetadataExtractor_interfaces,
    INTERFACES_MetadataExtractor,
    MPH_to_MetadataExtractor,
    "MetadataExtractor",
    sizeof(CMetadataExtractor),
    SL_OBJECTID_METADATAEXTRACTOR,
    XA_OBJECTID_METADATAEXTRACTOR,
    NULL,
    NULL,
    NULL,
    NULL
};

#endif


#if USE_PROFILES & USE_PROFILES_GAME

// MidiPlayer class

static const struct iid_vtable MidiPlayer_interfaces[INTERFACES_MidiPlayer] = {
    {MPH_OBJECT, INTERFACE_IMPLICIT_PREREALIZE, offsetof(CMidiPlayer, mObject)},
    {MPH_DYNAMICINTERFACEMANAGEMENT, INTERFACE_IMPLICIT,
        offsetof(CMidiPlayer, mDynamicInterfaceManagement)},
    {MPH_PLAY, INTERFACE_IMPLICIT, offsetof(CMidiPlayer, mPlay)},
    {MPH_3DDOPPLER, INTERFACE_DYNAMIC_GAME, offsetof(C3DGroup, m3DDoppler)},
    {MPH_3DGROUPING, INTERFACE_EXPLICIT_GAME, offsetof(CMidiPlayer, m3DGrouping)},
    {MPH_3DLOCATION, INTERFACE_EXPLICIT_GAME, offsetof(CMidiPlayer, m3DLocation)},
    {MPH_3DSOURCE, INTERFACE_EXPLICIT_GAME, offsetof(CMidiPlayer, m3DSource)},
    {MPH_BUFFERQUEUE, INTERFACE_EXPLICIT_GAME, offsetof(CMidiPlayer, mBufferQueue)},
    {MPH_EFFECTSEND, INTERFACE_EXPLICIT_GAME, offsetof(CMidiPlayer, mEffectSend)},
    {MPH_MUTESOLO, INTERFACE_EXPLICIT_GAME, offsetof(CMidiPlayer, mMuteSolo)},
    {MPH_METADATAEXTRACTION, INTERFACE_DYNAMIC_GAME, offsetof(CMidiPlayer, mMetadataExtraction)},
    {MPH_METADATATRAVERSAL, INTERFACE_DYNAMIC_GAME, offsetof(CMidiPlayer, mMetadataTraversal)},
    {MPH_MIDIMESSAGE, INTERFACE_EXPLICIT_GAME_PHONE, offsetof(CMidiPlayer, mMIDIMessage)},
    {MPH_MIDITIME, INTERFACE_EXPLICIT_GAME_PHONE, offsetof(CMidiPlayer, mMIDITime)},
    {MPH_MIDITEMPO, INTERFACE_EXPLICIT_GAME_PHONE, offsetof(CMidiPlayer, mMIDITempo)},
    {MPH_MIDIMUTESOLO, INTERFACE_EXPLICIT_GAME, offsetof(CMidiPlayer, mMIDIMuteSolo)},
    {MPH_PREFETCHSTATUS, INTERFACE_EXPLICIT_GAME_PHONE, offsetof(CMidiPlayer, mPrefetchStatus)},
    {MPH_SEEK, INTERFACE_EXPLICIT_GAME_PHONE, offsetof(CMidiPlayer, mSeek)},
    {MPH_VOLUME, INTERFACE_EXPLICIT_GAME_PHONE, offsetof(CMidiPlayer, mVolume)},
    {MPH_3DMACROSCOPIC, INTERFACE_OPTIONAL, offsetof(CMidiPlayer, m3DMacroscopic)},
    {MPH_BASSBOOST, INTERFACE_DYNAMIC_OPTIONAL, offsetof(CMidiPlayer, mBassBoost)},
    {MPH_DYNAMICSOURCE, INTERFACE_OPTIONAL, offsetof(CMidiPlayer, mDynamicSource)},
    {MPH_ENVIRONMENTALREVERB, INTERFACE_DYNAMIC_OPTIONAL,
        offsetof(CMidiPlayer, mEnvironmentalReverb)},
    {MPH_EQUALIZER, INTERFACE_DYNAMIC_OPTIONAL, offsetof(CMidiPlayer, mEqualizer)},
    {MPH_PITCH, INTERFACE_DYNAMIC_OPTIONAL, offsetof(CMidiPlayer, mPitch)},
    {MPH_PRESETREVERB, INTERFACE_DYNAMIC_OPTIONAL, offsetof(CMidiPlayer, mPresetReverb)},
    {MPH_PLAYBACKRATE, INTERFACE_DYNAMIC_OPTIONAL, offsetof(CMidiPlayer, mPlaybackRate)},
    {MPH_VIRTUALIZER, INTERFACE_DYNAMIC_OPTIONAL, offsetof(CMidiPlayer, mVirtualizer)},
    {MPH_VISUALIZATION, INTERFACE_OPTIONAL, offsetof(CMidiPlayer, mVisualization)},
};

static const ClassTable CMidiPlayer_class = {
    MidiPlayer_interfaces,
    INTERFACES_MidiPlayer,
    MPH_to_MidiPlayer,
    "MidiPlayer",
    sizeof(CMidiPlayer),
    SL_OBJECTID_MIDIPLAYER,
    0,      // OpenMAX AL object ID
    NULL,
    NULL,
    NULL,
    NULL
};

#endif


// OutputMix class

static const struct iid_vtable OutputMix_interfaces[INTERFACES_OutputMix] = {
    {MPH_OBJECT, INTERFACE_IMPLICIT_PREREALIZE, offsetof(COutputMix, mObject)},
    {MPH_DYNAMICINTERFACEMANAGEMENT, INTERFACE_IMPLICIT,
        offsetof(COutputMix, mDynamicInterfaceManagement)},
    {MPH_OUTPUTMIX, INTERFACE_IMPLICIT, offsetof(COutputMix, mOutputMix)},
#ifdef USE_OUTPUTMIXEXT
    {MPH_OUTPUTMIXEXT, INTERFACE_UNAVAILABLE, offsetof(COutputMix, mOutputMixExt)},
#else
    {MPH_OUTPUTMIXEXT, INTERFACE_UNAVAILABLE, 0},
#endif
    {MPH_ENVIRONMENTALREVERB, INTERFACE_DYNAMIC,
        offsetof(COutputMix, mEnvironmentalReverb)},
    {MPH_EQUALIZER, INTERFACE_DYNAMIC, offsetof(COutputMix, mEqualizer)},
    {MPH_PRESETREVERB, INTERFACE_DYNAMIC, offsetof(COutputMix, mPresetReverb)},
    {MPH_VIRTUALIZER, INTERFACE_DYNAMIC, offsetof(COutputMix, mVirtualizer)},
    // The overall Volume interface is explicit optional,
    // but portions of Volume are mandated only in Game and Music profiles
    {MPH_VOLUME, INTERFACE_OPTIONAL, offsetof(COutputMix, mVolume)},
    {MPH_BASSBOOST, INTERFACE_DYNAMIC, offsetof(COutputMix, mBassBoost)},
    {MPH_VISUALIZATION, INTERFACE_OPTIONAL, offsetof(COutputMix, mVisualization)},
#ifdef ANDROID
    {MPH_ANDROIDEFFECT, INTERFACE_EXPLICIT, offsetof(COutputMix, mAndroidEffect)},
#endif
};

static const ClassTable COutputMix_class = {
    OutputMix_interfaces,
    INTERFACES_OutputMix,
    MPH_to_OutputMix,
    "OutputMix",
    sizeof(COutputMix),
    SL_OBJECTID_OUTPUTMIX,
    XA_OBJECTID_OUTPUTMIX,
    COutputMix_Realize,
    COutputMix_Resume,
    COutputMix_Destroy,
    COutputMix_PreDestroy
};


#if USE_PROFILES & USE_PROFILES_OPTIONAL

// Vibra class

static const struct iid_vtable VibraDevice_interfaces[INTERFACES_VibraDevice] = {
    {MPH_OBJECT, INTERFACE_IMPLICIT_PREREALIZE, offsetof(CVibraDevice, mObject)},
    {MPH_DYNAMICINTERFACEMANAGEMENT, INTERFACE_IMPLICIT,
        offsetof(CVibraDevice, mDynamicInterfaceManagement)},
    {MPH_VIBRA, INTERFACE_IMPLICIT, offsetof(CVibraDevice, mVibra)},
};

static const ClassTable CVibraDevice_class = {
    VibraDevice_interfaces,
    INTERFACES_VibraDevice,
    MPH_to_Vibra,
    "VibraDevice",
    sizeof(CVibraDevice),
    SL_OBJECTID_VIBRADEVICE,
    XA_OBJECTID_VIBRADEVICE,
    NULL,
    NULL,
    NULL,
    NULL
};

#endif


// Media player class

static const struct iid_vtable MediaPlayer_interfaces[INTERFACES_MediaPlayer] = {
    {MPH_XAOBJECT, INTERFACE_IMPLICIT_PREREALIZE, offsetof(CMediaPlayer, mObject)},
    {MPH_XADYNAMICINTERFACEMANAGEMENT, INTERFACE_IMPLICIT,
        offsetof(CMediaPlayer, mDynamicInterfaceManagement)},
    {MPH_XAPLAY, INTERFACE_IMPLICIT, offsetof(CMediaPlayer, mPlay)},
    {MPH_XASTREAMINFORMATION, INTERFACE_EXPLICIT, offsetof(CMediaPlayer, mStreamInfo)},
    {MPH_XAVOLUME, INTERFACE_IMPLICIT, offsetof(CMediaPlayer, mVolume)},
    {MPH_XASEEK, INTERFACE_EXPLICIT, offsetof(CMediaPlayer, mSeek)},
    {MPH_XAPREFETCHSTATUS, INTERFACE_EXPLICIT, offsetof(CMediaPlayer, mPrefetchStatus)},
#ifdef ANDROID
    {MPH_ANDROIDBUFFERQUEUESOURCE, INTERFACE_EXPLICIT, offsetof(CMediaPlayer, mAndroidBufferQueue)},
#endif
};

static const ClassTable CMediaPlayer_class = {
    MediaPlayer_interfaces,
    INTERFACES_MediaPlayer,
    MPH_to_MediaPlayer,
    "MediaPlayer",
    sizeof(CMediaPlayer),
    0,      // OpenSL ES object ID
    XA_OBJECTID_MEDIAPLAYER,
    CMediaPlayer_Realize,
    CMediaPlayer_Resume,
    CMediaPlayer_Destroy,
    CMediaPlayer_PreDestroy
};


static const ClassTable * const slClasses[] = {
    // Do not change order of these entries; they are in numerical order
    &CEngine_class,
#if USE_PROFILES & USE_PROFILES_OPTIONAL
    &CLEDDevice_class,
    &CVibraDevice_class,
#else
    NULL,
    NULL,
#endif
    &CAudioPlayer_class,
#if (USE_PROFILES & USE_PROFILES_OPTIONAL) || defined(ANDROID)
    &CAudioRecorder_class,
#else
    NULL,
#endif
#if USE_PROFILES & (USE_PROFILES_GAME | USE_PROFILES_PHONE)
    &CMidiPlayer_class,
#else
    NULL,
#endif
#if USE_PROFILES & USE_PROFILES_GAME
    &CListener_class,
    &C3DGroup_class,
#else
    NULL,
    NULL,
#endif
    &COutputMix_class,
#if USE_PROFILES & (USE_PROFILES_GAME | USE_PROFILES_MUSIC)
    &CMetadataExtractor_class
#else
    NULL
#endif
};


static const ClassTable * const xaClasses[] = {
    &CEngine_class,
#if USE_PROFILES & USE_PROFILES_OPTIONAL
    &CLEDDevice_class,
    &CVibraDevice_class,
#else
    NULL,
    NULL,
#endif
    &CMediaPlayer_class,
#if 1
    NULL,
    NULL,
#else
    &CMediaRecorder_class,
    &CRadioDevice_class,
#endif
    &COutputMix_class,
#if USE_PROFILES & (USE_PROFILES_GAME | USE_PROFILES_MUSIC)
    &CMetadataExtractor_class,
#else
    NULL,
#endif
#if 1
    NULL
#else
    &CCameraDevice_class
#endif
};


/* \brief Map SL_OBJECTID to class or NULL if object ID not supported */

LI_API const ClassTable *objectIDtoClass(SLuint32 objectID)
{
    // object ID is the engine and always present
    assert(NULL != slClasses[0]);
    SLuint32 slObjectID0 = slClasses[0]->mSLObjectID;
    if ((slObjectID0 <= objectID) && ((slObjectID0 + sizeof(slClasses)/sizeof(slClasses[0])) >
            objectID)) {
        return slClasses[objectID - slObjectID0];
    }
    assert(NULL != xaClasses[0]);
    SLuint32 xaObjectID0 = xaClasses[0]->mXAObjectID;
    if ((xaObjectID0 <= objectID) && ((xaObjectID0 + sizeof(xaClasses)/sizeof(xaClasses[0])) >
            objectID)) {
        return xaClasses[objectID - xaObjectID0];
    }
    return NULL;
}
