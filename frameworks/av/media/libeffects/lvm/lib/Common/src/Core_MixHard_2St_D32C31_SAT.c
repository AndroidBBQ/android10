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

#include "Mixer_private.h"
#include "LVM_Macros.h"

/**********************************************************************************
   FUNCTION CORE_MIXHARD_2ST_D32C31_SAT
***********************************************************************************/
#ifdef BUILD_FLOAT
void Core_MixHard_2St_D32C31_SAT(   Mix_2St_Cll_FLOAT_t       *pInstance,
                                    const LVM_FLOAT     *src1,
                                    const LVM_FLOAT     *src2,
                                    LVM_FLOAT     *dst,
                                    LVM_INT16     n)
{
    LVM_FLOAT  Temp1,Temp2,Temp3;
    LVM_INT16 ii;
    LVM_FLOAT Current1Short;
    LVM_FLOAT Current2Short;

    Current1Short = (pInstance->Current1);
    Current2Short = (pInstance->Current2);

    for (ii = n; ii != 0; ii--){
        Temp1 = *src1++;
        Temp3 = Temp1 * Current1Short;
        Temp2 = *src2++;
        Temp1 = Temp2 * Current2Short;
        Temp2 = (Temp1 / 2.0f) + (Temp3 / 2.0f);
        if (Temp2 > 0.5f)
            Temp2 = 1.0f;
        else if (Temp2 < -0.5f )
            Temp2 = -1.0f;
        else
            Temp2 = (Temp2 * 2);
            *dst++ = Temp2;
    }
}
#else
void Core_MixHard_2St_D32C31_SAT(   Mix_2St_Cll_t       *pInstance,
                                    const LVM_INT32     *src1,
                                    const LVM_INT32     *src2,
                                          LVM_INT32     *dst,
                                          LVM_INT16     n)
{
    LVM_INT32  Temp1,Temp2,Temp3;
    LVM_INT16 ii;
    LVM_INT16 Current1Short;
    LVM_INT16 Current2Short;

    Current1Short = (LVM_INT16)(pInstance->Current1 >> 16);
    Current2Short = (LVM_INT16)(pInstance->Current2 >> 16);

    for (ii = n; ii != 0; ii--){
        Temp1=*src1++;
        MUL32x16INTO32(Temp1,Current1Short,Temp3,15)
        Temp2=*src2++;
        MUL32x16INTO32(Temp2,Current2Short,Temp1,15)
        Temp2=(Temp1>>1)+(Temp3>>1);
        if (Temp2 > 0x3FFFFFFF)
            Temp2 = 0x7FFFFFFF;
        else if (Temp2 < - 0x40000000)
            Temp2 =  0x80000000;
        else
            Temp2=(Temp2<<1);
            *dst++ = Temp2;
    }
}
#endif
/**********************************************************************************/
