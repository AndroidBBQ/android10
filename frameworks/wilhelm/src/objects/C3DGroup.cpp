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

/** \file C3DGroup.c C3DGroup class */

#include "sles_allinclusive.h"


/** \brief Hook called by Object::Destroy before a 3D group is about to be destroyed */

predestroy_t C3DGroup_PreDestroy(void *self)
{
    C3DGroup *thiz = (C3DGroup *) self;
    // See design document for explanation
    if (0 == thiz->mMemberMask) {
        return predestroy_ok;
    }
    SL_LOGE("Object::Destroy(%p) for 3DGroup ignored; mMemberMask=0x%x", thiz, thiz->mMemberMask);
    return predestroy_error;
}
