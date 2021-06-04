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

#ifndef __MPH_H
#define __MPH_H

// Minimal perfect hash for each interface ID

#define MPH_NONE                      (-1)
#define MPH_MIN                         0

// OpenSL ES 1.0.1 interfaces
#define MPH_3DCOMMIT                    0
#define MPH_3DDOPPLER                   1
#define MPH_3DGROUPING                  2
#define MPH_3DLOCATION                  3
#define MPH_3DMACROSCOPIC               4
#define MPH_3DSOURCE                    5
#define MPH_AUDIODECODERCAPABILITIES    6
#define MPH_AUDIOENCODER                7
#define MPH_AUDIOENCODERCAPABILITIES    8
#define MPH_AUDIOIODEVICECAPABILITIES   9
#define MPH_BASSBOOST                  10
#define MPH_BUFFERQUEUE                11
#define MPH_DEVICEVOLUME               12
#define MPH_DYNAMICINTERFACEMANAGEMENT 13
#define MPH_DYNAMICSOURCE              14
#define MPH_EFFECTSEND                 15
#define MPH_ENGINE                     16
#define MPH_ENGINECAPABILITIES         17
#define MPH_ENVIRONMENTALREVERB        18
#define MPH_EQUALIZER                  19
#define MPH_LED                        20
#define MPH_METADATAEXTRACTION         21
#define MPH_METADATATRAVERSAL          22
#define MPH_MIDIMESSAGE                23
#define MPH_MIDIMUTESOLO               24
#define MPH_MIDITEMPO                  25
#define MPH_MIDITIME                   26
#define MPH_MUTESOLO                   27
#define MPH_NULL                       28  // shared by SL and XA
#define MPH_OBJECT                     29
#define MPH_OUTPUTMIX                  30
#define MPH_PITCH                      31
#define MPH_PLAY                       32
#define MPH_PLAYBACKRATE               33
#define MPH_PREFETCHSTATUS             34
#define MPH_PRESETREVERB               35
#define MPH_RATEPITCH                  36
#define MPH_RECORD                     37
#define MPH_SEEK                       38
#define MPH_THREADSYNC                 39
#define MPH_VIBRA                      40
#define MPH_VIRTUALIZER                41
#define MPH_VISUALIZATION              42
#define MPH_VOLUME                     43

// Wilhelm desktop extended interfaces
#define MPH_OUTPUTMIXEXT               44

// Android API level 9 extended interfaces
// GUID and MPH are shared by SL and XA, but currently documented for SL only
#define MPH_ANDROIDEFFECT              45
#define MPH_ANDROIDEFFECTCAPABILITIES  46
#define MPH_ANDROIDEFFECTSEND          47
#define MPH_ANDROIDCONFIGURATION       48
#define MPH_ANDROIDSIMPLEBUFFERQUEUE   49

// Android API level 12 extended interfaces
// GUID and MPH are shared by SL and XA, but currently documented for XA only
#define MPH_ANDROIDBUFFERQUEUESOURCE   50

// OpenMAX AL 1.0.1 interfaces
#define MPH_XAAUDIODECODERCAPABILITIES   51
#define MPH_XAAUDIOENCODER               52
#define MPH_XAAUDIOENCODERCAPABILITIES   53
#define MPH_XAAUDIOIODEVICECAPABILITIES  54
#define MPH_XACAMERA                     55
#define MPH_XACAMERACAPABILITIES         56
#define MPH_XACONFIGEXTENSION            57
#define MPH_XADEVICEVOLUME               58
#define MPH_XADYNAMICINTERFACEMANAGEMENT 59
#define MPH_XADYNAMICSOURCE              60
#define MPH_XAENGINE                     61
#define MPH_XAEQUALIZER                  62
#define MPH_XAIMAGECONTROLS              63
#define MPH_XAIMAGEDECODERCAPABILITIES   64
#define MPH_XAIMAGEEFFECTS               65
#define MPH_XAIMAGEENCODER               66
#define MPH_XAIMAGEENCODERCAPABILITIES   67
#define MPH_XALED                        68
#define MPH_XAMETADATAEXTRACTION         69
#define MPH_XAMETADATAINSERTION          70
#define MPH_XAMETADATATRAVERSAL          71
#define MPH_XANULL                 MPH_NULL
#define MPH_XAOBJECT                     72
#define MPH_XAOUTPUTMIX                  73
#define MPH_XAPLAY                       74
#define MPH_XAPLAYBACKRATE               75
#define MPH_XAPREFETCHSTATUS             76
#define MPH_XARADIO                      77
#define MPH_XARDS                        78
#define MPH_XARECORD                     79
#define MPH_XASEEK                       80
#define MPH_XASNAPSHOT                   81
#define MPH_XASTREAMINFORMATION          82
#define MPH_XATHREADSYNC                 83
#define MPH_XAVIBRA                      84
#define MPH_XAVIDEODECODERCAPABILITIES   85
#define MPH_XAVIDEOENCODER               86
#define MPH_XAVIDEOENCODERCAPABILITIES   87
#define MPH_XAVIDEOPOSTPROCESSING        88
#define MPH_XAVOLUME                     89

// Android API level 20 extended interfaces
#define MPH_ANDROIDACOUSTICECHOCANCELLATION 90
#define MPH_ANDROIDAUTOMATICGAINCONTROL     91
#define MPH_ANDROIDNOISESUPPRESSION         92

// total number of interface IDs
#define MPH_MAX                          93

#endif // !defined(__MPH_H)
