/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef ANDROID_ML_NN_RUNTIME_NEURAL_NETWORKS_OEM_H
#define ANDROID_ML_NN_RUNTIME_NEURAL_NETWORKS_OEM_H

/******************************************************************
 *
 * IMPORTANT NOTICE:
 *
 *   OEM operation and operand types are DEPRECATED. Use Extensions instead.
 *
 *   This file is not intended for use by general developers -- only
 *   by OEM applications.  The semantics of each enum are OEM-specific
 *   and may change at any time.
 *
 *   OEM source AND binary code relies on the definitions
 *   here to be FROZEN ON ALL UPCOMING PLATFORM RELEASES.
 *
 *   - DO NOT MODIFY ENUMS (EXCEPT IF YOU ADD NEW 32-BIT VALUES)
 *   - DO NOT MODIFY CONSTANTS OR FUNCTIONAL MACROS
 *   - DO NOT CHANGE THE SIGNATURE OF FUNCTIONS IN ANY WAY
 *   - DO NOT CHANGE THE LAYOUT OR SIZE OF STRUCTURES
 */

#if __ANDROID_API__ >= __ANDROID_API_O_MR1__

enum {
    /**
     * DEPRECATED. Use Extensions instead.
     *
     * OEM specific scalar value.
     */
    ANEURALNETWORKS_OEM_SCALAR = 10000,

    /**
     * DEPRECATED. Use Extensions instead.
     *
     * A tensor of OEM specific values.
     */
    ANEURALNETWORKS_TENSOR_OEM_BYTE = 10001,
};  // extends OperandCode

/**
 * If a model contains an {@link ANEURALNETWORKS_OEM_OPERATION}, then
 * either the model must contain only a single operation, or every
 * tensor operand type in the model must be fully specified.
 */
enum {
    /**
     * DEPRECATED. Use Extensions instead.
     *
     * OEM specific operation.
     */
    ANEURALNETWORKS_OEM_OPERATION = 10000,
};  // extends OperationCode

#endif  //  __ANDROID_API__ >= 27

#endif  // ANDROID_ML_NN_RUNTIME_NEURAL_NETWORKS_OEM_H
