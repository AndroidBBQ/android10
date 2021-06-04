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

/**********************************************************************************
   INCLUDE FILES
***********************************************************************************/

#include "LVC_Mixer_Private.h"
#include "LVM_Macros.h"
#include "ScalarArithmetic.h"

/**********************************************************************************
   FUNCTION LVCore_MIXSOFT_1ST_D16C31_WRA
***********************************************************************************/
#ifdef BUILD_FLOAT
void LVC_Core_MixSoft_1St_D16C31_WRA(LVMixer3_FLOAT_st *ptrInstance,
                                     const LVM_FLOAT   *src,
                                           LVM_FLOAT   *dst,
                                           LVM_INT16   n)
{
    LVM_INT16   OutLoop;
    LVM_INT16   InLoop;
    LVM_INT32   ii;
    Mix_Private_FLOAT_st  *pInstance=(Mix_Private_FLOAT_st *)(ptrInstance->PrivateParams);
    LVM_FLOAT   Delta= (LVM_FLOAT)pInstance->Delta;
    LVM_FLOAT   Current = (LVM_FLOAT)pInstance->Current;
    LVM_FLOAT   Target= (LVM_FLOAT)pInstance->Target;
    LVM_FLOAT   Temp;

    InLoop = (LVM_INT16)(n >> 2); /* Process per 4 samples */
    OutLoop = (LVM_INT16)(n - (InLoop << 2));

    if(Current<Target){
        if (OutLoop){

            Temp = Current + Delta;
            if (Temp > 1.0f)
                Temp = 1.0f;
            else if (Temp < -1.0f)
                Temp = -1.0f;

            Current=Temp;
            if (Current > Target)
                Current = Target;

            for (ii = OutLoop; ii != 0; ii--){
                *(dst++) = (((LVM_FLOAT)*(src++) * (LVM_FLOAT)Current));
            }
        }

        for (ii = InLoop; ii != 0; ii--){

            Temp = Current + Delta;

            if (Temp > 1.0f)
                Temp = 1.0f;
            else if (Temp < -1.0f)
                Temp = -1.0f;

            Current=Temp;
            if (Current > Target)
                Current = Target;

            *(dst++) = (((LVM_FLOAT)*(src++) * Current) );
            *(dst++) = (((LVM_FLOAT)*(src++) * Current) );
            *(dst++) = (((LVM_FLOAT)*(src++) * Current) );
            *(dst++) = (((LVM_FLOAT)*(src++) * Current) );
        }
    }
    else{
        if (OutLoop){
            Current -= Delta;
            if (Current < Target)
                Current = Target;

            for (ii = OutLoop; ii != 0; ii--){
                *(dst++) = (((LVM_FLOAT)*(src++) * Current));
            }
        }

        for (ii = InLoop; ii != 0; ii--){
            Current -= Delta;
            if (Current < Target)
                Current = Target;

            *(dst++) = (((LVM_FLOAT)*(src++) * Current));
            *(dst++) = (((LVM_FLOAT)*(src++) * Current));
            *(dst++) = (((LVM_FLOAT)*(src++) * Current));
            *(dst++) = (((LVM_FLOAT)*(src++) * Current));
        }
    }
    pInstance->Current=Current;
}


#ifdef SUPPORT_MC
/*
 * FUNCTION:       LVC_Core_MixSoft_Mc_D16C31_WRA
 *
 * DESCRIPTION:
 *  Mixer function with support for processing multichannel input
 *
 * PARAMETERS:
 *  ptrInstance    Instance pointer
 *  src            Source
 *  dst            Destination
 *  NrFrames       Number of frames
 *  NrChannels     Number of channels
 *
 * RETURNS:
 *  void
 *
 */
void LVC_Core_MixSoft_Mc_D16C31_WRA(LVMixer3_FLOAT_st *ptrInstance,
                                    const LVM_FLOAT   *src,
                                          LVM_FLOAT   *dst,
                                          LVM_INT16   NrFrames,
                                          LVM_INT16   NrChannels)
{
    LVM_INT16   OutLoop;
    LVM_INT16   InLoop;
    LVM_INT32   ii, jj;
    Mix_Private_FLOAT_st  *pInstance=(Mix_Private_FLOAT_st *)(ptrInstance->PrivateParams);
    LVM_FLOAT   Delta= (LVM_FLOAT)pInstance->Delta;
    LVM_FLOAT   Current = (LVM_FLOAT)pInstance->Current;
    LVM_FLOAT   Target= (LVM_FLOAT)pInstance->Target;
    LVM_FLOAT   Temp;

    /*
     * Same operation is performed on consecutive frames.
     * So two frames are processed in one iteration and
     * the loop will run only for half the NrFrames value times.
     */
    InLoop = (LVM_INT16)(NrFrames >> 1);
    /* OutLoop is calculated to handle cases where NrFrames value can be odd.*/
    OutLoop = (LVM_INT16)(NrFrames - (InLoop << 1));

    if (Current<Target) {
        if (OutLoop) {

            Temp = Current + Delta;
            if (Temp > 1.0f)
                Temp = 1.0f;
            else if (Temp < -1.0f)
                Temp = -1.0f;

            Current=Temp;
            if (Current > Target)
                Current = Target;

            for (ii = OutLoop; ii != 0; ii--) {
                for (jj = NrChannels; jj !=0; jj--) {
                    *(dst++) = (((LVM_FLOAT)*(src++) * (LVM_FLOAT)Current));
                }
            }
        }

        for (ii = InLoop; ii != 0; ii--) {

            Temp = Current + Delta;

            if (Temp > 1.0f)
                Temp = 1.0f;
            else if (Temp < -1.0f)
                Temp = -1.0f;

            Current=Temp;
            if (Current > Target)
                Current = Target;

            for (jj = NrChannels; jj != 0 ; jj--)
            {
                *(dst++) = (((LVM_FLOAT)*(src++) * Current));
                *(dst++) = (((LVM_FLOAT)*(src++) * Current));
            }
        }
    }
    else{
        if (OutLoop) {
            Current -= Delta;
            if (Current < Target)
                Current = Target;

            for (ii = OutLoop; ii != 0; ii--) {
                for (jj = NrChannels; jj !=0; jj--) {
                    *(dst++) = (((LVM_FLOAT)*(src++) * (LVM_FLOAT)Current));
                }
            }
        }

        for (ii = InLoop; ii != 0; ii--) {
            Current -= Delta;
            if (Current < Target)
                Current = Target;

            for (jj = NrChannels; jj != 0 ; jj--)
            {
                *(dst++) = (((LVM_FLOAT)*(src++) * Current));
                *(dst++) = (((LVM_FLOAT)*(src++) * Current));
            }
        }
    }
    pInstance->Current=Current;
}
#endif

#else
void LVC_Core_MixSoft_1St_D16C31_WRA( LVMixer3_st *ptrInstance,
                                    const LVM_INT16     *src,
                                          LVM_INT16     *dst,
                                          LVM_INT16     n)
{
    LVM_INT16   OutLoop;
    LVM_INT16   InLoop;
    LVM_INT16   CurrentShort;
    LVM_INT32   ii;
    Mix_Private_st  *pInstance=(Mix_Private_st *)(ptrInstance->PrivateParams);
    LVM_INT32   Delta=pInstance->Delta;
    LVM_INT32   Current=pInstance->Current;
    LVM_INT32   Target=pInstance->Target;
    LVM_INT32   Temp;

    InLoop = (LVM_INT16)(n >> 2); /* Process per 4 samples */
    OutLoop = (LVM_INT16)(n - (InLoop << 2));

    if(Current<Target){
        if (OutLoop){
            ADD2_SAT_32x32(Current,Delta,Temp);                                      /* Q31 + Q31 into Q31*/
            Current=Temp;
            if (Current > Target)
                Current = Target;

            CurrentShort = (LVM_INT16)(Current>>16);                                 /* From Q31 to Q15*/

            for (ii = OutLoop; ii != 0; ii--){
                *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShort)>>15);    /* Q15*Q15>>15 into Q15 */
            }
        }

        for (ii = InLoop; ii != 0; ii--){
            ADD2_SAT_32x32(Current,Delta,Temp);                                      /* Q31 + Q31 into Q31*/
            Current=Temp;
            if (Current > Target)
                Current = Target;

            CurrentShort = (LVM_INT16)(Current>>16);                                 /* From Q31 to Q15*/

            *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShort)>>15);    /* Q15*Q15>>15 into Q15 */
            *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShort)>>15);
            *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShort)>>15);
            *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShort)>>15);
        }
    }
    else{
        if (OutLoop){
            Current -= Delta;                                                        /* Q31 + Q31 into Q31*/
            if (Current < Target)
                Current = Target;

            CurrentShort = (LVM_INT16)(Current>>16);                                 /* From Q31 to Q15*/

            for (ii = OutLoop; ii != 0; ii--){
                *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShort)>>15);    /* Q15*Q15>>15 into Q15 */
            }
        }

        for (ii = InLoop; ii != 0; ii--){
            Current -= Delta;                                                        /* Q31 + Q31 into Q31*/
            if (Current < Target)
                Current = Target;

            CurrentShort = (LVM_INT16)(Current>>16);                                 /* From Q31 to Q15*/

            *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShort)>>15);    /* Q15*Q15>>15 into Q15 */
            *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShort)>>15);
            *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShort)>>15);
            *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShort)>>15);
        }
    }
    pInstance->Current=Current;
}
#endif
/**********************************************************************************/
