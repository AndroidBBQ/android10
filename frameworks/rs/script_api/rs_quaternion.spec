#
# Copyright (C) 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

header:
summary: Quaternion Functions
description:
 The following functions manipulate quaternions.
end:

function: rsQuaternionAdd
version: 9 23
ret: void
arg: rs_quaternion* q, "Destination quaternion to add to."
arg: const rs_quaternion* rhs, "Quaternion to add."
summary: Add two quaternions
description:
 Adds two quaternions, i.e. <code>*q += *rhs;</code>
inline:
 q->w += rhs->w;
 q->x += rhs->x;
 q->y += rhs->y;
 q->z += rhs->z;
test: none
end:

function: rsQuaternionConjugate
version: 9 23
ret: void
arg: rs_quaternion* q, "Quaternion to modify."
summary: Conjugate a quaternion
description:
 Conjugates the quaternion.
inline:
 q->x = -q->x;
 q->y = -q->y;
 q->z = -q->z;
test: none
end:

function: rsQuaternionDot
version: 9 23
ret: float
arg: const rs_quaternion* q0, "First quaternion."
arg: const rs_quaternion* q1, "Second quaternion."
summary:  Dot product of two quaternions
description:
 Returns the dot product of two quaternions.
inline:
 return q0->w*q1->w + q0->x*q1->x + q0->y*q1->y + q0->z*q1->z;
test: none
end:

function: rsQuaternionGetMatrixUnit
version: 9 23
ret: void
arg: rs_matrix4x4* m, "Resulting matrix."
arg: const rs_quaternion* q, "Normalized quaternion."
summary: Get a rotation matrix from a quaternion
description:
 Computes a rotation matrix from the normalized quaternion.
inline:
 float xx = q->x * q->x;
 float xy = q->x * q->y;
 float xz = q->x * q->z;
 float xw = q->x * q->w;
 float yy = q->y * q->y;
 float yz = q->y * q->z;
 float yw = q->y * q->w;
 float zz = q->z * q->z;
 float zw = q->z * q->w;

 m->m[0]  = 1.0f - 2.0f * ( yy + zz );
 m->m[4]  =        2.0f * ( xy - zw );
 m->m[8]  =        2.0f * ( xz + yw );
 m->m[1]  =        2.0f * ( xy + zw );
 m->m[5]  = 1.0f - 2.0f * ( xx + zz );
 m->m[9]  =        2.0f * ( yz - xw );
 m->m[2]  =        2.0f * ( xz - yw );
 m->m[6]  =        2.0f * ( yz + xw );
 m->m[10] = 1.0f - 2.0f * ( xx + yy );
 m->m[3]  = m->m[7] = m->m[11] = m->m[12] = m->m[13] = m->m[14] = 0.0f;
 m->m[15] = 1.0f;
test: none
end:

function: rsQuaternionLoadRotateUnit
version: 9 23
ret: void
arg: rs_quaternion* q, "Destination quaternion."
arg: float rot, "Angle to rotate by, in radians."
arg: float x, "X component of the vector."
arg: float y, "Y component of the vector."
arg: float z, "Z component of the vector."
summary:  Quaternion that represents a rotation about an arbitrary unit vector
description:
 Loads a quaternion that represents a rotation about an arbitrary unit vector.
inline:
 rot *= (float)(M_PI / 180.0f) * 0.5f;
 float c = cos(rot);
 float s = sin(rot);

 q->w = c;
 q->x = x * s;
 q->y = y * s;
 q->z = z * s;
test: none
end:

function: rsQuaternionSet
version: 9 23
ret: void
arg: rs_quaternion* q, "Destination quaternion."
arg: float w, "W component."
arg: float x, "X component."
arg: float y, "Y component."
arg: float z, "Z component."
summary: Create a quaternion
description:
 Creates a quaternion from its four components or from another quaternion.
inline:
 q->w = w;
 q->x = x;
 q->y = y;
 q->z = z;
test: none
end:

function: rsQuaternionSet
version: 9 23
ret: void
arg: rs_quaternion* q
arg: const rs_quaternion* rhs, "Source quaternion."
inline:
 q->w = rhs->w;
 q->x = rhs->x;
 q->y = rhs->y;
 q->z = rhs->z;
test: none
end:

# NOTE: The following inline definitions depend on each other.  The order must be preserved
# for the compilation to work.

function: rsQuaternionLoadRotate
version: 9 23
ret: void
arg: rs_quaternion* q, "Destination quaternion."
arg: float rot, "Angle to rotate by."
arg: float x, "X component of a vector."
arg: float y, "Y component of a vector."
arg: float z, "Z component of a vector."
summary: Create a rotation quaternion
description:
 Loads a quaternion that represents a rotation about an arbitrary vector
 (doesn't have to be unit)
inline:
 const float len = x*x + y*y + z*z;
 if (len != 1) {
     const float recipLen = 1.f / sqrt(len);
     x *= recipLen;
     y *= recipLen;
     z *= recipLen;
 }
 rsQuaternionLoadRotateUnit(q, rot, x, y, z);
test: none
end:

function: rsQuaternionNormalize
version: 9 23
ret: void
arg: rs_quaternion* q, "Quaternion to normalize."
summary:  Normalize a quaternion
description:
 Normalizes the quaternion.
inline:
 const float len = rsQuaternionDot(q, q);
 if (len != 1) {
     const float recipLen = 1.f / sqrt(len);
     q->w *= recipLen;
     q->x *= recipLen;
     q->y *= recipLen;
     q->z *= recipLen;
 }
test: none
end:

function: rsQuaternionMultiply
version: 9 23
ret: void
arg: rs_quaternion* q, "Destination quaternion."
arg: float scalar, "Scalar to multiply the quaternion by."
summary:  Multiply a quaternion by a scalar or another quaternion
description:
 Multiplies a quaternion by a scalar or by another quaternion, e.g
 <code>*q = *q * scalar;</code> or <code>*q = *q * *rhs;</code>.
inline:
 q->w *= scalar;
 q->x *= scalar;
 q->y *= scalar;
 q->z *= scalar;
test: none
end:

function: rsQuaternionMultiply
version: 9 23
ret: void
arg: rs_quaternion* q
arg: const rs_quaternion* rhs, "Quaternion to multiply the destination quaternion by."
inline:
 rs_quaternion qtmp;
 rsQuaternionSet(&qtmp, q);

 q->w = qtmp.w*rhs->w - qtmp.x*rhs->x - qtmp.y*rhs->y - qtmp.z*rhs->z;
 q->x = qtmp.w*rhs->x + qtmp.x*rhs->w + qtmp.y*rhs->z - qtmp.z*rhs->y;
 q->y = qtmp.w*rhs->y + qtmp.y*rhs->w + qtmp.z*rhs->x - qtmp.x*rhs->z;
 q->z = qtmp.w*rhs->z + qtmp.z*rhs->w + qtmp.x*rhs->y - qtmp.y*rhs->x;
 rsQuaternionNormalize(q);
test: none
end:

function: rsQuaternionSlerp
version: 9 23
ret: void
arg: rs_quaternion* q, "Result quaternion from the interpolation."
arg: const rs_quaternion* q0, "First input quaternion."
arg: const rs_quaternion* q1, "Second input quaternion."
arg: float t, "How much to interpolate by."
summary: Spherical linear interpolation between two quaternions
description:
 Performs spherical linear interpolation between two quaternions.
inline:
 if (t <= 0.0f) {
     rsQuaternionSet(q, q0);
     return;
 }
 if (t >= 1.0f) {
     rsQuaternionSet(q, q1);
     return;
 }

 rs_quaternion tempq0, tempq1;
 rsQuaternionSet(&tempq0, q0);
 rsQuaternionSet(&tempq1, q1);

 float angle = rsQuaternionDot(q0, q1);
 if (angle < 0) {
     rsQuaternionMultiply(&tempq0, -1.0f);
     angle *= -1.0f;
 }

 float scale, invScale;
 if (angle + 1.0f > 0.05f) {
     if (1.0f - angle >= 0.05f) {
         float theta = acos(angle);
         float invSinTheta = 1.0f / sin(theta);
         scale = sin(theta * (1.0f - t)) * invSinTheta;
         invScale = sin(theta * t) * invSinTheta;
     } else {
         scale = 1.0f - t;
         invScale = t;
     }
 } else {
     rsQuaternionSet(&tempq1, tempq0.z, -tempq0.y, tempq0.x, -tempq0.w);
     scale = sin(M_PI * (0.5f - t));
     invScale = sin(M_PI * t);
 }

 rsQuaternionSet(q, tempq0.w*scale + tempq1.w*invScale, tempq0.x*scale + tempq1.x*invScale,
                     tempq0.y*scale + tempq1.y*invScale, tempq0.z*scale + tempq1.z*invScale);
test: none
end:

# New versions. Same signatures but don't contain a body.
function: rsQuaternionAdd
version: 24
ret: void
arg: rs_quaternion* q
arg: const rs_quaternion* rhs
test: none
end:

function: rsQuaternionConjugate
version: 24
ret: void
arg: rs_quaternion* q
test: none
end:

function: rsQuaternionDot
version: 24
ret: float
arg: const rs_quaternion* q0
arg: const rs_quaternion* q1
test: none
end:

function: rsQuaternionGetMatrixUnit
version: 24
ret: void
arg: rs_matrix4x4* m
arg: const rs_quaternion* q
test: none
end:

function: rsQuaternionLoadRotateUnit
version: 24
ret: void
arg: rs_quaternion* q
arg: float rot
arg: float x
arg: float y
arg: float z
test: none
end:

function: rsQuaternionSet
version: 24
ret: void
arg: rs_quaternion* q
arg: float w
arg: float x
arg: float y
arg: float z
test: none
end:

function: rsQuaternionSet
version: 24
ret: void
arg: rs_quaternion* q
arg: const rs_quaternion* rhs
test: none
end:

# NOTE: The following inline definitions depend on each other.  The order must be preserved
# for the compilation to work.

function: rsQuaternionLoadRotate
version: 24
ret: void
arg: rs_quaternion* q
arg: float rot
arg: float x
arg: float y
arg: float z
test: none
end:

function: rsQuaternionNormalize
version: 24
ret: void
arg: rs_quaternion* q
test: none
end:

function: rsQuaternionMultiply
version: 24
ret: void
arg: rs_quaternion* q
arg: float scalar
test: none
end:

function: rsQuaternionMultiply
version: 24
ret: void
arg: rs_quaternion* q
arg: const rs_quaternion* rhs
test: none
end:

function: rsQuaternionSlerp
version: 24
ret: void
arg: rs_quaternion* q
arg: const rs_quaternion* q0
arg: const rs_quaternion* q1
arg: float t
test: none
end:
