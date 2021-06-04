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

/**
 * This is test support for AAudio.
 */
#ifndef AAUDIO_AAUDIO_TESTING_H
#define AAUDIO_AAUDIO_TESTING_H

#include <aaudio/AAudio.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
 * The definitions below are only for testing. Do not use them in an application.
 * They may change or be removed at any time.
 ************************************************************************************/

enum {
    /**
     * Related feature is disabled and never used.
     */
    AAUDIO_POLICY_NEVER = 1,

    /**
     * If related feature works then use it. Otherwise fall back to something else.
     */
     AAUDIO_POLICY_AUTO,

    /**
     * Related feature must be used. If not available then fail.
     */
    AAUDIO_POLICY_ALWAYS
};
typedef int32_t aaudio_policy_t;

/**
 * Control whether AAudioStreamBuilder_openStream() will use the new MMAP data path
 * or the older "Legacy" data path.
 *
 * This will only affect the current process.
 *
 * If unspecified then the policy will be based on system properties or configuration.
 *
 * @note This is only for testing. Do not use this in an application.
 * It may change or be removed at any time.
 *
 * @param policy AAUDIO_UNSPECIFIED, AAUDIO_POLICY_NEVER, AAUDIO_POLICY_AUTO, or AAUDIO_POLICY_ALWAYS
 * @return AAUDIO_OK or a negative error
 */
AAUDIO_API aaudio_result_t AAudio_setMMapPolicy(aaudio_policy_t policy);

/**
 * Get the current MMAP policy set by AAudio_setMMapPolicy().
 *
 * @note This is only for testing. Do not use this in an application.
 * It may change or be removed at any time.
 *
 * @return current policy
 */
AAUDIO_API aaudio_policy_t AAudio_getMMapPolicy();

/**
 * Return true if the stream uses the MMAP data path versus the legacy path.
 *
 * @note This is only for testing. Do not use this in an application.
 * It may change or be removed at any time.
 *
 * @return true if the stream uses ther MMAP data path
 */
AAUDIO_API bool AAudioStream_isMMapUsed(AAudioStream* stream);

#ifdef __cplusplus
}
#endif

#endif //AAUDIO_AAUDIO_TESTING_H

/** @} */
