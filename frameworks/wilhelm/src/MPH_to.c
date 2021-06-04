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

// Map minimal perfect hash of an interface ID to its class index.

#include "MPH.h"

// If defined, then compile with C99 such as GNU C, not GNU C++ or non-GNU C.
//#define USE_DESIGNATED_INITIALIZERS

// It is critical that all entries are populated with either a specific index
// or -1. Do not let the compiler use a default initializer of zero, because
// that actually maps to the IObject index. For non-USE_DESIGNATED_INITIALIZERS
// builds, we use the automagically-generated MPH_to_*.h files for this reason.

// A good test is to use the GNU C compiler with -S option (for assembler output),
// and compile both with and without USE_DESIGNATED_INITIALIZERS.  The resulting .s
// files should be identical for both compilations.

// Important note: if you add any interfaces here, be sure to also
// update the #define for the corresponding INTERFACES_<Class>.

// IObject is the first interface in a class, so the index for MPH_OBJECT must be zero.
// Don't cross streams, otherwise bad things happen.


const signed char MPH_to_3DGroup[MPH_MAX] = {
#ifdef USE_DESIGNATED_INITIALIZERS
    [0 ... MPH_MAX-1] = -1,
    [MPH_OBJECT] = 0,
    [MPH_DYNAMICINTERFACEMANAGEMENT] = 1,
    [MPH_3DLOCATION] = 2,
    [MPH_3DDOPPLER] = 3,
    [MPH_3DSOURCE] = 4,
    [MPH_3DMACROSCOPIC] = 5
#else
#include "MPH_to_3DGroup.h"
#endif
};

const signed char MPH_to_AudioPlayer[MPH_MAX] = {
#ifdef USE_DESIGNATED_INITIALIZERS
    [0 ... MPH_MAX-1] = -1,
    [MPH_OBJECT] = 0,
    [MPH_DYNAMICINTERFACEMANAGEMENT] = 1,
    [MPH_PLAY] = 2,
    [MPH_3DDOPPLER] = 3,
    [MPH_3DGROUPING] = 4,
    [MPH_3DLOCATION] = 5,
    [MPH_3DSOURCE] = 6,
    [MPH_BUFFERQUEUE] = 7,
    [MPH_EFFECTSEND] = 8,
    [MPH_MUTESOLO] = 9,
    [MPH_METADATAEXTRACTION] = 10,
    [MPH_METADATATRAVERSAL] = 11,
    [MPH_PREFETCHSTATUS] = 12,
    [MPH_RATEPITCH] = 13,
    [MPH_SEEK] = 14,
    [MPH_VOLUME] = 15,
    [MPH_3DMACROSCOPIC] = 16,
    [MPH_BASSBOOST] = 17,
    [MPH_DYNAMICSOURCE] = 18,
    [MPH_ENVIRONMENTALREVERB] = 19,
    [MPH_EQUALIZER] = 20,
    [MPH_PITCH] = 21,
    [MPH_PRESETREVERB] = 22,
    [MPH_PLAYBACKRATE] = 23,
    [MPH_VIRTUALIZER] = 24,
    [MPH_VISUALIZATION] = 25,
#ifdef ANDROID
    [MPH_ANDROIDEFFECT] = 26,
    [MPH_ANDROIDEFFECTSEND] = 27,
    [MPH_ANDROIDCONFIGURATION] = 28,
    [MPH_ANDROIDSIMPLEBUFFERQUEUE] = 7,  // alias for [MPH_BUFFERQUEUE]
    [MPH_ANDROIDBUFFERQUEUESOURCE] = 29
#endif
#else
#include "MPH_to_AudioPlayer.h"
#endif
};

const signed char MPH_to_AudioRecorder[MPH_MAX] = {
#ifdef USE_DESIGNATED_INITIALIZERS
    [0 ... MPH_MAX-1] = -1,
    [MPH_OBJECT] = 0,
    [MPH_DYNAMICINTERFACEMANAGEMENT] = 1,
    [MPH_RECORD] = 2,
    [MPH_AUDIOENCODER] = 3,
    [MPH_BASSBOOST] = 4,
    [MPH_DYNAMICSOURCE] = 5,
    [MPH_EQUALIZER] = 6,
    [MPH_VISUALIZATION] = 7,
    [MPH_VOLUME] = 8,
#ifdef ANDROID
    [MPH_ANDROIDSIMPLEBUFFERQUEUE] = 9, // this is not an alias
    [MPH_ANDROIDCONFIGURATION] = 10,
    [MPH_ANDROIDACOUSTICECHOCANCELLATION] = 11,
    [MPH_ANDROIDAUTOMATICGAINCONTROL] = 12,
    [MPH_ANDROIDNOISESUPPRESSION] = 13,
#endif
#else
#include "MPH_to_AudioRecorder.h"
#endif
};

const signed char MPH_to_Engine[MPH_MAX] = {
#ifdef USE_DESIGNATED_INITIALIZERS
    [0 ... MPH_MAX-1] = -1,
    [MPH_OBJECT] = 0,
    [MPH_DYNAMICINTERFACEMANAGEMENT] = 1,
    [MPH_ENGINE] = 2,
    [MPH_ENGINECAPABILITIES] = 3,
    [MPH_THREADSYNC] = 4,
    [MPH_AUDIOIODEVICECAPABILITIES] = 5,
    [MPH_AUDIODECODERCAPABILITIES] = 6,
    [MPH_AUDIOENCODERCAPABILITIES] = 7,
    [MPH_3DCOMMIT] = 8,
    [MPH_DEVICEVOLUME] = 9,
    [MPH_XAENGINE] = 10,
#ifdef ANDROID
    [MPH_ANDROIDEFFECTCAPABILITIES] = 11,
#endif
    [MPH_XAVIDEODECODERCAPABILITIES] = 12
#else
#include "MPH_to_Engine.h"
#endif
};

const signed char MPH_to_LEDDevice[MPH_MAX] = {
#ifdef USE_DESIGNATED_INITIALIZERS
    [0 ... MPH_MAX-1] = -1,
    [MPH_OBJECT] = 0,
    [MPH_DYNAMICINTERFACEMANAGEMENT] = 1,
    [MPH_LED] = 2
#else
#include "MPH_to_LEDDevice.h"
#endif
};

const signed char MPH_to_Listener[MPH_MAX] = {
#ifdef USE_DESIGNATED_INITIALIZERS
    [0 ... MPH_MAX-1] = -1,
    [MPH_OBJECT] = 0,
    [MPH_DYNAMICINTERFACEMANAGEMENT] = 1,
    [MPH_3DDOPPLER] = 2,
    [MPH_3DLOCATION] = 3
#else
#include "MPH_to_Listener.h"
#endif
};

const signed char MPH_to_MetadataExtractor[MPH_MAX] = {
#ifdef USE_DESIGNATED_INITIALIZERS
    [0 ... MPH_MAX-1] = -1,
    [MPH_OBJECT] = 0,
    [MPH_DYNAMICINTERFACEMANAGEMENT] = 1,
    [MPH_DYNAMICSOURCE] = 2,
    [MPH_METADATAEXTRACTION] = 3,
    [MPH_METADATATRAVERSAL] = 4
#else
#include "MPH_to_MetadataExtractor.h"
#endif
};

const signed char MPH_to_MidiPlayer[MPH_MAX] = {
#ifdef USE_DESIGNATED_INITIALIZERS
    [0 ... MPH_MAX-1] = -1,
    [MPH_OBJECT] = 0,
    [MPH_DYNAMICINTERFACEMANAGEMENT] = 1,
    [MPH_PLAY] = 2,
    [MPH_3DDOPPLER] = 3,
    [MPH_3DGROUPING] = 4,
    [MPH_3DLOCATION] = 5,
    [MPH_3DSOURCE] = 6,
    [MPH_BUFFERQUEUE] = 7,
    [MPH_EFFECTSEND] = 8,
    [MPH_MUTESOLO] = 9,
    [MPH_METADATAEXTRACTION] = 10,
    [MPH_METADATATRAVERSAL] = 11,
    [MPH_MIDIMESSAGE] = 12,
    [MPH_MIDITIME] = 13,
    [MPH_MIDITEMPO] = 14,
    [MPH_MIDIMUTESOLO] = 15,
    [MPH_PREFETCHSTATUS] = 16,
    [MPH_SEEK] = 17,
    [MPH_VOLUME] = 18,
    [MPH_3DMACROSCOPIC] = 19,
    [MPH_BASSBOOST] = 20,
    [MPH_DYNAMICSOURCE] = 21,
    [MPH_ENVIRONMENTALREVERB] = 22,
    [MPH_EQUALIZER] = 23,
    [MPH_PITCH] = 24,
    [MPH_PRESETREVERB] = 25,
    [MPH_PLAYBACKRATE] = 26,
    [MPH_VIRTUALIZER] = 27,
    [MPH_VISUALIZATION] = 28,
#else
#include "MPH_to_MidiPlayer.h"
#endif
};

const signed char MPH_to_OutputMix[MPH_MAX] = {
#ifdef USE_DESIGNATED_INITIALIZERS
    [0 ... MPH_MAX-1] = -1,
    [MPH_OBJECT] = 0,
    [MPH_DYNAMICINTERFACEMANAGEMENT] = 1,
    [MPH_OUTPUTMIX] = 2,
#ifdef USE_OUTPUTMIXEXT
    [MPH_OUTPUTMIXEXT] = 3,
#endif
    [MPH_ENVIRONMENTALREVERB] = 4,
    [MPH_EQUALIZER] = 5,
    [MPH_PRESETREVERB] = 6,
    [MPH_VIRTUALIZER] = 7,
    [MPH_VOLUME] = 8,
    [MPH_BASSBOOST] = 9,
    [MPH_VISUALIZATION] = 10,
#ifdef ANDROID
    [MPH_ANDROIDEFFECT] = 11
#endif
#else
#include "MPH_to_OutputMix.h"
#endif
};

const signed char MPH_to_Vibra[MPH_MAX] = {
#ifdef USE_DESIGNATED_INITIALIZERS
    [0 ... MPH_MAX-1] = -1,
    [MPH_OBJECT] = 0,
    [MPH_DYNAMICINTERFACEMANAGEMENT] = 1,
    [MPH_VIBRA] = 2
#else
#include "MPH_to_Vibra.h"
#endif
};

const signed char MPH_to_MediaPlayer[MPH_MAX] = {
#ifdef USE_DESIGNATED_INITIALIZERS
    [0 ... MPH_MAX-1] = -1,
    [MPH_XAOBJECT] = 0,
    [MPH_XADYNAMICINTERFACEMANAGEMENT] = 1,
    [MPH_XAPLAY] = 2,
    [MPH_XASTREAMINFORMATION] = 3,
    [MPH_XAVOLUME] = 4,
    [MPH_XASEEK] = 5,
    [MPH_XAPREFETCHSTATUS] = 6,
#ifdef ANDROID
    [MPH_ANDROIDBUFFERQUEUESOURCE] = 7,
#endif
#else
#include "MPH_to_MediaPlayer.h"
#endif
};
