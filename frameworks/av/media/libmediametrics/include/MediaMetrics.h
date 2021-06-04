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

#ifndef ANDROID_MEDIA_MEDIAMETRICS_H
#define ANDROID_MEDIA_MEDIAMETRICS_H

//
// define a C interface to the media metrics functionality
//
// All functions that return a char * or const char * also give responsibility
// for that string to the caller. The caller is responsible for calling free()
// on that pointer when done using the value.

__BEGIN_DECLS

// internally re-cast to the behind-the-scenes C++ class instance
typedef int64_t mediametrics_handle_t;
typedef const char *mediametricskey_t;
typedef const char *attr_t;

mediametrics_handle_t mediametrics_create(mediametricskey_t key);
void mediametrics_delete(mediametrics_handle_t handle);

mediametricskey_t mediametrics_getKey(mediametrics_handle_t handle);


// set
void mediametrics_setInt32(mediametrics_handle_t handle, attr_t attr,
                           int32_t value);
void mediametrics_setInt64(mediametrics_handle_t handle, attr_t attr,
                           int64_t value);
void mediametrics_setDouble(mediametrics_handle_t handle, attr_t attr,
                            double value);
void mediametrics_setRate(mediametrics_handle_t handle, attr_t attr,
                          int64_t count, int64_t duration);
void mediametrics_setCString(mediametrics_handle_t handle, attr_t attr,
                            const char * value);

// fused get/add/set; if attr wasn't there, it's a simple set.
// these do not provide atomicity or mutual exclusion, only simpler code sequences.
void mediametrics_addInt32(mediametrics_handle_t handle, attr_t attr,
                           int32_t value);
void mediametrics_addInt64(mediametrics_handle_t handle, attr_t attr,
                           int64_t value);
void mediametrics_addDouble(mediametrics_handle_t handle, attr_t attr,
                            double value);
void mediametrics_addRate(mediametrics_handle_t handle, attr_t attr,
                          int64_t count, int64_t duration);

// find & extract values
// return indicates whether attr exists (and thus whether value filled in)
// NULL parameter value suppresses storage of value.
bool mediametrics_getInt32(mediametrics_handle_t handle, attr_t attr,
                           int32_t * value);
bool mediametrics_getInt64(mediametrics_handle_t handle, attr_t attr,
                           int64_t * value);
bool mediametrics_getDouble(mediametrics_handle_t handle, attr_t attr,
                            double *value);
bool mediametrics_getRate(mediametrics_handle_t handle, attr_t attr,
                          int64_t * count, int64_t * duration, double *rate);
bool mediametrics_getCString(mediametrics_handle_t handle, attr_t attr,
                            char **value);
// to release strings returned via getCString()
void mediametrics_freeCString(char *value);

// # of attributes set within this record.
int32_t mediametrics_count(mediametrics_handle_t handle);

bool mediametrics_selfRecord(mediametrics_handle_t handle);

const char *mediametrics_readable(mediametrics_handle_t handle);
void mediametrics_setUid(mediametrics_handle_t handle, uid_t uid);
bool mediametrics_isEnabled();

// serialized copy of the attributes/values, mostly for upstream getMetrics() calls
// caller owns the buffer allocated as part of this call.
bool mediametrics_getAttributes(mediametrics_handle_t handle, char **buffer, size_t *length);

__END_DECLS

#endif
