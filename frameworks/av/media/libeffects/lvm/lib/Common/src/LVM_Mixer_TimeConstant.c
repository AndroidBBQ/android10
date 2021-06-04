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

#include "LVM_Types.h"
#include "LVM_Macros.h"
#include "Mixer.h"
#include "LVM_Mixer_FilterCoeffs.h"


/************************************************************************/
/* FUNCTION:                                                            */
/*   LVM_Mix_GetTimeConstant                                            */
/*                                                                      */
/* DESCRIPTION:                                                         */
/*  This function calculates the filter coefficient using the following */
/*  equation:                                                           */
/*       Alpha = exp(ln(0.1)/ (tc * Update + 1.0))                      */
/*                                                                      */
/*  This is to be used with the follow first order filter, called at a  */
/*  rate of Update times a second. tc is the required time constant in  */
/*  units of 100us.                                                     */
/*                                                                      */
/*       Output(n) = Alpha * Output(n-1) + (1 - Alpha) * Target(n)      */
/*                                                                      */
/*  The function assumes the block size is large, i.e. the update rate  */
/*  is approximately a fixed, and correct factor of the value of Fs     */
/*  given in the call. This is especially not true when the block size  */
/*  is very small, see the mixer documentation for further details.     */
/*                                                                      */
/*  The function does not support all possible combinations of input    */
/*  values:                                                             */
/*                                                                      */
/*  1. NumChannels is limited to the values 1 (Mono) and 2 (Stereo)     */
/*  2. The product tc * Fs is limited approximately to the range        */
/*      8 < (tc * Fs) < 2^35                                            */
/*                                                                      */
/* PARAMETERS:                                                          */
/*  tc              - the time constant in 100us steps, i.e. 10 = 1ms   */
/*  Fs              - the filter update rate in Hz                      */
/*  NumChannels     - Number of channels 1=Mono, 2=Stereo               */
/*                                                                      */
/* RETURNS:                                                             */
/*  Alpha   - the filter coefficient Q31 format                         */
/*                                                                      */
/************************************************************************/
#ifdef BUILD_FLOAT
LVM_FLOAT LVM_Mixer_TimeConstant(LVM_UINT32   tc,
#ifdef HIGHER_FS
                                  LVM_UINT32   Fs,
#else
                                  LVM_UINT16   Fs,
#endif
                                  LVM_UINT16   NumChannels)
{

    LVM_UINT32  Product;
    LVM_FLOAT  ProductFloat;
    LVM_INT16   InterpolateShort;
    LVM_FLOAT   Interpolate;
    LVM_UINT16  Shift;
    LVM_FLOAT   Diff;
    LVM_FLOAT  Table[] = {ALPHA_Float_0,             /* Log spaced look-up table */
                          ALPHA_Float_1,
                          ALPHA_Float_2,
                          ALPHA_Float_3,
                          ALPHA_Float_4,
                          ALPHA_Float_5,
                          ALPHA_Float_6,
                          ALPHA_Float_7,
                          ALPHA_Float_8,
                          ALPHA_Float_9,
                          ALPHA_Float_10,
                          ALPHA_Float_11,
                          ALPHA_Float_12,
                          ALPHA_Float_13,
                          ALPHA_Float_14,
                          ALPHA_Float_15,
                          ALPHA_Float_16,
                          ALPHA_Float_17,
                          ALPHA_Float_18,
                          ALPHA_Float_19,
                          ALPHA_Float_20,
                          ALPHA_Float_21,
                          ALPHA_Float_22,
                          ALPHA_Float_23,
                          ALPHA_Float_24,
                          ALPHA_Float_25,
                          ALPHA_Float_26,
                          ALPHA_Float_27,
                          ALPHA_Float_28,
                          ALPHA_Float_29,
                          ALPHA_Float_30,
                          ALPHA_Float_31,
                          ALPHA_Float_32,
                          ALPHA_Float_33,
                          ALPHA_Float_34,
                          ALPHA_Float_35,
                          ALPHA_Float_36,
                          ALPHA_Float_37,
                          ALPHA_Float_38,
                          ALPHA_Float_39,
                          ALPHA_Float_40,
                          ALPHA_Float_41,
                          ALPHA_Float_42,
                          ALPHA_Float_43,
                          ALPHA_Float_44,
                          ALPHA_Float_45,
                          ALPHA_Float_46,
                          ALPHA_Float_47,
                          ALPHA_Float_48,
                          ALPHA_Float_49,
                          ALPHA_Float_50};

    /* Calculate the product of the time constant and the sample rate */
    Product = ((tc >> 16) * (LVM_UINT32)Fs) << 13;  /* Stereo value */
    Product = Product + (((tc & 0x0000FFFF) * (LVM_UINT32)Fs) >> 3);

    if (NumChannels == 1)
    {
        Product = Product >> 1;   /* Mono value */
    }

    /* Normalize to get the table index and interpolation factor */
    for (Shift = 0; Shift < ((Alpha_TableSize - 1) / 2); Shift++)
    {
        if ((Product & 0x80000000) != 0)
        {
            break;
        }

        Product = Product << 1;
    }
    Shift = (LVM_UINT16)((Shift << 1));

    if ((Product & 0x40000000)==0)
    {
        Shift++;
    }

    InterpolateShort = (LVM_INT16)((Product >> 15) & 0x00007FFF);
    Interpolate = (LVM_FLOAT)InterpolateShort / 32768.0f;

    Diff = (Table[Shift] - Table[Shift + 1]);
    Diff = Diff * Interpolate;
    ProductFloat = Table[Shift + 1] + Diff;

    return ProductFloat;
}
#else
LVM_UINT32 LVM_Mixer_TimeConstant(LVM_UINT32   tc,
                                  LVM_UINT16   Fs,
                                  LVM_UINT16   NumChannels)
{

    LVM_UINT32  Product;
    LVM_INT16   Interpolate;
    LVM_UINT16  Shift;
    LVM_INT32   Diff;
    LVM_UINT32  Table[] = {ALPHA_0,             /* Log spaced look-up table */
                           ALPHA_1,
                           ALPHA_2,
                           ALPHA_3,
                           ALPHA_4,
                           ALPHA_5,
                           ALPHA_6,
                           ALPHA_7,
                           ALPHA_8,
                           ALPHA_9,
                           ALPHA_10,
                           ALPHA_11,
                           ALPHA_12,
                           ALPHA_13,
                           ALPHA_14,
                           ALPHA_15,
                           ALPHA_16,
                           ALPHA_17,
                           ALPHA_18,
                           ALPHA_19,
                           ALPHA_20,
                           ALPHA_21,
                           ALPHA_22,
                           ALPHA_23,
                           ALPHA_24,
                           ALPHA_25,
                           ALPHA_26,
                           ALPHA_27,
                           ALPHA_28,
                           ALPHA_29,
                           ALPHA_30,
                           ALPHA_31,
                           ALPHA_32,
                           ALPHA_33,
                           ALPHA_34,
                           ALPHA_35,
                           ALPHA_36,
                           ALPHA_37,
                           ALPHA_38,
                           ALPHA_39,
                           ALPHA_40,
                           ALPHA_41,
                           ALPHA_42,
                           ALPHA_43,
                           ALPHA_44,
                           ALPHA_45,
                           ALPHA_46,
                           ALPHA_47,
                           ALPHA_48,
                           ALPHA_49,
                           ALPHA_50};


    /* Calculate the product of the time constant and the sample rate */
    Product = ((tc >> 16) * (LVM_UINT32)Fs) << 13;  /* Stereo value */
    Product = Product + (((tc & 0x0000FFFF) * (LVM_UINT32)Fs) >> 3);

    if (NumChannels == 1)
    {
        Product = Product >> 1;   /* Mono value */
    }

    /* Normalize to get the table index and interpolation factor */
    for (Shift=0; Shift<((Alpha_TableSize-1)/2); Shift++)
    {
        if ((Product & 0x80000000)!=0)
        {
            break;
        }

        Product = Product << 1;
    }
    Shift = (LVM_UINT16)((Shift << 1));

    if ((Product & 0x40000000)==0)
    {
        Shift++;
    }

    Interpolate = (LVM_INT16)((Product >> 15) & 0x00007FFF);

    Diff = (LVM_INT32)(Table[Shift] - Table[Shift+1]);
    MUL32x16INTO32(Diff,Interpolate,Diff,15)
        Product = Table[Shift+1] + (LVM_UINT32)Diff;

    return Product;
}
#endif
