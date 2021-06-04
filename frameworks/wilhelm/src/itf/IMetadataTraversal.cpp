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

/* MetadataTraversal implementation */

#include "sles_allinclusive.h"


static SLresult IMetadataTraversal_SetMode(SLMetadataTraversalItf self, SLuint32 mode)
{
    SL_ENTER_INTERFACE

    switch (mode) {
    case SL_METADATATRAVERSALMODE_ALL:
    case SL_METADATATRAVERSALMODE_NODE:
        {
        IMetadataTraversal *thiz = (IMetadataTraversal *) self;
        interface_lock_poke(thiz);
        thiz->mMode = mode;
        interface_unlock_poke(thiz);
        result = SL_RESULT_SUCCESS;
        }
        break;
    default:
        result = SL_RESULT_PARAMETER_INVALID;
        break;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMetadataTraversal_GetChildCount(SLMetadataTraversalItf self, SLuint32 *pCount)
{
    SL_ENTER_INTERFACE

    if (NULL == pCount) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMetadataTraversal *thiz = (IMetadataTraversal *) self;
        interface_lock_peek(thiz);
        SLuint32 count = thiz->mCount;
        interface_unlock_peek(thiz);
        *pCount = count;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMetadataTraversal_GetChildMIMETypeSize(
    SLMetadataTraversalItf self, SLuint32 index, SLuint32 *pSize)
{
    SL_ENTER_INTERFACE

    if (NULL == pSize) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IMetadataTraversal *thiz = (IMetadataTraversal *) self;
        interface_lock_peek(thiz);
        SLuint32 size = thiz->mSize;
        interface_unlock_peek(thiz);
        *pSize = size;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IMetadataTraversal_GetChildInfo(SLMetadataTraversalItf self, SLuint32 index,
    SLint32 *pNodeID, SLuint32 *pType, SLuint32 size, SLchar *pMimeType)
{
    SL_ENTER_INTERFACE

    //IMetadataTraversal *thiz = (IMetadataTraversal *) self;
    result = SL_RESULT_FEATURE_UNSUPPORTED;

    SL_LEAVE_INTERFACE
}


static SLresult IMetadataTraversal_SetActiveNode(SLMetadataTraversalItf self, SLuint32 index)
{
    SL_ENTER_INTERFACE

    if (SL_NODE_PARENT == index) {
        ;
    }
    IMetadataTraversal *thiz = (IMetadataTraversal *) self;
    thiz->mIndex = index;
    result = SL_RESULT_PARAMETER_INVALID;

    SL_LEAVE_INTERFACE
}


static const struct SLMetadataTraversalItf_ IMetadataTraversal_Itf = {
    IMetadataTraversal_SetMode,
    IMetadataTraversal_GetChildCount,
    IMetadataTraversal_GetChildMIMETypeSize,
    IMetadataTraversal_GetChildInfo,
    IMetadataTraversal_SetActiveNode
};

void IMetadataTraversal_init(void *self)
{
    IMetadataTraversal *thiz = (IMetadataTraversal *) self;
    thiz->mItf = &IMetadataTraversal_Itf;
    thiz->mIndex = 0;
    thiz->mMode = SL_METADATATRAVERSALMODE_NODE;
    thiz->mCount = 0;
    thiz->mSize = 0;
}
