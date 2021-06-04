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

#include "BIQUAD.h"
#include "FO_2I_D16F32Css_LShx_TRC_WRA_01_Private.h"
#include "LVM_Macros.h"

/**************************************************************************
ASSUMPTIONS:
COEFS-
pBiquadState->coefs[0] is A1,
pBiquadState->coefs[1] is A0,
pBiquadState->coefs[2] is -B1, these are in Q15 format
pBiquadState->Shift    is Shift value
DELAYS-
pBiquadState->pDelays[0] is x(n-1)L in Q15 format
pBiquadState->pDelays[1] is y(n-1)L in Q30 format
pBiquadState->pDelays[2] is x(n-1)R in Q15 format
pBiquadState->pDelays[3] is y(n-1)R in Q30 format
***************************************************************************/
#ifdef BUILD_FLOAT
void FO_2I_D16F32C15_LShx_TRC_WRA_01(Biquad_FLOAT_Instance_t       *pInstance,
                                     LVM_FLOAT               *pDataIn,
                                     LVM_FLOAT               *pDataOut,
                                     LVM_INT16               NrSamples)
    {
        LVM_FLOAT   ynL,ynR;
        LVM_FLOAT   Temp;
        LVM_FLOAT   NegSatValue;
        LVM_INT16   ii;

        PFilter_Float_State pBiquadState = (PFilter_Float_State) pInstance;

        NegSatValue = -1.0f;

        for (ii = NrSamples; ii != 0; ii--)
        {

            /**************************************************************************
                            PROCESSING OF THE LEFT CHANNEL
            ***************************************************************************/

            // ynL =A1  * x(n-1)L
            ynL = (LVM_FLOAT)pBiquadState->coefs[0] * pBiquadState->pDelays[0];
            // ynR =A1  * x(n-1)R
            ynR = (LVM_FLOAT)pBiquadState->coefs[0] * pBiquadState->pDelays[2];


            // ynL+=A0  * x(n)L
            ynL += (LVM_FLOAT)pBiquadState->coefs[1] * (*pDataIn);
            // ynR+=A0  * x(n)L
            ynR += (LVM_FLOAT)pBiquadState->coefs[1] * (*(pDataIn+1));


            // ynL +=  (-B1  * y(n-1)L  )
            Temp = pBiquadState->pDelays[1] * pBiquadState->coefs[2];
            ynL += Temp;
            // ynR +=  (-B1  * y(n-1)R ) )
            Temp = pBiquadState->pDelays[3] * pBiquadState->coefs[2];
            ynR += Temp;


            /**************************************************************************
                            UPDATING THE DELAYS
            ***************************************************************************/
            pBiquadState->pDelays[1] = ynL; // Update y(n-1)L
            pBiquadState->pDelays[0] = (*pDataIn++); // Update x(n-1)L

            pBiquadState->pDelays[3] = ynR; // Update y(n-1)R
            pBiquadState->pDelays[2] = (*pDataIn++); // Update x(n-1)R

            /**************************************************************************
                            WRITING THE OUTPUT
            ***************************************************************************/

            /*Saturate results*/
            if(ynL > 1.0f)
            {
                ynL = 1.0f;
            }
            else
            {
                if(ynL < NegSatValue)
                {
                    ynL = NegSatValue;
                }
            }

            if(ynR > 1.0f)
            {
                ynR = 1.0f;
            }
            else
            {
                if(ynR < NegSatValue)
                {
                    ynR = NegSatValue;
                }
            }

            *pDataOut++ = (LVM_FLOAT)ynL;
            *pDataOut++ = (LVM_FLOAT)ynR;
        }

    }
#ifdef SUPPORT_MC
/**************************************************************************
ASSUMPTIONS:
COEFS-
pBiquadState->coefs[0] is A1,
pBiquadState->coefs[1] is A0,
pBiquadState->coefs[2] is -B1,
DELAYS-
pBiquadState->pDelays[2*ch + 0] is x(n-1) of the 'ch' - channel
pBiquadState->pDelays[2*ch + 1] is y(n-1) of the 'ch' - channel
The index 'ch' runs from 0 to (NrChannels - 1)

PARAMETERS:
 pInstance        Pointer Instance
 pDataIn          Input/Source
 pDataOut         Output/Destination
 NrFrames         Number of frames
 NrChannels       Number of channels

RETURNS:
 void
***************************************************************************/
void FO_Mc_D16F32C15_LShx_TRC_WRA_01(Biquad_FLOAT_Instance_t *pInstance,
                                     LVM_FLOAT               *pDataIn,
                                     LVM_FLOAT               *pDataOut,
                                     LVM_INT16               NrFrames,
                                     LVM_INT16               NrChannels)
    {
        LVM_FLOAT   yn;
        LVM_FLOAT   Temp;
        LVM_INT16   ii;
        LVM_INT16   ch;
        PFilter_Float_State pBiquadState = (PFilter_Float_State) pInstance;

        LVM_FLOAT   *pDelays = pBiquadState->pDelays;
        LVM_FLOAT   *pCoefs  = &pBiquadState->coefs[0];
        LVM_FLOAT   A0 = pCoefs[1];
        LVM_FLOAT   A1 = pCoefs[0];
        LVM_FLOAT   B1 = pCoefs[2];




        for (ii = NrFrames; ii != 0; ii--)
        {

            /**************************************************************************
                            PROCESSING OF THE CHANNELS
            ***************************************************************************/
            for (ch = 0; ch < NrChannels; ch++)
            {
                // yn =A1  * x(n-1)
                yn = (LVM_FLOAT)A1 * pDelays[0];

                // yn+=A0  * x(n)
                yn += (LVM_FLOAT)A0 * (*pDataIn);

                // yn +=  (-B1  * y(n-1))
                Temp = B1 * pDelays[1];
                yn += Temp;


                /**************************************************************************
                                UPDATING THE DELAYS
                ***************************************************************************/
                pDelays[1] = yn; // Update y(n-1)
                pDelays[0] = (*pDataIn++); // Update x(n-1)

                /**************************************************************************
                                WRITING THE OUTPUT
                ***************************************************************************/

                /*Saturate results*/
                if (yn > 1.0f)
                {
                    yn = 1.0f;
                } else if (yn < -1.0f) {
                    yn = -1.0f;
                }

                *pDataOut++ = (LVM_FLOAT)yn;
                pDelays += 2;
            }
            pDelays -= NrChannels * 2;
        }
    }
#endif
#else
void FO_2I_D16F32C15_LShx_TRC_WRA_01(Biquad_Instance_t       *pInstance,
                                     LVM_INT16               *pDataIn,
                                     LVM_INT16               *pDataOut,
                                     LVM_INT16               NrSamples)
    {
        LVM_INT32   ynL,ynR;
        LVM_INT32   Temp;
        LVM_INT32   NegSatValue;
        LVM_INT16   ii;
        LVM_INT16   Shift;
        PFilter_State pBiquadState = (PFilter_State) pInstance;

        NegSatValue = LVM_MAXINT_16 +1;
        NegSatValue = -NegSatValue;

        Shift = pBiquadState->Shift;


        for (ii = NrSamples; ii != 0; ii--)
        {

            /**************************************************************************
                            PROCESSING OF THE LEFT CHANNEL
            ***************************************************************************/

            // ynL =A1 (Q15) * x(n-1)L (Q15) in Q30
            ynL=(LVM_INT32)pBiquadState->coefs[0]* pBiquadState->pDelays[0];
            // ynR =A1 (Q15) * x(n-1)R (Q15) in Q30
            ynR=(LVM_INT32)pBiquadState->coefs[0]* pBiquadState->pDelays[2];


            // ynL+=A0 (Q15) * x(n)L (Q15) in Q30
            ynL+=(LVM_INT32)pBiquadState->coefs[1]* (*pDataIn);
            // ynR+=A0 (Q15) * x(n)L (Q15) in Q30
            ynR+=(LVM_INT32)pBiquadState->coefs[1]* (*(pDataIn+1));


            // ynL +=  (-B1 (Q15) * y(n-1)L (Q30) ) in Q30
            MUL32x16INTO32(pBiquadState->pDelays[1],pBiquadState->coefs[2],Temp,15);
            ynL +=Temp;
            // ynR +=  (-B1 (Q15) * y(n-1)R (Q30) ) in Q30
            MUL32x16INTO32(pBiquadState->pDelays[3],pBiquadState->coefs[2],Temp,15);
            ynR +=Temp;


            /**************************************************************************
                            UPDATING THE DELAYS
            ***************************************************************************/
            pBiquadState->pDelays[1]=ynL; // Update y(n-1)L in Q30
            pBiquadState->pDelays[0]=(*pDataIn++); // Update x(n-1)L in Q15

            pBiquadState->pDelays[3]=ynR; // Update y(n-1)R in Q30
            pBiquadState->pDelays[2]=(*pDataIn++); // Update x(n-1)R in Q15

            /**************************************************************************
                            WRITING THE OUTPUT
            ***************************************************************************/
            /*Apply shift: Instead of left shift on 16-bit result, right shift of (15-shift) is applied
              for better SNR*/
            ynL = ynL>>(15-Shift);
            ynR = ynR>>(15-Shift);

            /*Saturate results*/
            if(ynL > LVM_MAXINT_16)
            {
                ynL = LVM_MAXINT_16;
            }
            else
            {
                if(ynL < NegSatValue)
                {
                    ynL = NegSatValue;
                }
            }

            if(ynR > LVM_MAXINT_16)
            {
                ynR = LVM_MAXINT_16;
            }
            else
            {
                if(ynR < NegSatValue)
                {
                    ynR = NegSatValue;
                }
            }

            *pDataOut++=(LVM_INT16)ynL;
            *pDataOut++=(LVM_INT16)ynR;
        }

    }
#endif
