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

/* DynamicSource implementation */

#include "sles_allinclusive.h"


static SLresult IDynamicSource_SetSource(SLDynamicSourceItf self, SLDataSource *pDataSource)
{
    SL_ENTER_INTERFACE

    if (NULL == pDataSource) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IDynamicSource *thiz = (IDynamicSource *) self;
#if 0
        DataLocatorFormat myDataSource;
        SLresult result;
        result = checkDataSource("pDataSource", pDataSource, &myDataSource, DATALOCATOR_MASK_ALL,
                DATAFORMAT_MASK_ALL);
        // handle result here
#endif
        // need to lock the object, as a change to source can impact most of object
        IObject *thisObject = InterfaceToIObject(thiz);
        object_lock_exclusive(thisObject);
        // a bit of a simplification to say the least! (see Khronos bug 6728)
        thiz->mDataSource = pDataSource;
        object_unlock_exclusive(thisObject);
        result = SL_RESULT_FEATURE_UNSUPPORTED;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLDynamicSourceItf_ IDynamicSource_Itf = {
    IDynamicSource_SetSource
};

void IDynamicSource_init(void *self)
{
    IDynamicSource *thiz = (IDynamicSource *) self;
    thiz->mItf = &IDynamicSource_Itf;
    // mDataSource will be re-initialized later in the containing object constructor
    thiz->mDataSource = NULL;
}
