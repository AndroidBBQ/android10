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

#ifdef __cplusplus
extern "C" {
#endif

// OpenSL ES 1.0.1 interfaces
const SLInterfaceID SL_IID_3DCOMMIT = &SL_IID_array[MPH_3DCOMMIT];
const SLInterfaceID SL_IID_3DDOPPLER = &SL_IID_array[MPH_3DDOPPLER];
const SLInterfaceID SL_IID_3DGROUPING = &SL_IID_array[MPH_3DGROUPING];
const SLInterfaceID SL_IID_3DLOCATION = &SL_IID_array[MPH_3DLOCATION];
const SLInterfaceID SL_IID_3DMACROSCOPIC = &SL_IID_array[MPH_3DMACROSCOPIC];
const SLInterfaceID SL_IID_3DSOURCE = &SL_IID_array[MPH_3DSOURCE];
const SLInterfaceID SL_IID_AUDIODECODERCAPABILITIES = &SL_IID_array[MPH_AUDIODECODERCAPABILITIES];
const SLInterfaceID SL_IID_AUDIOENCODER = &SL_IID_array[MPH_AUDIOENCODER];
const SLInterfaceID SL_IID_AUDIOENCODERCAPABILITIES = &SL_IID_array[MPH_AUDIOENCODERCAPABILITIES];
const SLInterfaceID SL_IID_AUDIOIODEVICECAPABILITIES = &SL_IID_array[MPH_AUDIOIODEVICECAPABILITIES];
const SLInterfaceID SL_IID_BASSBOOST = &SL_IID_array[MPH_BASSBOOST];
const SLInterfaceID SL_IID_BUFFERQUEUE = &SL_IID_array[MPH_BUFFERQUEUE];
const SLInterfaceID SL_IID_DEVICEVOLUME = &SL_IID_array[MPH_DEVICEVOLUME];
const SLInterfaceID SL_IID_DYNAMICINTERFACEMANAGEMENT =
    &SL_IID_array[MPH_DYNAMICINTERFACEMANAGEMENT];
const SLInterfaceID SL_IID_DYNAMICSOURCE = &SL_IID_array[MPH_DYNAMICSOURCE];
const SLInterfaceID SL_IID_EFFECTSEND = &SL_IID_array[MPH_EFFECTSEND];
const SLInterfaceID SL_IID_ENGINE = &SL_IID_array[MPH_ENGINE];
const SLInterfaceID SL_IID_ENGINECAPABILITIES = &SL_IID_array[MPH_ENGINECAPABILITIES];
const SLInterfaceID SL_IID_ENVIRONMENTALREVERB = &SL_IID_array[MPH_ENVIRONMENTALREVERB];
const SLInterfaceID SL_IID_EQUALIZER = &SL_IID_array[MPH_EQUALIZER];
const SLInterfaceID SL_IID_LED = &SL_IID_array[MPH_LED];
const SLInterfaceID SL_IID_METADATAEXTRACTION = &SL_IID_array[MPH_METADATAEXTRACTION];
const SLInterfaceID SL_IID_METADATATRAVERSAL = &SL_IID_array[MPH_METADATATRAVERSAL];
const SLInterfaceID SL_IID_MIDIMESSAGE = &SL_IID_array[MPH_MIDIMESSAGE];
const SLInterfaceID SL_IID_MIDIMUTESOLO = &SL_IID_array[MPH_MIDIMUTESOLO];
const SLInterfaceID SL_IID_MIDITEMPO = &SL_IID_array[MPH_MIDITEMPO];
const SLInterfaceID SL_IID_MIDITIME = &SL_IID_array[MPH_MIDITIME];
const SLInterfaceID SL_IID_MUTESOLO = &SL_IID_array[MPH_MUTESOLO];
const SLInterfaceID SL_IID_NULL = &SL_IID_array[MPH_NULL];
const SLInterfaceID SL_IID_OBJECT = &SL_IID_array[MPH_OBJECT];
const SLInterfaceID SL_IID_OUTPUTMIX = &SL_IID_array[MPH_OUTPUTMIX];
const SLInterfaceID SL_IID_PITCH = &SL_IID_array[MPH_PITCH];
const SLInterfaceID SL_IID_PLAY = &SL_IID_array[MPH_PLAY];
const SLInterfaceID SL_IID_PLAYBACKRATE = &SL_IID_array[MPH_PLAYBACKRATE];
const SLInterfaceID SL_IID_PREFETCHSTATUS = &SL_IID_array[MPH_PREFETCHSTATUS];
const SLInterfaceID SL_IID_PRESETREVERB = &SL_IID_array[MPH_PRESETREVERB];
const SLInterfaceID SL_IID_RATEPITCH = &SL_IID_array[MPH_RATEPITCH];
const SLInterfaceID SL_IID_RECORD = &SL_IID_array[MPH_RECORD];
const SLInterfaceID SL_IID_SEEK = &SL_IID_array[MPH_SEEK];
const SLInterfaceID SL_IID_THREADSYNC = &SL_IID_array[MPH_THREADSYNC];
const SLInterfaceID SL_IID_VIBRA = &SL_IID_array[MPH_VIBRA];
const SLInterfaceID SL_IID_VIRTUALIZER = &SL_IID_array[MPH_VIRTUALIZER];
const SLInterfaceID SL_IID_VISUALIZATION = &SL_IID_array[MPH_VISUALIZATION];
const SLInterfaceID SL_IID_VOLUME = &SL_IID_array[MPH_VOLUME];

// Wilhelm desktop extended interfaces
extern const SLInterfaceID SL_IID_OUTPUTMIXEXT;
const SLInterfaceID SL_IID_OUTPUTMIXEXT = &SL_IID_array[MPH_OUTPUTMIXEXT];

// Android API level 9 extended interfaces
const SLInterfaceID SL_IID_ANDROIDEFFECT = &SL_IID_array[MPH_ANDROIDEFFECT];
const SLInterfaceID SL_IID_ANDROIDEFFECTCAPABILITIES = &SL_IID_array[MPH_ANDROIDEFFECTCAPABILITIES];
const SLInterfaceID SL_IID_ANDROIDEFFECTSEND = &SL_IID_array[MPH_ANDROIDEFFECTSEND];
const SLInterfaceID SL_IID_ANDROIDCONFIGURATION = &SL_IID_array[MPH_ANDROIDCONFIGURATION];
const SLInterfaceID SL_IID_ANDROIDSIMPLEBUFFERQUEUE = &SL_IID_array[MPH_ANDROIDSIMPLEBUFFERQUEUE];

// Android API level 12 extended interfaces
// GUID and MPH are shared by SL and XA
const SLInterfaceID SL_IID_ANDROIDBUFFERQUEUESOURCE = &SL_IID_array[MPH_ANDROIDBUFFERQUEUESOURCE];

const SLInterfaceID SL_IID_ANDROIDACOUSTICECHOCANCELLATION =
        &SL_IID_array[MPH_ANDROIDACOUSTICECHOCANCELLATION];
const SLInterfaceID SL_IID_ANDROIDAUTOMATICGAINCONTROL =
        &SL_IID_array[MPH_ANDROIDAUTOMATICGAINCONTROL];
const SLInterfaceID SL_IID_ANDROIDNOISESUPPRESSION = &SL_IID_array[MPH_ANDROIDNOISESUPPRESSION];

#ifdef __cplusplus
}
#endif
