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

#include "LVPSA.h"
#include "LVPSA_QPD.h"
/************************************************************************************/
/*                                                                                  */
/*  Sample rate table                                                               */
/*                                                                                  */
/************************************************************************************/

/*
 * Sample rate table for converting between the enumerated type and the actual
 * frequency
 */
#ifndef HIGHER_FS
const LVM_UINT16    LVPSA_SampleRateTab[] = {   8000,                    /* 8kS/s  */
                                                11025,
                                                12000,
                                                16000,
                                                22050,
                                                24000,
                                                32000,
                                                44100,
                                                48000};                  /* 48kS/s */
#else
const LVM_UINT32    LVPSA_SampleRateTab[] = {   8000,                    /* 8kS/s  */
                                                11025,
                                                12000,
                                                16000,
                                                22050,
                                                24000,
                                                32000,
                                                44100,
                                                48000,
                                                88200,
                                                96000,
                                               176400,
                                               192000};                  /* 192kS/s */
#endif

/************************************************************************************/
/*                                                                                  */
/*  Sample rate inverse table                                                       */
/*                                                                                  */
/************************************************************************************/

/*
 * Sample rate table for converting between the enumerated type and the actual
 * frequency
 */
const LVM_UINT32    LVPSA_SampleRateInvTab[] = {    268435,                    /* 8kS/s  */
                                                    194783,
                                                    178957,
                                                    134218,
                                                    97391,
                                                    89478,
                                                    67109,
                                                    48696,
                                                    44739
#ifdef HIGHER_FS
                                                    ,24348
                                                    ,22369
                                                    ,12174
                                                    ,11185                  /* 192kS/s */
#endif
                                               };



/************************************************************************************/
/*                                                                                  */
/*  Number of samples in 20ms                                                       */
/*                                                                                  */
/************************************************************************************/

/*
 * Table for converting between the enumerated type and the number of samples
 * during 20ms
 */
const LVM_UINT16    LVPSA_nSamplesBufferUpdate[]  = {   160,                   /* 8kS/s  */
                                                        220,
                                                        240,
                                                        320,
                                                        441,
                                                        480,
                                                        640,
                                                        882,
                                                        960
#ifdef HIGHER_FS
                                                        ,1764
                                                        ,1920
                                                        ,3528
                                                        ,3840                  /* 192kS/s */
#endif
                                                    };
/************************************************************************************/
/*                                                                                  */
/*  Down sampling factors                                                           */
/*                                                                                  */
/************************************************************************************/

/*
 * Table for converting between the enumerated type and the down sampling factor
 */
const LVM_UINT16    LVPSA_DownSamplingFactor[]  = {     5,                    /* 8000  S/s  */
                                                        7,                    /* 11025 S/s  */
                                                        8,                    /* 12000 S/s  */
                                                        10,                   /* 16000 S/s  */
                                                        15,                   /* 22050 S/s  */
                                                        16,                   /* 24000 S/s  */
                                                        21,                   /* 32000 S/s  */
                                                        30,                   /* 44100 S/s  */
                                                        32                    /* 48000 S/s  */
#ifdef HIGHER_FS
                                                       ,60                   /* 88200 S/s  */
                                                       ,64                   /* 96000 S/s  */
                                                       ,120                  /* 176400 S/s  */
                                                       ,128                  /*192000 S/s  */
#endif
                                                  };


/************************************************************************************/
/*                                                                                  */
/*  Coefficient calculation tables                                                  */
/*                                                                                  */
/************************************************************************************/

/*
 * Table for 2 * Pi / Fs
 */
const LVM_INT16     LVPSA_TwoPiOnFsTable[] = {  26354,      /* 8kS/s */
                                                19123,
                                                17569,
                                                13177,
                                                 9561,
                                                 8785,
                                                 6588,
                                                 4781,
                                                 4392
#ifdef HIGHER_FS
                                                ,2390
                                                ,2196
                                                ,1195
                                                ,1098    /* 192kS/s */
#endif
                                             };

#ifdef BUILD_FLOAT
const LVM_FLOAT     LVPSA_Float_TwoPiOnFsTable[] = {  0.8042847f,      /* 8kS/s */
                                                      0.5836054f,
                                                      0.5361796f,
                                                      0.4021423f,
                                                      0.2917874f,
                                                      0.2681051f,
                                                      0.2010559f,
                                                      0.1459089f,
                                                      0.1340372f
#ifdef HIGHER_FS
                                                     ,0.0729476f
                                                     ,0.0670186f
                                                     ,0.0364738f
                                                     ,0.0335093f    /* 192kS/s */
#endif
                                                   };

#endif
/*
 * Gain table
 */
const LVM_INT16     LVPSA_GainTable[] = {   364,          /* -15dB gain */
                                            408,
                                            458,
                                            514,
                                            577,
                                            647,
                                            726,
                                            815,
                                            914,
                                            1026,
                                            1151,
                                            1292,
                                            1449,
                                            1626,
                                            1825,
                                            2048,         /* 0dB gain */
                                            2297,
                                            2578,
                                            2892,
                                            3245,
                                            3641,
                                            4096,
                                            4584,
                                            5144,
                                            5772,
                                            6476,
                                            7266,
                                            8153,
                                            9148,
                                            10264,
                                            11576};        /* +15dB gain */

#ifdef BUILD_FLOAT
const LVM_FLOAT  LVPSA_Float_GainTable[]={  0.177734375f,          /* -15dB gain */
                                            0.199218750f,
                                            0.223632812f,
                                            0.250976562f,
                                            0.281738281f,
                                            0.315917968f,
                                            0.354492187f,
                                            0.397949218f,
                                            0.446289062f,
                                            0.500976562f,
                                            0.562011718f,
                                            0.630859375f,
                                            0.707519531f,
                                            0.793945312f,
                                            0.891113281f,
                                            1.000000000f,         /* 0dB gain */
                                            1.121582031f,
                                            1.258789062f,
                                            1.412109375f,
                                            1.584472656f,
                                            1.777832031f,
                                            2.000000000f,
                                            2.238281250f,
                                            2.511718750f,
                                            2.818359375f,
                                            3.162109375f,
                                            3.547851562f,
                                            3.980957031f,
                                            4.466796875f,
                                            5.011718750f,
                                            5.652343750f};        /* +15dB gain */
#endif
/************************************************************************************/
/*                                                                                  */
/*  Cosone polynomial coefficients                                                  */
/*                                                                                  */
/************************************************************************************/

/*
 * Coefficients for calculating the cosine with the equation:
 *
 *  Cos(x) = (2^Shifts)*(a0 + a1*x + a2*x^2 + a3*x^3 + a4*x^4 + a5*x^5)
 *
 * These coefficients expect the input, x, to be in the range 0 to 32768 respresenting
 * a range of 0 to Pi. The output is in the range 32767 to -32768 representing the range
 * +1.0 to -1.0
 */
const LVM_INT16     LVPSA_CosCoef[] = { 3,                             /* Shifts */
                                        4096,                          /* a0 */
                                        -36,                           /* a1 */
                                        -19725,                        /* a2 */
                                        -2671,                         /* a3 */
                                        23730,                         /* a4 */
                                        -9490};                        /* a5 */
#ifdef BUILD_FLOAT
const LVM_FLOAT     LVPSA_Float_CosCoef[] = { 3,                             /* Shifts */
                                              0.1250038f,                          /* a0 */
                                              -0.0010986f,                           /* a1 */
                                              -0.6019775f,                        /* a2 */
                                              -0.0815149f,                         /* a3 */
                                              0.7242042f,                         /* a4 */
                                              -0.2896206f};                        /* a5 */
#endif
/*
 * Coefficients for calculating the cosine error with the equation:
 *
 *  CosErr(x) = (2^Shifts)*(a0 + a1*x + a2*x^2 + a3*x^3)
 *
 * These coefficients expect the input, x, to be in the range 0 to 32768 respresenting
 * a range of 0 to Pi/25. The output is in the range 0 to 32767 representing the range
 * 0.0 to 0.0078852986
 *
 * This is used to give a double precision cosine over the range 0 to Pi/25 using the
 * the equation:
 *
 * Cos(x) = 1.0 - CosErr(x)
 */
const LVM_INT16     LVPSA_DPCosCoef[] = {   1,                           /* Shifts */
                                            0,                           /* a0 */
                                            -6,                          /* a1 */
                                            16586,                       /* a2 */
                                            -44};                        /* a3 */
#ifdef BUILD_FLOAT
const LVM_FLOAT    LVPSA_Float_DPCosCoef[] = {1.0f,                        /* Shifts */
                                              0.0f,                        /* a0 */
                                              -0.00008311f,                 /* a1 */
                                              0.50617999f,                 /* a2 */
                                              -0.00134281f};                /* a3 */
#endif
/************************************************************************************/
/*                                                                                  */
/*  Quasi peak filter coefficients table                                            */
/*                                                                                  */
/************************************************************************************/
const QPD_C32_Coefs     LVPSA_QPD_Coefs[] = {

                                         {0x80CEFD2B,0x00CB9B17},  /* 8kS/s  */    /* LVPSA_SPEED_LOW   */
                                         {0x80D242E7,0x00CED11D},
                                         {0x80DCBAF5,0x00D91679},
                                         {0x80CEFD2B,0x00CB9B17},
                                         {0x80E13739,0x00DD7CD3},
                                         {0x80DCBAF5,0x00D91679},
                                         {0x80D94BAF,0x00D5B7E7},
                                         {0x80E13739,0x00DD7CD3},
                                         {0x80DCBAF5,0x00D91679},  /* 48kS/s */

                                         {0x8587513D,0x055C22CF},  /* 8kS/s  */    /* LVPSA_SPEED_MEDIUM      */
                                         {0x859D2967,0x0570F007},
                                         {0x85E2EFAC,0x05B34D79},
                                         {0x8587513D,0x055C22CF},
                                         {0x8600C7B9,0x05CFA6CF},
                                         {0x85E2EFAC,0x05B34D79},
                                         {0x85CC1018,0x059D8F69},
                                         {0x8600C7B9,0x05CFA6CF},//{0x8600C7B9,0x05CFA6CF},
                                         {0x85E2EFAC,0x05B34D79},  /* 48kS/s */

                                         {0xA115EA7A,0x1CDB3F5C},  /* 8kS/s  */   /* LVPSA_SPEED_HIGH      */
                                         {0xA18475F0,0x1D2C83A2},
                                         {0xA2E1E950,0x1E2A532E},
                                         {0xA115EA7A,0x1CDB3F5C},
                                         {0xA375B2C6,0x1E943BBC},
                                         {0xA2E1E950,0x1E2A532E},
                                         {0xA26FF6BD,0x1DD81530},
                                         {0xA375B2C6,0x1E943BBC},
                                         {0xA2E1E950,0x1E2A532E}}; /* 48kS/s */

#ifdef BUILD_FLOAT
const QPD_FLOAT_Coefs     LVPSA_QPD_Float_Coefs[] = {

                                         /* 8kS/s  */    /* LVPSA_SPEED_LOW   */
                                         {-0.9936831989325583f,0.0062135565094650f},
                                         {-0.9935833332128823f,0.0063115493394434f},
                                         {-0.9932638457976282f,0.0066249934025109f},
                                         {-0.9936831989325583f,0.0062135565094650f},
                                         {-0.9931269618682563f,0.0067592649720609f},
                                         {-0.9932638457976282f,0.0066249934025109f},
                                         {-0.9933686633594334f,0.0065221670083702f},
                                         {-0.9931269618682563f,0.0067592649720609f},
                                          /* 48kS/s */
                                         {-0.9932638457976282f,0.0066249934025109f},
#ifdef HIGHER_FS
                                         {-0.9931269618682563f,0.0067592649720609f},
                                         {-0.9932638457976282f,0.0066249934025109f},
                                         {-0.9931269618682563f,0.0067592649720609f},
                                         {-0.9932638457976282f,0.0066249934025109f},
#endif
                                         /* 8kS/s  */    /* LVPSA_SPEED_MEDIUM      */
                                         {-0.9568079425953329f,0.0418742666952312f},
                                         {-0.9561413046903908f,0.0425090822391212f},
                                         {-0.9540119562298059f,0.0445343819446862f},
                                         {-0.9568079425953329f,0.0418742666952312f},
                                         {-0.9531011912040412f,0.0453995238058269f},
                                         {-0.9540119562298059f,0.0445343819446862f},
                                         {-0.9547099955379963f,0.0438708555884659f},
                                          //{0x8600C7B9,0x05CFA6CF},
                                         {-0.9531011912040412f,0.0453995238058269f},
                                          /* 48kS/s */
                                         {-0.9540119562298059f,0.0445343819446862f},
#ifdef HIGHER_FS
                                         {-0.9531011912040412f,0.0453995238058269f},
                                         {-0.9540119562298059f,0.0445343819446862f},
                                         {-0.9531011912040412f,0.0453995238058269f},
                                         {-0.9540119562298059f,0.0445343819446862f},
#endif
                                          /* 8kS/s  */   /* LVPSA_SPEED_HIGH      */
                                         {-0.7415186790749431f,0.2254409026354551f},
                                         {-0.7381451204419136f,0.2279209652915597f},
                                         {-0.7274807319045067f,0.2356666540727019f},
                                         {-0.7415186790749431f,0.2254409026354551f},
                                         {-0.7229706319049001f,0.2388987224549055f},
                                         {-0.7274807319045067f,0.2356666540727019f},
                                         {-0.7309581353329122f,0.2331568226218224f},
                                         {-0.7229706319049001f,0.2388987224549055f},
                                           /* 48kS/s */
                                         {-0.7274807319045067f,0.2356666540727019f}
#ifdef HIGHER_FS
                                        ,{-0.7229706319049001f,0.2388987224549055f}
                                        ,{-0.7274807319045067f,0.2356666540727019f}
                                        ,{-0.7229706319049001f,0.2388987224549055f}
                                        ,{-0.7274807319045067f,0.2356666540727019f}
#endif
                                        };
#endif
