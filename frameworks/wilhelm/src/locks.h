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

#include "Configuration.h"

/** \file locks.h Mutual exclusion and condition variables */

#ifdef USE_DEBUG
extern void object_lock_exclusive_(IObject *thiz, const char *file, int line);
extern void object_unlock_exclusive_(IObject *thiz, const char *file, int line);
extern void object_unlock_exclusive_attributes_(IObject *thiz, unsigned attr,
    const char *file, int line);
extern void object_cond_wait_(IObject *thiz, const char *file, int line);
#else
extern void object_lock_exclusive(IObject *thiz);
extern void object_unlock_exclusive(IObject *thiz);
extern void object_unlock_exclusive_attributes(IObject *thiz, unsigned attr);
extern void object_cond_wait(IObject *thiz);
#endif
extern void object_cond_signal(IObject *thiz);
extern void object_cond_broadcast(IObject *thiz);

#ifdef USE_DEBUG
#define object_lock_exclusive(thiz) object_lock_exclusive_((thiz), __FILE__, __LINE__)
#define object_unlock_exclusive(thiz) object_unlock_exclusive_((thiz), __FILE__, __LINE__)
#define object_unlock_exclusive_attributes(thiz, attr) \
    object_unlock_exclusive_attributes_((thiz), (attr), __FILE__, __LINE__)
#define object_cond_wait(thiz) object_cond_wait_((thiz), __FILE__, __LINE__)
#endif

// Currently shared locks are implemented as exclusive, but don't count on it

#define object_lock_shared(thiz)   object_lock_exclusive(thiz)
#define object_unlock_shared(thiz) object_unlock_exclusive(thiz)

// Currently interface locks are actually on whole object, but don't count on it.
// These operations are undefined on IObject, as it lacks an mThis.
// If you have an IObject, then use the object_ functions instead.

#define interface_lock_exclusive(thiz)   object_lock_exclusive(InterfaceToIObject(thiz))
#define interface_unlock_exclusive(thiz) object_unlock_exclusive(InterfaceToIObject(thiz))
#define interface_unlock_exclusive_attributes(thiz, attr) \
    object_unlock_exclusive_attributes(InterfaceToIObject(thiz), (attr))
#define interface_lock_shared(thiz)      object_lock_shared(InterfaceToIObject(thiz))
#define interface_unlock_shared(thiz)    object_unlock_shared(InterfaceToIObject(thiz))
#define interface_cond_wait(thiz)        object_cond_wait(InterfaceToIObject(thiz))
#define interface_cond_signal(thiz)      object_cond_signal(InterfaceToIObject(thiz))
#define interface_cond_broadcast(thiz)   object_cond_broadcast(InterfaceToIObject(thiz))

// Peek and poke are an optimization for small atomic fields that don't "matter".
// Don't use for struct, as struct copy might not be atomic.
// On uniprocessor they can be no-ops, on SMP they could be memory barriers but locks are easier.

#define object_lock_peek(thiz)      object_lock_shared(thiz)
#define object_unlock_peek(thiz)    object_unlock_shared(thiz)
#define interface_lock_poke(thiz)   interface_lock_exclusive(thiz)
#define interface_unlock_poke(thiz) interface_unlock_exclusive(thiz)
#define interface_lock_peek(thiz)   interface_lock_shared(thiz)
#define interface_unlock_peek(thiz) interface_unlock_shared(thiz)
