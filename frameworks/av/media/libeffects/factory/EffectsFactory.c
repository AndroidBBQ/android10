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

#define LOG_TAG "EffectsFactory"
//#define LOG_NDEBUG 0

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cutils/properties.h>
#include <log/log.h>

#include <media/EffectsFactoryApi.h>

#include "EffectsConfigLoader.h"
#include "EffectsFactoryState.h"
#include "EffectsXmlConfigLoader.h"

#include "EffectsFactory.h"

static list_elem_t *gEffectList; // list of effect_entry_t: all currently created effects
static uint32_t gNumEffects;         // total number number of effects
static list_elem_t *gCurLib;    // current library in enumeration process
static list_elem_t *gCurEffect; // current effect in enumeration process
static uint32_t gCurEffectIdx;       // current effect index in enumeration process
/** Number of elements skipped during the effects configuration loading.
 *  -1 if the config loader failed
 *  -2 if config load was skipped
 */
static ssize_t gConfigNbElemSkipped = -2;

static int gInitDone; // true is global initialization has been preformed
static int gCanQueryEffect; // indicates that call to EffectQueryEffect() is valid, i.e. that the list of effects
                          // was not modified since last call to EffectQueryNumberEffects()
/////////////////////////////////////////////////
//      Local functions prototypes
/////////////////////////////////////////////////

static int init();
static void resetEffectEnumeration();
static uint32_t updateNumEffects();
// To search a subeffect in the gSubEffectList
static int findSubEffect(const effect_uuid_t *uuid,
               lib_entry_t **lib,
               effect_descriptor_t **desc);

/////////////////////////////////////////////////
//      Effect Control Interface functions
/////////////////////////////////////////////////

int Effect_Process(effect_handle_t self, audio_buffer_t *inBuffer, audio_buffer_t *outBuffer)
{
    int ret = init();
    if (ret < 0) {
        return ret;
    }
    effect_entry_t *fx = (effect_entry_t *)self;
    pthread_mutex_lock(&gLibLock);
    if (fx->lib == NULL) {
        pthread_mutex_unlock(&gLibLock);
        return -EPIPE;
    }
    pthread_mutex_lock(&fx->lib->lock);
    pthread_mutex_unlock(&gLibLock);

    ret = (*fx->subItfe)->process(fx->subItfe, inBuffer, outBuffer);
    pthread_mutex_unlock(&fx->lib->lock);
    return ret;
}

int Effect_Command(effect_handle_t self,
                   uint32_t cmdCode,
                   uint32_t cmdSize,
                   void *pCmdData,
                   uint32_t *replySize,
                   void *pReplyData)
{
    int ret = init();
    if (ret < 0) {
        return ret;
    }
    effect_entry_t *fx = (effect_entry_t *)self;
    pthread_mutex_lock(&gLibLock);
    if (fx->lib == NULL) {
        pthread_mutex_unlock(&gLibLock);
        return -EPIPE;
    }
    pthread_mutex_lock(&fx->lib->lock);
    pthread_mutex_unlock(&gLibLock);

    ret = (*fx->subItfe)->command(fx->subItfe, cmdCode, cmdSize, pCmdData, replySize, pReplyData);
    pthread_mutex_unlock(&fx->lib->lock);
    return ret;
}

int Effect_GetDescriptor(effect_handle_t self,
                         effect_descriptor_t *desc)
{
    int ret = init();
    if (ret < 0) {
        return ret;
    }
    effect_entry_t *fx = (effect_entry_t *)self;
    pthread_mutex_lock(&gLibLock);
    if (fx->lib == NULL) {
        pthread_mutex_unlock(&gLibLock);
        return -EPIPE;
    }
    pthread_mutex_lock(&fx->lib->lock);
    pthread_mutex_unlock(&gLibLock);

    ret = (*fx->subItfe)->get_descriptor(fx->subItfe, desc);
    pthread_mutex_unlock(&fx->lib->lock);
    return ret;
}

int Effect_ProcessReverse(effect_handle_t self, audio_buffer_t *inBuffer, audio_buffer_t *outBuffer)
{
    int ret = init();
    if (ret < 0) {
        return ret;
    }
    effect_entry_t *fx = (effect_entry_t *)self;
    pthread_mutex_lock(&gLibLock);
    if (fx->lib == NULL) {
        pthread_mutex_unlock(&gLibLock);
        return -EPIPE;
    }
    pthread_mutex_lock(&fx->lib->lock);
    pthread_mutex_unlock(&gLibLock);

    if ((*fx->subItfe)->process_reverse != NULL) {
        ret = (*fx->subItfe)->process_reverse(fx->subItfe, inBuffer, outBuffer);
    } else {
        ret = -ENOSYS;
    }
    pthread_mutex_unlock(&fx->lib->lock);
    return ret;
}


const struct effect_interface_s gInterface = {
        Effect_Process,
        Effect_Command,
        Effect_GetDescriptor,
        NULL
};

const struct effect_interface_s gInterfaceWithReverse = {
        Effect_Process,
        Effect_Command,
        Effect_GetDescriptor,
        Effect_ProcessReverse
};

/////////////////////////////////////////////////
//      Effect Factory Interface functions
/////////////////////////////////////////////////

int EffectQueryNumberEffects(uint32_t *pNumEffects)
{
    int ret = init();
    if (ret < 0) {
        return ret;
    }
    if (pNumEffects == NULL) {
        return -EINVAL;
    }

    pthread_mutex_lock(&gLibLock);
    *pNumEffects = gNumEffects;
    gCanQueryEffect = 1;
    pthread_mutex_unlock(&gLibLock);
    ALOGV("EffectQueryNumberEffects(): %d", *pNumEffects);
    return ret;
}

int EffectQueryEffect(uint32_t index, effect_descriptor_t *pDescriptor)
{
    int ret = init();
    if (ret < 0) {
        return ret;
    }
    if (pDescriptor == NULL ||
        index >= gNumEffects) {
        return -EINVAL;
    }
    if (gCanQueryEffect == 0) {
        return -ENOSYS;
    }

    pthread_mutex_lock(&gLibLock);
    ret = -ENOENT;
    if (index < gCurEffectIdx) {
        resetEffectEnumeration();
    }
    while (gCurLib) {
        if (gCurEffect) {
            if (index == gCurEffectIdx) {
                *pDescriptor = *(effect_descriptor_t *)gCurEffect->object;
                ret = 0;
                break;
            } else {
                gCurEffect = gCurEffect->next;
                gCurEffectIdx++;
            }
        } else {
            gCurLib = gCurLib->next;
            gCurEffect = ((lib_entry_t *)gCurLib->object)->effects;
        }
    }

#if (LOG_NDEBUG == 0)
    char str[512];
    dumpEffectDescriptor(pDescriptor, str, sizeof(str), 0 /* indent */);
    ALOGV("EffectQueryEffect() desc:%s", str);
#endif
    pthread_mutex_unlock(&gLibLock);
    return ret;
}

int EffectGetDescriptor(const effect_uuid_t *uuid, effect_descriptor_t *pDescriptor)
{
    lib_entry_t *l = NULL;
    effect_descriptor_t *d = NULL;

    int ret = init();
    if (ret < 0) {
        return ret;
    }
    if (pDescriptor == NULL || uuid == NULL) {
        return -EINVAL;
    }
    pthread_mutex_lock(&gLibLock);
    ret = findEffect(NULL, uuid, &l, &d);
    if (ret == 0) {
        *pDescriptor = *d;
    }
    pthread_mutex_unlock(&gLibLock);
    return ret;
}

int EffectCreate(const effect_uuid_t *uuid, int32_t sessionId, int32_t ioId, effect_handle_t *pHandle)
{
    list_elem_t *e = gLibraryList;
    lib_entry_t *l = NULL;
    effect_descriptor_t *d = NULL;
    effect_handle_t itfe;
    effect_entry_t *fx;
    int ret;

    if (uuid == NULL || pHandle == NULL) {
        return -EINVAL;
    }

    ALOGV("EffectCreate() UUID: %08X-%04X-%04X-%04X-%02X%02X%02X%02X%02X%02X\n",
            uuid->timeLow, uuid->timeMid, uuid->timeHiAndVersion,
            uuid->clockSeq, uuid->node[0], uuid->node[1],uuid->node[2],
            uuid->node[3],uuid->node[4],uuid->node[5]);

    ret = init();

    if (ret < 0) {
        ALOGW("EffectCreate() init error: %d", ret);
        return ret;
    }

    pthread_mutex_lock(&gLibLock);

    ret = findEffect(NULL, uuid, &l, &d);
    if (ret < 0){
        // Sub effects are not associated with the library->effects,
        // so, findEffect will fail. Search for the effect in gSubEffectList.
        ret = findSubEffect(uuid, &l, &d);
        if (ret < 0 ) {
            goto exit;
        }
    }

    // create effect in library
    ret = l->desc->create_effect(uuid, sessionId, ioId, &itfe);
    if (ret != 0) {
        ALOGW("EffectCreate() library %s: could not create fx %s, error %d", l->name, d->name, ret);
        goto exit;
    }

    // add entry to effect list
    fx = (effect_entry_t *)malloc(sizeof(effect_entry_t));
    fx->subItfe = itfe;
    if ((*itfe)->process_reverse != NULL) {
        fx->itfe = (struct effect_interface_s *)&gInterfaceWithReverse;
        ALOGV("EffectCreate() gInterfaceWithReverse");
    }   else {
        fx->itfe = (struct effect_interface_s *)&gInterface;
        ALOGV("EffectCreate() gInterface");
    }
    fx->lib = l;

    e = (list_elem_t *)malloc(sizeof(list_elem_t));
    e->object = fx;
    e->next = gEffectList;
    gEffectList = e;

    *pHandle = (effect_handle_t)fx;

    ALOGV("EffectCreate() created entry %p with sub itfe %p in library %s", *pHandle, itfe, l->name);

exit:
    pthread_mutex_unlock(&gLibLock);
    return ret;
}

int EffectRelease(effect_handle_t handle)
{
    effect_entry_t *fx;
    list_elem_t *e1;
    list_elem_t *e2;

    int ret = init();
    if (ret < 0) {
        return ret;
    }

    // remove effect from effect list
    pthread_mutex_lock(&gLibLock);
    e1 = gEffectList;
    e2 = NULL;
    while (e1) {
        if (e1->object == handle) {
            if (e2) {
                e2->next = e1->next;
            } else {
                gEffectList = e1->next;
            }
            fx = (effect_entry_t *)e1->object;
            free(e1);
            break;
        }
        e2 = e1;
        e1 = e1->next;
    }
    if (e1 == NULL) {
        ret = -ENOENT;
        goto exit;
    }

    // release effect in library
    if (fx->lib == NULL) {
        ALOGW("EffectRelease() fx %p library already unloaded", handle);
    } else {
        pthread_mutex_lock(&fx->lib->lock);
        fx->lib->desc->release_effect(fx->subItfe);
        pthread_mutex_unlock(&fx->lib->lock);
    }
    free(fx);

exit:
    pthread_mutex_unlock(&gLibLock);
    return ret;
}

int EffectIsNullUuid(const effect_uuid_t *uuid)
{
    if (memcmp(uuid, EFFECT_UUID_NULL, sizeof(effect_uuid_t))) {
        return 0;
    }
    return 1;
}

// Function to get the sub effect descriptors of the effect whose uuid
// is pointed by the first argument. It searches the gSubEffectList for the
// matching uuid and then copies the corresponding sub effect descriptors
// to the inout param
int EffectGetSubEffects(const effect_uuid_t *uuid, sub_effect_entry_t **pSube,
                        size_t size)
{
   ALOGV("EffectGetSubEffects() UUID: %08X-%04X-%04X-%04X-%02X%02X%02X%02X%02X"
          "%02X\n",uuid->timeLow, uuid->timeMid, uuid->timeHiAndVersion,
          uuid->clockSeq, uuid->node[0], uuid->node[1],uuid->node[2],
          uuid->node[3],uuid->node[4],uuid->node[5]);

   // Check if the size of the desc buffer is large enough for 2 subeffects
   if ((uuid == NULL) || (pSube == NULL) || (size < 2)) {
       ALOGW("NULL pointer or insufficient memory. Cannot query subeffects");
       return -EINVAL;
   }
   int ret = init();
   if (ret < 0)
      return ret;
   list_sub_elem_t *e = gSubEffectList;
   sub_effect_entry_t *subeffect;
   effect_descriptor_t *d;
   int count = 0;
   while (e != NULL) {
       d = (effect_descriptor_t*)e->object;
       if (memcmp(uuid, &d->uuid, sizeof(effect_uuid_t)) == 0) {
           ALOGV("EffectGetSubEffects: effect found in the list");
           list_elem_t *subefx = e->sub_elem;
           while (subefx != NULL) {
               subeffect = (sub_effect_entry_t*)subefx->object;
               pSube[count++] = subeffect;
               subefx = subefx->next;
           }
           ALOGV("EffectGetSubEffects end - copied the sub effect structures");
           return count;
       }
       e = e->next;
   }
   return -ENOENT;
}
/////////////////////////////////////////////////
//      Local functions
/////////////////////////////////////////////////

int init() {
    if (gInitDone) {
        return 0;
    }

    // ignore effects or not?
    const bool ignoreFxConfFiles = property_get_bool(PROPERTY_IGNORE_EFFECTS, false);

    pthread_mutex_init(&gLibLock, NULL);

    if (ignoreFxConfFiles) {
        ALOGI("Audio effects in configuration files will be ignored");
    } else {
        gConfigNbElemSkipped = EffectLoadXmlEffectConfig(NULL);
        if (gConfigNbElemSkipped < 0) {
            ALOGW("Failed to load XML effect configuration, fallback to .conf");
            EffectLoadEffectConfig();
        } else if (gConfigNbElemSkipped > 0) {
            ALOGE("Effect config is partially invalid, skipped %zd elements", gConfigNbElemSkipped);
        }
    }

    updateNumEffects();
    gInitDone = 1;
    ALOGV("init() done");
    return 0;
}

// Searches the sub effect matching to the specified uuid
// in the gSubEffectList. It gets the lib_entry_t for
// the matched sub_effect . Used in EffectCreate of sub effects
int findSubEffect(const effect_uuid_t *uuid,
               lib_entry_t **lib,
               effect_descriptor_t **desc)
{
    list_sub_elem_t *e = gSubEffectList;
    list_elem_t *subefx;
    sub_effect_entry_t *effect;
    lib_entry_t *l = NULL;
    effect_descriptor_t *d = NULL;
    int found = 0;
    int ret = 0;

    if (uuid == NULL)
        return -EINVAL;

    while (e != NULL && !found) {
        subefx = (list_elem_t*)(e->sub_elem);
        while (subefx != NULL) {
            effect = (sub_effect_entry_t*)subefx->object;
            l = (lib_entry_t *)effect->lib;
            d = (effect_descriptor_t *)effect->object;
            if (memcmp(&d->uuid, uuid, sizeof(effect_uuid_t)) == 0) {
                ALOGV("uuid matched");
                found = 1;
                break;
            }
            subefx = subefx->next;
        }
        e = e->next;
    }
    if (!found) {
        ALOGV("findSubEffect() effect not found");
        ret = -ENOENT;
    } else {
        ALOGV("findSubEffect() found effect: %s in lib %s", d->name, l->name);
        *lib = l;
        if (desc != NULL) {
            *desc = d;
        }
    }
    return ret;
}

void resetEffectEnumeration()
{
    gCurLib = gLibraryList;
    gCurEffect = NULL;
    if (gCurLib) {
        gCurEffect = ((lib_entry_t *)gCurLib->object)->effects;
    }
    gCurEffectIdx = 0;
}

uint32_t updateNumEffects() {
    list_elem_t *e;
    uint32_t cnt = 0;

    resetEffectEnumeration();

    e = gLibraryList;
    while (e) {
        lib_entry_t *l = (lib_entry_t *)e->object;
        list_elem_t *efx = l->effects;
        while (efx) {
            cnt++;
            efx = efx->next;
        }
        e = e->next;
    }
    gNumEffects = cnt;
    gCanQueryEffect = 0;
    return cnt;
}

int EffectDumpEffects(int fd) {
    char s[512];

    list_elem_t *fe = gLibraryFailedList;
    lib_failed_entry_t *fl = NULL;

    dprintf(fd, "Libraries NOT loaded:\n");

    while (fe) {
        fl = (lib_failed_entry_t *)fe->object;
        dprintf(fd, " Library %s\n", fl->name);
        dprintf(fd, "  path: %s\n", fl->path);
        fe = fe->next;
    }

    list_elem_t *e = gLibraryList;
    lib_entry_t *l = NULL;
    effect_descriptor_t *d = NULL;
    int ret = 0;

    dprintf(fd, "Libraries loaded:\n");
    while (e) {
        l = (lib_entry_t *)e->object;
        list_elem_t *efx = l->effects;
        dprintf(fd, " Library %s\n", l->name);
        dprintf(fd, "  path: %s\n", l->path);
        if (!efx) {
            dprintf(fd, "  (no effects)\n");
        }
        while (efx) {
            d = (effect_descriptor_t *)efx->object;
            dumpEffectDescriptor(d, s, sizeof(s), 2);
            dprintf(fd, "%s", s);
            efx = efx->next;
        }
        e = e->next;
    }

    e = gSkippedEffects;
    if (e) {
        dprintf(fd, "Skipped effects\n");
        while(e) {
            d = (effect_descriptor_t *)e->object;
            dumpEffectDescriptor(d, s, sizeof(s), 2 /* indent */);
            dprintf(fd, "%s", s);
            e = e->next;
        }
    }
    switch (gConfigNbElemSkipped) {
    case -2:
        dprintf(fd, "Effect configuration loading skipped.\n");
        break;
    case -1:
        dprintf(fd, "XML effect configuration failed to load.\n");
        break;
    case 0:
        dprintf(fd, "XML effect configuration loaded successfully.\n");
        break;
    default:
        dprintf(fd, "XML effect configuration partially loaded, skipped %zd elements.\n",
                gConfigNbElemSkipped);
    }
    return ret;
}

