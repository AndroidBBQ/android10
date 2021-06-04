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

// Map minimal perfect hash of an interface ID to its name

#include <assert.h>
#include <string.h>
#include "MPH.h"

const char * const interface_names[MPH_MAX] = {

    // OpenSL ES 1.0.1 interfaces
    "3DCOMMIT",
    "3DDOPPLER",
    "3DGROUPING",
    "3DLOCATION",
    "3DMACROSCOPIC",
    "3DSOURCE",
    "AUDIODECODERCAPABILITIES",
    "AUDIOENCODER",
    "AUDIOENCODERCAPABILITIES",
    "AUDIOIODEVICECAPABILITIES",
    "BASSBOOST",
    "BUFFERQUEUE",
    "DEVICEVOLUME",
    "DYNAMICINTERFACEMANAGEMENT",
    "DYNAMICSOURCE",
    "EFFECTSEND",
    "ENGINE",
    "ENGINECAPABILITIES",
    "ENVIRONMENTALREVERB",
    "EQUALIZER",
    "LED",
    "METADATAEXTRACTION",
    "METADATATRAVERSAL",
    "MIDIMESSAGE",
    "MIDIMUTESOLO",
    "MIDITEMPO",
    "MIDITIME",
    "MUTESOLO",
    "NULL",
    "OBJECT",
    "OUTPUTMIX",
    "PITCH",
    "PLAY",
    "PLAYBACKRATE",
    "PREFETCHSTATUS",
    "PRESETREVERB",
    "RATEPITCH",
    "RECORD",
    "SEEK",
    "THREADSYNC",
    "VIBRA",
    "VIRTUALIZER",
    "VISUALIZATION",
    "VOLUME",

    // Wilhelm desktop extended interfaces
    "OUTPUTMIXEXT",

    // Android API level 9 extended interfaces
    "ANDROIDEFFECT",
    "ANDROIDEFFECTCAPABILITIES",
    "ANDROIDEFFECTSEND",
    "ANDROIDCONFIGURATION",
    "ANDROIDSIMPLEBUFFERQUEUE",

    // Android API level 12 extended interfaces
    "ANDROIDBUFFERQUEUESOURCE",

    // OpenMAX AL 1.0.1 interfaces
    "XAAUDIODECODERCAPABILITIES",
    "XAAUDIOENCODER",
    "XAAUDIOENCODERCAPABILITIES",
    "XAAUDIOIODEVICECAPABILITIES",
    "XACAMERA",
    "XACAMERACAPABILITIES",
    "XACONFIGEXTENSION",
    "XADEVICEVOLUME",
    "XADYNAMICINTERFACEMANAGEMENT",
    "XADYNAMICSOURCE",
    "XAENGINE",
    "XAEQUALIZER",
    "XAIMAGECONTROLS",
    "XAIMAGEDECODERCAPABILITIES",
    "XAIMAGEEFFECTS",
    "XAIMAGEENCODER",
    "XAIMAGEENCODERCAPABILITIES",
    "XALED",
    "XAMETADATAEXTRACTION",
    "XAMETADATAINSERTION",
    "XAMETADATATRAVERSAL",
    // "XANULL",
    "XAOBJECT",
    "XAOUTPUTMIX",
    "XAPLAY",
    "XAPLAYBACKRATE",
    "XAPREFETCHSTATUS",
    "XARADIO",
    "XARDS",
    "XARECORD",
    "XASEEK",
    "XASNAPSHOT",
    "XASTREAMINFORMATION",
    "XATHREADSYNC",
    "XAVIBRA",
    "XAVIDEODECODERCAPABILITIES",
    "XAVIDEOENCODER",
    "XAVIDEOENCODERCAPABILITIES",
    "XAVIDEOPOSTPROCESSING",
    "XAVOLUME",

    // Android API level 20 extended interfaces
    "ANDROIDACOUSTICECHOCANCELLATION",
    "ANDROIDAUTOMATICGAINCONTROL",
    "ANDROIDNOISESUPPRESSION",

};


/** Convert an MPH value to an MPH identifier */

void MPH_to_MPH_string(unsigned MPH, char buffer[40])
{
    assert(MPH_MAX > MPH);
    const char *infix;
    infix = "";
    strcpy(buffer, "MPH");
    strcpy(&buffer[3], infix);
    strcat(buffer, "_");
    strcat(buffer, interface_names[MPH]);
}
