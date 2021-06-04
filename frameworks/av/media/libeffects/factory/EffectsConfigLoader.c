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

#define LOG_TAG "EffectsFactoryConfigLoader"
//#define LOG_NDEBUG 0

#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>

#include <cutils/config_utils.h>
#include <cutils/misc.h>
#include <log/log.h>

#include <system/audio_effects/audio_effects_conf.h>

#include "EffectsConfigLoader.h"
#include "EffectsFactoryState.h"

/////////////////////////////////////////////////
//      Local functions prototypes
/////////////////////////////////////////////////

static int loadEffectConfigFile(const char *path);
static int loadLibraries(cnode *root);
static int loadLibrary(cnode *root, const char *name);
static int loadEffects(cnode *root);
static int loadEffect(cnode *node);
// To get and add the effect pointed by the passed node to the gSubEffectList
static int addSubEffect(cnode *root);
static lib_entry_t *getLibrary(const char *path);

static lib_entry_t *gCachedLibrary;  // last library accessed by getLibrary()

int EffectLoadEffectConfig()
{
    if (access(AUDIO_EFFECT_VENDOR_CONFIG_FILE, R_OK) == 0) {
        return loadEffectConfigFile(AUDIO_EFFECT_VENDOR_CONFIG_FILE);
    } else if (access(AUDIO_EFFECT_DEFAULT_CONFIG_FILE, R_OK) == 0) {
        return loadEffectConfigFile(AUDIO_EFFECT_DEFAULT_CONFIG_FILE);
    }
    return 0;
}

int loadEffectConfigFile(const char *path)
{
    cnode *root;
    char *data;

    data = load_file(path, NULL);
    if (data == NULL) {
        return -ENODEV;
    }
    root = config_node("", "");
    config_load(root, data);
    loadLibraries(root);
    loadEffects(root);
    config_free(root);
    free(root);
    free(data);

    return 0;
}

int loadLibraries(cnode *root)
{
    cnode *node;

    node = config_find(root, LIBRARIES_TAG);
    if (node == NULL) {
        return -ENOENT;
    }
    node = node->first_child;
    while (node) {
        loadLibrary(node, node->name);
        node = node->next;
    }
    return 0;
}

#ifdef __LP64__
// audio_effects.conf always specifies 32 bit lib path: convert to 64 bit path if needed
static const char *kLibraryPathRoot[] =
        {"/odm/lib64/soundfx", "/vendor/lib64/soundfx", "/system/lib64/soundfx"};
#else
static const char *kLibraryPathRoot[] =
        {"/odm/lib/soundfx", "/vendor/lib/soundfx", "/system/lib/soundfx"};
#endif

static const int kLibraryPathRootSize =
        (sizeof(kLibraryPathRoot) / sizeof(kLibraryPathRoot[0]));

// Checks if the library path passed as lib_path_in can be opened and if not
// tries in standard effect library directories with just the library name and returns correct path
// in lib_path_out
int checkLibraryPath(const char *lib_path_in, char *lib_path_out) {
    char *str;
    const char *lib_name;
    size_t len;

    if (lib_path_in == NULL || lib_path_out == NULL) {
        return -EINVAL;
    }

    strlcpy(lib_path_out, lib_path_in, PATH_MAX);

    // Try exact path first
    str = strstr(lib_path_out, "/lib/soundfx/");
    if (str == NULL) {
        return -EINVAL;
    }

    // Extract library name from input path
    len = str - lib_path_out;
    lib_name = lib_path_in + len + strlen("/lib/soundfx/");

    // Then try with library name and standard path names in order of preference
    for (int i = 0; i < kLibraryPathRootSize; i++) {
        char path[PATH_MAX];

        snprintf(path,
                 PATH_MAX,
                 "%s/%s",
                 kLibraryPathRoot[i],
                 lib_name);
        if (F_OK == access(path, 0)) {
            strcpy(lib_path_out, path);
            ALOGW_IF(strncmp(lib_path_out, lib_path_in, PATH_MAX) != 0,
                "checkLibraryPath() corrected library path %s to %s", lib_path_in, lib_path_out);
            return 0;
        }
    }
    return -EINVAL;
}



int loadLibrary(cnode *root, const char *name)
{
    cnode *node;
    void *hdl = NULL;
    audio_effect_library_t *desc;
    list_elem_t *e;
    lib_entry_t *l;
    char path[PATH_MAX];

    node = config_find(root, PATH_TAG);
    if (node == NULL) {
        return -EINVAL;
    }

    if (checkLibraryPath((const char *)node->value, path) != 0) {
        ALOGW("loadLibrary() could not find library %s", path);
        goto error;
    }

    hdl = dlopen(path, RTLD_NOW);
    if (hdl == NULL) {
        ALOGW("loadLibrary() failed to open %s", path);
        goto error;
    }

    desc = (audio_effect_library_t *)dlsym(hdl, AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR);
    if (desc == NULL) {
        ALOGW("loadLibrary() could not find symbol %s", AUDIO_EFFECT_LIBRARY_INFO_SYM_AS_STR);
        goto error;
    }

    if (AUDIO_EFFECT_LIBRARY_TAG != desc->tag) {
        ALOGW("getLibrary() bad tag %08x in lib info struct", desc->tag);
        goto error;
    }

    if (EFFECT_API_VERSION_MAJOR(desc->version) !=
            EFFECT_API_VERSION_MAJOR(EFFECT_LIBRARY_API_VERSION)) {
        ALOGW("loadLibrary() bad lib version %08x", desc->version);
        goto error;
    }

    // add entry for library in gLibraryList
    l = malloc(sizeof(lib_entry_t));
    l->name = strndup(name, PATH_MAX);
    l->path = strndup(path, PATH_MAX);
    l->handle = hdl;
    l->desc = desc;
    l->effects = NULL;
    pthread_mutex_init(&l->lock, NULL);

    e = malloc(sizeof(list_elem_t));
    e->object = l;
    pthread_mutex_lock(&gLibLock);
    e->next = gLibraryList;
    gLibraryList = e;
    pthread_mutex_unlock(&gLibLock);
    ALOGV("getLibrary() linked library %p for path %s", l, path);

    return 0;

error:
    if (hdl != NULL) {
        dlclose(hdl);
    }
    //add entry for library errors in gLibraryFailedList
    lib_failed_entry_t *fl = malloc(sizeof(lib_failed_entry_t));
    fl->name = strndup(name, PATH_MAX);
    fl->path = strndup(path, PATH_MAX);

    list_elem_t *fe = malloc(sizeof(list_elem_t));
    fe->object = fl;
    fe->next = gLibraryFailedList;
    gLibraryFailedList = fe;
    ALOGV("getLibrary() linked error in library %p for path %s", fl, path);

    return -EINVAL;
}

// This will find the library and UUID tags of the sub effect pointed by the
// node, gets the effect descriptor and lib_entry_t and adds the subeffect -
// sub_entry_t to the gSubEffectList
int addSubEffect(cnode *root)
{
    ALOGV("addSubEffect");
    cnode *node;
    effect_uuid_t uuid;
    effect_descriptor_t *d;
    lib_entry_t *l;
    list_elem_t *e;
    node = config_find(root, LIBRARY_TAG);
    if (node == NULL) {
        return -EINVAL;
    }
    l = getLibrary(node->value);
    if (l == NULL) {
        ALOGW("addSubEffect() could not get library %s", node->value);
        return -EINVAL;
    }
    node = config_find(root, UUID_TAG);
    if (node == NULL) {
        return -EINVAL;
    }
    if (stringToUuid(node->value, &uuid) != 0) {
        ALOGW("addSubEffect() invalid uuid %s", node->value);
        return -EINVAL;
    }
    d = malloc(sizeof(effect_descriptor_t));
    if (l->desc->get_descriptor(&uuid, d) != 0) {
        char s[40];
        uuidToString(&uuid, s, 40);
        ALOGW("Error querying effect %s on lib %s", s, l->name);
        free(d);
        return -EINVAL;
    }
#if (LOG_NDEBUG==0)
    char s[512];
    dumpEffectDescriptor(d, s, sizeof(s), 0 /* indent */);
    ALOGV("addSubEffect() read descriptor %p:%s",d, s);
#endif
    if (EFFECT_API_VERSION_MAJOR(d->apiVersion) !=
            EFFECT_API_VERSION_MAJOR(EFFECT_CONTROL_API_VERSION)) {
        ALOGW("Bad API version %08x on lib %s", d->apiVersion, l->name);
        free(d);
        return -EINVAL;
    }
    sub_effect_entry_t *sub_effect = malloc(sizeof(sub_effect_entry_t));
    sub_effect->object = d;
    // lib_entry_t is stored since the sub effects are not linked to the library
    sub_effect->lib = l;
    e = malloc(sizeof(list_elem_t));
    e->object = sub_effect;
    e->next = gSubEffectList->sub_elem;
    gSubEffectList->sub_elem = e;
    ALOGV("addSubEffect end");
    return 0;
}

int loadEffects(cnode *root)
{
    cnode *node;

    node = config_find(root, EFFECTS_TAG);
    if (node == NULL) {
        return -ENOENT;
    }
    node = node->first_child;
    while (node) {
        loadEffect(node);
        node = node->next;
    }
    return 0;
}

int loadEffect(cnode *root)
{
    cnode *node;
    effect_uuid_t uuid;
    lib_entry_t *l;
    effect_descriptor_t *d;
    list_elem_t *e;

    node = config_find(root, LIBRARY_TAG);
    if (node == NULL) {
        return -EINVAL;
    }

    l = getLibrary(node->value);
    if (l == NULL) {
        ALOGW("loadEffect() could not get library %s", node->value);
        return -EINVAL;
    }

    node = config_find(root, UUID_TAG);
    if (node == NULL) {
        return -EINVAL;
    }
    if (stringToUuid(node->value, &uuid) != 0) {
        ALOGW("loadEffect() invalid uuid %s", node->value);
        return -EINVAL;
    }
    lib_entry_t *tmp;
    bool skip = false;
    if (findEffect(NULL, &uuid, &tmp, NULL) == 0) {
        ALOGW("skipping duplicate uuid %s %s", node->value,
                node->next ? "and its sub-effects" : "");
        skip = true;
    }

    d = malloc(sizeof(effect_descriptor_t));
    if (l->desc->get_descriptor(&uuid, d) != 0) {
        char s[40];
        uuidToString(&uuid, s, 40);
        ALOGW("Error querying effect %s on lib %s", s, l->name);
        free(d);
        return -EINVAL;
    }
#if (LOG_NDEBUG==0)
    char s[512];
    dumpEffectDescriptor(d, s, sizeof(s), 0 /* indent */);
    ALOGV("loadEffect() read descriptor %p:%s",d, s);
#endif
    if (EFFECT_API_VERSION_MAJOR(d->apiVersion) !=
            EFFECT_API_VERSION_MAJOR(EFFECT_CONTROL_API_VERSION)) {
        ALOGW("Bad API version %08x on lib %s", d->apiVersion, l->name);
        free(d);
        return -EINVAL;
    }
    e = malloc(sizeof(list_elem_t));
    e->object = d;
    if (skip) {
        e->next = gSkippedEffects;
        gSkippedEffects = e;
        return -EINVAL;
    } else {
        e->next = l->effects;
        l->effects = e;
    }

    // After the UUID node in the config_tree, if node->next is valid,
    // that would be sub effect node.
    // Find the sub effects and add them to the gSubEffectList
    node = node->next;
    int count = 2;
    bool hwSubefx = false, swSubefx = false;
    list_sub_elem_t *sube = NULL;
    if (node != NULL) {
        ALOGV("Adding the effect to gEffectSubList as there are sub effects");
        sube = malloc(sizeof(list_sub_elem_t));
        sube->object = d;
        sube->sub_elem = NULL;
        sube->next = gSubEffectList;
        gSubEffectList = sube;
    }
    while (node != NULL && count) {
       if (addSubEffect(node)) {
           ALOGW("loadEffect() could not add subEffect %s", node->value);
           // Change the gSubEffectList to point to older list;
           gSubEffectList = sube->next;
           free(sube->sub_elem);// Free an already added sub effect
           sube->sub_elem = NULL;
           free(sube);
           return -ENOENT;
       }
       sub_effect_entry_t *subEntry = (sub_effect_entry_t*)gSubEffectList->sub_elem->object;
       effect_descriptor_t *subEffectDesc = (effect_descriptor_t*)(subEntry->object);
       // Since we return a dummy descriptor for the proxy during
       // get_descriptor call,we replace it with the correspoding
       // sw effect descriptor, but with Proxy UUID
       // check for Sw desc
        if (!((subEffectDesc->flags & EFFECT_FLAG_HW_ACC_MASK) ==
                                           EFFECT_FLAG_HW_ACC_TUNNEL)) {
             swSubefx = true;
             *d = *subEffectDesc;
             d->uuid = uuid;
             ALOGV("loadEffect() Changed the Proxy desc");
       } else
           hwSubefx = true;
       count--;
       node = node->next;
    }
    // 1 HW and 1 SW sub effect found. Set the offload flag in the Proxy desc
    if (hwSubefx && swSubefx) {
        d->flags |= EFFECT_FLAG_OFFLOAD_SUPPORTED;
    }
    return 0;
}

lib_entry_t *getLibrary(const char *name)
{
    list_elem_t *e;

    if (gCachedLibrary &&
            !strncmp(gCachedLibrary->name, name, PATH_MAX)) {
        return gCachedLibrary;
    }

    e = gLibraryList;
    while (e) {
        lib_entry_t *l = (lib_entry_t *)e->object;
        if (!strcmp(l->name, name)) {
            gCachedLibrary = l;
            return l;
        }
        e = e->next;
    }

    return NULL;
}
