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

// OpenMAX AL 1.0.1 interfaces

const XAInterfaceID XA_IID_AUDIODECODERCAPABILITIES =
        (XAInterfaceID) &SL_IID_array[MPH_XAAUDIODECODERCAPABILITIES];
const XAInterfaceID XA_IID_AUDIOENCODER = (XAInterfaceID) &SL_IID_array[MPH_XAAUDIOENCODER];
const XAInterfaceID XA_IID_AUDIOENCODERCAPABILITIES =
        (XAInterfaceID) &SL_IID_array[MPH_XAAUDIOENCODERCAPABILITIES];
const XAInterfaceID XA_IID_AUDIOIODEVICECAPABILITIES =
        (XAInterfaceID) &SL_IID_array[MPH_XAAUDIOIODEVICECAPABILITIES];
const XAInterfaceID XA_IID_CAMERA = (XAInterfaceID) &SL_IID_array[MPH_XACAMERA];
const XAInterfaceID XA_IID_CAMERACAPABILITIES =
        (XAInterfaceID) &SL_IID_array[MPH_XACAMERACAPABILITIES];
const XAInterfaceID XA_IID_CONFIGEXTENSION = (XAInterfaceID) &SL_IID_array[MPH_XACONFIGEXTENSION];
const XAInterfaceID XA_IID_DEVICEVOLUME = (XAInterfaceID) &SL_IID_array[MPH_XADEVICEVOLUME];
const XAInterfaceID XA_IID_DYNAMICINTERFACEMANAGEMENT =
        (XAInterfaceID) &SL_IID_array[MPH_XADYNAMICINTERFACEMANAGEMENT];
const XAInterfaceID XA_IID_DYNAMICSOURCE = (XAInterfaceID) &SL_IID_array[MPH_XADYNAMICSOURCE];
const XAInterfaceID XA_IID_ENGINE = (XAInterfaceID) &SL_IID_array[MPH_XAENGINE];
const XAInterfaceID XA_IID_EQUALIZER = (XAInterfaceID) &SL_IID_array[MPH_XAEQUALIZER];
const XAInterfaceID XA_IID_IMAGECONTROLS = (XAInterfaceID) &SL_IID_array[MPH_XAIMAGECONTROLS];
const XAInterfaceID XA_IID_IMAGEDECODERCAPABILITIES =
        (XAInterfaceID) &SL_IID_array[MPH_XAIMAGEDECODERCAPABILITIES];
const XAInterfaceID XA_IID_IMAGEEFFECTS = (XAInterfaceID) &SL_IID_array[MPH_XAIMAGEEFFECTS];
const XAInterfaceID XA_IID_IMAGEENCODER = (XAInterfaceID) &SL_IID_array[MPH_XAIMAGEENCODER];
const XAInterfaceID XA_IID_IMAGEENCODERCAPABILITIES =
        (XAInterfaceID) &SL_IID_array[MPH_XAIMAGEENCODERCAPABILITIES];
const XAInterfaceID XA_IID_LED = (XAInterfaceID) &SL_IID_array[MPH_XALED];
const XAInterfaceID XA_IID_METADATAEXTRACTION =
        (XAInterfaceID) &SL_IID_array[MPH_XAMETADATAEXTRACTION];
const XAInterfaceID XA_IID_METADATAINSERTION =
        (XAInterfaceID) &SL_IID_array[MPH_XAMETADATAINSERTION];
const XAInterfaceID XA_IID_METADATATRAVERSAL =
        (XAInterfaceID) &SL_IID_array[MPH_XAMETADATATRAVERSAL];
const XAInterfaceID XA_IID_NULL = (XAInterfaceID) &SL_IID_array[MPH_XANULL];
const XAInterfaceID XA_IID_OBJECT = (XAInterfaceID) &SL_IID_array[MPH_XAOBJECT];
const XAInterfaceID XA_IID_OUTPUTMIX = (XAInterfaceID) &SL_IID_array[MPH_XAOUTPUTMIX];
const XAInterfaceID XA_IID_PLAY = (XAInterfaceID) &SL_IID_array[MPH_XAPLAY];
const XAInterfaceID XA_IID_PLAYBACKRATE = (XAInterfaceID) &SL_IID_array[MPH_XAPLAYBACKRATE];
const XAInterfaceID XA_IID_PREFETCHSTATUS = (XAInterfaceID) &SL_IID_array[MPH_XAPREFETCHSTATUS];
const XAInterfaceID XA_IID_RADIO = (XAInterfaceID) &SL_IID_array[MPH_XARADIO];
const XAInterfaceID XA_IID_RDS = (XAInterfaceID) &SL_IID_array[MPH_XARDS];
const XAInterfaceID XA_IID_RECORD = (XAInterfaceID) &SL_IID_array[MPH_XARECORD];
const XAInterfaceID XA_IID_SEEK = (XAInterfaceID) &SL_IID_array[MPH_XASEEK];
const XAInterfaceID XA_IID_SNAPSHOT = (XAInterfaceID) &SL_IID_array[MPH_XASNAPSHOT];
const XAInterfaceID XA_IID_STREAMINFORMATION =
        (XAInterfaceID) &SL_IID_array[MPH_XASTREAMINFORMATION];
const XAInterfaceID XA_IID_THREADSYNC = (XAInterfaceID) &SL_IID_array[MPH_XATHREADSYNC];
const XAInterfaceID XA_IID_VIBRA = (XAInterfaceID) &SL_IID_array[MPH_XAVIBRA];
const XAInterfaceID XA_IID_VIDEODECODERCAPABILITIES =
        (XAInterfaceID) &SL_IID_array[MPH_XAVIDEODECODERCAPABILITIES];
const XAInterfaceID XA_IID_VIDEOENCODER = (XAInterfaceID) &SL_IID_array[MPH_XAVIDEOENCODER];
const XAInterfaceID XA_IID_VIDEOENCODERCAPABILITIES =
        (XAInterfaceID) &SL_IID_array[MPH_XAVIDEOENCODERCAPABILITIES];
const XAInterfaceID XA_IID_VIDEOPOSTPROCESSING =
        (XAInterfaceID) &SL_IID_array[MPH_XAVIDEOPOSTPROCESSING];
const XAInterfaceID XA_IID_VOLUME = (XAInterfaceID) &SL_IID_array[MPH_XAVOLUME];

// OpenMAX AL 1.0.1 Android API level 12 extended interfaces
// GUID and MPH are shared by SL and XA
const XAInterfaceID XA_IID_ANDROIDBUFFERQUEUESOURCE =
        (XAInterfaceID) &SL_IID_array[MPH_ANDROIDBUFFERQUEUESOURCE];

#ifdef __cplusplus
}
#endif
