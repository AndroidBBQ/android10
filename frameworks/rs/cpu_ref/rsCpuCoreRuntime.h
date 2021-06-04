/*
 * Copyright (C) 2015 The Android Open Source Project
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

// Warning: This header file is shared with the core runtime, which is written in C;
// hence, this header file must be written in C.

#ifndef RSD_CPU_CORE_RUNTIME_H
#define RSD_CPU_CORE_RUNTIME_H

// Warning: This value is shared with the compiler
// Any change to this value requires a matching compiler change
#define RS_KERNEL_INPUT_LIMIT 8

struct RsLaunchDimensions {
    // Warning: This structure is shared with the compiler
    // Any change to the fields here requires a matching compiler change

    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t lod;
    uint32_t face;
    uint32_t array[4 /*make a define*/];
};

struct RsExpandKernelDriverInfo {
    // Warning: This structure is shared with the compiler
    // Any change to the fields here requires a matching compiler change

    const uint8_t *inPtr[RS_KERNEL_INPUT_LIMIT];
    uint32_t inStride[RS_KERNEL_INPUT_LIMIT];
    uint32_t inLen;

    uint8_t *outPtr[RS_KERNEL_INPUT_LIMIT];
    uint32_t outStride[RS_KERNEL_INPUT_LIMIT];
    uint32_t outLen;

    // Dimension of the launch
    struct RsLaunchDimensions dim;

    // The walking iterator of the launch
    struct RsLaunchDimensions current;

    const void *usr;
    uint32_t usrLen;

    // Items below this line are not used by the compiler and can be change in the driver
    uint32_t lid;
    uint32_t slot;
};

#endif
