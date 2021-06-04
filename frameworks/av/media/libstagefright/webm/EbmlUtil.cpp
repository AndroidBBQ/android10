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

#include <stdint.h>

namespace {

// Table for Seal's algorithm for Number of Trailing Zeros. Hacker's Delight
// online, Figure 5-18 (http://www.hackersdelight.org/revisions.pdf)
// The entries whose value is -1 are never referenced.
int NTZ_TABLE[] = {
    32,  0,  1, 12,  2,  6, -1, 13,   3, -1,  7, -1, -1, -1, -1, 14,
    10,  4, -1, -1,  8, -1, -1, 25,  -1, -1, -1, -1, -1, 21, 27, 15,
    31, 11,  5, -1, -1, -1, -1, -1,   9, -1, -1, 24, -1, -1, 20, 26,
    30, -1, -1, -1, -1, 23, -1, 19,  29, -1, 22, 18, 28, 17, 16, -1
};

int numberOfTrailingZeros32(int32_t i) {
    int64_t i64 = i;
    i64 = (i64 & -i64) * 0x0450FBAF;
    uint32_t u = i64;
    return NTZ_TABLE[(u) >> 26];
}

uint64_t highestOneBit(uint64_t n) {
    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 4);
    n |= (n >> 8);
    n |= (n >> 16);
    n |= (n >> 32);
    return n - (n >> 1);
}

uint64_t _powerOf2(uint64_t u) {
    uint64_t powerOf2 = highestOneBit(u);
    return powerOf2 ? powerOf2 : 1;
}

// Based on Long.numberOfTrailingZeros in Long.java
int numberOfTrailingZeros(uint64_t u) {
    int32_t low = u;
    return low !=0 ? numberOfTrailingZeros32(low)
                   : 32 + numberOfTrailingZeros32((int32_t) (u >> 32));
}
}

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
uint64_t encodeUnsigned(uint64_t u) {
    uint64_t powerOf2 = _powerOf2(u);
    if (u + 1 == powerOf2 << 1)
        powerOf2 <<= 1;
    int shiftWidth = (7 + numberOfTrailingZeros(powerOf2)) / 7 * 7;
    long lengthDescriptor = 1 << shiftWidth;
    return lengthDescriptor | u;
}

// Like above but pads the input value with leading zeros up to the specified width. The length
// descriptor is calculated based on width.
uint64_t encodeUnsigned(uint64_t u, int width) {
    int shiftWidth = 7 * width;
    uint64_t lengthDescriptor = 1;
    lengthDescriptor <<= shiftWidth;
    return lengthDescriptor | u;
}

// Calculate the length of an EBML coded id or size from its length descriptor.
int sizeOf(uint64_t u) {
    uint64_t powerOf2 = _powerOf2(u);
    int unsignedLength = numberOfTrailingZeros(powerOf2) / 8 + 1;
    return unsignedLength;
}

// Serialize an EBML coded id or size in big-endian order.
int serializeCodedUnsigned(uint64_t u, uint8_t* bary) {
    int unsignedLength = sizeOf(u);
    for (int i = unsignedLength - 1; i >= 0; i--) {
        bary[i] = u & 0xff;
        u >>= 8;
    }
    return unsignedLength;
}

}
