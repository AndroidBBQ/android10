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

/** \file OpenSLESUT.c OpenSL ES Utility Toolkit */

#include <SLES/OpenSLES.h>
#include <OMXAL/OpenMAXAL.h>
#ifdef ANDROID
#include <SLES/OpenSLES_Android.h>
#include <OMXAL/OpenMAXAL_Android.h>
#endif
#include "OpenSLESUT.h"
#include <stdio.h>
#include <string.h>


/** \brief Maps an interface ID to its display name */

typedef struct
{
    const SLInterfaceID *iid;   ///< The interface ID
    const char *name;           ///< The display name
} Pair;

// ## is token concatenation e.g. a##b becomes ab
// # is stringize operator to convert a symbol to a string constant e.g. #a becomes "a"

#define _(x) { &SL_IID_##x, "SL_IID_" #x }
#define _X(x) { (const SLInterfaceID *) &XA_IID_##x, "XA_IID_" #x }

/** \brief Array of mappings from interface IDs to display names */

static Pair pairs[] = {
    _(3DCOMMIT),
    _(3DDOPPLER),
    _(3DGROUPING),
    _(3DLOCATION),
    _(3DMACROSCOPIC),
    _(3DSOURCE),
    _(AUDIODECODERCAPABILITIES),
    _(AUDIOENCODER),
    _(AUDIOENCODERCAPABILITIES),
    _(AUDIOIODEVICECAPABILITIES),
    _(BASSBOOST),
    _(BUFFERQUEUE),
    _(DEVICEVOLUME),
    _(DYNAMICINTERFACEMANAGEMENT),
    _(DYNAMICSOURCE),
    _(EFFECTSEND),
    _(ENGINE),
    _(ENGINECAPABILITIES),
    _(ENVIRONMENTALREVERB),
    _(EQUALIZER),
    _(LED),
    _(METADATAEXTRACTION),
    _(METADATATRAVERSAL),
    _(MIDIMESSAGE),
    _(MIDIMUTESOLO),
    _(MIDITEMPO),
    _(MIDITIME),
    _(MUTESOLO),
    _(NULL),
    _(OBJECT),
    _(OUTPUTMIX),
    _(PITCH),
    _(PLAY),
    _(PLAYBACKRATE),
    _(PREFETCHSTATUS),
    _(PRESETREVERB),
    _(RATEPITCH),
    _(RECORD),
    _(SEEK),
    _(THREADSYNC),
    _(VIBRA),
    _(VIRTUALIZER),
    _(VISUALIZATION),
    _(VOLUME),
#if 0 // ifdef USE_OUTPUTMIXEXT
    _(OUTPUTMIXEXT),
#endif
    _X(ENGINE),
    _X(VIDEODECODERCAPABILITIES),
#ifdef ANDROID
    _(ANDROIDEFFECT),
    _(ANDROIDEFFECTCAPABILITIES),
    _(ANDROIDEFFECTSEND),
    _(ANDROIDCONFIGURATION),
    _(ANDROIDSIMPLEBUFFERQUEUE),
    _(ANDROIDBUFFERQUEUESOURCE),
    _(ANDROIDACOUSTICECHOCANCELLATION),
    _(ANDROIDAUTOMATICGAINCONTROL),
    _(ANDROIDNOISESUPPRESSION)
#endif
};


/** \brief Print an interface ID in human-readable format */

void slesutPrintIID(SLInterfaceID iid)
{
    Pair *p;
    const Pair *end = &pairs[sizeof(pairs)/sizeof(pairs[0])];
    for (p = pairs; p != end; ++p) {
        if (!memcmp(*p->iid, iid, sizeof(struct SLInterfaceID_))) {
            printf("SL_IID_%s = ", p->name);
            break;
        }
    }
    printf(
        "{ 0x%08X, 0x%04X, 0x%04X, 0x%04X, { 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X } }\n",
        (unsigned) iid->time_low, iid->time_mid, iid->time_hi_and_version, iid->clock_seq,
        iid->node[0], iid->node[1], iid->node[2], iid->node[3], iid->node[4], iid->node[5]);
}


/** \brief Print an array of interface IDs in human-readable format,
 *  including whether they are required or optional
 */

void slesutPrintIIDs(SLInterfaceID *pInterfaceIds, SLboolean *pInterfaceRequired,
    unsigned numInterfaces)
{
    unsigned i;
    for (i = 0; i < numInterfaces; ++i) {
        printf("interfaces[%u]=", i);
        slesutPrintIID(pInterfaceIds[i]);
        printf(" %s\n", (unsigned) pInterfaceRequired[i] ? "required" : "optional");
    }
}


/** \brief Convert an object ID to a string or NULL. */

const char *slesutObjectIDToString(SLuint32 objectID)
{
    static const char * const objectIDstrings[10] = {
        "SL_OBJECTID_ENGINE",
        "SL_OBJECTID_LEDDEVICE",
        "SL_OBJECTID_VIBRADEVICE",
        "SL_OBJECTID_AUDIOPLAYER",
        "SL_OBJECTID_AUDIORECORDER",
        "SL_OBJECTID_MIDIPLAYER",
        "SL_OBJECTID_LISTENER",
        "SL_OBJECTID_3DGROUP",
        "SL_OBJECTID_OUTPUTMIX",
        "SL_OBJECTID_METADATAEXTRACTOR"
    };
    return (0x1001 <= objectID) && (objectID <= 0x100A) ? objectIDstrings[objectID - 0x1001] : NULL;
}
