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

/* 3DGrouping implementation */

#include "sles_allinclusive.h"


static SLresult I3DGrouping_Set3DGroup(SL3DGroupingItf self, SLObjectItf group)
{
    SL_ENTER_INTERFACE

    // validate input parameters
    C3DGroup *newGroup = (C3DGroup *) group;
    result = SL_RESULT_SUCCESS;
    if (NULL != newGroup) {
        // check that new group has the correct object ID and is realized, and acquire a strong
        // reference to it. FYI note that a deadlock will occur if application incorrectly
        // specifies group as this audio player
        result = AcquireStrongRef(&newGroup->mObject, SL_OBJECTID_3DGROUP);
        // the new group is left unlocked, but it will be locked again below
    }
    if (SL_RESULT_SUCCESS == result) {
        I3DGrouping *thiz = (I3DGrouping *) self;
        IObject *thisObject = InterfaceToIObject(thiz);
        unsigned id = thisObject->mInstanceID;
        assert(0 != id);        // player object must be published by this point
        --id;
        assert(MAX_INSTANCE > id);
        unsigned mask = 1 << id;
        interface_lock_exclusive(thiz);
        C3DGroup *oldGroup = thiz->mGroup;
        if (newGroup != oldGroup) {
            // remove this object from the old group's set of objects
            if (NULL != oldGroup) {
                IObject *oldGroupObject = &oldGroup->mObject;
                // note that we already have a strong reference to the old group
                object_lock_exclusive(oldGroupObject);
                assert(oldGroup->mMemberMask & mask);
                oldGroup->mMemberMask &= ~mask;
                ReleaseStrongRefAndUnlockExclusive(oldGroupObject);
            }
            // add this object to the new group's set of objects
            if (NULL != newGroup) {
                IObject *newGroupObject = &newGroup->mObject;
                // we already have a strong reference to the new group, but we need to re-lock it
                // so that we always lock objects in the same nesting order to prevent a deadlock
                object_lock_exclusive(newGroupObject);
                assert(!(newGroup->mMemberMask & mask));
                newGroup->mMemberMask |= mask;
                object_unlock_exclusive(newGroupObject);
            }
            thiz->mGroup = newGroup;
        }
        interface_unlock_exclusive(thiz);
    }

    SL_LEAVE_INTERFACE
}


static SLresult I3DGrouping_Get3DGroup(SL3DGroupingItf self, SLObjectItf *pGroup)
{
    SL_ENTER_INTERFACE

    if (NULL == pGroup) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        I3DGrouping *thiz = (I3DGrouping *) self;
        interface_lock_shared(thiz);
        C3DGroup *group = thiz->mGroup;
        *pGroup = (NULL != group) ? &group->mObject.mItf : NULL;
        interface_unlock_shared(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SL3DGroupingItf_ I3DGrouping_Itf = {
    I3DGrouping_Set3DGroup,
    I3DGrouping_Get3DGroup
};

void I3DGrouping_init(void *self)
{
    I3DGrouping *thiz = (I3DGrouping *) self;
    thiz->mItf = &I3DGrouping_Itf;
    thiz->mGroup = NULL;
}

void I3DGrouping_deinit(void *self)
{
    I3DGrouping *thiz = (I3DGrouping *) self;
    C3DGroup *group = thiz->mGroup;
    if (NULL != group) {
        unsigned mask = 1 << (InterfaceToIObject(thiz)->mInstanceID - 1);
        IObject *groupObject = &group->mObject;
        object_lock_exclusive(groupObject);
        assert(group->mMemberMask & mask);
        group->mMemberMask &= ~mask;
        ReleaseStrongRefAndUnlockExclusive(groupObject);
    }
}
