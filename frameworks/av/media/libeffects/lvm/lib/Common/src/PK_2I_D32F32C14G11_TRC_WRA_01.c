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
#include "PK_2I_D32F32CssGss_TRC_WRA_01_Private.h"
#include "LVM_Macros.h"

/**************************************************************************
 ASSUMPTIONS:
 COEFS-
 pBiquadState->coefs[0] is A0,
 pBiquadState->coefs[1] is -B2,
 pBiquadState->coefs[2] is -B1, these are in Q14 format
 pBiquadState->coefs[3] is Gain, in Q11 format


 DELAYS-
 pBiquadState->pDelays[0] is x(n-1)L in Q0 format
 pBiquadState->pDelays[1] is x(n-1)R in Q0 format
 pBiquadState->pDelays[2] is x(n-2)L in Q0 format
 pBiquadState->pDelays[3] is x(n-2)R in Q0 format
 pBiquadState->pDelays[4] is y(n-1)L in Q0 format
 pBiquadState->pDelays[5] is y(n-1)R in Q0 format
 pBiquadState->pDelays[6] is y(n-2)L in Q0 format
 pBiquadState->pDelays[7] is y(n-2)R in Q0 format
***************************************************************************/
#ifdef BUILD_FLOAT
void PK_2I_D32F32C14G11_TRC_WRA_01 ( Biquad_FLOAT_Instance_t       *pInstance,
                                     LVM_FLOAT               *pDataIn,
                                     LVM_FLOAT               *pDataOut,
                                     LVM_INT16               NrSamples)
    {
        LVM_FLOAT ynL,ynR,ynLO,ynRO,templ;
        LVM_INT16 ii;
        PFilter_State_Float pBiquadState = (PFilter_State_Float) pInstance;

         for (ii = NrSamples; ii != 0; ii--)
         {


            /**************************************************************************
                            PROCESSING OF THE LEFT CHANNEL
            ***************************************************************************/
            /* ynL= (A0  * (x(n)L - x(n-2)L  ) )*/
            templ = (*pDataIn) - pBiquadState->pDelays[2];
            ynL = templ * pBiquadState->coefs[0];

            /* ynL+= ((-B2  * y(n-2)L  )) */
            templ = pBiquadState->pDelays[6] * pBiquadState->coefs[1];
            ynL += templ;

            /* ynL+= ((-B1 * y(n-1)L  ) ) */
            templ = pBiquadState->pDelays[4] * pBiquadState->coefs[2];
            ynL += templ;

            /* ynLO= ((Gain * ynL )) */
            ynLO = ynL * pBiquadState->coefs[3];

            /* ynLO=( ynLO + x(n)L  )*/
            ynLO += (*pDataIn);

            /**************************************************************************
                            PROCESSING OF THE RIGHT CHANNEL
            ***************************************************************************/
            /* ynR= (A0  * (x(n)R  - x(n-2)R  ) ) */
            templ = (*(pDataIn + 1)) - pBiquadState->pDelays[3];
            ynR = templ * pBiquadState->coefs[0];

            /* ynR+= ((-B2  * y(n-2)R  ) )  */
            templ = pBiquadState->pDelays[7] * pBiquadState->coefs[1];
            ynR += templ;

            /* ynR+= ((-B1  * y(n-1)R  ) )   */
            templ = pBiquadState->pDelays[5] * pBiquadState->coefs[2];
            ynR += templ;

            /* ynRO= ((Gain  * ynR )) */
            ynRO = ynR * pBiquadState->coefs[3];

            /* ynRO=( ynRO + x(n)R  )*/
            ynRO += (*(pDataIn+1));

            /**************************************************************************
                            UPDATING THE DELAYS
            ***************************************************************************/
            pBiquadState->pDelays[7] = pBiquadState->pDelays[5]; /* y(n-2)R=y(n-1)R*/
            pBiquadState->pDelays[6] = pBiquadState->pDelays[4]; /* y(n-2)L=y(n-1)L*/
            pBiquadState->pDelays[3] = pBiquadState->pDelays[1]; /* x(n-2)R=x(n-1)R*/
            pBiquadState->pDelays[2] = pBiquadState->pDelays[0]; /* x(n-2)L=x(n-1)L*/
            pBiquadState->pDelays[5] = ynR; /* Update y(n-1)R */
            pBiquadState->pDelays[4] = ynL; /* Update y(n-1)L */
            pBiquadState->pDelays[0] = (*pDataIn); /* Update x(n-1)L */
            pDataIn++;
            pBiquadState->pDelays[1] = (*pDataIn); /* Update x(n-1)R */
            pDataIn++;

            /**************************************************************************
                            WRITING THE OUTPUT
            ***************************************************************************/
            *pDataOut = ynLO; /* Write Left output*/
            pDataOut++;
            *pDataOut = ynRO; /* Write Right ouput*/
            pDataOut++;

        }

    }

#ifdef SUPPORT_MC
/**************************************************************************
DELAYS-
pBiquadState->pDelays[0] to
pBiquadState->pDelays[NrChannels - 1] is x(n-1) for all NrChannels

pBiquadState->pDelays[NrChannels] to
pBiquadState->pDelays[2*NrChannels - 1] is x(n-2) for all NrChannels

pBiquadState->pDelays[2*NrChannels] to
pBiquadState->pDelays[3*NrChannels - 1] is y(n-1) for all NrChannels

pBiquadState->pDelays[3*NrChannels] to
pBiquadState->pDelays[4*NrChannels - 1] is y(n-2) for all NrChannels
***************************************************************************/

void PK_Mc_D32F32C14G11_TRC_WRA_01 (Biquad_FLOAT_Instance_t       *pInstance,
                                    LVM_FLOAT               *pDataIn,
                                    LVM_FLOAT               *pDataOut,
                                    LVM_INT16               NrFrames,
                                    LVM_INT16               NrChannels)
    {
        LVM_FLOAT yn, ynO, temp;
        LVM_INT16 ii, jj;
        PFilter_State_Float pBiquadState = (PFilter_State_Float) pInstance;

         for (ii = NrFrames; ii != 0; ii--)
         {

            for (jj = 0; jj < NrChannels; jj++)
            {
                /**************************************************************************
                                PROCESSING OF THE jj CHANNEL
                ***************************************************************************/
                /* yn= (A0  * (x(n) - x(n-2)))*/
                temp = (*pDataIn) - pBiquadState->pDelays[NrChannels + jj];
                yn = temp * pBiquadState->coefs[0];

                /* yn+= ((-B2  * y(n-2))) */
                temp = pBiquadState->pDelays[NrChannels*3 + jj] * pBiquadState->coefs[1];
                yn += temp;

                /* yn+= ((-B1 * y(n-1))) */
                temp = pBiquadState->pDelays[NrChannels*2 + jj] * pBiquadState->coefs[2];
                yn += temp;

                /* ynO= ((Gain * yn)) */
                ynO = yn * pBiquadState->coefs[3];

                /* ynO=(ynO + x(n))*/
                ynO += (*pDataIn);

                /**************************************************************************
                                UPDATING THE DELAYS
                ***************************************************************************/
                pBiquadState->pDelays[NrChannels * 3 + jj] =
                    pBiquadState->pDelays[NrChannels * 2 + jj]; /* y(n-2)=y(n-1)*/
                pBiquadState->pDelays[NrChannels * 1 + jj] =
                    pBiquadState->pDelays[jj]; /* x(n-2)=x(n-1)*/
                pBiquadState->pDelays[NrChannels * 2 + jj] = yn; /* Update y(n-1) */
                pBiquadState->pDelays[jj] = (*pDataIn); /* Update x(n-1)*/
                pDataIn++;

                /**************************************************************************
                                WRITING THE OUTPUT
                ***************************************************************************/
                *pDataOut = ynO; /* Write output*/
                pDataOut++;
            }
        }

    }
#endif
#else
void PK_2I_D32F32C14G11_TRC_WRA_01 ( Biquad_Instance_t       *pInstance,
                                     LVM_INT32               *pDataIn,
                                     LVM_INT32               *pDataOut,
                                     LVM_INT16               NrSamples)
    {
        LVM_INT32 ynL,ynR,ynLO,ynRO,templ;
        LVM_INT16 ii;
        PFilter_State pBiquadState = (PFilter_State) pInstance;

         for (ii = NrSamples; ii != 0; ii--)
         {


            /**************************************************************************
                            PROCESSING OF THE LEFT CHANNEL
            ***************************************************************************/
            /* ynL= (A0 (Q14) * (x(n)L (Q0) - x(n-2)L (Q0) ) >>14)  in Q0*/
            templ=(*pDataIn)-pBiquadState->pDelays[2];
            MUL32x16INTO32(templ,pBiquadState->coefs[0],ynL,14)

            /* ynL+= ((-B2 (Q14) * y(n-2)L (Q0) ) >>14) in Q0*/
            MUL32x16INTO32(pBiquadState->pDelays[6],pBiquadState->coefs[1],templ,14)
            ynL+=templ;

            /* ynL+= ((-B1 (Q14) * y(n-1)L (Q0) ) >>14) in Q0 */
            MUL32x16INTO32(pBiquadState->pDelays[4],pBiquadState->coefs[2],templ,14)
            ynL+=templ;

            /* ynLO= ((Gain (Q11) * ynL (Q0))>>11) in Q0*/
            MUL32x16INTO32(ynL,pBiquadState->coefs[3],ynLO,11)

            /* ynLO=( ynLO(Q0) + x(n)L (Q0) ) in Q0*/
            ynLO+= (*pDataIn);

            /**************************************************************************
                            PROCESSING OF THE RIGHT CHANNEL
            ***************************************************************************/
            /* ynR= (A0 (Q14) * (x(n)R (Q0) - x(n-2)R (Q0) ) >>14)   in Q0*/
            templ=(*(pDataIn+1))-pBiquadState->pDelays[3];
            MUL32x16INTO32(templ,pBiquadState->coefs[0],ynR,14)

            /* ynR+= ((-B2 (Q14) * y(n-2)R (Q0) ) >>14)  in Q0*/
            MUL32x16INTO32(pBiquadState->pDelays[7],pBiquadState->coefs[1],templ,14)
            ynR+=templ;

            /* ynR+= ((-B1 (Q14) * y(n-1)R (Q0) ) >>14)  in Q0 */
            MUL32x16INTO32(pBiquadState->pDelays[5],pBiquadState->coefs[2],templ,14)
            ynR+=templ;

            /* ynRO= ((Gain (Q11) * ynR (Q0))>>11) in Q0*/
            MUL32x16INTO32(ynR,pBiquadState->coefs[3],ynRO,11)

            /* ynRO=( ynRO(Q0) + x(n)R (Q0) ) in Q0*/
            ynRO+= (*(pDataIn+1));

            /**************************************************************************
                            UPDATING THE DELAYS
            ***************************************************************************/
            pBiquadState->pDelays[7]=pBiquadState->pDelays[5]; /* y(n-2)R=y(n-1)R*/
            pBiquadState->pDelays[6]=pBiquadState->pDelays[4]; /* y(n-2)L=y(n-1)L*/
            pBiquadState->pDelays[3]=pBiquadState->pDelays[1]; /* x(n-2)R=x(n-1)R*/
            pBiquadState->pDelays[2]=pBiquadState->pDelays[0]; /* x(n-2)L=x(n-1)L*/
            pBiquadState->pDelays[5]=ynR; /* Update y(n-1)R in Q0*/
            pBiquadState->pDelays[4]=ynL; /* Update y(n-1)L in Q0*/
            pBiquadState->pDelays[0]=(*pDataIn); /* Update x(n-1)L in Q0*/
            pDataIn++;
            pBiquadState->pDelays[1]=(*pDataIn); /* Update x(n-1)R in Q0*/
            pDataIn++;

            /**************************************************************************
                            WRITING THE OUTPUT
            ***************************************************************************/
            *pDataOut=ynLO; /* Write Left output in Q0*/
            pDataOut++;
            *pDataOut=ynRO; /* Write Right ouput in Q0*/
            pDataOut++;

        }

    }
#endif
