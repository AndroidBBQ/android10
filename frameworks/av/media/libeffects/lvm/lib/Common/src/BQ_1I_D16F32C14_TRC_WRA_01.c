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
#include "BQ_1I_D16F32Css_TRC_WRA_01_Private.h"
#include "LVM_Macros.h"

/**************************************************************************
 ASSUMPTIONS:
 COEFS-
 pBiquadState->coefs[0] is A2, pBiquadState->coefs[1] is A1
 pBiquadState->coefs[2] is A0, pBiquadState->coefs[3] is -B2
 pBiquadState->coefs[4] is -B1, these are in Q14 format

 DELAYS-
 pBiquadState->pDelays[0] is x(n-1)L in Q0 format
 pBiquadState->pDelays[1] is x(n-2)L in Q0 format
 pBiquadState->pDelays[2] is y(n-1)L in Q16 format
 pBiquadState->pDelays[3] is y(n-2)L in Q16 format
***************************************************************************/
#ifdef BUILD_FLOAT
void BQ_1I_D16F32C14_TRC_WRA_01 ( Biquad_FLOAT_Instance_t       *pInstance,
                                  LVM_FLOAT               *pDataIn,
                                  LVM_FLOAT               *pDataOut,
                                  LVM_INT16               NrSamples)
    {
        LVM_FLOAT  ynL;
        LVM_INT16 ii;
        PFilter_State_FLOAT pBiquadState = (PFilter_State_FLOAT) pInstance;

         for (ii = NrSamples; ii != 0; ii--)
         {


            /**************************************************************************
                            PROCESSING OF THE LEFT CHANNEL
            ***************************************************************************/
            // ynL=A2  * x(n-2)L
            ynL = (LVM_FLOAT)pBiquadState->coefs[0] * pBiquadState->pDelays[1];

            // ynL+=A1  * x(n-1)L
            ynL += (LVM_FLOAT)pBiquadState->coefs[1] * pBiquadState->pDelays[0];

            // ynL+=A0  * x(n)L
            ynL += (LVM_FLOAT)pBiquadState->coefs[2] * (*pDataIn);

            // ynL+= ( (-B2  * y(n-2)L )
            ynL += pBiquadState->pDelays[3] * pBiquadState->coefs[3];

            // ynL+= -B1  * y(n-1)L
            ynL += pBiquadState->pDelays[2] * pBiquadState->coefs[4];

            /**************************************************************************
                            UPDATING THE DELAYS
            ***************************************************************************/
            pBiquadState->pDelays[3] = pBiquadState->pDelays[2];  // y(n-2)L=y(n-1)L
            pBiquadState->pDelays[1] = pBiquadState->pDelays[0];  // x(n-2)L=x(n-1)L
            pBiquadState->pDelays[2] = ynL;                    // Update y(n-1)L
            pBiquadState->pDelays[0] = (*pDataIn++);              // Update x(n-1)L

            /**************************************************************************
                            WRITING THE OUTPUT
            ***************************************************************************/
            *pDataOut++ = (LVM_FLOAT)(ynL); // Write Left output

        }
    }
#else
void BQ_1I_D16F32C14_TRC_WRA_01 ( Biquad_Instance_t       *pInstance,
                                  LVM_INT16               *pDataIn,
                                  LVM_INT16               *pDataOut,
                                  LVM_INT16               NrSamples)
    {
        LVM_INT32  ynL,templ;
        LVM_INT16 ii;
        PFilter_State pBiquadState = (PFilter_State) pInstance;

         for (ii = NrSamples; ii != 0; ii--)
         {


            /**************************************************************************
                            PROCESSING OF THE LEFT CHANNEL
            ***************************************************************************/
            // ynL=A2 (Q14) * x(n-2)L (Q0) in Q14
            ynL=(LVM_INT32)pBiquadState->coefs[0]* pBiquadState->pDelays[1];

            // ynL+=A1 (Q14) * x(n-1)L (Q0) in Q14
            ynL+=(LVM_INT32)pBiquadState->coefs[1]* pBiquadState->pDelays[0];

            // ynL+=A0 (Q14) * x(n)L (Q0) in Q14
            ynL+=(LVM_INT32)pBiquadState->coefs[2]* (*pDataIn);

            // ynL+= ( (-B2 (Q14) * y(n-2)L (Q16) )>>16) in Q14
            MUL32x16INTO32(pBiquadState->pDelays[3],pBiquadState->coefs[3],templ,16)
            ynL+=templ;

            // ynL+= ( (-B1 (Q14) * y(n-1)L (Q16) )>>16) in Q14
            MUL32x16INTO32(pBiquadState->pDelays[2],pBiquadState->coefs[4],templ,16)
            ynL+=templ;

            /**************************************************************************
                            UPDATING THE DELAYS
            ***************************************************************************/
            pBiquadState->pDelays[3]=pBiquadState->pDelays[2];  // y(n-2)L=y(n-1)L
            pBiquadState->pDelays[1]=pBiquadState->pDelays[0];  // x(n-2)L=x(n-1)L
            pBiquadState->pDelays[2]=ynL<<2;                    // Update y(n-1)L in Q16
            pBiquadState->pDelays[0]=(*pDataIn++);              // Update x(n-1)L in Q0

            /**************************************************************************
                            WRITING THE OUTPUT
            ***************************************************************************/
            *pDataOut++=(LVM_INT16)(ynL>>14); // Write Left output in Q0

        }
    }
#endif
