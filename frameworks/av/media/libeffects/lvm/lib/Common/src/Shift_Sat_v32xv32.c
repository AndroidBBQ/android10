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

/**********************************************************************************
   FUNCTION Shift_Sat_v32xv32
***********************************************************************************/
#ifdef BUILD_FLOAT
void Shift_Sat_Float (const   LVM_INT16   val,
                      const   LVM_FLOAT   *src,
                      LVM_FLOAT   *dst,
                      LVM_INT16   n)
{
    LVM_FLOAT   temp;
    LVM_INT32   ii,ij;
    LVM_INT16   RShift;

    if(val > 0)
    {
        for (ii = n; ii != 0; ii--)
        {
            temp = (LVM_FLOAT)*src;
            src++;
            for(ij = 0; ij < val; ij++)
            {
                temp = temp * 2;
            }

            if(temp > 1.0)
                temp = 1.0;
            if(temp < -1.0)
                temp = -1.0;

            *dst = (LVM_FLOAT)temp;
            dst++;
        }
    }
    else if(val < 0)
    {
        RShift=(LVM_INT16)(-val);

        for (ii = n; ii != 0; ii--)
        {
            temp = (LVM_FLOAT)*src;
            src++;
            for(ij = 0; ij < RShift; ij++)
            {
                temp = temp / 2;
            }
            *dst = (LVM_FLOAT)temp;
            dst++;
        }
    }
    else
    {
        if(src != dst)
        {
            Copy_Float(src, dst, n);
        }
    }
    return;
}
#else
void Shift_Sat_v32xv32 (const   LVM_INT16   val,
                        const   LVM_INT32   *src,
                        LVM_INT32   *dst,
                        LVM_INT16   n)
{
    LVM_INT32   ii;
    LVM_INT16   RShift;

    if(val>0)
    {
        LVM_INT32 a,b;

        for (ii = n; ii != 0; ii--)
        {
            a=*src;
            src++;

            b=(a<<val);

            if( (b>>val) != a ) /* if overflow occured, right shift will show difference*/
            {
                if(a<0)
                {
                    b=0x80000000l;
                }
                else
                {
                    b=0x7FFFFFFFl;
                }
            }

            *dst = b;
            dst++;
        }
    }
    else if(val<0)
    {
        RShift=(LVM_INT16)(-val);
        for (ii = n; ii != 0; ii--)
        {
            *dst = (*src >> RShift);
            dst++;
            src++;
        }
    }
    else
    {
        if(src!=dst)
        {
            Copy_16((LVM_INT16 *)src,(LVM_INT16 *)dst,(LVM_INT16)(n<<1));
        }
    }
    return;
}
#endif
/**********************************************************************************/
