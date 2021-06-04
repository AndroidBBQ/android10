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

#ifndef _DC_2I_D16_TRC_WRA_01_PRIVATE_H_
#define _DC_2I_D16_TRC_WRA_01_PRIVATE_H_

#ifdef BUILD_FLOAT
#define DC_FLOAT_STEP   0.0000002384f;
#else
#define DC_D16_STEP     0x200;
#endif


/* The internal state variables are implemented in a (for the user)  hidden structure */
/* In this (private) file, the internal structure is declared fro private use.*/
#ifdef BUILD_FLOAT
typedef struct _Filter_FLOAT_State_
{
    LVM_FLOAT  LeftDC;     /* LeftDC  */
    LVM_FLOAT  RightDC;    /* RightDC  */
}Filter_FLOAT_State;
typedef Filter_FLOAT_State * PFilter_FLOAT_State ;
#ifdef SUPPORT_MC
typedef struct _Filter_FLOAT_State_Mc_
{
    LVM_FLOAT  ChDC[LVM_MAX_CHANNELS];     /* ChannelDC  */
} Filter_FLOAT_State_Mc;
typedef Filter_FLOAT_State_Mc * PFilter_FLOAT_State_Mc ;
#endif
#else
typedef struct _Filter_State_
{
  LVM_INT32  LeftDC;     /* LeftDC  */
  LVM_INT32  RightDC;    /* RightDC  */
}Filter_State;

typedef Filter_State * PFilter_State ;
#endif
#endif /* _DC_2I_D16_TRC_WRA_01_PRIVATE_H_ */
