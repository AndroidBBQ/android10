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

#ifndef ANDROID_EFFECTSXMLCONFIGLOADER_H
#define ANDROID_EFFECTSXMLCONFIGLOADER_H

#include <unistd.h>

#include <cutils/compiler.h>

#include "EffectsFactoryState.h"

#if __cplusplus
extern "C" {
#endif

/** Parses the platform effect xml configuration and stores its content in EffectFactoryState.
 * @param[in] path of the configuration file or NULL to load the default one
 * @return -1 on unrecoverable error (eg: no configuration file)
 *         0 on success
 *         the number of invalid elements (lib & effect) skipped if the config is partially invalid
 * @note this function is exported for test purpose only. Do not call from outside this library.
 */
ANDROID_API
ssize_t EffectLoadXmlEffectConfig(const char* path);

#if __cplusplus
} // extern "C"
#endif

#endif  // ANDROID_EFFECTSXMLCONFIGLOADER_H
