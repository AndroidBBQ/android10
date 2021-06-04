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

/** \file sles_allinclusive.h Everything including the kitchen sink */

#include "Configuration.h"
#include <SLES/OpenSLES.h>
#include <OMXAL/OpenMAXAL.h>
#ifdef ANDROID
#include <SLES/OpenSLES_Android.h>
#include <OMXAL/OpenMAXAL_Android.h>
#endif
#include <stddef.h> // offsetof
#include <stdlib.h> // malloc
#include <string.h> // memcmp
#include <strings.h>
#include <stdio.h>  // debugging
#include <assert.h> // debugging
#include <pthread.h>
#include <unistd.h> // usleep
#include <errno.h>

#ifndef __cplusplus
typedef int bool;
#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif
#endif

// The OpenSLES.h definitions of SL_PROFILES_... have casts, so are unusable by preprocessor
#define USE_PROFILES_PHONE    0x1   // == SL_PROFILES_PHONE
#define USE_PROFILES_MUSIC    0x2   // == SL_PROFILES_MUSIC
#define USE_PROFILES_GAME     0x4   // == SL_PROFILES_GAME
// Pseudo profiles, used to decide whether to include code for incomplete or untested features
// Features that are not in union of all profiles: audio recorder, LED, Vibra
#define USE_PROFILES_OPTIONAL 0x8
// Features that are in the intersection of all profiles:
// object priorities, preemption, loss of control, device configuration
#define USE_PROFILES_BASE     0x10

#include "MPH.h"
#include "MPH_to.h"
#include "devices.h"
#include "ut/OpenSLESUT.h"
#include "ThreadPool.h"

typedef struct CEngine_struct CEngine;
typedef struct CAudioPlayer_struct CAudioPlayer;
typedef struct CAudioRecorder_struct CAudioRecorder;
typedef struct C3DGroup_struct C3DGroup;
typedef struct COutputMix_struct COutputMix;

#ifdef USE_SNDFILE
#include <sndfile.h>
#include "desktop/SLSndFile.h"
#endif // USE_SNDFILE

#ifdef USE_SDL
#include <SDL/SDL_audio.h>
#endif // USE_SDL

#define STEREO_CHANNELS 2

/**
 * Constants to define unknown property values
 */
#define UNKNOWN_NUMCHANNELS 0
#define UNKNOWN_SAMPLERATE  0

#ifdef ANDROID
#include <utils/Log.h>
#include <utils/KeyedVector.h>
#include "media/AudioSystem.h"
#include "media/mediarecorder.h"
#include "media/AudioRecord.h"
#include "media/AudioTrack.h"
#include "media/mediaplayer.h"
#include <utils/String8.h>
#define ANDROID_SL_MILLIBEL_MAX 0
#include "android/android_sles_conversions.h"
#include "android/android_defs.h"
#endif

#ifdef USE_OUTPUTMIXEXT
#include "desktop/OutputMixExt.h"
#endif

#include "sllog.h"

typedef enum {
    predestroy_error,   // Application should not be calling destroy now
    predestroy_ok,      // OK to destroy object now
    predestroy_again    // Application did nothing wrong, but should destroy again to be effective
} predestroy_t;

// Hook functions

typedef void (*VoidHook)(void *self);
//typedef SLresult (*ResultHook)(void *self);
typedef SLresult (*AsyncHook)(void *self, SLboolean async);
typedef bool (*BoolHook)(void *self);
typedef predestroy_t (*PreDestroyHook)(void *self);

// Describes how an interface is related to a given class, used in iid_vtable::mInterface

#define INTERFACE_IMPLICIT            0 // no need for application to request prior to GetInterface
#define INTERFACE_EXPLICIT            1 // must be requested explicitly during object creation
#define INTERFACE_DYNAMIC             2 // can be requested after object creation
#define INTERFACE_UNAVAILABLE         3 // this interface is not available on objects of this class
#define INTERFACE_IMPLICIT_PREREALIZE 4 // implicit, and can call GetInterface before Realize
#define INTERFACE_EXPLICIT_PREREALIZE 5 // explicit, and can call GetInterface before Realize
// 6 and 7 are reserved for the meaningless DYNAMIC_PREREALIZE and UNAVAILABLE_PREREALIZE
// note that INTERFACE_OPTIONAL is always re-mapped to one of the above
#define INTERFACE_PREREALIZE          4 // bit-mask to test for calling GetInterface before Realize

// Profile-specific interfaces

#if USE_PROFILES & USE_PROFILES_BASE
#define INTERFACE_IMPLICIT_BASE       INTERFACE_IMPLICIT
#define INTERFACE_EXPLICIT_BASE       INTERFACE_EXPLICIT
#else
#define INTERFACE_IMPLICIT_BASE       INTERFACE_UNAVAILABLE
#define INTERFACE_EXPLICIT_BASE       INTERFACE_UNAVAILABLE
#endif

#if USE_PROFILES & USE_PROFILES_GAME
#define INTERFACE_DYNAMIC_GAME        INTERFACE_DYNAMIC
#define INTERFACE_EXPLICIT_GAME       INTERFACE_EXPLICIT
#else
#define INTERFACE_DYNAMIC_GAME        INTERFACE_OPTIONAL
#define INTERFACE_EXPLICIT_GAME       INTERFACE_OPTIONAL
#endif

#if USE_PROFILES & USE_PROFILES_MUSIC
#define INTERFACE_DYNAMIC_MUSIC       INTERFACE_DYNAMIC
#else
#define INTERFACE_DYNAMIC_MUSIC       INTERFACE_OPTIONAL
#endif

#if USE_PROFILES & (USE_PROFILES_GAME | USE_PROFILES_MUSIC)
#define INTERFACE_DYNAMIC_GAME_MUSIC  INTERFACE_DYNAMIC
#define INTERFACE_EXPLICIT_GAME_MUSIC INTERFACE_EXPLICIT
#else
#define INTERFACE_DYNAMIC_GAME_MUSIC  INTERFACE_OPTIONAL
#define INTERFACE_EXPLICIT_GAME_MUSIC INTERFACE_OPTIONAL
#endif

#if USE_PROFILES & (USE_PROFILES_GAME | USE_PROFILES_PHONE)
#define INTERFACE_EXPLICIT_GAME_PHONE INTERFACE_EXPLICIT
#else
#define INTERFACE_EXPLICIT_GAME_PHONE INTERFACE_OPTIONAL
#endif

#if USE_PROFILES & USE_PROFILES_OPTIONAL
#define INTERFACE_OPTIONAL            INTERFACE_EXPLICIT
#define INTERFACE_DYNAMIC_OPTIONAL    INTERFACE_DYNAMIC
#else
#define INTERFACE_OPTIONAL            INTERFACE_UNAVAILABLE
#define INTERFACE_DYNAMIC_OPTIONAL    INTERFACE_UNAVAILABLE
#endif

// Describes how an interface is related to a given object

#define INTERFACE_UNINITIALIZED 0  ///< not available
#define INTERFACE_INITIALIZED   1  ///< not requested at object creation time
#define INTERFACE_EXPOSED       2  ///< requested at object creation time
#define INTERFACE_ADDING_1      3  ///< part 1 of asynchronous AddInterface, pending
#define INTERFACE_ADDING_2      4  ///< synchronous AddInterface, or part 2 of asynchronous
#define INTERFACE_ADDED         5  ///< AddInterface has completed
#define INTERFACE_REMOVING      6  ///< unlocked phase of (synchronous) RemoveInterface
#define INTERFACE_SUSPENDING    7  ///< suspend in progress
#define INTERFACE_SUSPENDED     8  ///< suspend has completed
#define INTERFACE_RESUMING_1    9  ///< part 1 of asynchronous ResumeInterface, pending
#define INTERFACE_RESUMING_2   10  ///< synchronous ResumeInterface, or part 2 of asynchronous
#define INTERFACE_ADDING_1A    11  ///< part 1 of asynchronous AddInterface, aborted
#define INTERFACE_RESUMING_1A  12  ///< part 1 of asynchronous ResumeInterface, aborted


// Maps an interface ID to its offset within the class that exposes it

struct iid_vtable {
    unsigned char mMPH;         // primary MPH for this interface, does not include any aliases
    unsigned char mInterface;   // relationship of interface to this class
    /*size_t*/ unsigned short mOffset;
};

// Per-class const data shared by all instances of the same class

typedef struct {
    const struct iid_vtable *mInterfaces;   // maps interface index to info about that interface
    SLuint32 mInterfaceCount;  // number of possible interfaces
    const signed char *mMPH_to_index;
    const char * const mName;
    size_t mSize;
    // OpenSL ES and OpenMAX AL object IDs come from different ranges, and some objects such as
    // Engine, Output Mix, LED, and Vibra belong to both APIs, so we keep both object IDs
    SLuint16 mSLObjectID;   // OpenSL ES object ID
    XAuint16 mXAObjectID;   // OpenMAX AL object ID
    // hooks
    AsyncHook mRealize;     // called with mutex locked; can temporarily unlock mutex (for async)
    AsyncHook mResume;      // called with mutex locked; can temporarily unlock mutex (for async)
    VoidHook mDestroy;      // called with mutex locked and must keep mutex locked throughout
    PreDestroyHook mPreDestroy; // called with mutex locked; can temporarily unlock mutex (for wait)
} ClassTable;

// BufferHeader describes each element of a BufferQueue, other than the data
typedef struct {
    const void *mBuffer;
    SLuint32 mSize;
} BufferHeader;

#ifdef ANDROID
// Holds information about all commands that can be passed alongside an MPEG-2 TS buffer
// Is used with buffers of type kAndroidBufferTypeMpeg2Ts
typedef struct {
    SLuint32 mTsCmdCode;
    SLAuint64 mPts;
} Mpeg2TsCommands;

// Holds information about all commands that can be passed alongside an AAC ADTS buffer
// Is used with buffers of type kAndroidBufferTypeAacadts
typedef struct {
    SLuint32 mAdtsCmdCode;
} AdtsCommands;

// Union of the different structures to hold items stored in an AdvancedBufferHeader
//   when an item comes from an AndroidBufferQueue as the data source, it's a command
//   when an item is output to an AndroidBufferQueue as the data sink, it's a message (or metadata)
typedef union {
    Mpeg2TsCommands mTsCmdData;
    AdtsCommands    mAdtsCmdData;
} AdvancedBufferItems;

// AdvancedBufferHeader describes each element of an AndroidBufferQueue, other than the data
//  and associated messages
typedef struct {
    const void *mDataBuffer;
    SLuint32 mDataSize;
    SLuint32 mDataSizeConsumed;
    AdvancedBufferItems mItems;
    const void *mBufferContext;
    // mBufferState will be used for the other ABQ events we'll support in the future
    // SLuint32 mBufferState;
} AdvancedBufferHeader;
#endif

#ifdef USE_SNDFILE

#define SndFile_BUFSIZE 512     // in 16-bit samples
#define SndFile_NUMBUFS 2

struct SndFile {
    // save URI also?
    SLchar *mPathname;
    SNDFILE *mSNDFILE;
    SF_INFO mSfInfo;
    pthread_mutex_t mMutex; // protects mSNDFILE only
    SLboolean mEOF;         // sf_read returned zero sample frames
    SLuint32 mWhich;        // which buffer to use next
    short mBuffer[SndFile_BUFSIZE * SndFile_NUMBUFS];
};

#endif // USE_SNDFILE

#include "data.h"
#include "itfstruct.h"
#include "classes.h"

struct MPH_init {
    VoidHook mInit;     // called first to initialize the interface, right after object is allocated
    // Each interface is initialized regardless whether it is exposed to application.
    VoidHook mResume;   // called to resume interface after suspension, not currently used
    VoidHook mDeinit;   // called last when object is about to be destroyed
    BoolHook mExpose;   // called after initialization, only if interface is exposed to application
    VoidHook mRemove;   // called by DynamicInterfaceManager::RemoveInterface, and prior to mDeinit
    // will need a suspend hook when suspend is implemented
};

extern /*static*/ int IID_to_MPH(const SLInterfaceID iid);
extern /*static*/ const struct MPH_init MPH_init_table[MPH_MAX];
extern SLresult checkInterfaces(const ClassTable *clazz,
    SLuint32 numInterfaces, const SLInterfaceID *pInterfaceIds,
    const SLboolean *pInterfaceRequired, unsigned *pExposedMask, unsigned *pRequiredMask);
extern IObject *construct(const ClassTable *clazz,
    unsigned exposedMask, SLEngineItf engine);
extern const ClassTable *objectIDtoClass(SLuint32 objectID);
extern const struct SLInterfaceID_ SL_IID_array[MPH_MAX];
extern SLuint32 IObjectToObjectID(IObject *object);
extern void IObject_Publish(IObject *thiz);
extern void IObject_Destroy(SLObjectItf self);

// Map an interface to it's "object ID" (which is really a class ID).
// Note: this operation is undefined on IObject, as it lacks an mThis.
// If you have an IObject, then use IObjectToObjectID directly.

#define InterfaceToObjectID(thiz) IObjectToObjectID((thiz)->mThis)

// Map an interface to it's corresponding IObject.
// Note: this operation is undefined on IObject, as it lacks an mThis.
// If you have an IObject, then you're done -- you already have what you need.

#define InterfaceToIObject(thiz) ((thiz)->mThis)

#define InterfaceToCAudioPlayer(thiz) (((CAudioPlayer*)InterfaceToIObject(thiz)))

#define InterfaceToCAudioRecorder(thiz) (((CAudioRecorder*)InterfaceToIObject(thiz)))

#define InterfaceToCAudioRecorder(thiz) (((CAudioRecorder*)InterfaceToIObject(thiz)))

#define InterfaceToCMediaPlayer(thiz) (((CMediaPlayer*)InterfaceToIObject(thiz)))

#ifdef ANDROID
#include "android/MediaPlayer_to_android.h"
#include "android/OutputMix_to_android.h"
#include "android/AudioPlayer_to_android.h"
#include "android/AudioRecorder_to_android.h"
#endif

extern predestroy_t C3DGroup_PreDestroy(void *self);

extern SLresult CAudioPlayer_Realize(void *self, SLboolean async);
extern SLresult CAudioPlayer_Resume(void *self, SLboolean async);
extern void CAudioPlayer_Destroy(void *self);
extern predestroy_t CAudioPlayer_PreDestroy(void *self);

extern SLresult CAudioRecorder_Realize(void *self, SLboolean async);
extern SLresult CAudioRecorder_Resume(void *self, SLboolean async);
extern void CAudioRecorder_Destroy(void *self);
extern predestroy_t CAudioRecorder_PreDestroy(void *self);

extern SLresult CEngine_Realize(void *self, SLboolean async);
extern SLresult CEngine_Resume(void *self, SLboolean async);
extern void CEngine_Destroy(void *self);
extern predestroy_t CEngine_PreDestroy(void *self);
extern void CEngine_Destroyed(CEngine *self);

extern SLresult COutputMix_Realize(void *self, SLboolean async);
extern SLresult COutputMix_Resume(void *self, SLboolean async);
extern void COutputMix_Destroy(void *self);
extern predestroy_t COutputMix_PreDestroy(void *self);

extern SLresult CMediaPlayer_Realize(void *self, SLboolean async);
extern SLresult CMediaPlayer_Resume(void *self, SLboolean async);
extern void CMediaPlayer_Destroy(void *self);
extern predestroy_t CMediaPlayer_PreDestroy(void *self);

#ifdef USE_SDL
extern void SDL_open(IEngine *thisEngine);
extern void SDL_close(void);
#endif

#define SL_OBJECT_STATE_REALIZING_1  ((SLuint32) 0x4) // async realize on work queue
#define SL_OBJECT_STATE_REALIZING_2  ((SLuint32) 0x5) // sync realize, or async realize hook
#define SL_OBJECT_STATE_RESUMING_1   ((SLuint32) 0x6) // async resume on work queue
#define SL_OBJECT_STATE_RESUMING_2   ((SLuint32) 0x7) // sync resume, or async resume hook
#define SL_OBJECT_STATE_SUSPENDING   ((SLuint32) 0x8) // suspend in progress
#define SL_OBJECT_STATE_REALIZING_1A ((SLuint32) 0x9) // abort while async realize on work queue
#define SL_OBJECT_STATE_RESUMING_1A  ((SLuint32) 0xA) // abort while async resume on work queue
#define SL_OBJECT_STATE_DESTROYING   ((SLuint32) 0xB) // destroy object when no strong references

#ifdef USE_OUTPUTMIXEXT
#define SL_PLAYSTATE_STOPPING ((SLuint32) 0x4) // Play::Stop while PLAYING
// If we needed it, could have PLAYING mean mixer is currently reading from front buffer,
// while PLAYABLE would mean application requested PLAYING, but buffer queue is empty
#endif

#ifndef ANDROID
extern void *sync_start(void *arg);
#endif
extern SLresult err_to_result(int err);

#ifdef __GNUC__
#define ctz __builtin_ctz
#else
extern unsigned ctz(unsigned);
#endif
extern const char * const interface_names[MPH_MAX];
#include "platform.h"
#include "attr.h"
#include "handlers.h"
#include "trace.h"

#ifdef USE_SNDFILE
extern void audioPlayerTransportUpdate(CAudioPlayer *audioPlayer);
#endif

#ifndef FALLTHROUGH_INTENDED
#ifdef __clang__
#define FALLTHROUGH_INTENDED [[clang::fallthrough]]
#else
#define FALLTHROUGH_INTENDED
#endif // __clang__
#endif // FALLTHROUGH_INTENDED

extern SLresult IBufferQueue_Enqueue(SLBufferQueueItf self, const void *pBuffer, SLuint32 size);
extern SLresult IBufferQueue_Clear(SLBufferQueueItf self);
extern SLresult IBufferQueue_RegisterCallback(SLBufferQueueItf self,
    slBufferQueueCallback callback, void *pContext);

extern bool IsInterfaceInitialized(IObject *thiz, unsigned MPH);
extern SLresult AcquireStrongRef(IObject *object, SLuint32 expectedObjectID);
extern void ReleaseStrongRef(IObject *object);
extern void ReleaseStrongRefAndUnlockExclusive(IObject *object);

extern COutputMix *CAudioPlayer_GetOutputMix(CAudioPlayer *audioPlayer);
extern SLresult IEngineCapabilities_QueryLEDCapabilities(SLEngineCapabilitiesItf self,
    SLuint32 *pIndex, SLuint32 *pLEDDeviceID, SLLEDDescriptor *pDescriptor);
extern SLresult IEngineCapabilities_QueryVibraCapabilities(SLEngineCapabilitiesItf self,
    SLuint32 *pIndex, SLuint32 *pVibraDeviceID, SLVibraDescriptor *pDescriptor);

extern CEngine *theOneTrueEngine;
extern pthread_mutex_t theOneTrueMutex;
extern unsigned theOneTrueRefCount;

extern LI_API SLresult liCreateEngine(SLObjectItf *pEngine, SLuint32 numOptions,
    const SLEngineOption *pEngineOptions, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired,
    const ClassTable *pCEngine_class);
extern LI_API SLresult liQueryNumSupportedInterfaces(SLuint32 *pNumSupportedInterfaces,
        const ClassTable *clazz);
extern LI_API SLresult liQuerySupportedInterfaces(SLuint32 index, SLInterfaceID *pInterfaceId,
        const ClassTable *clazz);

// The EnqueueAsyncCallback macros provide a safe way to asynchronously call an application-level
// callback handler that is permitted to do almost anything, including block.  This is intended
// primarily for "notification" callbacks such as play head progress.  Do not use for callbacks
// which must be synchronous, such as buffer queue completions.  The enqueue may fail if
// the callback queue is full.  This almost always indicates an application error such as blocking
// for an excessive time within a callback handler or requesting too frequent callbacks.  The
// recommended recovery is to either retry later, or log a warning or error as appropriate.
// If the callback absolutely must be called, then you should be calling it directly instead.
// Example usage:
//  CAudioPlayer *ap;
//  SLresult result = EnqueueAsyncCallback_ppi(ap, playCallback, &ap->mPlay.mItf, playContext,
//       SL_PLAYEVENT_HEADATEND);
//  if (SL_RESULT_SUCCESS != result) {
//    ALOGW("Callback %p(%p, %p, SL_PLAYEVENT_HEADATEND) dropped", playCallback, &ap->mPlay.mItf,
//        playContext);
//  }
// which replaces:
//  (*playCallback)(&ap->mPlay.mItf, playContext, SL_PLAYEVENT_HEADATEND);
#define EnqueueAsyncCallback_ppi(object, handler, p1, p2, i1) \
        ThreadPool_add_ppi(&(object)->mObject.mEngine->mThreadPool, \
            (ClosureHandler_ppi) (handler), (p1), (p2), (i1))
#define EnqueueAsyncCallback_ppii(object, handler, p1, p2, i1, i2) \
        ThreadPool_add_ppii(&(object)->mObject.mEngine->mThreadPool, \
            (ClosureHandler_ppii) (handler), (p1), (p2), (i1), (i2))
#define EnqueueAsyncCallback_piipp(object, handler, p1, i1, i2, p2, p3) \
        ThreadPool_add_piipp(&(object)->mObject.mEngine->mThreadPool, \
            (ClosureHandler_piipp) (handler), (p1), (i1), (i2), (p2), (p3))

#define SL_PREFETCHEVENT_NONE ((SLuint32) 0)    // placeholder for non-existent SL_PREFETCHEVENT_*
