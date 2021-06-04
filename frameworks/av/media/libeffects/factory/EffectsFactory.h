/*
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

#ifndef ANDROID_EFFECTSFACTORY_H_
#define ANDROID_EFFECTSFACTORY_H_

#include <dirent.h>
#include <pthread.h>

#include <cutils/compiler.h>
#include <hardware/audio_effect.h>

#if __cplusplus
extern "C" {
#endif

#define PROPERTY_IGNORE_EFFECTS "ro.audio.ignore_effects"

typedef struct list_elem_s {
    void *object;
    struct list_elem_s *next;
} list_elem_t;

// Structure used for storing effects with their sub effects.
// Used in creating gSubEffectList. Here,
// object holds the effect desc and the list sub_elem holds the sub effects
typedef struct list_sub_elem_s {
    void *object;
    list_elem_t *sub_elem;
    struct list_sub_elem_s *next;
} list_sub_elem_t;

typedef struct lib_entry_s {
    audio_effect_library_t *desc;
    char *name;
    char *path;
    void *handle;
    list_elem_t *effects; //list of effect_descriptor_t
    pthread_mutex_t lock;
} lib_entry_t;

typedef struct effect_entry_s {
    struct effect_interface_s *itfe;
    effect_handle_t subItfe;
    lib_entry_t *lib;
} effect_entry_t;

typedef struct lib_failed_entry_s {
    char *name;
    char *path;
} lib_failed_entry_t;

// Structure used to store the lib entry
// and the descriptor of the sub effects.
// The library entry is to be stored in case of
// sub effects as the sub effects are not linked
// to the library list - gLibraryList.
typedef struct sub_effect_entry_s {
    lib_entry_t *lib;
    void *object;
} sub_effect_entry_t;



////////////////////////////////////////////////////////////////////////////////
//
//    Function:       EffectGetSubEffects
//
//    Description:    Returns the descriptors of the sub effects of the effect
//                    whose uuid is pointed to by first argument.
//
//    Input:
//          pEffectUuid:    pointer to the effect uuid.
//          size:           max number of sub_effect_entry_t * in pSube.
//
//    Input/Output:
//          pSube:          address where to return the sub effect structures.
//    Output:
//        returned value:    0          successful operation.
//                          -ENODEV     factory failed to initialize
//                          -EINVAL     invalid pEffectUuid or pDescriptor
//                          -ENOENT     no effect with this uuid found
//        *pDescriptor:     updated with the sub effect descriptors.
//
////////////////////////////////////////////////////////////////////////////////
ANDROID_API
int EffectGetSubEffects(const effect_uuid_t *pEffectUuid,
                        sub_effect_entry_t **pSube,
                        size_t size);

#if __cplusplus
}  // extern "C"
#endif


#endif /*ANDROID_EFFECTSFACTORY_H_*/
