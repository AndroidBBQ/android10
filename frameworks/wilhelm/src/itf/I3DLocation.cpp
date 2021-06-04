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

/* 3DLocation implementation */

#include "sles_allinclusive.h"


static SLresult I3DLocation_SetLocationCartesian(SL3DLocationItf self, const SLVec3D *pLocation)
{
    SL_ENTER_INTERFACE

    if (NULL == pLocation) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        I3DLocation *thiz = (I3DLocation *) self;
        SLVec3D locationCartesian = *pLocation;
        interface_lock_exclusive(thiz);
        thiz->mLocationCartesian = locationCartesian;
        thiz->mLocationActive = CARTESIAN_SET_SPHERICAL_UNKNOWN;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult I3DLocation_SetLocationSpherical(SL3DLocationItf self,
    SLmillidegree azimuth, SLmillidegree elevation, SLmillimeter distance)
{
    SL_ENTER_INTERFACE

    if (!((-360000 <= azimuth) && (azimuth <= 360000) &&
        (-90000 <= elevation) && (elevation <= 90000) &&
        (0 <= distance) && (distance <= SL_MILLIMETER_MAX))) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        I3DLocation *thiz = (I3DLocation *) self;
        interface_lock_exclusive(thiz);
        thiz->mLocationSpherical.mAzimuth = azimuth;
        thiz->mLocationSpherical.mElevation = elevation;
        thiz->mLocationSpherical.mDistance = distance;
        thiz->mLocationActive = CARTESIAN_UNKNOWN_SPHERICAL_SET;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult I3DLocation_Move(SL3DLocationItf self, const SLVec3D *pMovement)
{
    SL_ENTER_INTERFACE

    if (NULL == pMovement) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        I3DLocation *thiz = (I3DLocation *) self;
        SLVec3D movementCartesian = *pMovement;
        interface_lock_exclusive(thiz);
        for (;;) {
            enum CartesianSphericalActive locationActive = thiz->mLocationActive;
            switch (locationActive) {
            case CARTESIAN_COMPUTED_SPHERICAL_SET:
            case CARTESIAN_SET_SPHERICAL_COMPUTED:  // not in 1.0.1
            case CARTESIAN_SET_SPHERICAL_REQUESTED: // not in 1.0.1
            case CARTESIAN_SET_SPHERICAL_UNKNOWN:
                thiz->mLocationCartesian.x += movementCartesian.x;
                thiz->mLocationCartesian.y += movementCartesian.y;
                thiz->mLocationCartesian.z += movementCartesian.z;
                thiz->mLocationActive = CARTESIAN_SET_SPHERICAL_UNKNOWN;
                break;
            case CARTESIAN_UNKNOWN_SPHERICAL_SET:
                thiz->mLocationActive = CARTESIAN_REQUESTED_SPHERICAL_SET;
                FALLTHROUGH_INTENDED;
            case CARTESIAN_REQUESTED_SPHERICAL_SET:
                // matched by cond_broadcast in case multiple requesters
#if 0
                interface_cond_wait(thiz);
#else
                thiz->mLocationActive = CARTESIAN_COMPUTED_SPHERICAL_SET;
#endif
                continue;
            default:
                assert(SL_BOOLEAN_FALSE);
                break;
            }
            break;
        }
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult I3DLocation_GetLocationCartesian(SL3DLocationItf self, SLVec3D *pLocation)
{
    SL_ENTER_INTERFACE

    if (NULL == pLocation) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        I3DLocation *thiz = (I3DLocation *) self;
        interface_lock_exclusive(thiz);
        for (;;) {
            enum CartesianSphericalActive locationActive = thiz->mLocationActive;
            switch (locationActive) {
            case CARTESIAN_COMPUTED_SPHERICAL_SET:
            case CARTESIAN_SET_SPHERICAL_COMPUTED:  // not in 1.0.1
            case CARTESIAN_SET_SPHERICAL_REQUESTED: // not in 1.0.1
            case CARTESIAN_SET_SPHERICAL_UNKNOWN:
                {
                SLVec3D locationCartesian = thiz->mLocationCartesian;
                interface_unlock_exclusive(thiz);
                *pLocation = locationCartesian;
                }
                break;
            case CARTESIAN_UNKNOWN_SPHERICAL_SET:
                thiz->mLocationActive = CARTESIAN_REQUESTED_SPHERICAL_SET;
                FALLTHROUGH_INTENDED;
            case CARTESIAN_REQUESTED_SPHERICAL_SET:
                // matched by cond_broadcast in case multiple requesters
#if 0
                interface_cond_wait(thiz);
#else
                thiz->mLocationActive = CARTESIAN_COMPUTED_SPHERICAL_SET;
#endif
                continue;
            default:
                assert(SL_BOOLEAN_FALSE);
                interface_unlock_exclusive(thiz);
                pLocation->x = 0;
                pLocation->y = 0;
                pLocation->z = 0;
                break;
            }
            break;
        }
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult I3DLocation_SetOrientationVectors(SL3DLocationItf self,
    const SLVec3D *pFront, const SLVec3D *pAbove)
{
    SL_ENTER_INTERFACE

    if (NULL == pFront || NULL == pAbove) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        SLVec3D front = *pFront;
        SLVec3D above = *pAbove;
        // NTH Check for vectors close to zero or close to parallel
        I3DLocation *thiz = (I3DLocation *) self;
        interface_lock_exclusive(thiz);
        thiz->mOrientationVectors.mFront = front;
        thiz->mOrientationVectors.mAbove = above;
        thiz->mOrientationActive = ANGLES_UNKNOWN_VECTORS_SET;
        thiz->mRotatePending = SL_BOOLEAN_FALSE;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult I3DLocation_SetOrientationAngles(SL3DLocationItf self,
    SLmillidegree heading, SLmillidegree pitch, SLmillidegree roll)
{
    SL_ENTER_INTERFACE

    if (!((-360000 <= heading) && (heading <= 360000) &&
        (-90000 <= pitch) && (pitch <= 90000) &&
        (-360000 <= roll) && (roll <= 360000))) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        I3DLocation *thiz = (I3DLocation *) self;
        interface_lock_exclusive(thiz);
        thiz->mOrientationAngles.mHeading = heading;
        thiz->mOrientationAngles.mPitch = pitch;
        thiz->mOrientationAngles.mRoll = roll;
        thiz->mOrientationActive = ANGLES_SET_VECTORS_UNKNOWN;
        thiz->mRotatePending = SL_BOOLEAN_FALSE;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult I3DLocation_Rotate(SL3DLocationItf self, SLmillidegree theta, const SLVec3D *pAxis)
{
    SL_ENTER_INTERFACE

    if (!((-360000 <= theta) && (theta <= 360000)) || (NULL == pAxis)) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        SLVec3D axis = *pAxis;
        // NTH Check that axis is not (close to) zero vector, length does not matter
        I3DLocation *thiz = (I3DLocation *) self;
        interface_lock_exclusive(thiz);
        while (thiz->mRotatePending)
#if 0
            interface_cond_wait(thiz);
#else
            break;
#endif
        thiz->mTheta = theta;
        thiz->mAxis = axis;
        thiz->mRotatePending = SL_BOOLEAN_TRUE;
        interface_unlock_exclusive(thiz);
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static SLresult I3DLocation_GetOrientationVectors(SL3DLocationItf self,
    SLVec3D *pFront, SLVec3D *pUp)
{
    SL_ENTER_INTERFACE

    if (NULL == pFront || NULL == pUp) {
        result = SL_RESULT_PARAMETER_INVALID;
    } else {
        I3DLocation *thiz = (I3DLocation *) self;
        interface_lock_shared(thiz);
        SLVec3D front = thiz->mOrientationVectors.mFront;
        SLVec3D up = thiz->mOrientationVectors.mUp;
        interface_unlock_shared(thiz);
        *pFront = front;
        *pUp = up;
        result = SL_RESULT_SUCCESS;
    }

    SL_LEAVE_INTERFACE
}


static const struct SL3DLocationItf_ I3DLocation_Itf = {
    I3DLocation_SetLocationCartesian,
    I3DLocation_SetLocationSpherical,
    I3DLocation_Move,
    I3DLocation_GetLocationCartesian,
    I3DLocation_SetOrientationVectors,
    I3DLocation_SetOrientationAngles,
    I3DLocation_Rotate,
    I3DLocation_GetOrientationVectors
};

void I3DLocation_init(void *self)
{
    I3DLocation *thiz = (I3DLocation *) self;
    thiz->mItf = &I3DLocation_Itf;
    thiz->mLocationCartesian.x = 0;
    thiz->mLocationCartesian.y = 0;
    thiz->mLocationCartesian.z = 0;
    memset(&thiz->mLocationSpherical, 0x55, sizeof(thiz->mLocationSpherical));
    thiz->mLocationActive = CARTESIAN_SET_SPHERICAL_UNKNOWN;
    thiz->mOrientationAngles.mHeading = 0;
    thiz->mOrientationAngles.mPitch = 0;
    thiz->mOrientationAngles.mRoll = 0;
    memset(&thiz->mOrientationVectors, 0x55, sizeof(thiz->mOrientationVectors));
    thiz->mOrientationActive = ANGLES_SET_VECTORS_UNKNOWN;
    thiz->mTheta = 0x55555555;
    thiz->mAxis.x = 0x55555555;
    thiz->mAxis.y = 0x55555555;
    thiz->mAxis.z = 0x55555555;
    thiz->mRotatePending = SL_BOOLEAN_FALSE;
}
