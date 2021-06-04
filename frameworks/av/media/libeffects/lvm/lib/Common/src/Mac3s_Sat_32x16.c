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

#include "VectorArithmetic.h"
#include "LVM_Macros.h"

/**********************************************************************************
   FUNCTION MAC3S_16X16
***********************************************************************************/

void Mac3s_Sat_32x16(  const LVM_INT32 *src,
                     const LVM_INT16 val,
                     LVM_INT32 *dst,
                     LVM_INT16 n)
{
    LVM_INT16 ii;
    LVM_INT32 srcval,temp, dInVal, dOutVal;


    for (ii = n; ii != 0; ii--)
    {
        srcval=*src;
        src++;

        MUL32x16INTO32(srcval,val,temp,15)

            dInVal  = *dst;
        dOutVal = temp + dInVal;


        if ((((dOutVal ^ temp) & (dOutVal ^ dInVal)) >> 31)!=0)     /* overflow / underflow */
        {
            if(temp<0)
            {
                dOutVal=0x80000000L;
            }
            else
            {
                dOutVal=0x7FFFFFFFL;
            }
        }

        *dst = dOutVal;
        dst++;
    }

    return;
}
#ifdef BUILD_FLOAT
void Mac3s_Sat_Float(const LVM_FLOAT *src,
                     const LVM_FLOAT val,
                     LVM_FLOAT *dst,
                     LVM_INT16 n)
{
    LVM_INT16 ii;
    LVM_FLOAT srcval;
    LVM_FLOAT Temp,dInVal;

    for (ii = n; ii != 0; ii--)
    {
        srcval = *src;
        src++;

        Temp = srcval * val;

        dInVal  = (LVM_FLOAT)*dst;
        Temp = Temp + dInVal;

        if (Temp > 1.000000f)
        {
            *dst = 1.000000f;
        }
        else if (Temp < -1.000000f)
        {
            *dst = -1.000000f;
        }
        else
        {
            *dst = Temp;
        }
        dst++;
    }

    return;
}
#endif
/**********************************************************************************/



