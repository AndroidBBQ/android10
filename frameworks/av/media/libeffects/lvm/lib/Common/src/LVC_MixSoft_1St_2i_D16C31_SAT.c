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

#include <system/audio.h>

#include "LVC_Mixer_Private.h"
#include "VectorArithmetic.h"
#include "ScalarArithmetic.h"

/**********************************************************************************
   DEFINITIONS
***********************************************************************************/

#define TRUE          1
#define FALSE         0

#define ARRAY_SIZE(a) ((sizeof(a)) / (sizeof(*(a))))

/**********************************************************************************
   FUNCTION LVC_MixSoft_1St_2i_D16C31_SAT
***********************************************************************************/
#ifdef BUILD_FLOAT
#ifdef SUPPORT_MC
/* This threshold is used to decide on the processing to be applied on
 * front center and back center channels
 */
#define LVM_VOL_BAL_THR (0.000016f)
void LVC_MixSoft_1St_MC_float_SAT (LVMixer3_2St_FLOAT_st *ptrInstance,
                                    const LVM_FLOAT       *src,
                                    LVM_FLOAT             *dst,
                                    LVM_INT16             NrFrames,
                                    LVM_INT32             NrChannels,
                                    LVM_INT32             ChMask)
{
    char        HardMixing = TRUE;
    LVM_FLOAT   TargetGain;
    Mix_Private_FLOAT_st  Target_lfe = {LVM_MAXFLOAT, LVM_MAXFLOAT, LVM_MAXFLOAT};
    Mix_Private_FLOAT_st  Target_ctr = {LVM_MAXFLOAT, LVM_MAXFLOAT, LVM_MAXFLOAT};
    Mix_Private_FLOAT_st  *pInstance1 = \
                              (Mix_Private_FLOAT_st *)(ptrInstance->MixerStream[0].PrivateParams);
    Mix_Private_FLOAT_st  *pInstance2 = \
                              (Mix_Private_FLOAT_st *)(ptrInstance->MixerStream[1].PrivateParams);
    Mix_Private_FLOAT_st  *pMixPrivInst[4] = {pInstance1, pInstance2, &Target_ctr, &Target_lfe};
    Mix_Private_FLOAT_st  *pInstance[NrChannels];

    if (audio_channel_mask_get_representation(ChMask)
            == AUDIO_CHANNEL_REPRESENTATION_INDEX)
    {
        for (int i = 0; i < 2; i++)
        {
            pInstance[i] = pMixPrivInst[i];
        }
        for (int i = 2; i < NrChannels; i++)
        {
            pInstance[i] = pMixPrivInst[2];
        }
    }
    else
    {
        // TODO: Combine with system/media/audio_utils/Balance.cpp
        // Constants in system/media/audio/include/system/audio-base.h
        // 'mixInstIdx' is used to map the appropriate mixer instance for each channel.
        const int mixInstIdx[] = {
            0, // AUDIO_CHANNEL_OUT_FRONT_LEFT            = 0x1u,
            1, // AUDIO_CHANNEL_OUT_FRONT_RIGHT           = 0x2u,
            2, // AUDIO_CHANNEL_OUT_FRONT_CENTER          = 0x4u,
            3, // AUDIO_CHANNEL_OUT_LOW_FREQUENCY         = 0x8u,
            0, // AUDIO_CHANNEL_OUT_BACK_LEFT             = 0x10u,
            1, // AUDIO_CHANNEL_OUT_BACK_RIGHT            = 0x20u,
            0, // AUDIO_CHANNEL_OUT_FRONT_LEFT_OF_CENTER  = 0x40u,
            1, // AUDIO_CHANNEL_OUT_FRONT_RIGHT_OF_CENTER = 0x80u,
            2, // AUDIO_CHANNEL_OUT_BACK_CENTER           = 0x100u,
            0, // AUDIO_CHANNEL_OUT_SIDE_LEFT             = 0x200u,
            1, // AUDIO_CHANNEL_OUT_SIDE_RIGHT            = 0x400u,
            2, // AUDIO_CHANNEL_OUT_TOP_CENTER            = 0x800u,
            0, // AUDIO_CHANNEL_OUT_TOP_FRONT_LEFT        = 0x1000u,
            2, // AUDIO_CHANNEL_OUT_TOP_FRONT_CENTER      = 0x2000u,
            1, // AUDIO_CHANNEL_OUT_TOP_FRONT_RIGHT       = 0x4000u,
            0, // AUDIO_CHANNEL_OUT_TOP_BACK_LEFT         = 0x8000u,
            2, // AUDIO_CHANNEL_OUT_TOP_BACK_CENTER       = 0x10000u,
            1, // AUDIO_CHANNEL_OUT_TOP_BACK_RIGHT        = 0x20000u,
            0, // AUDIO_CHANNEL_OUT_TOP_SIDE_LEFT         = 0x40000u,
            1, // AUDIO_CHANNEL_OUT_TOP_SIDE_RIGHT        = 0x80000u
        };
        if (pInstance1->Target <= LVM_VOL_BAL_THR ||
            pInstance2->Target <= LVM_VOL_BAL_THR)
        {
            Target_ctr.Target  = 0.0f;
            Target_ctr.Current = 0.0f;
            Target_ctr.Delta   = 0.0f;
        }
        const unsigned int idxArrSize = ARRAY_SIZE(mixInstIdx);
        for (unsigned int i = 0, channel = ChMask; channel !=0 ; ++i)
        {
            const unsigned int idx = __builtin_ctz(channel);
            if (idx < idxArrSize)
            {
                pInstance[i] = pMixPrivInst[mixInstIdx[idx]];
            }
            else
            {
                pInstance[i] = pMixPrivInst[2];
            }
            channel &= ~(1 << idx);
        }
    }

    if (NrFrames <= 0)    return;

    /******************************************************************************
       SOFT MIXING
    *******************************************************************************/

    if ((pInstance1->Current != pInstance1->Target) ||
        (pInstance2->Current != pInstance2->Target))
    {
        // TODO: combine similar checks below.
        if (pInstance1->Delta == LVM_MAXFLOAT
                || Abs_Float(pInstance1->Current - pInstance1->Target) < pInstance1->Delta)
        {
            /* Difference is not significant anymore. Make them equal. */
            pInstance1->Current = pInstance1->Target;
            TargetGain = pInstance1->Target;
            LVC_Mixer_SetTarget(&(ptrInstance->MixerStream[0]), TargetGain);
        }
        else
        {
            /* Soft mixing has to be applied */
            HardMixing = FALSE;
        }

        if (HardMixing == TRUE)
        {
            if (pInstance2->Delta == LVM_MAXFLOAT
                    || Abs_Float(pInstance2->Current - pInstance2->Target) < pInstance2->Delta)
            {
                /* Difference is not significant anymore. Make them equal. */
                pInstance2->Current = pInstance2->Target;
                TargetGain = pInstance2->Target;
                LVC_Mixer_SetTarget(&(ptrInstance->MixerStream[1]), TargetGain);
            }
            else
            {
                /* Soft mixing has to be applied */
                HardMixing = FALSE;
            }
        }

        if (HardMixing == FALSE)
        {
             LVC_Core_MixSoft_1St_MC_float_WRA (&pInstance[0],
                                                 src, dst, NrFrames, NrChannels);
        }
    }

    /******************************************************************************
       HARD MIXING
    *******************************************************************************/

    if (HardMixing == TRUE)
    {
        if ((pInstance1->Target == LVM_MAXFLOAT) && (pInstance2->Target == LVM_MAXFLOAT))
        {
            if (src != dst)
            {
                Copy_Float(src, dst, NrFrames*NrChannels);
            }
        }
        else
        {
            LVC_Core_MixHard_1St_MC_float_SAT(&(pInstance[0]),
                                               src, dst, NrFrames, NrChannels);
        }
    }

    /******************************************************************************
       CALL BACK
    *******************************************************************************/

    if (ptrInstance->MixerStream[0].CallbackSet)
    {
        if (Abs_Float(pInstance1->Current - pInstance1->Target) < pInstance1->Delta)
        {
            pInstance1->Current = pInstance1->Target; /* Difference is not significant anymore. \
                                                         Make them equal. */
            TargetGain = pInstance1->Target;
            LVC_Mixer_SetTarget(&ptrInstance->MixerStream[0], TargetGain);
            ptrInstance->MixerStream[0].CallbackSet = FALSE;
            if (ptrInstance->MixerStream[0].pCallBack != 0)
            {
                (*ptrInstance->MixerStream[0].pCallBack) (\
                    ptrInstance->MixerStream[0].pCallbackHandle,
                    ptrInstance->MixerStream[0].pGeneralPurpose,
                    ptrInstance->MixerStream[0].CallbackParam);
            }
        }
    }
    if (ptrInstance->MixerStream[1].CallbackSet)
    {
        if (Abs_Float(pInstance2->Current - pInstance2->Target) < pInstance2->Delta)
        {
            pInstance2->Current = pInstance2->Target; /* Difference is not significant anymore.
                                                         Make them equal. */
            TargetGain = pInstance2->Target;
            LVC_Mixer_SetTarget(&ptrInstance->MixerStream[1], TargetGain);
            ptrInstance->MixerStream[1].CallbackSet = FALSE;
            if (ptrInstance->MixerStream[1].pCallBack != 0)
            {
                (*ptrInstance->MixerStream[1].pCallBack) (\
                    ptrInstance->MixerStream[1].pCallbackHandle,
                    ptrInstance->MixerStream[1].pGeneralPurpose,
                    ptrInstance->MixerStream[1].CallbackParam);
            }
        }
    }
}
#endif
void LVC_MixSoft_1St_2i_D16C31_SAT( LVMixer3_2St_FLOAT_st *ptrInstance,
                                    const LVM_FLOAT             *src,
                                    LVM_FLOAT             *dst,
                                    LVM_INT16             n)
{
    char        HardMixing = TRUE;
    LVM_FLOAT   TargetGain;
    Mix_Private_FLOAT_st  *pInstance1 = \
                              (Mix_Private_FLOAT_st *)(ptrInstance->MixerStream[0].PrivateParams);
    Mix_Private_FLOAT_st  *pInstance2 = \
                              (Mix_Private_FLOAT_st *)(ptrInstance->MixerStream[1].PrivateParams);

    if(n <= 0)    return;

    /******************************************************************************
       SOFT MIXING
    *******************************************************************************/
    if ((pInstance1->Current != pInstance1->Target) || (pInstance2->Current != pInstance2->Target))
    {
        if(pInstance1->Delta == 1.0f)
        {
            pInstance1->Current = pInstance1->Target;
            TargetGain = pInstance1->Target;
            LVC_Mixer_SetTarget(&(ptrInstance->MixerStream[0]), TargetGain);
        }
        else if (Abs_Float(pInstance1->Current - pInstance1->Target) < pInstance1->Delta)
        {
            pInstance1->Current = pInstance1->Target; /* Difference is not significant anymore. \
                                                         Make them equal. */
            TargetGain = pInstance1->Target;
            LVC_Mixer_SetTarget(&(ptrInstance->MixerStream[0]), TargetGain);
        }
        else
        {
            /* Soft mixing has to be applied */
            HardMixing = FALSE;
        }

        if(HardMixing == TRUE)
        {
            if(pInstance2->Delta == 1.0f)
            {
                pInstance2->Current = pInstance2->Target;
                TargetGain = pInstance2->Target;
                LVC_Mixer_SetTarget(&(ptrInstance->MixerStream[1]), TargetGain);
            }
            else if (Abs_Float(pInstance2->Current - pInstance2->Target) < pInstance2->Delta)
            {
                pInstance2->Current = pInstance2->Target; /* Difference is not significant anymore. \
                                                             Make them equal. */
                TargetGain = pInstance2->Target;
                LVC_Mixer_SetTarget(&(ptrInstance->MixerStream[1]), TargetGain);
            }
            else
            {
                /* Soft mixing has to be applied */
                HardMixing = FALSE;
            }
        }

        if(HardMixing == FALSE)
        {
             LVC_Core_MixSoft_1St_2i_D16C31_WRA( &(ptrInstance->MixerStream[0]),
                                                 &(ptrInstance->MixerStream[1]),
                                                 src, dst, n);
        }
    }

    /******************************************************************************
       HARD MIXING
    *******************************************************************************/

    if (HardMixing)
    {
        if ((pInstance1->Target == 1.0f) && (pInstance2->Target == 1.0f))
        {
            if(src != dst)
            {
                Copy_Float(src, dst, n);
            }
        }
        else
        {
            LVC_Core_MixHard_1St_2i_D16C31_SAT(&(ptrInstance->MixerStream[0]),
                                               &(ptrInstance->MixerStream[1]),
                                               src, dst, n);
        }
    }

    /******************************************************************************
       CALL BACK
    *******************************************************************************/

    if (ptrInstance->MixerStream[0].CallbackSet)
    {
        if (Abs_Float(pInstance1->Current - pInstance1->Target) < pInstance1->Delta)
        {
            pInstance1->Current = pInstance1->Target; /* Difference is not significant anymore. \
                                                         Make them equal. */
            TargetGain = pInstance1->Target;
            LVC_Mixer_SetTarget(&ptrInstance->MixerStream[0], TargetGain);
            ptrInstance->MixerStream[0].CallbackSet = FALSE;
            if (ptrInstance->MixerStream[0].pCallBack != 0)
            {
                (*ptrInstance->MixerStream[0].pCallBack) ( \
                                                ptrInstance->MixerStream[0].pCallbackHandle,
                                                ptrInstance->MixerStream[0].pGeneralPurpose,
                                                ptrInstance->MixerStream[0].CallbackParam );
            }
        }
    }
    if (ptrInstance->MixerStream[1].CallbackSet)
    {
        if (Abs_Float(pInstance2->Current - pInstance2->Target) < pInstance2->Delta)
        {
            pInstance2->Current = pInstance2->Target; /* Difference is not significant anymore.
                                                         Make them equal. */
            TargetGain = pInstance2->Target;
            LVC_Mixer_SetTarget(&ptrInstance->MixerStream[1], TargetGain);
            ptrInstance->MixerStream[1].CallbackSet = FALSE;
            if (ptrInstance->MixerStream[1].pCallBack != 0)
            {
                (*ptrInstance->MixerStream[1].pCallBack) (
                                                ptrInstance->MixerStream[1].pCallbackHandle,
                                                ptrInstance->MixerStream[1].pGeneralPurpose,
                                                ptrInstance->MixerStream[1].CallbackParam );
            }
        }
    }
}
#else
void LVC_MixSoft_1St_2i_D16C31_SAT( LVMixer3_2St_st *ptrInstance,
                                  const LVM_INT16             *src,
                                        LVM_INT16             *dst,
                                        LVM_INT16             n)
{
    char        HardMixing = TRUE;
    LVM_INT32   TargetGain;
    Mix_Private_st  *pInstance1=(Mix_Private_st *)(ptrInstance->MixerStream[0].PrivateParams);
    Mix_Private_st  *pInstance2=(Mix_Private_st *)(ptrInstance->MixerStream[1].PrivateParams);

    if(n<=0)    return;

    /******************************************************************************
       SOFT MIXING
    *******************************************************************************/
    if ((pInstance1->Current != pInstance1->Target)||(pInstance2->Current != pInstance2->Target))
    {
        if(pInstance1->Delta == 0x7FFFFFFF)
        {
            pInstance1->Current = pInstance1->Target;
            TargetGain=pInstance1->Target>>16;  // TargetGain in Q16.15 format, no integer part
            LVC_Mixer_SetTarget(&(ptrInstance->MixerStream[0]),TargetGain);
        }
        else if (Abs_32(pInstance1->Current-pInstance1->Target) < pInstance1->Delta)
        {
            pInstance1->Current = pInstance1->Target; /* Difference is not significant anymore.  Make them equal. */
            TargetGain=pInstance1->Target>>16;  // TargetGain in Q16.15 format, no integer part
            LVC_Mixer_SetTarget(&(ptrInstance->MixerStream[0]),TargetGain);
        }
        else
        {
            /* Soft mixing has to be applied */
            HardMixing = FALSE;
        }

        if(HardMixing == TRUE)
        {
            if(pInstance2->Delta == 0x7FFFFFFF)
            {
                pInstance2->Current = pInstance2->Target;
                TargetGain=pInstance2->Target>>16;  // TargetGain in Q16.15 format, no integer part
                LVC_Mixer_SetTarget(&(ptrInstance->MixerStream[1]),TargetGain);
            }
            else if (Abs_32(pInstance2->Current-pInstance2->Target) < pInstance2->Delta)
            {
                pInstance2->Current = pInstance2->Target; /* Difference is not significant anymore.  Make them equal. */
                TargetGain=pInstance2->Target>>16;  // TargetGain in Q16.15 format, no integer part
                LVC_Mixer_SetTarget(&(ptrInstance->MixerStream[1]),TargetGain);
            }
            else
            {
                /* Soft mixing has to be applied */
                HardMixing = FALSE;
            }
        }

        if(HardMixing == FALSE)
        {
             LVC_Core_MixSoft_1St_2i_D16C31_WRA( &(ptrInstance->MixerStream[0]),&(ptrInstance->MixerStream[1]), src, dst, n);
        }
    }

    /******************************************************************************
       HARD MIXING
    *******************************************************************************/

    if (HardMixing)
    {
        if (((pInstance1->Target>>16) == 0x7FFF)&&((pInstance2->Target>>16) == 0x7FFF))
        {
            if(src!=dst)
            {
                Copy_16(src, dst, n);
            }
        }
        else
        {
            LVC_Core_MixHard_1St_2i_D16C31_SAT(&(ptrInstance->MixerStream[0]),&(ptrInstance->MixerStream[1]), src, dst, n);
        }
    }

    /******************************************************************************
       CALL BACK
    *******************************************************************************/

    if (ptrInstance->MixerStream[0].CallbackSet)
    {
        if (Abs_32(pInstance1->Current-pInstance1->Target) < pInstance1->Delta)
        {
            pInstance1->Current = pInstance1->Target; /* Difference is not significant anymore.  Make them equal. */
            TargetGain=pInstance1->Target>>(16-pInstance1->Shift);  // TargetGain in Q16.15 format
            LVC_Mixer_SetTarget(&ptrInstance->MixerStream[0],TargetGain);
            ptrInstance->MixerStream[0].CallbackSet = FALSE;
            if (ptrInstance->MixerStream[0].pCallBack != 0)
            {
                (*ptrInstance->MixerStream[0].pCallBack) ( ptrInstance->MixerStream[0].pCallbackHandle, ptrInstance->MixerStream[0].pGeneralPurpose,ptrInstance->MixerStream[0].CallbackParam );
            }
        }
    }
    if (ptrInstance->MixerStream[1].CallbackSet)
    {
        if (Abs_32(pInstance2->Current-pInstance2->Target) < pInstance2->Delta)
        {
            pInstance2->Current = pInstance2->Target; /* Difference is not significant anymore.  Make them equal. */
            TargetGain=pInstance2->Target>>(16-pInstance2->Shift);  // TargetGain in Q16.15 format
            LVC_Mixer_SetTarget(&ptrInstance->MixerStream[1],TargetGain);
            ptrInstance->MixerStream[1].CallbackSet = FALSE;
            if (ptrInstance->MixerStream[1].pCallBack != 0)
            {
                (*ptrInstance->MixerStream[1].pCallBack) ( ptrInstance->MixerStream[1].pCallbackHandle, ptrInstance->MixerStream[1].pGeneralPurpose,ptrInstance->MixerStream[1].CallbackParam );
            }
        }
    }
}
#endif
/**********************************************************************************/
