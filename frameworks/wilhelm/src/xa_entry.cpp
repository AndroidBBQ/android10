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

/** \brief xaCreateEngine Function */

XA_API XAresult XAAPIENTRY xaCreateEngine(XAObjectItf *pEngine, XAuint32 numOptions,
        const XAEngineOption *pEngineOptions, XAuint32 numInterfaces,
        const XAInterfaceID *pInterfaceIds, const XAboolean *pInterfaceRequired)
{
    XA_ENTER_GLOBAL

    result = liCreateEngine((SLObjectItf *) pEngine, numOptions,
            (const SLEngineOption *) pEngineOptions, numInterfaces,
            (const SLInterfaceID *) pInterfaceIds, (const SLboolean *) pInterfaceRequired,
            objectIDtoClass(XA_OBJECTID_ENGINE));

    XA_LEAVE_GLOBAL
}


/** \brief xaQueryNumSupportedEngineInterfaces Function */

XA_API XAresult XAAPIENTRY xaQueryNumSupportedEngineInterfaces(XAuint32 *pNumSupportedInterfaces)
{
    XA_ENTER_GLOBAL

    result = liQueryNumSupportedInterfaces(pNumSupportedInterfaces,
            objectIDtoClass(XA_OBJECTID_ENGINE));

    XA_LEAVE_GLOBAL
}


/** \brief xaQuerySupportedEngineInterfaces Function */

XA_API XAresult XAAPIENTRY xaQuerySupportedEngineInterfaces(XAuint32 index,
        XAInterfaceID *pInterfaceId)
{
    XA_ENTER_GLOBAL

    result = liQuerySupportedInterfaces(index, (SLInterfaceID *) pInterfaceId,
            objectIDtoClass(XA_OBJECTID_ENGINE));

    XA_LEAVE_GLOBAL
}
