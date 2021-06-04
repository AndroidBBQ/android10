/*
 * Copyright (C) 2004-2010 NXP Software
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

/****************************************************************************************/
/*                                                                                      */
/*  Includes                                                                            */
/*                                                                                      */
/****************************************************************************************/
#include "LVREV.h"

/****************************************************************************************/
/*                                                                                      */
/*  Tables                                                                              */
/*                                                                                      */
/****************************************************************************************/

/* Table with supported sampling rates.  The table can be indexed using LVM_Fs_en       */
#ifndef HIGHER_FS
const LVM_UINT16 LVM_FsTable[] = {
    8000 ,
    11025,
    12000,
    16000,
    22050,
    24000,
    32000,
    44100,
    48000
};
#else
const LVM_UINT32 LVM_FsTable[] = {
    8000 ,
    11025,
    12000,
    16000,
    22050,
    24000,
    32000,
    44100,
    48000,
    88200,
    96000,
    176400,
    192000
};
#endif
/* Table with supported sampling rates.  The table can be indexed using LVM_Fs_en       */
#ifndef HIGHER_FS
LVM_UINT16 LVM_GetFsFromTable(LVM_Fs_en FsIndex){
    if (FsIndex > LVM_FS_48000)
        return 0;

    return (LVM_FsTable[FsIndex]);
}
#else
LVM_UINT32 LVM_GetFsFromTable(LVM_Fs_en FsIndex){
    if (FsIndex > LVM_FS_192000)
        return 0;

    return (LVM_FsTable[FsIndex]);
}
#endif

/* In order to maintain consistant input and out put signal strengths
   output gain/attenuation is applied. This gain depends on T60 and Rooms
   size parameters. These polynomial coefficients are calculated experimentally.
   First value in the table is room size
   second value is  A0
   third value is   A1
   fourth value is  A2
   fifth value is   A3
   sixth value is   A4

     shift value  is to be added array (to use LVM_Polynomial function)

  The gain is calculated using variable x=(T60*32767/7000)*32768;

   first values is used to get polynomial set for given room size,
   For room sizes which are not in the table, linear interpolation can be used.

  */

/* Normalizing output including Reverb Level part (only shift up)*/
#ifndef BUILD_FLOAT
const LVM_INT32 LVREV_GainPolyTable[24][5]={{1,17547434,128867434,-120988896,50761228,},
                                            {2,18256869,172666902,-193169292,88345744,},
                                            {3,16591311,139250151,-149667234,66770059,},
                                            {4,17379977,170835131,-173579321,76278163,},
                                            {5,18963512,210364934,-228623519,103435022,},
                                            {6,17796318,135756417,-144084053,64327698,},
                                            {7,17454695,174593214,-187513064,85146582,},
                                            {8,17229257,140715570,-145790588,65361740,},
                                            {9,17000547,163195946,-176733969,79562130,},
                                            {10,16711699,142476304,-133339887,58366547,},
                                            {13,18108419,149223697,-161762020,74397589,},
                                            {15,16682043,124844884,-134284487,60082180,},
                                            {17,16627346,120936430,-121766674,53146421,},
                                            {20,17338325,125432694,-126616983,56534237,},
                                            {25,16489146,99218217,-94597467,40616506,},
                                            {30,15582373,84479043,-75365006,30952348,},
                                            {40,16000669,84896611,-75031127,30696306,},
                                            {50,15087054,71695031,-59349268,23279669,},
                                            {60,15830714,68672971,-58211201,23671158,},
                                            {70,15536061,66657972,-55901437,22560153,},
                                            {75,15013145,48179917,-24138354,5232074,},
                                            {80,15688738,50195036,-34206760,11515792,},
                                            {90,16003322,48323661,-35607378,13153872,},
                                            {100,15955223,48558201,-33706865,11715792,},
                                            };
#else
const LVM_FLOAT LVREV_GainPolyTable[24][5]={{1,1.045909f,7.681098f,-7.211500f,3.025605f,},
                                            {2,1.088194f,10.291749f,-11.513787f,5.265817f,},
                                            {3,0.988919f,8.299956f,-8.920862f,3.979806f,},
                                            {4,1.035927f,10.182567f,-10.346134f,4.546533f,},
                                            {5,1.130313f,12.538727f,-13.627023f,6.165208f,},
                                            {6,1.060743f,8.091713f,-8.588079f,3.834230f,},
                                            {7,1.040381f,10.406566f,-11.176650f,5.075132f,},
                                            {8,1.026944f,8.387302f,-8.689796f,3.895863f,},
                                            {9,1.013312f,9.727236f,-10.534165f,4.742272f,},
                                            {10,0.996095f,8.492249f,-7.947677f,3.478917f,},
                                            {13,1.079346f,8.894425f,-9.641768f,4.434442f,},
                                            {15,0.994327f,7.441335f,-8.003979f,3.581177f,},
                                            {17,0.991067f,7.208373f,-7.257859f,3.167774f,},
                                            {20,1.033445f,7.476371f,-7.546960f,3.369703f,},
                                            {25,0.982830f,5.913867f,-5.638448f,2.420932f,},
                                            {30,0.928782f,5.035343f,-4.492104f,1.844904f,},
                                            {40,0.953714f,5.060232f,-4.472204f,1.829642f,},
                                            {50,0.899258f,4.273357f,-3.537492f,1.387576f,},
                                            {60,0.943584f,4.093228f,-3.469658f,1.410911f,},
                                            {70,0.926021f,3.973125f,-3.331985f,1.344690f,},
                                            {75,0.894853f,2.871747f,-1.438758f,0.311856f,},
                                            {80,0.935122f,2.991857f,-2.038882f,0.686395f,},
                                            {90,0.953872f,2.880315f,-2.122365f,0.784032f,},
                                            {100,0.951005f,2.894294f,-2.009086f,0.698316f,},
};
#endif
/* End of file */

