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

#ifndef ANDROID_AUDIOPOLICYEFFECTS_H
#define ANDROID_AUDIOPOLICYEFFECTS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cutils/misc.h>
#include <media/AudioEffect.h>
#include <system/audio.h>
#include <utils/Vector.h>
#include <utils/SortedVector.h>

namespace android {

// ----------------------------------------------------------------------------

// AudioPolicyEffects class
// This class will manage all effects attached to input and output streams in
// AudioPolicyService as configured in audio_effects.conf.
class AudioPolicyEffects : public RefBase
{

public:

    // The constructor will parse audio_effects.conf
    // First it will look whether vendor specific file exists,
    // otherwise it will parse the system default file.
	         AudioPolicyEffects();
    virtual ~AudioPolicyEffects();

    // NOTE: methods on AudioPolicyEffects should never be called with the AudioPolicyService
    // main mutex (mLock) held as they will indirectly call back into AudioPolicyService when
    // managing audio effects.

    // Return a list of effect descriptors for default input effects
    // associated with audioSession
    status_t queryDefaultInputEffects(audio_session_t audioSession,
                             effect_descriptor_t *descriptors,
                             uint32_t *count);

    // Add all input effects associated with this input
    // Effects are attached depending on the audio_source_t
    status_t addInputEffects(audio_io_handle_t input,
                             audio_source_t inputSource,
                             audio_session_t audioSession);

    // Add all input effects associated to this input
    status_t releaseInputEffects(audio_io_handle_t input,
                                 audio_session_t audioSession);

    // Return a list of effect descriptors for default output effects
    // associated with audioSession
    status_t queryDefaultOutputSessionEffects(audio_session_t audioSession,
                             effect_descriptor_t *descriptors,
                             uint32_t *count);

    // Add all output effects associated to this output
    // Effects are attached depending on the audio_stream_type_t
    status_t addOutputSessionEffects(audio_io_handle_t output,
                             audio_stream_type_t stream,
                             audio_session_t audioSession);

    // release all output effects associated with this output stream and audiosession
    status_t releaseOutputSessionEffects(audio_io_handle_t output,
                             audio_stream_type_t stream,
                             audio_session_t audioSession);

    // Add the effect to the list of default effects for sources of type |source|.
    status_t addSourceDefaultEffect(const effect_uuid_t *type,
                                    const String16& opPackageName,
                                    const effect_uuid_t *uuid,
                                    int32_t priority,
                                    audio_source_t source,
                                    audio_unique_id_t* id);

    // Add the effect to the list of default effects for streams of a given usage.
    status_t addStreamDefaultEffect(const effect_uuid_t *type,
                                    const String16& opPackageName,
                                    const effect_uuid_t *uuid,
                                    int32_t priority,
                                    audio_usage_t usage,
                                    audio_unique_id_t* id);

    // Remove the default source effect from wherever it's attached.
    status_t removeSourceDefaultEffect(audio_unique_id_t id);

    // Remove the default stream effect from wherever it's attached.
    status_t removeStreamDefaultEffect(audio_unique_id_t id);

private:

    // class to store the description of an effects and its parameters
    // as defined in audio_effects.conf
    class EffectDesc {
    public:
        EffectDesc(const char *name,
                   const effect_uuid_t& typeUuid,
                   const String16& opPackageName,
                   const effect_uuid_t& uuid,
                   uint32_t priority,
                   audio_unique_id_t id) :
                        mName(strdup(name)),
                        mTypeUuid(typeUuid),
                        mOpPackageName(opPackageName),
                        mUuid(uuid),
                        mPriority(priority),
                        mId(id) { }
        EffectDesc(const char *name, const effect_uuid_t& uuid) :
                        EffectDesc(name,
                                   *EFFECT_UUID_NULL,
                                   String16(""),
                                   uuid,
                                   0,
                                   AUDIO_UNIQUE_ID_ALLOCATE) { }
        EffectDesc(const EffectDesc& orig) :
                        mName(strdup(orig.mName)),
                        mTypeUuid(orig.mTypeUuid),
                        mOpPackageName(orig.mOpPackageName),
                        mUuid(orig.mUuid),
                        mPriority(orig.mPriority),
                        mId(orig.mId) {
                            // deep copy mParams
                            for (size_t k = 0; k < orig.mParams.size(); k++) {
                                effect_param_t *origParam = orig.mParams[k];
                                // psize and vsize are rounded up to an int boundary for allocation
                                size_t origSize = sizeof(effect_param_t) +
                                                  ((origParam->psize + 3) & ~3) +
                                                  ((origParam->vsize + 3) & ~3);
                                effect_param_t *dupParam = (effect_param_t *) malloc(origSize);
                                memcpy(dupParam, origParam, origSize);
                                // This works because the param buffer allocation is also done by
                                // multiples of 4 bytes originally. In theory we should memcpy only
                                // the actual param size, that is without rounding vsize.
                                mParams.add(dupParam);
                            }
                        }
        /*virtual*/ ~EffectDesc() {
            free(mName);
            for (size_t k = 0; k < mParams.size(); k++) {
                free(mParams[k]);
            }
        }
        char *mName;
        effect_uuid_t mTypeUuid;
        String16 mOpPackageName;
        effect_uuid_t mUuid;
        int32_t mPriority;
        audio_unique_id_t mId;
        Vector <effect_param_t *> mParams;
    };

    // class to store voctor of EffectDesc
    class EffectDescVector {
    public:
        EffectDescVector() {}
        /*virtual*/ ~EffectDescVector() {
            for (size_t j = 0; j < mEffects.size(); j++) {
                delete mEffects[j];
            }
        }
        Vector <EffectDesc *> mEffects;
    };

    // class to store voctor of AudioEffects
    class EffectVector {
    public:
        explicit EffectVector(audio_session_t session) : mSessionId(session), mRefCount(0) {}
        /*virtual*/ ~EffectVector() {}

        // Enable or disable all effects in effect vector
        void setProcessorEnabled(bool enabled);

        const audio_session_t mSessionId;
        // AudioPolicyManager keeps mLock, no need for lock on reference count here
        int mRefCount;
        Vector< sp<AudioEffect> >mEffects;
    };


    static const char * const kInputSourceNames[AUDIO_SOURCE_CNT -1];
    static audio_source_t inputSourceNameToEnum(const char *name);

    static const char *kStreamNames[AUDIO_STREAM_PUBLIC_CNT+1]; //+1 required as streams start from -1
    audio_stream_type_t streamNameToEnum(const char *name);

    // Parse audio_effects.conf
    status_t loadAudioEffectConfig(const char *path); // TODO: add legacy in the name
    status_t loadAudioEffectXmlConfig(); // TODO: remove "Xml" in the name

    // Load all effects descriptors in configuration file
    status_t loadEffects(cnode *root, Vector <EffectDesc *>& effects);
    EffectDesc *loadEffect(cnode *root);

    // Load all automatic effect configurations
    status_t loadInputEffectConfigurations(cnode *root, const Vector <EffectDesc *>& effects);
    status_t loadStreamEffectConfigurations(cnode *root, const Vector <EffectDesc *>& effects);
    EffectDescVector *loadEffectConfig(cnode *root, const Vector <EffectDesc *>& effects);

    // Load all automatic effect parameters
    void loadEffectParameters(cnode *root, Vector <effect_param_t *>& params);
    effect_param_t *loadEffectParameter(cnode *root);
    size_t readParamValue(cnode *node,
                          char **param,
                          size_t *curSize,
                          size_t *totSize);
    size_t growParamSize(char **param,
                         size_t size,
                         size_t *curSize,
                         size_t *totSize);

    // protects access to mInputSources, mInputSessions, mOutputStreams, mOutputSessions
    // never hold AudioPolicyService::mLock when calling AudioPolicyEffects methods as
    // those can call back into AudioPolicyService methods and try to acquire the mutex
    Mutex mLock;
    // Automatic input effects are configured per audio_source_t
    KeyedVector< audio_source_t, EffectDescVector* > mInputSources;
    // Automatic input effects are unique for audio_io_handle_t
    KeyedVector< audio_session_t, EffectVector* > mInputSessions;

    // Automatic output effects are organized per audio_stream_type_t
    KeyedVector< audio_stream_type_t, EffectDescVector* > mOutputStreams;
    // Automatic output effects are unique for audiosession ID
    KeyedVector< audio_session_t, EffectVector* > mOutputSessions;
};

} // namespace android

#endif // ANDROID_AUDIOPOLICYEFFECTS_H
