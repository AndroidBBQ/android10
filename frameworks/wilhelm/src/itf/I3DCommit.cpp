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

/* 3DCommit implementation */

#include "sles_allinclusive.h"


static SLresult I3DCommit_Commit(SL3DCommitItf self)
{
    SL_ENTER_INTERFACE

    I3DCommit *thiz = (I3DCommit *) self;
    IObject *thisObject = InterfaceToIObject(thiz);
    object_lock_exclusive(thisObject);
    if (thiz->mDeferred) {
        SLuint32 myGeneration = thiz->mGeneration;
        do {
            ++thiz->mWaiting;
            object_cond_wait(thisObject);
        } while (thiz->mGeneration == myGeneration);
    }
    object_unlock_exclusive(thisObject);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult I3DCommit_SetDeferred(SL3DCommitItf self, SLboolean deferred)
{
    SL_ENTER_INTERFACE

    I3DCommit *thiz = (I3DCommit *) self;
    IObject *thisObject = InterfaceToIObject(thiz);
    object_lock_exclusive(thisObject);
    thiz->mDeferred = SL_BOOLEAN_FALSE != deferred; // normalize
    object_unlock_exclusive(thisObject);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static const struct SL3DCommitItf_ I3DCommit_Itf = {
    I3DCommit_Commit,
    I3DCommit_SetDeferred
};

void I3DCommit_init(void *self)
{
    I3DCommit *thiz = (I3DCommit *) self;
    thiz->mItf = &I3DCommit_Itf;
    thiz->mDeferred = SL_BOOLEAN_FALSE;
    thiz->mGeneration = 0;
    thiz->mWaiting = 0;
}
