/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef EBMLUTIL_H_
#define EBMLUTIL_H_

#include <stdint.h>

namespace webm {

// Encode the id and/or size of an EBML element bytes by setting a leading length descriptor bit:
//
//   1xxxxxxx                                                                - 1-byte values
//   01xxxxxx xxxxxxxx                                                       -
//   001xxxxx xxxxxxxx xxxxxxxx                                              -
//   0001xxxx xxxxxxxx xxxxxxxx xxxxxxxx                                     - ...
//   00001xxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx                            -
//   000001xx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx                   -
//   0000001x xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx          -
//   00000001 xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx - 8-byte values
//
// This function uses the least the number of bytes possible.
uint64_t encodeUnsigned(uint64_t u);

// Like above but pads the input value with leading zeros up to the specified width. The length
// descriptor is calculated based on width.
uint64_t encodeUnsigned(uint64_t u, int width);

// Serialize an EBML coded id or size in big-endian order.
int serializeCodedUnsigned(uint64_t u, uint8_t* bary);

// Calculate the length of an EBML coded id or size from its length descriptor.
int sizeOf(uint64_t u);

}

#endif /* EBMLUTIL_H_ */
