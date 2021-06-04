/*
 * Copyright (C) 2018 The Android Open Source Project
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

#define LOG_TAG "MediaMetrics"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <media/MediaAnalyticsItem.h>
#include <media/MediaMetrics.h>

//
// provide a C-ish interface that is easier to stabilize than the existing C++
// interface
//
// ALL functions returning a char * give responsibility for the allocated buffer
// to the caller. The caller is responsible to call free() on that pointer.
//

// manage the overall record
mediametrics_handle_t mediametrics_create(mediametricskey_t key) {
    android::MediaAnalyticsItem *item = android::MediaAnalyticsItem::create(key);
    return (mediametrics_handle_t) item;
}

void mediametrics_delete(mediametrics_handle_t handle) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item == NULL) return;
    delete item;
}

mediametricskey_t mediametrics_getKey(mediametrics_handle_t handle) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item == NULL) return NULL;
    return strdup(item->getKey().c_str());
}

// nuplayer, et al use it when acting as proxies
void mediametrics_setUid(mediametrics_handle_t handle, uid_t uid) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item != NULL) item->setUid(uid);
}

// set attributes
//

void mediametrics_setInt32(mediametrics_handle_t handle, attr_t attr,
                                int32_t value) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item != NULL) item->setInt32(attr, value);
}

void mediametrics_setInt64(mediametrics_handle_t handle, attr_t attr,
                                int64_t value) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item != NULL) item->setInt64(attr, value);
}

void mediametrics_setDouble(mediametrics_handle_t handle, attr_t attr,
                                 double value) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item != NULL) item->setDouble(attr, value);
}

void mediametrics_setRate(mediametrics_handle_t handle, attr_t attr,
                               int64_t count, int64_t duration) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item != NULL) item->setRate(attr, count, duration);
}

void mediametrics_setCString(mediametrics_handle_t handle, attr_t attr,
                                 const char *value) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item != NULL) item->setCString(attr, value);
}

// fused get/add/set; if attr wasn't there, it's a simple set.
//

void mediametrics_addInt32(mediametrics_handle_t handle, attr_t attr,
                                int32_t value) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item != NULL) item->addInt32(attr, value);
}

void mediametrics_addInt64(mediametrics_handle_t handle, attr_t attr,
                                int64_t value) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item != NULL) item->addInt64(attr, value);
}

void mediametrics_addDouble(mediametrics_handle_t handle, attr_t attr,
                                 double value) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item != NULL) item->addDouble(attr, value);
}

void mediametrics_addRate(mediametrics_handle_t handle, attr_t attr,
                               int64_t count, int64_t duration) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item != NULL) item->addRate(attr, count, duration);
}

// find & extract values
// return indicates whether attr exists (and thus whether value filled in)
// NULL parameter value suppresses storage of value.
//

bool mediametrics_getInt32(mediametrics_handle_t handle, attr_t attr,
                                int32_t * value) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item == NULL) return false;
    return item->getInt32(attr, value);
}

bool mediametrics_getInt64(mediametrics_handle_t handle, attr_t attr,
                                int64_t * value) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item == NULL) return false;
    return item->getInt64(attr, value);
}

bool mediametrics_getDouble(mediametrics_handle_t handle, attr_t attr,
                                 double *value) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item == NULL) return false;
    return item->getDouble(attr, value);
}

bool mediametrics_getRate(mediametrics_handle_t handle, attr_t attr,
                               int64_t * count, int64_t * duration, double *rate) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item == NULL) return false;
    return item->getRate(attr, count, duration, rate);
}

// NB: caller owns the string that comes back, is responsible for freeing it
bool mediametrics_getCString(mediametrics_handle_t handle, attr_t attr,
                                 char **value) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item == NULL) return false;

    return item->getCString(attr, value);
}

// to release strings returned via getCString()
void mediametrics_freeCString(char *value) {
    free(value);
}

bool mediametrics_selfRecord(mediametrics_handle_t handle) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item == NULL) return false;
    return item->selfrecord();
}


const char *mediametrics_readable(mediametrics_handle_t handle) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item == NULL) return "";
    return item->toCString();
}

int32_t mediametrics_count(mediametrics_handle_t handle) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item == NULL) return 0;
    return item->count();
}

bool mediametrics_isEnabled() {
    // static, so doesn't need an instance
    return android::MediaAnalyticsItem::isEnabled();
}

bool mediametrics_getAttributes(mediametrics_handle_t handle, char **buffer, size_t *length) {
    android::MediaAnalyticsItem *item = (android::MediaAnalyticsItem *) handle;
    if (item == NULL) return false;
    return item->dumpAttributes(buffer, length);

}
