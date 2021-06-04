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

#ifndef __LVM_COEFFS_H__
#define __LVM_COEFFS_H__


/************************************************************************************/
/*                                                                                  */
/* High Pass Shelving Filter coefficients                                           */
/*                                                                                  */
/************************************************************************************/

#define TrebleBoostCorner                                  8000
#define TrebleBoostMinRate                                    4
#define TrebleBoostSteps                                     15

#ifdef BUILD_FLOAT
/* Coefficients for sample rate 22050Hz */
                                                                    /* Gain =  1.000000 dB */
#define HPF_Fs22050_Gain1_A0                            1.038434
#define HPF_Fs22050_Gain1_A1                            0.331599
#define HPF_Fs22050_Gain1_A2                            0.000000
#define HPF_Fs22050_Gain1_B1                            0.370033
#define HPF_Fs22050_Gain1_B2                            0.000000
                                                                    /* Gain =  2.000000 dB */
#define HPF_Fs22050_Gain2_A0                            1.081557
#define HPF_Fs22050_Gain2_A1                            0.288475
#define HPF_Fs22050_Gain2_A2                            0.000000
#define HPF_Fs22050_Gain2_B1                            0.370033
#define HPF_Fs22050_Gain2_B2                            0.000000
                                                                    /* Gain =  3.000000 dB */
#define HPF_Fs22050_Gain3_A0                            1.129943
#define HPF_Fs22050_Gain3_A1                            0.240090
#define HPF_Fs22050_Gain3_A2                            0.000000
#define HPF_Fs22050_Gain3_B1                            0.370033
#define HPF_Fs22050_Gain3_B2                            0.000000
                                                                    /* Gain =  4.000000 dB */
#define HPF_Fs22050_Gain4_A0                            1.184232
#define HPF_Fs22050_Gain4_A1                            0.185801
#define HPF_Fs22050_Gain4_A2                            0.000000
#define HPF_Fs22050_Gain4_B1                            0.370033
#define HPF_Fs22050_Gain4_B2                            0.000000
                                                                    /* Gain =  5.000000 dB */
#define HPF_Fs22050_Gain5_A0                            1.245145
#define HPF_Fs22050_Gain5_A1                            0.124887
#define HPF_Fs22050_Gain5_A2                            0.000000
#define HPF_Fs22050_Gain5_B1                            0.370033
#define HPF_Fs22050_Gain5_B2                            0.000000
                                                                    /* Gain =  6.000000 dB */
#define HPF_Fs22050_Gain6_A0                            1.313491
#define HPF_Fs22050_Gain6_A1                            0.056541
#define HPF_Fs22050_Gain6_A2                            0.000000
#define HPF_Fs22050_Gain6_B1                            0.370033
#define HPF_Fs22050_Gain6_B2                            0.000000
                                                                    /* Gain =  7.000000 dB */
#define HPF_Fs22050_Gain7_A0                            1.390177
#define HPF_Fs22050_Gain7_A1                            (-0.020144)
#define HPF_Fs22050_Gain7_A2                            0.000000
#define HPF_Fs22050_Gain7_B1                            0.370033
#define HPF_Fs22050_Gain7_B2                            0.000000
                                                                    /* Gain =  8.000000 dB */
#define HPF_Fs22050_Gain8_A0                            1.476219
#define HPF_Fs22050_Gain8_A1                            (-0.106187)
#define HPF_Fs22050_Gain8_A2                            0.000000
#define HPF_Fs22050_Gain8_B1                            0.370033
#define HPF_Fs22050_Gain8_B2                            0.000000
                                                                    /* Gain =  9.000000 dB */
#define HPF_Fs22050_Gain9_A0                            1.572761
#define HPF_Fs22050_Gain9_A1                            (-0.202728)
#define HPF_Fs22050_Gain9_A2                            0.000000
#define HPF_Fs22050_Gain9_B1                            0.370033
#define HPF_Fs22050_Gain9_B2                            0.000000
                                                                    /* Gain =  10.000000 dB */
#define HPF_Fs22050_Gain10_A0                           1.681082
#define HPF_Fs22050_Gain10_A1                           (-0.311049)
#define HPF_Fs22050_Gain10_A2                           0.000000
#define HPF_Fs22050_Gain10_B1                           0.370033
#define HPF_Fs22050_Gain10_B2                           0.000000
                                                                    /* Gain =  11.000000 dB */
#define HPF_Fs22050_Gain11_A0                           1.802620
#define HPF_Fs22050_Gain11_A1                           (-0.432588)
#define HPF_Fs22050_Gain11_A2                           0.000000
#define HPF_Fs22050_Gain11_B1                           0.370033
#define HPF_Fs22050_Gain11_B2                           0.000000
                                                                    /* Gain =  12.000000 dB */
#define HPF_Fs22050_Gain12_A0                           1.938989
#define HPF_Fs22050_Gain12_A1                           (-0.568956)
#define HPF_Fs22050_Gain12_A2                           0.000000
#define HPF_Fs22050_Gain12_B1                           0.370033
#define HPF_Fs22050_Gain12_B2                           0.000000
                                                                    /* Gain =  13.000000 dB */
#define HPF_Fs22050_Gain13_A0                           2.091997
#define HPF_Fs22050_Gain13_A1                           (-0.721964)
#define HPF_Fs22050_Gain13_A2                           0.000000
#define HPF_Fs22050_Gain13_B1                           0.370033
#define HPF_Fs22050_Gain13_B2                           0.000000
                                                                    /* Gain =  14.000000 dB */
#define HPF_Fs22050_Gain14_A0                           2.263674
#define HPF_Fs22050_Gain14_A1                           (-0.893641)
#define HPF_Fs22050_Gain14_A2                           0.000000
#define HPF_Fs22050_Gain14_B1                           0.370033
#define HPF_Fs22050_Gain14_B2                           0.000000
                                                                    /* Gain =  15.000000 dB */
#define HPF_Fs22050_Gain15_A0                           2.456300
#define HPF_Fs22050_Gain15_A1                           (-1.086267)
#define HPF_Fs22050_Gain15_A2                           0.000000
#define HPF_Fs22050_Gain15_B1                           0.370033
#define HPF_Fs22050_Gain15_B2                           0.000000
/* Coefficients for sample rate 24000Hz */
                                                                    /* Gain =  1.000000 dB */
#define HPF_Fs24000_Gain1_A0                            1.044662
#define HPF_Fs24000_Gain1_A1                            0.223287
#define HPF_Fs24000_Gain1_A2                            0.000000
#define HPF_Fs24000_Gain1_B1                            0.267949
#define HPF_Fs24000_Gain1_B2                            0.000000
                                                                    /* Gain =  2.000000 dB */
#define HPF_Fs24000_Gain2_A0                            1.094773
#define HPF_Fs24000_Gain2_A1                            0.173176
#define HPF_Fs24000_Gain2_A2                            0.000000
#define HPF_Fs24000_Gain2_B1                            0.267949
#define HPF_Fs24000_Gain2_B2                            0.000000
                                                                    /* Gain =  3.000000 dB */
#define HPF_Fs24000_Gain3_A0                            1.150999
#define HPF_Fs24000_Gain3_A1                            0.116950
#define HPF_Fs24000_Gain3_A2                            0.000000
#define HPF_Fs24000_Gain3_B1                            0.267949
#define HPF_Fs24000_Gain3_B2                            0.000000
                                                                    /* Gain =  4.000000 dB */
#define HPF_Fs24000_Gain4_A0                            1.214086
#define HPF_Fs24000_Gain4_A1                            0.053863
#define HPF_Fs24000_Gain4_A2                            0.000000
#define HPF_Fs24000_Gain4_B1                            0.267949
#define HPF_Fs24000_Gain4_B2                            0.000000
                                                                    /* Gain =  5.000000 dB */
#define HPF_Fs24000_Gain5_A0                            1.284870
#define HPF_Fs24000_Gain5_A1                            (-0.016921)
#define HPF_Fs24000_Gain5_A2                            0.000000
#define HPF_Fs24000_Gain5_B1                            0.267949
#define HPF_Fs24000_Gain5_B2                            0.000000
                                                                    /* Gain =  6.000000 dB */
#define HPF_Fs24000_Gain6_A0                           1.364291
#define HPF_Fs24000_Gain6_A1                           (-0.096342)
#define HPF_Fs24000_Gain6_A2                           0.000000
#define HPF_Fs24000_Gain6_B1                           0.267949
#define HPF_Fs24000_Gain6_B2                           0.000000
                                                                    /* Gain =  7.000000 dB */
#define HPF_Fs24000_Gain7_A0                            1.453403
#define HPF_Fs24000_Gain7_A1                            (-0.185454)
#define HPF_Fs24000_Gain7_A2                            0.000000
#define HPF_Fs24000_Gain7_B1                            0.267949
#define HPF_Fs24000_Gain7_B2                            0.000000
                                                                    /* Gain =  8.000000 dB */
#define HPF_Fs24000_Gain8_A0                            1.553389
#define HPF_Fs24000_Gain8_A1                            (-0.285440)
#define HPF_Fs24000_Gain8_A2                            0.000000
#define HPF_Fs24000_Gain8_B1                            0.267949
#define HPF_Fs24000_Gain8_B2                            0.000000
                                                                    /* Gain =  9.000000 dB */
#define HPF_Fs24000_Gain9_A0                            1.665574
#define HPF_Fs24000_Gain9_A1                            (-0.397625)
#define HPF_Fs24000_Gain9_A2                            0.000000
#define HPF_Fs24000_Gain9_B1                            0.267949
#define HPF_Fs24000_Gain9_B2                            0.000000
                                                                    /* Gain =  10.000000 dB */
#define HPF_Fs24000_Gain10_A0                           1.791449
#define HPF_Fs24000_Gain10_A1                           (-0.523499)
#define HPF_Fs24000_Gain10_A2                           0.000000
#define HPF_Fs24000_Gain10_B1                           0.267949
#define HPF_Fs24000_Gain10_B2                           0.000000
                                                                    /* Gain =  11.000000 dB */
#define HPF_Fs24000_Gain11_A0                           1.932682
#define HPF_Fs24000_Gain11_A1                           (-0.664733)
#define HPF_Fs24000_Gain11_A2                           0.000000
#define HPF_Fs24000_Gain11_B1                           0.267949
#define HPF_Fs24000_Gain11_B2                           0.000000
                                                                    /* Gain =  12.000000 dB */
#define HPF_Fs24000_Gain12_A0                           2.091148
#define HPF_Fs24000_Gain12_A1                           (-0.823199)
#define HPF_Fs24000_Gain12_A2                           0.000000
#define HPF_Fs24000_Gain12_B1                           0.267949
#define HPF_Fs24000_Gain12_B2                           0.000000
                                                                    /* Gain =  13.000000 dB */
#define HPF_Fs24000_Gain13_A0                           2.268950
#define HPF_Fs24000_Gain13_A1                           (-1.001001)
#define HPF_Fs24000_Gain13_A2                           0.000000
#define HPF_Fs24000_Gain13_B1                           0.267949
#define HPF_Fs24000_Gain13_B2                           0.000000
                                                                    /* Gain =  14.000000 dB */
#define HPF_Fs24000_Gain14_A0                           2.468447
#define HPF_Fs24000_Gain14_A1                           (-1.200498)
#define HPF_Fs24000_Gain14_A2                           0.000000
#define HPF_Fs24000_Gain14_B1                           0.267949
#define HPF_Fs24000_Gain14_B2                           0.000000
                                                                    /* Gain =  15.000000 dB */
#define HPF_Fs24000_Gain15_A0                           2.692287
#define HPF_Fs24000_Gain15_A1                           (-1.424338)
#define HPF_Fs24000_Gain15_A2                           0.000000
#define HPF_Fs24000_Gain15_B1                           0.267949
#define HPF_Fs24000_Gain15_B2                           0.000000
/* Coefficients for sample rate 32000Hz */
                                                                    /* Gain =  1.000000 dB */
#define HPF_Fs32000_Gain1_A0                            1.061009
#define HPF_Fs32000_Gain1_A1                            (-0.061009)
#define HPF_Fs32000_Gain1_A2                            0.000000
#define HPF_Fs32000_Gain1_B1                            (-0.000000)
#define HPF_Fs32000_Gain1_B2                            0.000000
                                                                    /* Gain =  2.000000 dB */
#define HPF_Fs32000_Gain2_A0                             1.129463
#define HPF_Fs32000_Gain2_A1                             (-0.129463)
#define HPF_Fs32000_Gain2_A2                             0.000000
#define HPF_Fs32000_Gain2_B1                             (-0.000000)
#define HPF_Fs32000_Gain2_B2                             0.000000
                                                                    /* Gain =  3.000000 dB */
#define HPF_Fs32000_Gain3_A0                             1.206267
#define HPF_Fs32000_Gain3_A1                             (-0.206267)
#define HPF_Fs32000_Gain3_A2                             0.000000
#define HPF_Fs32000_Gain3_B1                             (-0.000000)
#define HPF_Fs32000_Gain3_B2                             0.000000
                                                                    /* Gain =  4.000000 dB */
#define HPF_Fs32000_Gain4_A0                            1.292447
#define HPF_Fs32000_Gain4_A1                            (-0.292447)
#define HPF_Fs32000_Gain4_A2                            0.000000
#define HPF_Fs32000_Gain4_B1                            (-0.000000)
#define HPF_Fs32000_Gain4_B2                            0.000000
                                                                    /* Gain =  5.000000 dB */
#define HPF_Fs32000_Gain5_A0                            1.389140
#define HPF_Fs32000_Gain5_A1                            (-0.389140)
#define HPF_Fs32000_Gain5_A2                            0.000000
#define HPF_Fs32000_Gain5_B1                            (-0.000000)
#define HPF_Fs32000_Gain5_B2                            0.000000
                                                                    /* Gain =  6.000000 dB */
#define HPF_Fs32000_Gain6_A0                             1.497631
#define HPF_Fs32000_Gain6_A1                             (-0.497631)
#define HPF_Fs32000_Gain6_A2                             0.000000
#define HPF_Fs32000_Gain6_B1                             (-0.000000)
#define HPF_Fs32000_Gain6_B2                             0.000000
                                                                    /* Gain =  7.000000 dB */
#define HPF_Fs32000_Gain7_A0                             1.619361
#define HPF_Fs32000_Gain7_A1                             (-0.619361)
#define HPF_Fs32000_Gain7_A2                             0.000000
#define HPF_Fs32000_Gain7_B1                             (-0.000000)
#define HPF_Fs32000_Gain7_B2                             0.000000
                                                                    /* Gain =  8.000000 dB */
#define HPF_Fs32000_Gain8_A0                             1.755943
#define HPF_Fs32000_Gain8_A1                             (-0.755943)
#define HPF_Fs32000_Gain8_A2                             0.000000
#define HPF_Fs32000_Gain8_B1                             (-0.000000)
#define HPF_Fs32000_Gain8_B2                             0.000000
                                                                    /* Gain =  9.000000 dB */
#define HPF_Fs32000_Gain9_A0                             1.909191
#define HPF_Fs32000_Gain9_A1                             (-0.909191)
#define HPF_Fs32000_Gain9_A2                             0.000000
#define HPF_Fs32000_Gain9_B1                             (-0.000000)
#define HPF_Fs32000_Gain9_B2                             0.000000
                                                                    /* Gain =  10.000000 dB */
#define HPF_Fs32000_Gain10_A0                            2.081139
#define HPF_Fs32000_Gain10_A1                            (-1.081139)
#define HPF_Fs32000_Gain10_A2                            0.000000
#define HPF_Fs32000_Gain10_B1                            (-0.000000)
#define HPF_Fs32000_Gain10_B2                            0.000000
                                                                    /* Gain =  11.000000 dB */
#define HPF_Fs32000_Gain11_A0                           2.274067
#define HPF_Fs32000_Gain11_A1                           (-1.274067)
#define HPF_Fs32000_Gain11_A2                           0.000000
#define HPF_Fs32000_Gain11_B1                           (-0.000000)
#define HPF_Fs32000_Gain11_B2                           0.000000
                                                                    /* Gain =  12.000000 dB */
#define HPF_Fs32000_Gain12_A0                          2.490536
#define HPF_Fs32000_Gain12_A1                          (-1.490536)
#define HPF_Fs32000_Gain12_A2                          0.000000
#define HPF_Fs32000_Gain12_B1                          (-0.000000)
#define HPF_Fs32000_Gain12_B2                          0.000000
                                                                    /* Gain =  13.000000 dB */
#define HPF_Fs32000_Gain13_A0                           2.733418
#define HPF_Fs32000_Gain13_A1                           (-1.733418)
#define HPF_Fs32000_Gain13_A2                           0.000000
#define HPF_Fs32000_Gain13_B1                           (-0.000000)
#define HPF_Fs32000_Gain13_B2                           0.000000
                                                                    /* Gain =  14.000000 dB */
#define HPF_Fs32000_Gain14_A0                           3.005936
#define HPF_Fs32000_Gain14_A1                           (-2.005936)
#define HPF_Fs32000_Gain14_A2                           0.000000
#define HPF_Fs32000_Gain14_B1                           (-0.000000)
#define HPF_Fs32000_Gain14_B2                           0.000000
                                                                    /* Gain =  15.000000 dB */
#define HPF_Fs32000_Gain15_A0                          3.311707
#define HPF_Fs32000_Gain15_A1                          (-2.311707)
#define HPF_Fs32000_Gain15_A2                          0.000000
#define HPF_Fs32000_Gain15_B1                          (-0.000000)
#define HPF_Fs32000_Gain15_B2                          0.000000
/* Coefficients for sample rate 44100Hz */
                                                                    /* Gain =  1.000000 dB */
#define HPF_Fs44100_Gain1_A0                            1.074364
#define HPF_Fs44100_Gain1_A1                            (-0.293257)
#define HPF_Fs44100_Gain1_A2                            0.000000
#define HPF_Fs44100_Gain1_B1                            (-0.218894)
#define HPF_Fs44100_Gain1_B2                            0.000000
                                                                    /* Gain =  2.000000 dB */
#define HPF_Fs44100_Gain2_A0                            1.157801
#define HPF_Fs44100_Gain2_A1                            (-0.376695)
#define HPF_Fs44100_Gain2_A2                            0.000000
#define HPF_Fs44100_Gain2_B1                            (-0.218894)
#define HPF_Fs44100_Gain2_B2                            0.000000
                                                                    /* Gain =  3.000000 dB */
#define HPF_Fs44100_Gain3_A0                           1.251420
#define HPF_Fs44100_Gain3_A1                           (-0.470313)
#define HPF_Fs44100_Gain3_A2                           0.000000
#define HPF_Fs44100_Gain3_B1                           (-0.218894)
#define HPF_Fs44100_Gain3_B2                           0.000000
                                                                    /* Gain =  4.000000 dB */
#define HPF_Fs44100_Gain4_A0                            1.356461
#define HPF_Fs44100_Gain4_A1                            (-0.575355)
#define HPF_Fs44100_Gain4_A2                            0.000000
#define HPF_Fs44100_Gain4_B1                            (-0.218894)
#define HPF_Fs44100_Gain4_B2                            0.000000
                                                                    /* Gain =  5.000000 dB */
#define HPF_Fs44100_Gain5_A0                            1.474320
#define HPF_Fs44100_Gain5_A1                            (-0.693213)
#define HPF_Fs44100_Gain5_A2                            0.000000
#define HPF_Fs44100_Gain5_B1                            (-0.218894)
#define HPF_Fs44100_Gain5_B2                            0.000000
                                                                    /* Gain =  6.000000 dB */
#define HPF_Fs44100_Gain6_A0                           1.606559
#define HPF_Fs44100_Gain6_A1                           (-0.825453)
#define HPF_Fs44100_Gain6_A2                           0.000000
#define HPF_Fs44100_Gain6_B1                           (-0.218894)
#define HPF_Fs44100_Gain6_B2                           0.000000
                                                                    /* Gain =  7.000000 dB */
#define HPF_Fs44100_Gain7_A0                           1.754935
#define HPF_Fs44100_Gain7_A1                           (-0.973828)
#define HPF_Fs44100_Gain7_A2                           0.000000
#define HPF_Fs44100_Gain7_B1                           (-0.218894)
#define HPF_Fs44100_Gain7_B2                           0.000000
                                                                    /* Gain =  8.000000 dB */
#define HPF_Fs44100_Gain8_A0                            1.921414
#define HPF_Fs44100_Gain8_A1                            (-1.140308)
#define HPF_Fs44100_Gain8_A2                            0.000000
#define HPF_Fs44100_Gain8_B1                            (-0.218894)
#define HPF_Fs44100_Gain8_B2                            0.000000
                                                                    /* Gain =  9.000000 dB */
#define HPF_Fs44100_Gain9_A0                            2.108208
#define HPF_Fs44100_Gain9_A1                            (-1.327101)
#define HPF_Fs44100_Gain9_A2                            0.000000
#define HPF_Fs44100_Gain9_B1                            (-0.218894)
#define HPF_Fs44100_Gain9_B2                            0.000000
                                                                    /* Gain =  10.000000 dB */
#define HPF_Fs44100_Gain10_A0                          2.317793
#define HPF_Fs44100_Gain10_A1                          (-1.536687)
#define HPF_Fs44100_Gain10_A2                          0.000000
#define HPF_Fs44100_Gain10_B1                          (-0.218894)
#define HPF_Fs44100_Gain10_B2                          0.000000
                                                                    /* Gain =  11.000000 dB */
#define HPF_Fs44100_Gain11_A0                          2.552952
#define HPF_Fs44100_Gain11_A1                          (-1.771846)
#define HPF_Fs44100_Gain11_A2                          0.000000
#define HPF_Fs44100_Gain11_B1                          (-0.218894)
#define HPF_Fs44100_Gain11_B2                          0.000000
                                                                    /* Gain =  12.000000 dB */
#define HPF_Fs44100_Gain12_A0                          2.816805
#define HPF_Fs44100_Gain12_A1                          (-2.035698)
#define HPF_Fs44100_Gain12_A2                          0.000000
#define HPF_Fs44100_Gain12_B1                          (-0.218894)
#define HPF_Fs44100_Gain12_B2                          0.000000
                                                                    /* Gain =  13.000000 dB */
#define HPF_Fs44100_Gain13_A0                           3.112852
#define HPF_Fs44100_Gain13_A1                           (-2.331746)
#define HPF_Fs44100_Gain13_A2                           0.000000
#define HPF_Fs44100_Gain13_B1                           (-0.218894)
#define HPF_Fs44100_Gain13_B2                           0.000000
                                                                    /* Gain =  14.000000 dB */
#define HPF_Fs44100_Gain14_A0                          3.445023
#define HPF_Fs44100_Gain14_A1                          (-2.663916)
#define HPF_Fs44100_Gain14_A2                          0.000000
#define HPF_Fs44100_Gain14_B1                          (-0.218894)
#define HPF_Fs44100_Gain14_B2                          0.000000
                                                                    /* Gain =  15.000000 dB */
#define HPF_Fs44100_Gain15_A0                          3.817724
#define HPF_Fs44100_Gain15_A1                          (-3.036618)
#define HPF_Fs44100_Gain15_A2                          0.000000
#define HPF_Fs44100_Gain15_B1                          (-0.218894)
#define HPF_Fs44100_Gain15_B2                          0.000000
/* Coefficients for sample rate 48000Hz */
                                                                    /* Gain =  1.000000 dB */
#define HPF_Fs48000_Gain1_A0                          1.077357
#define HPF_Fs48000_Gain1_A1                          (-0.345306)
#define HPF_Fs48000_Gain1_A2                          0.000000
#define HPF_Fs48000_Gain1_B1                          (-0.267949)
#define HPF_Fs48000_Gain1_B2                          0.000000
                                                                    /* Gain =  2.000000 dB */
#define HPF_Fs48000_Gain2_A0                          1.164152
#define HPF_Fs48000_Gain2_A1                          (-0.432101)
#define HPF_Fs48000_Gain2_A2                          0.000000
#define HPF_Fs48000_Gain2_B1                          (-0.267949)
#define HPF_Fs48000_Gain2_B2                          0.000000
                                                                    /* Gain =  3.000000 dB */
#define HPF_Fs48000_Gain3_A0                          1.261538
#define HPF_Fs48000_Gain3_A1                          (-0.529488)
#define HPF_Fs48000_Gain3_A2                          0.000000
#define HPF_Fs48000_Gain3_B1                          (-0.267949)
#define HPF_Fs48000_Gain3_B2                          0.000000
                                                                    /* Gain =  4.000000 dB */
#define HPF_Fs48000_Gain4_A0                           1.370807
#define HPF_Fs48000_Gain4_A1                           (-0.638757)
#define HPF_Fs48000_Gain4_A2                           0.000000
#define HPF_Fs48000_Gain4_B1                           (-0.267949)
#define HPF_Fs48000_Gain4_B2                           0.000000
                                                                    /* Gain =  5.000000 dB */
#define HPF_Fs48000_Gain5_A0                           1.493409
#define HPF_Fs48000_Gain5_A1                           (-0.761359)
#define HPF_Fs48000_Gain5_A2                           0.000000
#define HPF_Fs48000_Gain5_B1                           (-0.267949)
#define HPF_Fs48000_Gain5_B2                           0.000000
                                                                    /* Gain =  6.000000 dB */
#define HPF_Fs48000_Gain6_A0                            1.630971
#define HPF_Fs48000_Gain6_A1                            (-0.898920)
#define HPF_Fs48000_Gain6_A2                            0.000000
#define HPF_Fs48000_Gain6_B1                            (-0.267949)
#define HPF_Fs48000_Gain6_B2                            0.000000
                                                                    /* Gain =  7.000000 dB */
#define HPF_Fs48000_Gain7_A0                            1.785318
#define HPF_Fs48000_Gain7_A1                            (-1.053267)
#define HPF_Fs48000_Gain7_A2                            0.000000
#define HPF_Fs48000_Gain7_B1                            (-0.267949)
#define HPF_Fs48000_Gain7_B2                            0.000000
                                                                    /* Gain =  8.000000 dB */
#define HPF_Fs48000_Gain8_A0                           1.958498
#define HPF_Fs48000_Gain8_A1                           (-1.226447)
#define HPF_Fs48000_Gain8_A2                           0.000000
#define HPF_Fs48000_Gain8_B1                           (-0.267949)
#define HPF_Fs48000_Gain8_B2                           0.000000
                                                                    /* Gain =  9.000000 dB */
#define HPF_Fs48000_Gain9_A0                          2.152809
#define HPF_Fs48000_Gain9_A1                          (-1.420758)
#define HPF_Fs48000_Gain9_A2                          0.000000
#define HPF_Fs48000_Gain9_B1                          (-0.267949)
#define HPF_Fs48000_Gain9_B2                          0.000000
                                                                    /* Gain =  10.000000 dB */
#define HPF_Fs48000_Gain10_A0                         2.370829
#define HPF_Fs48000_Gain10_A1                         (-1.638778)
#define HPF_Fs48000_Gain10_A2                         0.000000
#define HPF_Fs48000_Gain10_B1                         (-0.267949)
#define HPF_Fs48000_Gain10_B2                         0.000000
                                                                    /* Gain =  11.000000 dB */
#define HPF_Fs48000_Gain11_A0                          2.615452
#define HPF_Fs48000_Gain11_A1                          (-1.883401)
#define HPF_Fs48000_Gain11_A2                          0.000000
#define HPF_Fs48000_Gain11_B1                          (-0.267949)
#define HPF_Fs48000_Gain11_B2                          0.000000
                                                                    /* Gain =  12.000000 dB */
#define HPF_Fs48000_Gain12_A0                          2.889924
#define HPF_Fs48000_Gain12_A1                          (-2.157873)
#define HPF_Fs48000_Gain12_A2                          0.000000
#define HPF_Fs48000_Gain12_B1                          (-0.267949)
#define HPF_Fs48000_Gain12_B2                          0.000000
                                                                    /* Gain =  13.000000 dB */
#define HPF_Fs48000_Gain13_A0                           3.197886
#define HPF_Fs48000_Gain13_A1                           (-2.465835)
#define HPF_Fs48000_Gain13_A2                           0.000000
#define HPF_Fs48000_Gain13_B1                           (-0.267949)
#define HPF_Fs48000_Gain13_B2                           0.000000
                                                                    /* Gain =  14.000000 dB */
#define HPF_Fs48000_Gain14_A0                          3.543425
#define HPF_Fs48000_Gain14_A1                          (-2.811374)
#define HPF_Fs48000_Gain14_A2                          0.000000
#define HPF_Fs48000_Gain14_B1                          (-0.267949)
#define HPF_Fs48000_Gain14_B2                          0.000000
                                                                    /* Gain =  15.000000 dB */
#define HPF_Fs48000_Gain15_A0                         3.931127
#define HPF_Fs48000_Gain15_A1                         (-3.199076)
#define HPF_Fs48000_Gain15_A2                         0.000000
#define HPF_Fs48000_Gain15_B1                         (-0.267949)
#define HPF_Fs48000_Gain15_B2                         0.000000

#ifdef HIGHER_FS
/* Coefficients for sample rate 88200 */
/* Gain = 1.000000 dB */
#define HPF_Fs88200_Gain1_A0                          1.094374f
#define HPF_Fs88200_Gain1_A1                          (-0.641256f)
#define HPF_Fs88200_Gain1_A2                          0.000000f
#define HPF_Fs88200_Gain1_B1                          (-0.546882f)
#define HPF_Fs88200_Gain1_B2                          0.000000f
/* Gain = 2.000000 dB */
#define HPF_Fs88200_Gain2_A0                          1.200264f
#define HPF_Fs88200_Gain2_A1                          (-0.747146f)
#define HPF_Fs88200_Gain2_A2                          0.000000f
#define HPF_Fs88200_Gain2_B1                          (-0.546882f)
#define HPF_Fs88200_Gain2_B2                          0.000000f
/* Gain = 3.000000 dB */
#define HPF_Fs88200_Gain3_A0                          1.319074f
#define HPF_Fs88200_Gain3_A1                          (-0.865956f)
#define HPF_Fs88200_Gain3_A2                          0.000000f
#define HPF_Fs88200_Gain3_B1                          (-0.546882f)
#define HPF_Fs88200_Gain3_B2                          0.000000f
/* Gain = 4.000000 dB */
#define HPF_Fs88200_Gain4_A0                          1.452380f
#define HPF_Fs88200_Gain4_A1                          (-0.999263f)
#define HPF_Fs88200_Gain4_A2                          0.000000f
#define HPF_Fs88200_Gain4_B1                          (-0.546882f)
#define HPF_Fs88200_Gain4_B2                          0.000000f
/* Gain = 5.000000 dB */
#define HPF_Fs88200_Gain5_A0                          1.601953f
#define HPF_Fs88200_Gain5_A1                          (-1.148836f)
#define HPF_Fs88200_Gain5_A2                          0.000000f
#define HPF_Fs88200_Gain5_B1                          (-0.546882f)
#define HPF_Fs88200_Gain5_B2                          0.000000f
/* Gain = 6.000000 dB */
#define HPF_Fs88200_Gain6_A0                          1.769777f
#define HPF_Fs88200_Gain6_A1                          (-1.316659f)
#define HPF_Fs88200_Gain6_A2                          0.000000f
#define HPF_Fs88200_Gain6_B1                          (-0.546882f)
#define HPF_Fs88200_Gain6_B2                          0.000000f
/* Gain = 7.000000 dB */
#define HPF_Fs88200_Gain7_A0                          1.958078f
#define HPF_Fs88200_Gain7_A1                          (-1.504960f)
#define HPF_Fs88200_Gain7_A2                          0.000000f
#define HPF_Fs88200_Gain7_B1                          (-0.546882f)
#define HPF_Fs88200_Gain7_B2                          0.000000f
/* Gain = 8.000000 dB */
#define HPF_Fs88200_Gain8_A0                          2.169355f
#define HPF_Fs88200_Gain8_A1                          (-1.716238f)
#define HPF_Fs88200_Gain8_A2                          0.000000f
#define HPF_Fs88200_Gain8_B1                          (-0.546882f)
#define HPF_Fs88200_Gain8_B2                          0.000000f
/* Gain = 9.000000 dB */
#define HPF_Fs88200_Gain9_A0                          2.406412f
#define HPF_Fs88200_Gain9_A1                          (-1.953295f)
#define HPF_Fs88200_Gain9_A2                          0.000000f
#define HPF_Fs88200_Gain9_B1                          (-0.546882f)
#define HPF_Fs88200_Gain9_B2                          0.000000f
/* Gain = 10.000000 dB */
#define HPF_Fs88200_Gain10_A0                          2.672395f
#define HPF_Fs88200_Gain10_A1                          (-2.219277f)
#define HPF_Fs88200_Gain10_A2                          0.000000f
#define HPF_Fs88200_Gain10_B1                          (-0.546882f)
#define HPF_Fs88200_Gain10_B2                          0.000000f
/* Gain = 11.000000 dB */
#define HPF_Fs88200_Gain11_A0                          2.970832f
#define HPF_Fs88200_Gain11_A1                          (-2.517714f)
#define HPF_Fs88200_Gain11_A2                          0.000000f
#define HPF_Fs88200_Gain11_B1                          (-0.546882f)
#define HPF_Fs88200_Gain11_B2                          0.000000f
/* Gain = 12.000000 dB */
#define HPF_Fs88200_Gain12_A0                          3.305684f
#define HPF_Fs88200_Gain12_A1                          (-2.852566f)
#define HPF_Fs88200_Gain12_A2                          0.000000f
#define HPF_Fs88200_Gain12_B1                          (-0.546882f)
#define HPF_Fs88200_Gain12_B2                          0.000000f
/* Gain = 13.000000 dB */
#define HPF_Fs88200_Gain13_A0                          3.681394f
#define HPF_Fs88200_Gain13_A1                          (-3.228276f)
#define HPF_Fs88200_Gain13_A2                          0.000000f
#define HPF_Fs88200_Gain13_B1                          (-0.546882f)
#define HPF_Fs88200_Gain13_B2                          0.000000f
/* Gain = 14.000000 dB */
#define HPF_Fs88200_Gain14_A0                          4.102947f
#define HPF_Fs88200_Gain14_A1                          (-3.649830f)
#define HPF_Fs88200_Gain14_A2                          0.000000f
#define HPF_Fs88200_Gain14_B1                          (-0.546882f)
#define HPF_Fs88200_Gain14_B2                          0.000000f
/* Gain = 15.000000 dB */
#define HPF_Fs88200_Gain15_A0                          4.575938f
#define HPF_Fs88200_Gain15_A1                          (-4.122820f)
#define HPF_Fs88200_Gain15_A2                          0.000000f
#define HPF_Fs88200_Gain15_B1                          (-0.546882f)
#define HPF_Fs88200_Gain15_B2                          0.000000f

/* Coefficients for sample rate 96000Hz */
                                                                 /* Gain =  1.000000 dB */
#define HPF_Fs96000_Gain1_A0                          1.096233
#define HPF_Fs96000_Gain1_A1                          (-0.673583)
#define HPF_Fs96000_Gain1_A2                          0.000000
#define HPF_Fs96000_Gain1_B1                          (-0.577350)
#define HPF_Fs96000_Gain1_B2                          0.000000
                                                                 /* Gain =  2.000000 dB */
#define HPF_Fs96000_Gain2_A0                          1.204208
#define HPF_Fs96000_Gain2_A1                          (-0.781558)
#define HPF_Fs96000_Gain2_A2                          0.000000
#define HPF_Fs96000_Gain2_B1                          (-0.577350)
#define HPF_Fs96000_Gain2_B2                          0.000000
                                                                 /* Gain =  3.000000 dB */
#define HPF_Fs96000_Gain3_A0                          1.325358
#define HPF_Fs96000_Gain3_A1                          (-0.902708)
#define HPF_Fs96000_Gain3_A2                          0.000000
#define HPF_Fs96000_Gain3_B1                          (-0.577350)
#define HPF_Fs96000_Gain3_B2                          0.000000
                                                                 /* Gain =  4.000000 dB */
#define HPF_Fs96000_Gain4_A0                           1.461291
#define HPF_Fs96000_Gain4_A1                           (-1.038641)
#define HPF_Fs96000_Gain4_A2                           0.000000
#define HPF_Fs96000_Gain4_B1                           (-0.577350)
#define HPF_Fs96000_Gain4_B2                           0.000000
                                                                 /* Gain =  5.000000 dB */
#define HPF_Fs96000_Gain5_A0                           1.613810
#define HPF_Fs96000_Gain5_A1                           (-1.191160)
#define HPF_Fs96000_Gain5_A2                           0.000000
#define HPF_Fs96000_Gain5_B1                           (-0.577350)
#define HPF_Fs96000_Gain5_B2                           0.000000
                                                                 /* Gain =  6.000000 dB */
#define HPF_Fs96000_Gain6_A0                            1.784939
#define HPF_Fs96000_Gain6_A1                            (-1.362289)
#define HPF_Fs96000_Gain6_A2                            0.000000
#define HPF_Fs96000_Gain6_B1                            (-0.577350)
#define HPF_Fs96000_Gain6_B2                            0.000000
                                                                /* Gain =  7.000000 dB */
#define HPF_Fs96000_Gain7_A0                            1.976949
#define HPF_Fs96000_Gain7_A1                            (-1.554299)
#define HPF_Fs96000_Gain7_A2                            0.000000
#define HPF_Fs96000_Gain7_B1                            (-0.577350)
#define HPF_Fs96000_Gain7_B2                            0.000000
                                                                 /* Gain =  8.000000 dB */
#define HPF_Fs96000_Gain8_A0                           2.192387
#define HPF_Fs96000_Gain8_A1                           (-1.769738)
#define HPF_Fs96000_Gain8_A2                           0.000000
#define HPF_Fs96000_Gain8_B1                           (-0.577350)
#define HPF_Fs96000_Gain8_B2                           0.000000
                                                                /* Gain =  9.000000 dB */
#define HPF_Fs96000_Gain9_A0                          2.434113
#define HPF_Fs96000_Gain9_A1                          (-2.011464)
#define HPF_Fs96000_Gain9_A2                          0.000000
#define HPF_Fs96000_Gain9_B1                          (-0.577350)
#define HPF_Fs96000_Gain9_B2                          0.000000
                                                               /* Gain =  10.000000 dB */
#define HPF_Fs96000_Gain10_A0                        2.705335
#define HPF_Fs96000_Gain10_A1                        (-2.282685)
#define HPF_Fs96000_Gain10_A2                         0.000000
#define HPF_Fs96000_Gain10_B1                         (-0.577350)
#define HPF_Fs96000_Gain10_B2                         0.000000
                                                              /* Gain =  11.000000 dB */
#define HPF_Fs96000_Gain11_A0                          3.009650
#define HPF_Fs96000_Gain11_A1                          (-2.587000)
#define HPF_Fs96000_Gain11_A2                          0.000000
#define HPF_Fs96000_Gain11_B1                          (-0.577350)
#define HPF_Fs96000_Gain11_B2                          0.000000
                                                                  /* Gain =  12.000000 dB */
#define HPF_Fs96000_Gain12_A0                          3.351097
#define HPF_Fs96000_Gain12_A1                          (-2.928447)
#define HPF_Fs96000_Gain12_A2                          0.000000
#define HPF_Fs96000_Gain12_B1                          (-0.577350)
#define HPF_Fs96000_Gain12_B2                          0.000000
                                                                /* Gain =  13.000000 dB */
#define HPF_Fs96000_Gain13_A0                           3.734207
#define HPF_Fs96000_Gain13_A1                           (-3.311558)
#define HPF_Fs96000_Gain13_A2                           0.000000
#define HPF_Fs96000_Gain13_B1                           (-0.577350)
#define HPF_Fs96000_Gain13_B2                           0.000000
                                                                 /* Gain =  14.000000 dB */
#define HPF_Fs96000_Gain14_A0                         4.164064
#define HPF_Fs96000_Gain14_A1                         (-3.741414)
#define HPF_Fs96000_Gain14_A2                          0.000000
#define HPF_Fs96000_Gain14_B1                          (-0.577350)
#define HPF_Fs96000_Gain14_B2                          0.000000
                                                                 /* Gain =  15.000000 dB */
#define HPF_Fs96000_Gain15_A0                         4.646371
#define HPF_Fs96000_Gain15_A1                         (-4.223721)
#define HPF_Fs96000_Gain15_A2                         0.000000
#define HPF_Fs96000_Gain15_B1                         (-0.577350)
#define HPF_Fs96000_Gain15_B2                         0.000000

/* Coefficients for sample rate 176400 */
/* Gain = 1.000000 dB */
#define HPF_Fs176400_Gain1_A0                          1.106711f
#define HPF_Fs176400_Gain1_A1                          (-0.855807f)
#define HPF_Fs176400_Gain1_A2                          0.000000f
#define HPF_Fs176400_Gain1_B1                          (-0.749096f)
#define HPF_Fs176400_Gain1_B2                          0.000000f
/* Gain = 2.000000 dB */
#define HPF_Fs176400_Gain2_A0                          1.226443f
#define HPF_Fs176400_Gain2_A1                          (-0.975539f)
#define HPF_Fs176400_Gain2_A2                          0.000000f
#define HPF_Fs176400_Gain2_B1                          (-0.749096f)
#define HPF_Fs176400_Gain2_B2                          0.000000f
/* Gain = 3.000000 dB */
#define HPF_Fs176400_Gain3_A0                          1.360784f
#define HPF_Fs176400_Gain3_A1                          (-1.109880f)
#define HPF_Fs176400_Gain3_A2                          0.000000f
#define HPF_Fs176400_Gain3_B1                          (-0.749096f)
#define HPF_Fs176400_Gain3_B2                          0.000000f
/* Gain = 4.000000 dB */
#define HPF_Fs176400_Gain4_A0                          1.511517f
#define HPF_Fs176400_Gain4_A1                          (-1.260613f)
#define HPF_Fs176400_Gain4_A2                          0.000000f
#define HPF_Fs176400_Gain4_B1                          (-0.749096f)
#define HPF_Fs176400_Gain4_B2                          0.000000f
/* Gain = 5.000000 dB */
#define HPF_Fs176400_Gain5_A0                          1.680643f
#define HPF_Fs176400_Gain5_A1                          (-1.429739f)
#define HPF_Fs176400_Gain5_A2                          0.000000f
#define HPF_Fs176400_Gain5_B1                          (-0.749096f)
#define HPF_Fs176400_Gain5_B2                          0.000000f
/* Gain = 6.000000 dB */
#define HPF_Fs176400_Gain6_A0                          1.870405f
#define HPF_Fs176400_Gain6_A1                          (-1.619501f)
#define HPF_Fs176400_Gain6_A2                          0.000000f
#define HPF_Fs176400_Gain6_B1                          (-0.749096f)
#define HPF_Fs176400_Gain6_B2                          0.000000f
/* Gain = 7.000000 dB */
#define HPF_Fs176400_Gain7_A0                          2.083321f
#define HPF_Fs176400_Gain7_A1                          (-1.832417f)
#define HPF_Fs176400_Gain7_A2                          0.000000f
#define HPF_Fs176400_Gain7_B1                          (-0.749096f)
#define HPF_Fs176400_Gain7_B2                          0.000000f
/* Gain = 8.000000 dB */
#define HPF_Fs176400_Gain8_A0                          2.322217f
#define HPF_Fs176400_Gain8_A1                          (-2.071313f)
#define HPF_Fs176400_Gain8_A2                          0.000000f
#define HPF_Fs176400_Gain8_B1                          (-0.749096f)
#define HPF_Fs176400_Gain8_B2                          0.000000f
/* Gain = 9.000000 dB */
#define HPF_Fs176400_Gain9_A0                          2.590263f
#define HPF_Fs176400_Gain9_A1                          (-2.339359f)
#define HPF_Fs176400_Gain9_A2                          0.000000f
#define HPF_Fs176400_Gain9_B1                          (-0.749096f)
#define HPF_Fs176400_Gain9_B2                          0.000000f
/* Gain = 10.000000 dB */
#define HPF_Fs176400_Gain10_A0                          2.891016f
#define HPF_Fs176400_Gain10_A1                          (-2.640112f)
#define HPF_Fs176400_Gain10_A2                          0.000000f
#define HPF_Fs176400_Gain10_B1                          (-0.749096f)
#define HPF_Fs176400_Gain10_B2                          0.000000f
/* Gain = 11.000000 dB */
#define HPF_Fs176400_Gain11_A0                          3.228465f
#define HPF_Fs176400_Gain11_A1                          (-2.977561f)
#define HPF_Fs176400_Gain11_A2                          0.000000f
#define HPF_Fs176400_Gain11_B1                          (-0.749096f)
#define HPF_Fs176400_Gain11_B2                          0.000000f
/* Gain = 12.000000 dB */
#define HPF_Fs176400_Gain12_A0                          3.607090f
#define HPF_Fs176400_Gain12_A1                          (-3.356186f)
#define HPF_Fs176400_Gain12_A2                          0.000000f
#define HPF_Fs176400_Gain12_B1                          (-0.749096f)
#define HPF_Fs176400_Gain12_B2                          0.000000f
/* Gain = 13.000000 dB */
#define HPF_Fs176400_Gain13_A0                          4.031914f
#define HPF_Fs176400_Gain13_A1                          (-3.781010f)
#define HPF_Fs176400_Gain13_A2                          0.000000f
#define HPF_Fs176400_Gain13_B1                          (-0.749096f)
#define HPF_Fs176400_Gain13_B2                          0.000000f
/* Gain = 14.000000 dB */
#define HPF_Fs176400_Gain14_A0                          4.508575f
#define HPF_Fs176400_Gain14_A1                          (-4.257671f)
#define HPF_Fs176400_Gain14_A2                          0.000000f
#define HPF_Fs176400_Gain14_B1                          (-0.749096f)
#define HPF_Fs176400_Gain14_B2                          0.000000f
/* Gain = 15.000000 dB */
#define HPF_Fs176400_Gain15_A0                          5.043397f
#define HPF_Fs176400_Gain15_A1                          (-4.792493f)
#define HPF_Fs176400_Gain15_A2                          0.000000f
#define HPF_Fs176400_Gain15_B1                          (-0.749096f)
#define HPF_Fs176400_Gain15_B2                          0.000000f

/* Coefficients for sample rate 192000Hz */
                                                                  /* Gain =  1.000000 dB */
#define HPF_Fs192000_Gain1_A0                          1.107823
#define HPF_Fs192000_Gain1_A1                          (-0.875150)
#define HPF_Fs192000_Gain1_A2                          0.000000
#define HPF_Fs192000_Gain1_B1                          (-0.767327)
#define HPF_Fs192000_Gain1_B2                          0.000000
                                                                  /* Gain =  2.000000 dB */
#define HPF_Fs192000_Gain2_A0                          1.228803
#define HPF_Fs192000_Gain2_A1                          (-0.996130)
#define HPF_Fs192000_Gain2_A2                          0.000000
#define HPF_Fs192000_Gain2_B1                          (-0.767327)
#define HPF_Fs192000_Gain2_B2                          0.000000
                                                                   /* Gain =  3.000000 dB */
#define HPF_Fs192000_Gain3_A0                          1.364544
#define HPF_Fs192000_Gain3_A1                          (-1.131871)
#define HPF_Fs192000_Gain3_A2                          0.000000
#define HPF_Fs192000_Gain3_B1                          (-0.767327)
#define HPF_Fs192000_Gain3_B2                          0.000000
                                                                   /* Gain =  4.000000 dB */
#define HPF_Fs192000_Gain4_A0                          1.516849
#define HPF_Fs192000_Gain4_A1                          (-1.284176)
#define HPF_Fs192000_Gain4_A2                           0.000000
#define HPF_Fs192000_Gain4_B1                           (-0.767327)
#define HPF_Fs192000_Gain4_B2                           0.000000
                                                                   /* Gain =  5.000000 dB */
#define HPF_Fs192000_Gain5_A0                           1.687737
#define HPF_Fs192000_Gain5_A1                           (-1.455064)
#define HPF_Fs192000_Gain5_A2                           0.000000
#define HPF_Fs192000_Gain5_B1                           (-0.767327)
#define HPF_Fs192000_Gain5_B2                           0.000000
                                                                   /* Gain =  6.000000 dB */
#define HPF_Fs192000_Gain6_A0                            1.879477
#define HPF_Fs192000_Gain6_A1                            (-1.646804)
#define HPF_Fs192000_Gain6_A2                            0.000000
#define HPF_Fs192000_Gain6_B1                            (-0.767327)
#define HPF_Fs192000_Gain6_B2                            0.000000
                                                                 /* Gain =  7.000000 dB */
#define HPF_Fs192000_Gain7_A0                            2.094613
#define HPF_Fs192000_Gain7_A1                            (-1.861940)
#define HPF_Fs192000_Gain7_A2                            0.000000
#define HPF_Fs192000_Gain7_B1                            (-0.767327)
#define HPF_Fs192000_Gain7_B2                            0.000000
                                                                   /* Gain =  8.000000 dB */
#define HPF_Fs192000_Gain8_A0                           2.335999
#define HPF_Fs192000_Gain8_A1                           (-2.103326)
#define HPF_Fs192000_Gain8_A2                           0.000000
#define HPF_Fs192000_Gain8_B1                           (-0.767327)
#define HPF_Fs192000_Gain8_B2                           0.000000
                                                                   /* Gain =  9.000000 dB */
#define HPF_Fs192000_Gain9_A0                          2.606839
#define HPF_Fs192000_Gain9_A1                          (-2.374166)
#define HPF_Fs192000_Gain9_A2                          0.000000
#define HPF_Fs192000_Gain9_B1                          (-0.767327)
#define HPF_Fs192000_Gain9_B2                          0.000000
                                                                 /* Gain =  10.000000 dB */
#define HPF_Fs192000_Gain10_A0                        2.910726
#define HPF_Fs192000_Gain10_A1                        (-2.678053)
#define HPF_Fs192000_Gain10_A2                         0.000000
#define HPF_Fs192000_Gain10_B1                         (-0.767327)
#define HPF_Fs192000_Gain10_B2                         0.000000
                                                                  /* Gain =  11.000000 dB */
#define HPF_Fs192000_Gain11_A0                          3.251693
#define HPF_Fs192000_Gain11_A1                          (-3.019020)
#define HPF_Fs192000_Gain11_A2                          0.000000
#define HPF_Fs192000_Gain11_B1                          (-0.767327)
#define HPF_Fs192000_Gain11_B2                          0.000000
                                                                  /* Gain =  12.000000 dB */
#define HPF_Fs192000_Gain12_A0                          3.634264
#define HPF_Fs192000_Gain12_A1                          (-3.401591)
#define HPF_Fs192000_Gain12_A2                          0.000000
#define HPF_Fs192000_Gain12_B1                          (-0.767327)
#define HPF_Fs192000_Gain12_B2                          0.000000
                                                                /* Gain =  13.000000 dB */
#define HPF_Fs192000_Gain13_A0                           4.063516
#define HPF_Fs192000_Gain13_A1                           (-3.830843)
#define HPF_Fs192000_Gain13_A2                           0.000000
#define HPF_Fs192000_Gain13_B1                           (-0.767327)
#define HPF_Fs192000_Gain13_B2                           0.000000
                                                                /* Gain =  14.000000 dB */
#define HPF_Fs192000_Gain14_A0                          4.545145
#define HPF_Fs192000_Gain14_A1                          (-4.312472)
#define HPF_Fs192000_Gain14_A2                          0.000000
#define HPF_Fs192000_Gain14_B1                          (-0.767327)
#define HPF_Fs192000_Gain14_B2                          0.000000
                                                                  /* Gain =  15.000000 dB */
#define HPF_Fs192000_Gain15_A0                         5.085542
#define HPF_Fs192000_Gain15_A1                         (-4.852868)
#define HPF_Fs192000_Gain15_A2                         0.000000
#define HPF_Fs192000_Gain15_B1                         (-0.767327)
#define HPF_Fs192000_Gain15_B2                         0.000000

#endif

#else
/* Coefficients for sample rate 22050Hz */
                                                                    /* Gain =  1.000000 dB */
#define HPF_Fs22050_Gain1_A0                             5383         /* Floating point value 0.164291 */
#define HPF_Fs22050_Gain1_A1                            16859         /* Floating point value 0.514492 */
#define HPF_Fs22050_Gain1_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain1_B1                            12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain1_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain1_Shift                             1         /* Shift value */
                                                                    /* Gain =  2.000000 dB */
#define HPF_Fs22050_Gain2_A0                             4683         /* Floating point value 0.142925 */
#define HPF_Fs22050_Gain2_A1                            17559         /* Floating point value 0.535858 */
#define HPF_Fs22050_Gain2_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain2_B1                            12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain2_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain2_Shift                             1         /* Shift value */
                                                                    /* Gain =  3.000000 dB */
#define HPF_Fs22050_Gain3_A0                             3898         /* Floating point value 0.118953 */
#define HPF_Fs22050_Gain3_A1                            18345         /* Floating point value 0.559830 */
#define HPF_Fs22050_Gain3_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain3_B1                            12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain3_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain3_Shift                             1         /* Shift value */
                                                                    /* Gain =  4.000000 dB */
#define HPF_Fs22050_Gain4_A0                             3016         /* Floating point value 0.092055 */
#define HPF_Fs22050_Gain4_A1                            19226         /* Floating point value 0.586728 */
#define HPF_Fs22050_Gain4_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain4_B1                            12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain4_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain4_Shift                             1         /* Shift value */
                                                                    /* Gain =  5.000000 dB */
#define HPF_Fs22050_Gain5_A0                             2028         /* Floating point value 0.061876 */
#define HPF_Fs22050_Gain5_A1                            20215         /* Floating point value 0.616907 */
#define HPF_Fs22050_Gain5_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain5_B1                            12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain5_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain5_Shift                             1         /* Shift value */
                                                                    /* Gain =  6.000000 dB */
#define HPF_Fs22050_Gain6_A0                              918         /* Floating point value 0.028013 */
#define HPF_Fs22050_Gain6_A1                            21324         /* Floating point value 0.650770 */
#define HPF_Fs22050_Gain6_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain6_B1                            12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain6_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain6_Shift                             1         /* Shift value */
                                                                    /* Gain =  7.000000 dB */
#define HPF_Fs22050_Gain7_A0                            (-164)        /* Floating point value -0.005002 */
#define HPF_Fs22050_Gain7_A1                            11311         /* Floating point value 0.345199 */
#define HPF_Fs22050_Gain7_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain7_B1                            12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain7_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain7_Shift                             2         /* Shift value */
                                                                    /* Gain =  8.000000 dB */
#define HPF_Fs22050_Gain8_A0                            (-864)        /* Floating point value -0.026368 */
#define HPF_Fs22050_Gain8_A1                            12012         /* Floating point value 0.366565 */
#define HPF_Fs22050_Gain8_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain8_B1                            12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain8_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain8_Shift                             2         /* Shift value */
                                                                    /* Gain =  9.000000 dB */
#define HPF_Fs22050_Gain9_A0                           (-1650)        /* Floating point value -0.050340 */
#define HPF_Fs22050_Gain9_A1                            12797         /* Floating point value 0.390537 */
#define HPF_Fs22050_Gain9_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain9_B1                            12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain9_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain9_Shift                             2         /* Shift value */
                                                                    /* Gain =  10.000000 dB */
#define HPF_Fs22050_Gain10_A0                          (-2531)        /* Floating point value -0.077238 */
#define HPF_Fs22050_Gain10_A1                           13679         /* Floating point value 0.417435 */
#define HPF_Fs22050_Gain10_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain10_B1                           12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain10_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain10_Shift                            2         /* Shift value */
                                                                    /* Gain =  11.000000 dB */
#define HPF_Fs22050_Gain11_A0                          (-3520)        /* Floating point value -0.107417 */
#define HPF_Fs22050_Gain11_A1                           14667         /* Floating point value 0.447615 */
#define HPF_Fs22050_Gain11_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain11_B1                           12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain11_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain11_Shift                            2         /* Shift value */
                                                                    /* Gain =  12.000000 dB */
#define HPF_Fs22050_Gain12_A0                          (-4629)        /* Floating point value -0.141279 */
#define HPF_Fs22050_Gain12_A1                           15777         /* Floating point value 0.481477 */
#define HPF_Fs22050_Gain12_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain12_B1                           12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain12_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain12_Shift                            2         /* Shift value */
                                                                    /* Gain =  13.000000 dB */
#define HPF_Fs22050_Gain13_A0                          (-2944)        /* Floating point value -0.089849 */
#define HPF_Fs22050_Gain13_A1                            8531         /* Floating point value 0.260352 */
#define HPF_Fs22050_Gain13_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain13_B1                           12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain13_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain13_Shift                            3         /* Shift value */
                                                                    /* Gain =  14.000000 dB */
#define HPF_Fs22050_Gain14_A0                          (-3644)        /* Floating point value -0.111215 */
#define HPF_Fs22050_Gain14_A1                            9231         /* Floating point value 0.281718 */
#define HPF_Fs22050_Gain14_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain14_B1                           12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain14_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain14_Shift                            3         /* Shift value */
                                                                    /* Gain =  15.000000 dB */
#define HPF_Fs22050_Gain15_A0                          (-4430)        /* Floating point value -0.135187 */
#define HPF_Fs22050_Gain15_A1                           10017         /* Floating point value 0.305690 */
#define HPF_Fs22050_Gain15_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain15_B1                           12125         /* Floating point value 0.370033 */
#define HPF_Fs22050_Gain15_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs22050_Gain15_Shift                            3         /* Shift value */


/* Coefficients for sample rate 24000Hz */
                                                                    /* Gain =  1.000000 dB */
#define HPF_Fs24000_Gain1_A0                             3625         /* Floating point value 0.110628 */
#define HPF_Fs24000_Gain1_A1                            16960         /* Floating point value 0.517578 */
#define HPF_Fs24000_Gain1_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain1_B1                             8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain1_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain1_Shift                             1         /* Shift value */
                                                                    /* Gain =  2.000000 dB */
#define HPF_Fs24000_Gain2_A0                             2811         /* Floating point value 0.085800 */
#define HPF_Fs24000_Gain2_A1                            17774         /* Floating point value 0.542406 */
#define HPF_Fs24000_Gain2_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain2_B1                             8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain2_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain2_Shift                             1         /* Shift value */
                                                                    /* Gain =  3.000000 dB */
#define HPF_Fs24000_Gain3_A0                             1899         /* Floating point value 0.057943 */
#define HPF_Fs24000_Gain3_A1                            18686         /* Floating point value 0.570263 */
#define HPF_Fs24000_Gain3_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain3_B1                             8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain3_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain3_Shift                             1         /* Shift value */
                                                                    /* Gain =  4.000000 dB */
#define HPF_Fs24000_Gain4_A0                              874         /* Floating point value 0.026687 */
#define HPF_Fs24000_Gain4_A1                            19711         /* Floating point value 0.601519 */
#define HPF_Fs24000_Gain4_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain4_B1                             8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain4_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain4_Shift                             1         /* Shift value */
                                                                    /* Gain =  5.000000 dB */
#define HPF_Fs24000_Gain5_A0                            (-275)        /* Floating point value -0.008383 */
#define HPF_Fs24000_Gain5_A1                            20860         /* Floating point value 0.636589 */
#define HPF_Fs24000_Gain5_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain5_B1                             8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain5_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain5_Shift                             1         /* Shift value */
                                                                    /* Gain =  6.000000 dB */
#define HPF_Fs24000_Gain6_A0                           (-1564)        /* Floating point value -0.047733 */
#define HPF_Fs24000_Gain6_A1                            22149         /* Floating point value 0.675938 */
#define HPF_Fs24000_Gain6_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain6_B1                             8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain6_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain6_Shift                             1         /* Shift value */
                                                                    /* Gain =  7.000000 dB */
#define HPF_Fs24000_Gain7_A0                           (-1509)        /* Floating point value -0.046051 */
#define HPF_Fs24000_Gain7_A1                            11826         /* Floating point value 0.360899 */
#define HPF_Fs24000_Gain7_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain7_B1                             8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain7_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain7_Shift                             2         /* Shift value */
                                                                    /* Gain =  8.000000 dB */
#define HPF_Fs24000_Gain8_A0                           (-2323)        /* Floating point value -0.070878 */
#define HPF_Fs24000_Gain8_A1                            12640         /* Floating point value 0.385727 */
#define HPF_Fs24000_Gain8_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain8_B1                             8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain8_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain8_Shift                             2         /* Shift value */
                                                                    /* Gain =  9.000000 dB */
#define HPF_Fs24000_Gain9_A0                           (-3235)        /* Floating point value -0.098736 */
#define HPF_Fs24000_Gain9_A1                            13552         /* Floating point value 0.413584 */
#define HPF_Fs24000_Gain9_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain9_B1                             8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain9_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain9_Shift                             2         /* Shift value */
                                                                    /* Gain =  10.000000 dB */
#define HPF_Fs24000_Gain10_A0                          (-4260)        /* Floating point value -0.129992 */
#define HPF_Fs24000_Gain10_A1                           14577         /* Floating point value 0.444841 */
#define HPF_Fs24000_Gain10_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain10_B1                            8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain10_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain10_Shift                            2         /* Shift value */
                                                                    /* Gain =  11.000000 dB */
#define HPF_Fs24000_Gain11_A0                          (-5409)        /* Floating point value -0.165062 */
#define HPF_Fs24000_Gain11_A1                           15726         /* Floating point value 0.479911 */
#define HPF_Fs24000_Gain11_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain11_B1                            8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain11_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain11_Shift                            2         /* Shift value */
                                                                    /* Gain =  12.000000 dB */
#define HPF_Fs24000_Gain12_A0                          (-6698)        /* Floating point value -0.204411 */
#define HPF_Fs24000_Gain12_A1                           17015         /* Floating point value 0.519260 */
#define HPF_Fs24000_Gain12_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain12_B1                            8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain12_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain12_Shift                            2         /* Shift value */
                                                                    /* Gain =  13.000000 dB */
#define HPF_Fs24000_Gain13_A0                          (-4082)        /* Floating point value -0.124576 */
#define HPF_Fs24000_Gain13_A1                            9253         /* Floating point value 0.282374 */
#define HPF_Fs24000_Gain13_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain13_B1                            8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain13_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain13_Shift                            3         /* Shift value */
                                                                    /* Gain =  14.000000 dB */
#define HPF_Fs24000_Gain14_A0                          (-4896)        /* Floating point value -0.149404 */
#define HPF_Fs24000_Gain14_A1                           10066         /* Floating point value 0.307202 */
#define HPF_Fs24000_Gain14_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain14_B1                            8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain14_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain14_Shift                            3         /* Shift value */
                                                                    /* Gain =  15.000000 dB */
#define HPF_Fs24000_Gain15_A0                          (-5808)        /* Floating point value -0.177261 */
#define HPF_Fs24000_Gain15_A1                           10979         /* Floating point value 0.335059 */
#define HPF_Fs24000_Gain15_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain15_B1                            8780         /* Floating point value 0.267949 */
#define HPF_Fs24000_Gain15_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs24000_Gain15_Shift                            3         /* Shift value */


/* Coefficients for sample rate 32000Hz */
                                                                    /* Gain =  1.000000 dB */
#define HPF_Fs32000_Gain1_A0                            17225         /* Floating point value 0.525677 */
#define HPF_Fs32000_Gain1_A1                            (-990)        /* Floating point value -0.030227 */
#define HPF_Fs32000_Gain1_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain1_B1                                0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain1_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain1_Shift                             1         /* Shift value */
                                                                    /* Gain =  2.000000 dB */
#define HPF_Fs32000_Gain2_A0                            18337         /* Floating point value 0.559593 */
#define HPF_Fs32000_Gain2_A1                           (-2102)        /* Floating point value -0.064142 */
#define HPF_Fs32000_Gain2_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain2_B1                                0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain2_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain2_Shift                             1         /* Shift value */
                                                                    /* Gain =  3.000000 dB */
#define HPF_Fs32000_Gain3_A0                            19584         /* Floating point value 0.597646 */
#define HPF_Fs32000_Gain3_A1                           (-3349)        /* Floating point value -0.102196 */
#define HPF_Fs32000_Gain3_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain3_B1                                0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain3_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain3_Shift                             1         /* Shift value */
                                                                    /* Gain =  4.000000 dB */
#define HPF_Fs32000_Gain4_A0                            20983         /* Floating point value 0.640343 */
#define HPF_Fs32000_Gain4_A1                           (-4748)        /* Floating point value -0.144893 */
#define HPF_Fs32000_Gain4_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain4_B1                                0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain4_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain4_Shift                             1         /* Shift value */
                                                                    /* Gain =  5.000000 dB */
#define HPF_Fs32000_Gain5_A0                            22553         /* Floating point value 0.688250 */
#define HPF_Fs32000_Gain5_A1                           (-6318)        /* Floating point value -0.192799 */
#define HPF_Fs32000_Gain5_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain5_B1                                0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain5_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain5_Shift                             1         /* Shift value */
                                                                    /* Gain =  6.000000 dB */
#define HPF_Fs32000_Gain6_A0                            24314         /* Floating point value 0.742002 */
#define HPF_Fs32000_Gain6_A1                           (-8079)        /* Floating point value -0.246551 */
#define HPF_Fs32000_Gain6_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain6_B1                                0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain6_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain6_Shift                             1         /* Shift value */
                                                                    /* Gain =  7.000000 dB */
#define HPF_Fs32000_Gain7_A0                            13176         /* Floating point value 0.402109 */
#define HPF_Fs32000_Gain7_A1                           (-5040)        /* Floating point value -0.153795 */
#define HPF_Fs32000_Gain7_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain7_B1                                0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain7_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain7_Shift                             2         /* Shift value */
                                                                    /* Gain =  8.000000 dB */
#define HPF_Fs32000_Gain8_A0                            14288         /* Floating point value 0.436024 */
#define HPF_Fs32000_Gain8_A1                           (-6151)        /* Floating point value -0.187711 */
#define HPF_Fs32000_Gain8_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain8_B1                                0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain8_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain8_Shift                             2         /* Shift value */
                                                                    /* Gain =  9.000000 dB */
#define HPF_Fs32000_Gain9_A0                            15535         /* Floating point value 0.474078 */
#define HPF_Fs32000_Gain9_A1                           (-7398)        /* Floating point value -0.225764 */
#define HPF_Fs32000_Gain9_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain9_B1                                0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain9_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain9_Shift                             2         /* Shift value */
                                                                    /* Gain =  10.000000 dB */
#define HPF_Fs32000_Gain10_A0                           16934         /* Floating point value 0.516774 */
#define HPF_Fs32000_Gain10_A1                          (-8797)        /* Floating point value -0.268461 */
#define HPF_Fs32000_Gain10_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain10_B1                               0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain10_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain10_Shift                            2         /* Shift value */
                                                                    /* Gain =  11.000000 dB */
#define HPF_Fs32000_Gain11_A0                           18503         /* Floating point value 0.564681 */
#define HPF_Fs32000_Gain11_A1                         (-10367)        /* Floating point value -0.316368 */
#define HPF_Fs32000_Gain11_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain11_B1                               0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain11_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain11_Shift                            2         /* Shift value */
                                                                    /* Gain =  12.000000 dB */
#define HPF_Fs32000_Gain12_A0                           20265         /* Floating point value 0.618433 */
#define HPF_Fs32000_Gain12_A1                         (-12128)        /* Floating point value -0.370120 */
#define HPF_Fs32000_Gain12_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain12_B1                               0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain12_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain12_Shift                            2         /* Shift value */
                                                                    /* Gain =  13.000000 dB */
#define HPF_Fs32000_Gain13_A0                           11147         /* Floating point value 0.340178 */
#define HPF_Fs32000_Gain13_A1                          (-7069)        /* Floating point value -0.215726 */
#define HPF_Fs32000_Gain13_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain13_B1                               0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain13_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain13_Shift                            3         /* Shift value */
                                                                    /* Gain =  14.000000 dB */
#define HPF_Fs32000_Gain14_A0                           12258         /* Floating point value 0.374093 */
#define HPF_Fs32000_Gain14_A1                          (-8180)        /* Floating point value -0.249642 */
#define HPF_Fs32000_Gain14_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain14_B1                               0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain14_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain14_Shift                            3         /* Shift value */
                                                                    /* Gain =  15.000000 dB */
#define HPF_Fs32000_Gain15_A0                           13505         /* Floating point value 0.412147 */
#define HPF_Fs32000_Gain15_A1                          (-9427)        /* Floating point value -0.287695 */
#define HPF_Fs32000_Gain15_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain15_B1                               0         /* Floating point value -0.000000 */
#define HPF_Fs32000_Gain15_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs32000_Gain15_Shift                            3         /* Shift value */


/* Coefficients for sample rate 44100Hz */
                                                                    /* Gain =  1.000000 dB */
#define HPF_Fs44100_Gain1_A0                            17442         /* Floating point value 0.532294 */
#define HPF_Fs44100_Gain1_A1                           (-4761)        /* Floating point value -0.145294 */
#define HPF_Fs44100_Gain1_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain1_B1                           (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain1_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain1_Shift                             1         /* Shift value */
                                                                    /* Gain =  2.000000 dB */
#define HPF_Fs44100_Gain2_A0                            18797         /* Floating point value 0.573633 */
#define HPF_Fs44100_Gain2_A1                           (-6116)        /* Floating point value -0.186634 */
#define HPF_Fs44100_Gain2_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain2_B1                           (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain2_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain2_Shift                             1         /* Shift value */
                                                                    /* Gain =  3.000000 dB */
#define HPF_Fs44100_Gain3_A0                            20317         /* Floating point value 0.620016 */
#define HPF_Fs44100_Gain3_A1                           (-7635)        /* Floating point value -0.233017 */
#define HPF_Fs44100_Gain3_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain3_B1                           (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain3_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain3_Shift                             1         /* Shift value */
                                                                    /* Gain =  4.000000 dB */
#define HPF_Fs44100_Gain4_A0                            22022         /* Floating point value 0.672059 */
#define HPF_Fs44100_Gain4_A1                           (-9341)        /* Floating point value -0.285060 */
#define HPF_Fs44100_Gain4_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain4_B1                           (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain4_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain4_Shift                             1         /* Shift value */
                                                                    /* Gain =  5.000000 dB */
#define HPF_Fs44100_Gain5_A0                            23935         /* Floating point value 0.730452 */
#define HPF_Fs44100_Gain5_A1                          (-11254)        /* Floating point value -0.343453 */
#define HPF_Fs44100_Gain5_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain5_B1                           (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain5_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain5_Shift                             1         /* Shift value */
                                                                    /* Gain =  6.000000 dB */
#define HPF_Fs44100_Gain6_A0                            26082         /* Floating point value 0.795970 */
#define HPF_Fs44100_Gain6_A1                          (-13401)        /* Floating point value -0.408971 */
#define HPF_Fs44100_Gain6_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain6_B1                           (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain6_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain6_Shift                             1         /* Shift value */
                                                                    /* Gain =  7.000000 dB */
#define HPF_Fs44100_Gain7_A0                            14279         /* Floating point value 0.435774 */
#define HPF_Fs44100_Gain7_A1                           (-7924)        /* Floating point value -0.241815 */
#define HPF_Fs44100_Gain7_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain7_B1                           (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain7_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain7_Shift                             2         /* Shift value */
                                                                    /* Gain =  8.000000 dB */
#define HPF_Fs44100_Gain8_A0                            15634         /* Floating point value 0.477113 */
#define HPF_Fs44100_Gain8_A1                           (-9278)        /* Floating point value -0.283154 */
#define HPF_Fs44100_Gain8_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain8_B1                           (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain8_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain8_Shift                             2         /* Shift value */
                                                                    /* Gain =  9.000000 dB */
#define HPF_Fs44100_Gain9_A0                            17154         /* Floating point value 0.523496 */
#define HPF_Fs44100_Gain9_A1                          (-10798)        /* Floating point value -0.329537 */
#define HPF_Fs44100_Gain9_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain9_B1                           (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain9_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain9_Shift                             2         /* Shift value */
                                                                    /* Gain =  10.000000 dB */
#define HPF_Fs44100_Gain10_A0                           18859         /* Floating point value 0.575539 */
#define HPF_Fs44100_Gain10_A1                         (-12504)        /* Floating point value -0.381580 */
#define HPF_Fs44100_Gain10_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain10_B1                          (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain10_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain10_Shift                            2         /* Shift value */
                                                                    /* Gain =  11.000000 dB */
#define HPF_Fs44100_Gain11_A0                           20773         /* Floating point value 0.633932 */
#define HPF_Fs44100_Gain11_A1                         (-14417)        /* Floating point value -0.439973 */
#define HPF_Fs44100_Gain11_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain11_B1                          (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain11_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain11_Shift                            2         /* Shift value */
                                                                    /* Gain =  12.000000 dB */
#define HPF_Fs44100_Gain12_A0                           22920         /* Floating point value 0.699450 */
#define HPF_Fs44100_Gain12_A1                         (-16564)        /* Floating point value -0.505491 */
#define HPF_Fs44100_Gain12_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain12_B1                          (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain12_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain12_Shift                            2         /* Shift value */
                                                                    /* Gain =  13.000000 dB */
#define HPF_Fs44100_Gain13_A0                           12694         /* Floating point value 0.387399 */
#define HPF_Fs44100_Gain13_A1                          (-9509)        /* Floating point value -0.290189 */
#define HPF_Fs44100_Gain13_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain13_B1                          (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain13_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain13_Shift                            3         /* Shift value */
                                                                    /* Gain =  14.000000 dB */
#define HPF_Fs44100_Gain14_A0                           14049         /* Floating point value 0.428738 */
#define HPF_Fs44100_Gain14_A1                         (-10864)        /* Floating point value -0.331528 */
#define HPF_Fs44100_Gain14_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain14_B1                          (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain14_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain14_Shift                            3         /* Shift value */
                                                                    /* Gain =  15.000000 dB */
#define HPF_Fs44100_Gain15_A0                           15569         /* Floating point value 0.475121 */
#define HPF_Fs44100_Gain15_A1                         (-12383)        /* Floating point value -0.377912 */
#define HPF_Fs44100_Gain15_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain15_B1                          (-7173)        /* Floating point value -0.218894 */
#define HPF_Fs44100_Gain15_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs44100_Gain15_Shift                            3         /* Shift value */


/* Coefficients for sample rate 48000Hz */
                                                                    /* Gain =  1.000000 dB */
#define HPF_Fs48000_Gain1_A0                            17491         /* Floating point value 0.533777 */
#define HPF_Fs48000_Gain1_A1                           (-5606)        /* Floating point value -0.171082 */
#define HPF_Fs48000_Gain1_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain1_B1                           (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain1_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain1_Shift                             1         /* Shift value */
                                                                    /* Gain =  2.000000 dB */
#define HPF_Fs48000_Gain2_A0                            18900         /* Floating point value 0.576779 */
#define HPF_Fs48000_Gain2_A1                           (-7015)        /* Floating point value -0.214085 */
#define HPF_Fs48000_Gain2_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain2_B1                           (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain2_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain2_Shift                             1         /* Shift value */
                                                                    /* Gain =  3.000000 dB */
#define HPF_Fs48000_Gain3_A0                            20481         /* Floating point value 0.625029 */
#define HPF_Fs48000_Gain3_A1                           (-8596)        /* Floating point value -0.262335 */
#define HPF_Fs48000_Gain3_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain3_B1                           (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain3_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain3_Shift                             1         /* Shift value */
                                                                    /* Gain =  4.000000 dB */
#define HPF_Fs48000_Gain4_A0                            22255         /* Floating point value 0.679167 */
#define HPF_Fs48000_Gain4_A1                          (-10370)        /* Floating point value -0.316472 */
#define HPF_Fs48000_Gain4_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain4_B1                           (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain4_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain4_Shift                             1         /* Shift value */
                                                                    /* Gain =  5.000000 dB */
#define HPF_Fs48000_Gain5_A0                            24245         /* Floating point value 0.739910 */
#define HPF_Fs48000_Gain5_A1                          (-12361)        /* Floating point value -0.377215 */
#define HPF_Fs48000_Gain5_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain5_B1                           (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain5_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain5_Shift                             1         /* Shift value */
                                                                    /* Gain =  6.000000 dB */
#define HPF_Fs48000_Gain6_A0                            26479         /* Floating point value 0.808065 */
#define HPF_Fs48000_Gain6_A1                          (-14594)        /* Floating point value -0.445370 */
#define HPF_Fs48000_Gain6_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain6_B1                           (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain6_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain6_Shift                             1         /* Shift value */
                                                                    /* Gain =  7.000000 dB */
#define HPF_Fs48000_Gain7_A0                            14527         /* Floating point value 0.443318 */
#define HPF_Fs48000_Gain7_A1                           (-8570)        /* Floating point value -0.261540 */
#define HPF_Fs48000_Gain7_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain7_B1                           (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain7_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain7_Shift                             2         /* Shift value */
                                                                    /* Gain =  8.000000 dB */
#define HPF_Fs48000_Gain8_A0                            15936         /* Floating point value 0.486321 */
#define HPF_Fs48000_Gain8_A1                           (-9979)        /* Floating point value -0.304543 */
#define HPF_Fs48000_Gain8_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain8_B1                           (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain8_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain8_Shift                             2         /* Shift value */
                                                                    /* Gain =  9.000000 dB */
#define HPF_Fs48000_Gain9_A0                            17517         /* Floating point value 0.534571 */
#define HPF_Fs48000_Gain9_A1                          (-11560)        /* Floating point value -0.352793 */
#define HPF_Fs48000_Gain9_A2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain9_B1                           (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain9_B2                                0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain9_Shift                             2         /* Shift value */
                                                                    /* Gain =  10.000000 dB */
#define HPF_Fs48000_Gain10_A0                           19291         /* Floating point value 0.588708 */
#define HPF_Fs48000_Gain10_A1                         (-13334)        /* Floating point value -0.406930 */
#define HPF_Fs48000_Gain10_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain10_B1                          (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain10_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain10_Shift                            2         /* Shift value */
                                                                    /* Gain =  11.000000 dB */
#define HPF_Fs48000_Gain11_A0                           21281         /* Floating point value 0.649452 */
#define HPF_Fs48000_Gain11_A1                         (-15325)        /* Floating point value -0.467674 */
#define HPF_Fs48000_Gain11_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain11_B1                          (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain11_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain11_Shift                            2         /* Shift value */
                                                                    /* Gain =  12.000000 dB */
#define HPF_Fs48000_Gain12_A0                           23515         /* Floating point value 0.717607 */
#define HPF_Fs48000_Gain12_A1                         (-17558)        /* Floating point value -0.535829 */
#define HPF_Fs48000_Gain12_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain12_B1                          (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain12_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain12_Shift                            2         /* Shift value */
                                                                    /* Gain =  13.000000 dB */
#define HPF_Fs48000_Gain13_A0                           13041         /* Floating point value 0.397982 */
#define HPF_Fs48000_Gain13_A1                         (-10056)        /* Floating point value -0.306877 */
#define HPF_Fs48000_Gain13_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain13_B1                          (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain13_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain13_Shift                            3         /* Shift value */
                                                                    /* Gain =  14.000000 dB */
#define HPF_Fs48000_Gain14_A0                           14450         /* Floating point value 0.440984 */
#define HPF_Fs48000_Gain14_A1                         (-11465)        /* Floating point value -0.349880 */
#define HPF_Fs48000_Gain14_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain14_B1                          (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain14_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain14_Shift                            3         /* Shift value */
                                                                    /* Gain =  15.000000 dB */
#define HPF_Fs48000_Gain15_A0                           16031         /* Floating point value 0.489234 */
#define HPF_Fs48000_Gain15_A1                         (-13046)        /* Floating point value -0.398130 */
#define HPF_Fs48000_Gain15_A2                               0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain15_B1                          (-8780)        /* Floating point value -0.267949 */
#define HPF_Fs48000_Gain15_B2                               0         /* Floating point value 0.000000 */
#define HPF_Fs48000_Gain15_Shift                            3         /* Shift value */


#endif
#endif
