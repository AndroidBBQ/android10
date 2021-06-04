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

#include "Configuration.h"
#ifdef ANDROID
#include "android/CallbackProtector.h"
#include "android/android_Effect.h"
#include "android/android_GenericPlayer.h"
#include <media/TrackPlayerBase.h>
#include <audiomanager/IAudioManager.h>
#endif

// Class structures


/*typedef*/ struct CAudioPlayer_struct {
    IObject mObject;
#ifdef ANDROID
#define INTERFACES_AudioPlayer 30 // see MPH_to_AudioPlayer in MPH_to.c for list of interfaces
#else
#define INTERFACES_AudioPlayer 26 // see MPH_to_AudioPlayer in MPH_to.c for list of interfaces
#endif
    SLuint8 mInterfaceStates2[INTERFACES_AudioPlayer - INTERFACES_Default];
    IDynamicInterfaceManagement mDynamicInterfaceManagement;
    IPlay mPlay;
    I3DDoppler m3DDoppler;
    I3DGrouping m3DGrouping;
    I3DLocation m3DLocation;
    I3DSource m3DSource;
    IBufferQueue mBufferQueue;
    IEffectSend mEffectSend;
    IMetadataExtraction mMetadataExtraction;
    IMetadataTraversal mMetadataTraversal;
    IPrefetchStatus mPrefetchStatus;
    IRatePitch mRatePitch;
    ISeek mSeek;
    IVolume mVolume;
    IMuteSolo mMuteSolo;
#ifdef ANDROID
    IAndroidEffect mAndroidEffect;
    IAndroidEffectSend mAndroidEffectSend;
    IAndroidConfiguration mAndroidConfiguration;
    IAndroidBufferQueue mAndroidBufferQueue;
#endif
    // optional interfaces
    I3DMacroscopic m3DMacroscopic;
    IBassBoost mBassBoost;
    IDynamicSource mDynamicSource;
    IEnvironmentalReverb mEnvironmentalReverb;
    IEqualizer mEqualizer;
    IPitch mPitch;
    IPresetReverb mPresetReverb;
    IPlaybackRate mPlaybackRate;
    IVirtualizer mVirtualizer;
    IVisualization mVisualization;
    // fields below are per-instance private fields not associated with an interface
    DataLocatorFormat mDataSource;
    DataLocatorFormat mDataSink;
    // cached data for this instance
    // Formerly at IMuteSolo
    SLuint8 mMuteMask;      // Mask for which channels are muted: bit 0=left, 1=right
    SLuint8 mSoloMask;      // Mask for which channels are soloed: bit 0=left, 1=right
    SLuint8 mNumChannels;   // initially UNKNOWN_NUMCHANNELS, then const once it is known,
                            // range 1 <= x <= FCC_8
    // End of former IMuteSolo fields
    SLuint32 mSampleRateMilliHz;// initially UNKNOWN_SAMPLERATE, then const once it is known
    // Formerly at IEffectSend
    /**
     * Dry volume modified by effect send interfaces: SLEffectSendItf and SLAndroidEffectSendItf
     */
    SLmillibel mDirectLevel;
    // implementation-specific data for this instance
#ifdef USE_OUTPUTMIXEXT
    Track *mTrack;
    float mGains[STEREO_CHANNELS];  ///< Computed gain based on volume, mute, solo, stereo position
    SLboolean mDestroyRequested;    ///< Mixer to acknowledge application's call to Object::Destroy
#endif
#ifdef USE_SNDFILE
    struct SndFile mSndFile;
#endif // USE_SNDFILE
#ifdef ANDROID
    enum AndroidObjectType mAndroidObjType;
    /** identifies the initialization and preparation state */
    enum AndroidObjectState mAndroidObjState;
    /** identifies which group of effects ("session") this player belongs to */
    audio_session_t mSessionId;
    /** identifies the Android stream type playback will occur on */
    audio_stream_type_t mStreamType;
    /** player interface ID, uniquely identifies the player in the system */
    audio_unique_id_t mPIId;
    // FIXME consolidate the next several variables into one class to avoid placement new
    /** plays the PCM data for this player and reflects its state in AudioManager,
     *  always non-null once the AudioPlayer is created (i.e. before realize step) */
    android::sp<android::TrackPlayerBase> mTrackPlayer;
    android::sp<android::CallbackProtector> mCallbackProtector;
    android::sp<android::GenericPlayer> mAPlayer;
    /** aux effect the AudioTrack will be attached to if aux send enabled */
    android::sp<android::AudioEffect> mAuxEffect;
    // FIXME all levels below need to be encapsulated in a field of type AndroidAudioLevels
    /** send level to aux effect, there's a single aux bus, so there's a single level */
    SLmillibel mAuxSendLevel;
    /**
     * Attenuation factor derived from direct level
     */
    float mAmplFromDirectLevel;
    /** FIXME whether to call AudioTrack::start() at the next safe opportunity */
    bool mDeferredStart;
    SLuint32 mPerformanceMode;
#endif
} /*CAudioPlayer*/;


/*typedef*/ struct CAudioRecorder_struct {
    // mandated interfaces
    IObject mObject;
#ifdef ANDROID
#define INTERFACES_AudioRecorder 14 // see MPH_to_AudioRecorder in MPH_to.c for list of interfaces
#else
#define INTERFACES_AudioRecorder 9  // see MPH_to_AudioRecorder in MPH_to.c for list of interfaces
#endif
    SLuint8 mInterfaceStates2[INTERFACES_AudioRecorder - INTERFACES_Default];
    IDynamicInterfaceManagement mDynamicInterfaceManagement;
    IRecord mRecord;
    IAudioEncoder mAudioEncoder;
    // optional interfaces
    IBassBoost mBassBoost;
    IDynamicSource mDynamicSource;
    IEqualizer mEqualizer;
    IVisualization mVisualization;
    IVolume mVolume;
#ifdef ANDROID
    IBufferQueue mBufferQueue;
    IAndroidConfiguration mAndroidConfiguration;
    IAndroidAcousticEchoCancellation  mAcousticEchoCancellation;
    IAndroidAutomaticGainControl mAutomaticGainControl;
    IAndroidNoiseSuppression mNoiseSuppression;
#endif
    // remaining are per-instance private fields not associated with an interface
    DataLocatorFormat mDataSource;
    DataLocatorFormat mDataSink;
    // cached data for this instance
    SLuint8 mNumChannels;   // initially UNKNOWN_NUMCHANNELS, then const once it is known,
                            // range 1 <= x <= FCC_8
    SLuint32 mSampleRateMilliHz;// initially UNKNOWN_SAMPLERATE, then const once it is known
    // implementation-specific data for this instance
#ifdef ANDROID
    // FIXME consolidate the next several variables into ARecorder class to avoid placement new
    enum AndroidObjectType mAndroidObjType;
    android::sp<android::AudioRecord> mAudioRecord;
    android::sp<android::CallbackProtector> mCallbackProtector;
    audio_source_t mRecordSource;
    SLuint32 mPerformanceMode;
#endif
} /*CAudioRecorder*/;


/*typedef*/ struct CEngine_struct {
    // mandated implicit interfaces
    IObject mObject;
#ifdef ANDROID
#define INTERFACES_Engine 13 // see MPH_to_Engine in MPH_to.c for list of interfaces
#else
#define INTERFACES_Engine 12 // see MPH_to_Engine in MPH_to.c for list of interfaces
#endif
    SLuint8 mInterfaceStates2[INTERFACES_Engine - INTERFACES_Default];
    IDynamicInterfaceManagement mDynamicInterfaceManagement;
    IEngine mEngine;
    IEngineCapabilities mEngineCapabilities;
    IThreadSync mThreadSync;
    // mandated explicit interfaces
    IAudioIODeviceCapabilities mAudioIODeviceCapabilities;
    IAudioDecoderCapabilities mAudioDecoderCapabilities;
    IAudioEncoderCapabilities mAudioEncoderCapabilities;
    I3DCommit m3DCommit;
    // optional interfaces
    IDeviceVolume mDeviceVolume;
    // OpenMAX AL mandated implicit interfaces
    IXAEngine mXAEngine;
#ifdef ANDROID
    IAndroidEffectCapabilities mAndroidEffectCapabilities;
#endif
    // OpenMAX AL explicit interfaces
    IVideoDecoderCapabilities mVideoDecoderCapabilities;
    // remaining are per-instance private fields not associated with an interface
    ThreadPool mThreadPool; // for asynchronous operations
    pthread_t mSyncThread;
#if defined(ANDROID)
    // FIXME number of presets will only be saved in IEqualizer, preset names will not be stored
    SLuint32 mEqNumPresets;
    char** mEqPresetNames;
    android::sp<android::IAudioManager> mAudioManager;
#endif
} /*CEngine*/;

typedef struct {
    // mandated interfaces
    IObject mObject;
#define INTERFACES_LEDDevice 3 // see MPH_to_LEDDevice in MPH_to.c for list of interfaces
    SLuint8 mInterfaceStates2[INTERFACES_LEDDevice - INTERFACES_Default];
    IDynamicInterfaceManagement mDynamicInterfaceManagement;
    ILEDArray mLEDArray;
    // remaining are per-instance private fields not associated with an interface
    SLuint32 mDeviceID;
} CLEDDevice;

typedef struct {
    // mandated interfaces
    IObject mObject;
#define INTERFACES_Listener 4 // see MPH_to_Listener in MPH_to.c for list of interfaces
    SLuint8 mInterfaceStates2[INTERFACES_Listener - INTERFACES_Default];
    IDynamicInterfaceManagement mDynamicInterfaceManagement;
    I3DDoppler m3DDoppler;
    I3DLocation m3DLocation;
    // remaining are per-instance private fields not associated with an interface
} CListener;

typedef struct {
    // mandated interfaces
    IObject mObject;
#define INTERFACES_MetadataExtractor 5 // see MPH_to_MetadataExtractor in MPH_to.c for list of
                                       // interfaces
    SLuint8 mInterfaceStates2[INTERFACES_MetadataExtractor - INTERFACES_Default];
    IDynamicInterfaceManagement mDynamicInterfaceManagement;
    IDynamicSource mDynamicSource;
    IMetadataExtraction mMetadataExtraction;
    IMetadataTraversal mMetadataTraversal;
    // remaining are per-instance private fields not associated with an interface
} CMetadataExtractor;

typedef struct {
    // mandated interfaces
    IObject mObject;

#define INTERFACES_MidiPlayer 29 // see MPH_to_MidiPlayer in MPH_to.c for list of interfaces
    SLuint8 mInterfaceStates2[INTERFACES_MidiPlayer - INTERFACES_Default];
    IDynamicInterfaceManagement mDynamicInterfaceManagement;
    IPlay mPlay;
    I3DDoppler m3DDoppler;
    I3DGrouping m3DGrouping;
    I3DLocation m3DLocation;
    I3DSource m3DSource;
    IBufferQueue mBufferQueue;
    IEffectSend mEffectSend;
    IMetadataExtraction mMetadataExtraction;
    IMetadataTraversal mMetadataTraversal;
    IMIDIMessage mMIDIMessage;
    IMIDITime mMIDITime;
    IMIDITempo mMIDITempo;
    IMIDIMuteSolo mMIDIMuteSolo;
    IPrefetchStatus mPrefetchStatus;
    ISeek mSeek;
    IVolume mVolume;
    IMuteSolo mMuteSolo;
    // optional interfaces
    I3DMacroscopic m3DMacroscopic;
    IBassBoost mBassBoost;
    IDynamicSource mDynamicSource;
    IEnvironmentalReverb mEnvironmentalReverb;
    IEqualizer mEqualizer;
    IPitch mPitch;
    IPresetReverb mPresetReverb;
    IPlaybackRate mPlaybackRate;
    IVirtualizer mVirtualizer;
    IVisualization mVisualization;
    // remaining are per-instance private fields not associated with an interface
} CMidiPlayer;

/*typedef*/ struct COutputMix_struct {
    // mandated interfaces
    IObject mObject;
#ifdef ANDROID
#define INTERFACES_OutputMix 12 // see MPH_to_OutputMix in MPH_to.c for list of interfaces
#else
#define INTERFACES_OutputMix 11 // see MPH_to_OutputMix in MPH_to.c for list of interfaces
#endif
    SLuint8 mInterfaceStates2[INTERFACES_OutputMix - INTERFACES_Default];
    IDynamicInterfaceManagement mDynamicInterfaceManagement;
    IOutputMix mOutputMix;
#ifdef USE_OUTPUTMIXEXT
    IOutputMixExt mOutputMixExt;
#endif
    IEnvironmentalReverb mEnvironmentalReverb;
    IEqualizer mEqualizer;
    IPresetReverb mPresetReverb;
    IVirtualizer mVirtualizer;
    IVolume mVolume;
    // optional interfaces
    IBassBoost mBassBoost;
    IVisualization mVisualization;
#ifdef ANDROID
    IAndroidEffect mAndroidEffect;
#endif
    // remaining are per-instance private fields not associated with an interface
} /*COutputMix*/;

typedef struct {
    // mandated interfaces
    IObject mObject;
#define INTERFACES_VibraDevice 3 // see MPH_to_VibraDevice in MPH_to.c for list of interfaces
    SLuint8 mInterfaceStates2[INTERFACES_VibraDevice - INTERFACES_Default];
    IDynamicInterfaceManagement mDynamicInterfaceManagement;
    IVibra mVibra;
    // remaining are per-instance private fields not associated with an interface
    SLuint32 mDeviceID;
} CVibraDevice;


typedef struct CMediaPlayer_struct {
    IObject mObject;
#ifdef ANDROID
#define INTERFACES_MediaPlayer 8
#else
#define INTERFACES_MediaPlayer 7
#endif
    XAuint8 mInterfaceStates2[INTERFACES_MediaPlayer - INTERFACES_Default];
    IDynamicInterfaceManagement mDynamicInterfaceManagement;
    IDynamicSource mDynamicSource;
    IPlay mPlay;
    IStreamInformation mStreamInfo;
    IVolume mVolume;
    ISeek mSeek;
    IPrefetchStatus mPrefetchStatus;
#ifdef ANDROID
    IAndroidBufferQueue mAndroidBufferQueue;
#endif
    // fields below are per-instance private fields not associated with an interface
    DataLocatorFormat mDataSource;
    DataLocatorFormat mBankSource;
    DataLocatorFormat mAudioSink;
    DataLocatorFormat mImageVideoSink;
    DataLocatorFormat mVibraSink;
    DataLocatorFormat mLEDArraySink;
    SLuint8 mNumChannels;   // initially UNKNOWN_NUMCHANNELS, then const once it is known,
                            // range 1 <= x <= 8 FIXME FCC_8
#ifdef ANDROID
    android::sp<android::GenericPlayer> mAVPlayer;
    android::sp<android::CallbackProtector> mCallbackProtector;
    enum AndroidObjectType mAndroidObjType;
    /** identifies the initialization and preparation state */
    enum AndroidObjectState mAndroidObjState;
    /** identifies which group of effects ("session") this player belongs to */
    audio_session_t mSessionId;
    /** identifies the Android stream type playback will occur on */
    audio_stream_type_t mStreamType;
#endif
} CMediaPlayer;
