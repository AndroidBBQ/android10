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
#include <system/audio.h>

#include "LVM_Private.h"
#include "VectorArithmetic.h"
#include "LVM_Coeffs.h"

/****************************************************************************************/
/*                                                                                      */
/* FUNCTION:                LVM_Process                                                 */
/*                                                                                      */
/* DESCRIPTION:                                                                         */
/*  Process function for the LifeVibes module.                                          */
/*                                                                                      */
/* PARAMETERS:                                                                          */
/*  hInstance               Instance handle                                             */
/*  pInData                 Pointer to the input data                                   */
/*  pOutData                Pointer to the output data                                  */
/*  NumSamples              Number of samples in the input buffer                       */
/*  AudioTime               Audio Time of the current input buffer in ms                */
/*                                                                                      */
/* RETURNS:                                                                             */
/*  LVM_SUCCESS            Succeeded                                                    */
/*  LVM_INVALIDNUMSAMPLES  When the NumSamples is not a valied multiple in unmanaged    */
/*                         buffer mode                                                  */
/*  LVM_ALIGNMENTERROR     When either the input our output buffers are not 32-bit      */
/*                         aligned in unmanaged mode                                    */
/*  LVM_NULLADDRESS        When one of hInstance, pInData or pOutData is NULL           */
/*                                                                                      */
/* NOTES:                                                                               */
/*                                                                                      */
/****************************************************************************************/
#ifdef BUILD_FLOAT
LVM_ReturnStatus_en LVM_Process(LVM_Handle_t                hInstance,
                                const LVM_FLOAT             *pInData,
                                LVM_FLOAT                   *pOutData,
                                LVM_UINT16                  NumSamples,
                                LVM_UINT32                  AudioTime)
{

    LVM_Instance_t      *pInstance  = (LVM_Instance_t  *)hInstance;
    LVM_UINT16          SampleCount = NumSamples;
    LVM_FLOAT           *pInput     = (LVM_FLOAT *)pInData;
    LVM_FLOAT           *pToProcess = (LVM_FLOAT *)pInData;
    LVM_FLOAT           *pProcessed = pOutData;
    LVM_ReturnStatus_en  Status;
#ifdef SUPPORT_MC
    LVM_INT32           NrChannels  = pInstance->NrChannels;
    LVM_INT32           ChMask      = pInstance->ChMask;
#define NrFrames SampleCount  // alias for clarity
#endif

    /*
     * Check if the number of samples is zero
     */
    if (NumSamples == 0)
    {
        return(LVM_SUCCESS);
    }


    /*
     * Check valid points have been given
     */
    if ((hInstance == LVM_NULL) || (pInData == LVM_NULL) || (pOutData == LVM_NULL))
    {
        return (LVM_NULLADDRESS);
    }

    /*
     * For unmanaged mode only
     */
    if(pInstance->InstParams.BufferMode == LVM_UNMANAGED_BUFFERS)
    {
         /*
         * Check if the number of samples is a good multiple (unmanaged mode only)
         */
        if((NumSamples % pInstance->BlickSizeMultiple) != 0)
        {
            return(LVM_INVALIDNUMSAMPLES);
        }

        /*
         * Check the buffer alignment
         */
        if((((uintptr_t)pInData % 4) != 0) || (((uintptr_t)pOutData % 4) != 0))
        {
            return(LVM_ALIGNMENTERROR);
        }
    }


    /*
     * Update new parameters if necessary
     */
    if (pInstance->ControlPending == LVM_TRUE)
    {
        Status = LVM_ApplyNewSettings(hInstance);
#ifdef SUPPORT_MC
        /* Update the local variable NrChannels from pInstance->NrChannels value */
        NrChannels = pInstance->NrChannels;
        ChMask     = pInstance->ChMask;
#endif

        if(Status != LVM_SUCCESS)
        {
            return Status;
        }
    }


    /*
     * Convert from Mono if necessary
     */
    if (pInstance->Params.SourceFormat == LVM_MONO)
    {
        MonoTo2I_Float(pInData,                                /* Source */
                       pOutData,                               /* Destination */
                       (LVM_INT16)NumSamples);                 /* Number of input samples */
        pInput     = pOutData;
        pToProcess = pOutData;
#ifdef SUPPORT_MC
        NrChannels = 2;
        ChMask     = AUDIO_CHANNEL_OUT_STEREO;
#endif
    }


    /*
     * Process the data with managed buffers
     */
    while (SampleCount != 0)
    {
        /*
         * Manage the input buffer and frame processing
         */
        LVM_BufferIn(hInstance,
                     pInput,
                     &pToProcess,
                     &pProcessed,
                     &SampleCount);

        /*
         * Only process data when SampleCount is none zero, a zero count can occur when
         * the BufferIn routine is working in managed mode.
         */
        if (SampleCount != 0)
        {
            /*
             * Apply ConcertSound if required
             */
            if (pInstance->CS_Active == LVM_TRUE)
            {
                (void)LVCS_Process(pInstance->hCSInstance,     /* Concert Sound instance handle */
                                   pToProcess,
                                   pProcessed,
                                   SampleCount);
                pToProcess = pProcessed;
            }

            /*
             * Apply volume if required
             */
            if (pInstance->VC_Active!=0)
            {
#ifdef SUPPORT_MC
                LVC_MixSoft_Mc_D16C31_SAT(&pInstance->VC_Volume,
                                       pToProcess,
                                       pProcessed,
                                       (LVM_INT16)(NrFrames),
                                       NrChannels);
#else
                LVC_MixSoft_1St_D16C31_SAT(&pInstance->VC_Volume,
                                       pToProcess,
                                       pProcessed,
                                       (LVM_INT16)(2 * SampleCount));     /* Left and right*/
#endif
                pToProcess = pProcessed;
            }

            /*
             * Call N-Band equaliser if enabled
             */
            if (pInstance->EQNB_Active == LVM_TRUE)
            {
                LVEQNB_Process(pInstance->hEQNBInstance,    /* N-Band equaliser instance handle */
                               pToProcess,
                               pProcessed,
                               SampleCount);
                pToProcess = pProcessed;
            }

            /*
             * Call bass enhancement if enabled
             */
            if (pInstance->DBE_Active == LVM_TRUE)
            {
                LVDBE_Process(pInstance->hDBEInstance,       /* Dynamic Bass Enhancement \
                                                                instance handle */
                              pToProcess,
                              pProcessed,
                              SampleCount);
                pToProcess = pProcessed;
            }

            /*
             * Bypass mode or everything off, so copy the input to the output
             */
            if (pToProcess != pProcessed)
            {
#ifdef SUPPORT_MC
                Copy_Float(pToProcess,                             /* Source */
                           pProcessed,                             /* Destination */
                           (LVM_INT16)(NrChannels * NrFrames));    /* Copy all samples */
#else
                Copy_Float(pToProcess,                             /* Source */
                           pProcessed,                             /* Destination */
                           (LVM_INT16)(2 * SampleCount));          /* Left and right */
#endif
            }

            /*
             * Apply treble boost if required
             */
            if (pInstance->TE_Active == LVM_TRUE)
            {
                /*
                 * Apply the filter
                 */
#ifdef SUPPORT_MC
                FO_Mc_D16F32C15_LShx_TRC_WRA_01(&pInstance->pTE_State->TrebleBoost_State,
                                           pProcessed,
                                           pProcessed,
                                           (LVM_INT16)NrFrames,
                                           (LVM_INT16)NrChannels);
#else
                FO_2I_D16F32C15_LShx_TRC_WRA_01(&pInstance->pTE_State->TrebleBoost_State,
                                           pProcessed,
                                           pProcessed,
                                           (LVM_INT16)SampleCount);
#endif

            }
#ifdef SUPPORT_MC
            /*
             * Volume balance
             */
            LVC_MixSoft_1St_MC_float_SAT(&pInstance->VC_BalanceMix,
                                          pProcessed,
                                          pProcessed,
                                          NrFrames,
                                          NrChannels,
                                          ChMask);
#else
            /*
             * Volume balance
             */
            LVC_MixSoft_1St_2i_D16C31_SAT(&pInstance->VC_BalanceMix,
                                          pProcessed,
                                          pProcessed,
                                          SampleCount);
#endif

            /*
             * Perform Parametric Spectum Analysis
             */
            if ((pInstance->Params.PSA_Enable == LVM_PSA_ON) &&
                                            (pInstance->InstParams.PSA_Included == LVM_PSA_ON))
            {
#ifdef SUPPORT_MC
                FromMcToMono_Float(pProcessed,
                                   pInstance->pPSAInput,
                                   (LVM_INT16)(NrFrames),
                                   NrChannels);
#else
                From2iToMono_Float(pProcessed,
                                   pInstance->pPSAInput,
                                   (LVM_INT16)(SampleCount));
#endif

                LVPSA_Process(pInstance->hPSAInstance,
                        pInstance->pPSAInput,
                        (LVM_UINT16)(SampleCount),
                        AudioTime);
            }

            /*
             * DC removal
             */
#ifdef SUPPORT_MC
            DC_Mc_D16_TRC_WRA_01(&pInstance->DC_RemovalInstance,
                                 pProcessed,
                                 pProcessed,
                                 (LVM_INT16)NrFrames,
                                 NrChannels);
#else
            DC_2I_D16_TRC_WRA_01(&pInstance->DC_RemovalInstance,
                                 pProcessed,
                                 pProcessed,
                                 (LVM_INT16)SampleCount);
#endif
        }
        /*
         * Manage the output buffer
         */
        LVM_BufferOut(hInstance,
                      pOutData,
                      &SampleCount);

    }

    return(LVM_SUCCESS);
}
#else
LVM_ReturnStatus_en LVM_Process(LVM_Handle_t                hInstance,
                                const LVM_INT16             *pInData,
                                LVM_INT16                   *pOutData,
                                LVM_UINT16                  NumSamples,
                                LVM_UINT32                  AudioTime)
{

    LVM_Instance_t      *pInstance  = (LVM_Instance_t  *)hInstance;
    LVM_UINT16          SampleCount = NumSamples;
    LVM_INT16           *pInput     = (LVM_INT16 *)pInData;
    LVM_INT16           *pToProcess = (LVM_INT16 *)pInData;
    LVM_INT16           *pProcessed = pOutData;
    LVM_ReturnStatus_en  Status;

    /*
     * Check if the number of samples is zero
     */
    if (NumSamples == 0)
    {
        return(LVM_SUCCESS);
    }


    /*
     * Check valid points have been given
     */
    if ((hInstance == LVM_NULL) || (pInData == LVM_NULL) || (pOutData == LVM_NULL))
    {
        return (LVM_NULLADDRESS);
    }

    /*
     * For unmanaged mode only
     */
    if(pInstance->InstParams.BufferMode == LVM_UNMANAGED_BUFFERS)
    {
         /*
         * Check if the number of samples is a good multiple (unmanaged mode only)
         */
        if((NumSamples % pInstance->BlickSizeMultiple) != 0)
        {
            return(LVM_INVALIDNUMSAMPLES);
        }

        /*
         * Check the buffer alignment
         */
        if((((uintptr_t)pInData % 4) != 0) || (((uintptr_t)pOutData % 4) != 0))
        {
            return(LVM_ALIGNMENTERROR);
        }
    }


    /*
     * Update new parameters if necessary
     */
    if (pInstance->ControlPending == LVM_TRUE)
    {
        Status = LVM_ApplyNewSettings(hInstance);

        if(Status != LVM_SUCCESS)
        {
            return Status;
        }
    }


    /*
     * Convert from Mono if necessary
     */
    if (pInstance->Params.SourceFormat == LVM_MONO)
    {
        MonoTo2I_16(pInData,                                /* Source */
                    pOutData,                               /* Destination */
                    (LVM_INT16)NumSamples);                 /* Number of input samples */
        pInput     = pOutData;
        pToProcess = pOutData;
    }


    /*
     * Process the data with managed buffers
     */
    while (SampleCount != 0)
    {
        /*
         * Manage the input buffer and frame processing
         */
        LVM_BufferIn(hInstance,
                     pInput,
                     &pToProcess,
                     &pProcessed,
                     &SampleCount);

        /*
         * Only process data when SampleCount is none zero, a zero count can occur when
         * the BufferIn routine is working in managed mode.
         */
        if (SampleCount != 0)
        {

            /*
             * Apply ConcertSound if required
             */
            if (pInstance->CS_Active == LVM_TRUE)
            {
                (void)LVCS_Process(pInstance->hCSInstance,          /* Concert Sound instance handle */
                                   pToProcess,
                                   pProcessed,
                                   SampleCount);
                pToProcess = pProcessed;
            }

            /*
             * Apply volume if required
             */
            if (pInstance->VC_Active!=0)
            {
                LVC_MixSoft_1St_D16C31_SAT(&pInstance->VC_Volume,
                                       pToProcess,
                                       pProcessed,
                                       (LVM_INT16)(2*SampleCount));     /* Left and right*/
                pToProcess = pProcessed;
            }

            /*
             * Call N-Band equaliser if enabled
             */
            if (pInstance->EQNB_Active == LVM_TRUE)
            {
                LVEQNB_Process(pInstance->hEQNBInstance,        /* N-Band equaliser instance handle */
                               pToProcess,
                               pProcessed,
                               SampleCount);
                pToProcess = pProcessed;
            }

            /*
             * Call bass enhancement if enabled
             */
            if (pInstance->DBE_Active == LVM_TRUE)
            {
                LVDBE_Process(pInstance->hDBEInstance,          /* Dynamic Bass Enhancement instance handle */
                              pToProcess,
                              pProcessed,
                              SampleCount);
                pToProcess = pProcessed;
            }

            /*
             * Bypass mode or everything off, so copy the input to the output
             */
            if (pToProcess != pProcessed)
            {
                Copy_16(pToProcess,                             /* Source */
                        pProcessed,                             /* Destination */
                        (LVM_INT16)(2*SampleCount));            /* Left and right */
            }

            /*
             * Apply treble boost if required
             */
            if (pInstance->TE_Active == LVM_TRUE)
            {
                /*
                 * Apply the filter
                 */
                FO_2I_D16F32C15_LShx_TRC_WRA_01(&pInstance->pTE_State->TrebleBoost_State,
                                           pProcessed,
                                           pProcessed,
                                           (LVM_INT16)SampleCount);

            }

            /*
             * Volume balance
             */
            LVC_MixSoft_1St_2i_D16C31_SAT(&pInstance->VC_BalanceMix,
                                            pProcessed,
                                            pProcessed,
                                            SampleCount);

            /*
             * Perform Parametric Spectum Analysis
             */
            if ((pInstance->Params.PSA_Enable == LVM_PSA_ON)&&(pInstance->InstParams.PSA_Included==LVM_PSA_ON))
            {
                    From2iToMono_16(pProcessed,
                             pInstance->pPSAInput,
                            (LVM_INT16) (SampleCount));

                    LVPSA_Process(pInstance->hPSAInstance,
                            pInstance->pPSAInput,
                            (LVM_UINT16) (SampleCount),
                            AudioTime);
            }


            /*
             * DC removal
             */
            DC_2I_D16_TRC_WRA_01(&pInstance->DC_RemovalInstance,
                                 pProcessed,
                                 pProcessed,
                                 (LVM_INT16)SampleCount);


        }

        /*
         * Manage the output buffer
         */
        LVM_BufferOut(hInstance,
                      pOutData,
                      &SampleCount);

    }

    return(LVM_SUCCESS);
}
#endif
