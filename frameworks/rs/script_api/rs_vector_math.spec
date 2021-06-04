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
summary: Vector Math Functions
description:
 These functions interpret the input arguments as representation of vectors in
 n-dimensional space.

 The precision of the mathematical operations on 32 bit floats is affected by the pragmas
# TODO Create an anchor for the section of http://developer.android.com/guide/topics/renderscript/compute.html that details rs_fp_* and link them here.
 rs_fp_relaxed and rs_fp_full.  See <a href='rs_math.html'>Mathematical Constants and Functions</a> for details.

 Different precision/speed tradeoffs can be achieved by using variants of the common math
 functions.  Functions with a name starting with<ul>
 <li>native_: May have custom hardware implementations with weaker precision.  Additionally,
   subnormal values may be flushed to zero, rounding towards zero may be used, and NaN and
   infinity input may not be handled correctly.</li>
 <li>fast_: May perform internal computations using 16 bit floats.  Additionally, subnormal
   values may be flushed to zero, and rounding towards zero may be used.</li>
 </ul>
end:

function: cross
version: 9
attrib: const
w: 3, 4
t: f32
ret: #2#1
arg: #2#1 left_vector
arg: #2#1 right_vector
summary: Cross product of two vectors
description:
 Computes the cross product of two vectors.
test: vector
end:

function: cross
version: 24
attrib: const
w: 3, 4
t: f16
ret: #2#1
arg: #2#1 left_vector
arg: #2#1 right_vector
test: vector
end:

function: distance
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2
arg: #2#1 left_vector
arg: #2#1 right_vector
summary: Distance between two points
description:
 Compute the distance between two points.

 See also @fast_distance(), @native_distance().
test: vector
end:

function: distance
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2
arg: #2#1 left_vector
arg: #2#1 right_vector
test: vector
end:

function: dot
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2
arg: #2#1 left_vector
arg: #2#1 right_vector
summary: Dot product of two vectors
description:
 Computes the dot product of two vectors.
test: vector
end:

function: dot
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2
arg: #2#1 left_vector
arg: #2#1 right_vector
test: vector
end:

function: fast_distance
version: 17
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2
arg: #2#1 left_vector
arg: #2#1 right_vector
summary: Approximate distance between two points
description:
 Computes the approximate distance between two points.

 The precision is what would be expected from doing the computation using 16 bit floating
 point values.

 See also @distance(), @native_distance().
test: vector
end:

function: fast_length
version: 17
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2
arg: #2#1 v
summary: Approximate length of a vector
description:
 Computes the approximate length of a vector.

 The precision is what would be expected from doing the computation using 16 bit floating
 point values.

 See also @length(), @native_length().
test: vector
end:

function: fast_normalize
version: 17
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate normalized vector
description:
 Approximately normalizes a vector.

 For vectors of size 1, returns -1.f for negative values, 0.f for null values, and 1.f for
 positive values.

 The precision is what would be expected from doing the computation using 16 bit floating
 point values.

 See also @normalize(), @native_normalize().
test: vector
end:

function: length
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2
arg: #2#1 v
summary: Length of a vector
description:
 Computes the length of a vector.

 See also @fast_length(), @native_length().
test: vector
end:

function: length
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2
arg: #2#1 v
test: vector
end:

function: native_distance
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2
arg: #2#1 left_vector
arg: #2#1 right_vector
summary: Approximate distance between two points
description:
 Computes the approximate distance between two points.

 See also @distance(), @fast_distance().
test: vector
end:

function: native_distance
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2
arg: #2#1 left_vector
arg: #2#1 right_vector
test: vector
end:

function: native_length
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2
arg: #2#1 v
summary: Approximate length of a vector
description:
 Compute the approximate length of a vector.

 See also @length(), @fast_length().
test: vector
end:

function: native_length
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2
arg: #2#1 v
test: vector
end:

function: native_normalize
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary:  Approximately normalize a vector
description:
 Approximately normalizes a vector.

 See also @normalize(), @fast_normalize().
test: vector
end:

function: native_normalize
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
test: vector
end:

function: normalize
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Normalize a vector
description:
 Normalize a vector.

 For vectors of size 1, returns -1.f for negative values, 0.f for null values, and 1.f for
 positive values.

 See also @fast_normalize(), @native_normalize().
test: vector
end:

function: normalize
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
test: vector
end:
