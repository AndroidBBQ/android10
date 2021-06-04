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

/* Visualization implementation */

#include "sles_allinclusive.h"


static SLresult IVisualization_RegisterVisualizationCallback(SLVisualizationItf self,
    slVisualizationCallback callback, void *pContext, SLmilliHertz rate)
{
    SL_ENTER_INTERFACE

    if (!(0 < rate && rate <= 20000)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        IVisualization *thiz = (IVisualization *) self;
        interface_lock_exclusive(thiz);
        thiz->mCallback = callback;
        thiz->mContext = pContext;
        thiz->mRate = rate;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult IVisualization_GetMaxRate(SLVisualizationItf self, SLmilliHertz *pRate)
{
    SL_ENTER_INTERFACE

    if (NULL == pRate) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        *pRate = 20000;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SLVisualizationItf_ IVisualization_Itf = {
    IVisualization_RegisterVisualizationCallback,
    IVisualization_GetMaxRate
};

void IVisualization_init(void *self)
{
    IVisualization *thiz = (IVisualization *) self;
    thiz->mItf = &IVisualization_Itf;
    thiz->mCallback = NULL;
    thiz->mContext = NULL;
    thiz->mRate = 20000;
}
