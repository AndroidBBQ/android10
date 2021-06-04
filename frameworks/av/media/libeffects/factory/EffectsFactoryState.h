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

#ifndef ANDROID_EFFECTSFACTORYSTATE_H_
#define ANDROID_EFFECTSFACTORYSTATE_H_

#include "EffectsFactory.h"

#if __cplusplus
extern "C" {
#endif

/** @file Contains the state shared with configuration loader of the Effect factory.
 *        This global state should probably be refactor in a structure
 *        provided by the config loader on EffectsFactory init.
 *        This header also contains some helper functions to work on the state.
 */

extern list_elem_t *gLibraryList; // list of lib_entry_t: all currently loaded libraries
// list of effects skipped because of duplicate uuid or invalid version
extern list_elem_t *gSkippedEffects;
// list of effect_descriptor and list of sub effects : all currently loaded
// It does not contain effects without sub effects.
extern list_sub_elem_t *gSubEffectList;
extern pthread_mutex_t gLibLock; // controls access to gLibraryList

extern list_elem_t *gLibraryFailedList;  //list of lib_failed_entry_t: libraries failed to load



int findEffect(const effect_uuid_t *type,
               const effect_uuid_t *uuid,
               lib_entry_t **lib,
               effect_descriptor_t **desc);

int stringToUuid(const char *str, effect_uuid_t *uuid);
/** Used to log UUIDs */
int uuidToString(const effect_uuid_t *uuid, char *str, size_t maxLen);

/** Used for debuging. */
void dumpEffectDescriptor(effect_descriptor_t *desc, char *str, size_t len, int indent);

#if __cplusplus
} // extern "C"
#endif

#endif // ANDROID_EFFECTSFACTORYSTATE_H_
