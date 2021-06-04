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
summary: Matrix Functions
description:
 These functions let you manipulate square matrices of rank 2x2, 3x3, and 4x4.
 They are particularly useful for graphical transformations and are compatible
 with OpenGL.

 We use a zero-based index for rows and columns.  E.g. the last element of a
 @rs_matrix4x4 is found at (3, 3).

 RenderScript uses column-major matrices and column-based vectors.  Transforming
 a vector is done by postmultiplying the vector, e.g. <code>(matrix * vector)</code>,
 as provided by @rsMatrixMultiply().

 To create a transformation matrix that performs two transformations at once,
 multiply the two source matrices, with the first transformation as the right
 argument.  E.g. to create a transformation matrix that applies the
 transformation s1 followed by s2, call <code>rsMatrixLoadMultiply(&amp;combined, &amp;s2, &amp;s1)</code>.
 This derives from <code>s2 * (s1 * v)</code>, which is <code>(s2 * s1) * v</code>.

 We have two style of functions to create transformation matrices:
 rsMatrixLoad<i>Transformation</i> and rsMatrix<i>Transformation</i>.  The former
 style simply stores the transformation matrix in the first argument.  The latter
 modifies a pre-existing transformation matrix so that the new transformation
 happens first.  E.g. if you call @rsMatrixTranslate() on a matrix that already
 does a scaling, the resulting matrix when applied to a vector will first do the
 translation then the scaling.
include:
 #include "rs_vector_math.rsh"
end:

function: rsExtractFrustumPlanes
version: 9 23
ret: void
arg: const rs_matrix4x4* viewProj, "Matrix to extract planes from."
arg: float4* left, "Left plane."
arg: float4* right, "Right plane."
arg: float4* top, "Top plane."
arg: float4* bottom, "Bottom plane."
arg: float4* near, "Near plane."
arg: float4* far, "Far plane."
summary: Compute frustum planes
description:
 Computes 6 frustum planes from the view projection matrix
inline:
 // x y z w = a b c d in the plane equation
 left->x = viewProj->m[3] + viewProj->m[0];
 left->y = viewProj->m[7] + viewProj->m[4];
 left->z = viewProj->m[11] + viewProj->m[8];
 left->w = viewProj->m[15] + viewProj->m[12];

 right->x = viewProj->m[3] - viewProj->m[0];
 right->y = viewProj->m[7] - viewProj->m[4];
 right->z = viewProj->m[11] - viewProj->m[8];
 right->w = viewProj->m[15] - viewProj->m[12];

 top->x = viewProj->m[3] - viewProj->m[1];
 top->y = viewProj->m[7] - viewProj->m[5];
 top->z = viewProj->m[11] - viewProj->m[9];
 top->w = viewProj->m[15] - viewProj->m[13];

 bottom->x = viewProj->m[3] + viewProj->m[1];
 bottom->y = viewProj->m[7] + viewProj->m[5];
 bottom->z = viewProj->m[11] + viewProj->m[9];
 bottom->w = viewProj->m[15] + viewProj->m[13];

 near->x = viewProj->m[3] + viewProj->m[2];
 near->y = viewProj->m[7] + viewProj->m[6];
 near->z = viewProj->m[11] + viewProj->m[10];
 near->w = viewProj->m[15] + viewProj->m[14];

 far->x = viewProj->m[3] - viewProj->m[2];
 far->y = viewProj->m[7] - viewProj->m[6];
 far->z = viewProj->m[11] - viewProj->m[10];
 far->w = viewProj->m[15] - viewProj->m[14];

 float len = length(left->xyz);
 *left /= len;
 len = length(right->xyz);
 *right /= len;
 len = length(top->xyz);
 *top /= len;
 len = length(bottom->xyz);
 *bottom /= len;
 len = length(near->xyz);
 *near /= len;
 len = length(far->xyz);
 *far /= len;
test: none
end:

# New version. Same signature but doesn't contain a body.
function: rsExtractFrustumPlanes
version: 24
ret: void
arg: const rs_matrix4x4* viewProj
arg: float4* left
arg: float4* righ
arg: float4* top
arg: float4* bottom
arg: float4* near
arg: float4* far
test: none
end:

function: rsIsSphereInFrustum
version: 9 23
attrib: always_inline
ret: bool
arg: float4* sphere, "float4 representing the sphere."
arg: float4* left, "Left plane."
arg: float4* right, "Right plane."
arg: float4* top, "Top plane."
arg: float4* bottom, "Bottom plane."
arg: float4* near, "Near plane."
arg: float4* far, "Far plane."
summary: Checks if a sphere is within the frustum planes
description:
 Returns true if the sphere is within the 6 frustum planes.
inline:
 float distToCenter = dot(left->xyz, sphere->xyz) + left->w;
 if (distToCenter < -sphere->w) {
     return false;
 }
 distToCenter = dot(right->xyz, sphere->xyz) + right->w;
 if (distToCenter < -sphere->w) {
     return false;
 }
 distToCenter = dot(top->xyz, sphere->xyz) + top->w;
 if (distToCenter < -sphere->w) {
     return false;
 }
 distToCenter = dot(bottom->xyz, sphere->xyz) + bottom->w;
 if (distToCenter < -sphere->w) {
     return false;
 }
 distToCenter = dot(near->xyz, sphere->xyz) + near->w;
 if (distToCenter < -sphere->w) {
     return false;
 }
 distToCenter = dot(far->xyz, sphere->xyz) + far->w;
 if (distToCenter < -sphere->w) {
     return false;
 }
 return true;
test: none
end:

# New version. Same signature but doesn't contain a body.
function: rsIsSphereInFrustum
version: 24
ret: bool
arg: float4* sphere
arg: float4* left
arg: float4* right
arg: float4* top
arg: float4* bottom
arg: float4* near
arg: float4* far
test: none
end:

function: rsMatrixGet
t: rs_matrix4x4, rs_matrix3x3, rs_matrix2x2
ret: float
arg: const #1* m, "Matrix to extract the element from."
arg: uint32_t col, "Zero-based column of the element to be extracted."
arg: uint32_t row, "Zero-based row of the element to extracted."
summary: Get one element
description:
 Returns one element of a matrix.

 <b>Warning:</b> The order of the column and row parameters may be unexpected.
test: none
end:

function: rsMatrixInverse
ret: bool
arg: rs_matrix4x4* m, "Matrix to invert."
summary: Inverts a matrix in place
description:
 Returns true if the matrix was successfully inverted.
test: none
end:

function: rsMatrixInverseTranspose
ret: bool
arg: rs_matrix4x4* m, "Matrix to modify."
summary: Inverts and transpose a matrix in place
description:
 The matrix is first inverted then transposed. Returns true if the matrix was
 successfully inverted.
test: none
end:

function: rsMatrixLoad
t: rs_matrix4x4, rs_matrix3x3, rs_matrix2x2
ret: void
arg: #1* destination, "Matrix to set."
arg: const float* array, "Array of values to set the matrix to. These arrays should be 4, 9, or 16 floats long, depending on the matrix size."
summary: Load or copy a matrix
description:
 Set the elements of a matrix from an array of floats or from another matrix.

 If loading from an array, the floats should be in row-major order, i.e. the element a
 <code>row 0, column 0</code> should be first, followed by the element at
 <code>row 0, column 1</code>, etc.

 If loading from a matrix and the source is smaller than the destination, the rest
 of the destination is filled with elements of the identity matrix.  E.g.
 loading a rs_matrix2x2 into a rs_matrix4x4 will give:
 <table style="max-width:300px">
 <tr><td>m00</td> <td>m01</td> <td>0.0</td> <td>0.0</td></tr>
 <tr><td>m10</td> <td>m11</td> <td>0.0</td> <td>0.0</td></tr>
 <tr><td>0.0</td> <td>0.0</td> <td>1.0</td> <td>0.0</td></tr>
 <tr><td>0.0</td> <td>0.0</td> <td>0.0</td> <td>1.0</td></tr>
 </table>
test: none
end:

function: rsMatrixLoad
t: rs_matrix4x4, rs_matrix3x3, rs_matrix2x2
ret: void
arg: #1* destination
arg: const #1* source, "Source matrix."
test: none
end:

function: rsMatrixLoad
t: rs_matrix3x3, rs_matrix2x2
ret: void
arg: rs_matrix4x4* destination
arg: const #1* source
test: none
end:

function: rsMatrixLoadFrustum
ret: void
arg: rs_matrix4x4* m, "Matrix to set."
arg: float left
arg: float right
arg: float bottom
arg: float top
arg: float near
arg: float far
summary: Load a frustum projection matrix
description:
 Constructs a frustum projection matrix, transforming the box identified by
 the six clipping planes <code>left, right, bottom, top, near, far</code>.

 To apply this projection to a vector, multiply the vector by the created
 matrix using @rsMatrixMultiply().
test: none
end:

function: rsMatrixLoadIdentity
t: rs_matrix4x4, rs_matrix3x3, rs_matrix2x2
ret: void
arg: #1* m, "Matrix to set."
summary: Load identity matrix
description:
 Set the elements of a matrix to the identity matrix.
test: none
end:

function: rsMatrixLoadMultiply
t: rs_matrix4x4, rs_matrix3x3, rs_matrix2x2
ret: void
arg: #1* m, "Matrix to set."
arg: const #1* lhs, "Left matrix of the product."
arg: const #1* rhs, "Right matrix of the product."
summary: Multiply two matrices
description:
 Sets m to the matrix product of <code>lhs * rhs</code>.

 To combine two 4x4 transformaton matrices, multiply the second transformation matrix
 by the first transformation matrix.  E.g. to create a transformation matrix that applies
 the transformation s1 followed by s2, call <code>rsMatrixLoadMultiply(&amp;combined, &amp;s2, &amp;s1)</code>.

 <b>Warning:</b> Prior to version 21, storing the result back into right matrix is not supported and
 will result in undefined behavior.  Use rsMatrixMulitply instead.   E.g. instead of doing
 rsMatrixLoadMultiply (&amp;m2r, &amp;m2r, &amp;m2l), use rsMatrixMultiply (&amp;m2r, &amp;m2l).
 rsMatrixLoadMultiply (&amp;m2l, &amp;m2r, &amp;m2l) works as expected.
test: none
end:

function: rsMatrixLoadOrtho
ret: void
arg: rs_matrix4x4* m, "Matrix to set."
arg: float left
arg: float right
arg: float bottom
arg: float top
arg: float near
arg: float far
summary: Load an orthographic projection matrix
description:
 Constructs an orthographic projection matrix, transforming the box identified by the
 six clipping planes <code>left, right, bottom, top, near, far</code> into a unit cube
 with a corner at <code>(-1, -1, -1)</code> and the opposite at <code>(1, 1, 1)</code>.

 To apply this projection to a vector, multiply the vector by the created matrix
 using @rsMatrixMultiply().

 See https://en.wikipedia.org/wiki/Orthographic_projection .
test: none
end:

function: rsMatrixLoadPerspective
ret: void
arg: rs_matrix4x4* m, "Matrix to set."
arg: float fovy, "Field of view, in degrees along the Y axis."
arg: float aspect, "Ratio of x / y."
arg: float near, "Near clipping plane."
arg: float far, "Far clipping plane."
summary: Load a perspective projection matrix
description:
 Constructs a perspective projection matrix, assuming a symmetrical field of view.

 To apply this projection to a vector, multiply the vector by the created matrix
 using @rsMatrixMultiply().
test: none
end:

function: rsMatrixLoadRotate
ret: void
arg: rs_matrix4x4* m, "Matrix to set."
arg: float rot, "How much rotation to do, in degrees."
arg: float x, "X component of the vector that is the axis of rotation."
arg: float y, "Y component of the vector that is the axis of rotation."
arg: float z, "Z component of the vector that is the axis of rotation."
summary: Load a rotation matrix
description:
 This function creates a rotation matrix.  The axis of rotation is the <code>(x, y, z)</code> vector.

 To rotate a vector, multiply the vector by the created matrix using @rsMatrixMultiply().

 See http://en.wikipedia.org/wiki/Rotation_matrix .
test: none
end:

function: rsMatrixLoadScale
ret: void
arg: rs_matrix4x4* m, "Matrix to set."
arg: float x, "Multiple to scale the x components by."
arg: float y, "Multiple to scale the y components by."
arg: float z, "Multiple to scale the z components by."
summary: Load a scaling matrix
description:
 This function creates a scaling matrix, where each component of a vector is multiplied
 by a number.  This number can be negative.

 To scale a vector, multiply the vector by the created matrix using @rsMatrixMultiply().
test: none
end:

function: rsMatrixLoadTranslate
ret: void
arg: rs_matrix4x4* m, "Matrix to set."
arg: float x, "Number to add to each x component."
arg: float y, "Number to add to each y component."
arg: float z, "Number to add to each z component."
summary: Load a translation matrix
description:
 This function creates a translation matrix, where a number is added to each element of
 a vector.

 To translate a vector, multiply the vector by the created matrix using
 @rsMatrixMultiply().
test: none
end:

function: rsMatrixMultiply
t: rs_matrix4x4, rs_matrix3x3, rs_matrix2x2
ret: void
arg: #1* m, "Left matrix of the product and the matrix to be set."
arg: const #1* rhs, "Right matrix of the product."
summary: Multiply a matrix by a vector or another matrix
description:
 For the matrix by matrix variant, sets m to the matrix product <code>m * rhs</code>.

 When combining two 4x4 transformation matrices using this function, the resulting
 matrix will correspond to performing the rhs transformation first followed by
 the original m transformation.

 For the matrix by vector variant, returns the post-multiplication of the vector
 by the matrix, ie. <code>m * in</code>.

 When multiplying a float3 to a @rs_matrix4x4, the vector is expanded with (1).

 When multiplying a float2 to a @rs_matrix4x4, the vector is expanded with (0, 1).

 When multiplying a float2 to a @rs_matrix3x3, the vector is expanded with (0).

 Starting with API 14, this function takes a const matrix as the first argument.
test: none
end:

function: rsMatrixMultiply
version: 9 13
ret: float4
arg: rs_matrix4x4* m
arg: float4 in
test: none
end:

function: rsMatrixMultiply
version: 9 13
ret: float4
arg: rs_matrix4x4* m
arg: float3 in
test: none
end:

function: rsMatrixMultiply
version: 9 13
ret: float4
arg: rs_matrix4x4* m
arg: float2 in
test: none
end:

function: rsMatrixMultiply
version: 9 13
ret: float3
arg: rs_matrix3x3* m
arg: float3 in
test: none
end:

function: rsMatrixMultiply
version: 9 13
ret: float3
arg: rs_matrix3x3* m
arg: float2 in
test: none
end:

function: rsMatrixMultiply
version: 9 13
ret: float2
arg: rs_matrix2x2* m
arg: float2 in
test: none
end:

function: rsMatrixMultiply
version: 14
ret: float4
arg: const rs_matrix4x4* m
arg: float4 in
test: none
end:

function: rsMatrixMultiply
version: 14
ret: float4
arg: const rs_matrix4x4* m
arg: float3 in
test: none
end:

function: rsMatrixMultiply
version: 14
ret: float4
arg: const rs_matrix4x4* m
arg: float2 in
test: none
end:

function: rsMatrixMultiply
version: 14
ret: float3
arg: const rs_matrix3x3* m
arg: float3 in
test: none
end:

function: rsMatrixMultiply
version: 14
ret: float3
arg: const rs_matrix3x3* m
arg: float2 in
test: none
end:

function: rsMatrixMultiply
version: 14
ret: float2
arg: const rs_matrix2x2* m
arg: float2 in
test: none
end:

function: rsMatrixRotate
ret: void
arg: rs_matrix4x4* m, "Matrix to modify."
arg: float rot, "How much rotation to do, in degrees."
arg: float x, "X component of the vector that is the axis of rotation."
arg: float y, "Y component of the vector that is the axis of rotation."
arg: float z, "Z component of the vector that is the axis of rotation."
summary: Apply a rotation to a transformation matrix
description:
 Multiply the matrix m with a rotation matrix.

 This function modifies a transformation matrix to first do a rotation.  The axis of
 rotation is the <code>(x, y, z)</code> vector.

 To apply this combined transformation to a vector, multiply the vector by the created
 matrix using @rsMatrixMultiply().
test: none
end:

function: rsMatrixScale
ret: void
arg: rs_matrix4x4* m, "Matrix to modify."
arg: float x, "Multiple to scale the x components by."
arg: float y, "Multiple to scale the y components by."
arg: float z, "Multiple to scale the z components by."
summary: Apply a scaling to a transformation matrix
description:
 Multiply the matrix m with a scaling matrix.

 This function modifies a transformation matrix to first do a scaling.   When scaling,
 each component of a vector is multiplied by a number.  This number can be negative.

 To apply this combined transformation to a vector, multiply the vector by the created
 matrix using @rsMatrixMultiply().
test: none
end:

function: rsMatrixSet
t: rs_matrix4x4, rs_matrix3x3, rs_matrix2x2
ret: void
arg: #1* m, "Matrix that will be modified."
arg: uint32_t col, "Zero-based column of the element to be set."
arg: uint32_t row, "Zero-based row of the element to be set."
arg: float v, "Value to set."
summary: Set one element
description:
 Set an element of a matrix.

 <b>Warning:</b> The order of the column and row parameters may be unexpected.
test: none
end:

function: rsMatrixTranslate
ret: void
arg: rs_matrix4x4* m, "Matrix to modify."
arg: float x, "Number to add to each x component."
arg: float y, "Number to add to each y component."
arg: float z, "Number to add to each z component."
summary: Apply a translation to a transformation matrix
description:
 Multiply the matrix m with a translation matrix.

 This function modifies a transformation matrix to first do a translation.  When
 translating, a number is added to each component of a vector.

 To apply this combined transformation to a vector, multiply the vector by the
 created matrix using @rsMatrixMultiply().
test: none
end:

function: rsMatrixTranspose
t: rs_matrix4x4*, rs_matrix3x3*, rs_matrix2x2*
ret: void
arg: #1 m, "Matrix to transpose."
summary: Transpose a matrix place
description:
 Transpose the matrix m in place.
test: none
end:
