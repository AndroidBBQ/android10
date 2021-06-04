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


/************************************************************************************/
/*                                                                                  */
/*  Includes                                                                        */
/*                                                                                  */
/************************************************************************************/

#include "LVCS.h"
#include "LVCS_Private.h"
#include "VectorArithmetic.h"
#include "CompLim.h"

/************************************************************************************/
/*                                                                                  */
/* FUNCTION:                LVCS_Process_CS                                         */
/*                                                                                  */
/* DESCRIPTION:                                                                     */
/*  Process function for the Concert Sound module based on the following block      */
/*  diagram:                                                                        */
/*            _________    ________    _____    _______     ___   ______            */
/*           |         |  |        |  |     |  |       |   |   | |      |           */
/*     ----->| Stereo  |->| Reverb |->| Equ |->| Alpha |-->| + |-| Gain |---->      */
/*        |  | Enhance |  |________|  |_____|  |_______|   |___| |______|           */
/*        |  |_________|                                     |                      */
/*        |                                 ___________      |                      */
/*        |                                |           |     |                      */
/*        |------------------------------->| 1 - Alpha |-----|                      */
/*                                         |___________|                            */
/*                                                                                  */
/*  The Stereo Enhancer, Reverb and Equaliser blocks are each configured to have    */
/*  their gain to give a near peak to peak output (-0.1dBFS) with a worst case      */
/*  input signal. The gains of these blocks are re-combined in the Alpha mixer and  */
/*  the gain block folloing the sum.                                                */
/*                                                                                  */
/*  The processing uses the output buffer for data storage after each processing    */
/*  block. When processing is inplace a copy of the input signal is made in scratch */
/*  memory for the 1-Alpha path.                                                    */
/*                                                                                  */
/*                                                                                  */
/* PARAMETERS:                                                                      */
/*  hInstance               Instance handle                                         */
/*  pInData                 Pointer to the input data                               */
/*  pOutData                Pointer to the output data                              */
/*  NumSamples              Number of samples in the input buffer                   */
/*                                                                                  */
/* RETURNS:                                                                         */
/*  LVCS_Success            Succeeded                                               */
/*                                                                                  */
/* NOTES:                                                                           */
/*                                                                                  */
/************************************************************************************/
#ifdef BUILD_FLOAT
LVCS_ReturnStatus_en LVCS_Process_CS(LVCS_Handle_t              hInstance,
                                     const LVM_FLOAT            *pInData,
                                     LVM_FLOAT                  *pOutData,
                                     LVM_UINT16                 NumSamples)
{
    const LVM_FLOAT     *pInput;
    LVCS_Instance_t     *pInstance = (LVCS_Instance_t  *)hInstance;
    LVM_FLOAT           *pScratch;
    LVCS_ReturnStatus_en err;
#ifdef SUPPORT_MC
    LVM_FLOAT           *pStIn;
    LVM_INT32           channels = pInstance->Params.NrChannels;
#define NrFrames NumSamples  // alias for clarity

    /*In case of mono processing, stereo input is created from mono
     *and stored in pInData before applying any of the effects.
     *However we do not update the value pInstance->Params.NrChannels
     *at this point.
     *So to treat the pInData as stereo we are setting channels to 2
     */
    if (channels == 1)
    {
        channels = 2;
    }
#endif

    pScratch  = (LVM_FLOAT *) \
                  pInstance->MemoryTable.Region[LVCS_MEMREGION_TEMPORARY_FAST].pBaseAddress;

    /*
     * Check if the processing is inplace
     */
#ifdef SUPPORT_MC
    /*
     * The pInput buffer holds the first 2 (Left, Right) channels information.
     * Hence the memory required by this buffer is 2 * NumFrames.
     * The Concert Surround module carries out processing only on L, R.
     */
    pInput = pScratch + (2 * NrFrames);
    pStIn  = pScratch + (LVCS_SCRATCHBUFFERS * NrFrames);
    /* The first two channel data is extracted from the input data and
     * copied into pInput buffer
     */
    Copy_Float_Mc_Stereo((LVM_FLOAT *)pInData,
                         (LVM_FLOAT *)pInput,
                         NrFrames,
                         channels);
    Copy_Float((LVM_FLOAT *)pInput,
               (LVM_FLOAT *)pStIn,
               (LVM_INT16)(2 * NrFrames));
#else
    if (pInData == pOutData)
    {
        /* Processing inplace */
        pInput = pScratch + (2 * NumSamples);
        Copy_Float((LVM_FLOAT *)pInData,           /* Source */
                   (LVM_FLOAT *)pInput,            /* Destination */
                   (LVM_INT16)(2 * NumSamples));     /* Left and right */
    }
    else
    {
        /* Processing outplace */
        pInput = pInData;
    }
#endif
    /*
     * Call the stereo enhancer
     */
#ifdef SUPPORT_MC
    err = LVCS_StereoEnhancer(hInstance,              /* Instance handle */
                              pStIn,                  /* Pointer to the input data */
                              pOutData,               /* Pointer to the output data */
                              NrFrames);              /* Number of frames to process */
#else
    err = LVCS_StereoEnhancer(hInstance,              /* Instance handle */
                              pInData,                    /* Pointer to the input data */
                              pOutData,                   /* Pointer to the output data */
                              NumSamples);                /* Number of samples to process */
#endif

    /*
     * Call the reverb generator
     */
    err = LVCS_ReverbGenerator(hInstance,             /* Instance handle */
                               pOutData,                  /* Pointer to the input data */
                               pOutData,                  /* Pointer to the output data */
                               NumSamples);               /* Number of samples to process */

    /*
     * Call the equaliser
     */
    err = LVCS_Equaliser(hInstance,                   /* Instance handle */
                         pOutData,                        /* Pointer to the input data */
                         NumSamples);                     /* Number of samples to process */

    /*
     * Call the bypass mixer
     */
    err = LVCS_BypassMixer(hInstance,                 /* Instance handle */
                           pOutData,                      /* Pointer to the processed data */
                           pInput,                        /* Pointer to the input (unprocessed) data */
                           pOutData,                      /* Pointer to the output data */
                           NumSamples);                   /* Number of samples to process */

    if(err != LVCS_SUCCESS)
    {
        return err;
    }

    return(LVCS_SUCCESS);
}
#else
LVCS_ReturnStatus_en LVCS_Process_CS(LVCS_Handle_t              hInstance,
                                     const LVM_INT16            *pInData,
                                     LVM_INT16                  *pOutData,
                                     LVM_UINT16                 NumSamples)
{
    const LVM_INT16     *pInput;
    LVCS_Instance_t     *pInstance = (LVCS_Instance_t  *)hInstance;
    LVM_INT16           *pScratch  = (LVM_INT16 *)pInstance->MemoryTable.Region[LVCS_MEMREGION_TEMPORARY_FAST].pBaseAddress;
    LVCS_ReturnStatus_en err;

    /*
     * Check if the processing is inplace
     */
    if (pInData == pOutData)
    {
        /* Processing inplace */
        pInput = pScratch + (2*NumSamples);
        Copy_16((LVM_INT16 *)pInData,           /* Source */
                (LVM_INT16 *)pInput,            /* Destination */
                (LVM_INT16)(2*NumSamples));     /* Left and right */
    }
    else
    {
        /* Processing outplace */
        pInput = pInData;
    }

    /*
     * Call the stereo enhancer
     */
    err=LVCS_StereoEnhancer(hInstance,              /* Instance handle */
                        pInData,                    /* Pointer to the input data */
                        pOutData,                   /* Pointer to the output data */
                        NumSamples);                /* Number of samples to process */

    /*
     * Call the reverb generator
     */
    err=LVCS_ReverbGenerator(hInstance,             /* Instance handle */
                         pOutData,                  /* Pointer to the input data */
                         pOutData,                  /* Pointer to the output data */
                         NumSamples);               /* Number of samples to process */

    /*
     * Call the equaliser
     */
    err=LVCS_Equaliser(hInstance,                   /* Instance handle */
                   pOutData,                        /* Pointer to the input data */
                   NumSamples);                     /* Number of samples to process */

    /*
     * Call the bypass mixer
     */
    err=LVCS_BypassMixer(hInstance,                 /* Instance handle */
                     pOutData,                      /* Pointer to the processed data */
                     pInput,                        /* Pointer to the input (unprocessed) data */
                     pOutData,                      /* Pointer to the output data */
                     NumSamples);                   /* Number of samples to process */

    if(err !=LVCS_SUCCESS)
    {
        return err;
    }

    return(LVCS_SUCCESS);
}
#endif
/************************************************************************************/
/*                                                                                  */
/* FUNCTION:                LVCS_Process                                            */
/*                                                                                  */
/* DESCRIPTION:                                                                     */
/*  Process function for the Concert Sound module. The implementation supports two  */
/*  variants of the algorithm, one for headphones and one for mobile speakers.      */
/*                                                                                  */
/*  Data can be processed in two formats, stereo or mono-in-stereo. Data in mono    */
/*  format is not supported, the calling routine must convert the mono stream to    */
/*  mono-in-stereo.                                                                 */
/*                                                                                  */
/*                                                                                  */
/* PARAMETERS:                                                                      */
/*  hInstance               Instance handle                                         */
/*  pInData                 Pointer to the input data                               */
/*  pOutData                Pointer to the output data                              */
/*  NumSamples              Number of samples in the input buffer                   */
/*                                                                                  */
/* RETURNS:                                                                         */
/*  LVCS_Success            Succeeded                                               */
/*  LVCS_TooManySamples     NumSamples was larger than the maximum block size       */
/*                                                                                  */
/* NOTES:                                                                           */
/*                                                                                  */
/************************************************************************************/
#ifdef BUILD_FLOAT
LVCS_ReturnStatus_en LVCS_Process(LVCS_Handle_t             hInstance,
                                  const LVM_FLOAT           *pInData,
                                  LVM_FLOAT                 *pOutData,
                                  LVM_UINT16                NumSamples)
{

    LVCS_Instance_t *pInstance = (LVCS_Instance_t  *)hInstance;
    LVCS_ReturnStatus_en err;
#ifdef SUPPORT_MC
    /*Extract number of Channels info*/
    LVM_INT32 channels = pInstance->Params.NrChannels;
#define NrFrames NumSamples  // alias for clarity
    if (channels == 1)
    {
        channels = 2;
    }
#endif
    /*
     * Check the number of samples is not too large
     */
    if (NumSamples > pInstance->Capabilities.MaxBlockSize)
    {
        return(LVCS_TOOMANYSAMPLES);
    }

    /*
     * Check if the algorithm is enabled
     */
    if (pInstance->Params.OperatingMode != LVCS_OFF)
    {
        /*
         * Call CS process function
         */
            err = LVCS_Process_CS(hInstance,
                                  pInData,
                                  pOutData,
                                  NumSamples);


        /*
         * Compress to reduce expansion effect of Concert Sound and correct volume
         * differences for difference settings. Not applied in test modes
         */
        if ((pInstance->Params.OperatingMode == LVCS_ON)&& \
                                        (pInstance->Params.CompressorMode == LVM_MODE_ON))
        {
            LVM_FLOAT Gain = pInstance->VolCorrect.CompMin;
            LVM_FLOAT Current1;

            Current1 = LVC_Mixer_GetCurrent(&pInstance->BypassMix.Mixer_Instance.MixerStream[0]);
            Gain = (LVM_FLOAT)(  pInstance->VolCorrect.CompMin
                               - (((LVM_FLOAT)pInstance->VolCorrect.CompMin  * (Current1)))
                               + (((LVM_FLOAT)pInstance->VolCorrect.CompFull * (Current1))));

            if(NumSamples < LVCS_COMPGAINFRAME)
            {
                NonLinComp_Float(Gain,                    /* Compressor gain setting */
                                 pOutData,
                                 pOutData,
                                 (LVM_INT32)(2 * NumSamples));
            }
            else
            {
                LVM_FLOAT  GainStep;
                LVM_FLOAT  FinalGain;
                LVM_INT16  SampleToProcess = NumSamples;
                LVM_FLOAT  *pOutPtr;

                /* Large changes in Gain can cause clicks in output
                   Split data into small blocks and use interpolated gain values */

                GainStep = (LVM_FLOAT)(((Gain-pInstance->CompressGain) * \
                                                LVCS_COMPGAINFRAME) / NumSamples);

                if((GainStep == 0) && (pInstance->CompressGain < Gain))
                {
                    GainStep = 1;
                }
                else
                {
                    if((GainStep == 0) && (pInstance->CompressGain > Gain))
                    {
                        GainStep = -1;
                    }
                }

                FinalGain = Gain;
                Gain = pInstance->CompressGain;
                pOutPtr = pOutData;

                while(SampleToProcess > 0)
                {
                    Gain = (LVM_FLOAT)(Gain + GainStep);
                    if((GainStep > 0) && (FinalGain <= Gain))
                    {
                        Gain = FinalGain;
                        GainStep = 0;
                    }

                    if((GainStep < 0) && (FinalGain > Gain))
                    {
                        Gain = FinalGain;
                        GainStep = 0;
                    }

                    if(SampleToProcess > LVCS_COMPGAINFRAME)
                    {
                        NonLinComp_Float(Gain,                    /* Compressor gain setting */
                                         pOutPtr,
                                         pOutPtr,
                                         (LVM_INT32)(2 * LVCS_COMPGAINFRAME));
                        pOutPtr += (2 * LVCS_COMPGAINFRAME);
                        SampleToProcess = (LVM_INT16)(SampleToProcess - LVCS_COMPGAINFRAME);
                    }
                    else
                    {
                        NonLinComp_Float(Gain,                    /* Compressor gain setting */
                                         pOutPtr,
                                         pOutPtr,
                                         (LVM_INT32)(2 * SampleToProcess));
                        SampleToProcess = 0;
                    }

                }
            }

            /* Store gain value*/
            pInstance->CompressGain = Gain;
        }


        if(pInstance->bInOperatingModeTransition == LVM_TRUE){

            /*
             * Re-init bypass mix when timer has completed
             */
            if ((pInstance->bTimerDone == LVM_TRUE) &&
                (pInstance->BypassMix.Mixer_Instance.MixerStream[1].CallbackSet == 0))
            {
                err = LVCS_BypassMixInit(hInstance,
                                         &pInstance->Params);

                if(err != LVCS_SUCCESS)
                {
                    return err;
                }

            }
            else{
                LVM_Timer ( &pInstance->TimerInstance,
                            (LVM_INT16)NumSamples);
            }
        }
#ifdef SUPPORT_MC
        Copy_Float_Stereo_Mc(pInData,
                             pOutData,
                             NrFrames,
                             channels);
#endif
    }
    else
    {
        if (pInData != pOutData)
        {
#ifdef SUPPORT_MC
            /*
             * The algorithm is disabled so just copy the data
             */
            Copy_Float((LVM_FLOAT *)pInData,               /* Source */
                       (LVM_FLOAT *)pOutData,                  /* Destination */
                       (LVM_INT16)(channels * NrFrames));    /* All Channels*/
#else
            /*
             * The algorithm is disabled so just copy the data
             */
            Copy_Float((LVM_FLOAT *)pInData,               /* Source */
                       (LVM_FLOAT *)pOutData,                  /* Destination */
                       (LVM_INT16)(2 * NumSamples));             /* Left and right */
#endif
        }
    }


    return(LVCS_SUCCESS);
}
#else
LVCS_ReturnStatus_en LVCS_Process(LVCS_Handle_t             hInstance,
                                  const LVM_INT16           *pInData,
                                  LVM_INT16                 *pOutData,
                                  LVM_UINT16                NumSamples)
{

    LVCS_Instance_t *pInstance =(LVCS_Instance_t  *)hInstance;
    LVCS_ReturnStatus_en err;

    /*
     * Check the number of samples is not too large
     */
    if (NumSamples > pInstance->Capabilities.MaxBlockSize)
    {
        return(LVCS_TOOMANYSAMPLES);
    }

    /*
     * Check if the algorithm is enabled
     */
    if (pInstance->Params.OperatingMode != LVCS_OFF)
    {
        /*
         * Call CS process function
         */
            err=LVCS_Process_CS(hInstance,
                            pInData,
                            pOutData,
                            NumSamples);

        /*
         * Compress to reduce expansion effect of Concert Sound and correct volume
         * differences for difference settings. Not applied in test modes
         */
        if ((pInstance->Params.OperatingMode == LVCS_ON)&&(pInstance->Params.CompressorMode == LVM_MODE_ON))
        {
            LVM_INT16 Gain = pInstance->VolCorrect.CompMin;
            LVM_INT32 Current1;

            Current1 = LVC_Mixer_GetCurrent(&pInstance->BypassMix.Mixer_Instance.MixerStream[0]);
            Gain = (LVM_INT16)(  pInstance->VolCorrect.CompMin
                               - (((LVM_INT32)pInstance->VolCorrect.CompMin  * (Current1)) >> 15)
                               + (((LVM_INT32)pInstance->VolCorrect.CompFull * (Current1)) >> 15) );

            if(NumSamples < LVCS_COMPGAINFRAME)
            {
                NonLinComp_D16(Gain,                    /* Compressor gain setting */
                    pOutData,
                    pOutData,
                    (LVM_INT32)(2*NumSamples));
            }
            else
            {
                LVM_INT16  GainStep;
                LVM_INT16  FinalGain;
                LVM_INT16  SampleToProcess = NumSamples;
                LVM_INT16  *pOutPtr;

                /* Large changes in Gain can cause clicks in output
                   Split data into small blocks and use interpolated gain values */

                GainStep = (LVM_INT16)(((Gain-pInstance->CompressGain) * LVCS_COMPGAINFRAME)/NumSamples);

                if((GainStep ==0)&&(pInstance->CompressGain < Gain))
                {
                    GainStep=1;
                }
                else
                {
                    if((GainStep ==0)&&(pInstance->CompressGain > Gain))
                    {
                        GainStep=-1;
                    }
                }

                FinalGain = Gain;
                Gain = pInstance->CompressGain;
                pOutPtr = pOutData;

                while(SampleToProcess > 0)
                {
                    Gain = (LVM_INT16)(Gain + GainStep);
                    if((GainStep > 0)&& (FinalGain <= Gain))
                    {
                        Gain = FinalGain;
                        GainStep =0;
                    }

                    if((GainStep < 0)&& (FinalGain > Gain))
                    {
                        Gain = FinalGain;
                        GainStep =0;
                    }

                    if(SampleToProcess > LVCS_COMPGAINFRAME)
                    {
                        NonLinComp_D16(Gain,                    /* Compressor gain setting */
                            pOutPtr,
                            pOutPtr,
                            (LVM_INT32)(2*LVCS_COMPGAINFRAME));
                        pOutPtr +=(2*LVCS_COMPGAINFRAME);
                        SampleToProcess = (LVM_INT16)(SampleToProcess-LVCS_COMPGAINFRAME);
                    }
                    else
                    {
                        NonLinComp_D16(Gain,                    /* Compressor gain setting */
                            pOutPtr,
                            pOutPtr,
                            (LVM_INT32)(2*SampleToProcess));

                        SampleToProcess = 0;
                    }

                }
            }

            /* Store gain value*/
            pInstance->CompressGain = Gain;
        }


        if(pInstance->bInOperatingModeTransition == LVM_TRUE){

            /*
             * Re-init bypass mix when timer has completed
             */
            if ((pInstance->bTimerDone == LVM_TRUE) &&
                (pInstance->BypassMix.Mixer_Instance.MixerStream[1].CallbackSet == 0))
            {
                err=LVCS_BypassMixInit(hInstance,
                                   &pInstance->Params);

                if(err != LVCS_SUCCESS)
                {
                    return err;
                }

            }
            else{
                LVM_Timer ( &pInstance->TimerInstance,
                            (LVM_INT16)NumSamples);
            }
        }
    }
    else
    {
        if (pInData != pOutData)
        {
            /*
             * The algorithm is disabled so just copy the data
             */
            Copy_16((LVM_INT16 *)pInData,               /* Source */
                (LVM_INT16 *)pOutData,                  /* Destination */
                (LVM_INT16)(2*NumSamples));             /* Left and right */
        }
    }


    return(LVCS_SUCCESS);
}
#endif
