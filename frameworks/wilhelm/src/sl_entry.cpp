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

#include "sles_allinclusive.h"

/** \brief slCreateEngine Function */

SL_API SLresult SLAPIENTRY slCreateEngine(SLObjectItf *pEngine, SLuint32 numOptions,
    const SLEngineOption *pEngineOptions, SLuint32 numInterfaces,
    const SLInterfaceID *pInterfaceIds, const SLboolean *pInterfaceRequired)
{
    SL_ENTER_GLOBAL

    result = liCreateEngine(pEngine, numOptions, pEngineOptions, numInterfaces, pInterfaceIds,
            pInterfaceRequired, objectIDtoClass(SL_OBJECTID_ENGINE));

    SL_LEAVE_GLOBAL
}



/** \brief slQueryNumSupportedEngineInterfaces Function */

SL_API SLresult SLAPIENTRY slQueryNumSupportedEngineInterfaces(SLuint32 *pNumSupportedInterfaces)
{
    SL_ENTER_GLOBAL

    result = liQueryNumSupportedInterfaces(pNumSupportedInterfaces,
            objectIDtoClass(SL_OBJECTID_ENGINE));

    SL_LEAVE_GLOBAL
}




/** \brief slQuerySupportedEngineInterfaces Function */

SL_API SLresult SLAPIENTRY slQuerySupportedEngineInterfaces(SLuint32 index,
        SLInterfaceID *pInterfaceId)
{
    SL_ENTER_GLOBAL

    result = liQuerySupportedInterfaces(index, pInterfaceId, objectIDtoClass(SL_OBJECTID_ENGINE));

    SL_LEAVE_GLOBAL
}
