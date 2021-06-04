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
#include "ScalarArithmetic.h"
#include "LVM_Macros.h"

/**********************************************************************************
   FUNCTION LVC_Core_MixSoft_1St_2i_D16C31_WRA
***********************************************************************************/
#ifdef BUILD_FLOAT
static LVM_FLOAT ADD2_SAT_FLOAT(LVM_FLOAT a,
                                LVM_FLOAT b,
                                LVM_FLOAT c)
{
    LVM_FLOAT temp;
    temp = a + b ;
    if (temp < -1.0f)
        c = -1.0f;
    else if (temp > 1.0f)
        c = 1.0f;
    else
        c = temp;
    return c;
}
void LVC_Core_MixSoft_1St_2i_D16C31_WRA( LVMixer3_FLOAT_st        *ptrInstance1,
                                         LVMixer3_FLOAT_st        *ptrInstance2,
                                         const LVM_FLOAT    *src,
                                         LVM_FLOAT          *dst,
                                         LVM_INT16          n)
{
    LVM_INT16   OutLoop;
    LVM_INT16   InLoop;
    LVM_INT32   ii;
    Mix_Private_FLOAT_st  *pInstanceL = (Mix_Private_FLOAT_st *)(ptrInstance1->PrivateParams);
    Mix_Private_FLOAT_st  *pInstanceR = (Mix_Private_FLOAT_st *)(ptrInstance2->PrivateParams);

    LVM_FLOAT   DeltaL = pInstanceL->Delta;
    LVM_FLOAT   CurrentL = pInstanceL->Current;
    LVM_FLOAT   TargetL = pInstanceL->Target;

    LVM_FLOAT   DeltaR = pInstanceR->Delta;
    LVM_FLOAT   CurrentR = pInstanceR->Current;
    LVM_FLOAT   TargetR = pInstanceR->Target;

    LVM_FLOAT   Temp = 0;

    InLoop = (LVM_INT16)(n >> 2); /* Process per 4 samples */
    OutLoop = (LVM_INT16)(n - (InLoop << 2));

    if (OutLoop)
    {
        if(CurrentL < TargetL)
        {
            ADD2_SAT_FLOAT(CurrentL, DeltaL, Temp);
            CurrentL = Temp;
            if (CurrentL > TargetL)
                CurrentL = TargetL;
        }
        else
        {
            CurrentL -= DeltaL;
            if (CurrentL < TargetL)
                CurrentL = TargetL;
        }

        if(CurrentR < TargetR)
        {
            ADD2_SAT_FLOAT(CurrentR, DeltaR, Temp);
            CurrentR = Temp;
            if (CurrentR > TargetR)
                CurrentR = TargetR;
        }
        else
        {
            CurrentR -= DeltaR;
            if (CurrentR < TargetR)
                CurrentR = TargetR;
        }

        for (ii = OutLoop * 2; ii != 0; ii -= 2)
        {
            *(dst++) = (LVM_FLOAT)(((LVM_FLOAT)*(src++) * (LVM_FLOAT)CurrentL));
            *(dst++) = (LVM_FLOAT)(((LVM_FLOAT)*(src++) * (LVM_FLOAT)CurrentR));
        }
    }

    for (ii = InLoop * 2; ii != 0; ii-=2)
    {
        if(CurrentL < TargetL)
        {
            ADD2_SAT_FLOAT(CurrentL, DeltaL, Temp);
            CurrentL = Temp;
            if (CurrentL > TargetL)
                CurrentL = TargetL;
        }
        else
        {
            CurrentL -= DeltaL;
            if (CurrentL < TargetL)
                CurrentL = TargetL;
        }

        if(CurrentR < TargetR)
        {
            ADD2_SAT_FLOAT(CurrentR, DeltaR, Temp);
            CurrentR = Temp;
            if (CurrentR > TargetR)
                CurrentR = TargetR;
        }
        else
        {
            CurrentR -= DeltaR;
            if (CurrentR < TargetR)
                CurrentR = TargetR;
        }

        *(dst++) = (LVM_FLOAT)(((LVM_FLOAT)*(src++) * (LVM_FLOAT)CurrentL));
        *(dst++) = (LVM_FLOAT)(((LVM_FLOAT)*(src++) * (LVM_FLOAT)CurrentR));
        *(dst++) = (LVM_FLOAT)(((LVM_FLOAT)*(src++) * (LVM_FLOAT)CurrentL));
        *(dst++) = (LVM_FLOAT)(((LVM_FLOAT)*(src++) * (LVM_FLOAT)CurrentR));
        *(dst++) = (LVM_FLOAT)(((LVM_FLOAT)*(src++) * (LVM_FLOAT)CurrentL));
        *(dst++) = (LVM_FLOAT)(((LVM_FLOAT)*(src++) * (LVM_FLOAT)CurrentR));
        *(dst++) = (LVM_FLOAT)(((LVM_FLOAT)*(src++) * (LVM_FLOAT)CurrentL));
        *(dst++) = (LVM_FLOAT)(((LVM_FLOAT)*(src++) * (LVM_FLOAT)CurrentR));
    }
    pInstanceL->Current = CurrentL;
    pInstanceR->Current = CurrentR;

}
#ifdef SUPPORT_MC
void LVC_Core_MixSoft_1St_MC_float_WRA (Mix_Private_FLOAT_st **ptrInstance,
                                         const LVM_FLOAT      *src,
                                         LVM_FLOAT            *dst,
                                         LVM_INT16            NrFrames,
                                         LVM_INT16            NrChannels)
{
    LVM_INT32   ii, ch;
    LVM_FLOAT   Temp =0.0f;
    LVM_FLOAT   tempCurrent[NrChannels];
    for (ch = 0; ch < NrChannels; ch++)
    {
        tempCurrent[ch] = ptrInstance[ch]->Current;
    }
    for (ii = NrFrames; ii > 0; ii--)
    {
        for (ch = 0; ch < NrChannels; ch++)
        {
            Mix_Private_FLOAT_st *pInstance = ptrInstance[ch];
            const LVM_FLOAT   Delta = pInstance->Delta;
            LVM_FLOAT         Current = tempCurrent[ch];
            const LVM_FLOAT   Target = pInstance->Target;
            if (Current < Target)
            {
                ADD2_SAT_FLOAT(Current, Delta, Temp);
                Current = Temp;
                if (Current > Target)
                    Current = Target;
            }
            else
            {
                Current -= Delta;
                if (Current < Target)
                    Current = Target;
            }
            *dst++ = *src++ * Current;
            tempCurrent[ch] = Current;
        }
    }
    for (ch = 0; ch < NrChannels; ch++)
    {
        ptrInstance[ch]->Current = tempCurrent[ch];
    }
}
#endif
#else
void LVC_Core_MixSoft_1St_2i_D16C31_WRA( LVMixer3_st        *ptrInstance1,
                                         LVMixer3_st        *ptrInstance2,
                                         const LVM_INT16    *src,
                                         LVM_INT16          *dst,
                                         LVM_INT16          n)
{
    LVM_INT16   OutLoop;
    LVM_INT16   InLoop;
    LVM_INT16   CurrentShortL;
    LVM_INT16   CurrentShortR;
    LVM_INT32   ii;
    Mix_Private_st  *pInstanceL=(Mix_Private_st *)(ptrInstance1->PrivateParams);
    Mix_Private_st  *pInstanceR=(Mix_Private_st *)(ptrInstance2->PrivateParams);

    LVM_INT32   DeltaL=pInstanceL->Delta;
    LVM_INT32   CurrentL=pInstanceL->Current;
    LVM_INT32   TargetL=pInstanceL->Target;

    LVM_INT32   DeltaR=pInstanceR->Delta;
    LVM_INT32   CurrentR=pInstanceR->Current;
    LVM_INT32   TargetR=pInstanceR->Target;

    LVM_INT32   Temp;

    InLoop = (LVM_INT16)(n >> 2); /* Process per 4 samples */
    OutLoop = (LVM_INT16)(n - (InLoop << 2));

    if (OutLoop)
    {
        if(CurrentL<TargetL)
        {
            ADD2_SAT_32x32(CurrentL,DeltaL,Temp);                                      /* Q31 + Q31 into Q31*/
            CurrentL=Temp;
            if (CurrentL > TargetL)
                CurrentL = TargetL;
        }
        else
        {
            CurrentL -= DeltaL;                                                        /* Q31 + Q31 into Q31*/
            if (CurrentL < TargetL)
                CurrentL = TargetL;
        }

        if(CurrentR<TargetR)
        {
            ADD2_SAT_32x32(CurrentR,DeltaR,Temp);                                      /* Q31 + Q31 into Q31*/
            CurrentR=Temp;
            if (CurrentR > TargetR)
                CurrentR = TargetR;
        }
        else
        {
            CurrentR -= DeltaR;                                                        /* Q31 + Q31 into Q31*/
            if (CurrentR < TargetR)
                CurrentR = TargetR;
        }

        CurrentShortL = (LVM_INT16)(CurrentL>>16);                                 /* From Q31 to Q15*/
        CurrentShortR = (LVM_INT16)(CurrentR>>16);                                 /* From Q31 to Q15*/

        for (ii = OutLoop*2; ii != 0; ii-=2)
        {
            *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShortL)>>15);    /* Q15*Q15>>15 into Q15 */
            *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShortR)>>15);    /* Q15*Q15>>15 into Q15 */
        }
    }

    for (ii = InLoop*2; ii != 0; ii-=2)
    {
        if(CurrentL<TargetL)
        {
            ADD2_SAT_32x32(CurrentL,DeltaL,Temp);                                      /* Q31 + Q31 into Q31*/
            CurrentL=Temp;
            if (CurrentL > TargetL)
                CurrentL = TargetL;
        }
        else
        {
            CurrentL -= DeltaL;                                                        /* Q31 + Q31 into Q31*/
            if (CurrentL < TargetL)
                CurrentL = TargetL;
        }

        if(CurrentR<TargetR)
        {
            ADD2_SAT_32x32(CurrentR,DeltaR,Temp);                                      /* Q31 + Q31 into Q31*/
            CurrentR=Temp;
            if (CurrentR > TargetR)
                CurrentR = TargetR;
        }
        else
        {
            CurrentR -= DeltaR;                                                        /* Q31 + Q31 into Q31*/
            if (CurrentR < TargetR)
                CurrentR = TargetR;
        }

        CurrentShortL = (LVM_INT16)(CurrentL>>16);                                 /* From Q31 to Q15*/
        CurrentShortR = (LVM_INT16)(CurrentR>>16);                                 /* From Q31 to Q15*/

        *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShortL)>>15);    /* Q15*Q15>>15 into Q15 */
        *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShortR)>>15);    /* Q15*Q15>>15 into Q15 */
        *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShortL)>>15);
        *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShortR)>>15);
        *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShortL)>>15);
        *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShortR)>>15);
        *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShortL)>>15);
        *(dst++) = (LVM_INT16)(((LVM_INT32)*(src++) * (LVM_INT32)CurrentShortR)>>15);
    }
    pInstanceL->Current=CurrentL;
    pInstanceR->Current=CurrentR;

}
#endif
/**********************************************************************************/
