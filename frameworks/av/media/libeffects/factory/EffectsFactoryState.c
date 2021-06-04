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

#define LOG_TAG "EffectsFactoryState"

#include "EffectsFactoryState.h"

#include "log/log.h"

list_elem_t *gLibraryList;
list_elem_t *gSkippedEffects;
list_sub_elem_t *gSubEffectList;
pthread_mutex_t gLibLock = PTHREAD_MUTEX_INITIALIZER;

list_elem_t *gLibraryFailedList;  //list of lib_failed_entry_t: libraries failed to load


int findEffect(const effect_uuid_t *type,
               const effect_uuid_t *uuid,
               lib_entry_t **lib,
               effect_descriptor_t **desc)
{
    list_elem_t *e = gLibraryList;
    lib_entry_t *l = NULL;
    effect_descriptor_t *d = NULL;
    int found = 0;
    int ret = 0;

    while (e && !found) {
        l = (lib_entry_t *)e->object;
        list_elem_t *efx = l->effects;
        while (efx) {
            d = (effect_descriptor_t *)efx->object;
            if (type != NULL && memcmp(&d->type, type, sizeof(effect_uuid_t)) == 0) {
                found = 1;
                break;
            }
            if (uuid != NULL && memcmp(&d->uuid, uuid, sizeof(effect_uuid_t)) == 0) {
                found = 1;
                break;
            }
            efx = efx->next;
        }
        e = e->next;
    }
    if (!found) {
        ALOGV("findEffect() effect not found");
        ret = -ENOENT;
    } else {
        ALOGV("findEffect() found effect: %s in lib %s", d->name, l->name);
        *lib = l;
        if (desc) {
            *desc = d;
        }
    }

    return ret;
}

int stringToUuid(const char *str, effect_uuid_t *uuid)
{
    int tmp[10];

    if (sscanf(str, "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
            tmp, tmp+1, tmp+2, tmp+3, tmp+4, tmp+5, tmp+6, tmp+7, tmp+8, tmp+9) < 10) {
        return -EINVAL;
    }
    uuid->timeLow = (uint32_t)tmp[0];
    uuid->timeMid = (uint16_t)tmp[1];
    uuid->timeHiAndVersion = (uint16_t)tmp[2];
    uuid->clockSeq = (uint16_t)tmp[3];
    uuid->node[0] = (uint8_t)tmp[4];
    uuid->node[1] = (uint8_t)tmp[5];
    uuid->node[2] = (uint8_t)tmp[6];
    uuid->node[3] = (uint8_t)tmp[7];
    uuid->node[4] = (uint8_t)tmp[8];
    uuid->node[5] = (uint8_t)tmp[9];

    return 0;
}

int uuidToString(const effect_uuid_t *uuid, char *str, size_t maxLen)
{

    snprintf(str, maxLen, "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
            uuid->timeLow,
            uuid->timeMid,
            uuid->timeHiAndVersion,
            uuid->clockSeq,
            uuid->node[0],
            uuid->node[1],
            uuid->node[2],
            uuid->node[3],
            uuid->node[4],
            uuid->node[5]);

    return 0;
}


void dumpEffectDescriptor(effect_descriptor_t *desc, char *str, size_t len, int indent) {
    char s[256];
    char ss[256];
    char idt[indent + 1];

    memset(idt, ' ', indent);
    idt[indent] = 0;

    str[0] = 0;

    snprintf(s, sizeof(s), "%s%s / %s\n", idt, desc->name, desc->implementor);
    strlcat(str, s, len);

    uuidToString(&desc->uuid, s, sizeof(s));
    snprintf(ss, sizeof(ss), "%s  UUID: %s\n", idt, s);
    strlcat(str, ss, len);

    uuidToString(&desc->type, s, sizeof(s));
    snprintf(ss, sizeof(ss), "%s  TYPE: %s\n", idt, s);
    strlcat(str, ss, len);

    sprintf(s, "%s  apiVersion: %08X\n%s  flags: %08X\n", idt,
            desc->apiVersion, idt, desc->flags);
    strlcat(str, s, len);
}
