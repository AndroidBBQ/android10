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

/* ThreadSync implementation */

#include "sles_allinclusive.h"


static SLresult IThreadSync_EnterCriticalSection(SLThreadSyncItf self)
{
    SL_ENTER_INTERFACE

    IThreadSync *thiz = (IThreadSync *) self;
    interface_lock_exclusive(thiz);
    for (;;) {
        if (thiz->mInCriticalSection) {
            if (!pthread_equal(thiz->mOwner, pthread_self())) {
                ++thiz->mWaiting;
                interface_cond_wait(thiz);
                continue;
            }
            // nested locks are not allowed
            result = SL_RESULT_PRECONDITIONS_VIOLATED;
            break;
        }
        thiz->mInCriticalSection = SL_BOOLEAN_TRUE;
        thiz->mOwner = pthread_self();
        result = SL_RESULT_SUCCESS;
        break;
    }
    interface_unlock_exclusive(thiz);

    SL_LEAVE_INTERFACE
}


static SLresult IThreadSync_ExitCriticalSection(SLThreadSyncItf self)
{
    SL_ENTER_INTERFACE

    IThreadSync *thiz = (IThreadSync *) self;
    interface_lock_exclusive(thiz);
    if (!thiz->mInCriticalSection || !pthread_equal(thiz->mOwner, pthread_self())) {
        result = SL_RESULT_PRECONDITIONS_VIOLATED;
    } else {
        thiz->mInCriticalSection = SL_BOOLEAN_FALSE;
        memset(&thiz->mOwner, 0, sizeof(pthread_t));
        if (thiz->mWaiting) {
            --thiz->mWaiting;
            interface_cond_signal(thiz);
        }
        result = SL_RESULT_SUCCESS;
    }
    interface_unlock_exclusive(thiz);

    SL_LEAVE_INTERFACE
}


static const struct SLThreadSyncItf_ IThreadSync_Itf = {
    IThreadSync_EnterCriticalSection,
    IThreadSync_ExitCriticalSection
};

void IThreadSync_init(void *self)
{
    IThreadSync *thiz = (IThreadSync *) self;
    thiz->mItf = &IThreadSync_Itf;
    thiz->mInCriticalSection = SL_BOOLEAN_FALSE;
    thiz->mWaiting = 0;
    memset(&thiz->mOwner, 0, sizeof(pthread_t));
}

void IThreadSync_deinit(void *self)
{
    IThreadSync *thiz = (IThreadSync *) self;
    if (thiz->mInCriticalSection) {
        SL_LOGW("ThreadSync::EnterCriticalSection was active at Engine::Destroy");
    }
}
