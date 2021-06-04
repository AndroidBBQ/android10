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

/* OpenSL ES private and global functions not associated with an interface or class */

#include "sles_allinclusive.h"


/** \brief Return true if the specified interface exists and has been initialized for this object.
 *  Returns false if the class does not support this kind of interface, or the class supports the
 *  interface but this particular object has not had the interface exposed at object creation time
 *  or by DynamicInterface::AddInterface. Note that the return value is not affected by whether
 *  the application has requested access to the interface with Object::GetInterface. Assumes on
 *  entry that the object is locked for either shared or exclusive access.
 */

bool IsInterfaceInitialized(IObject *thiz, unsigned MPH)
{
    assert(NULL != thiz);
    assert( /* (MPH_MIN <= MPH) && */ (MPH < (unsigned) MPH_MAX));
    const ClassTable *clazz = thiz->mClass;
    assert(NULL != clazz);
    int index;
    if (0 > (index = clazz->mMPH_to_index[MPH])) {
        return false;
    }
    assert(MAX_INDEX >= clazz->mInterfaceCount);
    assert(clazz->mInterfaceCount > (unsigned) index);
    switch (thiz->mInterfaceStates[index]) {
    case INTERFACE_EXPOSED:
    case INTERFACE_ADDED:
        return true;
    default:
        return false;
    }
}


/** \brief Map an IObject to it's "object ID" (which is really a class ID) */

SLuint32 IObjectToObjectID(IObject *thiz)
{
    assert(NULL != thiz);
    // Note this returns the OpenSL ES object ID in preference to the OpenMAX AL if both available
    const ClassTable *clazz = thiz->mClass;
    assert(NULL != clazz);
    SLuint32 id = clazz->mSLObjectID;
    if (!id) {
        id = clazz->mXAObjectID;
    }
    return id;
}


/** \brief Acquire a strong reference to an object.
 *  Check that object has the specified "object ID" (which is really a class ID) and is in the
 *  realized state.  If so, then acquire a strong reference to it and return true.
 *  Otherwise return false.
 */

SLresult AcquireStrongRef(IObject *object, SLuint32 expectedObjectID)
{
    if (NULL == object) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    // NTH additional validity checks on address here
    SLresult result;
    object_lock_exclusive(object);
    SLuint32 actualObjectID = IObjectToObjectID(object);
    if (expectedObjectID != actualObjectID) {
        SL_LOGE("object %p has object ID %u but expected %u", object, actualObjectID,
            expectedObjectID);
        result = SL_RESULT_PARAMETER_INVALID;
    } else if (SL_OBJECT_STATE_REALIZED != object->mState) {
        SL_LOGE("object %p with object ID %u is not realized", object, actualObjectID);
        result = SL_RESULT_PRECONDITIONS_VIOLATED;
    } else {
        ++object->mStrongRefCount;
        result = SL_RESULT_SUCCESS;
    }
    object_unlock_exclusive(object);
    return result;
}


/** \brief Release a strong reference to an object.
 *  Entry condition: the object is locked.
 *  Exit condition: the object is unlocked.
 *  Finishes the destroy if needed.
 */

void ReleaseStrongRefAndUnlockExclusive(IObject *object)
{
#ifdef USE_DEBUG
    assert(pthread_equal(pthread_self(), object->mOwner));
#endif
    assert(0 < object->mStrongRefCount);
    if ((0 == --object->mStrongRefCount) && (SL_OBJECT_STATE_DESTROYING == object->mState)) {
        // FIXME do the destroy here - merge with IDestroy
        // but can't do this until we move Destroy to the sync thread
        // as Destroy is now a blocking operation, and to avoid a race
    } else {
        object_unlock_exclusive(object);
    }
}


/** \brief Release a strong reference to an object.
 *  Entry condition: the object is unlocked.
 *  Exit condition: the object is unlocked.
 *  Finishes the destroy if needed.
 */

void ReleaseStrongRef(IObject *object)
{
    assert(NULL != object);
    object_lock_exclusive(object);
    ReleaseStrongRefAndUnlockExclusive(object);
}


/** \brief Convert POSIX pthread error code to OpenSL ES result code */

SLresult err_to_result(int err)
{
    if (EAGAIN == err || ENOMEM == err) {
        return SL_RESULT_RESOURCE_ERROR;
    }
    if (0 != err) {
        return SL_RESULT_INTERNAL_ERROR;
    }
    return SL_RESULT_SUCCESS;
}


/** \brief Check the interface IDs passed into a Create operation */

SLresult checkInterfaces(const ClassTable *clazz, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired,
    unsigned *pExposedMask, unsigned *pRequiredMask)
{
    assert(NULL != clazz && NULL != pExposedMask);
    // Initially no interfaces are exposed
    unsigned exposedMask = 0;
    unsigned requiredMask = 0;
    const struct iid_vtable *interfaces = clazz->mInterfaces;
    SLuint32 interfaceCount = clazz->mInterfaceCount;
    SLuint32 i;
    // Expose all implicit interfaces
    for (i = 0; i < interfaceCount; ++i) {
        switch (interfaces[i].mInterface) {
        case INTERFACE_IMPLICIT:
        case INTERFACE_IMPLICIT_PREREALIZE:
            // there must be an initialization hook present
            if (NULL != MPH_init_table[interfaces[i].mMPH].mInit) {
                exposedMask |= 1 << i;
            }
            break;
        case INTERFACE_EXPLICIT:
        case INTERFACE_DYNAMIC:
        case INTERFACE_UNAVAILABLE:
        case INTERFACE_EXPLICIT_PREREALIZE:
            break;
        default:
            assert(false);
            break;
        }
    }
    if (0 < numInterfaces) {
        if (NULL == pInterfaceIds || NULL == pInterfaceRequired) {
            return SL_RESULT_PARAMETER_INVALID;
        }
        bool anyRequiredButUnsupported = false;
        // Loop for each requested interface
        for (i = 0; i < numInterfaces; ++i) {
            SLInterfaceID iid = pInterfaceIds[i];
            if (NULL == iid) {
                return SL_RESULT_PARAMETER_INVALID;
            }
            SLboolean isRequired = pInterfaceRequired[i];
            int MPH, index;
            if ((0 > (MPH = IID_to_MPH(iid))) ||
                    // there must be an initialization hook present
                    (NULL == MPH_init_table[MPH].mInit) ||
                    (0 > (index = clazz->mMPH_to_index[MPH])) ||
                    (INTERFACE_UNAVAILABLE == interfaces[index].mInterface)) {
                // Here if interface was not found, or is not available for this object type
                if (isRequired) {
                    // Application said it required the interface, so give up
                    SL_LOGE("class %s interface %u required but unavailable MPH=%d",
                            clazz->mName, i, MPH);
                    anyRequiredButUnsupported = true;
                }
                // Application said it didn't really need the interface, so ignore with warning
                SL_LOGW("class %s interface %u requested but unavailable MPH=%d",
                        clazz->mName, i, MPH);
                continue;
            }
            if (isRequired) {
                requiredMask |= (1 << index);
            }
            // The requested interface was both found and available, so expose it
            exposedMask |= (1 << index);
            // Note that we ignore duplicate requests, including equal and aliased IDs
        }
        if (anyRequiredButUnsupported) {
            return SL_RESULT_FEATURE_UNSUPPORTED;
        }
    }
    *pExposedMask = exposedMask;
    if (NULL != pRequiredMask) {
        *pRequiredMask = requiredMask;
    }
    return SL_RESULT_SUCCESS;
}


/* Interface initialization hooks */

extern void
    I3DCommit_init(void *),
    I3DDoppler_init(void *),
    I3DGrouping_init(void *),
    I3DLocation_init(void *),
    I3DMacroscopic_init(void *),
    I3DSource_init(void *),
    IAndroidAcousticEchoCancellation_init(void *),
    IAndroidAutomaticGainControl_init(void *),
    IAndroidConfiguration_init(void *),
    IAndroidEffect_init(void *),
    IAndroidEffectCapabilities_init(void *),
    IAndroidEffectSend_init(void *),
    IAndroidBufferQueue_init(void *),
    IAudioDecoderCapabilities_init(void *),
    IAudioEncoder_init(void *),
    IAudioEncoderCapabilities_init(void *),
    IAudioIODeviceCapabilities_init(void *),
    IBassBoost_init(void *),
    IBufferQueue_init(void *),
    IDeviceVolume_init(void *),
    IDynamicInterfaceManagement_init(void *),
    IDynamicSource_init(void *),
    IEffectSend_init(void *),
    IEngine_init(void *),
    IEngineCapabilities_init(void *),
    IEnvironmentalReverb_init(void *),
    IEqualizer_init(void *),
    ILEDArray_init(void *),
    IMIDIMessage_init(void *),
    IMIDIMuteSolo_init(void *),
    IMIDITempo_init(void *),
    IMIDITime_init(void *),
    IMetadataExtraction_init(void *),
    IMetadataTraversal_init(void *),
    IMuteSolo_init(void *),
    IAndroidNoiseSuppression_init(void *),
    IObject_init(void *),
    IOutputMix_init(void *),
    IOutputMixExt_init(void *),
    IPitch_init(void *),
    IPlay_init(void *),
    IPlaybackRate_init(void *),
    IPrefetchStatus_init(void *),
    IPresetReverb_init(void *),
    IRatePitch_init(void *),
    IRecord_init(void *),
    ISeek_init(void *),
    IThreadSync_init(void *),
    IVibra_init(void *),
    IVirtualizer_init(void *),
    IVisualization_init(void *),
    IVolume_init(void *);

extern void
    I3DGrouping_deinit(void *),
    IAndroidAcousticEchoCancellation_deinit(void *),
    IAndroidAutomaticGainControl_deinit(void *),
    IAndroidEffect_deinit(void *),
    IAndroidEffectCapabilities_deinit(void *),
    IAndroidBufferQueue_deinit(void *),
    IAndroidNoiseSuppression_deinit(void*),
    IBassBoost_deinit(void *),
    IBufferQueue_deinit(void *),
    IEngine_deinit(void *),
    IEnvironmentalReverb_deinit(void *),
    IEqualizer_deinit(void *),
    IObject_deinit(void *),
    IPresetReverb_deinit(void *),
    IThreadSync_deinit(void *),
    IVirtualizer_deinit(void *),
    IAndroidConfiguration_deinit(void *);

extern bool
    IAndroidAcousticEchoCancellation_Expose(void *),
    IAndroidAutomaticGainControl_Expose(void *),
    IAndroidEffectCapabilities_Expose(void *),
    IAndroidNoiseSuppression_Expose(void *),
    IBassBoost_Expose(void *),
    IEnvironmentalReverb_Expose(void *),
    IEqualizer_Expose(void *),
    IPresetReverb_Expose(void *),
    IVirtualizer_Expose(void *);

extern void
    IXAEngine_init(void *),
    IStreamInformation_init(void*),
    IVideoDecoderCapabilities_init(void *);

extern void
    IXAEngine_deinit(void *),
    IStreamInformation_deinit(void *),
    IVideoDecoderCapabilities_deinit(void *);

extern bool
    IVideoDecoderCapabilities_expose(void *);

#if !(USE_PROFILES & USE_PROFILES_MUSIC)
#define IDynamicSource_init         NULL
#define IMetadataTraversal_init     NULL
#define IVisualization_init         NULL
#endif

#if !(USE_PROFILES & USE_PROFILES_GAME)
#define I3DCommit_init      NULL
#define I3DDoppler_init     NULL
#define I3DGrouping_init    NULL
#define I3DLocation_init    NULL
#define I3DMacroscopic_init NULL
#define I3DSource_init      NULL
#define IMIDIMessage_init   NULL
#define IMIDIMuteSolo_init  NULL
#define IMIDITempo_init     NULL
#define IMIDITime_init      NULL
#define IPitch_init         NULL
#define IRatePitch_init     NULL
#define I3DGrouping_deinit  NULL
#endif

#if !(USE_PROFILES & USE_PROFILES_BASE)
#define IAudioDecoderCapabilities_init   NULL
#define IAudioEncoderCapabilities_init   NULL
#define IAudioEncoder_init               NULL
#define IAudioIODeviceCapabilities_init  NULL
#define IDeviceVolume_init               NULL
#define IEngineCapabilities_init         NULL
#define IThreadSync_init                 NULL
#define IThreadSync_deinit               NULL
#endif

#if !(USE_PROFILES & USE_PROFILES_OPTIONAL)
#define ILEDArray_init  NULL
#define IVibra_init     NULL
#endif

#ifndef ANDROID
#define IAndroidConfiguration_init        NULL
#define IAndroidConfiguration_deinit      NULL
#define IAndroidEffect_init               NULL
#define IAndroidEffectCapabilities_init   NULL
#define IAndroidEffectSend_init           NULL
#define IAndroidEffect_deinit             NULL
#define IAndroidEffectCapabilities_deinit NULL
#define IAndroidEffectCapabilities_Expose NULL
#define IAndroidBufferQueue_init          NULL
#define IStreamInformation_init           NULL
#define IAndroidBufferQueue_deinit        NULL
#define IStreamInformation_deinit         NULL
#endif

#ifndef USE_OUTPUTMIXEXT
#define IOutputMixExt_init  NULL
#endif


/*static*/ const struct MPH_init MPH_init_table[MPH_MAX] = {
    { /* MPH_3DCOMMIT, */ I3DCommit_init, NULL, NULL, NULL, NULL },
    { /* MPH_3DDOPPLER, */ I3DDoppler_init, NULL, NULL, NULL, NULL },
    { /* MPH_3DGROUPING, */ I3DGrouping_init, NULL, I3DGrouping_deinit, NULL, NULL },
    { /* MPH_3DLOCATION, */ I3DLocation_init, NULL, NULL, NULL, NULL },
    { /* MPH_3DMACROSCOPIC, */ I3DMacroscopic_init, NULL, NULL, NULL, NULL },
    { /* MPH_3DSOURCE, */ I3DSource_init, NULL, NULL, NULL, NULL },
    { /* MPH_AUDIODECODERCAPABILITIES, */ IAudioDecoderCapabilities_init, NULL, NULL, NULL, NULL },
    { /* MPH_AUDIOENCODER, */ IAudioEncoder_init, NULL, NULL, NULL, NULL },
    { /* MPH_AUDIOENCODERCAPABILITIES, */ IAudioEncoderCapabilities_init, NULL, NULL, NULL, NULL },
    { /* MPH_AUDIOIODEVICECAPABILITIES, */ IAudioIODeviceCapabilities_init, NULL, NULL, NULL,
        NULL },
    { /* MPH_BASSBOOST, */ IBassBoost_init, NULL, IBassBoost_deinit, IBassBoost_Expose, NULL },
    { /* MPH_BUFFERQUEUE, */ IBufferQueue_init, NULL, IBufferQueue_deinit, NULL, NULL },
    { /* MPH_DEVICEVOLUME, */ IDeviceVolume_init, NULL, NULL, NULL, NULL },
    { /* MPH_DYNAMICINTERFACEMANAGEMENT, */ IDynamicInterfaceManagement_init, NULL, NULL, NULL,
        NULL },
    { /* MPH_DYNAMICSOURCE, */ IDynamicSource_init, NULL, NULL, NULL, NULL },
    { /* MPH_EFFECTSEND, */ IEffectSend_init, NULL, NULL, NULL, NULL },
    { /* MPH_ENGINE, */ IEngine_init, NULL, IEngine_deinit, NULL, NULL },
    { /* MPH_ENGINECAPABILITIES, */ IEngineCapabilities_init, NULL, NULL, NULL, NULL },
    { /* MPH_ENVIRONMENTALREVERB, */ IEnvironmentalReverb_init, NULL, IEnvironmentalReverb_deinit,
        IEnvironmentalReverb_Expose, NULL },
    { /* MPH_EQUALIZER, */ IEqualizer_init, NULL, IEqualizer_deinit, IEqualizer_Expose, NULL },
    { /* MPH_LED, */ ILEDArray_init, NULL, NULL, NULL, NULL },
    { /* MPH_METADATAEXTRACTION, */ IMetadataExtraction_init, NULL, NULL, NULL, NULL },
    { /* MPH_METADATATRAVERSAL, */ IMetadataTraversal_init, NULL, NULL, NULL, NULL },
    { /* MPH_MIDIMESSAGE, */ IMIDIMessage_init, NULL, NULL, NULL, NULL },
    { /* MPH_MIDITIME, */ IMIDITime_init, NULL, NULL, NULL, NULL },
    { /* MPH_MIDITEMPO, */ IMIDITempo_init, NULL, NULL, NULL, NULL },
    { /* MPH_MIDIMUTESOLO, */ IMIDIMuteSolo_init, NULL, NULL, NULL, NULL },
    { /* MPH_MUTESOLO, */ IMuteSolo_init, NULL, NULL, NULL, NULL },
    { /* MPH_NULL, */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_OBJECT, */ IObject_init, NULL, IObject_deinit, NULL, NULL },
    { /* MPH_OUTPUTMIX, */ IOutputMix_init, NULL, NULL, NULL, NULL },
    { /* MPH_PITCH, */ IPitch_init, NULL, NULL, NULL, NULL },
    { /* MPH_PLAY, */ IPlay_init, NULL, NULL, NULL, NULL },
    { /* MPH_PLAYBACKRATE, */ IPlaybackRate_init, NULL, NULL, NULL, NULL },
    { /* MPH_PREFETCHSTATUS, */ IPrefetchStatus_init, NULL, NULL, NULL, NULL },
    { /* MPH_PRESETREVERB, */ IPresetReverb_init, NULL, IPresetReverb_deinit,
        IPresetReverb_Expose, NULL },
    { /* MPH_RATEPITCH, */ IRatePitch_init, NULL, NULL, NULL, NULL },
    { /* MPH_RECORD, */ IRecord_init, NULL, NULL, NULL, NULL },
    { /* MPH_SEEK, */ ISeek_init, NULL, NULL, NULL, NULL },
    { /* MPH_THREADSYNC, */ IThreadSync_init, NULL, IThreadSync_deinit, NULL, NULL },
    { /* MPH_VIBRA, */ IVibra_init, NULL, NULL, NULL, NULL },
    { /* MPH_VIRTUALIZER, */ IVirtualizer_init, NULL, IVirtualizer_deinit, IVirtualizer_Expose,
        NULL },
    { /* MPH_VISUALIZATION, */ IVisualization_init, NULL, NULL, NULL, NULL },
    { /* MPH_VOLUME, */ IVolume_init, NULL, NULL, NULL, NULL },
// Wilhelm desktop extended interfaces
    { /* MPH_OUTPUTMIXEXT, */ IOutputMixExt_init, NULL, NULL, NULL, NULL },
// Android API level 9 extended interfaces
    { /* MPH_ANDROIDEFFECT */ IAndroidEffect_init, NULL, IAndroidEffect_deinit, NULL, NULL },
    { /* MPH_ANDROIDEFFECTCAPABILITIES */ IAndroidEffectCapabilities_init, NULL,
        IAndroidEffectCapabilities_deinit, IAndroidEffectCapabilities_Expose, NULL },
    { /* MPH_ANDROIDEFFECTSEND */ IAndroidEffectSend_init, NULL, NULL, NULL, NULL },
    { /* MPH_ANDROIDCONFIGURATION */ IAndroidConfiguration_init, NULL, IAndroidConfiguration_deinit,
            NULL, NULL },
    { /* MPH_ANDROIDSIMPLEBUFFERQUEUE */ IBufferQueue_init /* alias */, NULL, NULL, NULL, NULL },
// Android API level 10 extended interfaces
    { /* MPH_ANDROIDBUFFERQUEUESOURCE */ IAndroidBufferQueue_init, NULL, IAndroidBufferQueue_deinit,
        NULL, NULL },
// OpenMAX AL 1.0.1 interfaces
    { /* MPH_XAAUDIODECODERCAPABILITIES */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAAUDIOENCODER */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAAUDIOENCODERCAPABILITIES */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAAUDIOIODEVICECAPABILITIES */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XACAMERA */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XACAMERACAPABILITIES */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XACONFIGEXTENSION */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XADEVICEVOLUME */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XADYNAMICINTERFACEMANAGEMENT 59 */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XADYNAMICSOURCE */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAENGINE */ IXAEngine_init, NULL, IXAEngine_deinit, NULL, NULL },
    { /* MPH_XAEQUALIZER */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAIMAGECONTROLS */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAIMAGEDECODERCAPABILITIES */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAIMAGEEFFECTS */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAIMAGEENCODER */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAIMAGEENCODERCAPABILITIES */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XALED */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAMETADATAEXTRACTION */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAMETADATAINSERTION */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAMETADATATRAVERSAL */ NULL, NULL, NULL, NULL, NULL },
//  { /* MPH_XANULL */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAOBJECT */ IObject_init, NULL, IObject_deinit, NULL, NULL },
    { /* MPH_XAOUTPUTMIX */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAPLAY */ IPlay_init, NULL, NULL, NULL, NULL },
    { /* MPH_XAPLAYBACKRATE */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAPREFETCHSTATUS, */ IPrefetchStatus_init, NULL, NULL, NULL, NULL },
    { /* MPH_XARADIO */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XARDS */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XARECORD */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XASEEK */ ISeek_init, NULL, NULL, NULL, NULL },
    { /* MPH_XASNAPSHOT */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XASTREAMINFORMATION */ IStreamInformation_init, NULL, IStreamInformation_deinit,
        NULL, NULL },
    { /* MPH_XATHREADSYNC */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAVIBRA */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAVIDEODECODERCAPABILITIES */ IVideoDecoderCapabilities_init, NULL,
            IVideoDecoderCapabilities_deinit, IVideoDecoderCapabilities_expose, NULL },
    { /* MPH_XAVIDEOENCODER */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAVIDEOENCODERCAPABILITIES */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAVIDEOPOSTPROCESSING */ NULL, NULL, NULL, NULL, NULL },
    { /* MPH_XAVOLUME, */ IVolume_init, NULL, NULL, NULL, NULL },
    { /* MPH_ANDROIDACOUSTICECHOCANCELLATION, */ IAndroidAcousticEchoCancellation_init, NULL,
            IAndroidAcousticEchoCancellation_deinit, IAndroidAcousticEchoCancellation_Expose,
            NULL },
    { /* MPH_ANDROIDAUTOMATICGAINCONTROL, */ IAndroidAutomaticGainControl_init, NULL,
            IAndroidAutomaticGainControl_deinit, IAndroidAutomaticGainControl_Expose, NULL },
    { /* MPH_ANDROIDNOISESUPPRESSION, */ IAndroidNoiseSuppression_init, NULL,
            IAndroidNoiseSuppression_deinit, IAndroidNoiseSuppression_Expose, NULL },
};


/** \brief Construct a new instance of the specified class, exposing selected interfaces */

IObject *construct(const ClassTable *clazz, unsigned exposedMask, SLEngineItf engine)
{
    IObject *thiz;
    // Do not change this to malloc; we depend on the object being memset to zero
    thiz = (IObject *) calloc(1, clazz->mSize);
    if (NULL != thiz) {
        SL_LOGV("construct %s at %p", clazz->mName, thiz);
        unsigned lossOfControlMask = 0;
        // a NULL engine means we are constructing the engine
        IEngine *thisEngine = (IEngine *) engine;
        if (NULL == thisEngine) {
            // thisEngine = &((CEngine *) thiz)->mEngine;
            thiz->mEngine = (CEngine *) thiz;
        } else {
            thiz->mEngine = (CEngine *) thisEngine->mThis;
            interface_lock_exclusive(thisEngine);
            if (MAX_INSTANCE <= thisEngine->mInstanceCount) {
                SL_LOGE("Too many objects");
                interface_unlock_exclusive(thisEngine);
                free(thiz);
                return NULL;
            }
            // pre-allocate a pending slot, but don't assign bit from mInstanceMask yet
            ++thisEngine->mInstanceCount;
            assert(((unsigned) ~0) != thisEngine->mInstanceMask);
            interface_unlock_exclusive(thisEngine);
            // const, no lock needed
            if (thisEngine->mLossOfControlGlobal) {
                lossOfControlMask = ~0;
            }
        }
        thiz->mLossOfControlMask = lossOfControlMask;
        thiz->mClass = clazz;
        const struct iid_vtable *x = clazz->mInterfaces;
        SLuint8 *interfaceStateP = thiz->mInterfaceStates;
        SLuint32 index;
        for (index = 0; index < clazz->mInterfaceCount; ++index, ++x, exposedMask >>= 1) {
            SLuint8 state;
            // initialize all interfaces with init hooks, even if not exposed
            const struct MPH_init *mi = &MPH_init_table[x->mMPH];
            VoidHook init = mi->mInit;
            if (NULL != init) {
                void *self = (char *) thiz + x->mOffset;
                // IObject does not have an mThis, so [1] is not always defined
                if (index) {
                    ((IObject **) self)[1] = thiz;
                }
                // call the initialization hook
                (*init)(self);
                // IObject does not require a call to GetInterface
                if (index) {
                    // This trickery invalidates the v-table until GetInterface
                    ((size_t *) self)[0] ^= ~0;
                }
                // if interface is exposed, also call the optional expose hook
                BoolHook expose;
                state = (exposedMask & 1) && ((NULL == (expose = mi->mExpose)) || (*expose)(self)) ?
                        INTERFACE_EXPOSED : INTERFACE_INITIALIZED;
                // FIXME log or report to application if an expose hook on a
                // required explicit interface fails at creation time
            } else {
                state = INTERFACE_UNINITIALIZED;
            }
            *interfaceStateP++ = state;
        }
        // note that the new object is not yet published; creator must call IObject_Publish
    }
    return thiz;
}
