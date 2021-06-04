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

/* 3DDoppler implementation */

#include "sles_allinclusive.h"


static SLresult I3DDoppler_SetVelocityCartesian(SL3DDopplerItf self, const SLVec3D *pVelocity)
{
    SL_ENTER_INTERFACE

    if (NULL == pVelocity) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        I3DDoppler *thiz = (I3DDoppler *) self;
        SLVec3D velocityCartesian = *pVelocity;
        interface_lock_exclusive(thiz);
        thiz->mVelocityCartesian = velocityCartesian;
        thiz->mVelocityActive = CARTESIAN_SET_SPHERICAL_UNKNOWN;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult I3DDoppler_SetVelocitySpherical(SL3DDopplerItf self,
    SLmillidegree azimuth, SLmillidegree elevation, SLmillimeter speed)
{
    SL_ENTER_INTERFACE

    I3DDoppler *thiz = (I3DDoppler *) self;
    interface_lock_exclusive(thiz);
    thiz->mVelocitySpherical.mAzimuth = azimuth;
    thiz->mVelocitySpherical.mElevation = elevation;
    thiz->mVelocitySpherical.mSpeed = speed;
    thiz->mVelocityActive = CARTESIAN_UNKNOWN_SPHERICAL_SET;
    interface_unlock_exclusive(thiz);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult I3DDoppler_GetVelocityCartesian(SL3DDopplerItf self, SLVec3D *pVelocity)
{
    SL_ENTER_INTERFACE

    if (NULL == pVelocity) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        I3DDoppler *thiz = (I3DDoppler *) self;
        interface_lock_exclusive(thiz);
        for (;;) {
            enum CartesianSphericalActive velocityActive = thiz->mVelocityActive;
            switch (velocityActive) {
            case CARTESIAN_COMPUTED_SPHERICAL_SET:
            case CARTESIAN_SET_SPHERICAL_COMPUTED:  // not in 1.0.1
            case CARTESIAN_SET_SPHERICAL_REQUESTED: // not in 1.0.1
            case CARTESIAN_SET_SPHERICAL_UNKNOWN:
                {
                SLVec3D velocityCartesian = thiz->mVelocityCartesian;
                interface_unlock_exclusive(thiz);
                *pVelocity = velocityCartesian;
                }
                break;
            case CARTESIAN_UNKNOWN_SPHERICAL_SET:
                thiz->mVelocityActive = CARTESIAN_REQUESTED_SPHERICAL_SET;
                FALLTHROUGH_INTENDED;
            case CARTESIAN_REQUESTED_SPHERICAL_SET:
                // matched by cond_broadcast in case multiple requesters
#if 0
                interface_cond_wait(thiz);
#else
                thiz->mVelocityActive = CARTESIAN_COMPUTED_SPHERICAL_SET;
#endif
                continue;
            default:
                assert(SL_BOOLEAN_FALSE);
                interface_unlock_exclusive(thiz);
                pVelocity->x = 0;
                pVelocity->y = 0;
                pVelocity->z = 0;
                break;
            }
            break;
        }
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult I3DDoppler_SetDopplerFactor(SL3DDopplerItf self, SLpermille dopplerFactor)
{
    SL_ENTER_INTERFACE

    I3DDoppler *thiz = (I3DDoppler *) self;
    interface_lock_poke(thiz);
    thiz->mDopplerFactor = dopplerFactor;
    interface_unlock_poke(thiz);
    result = SL_RESULT_SUCCESS;

    SL_LEAVE_INTERFACE
}


static SLresult I3DDoppler_GetDopplerFactor(SL3DDopplerItf self, SLpermille *pDopplerFactor)
{
    SL_ENTER_INTERFACE

    if (NULL == pDopplerFactor) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        I3DDoppler *thiz = (I3DDoppler *) self;
        interface_lock_peek(thiz);
        SLpermille dopplerFactor = thiz->mDopplerFactor;
        interface_unlock_peek(thiz);
        *pDopplerFactor = dopplerFactor;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SL3DDopplerItf_ I3DDoppler_Itf = {
    I3DDoppler_SetVelocityCartesian,
    I3DDoppler_SetVelocitySpherical,
    I3DDoppler_GetVelocityCartesian,
    I3DDoppler_SetDopplerFactor,
    I3DDoppler_GetDopplerFactor
};

void I3DDoppler_init(void *self)
{
    I3DDoppler *thiz = (I3DDoppler *) self;
    thiz->mItf = &I3DDoppler_Itf;
    thiz->mVelocityCartesian.x = 0;
    thiz->mVelocityCartesian.y = 0;
    thiz->mVelocityCartesian.z = 0;
    memset(&thiz->mVelocitySpherical, 0x55, sizeof(thiz->mVelocitySpherical));
    thiz->mVelocityActive = CARTESIAN_SET_SPHERICAL_UNKNOWN;
    thiz->mDopplerFactor = 1000;
}
