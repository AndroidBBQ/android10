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
summary: Numerical Types
description:
 <h5>Scalars:</h5>

 RenderScript supports the following scalar numerical types:
 <table>
 <tr><td>                 </td>  <td>8 bits        </td>   <td>16 bits         </td>   <td>32 bits       </td>   <td>64 bits</td></tr>
 <tr><td>Integer:         </td>  <td>char, @int8_t  </td>   <td>short, @int16_t  </td>   <td>@int32_t       </td>   <td>long, long long, @int64_t</td></tr>
 <tr><td>Unsigned integer:</td>  <td>uchar, @uint8_t</td>   <td>ushort, @uint16_t</td>   <td>uint, @uint32_t</td>   <td>ulong, @uint64_t</td></tr>
 <tr><td>Floating point:  </td>  <td>              </td>   <td>half            </td>   <td>float         </td>   <td>double</td></tr>
 </table>

 <h5>Vectors:</h5>

 RenderScript supports fixed size vectors of length 2, 3, and 4.
 Vectors are declared using the common type name followed by a 2, 3, or 4.
 E.g. @float4, @int3, @double2, @ulong4.

 To create vector literals, use the vector type followed by the values enclosed
 between curly braces, e.g. <code>(float3){1.0f, 2.0f, 3.0f}</code>.

 Entries of a vector can be accessed using different naming styles.

 Single entries can be accessed by following the variable name with a dot and:<ul>
 <li>The letters x, y, z, and w,</li>
 <li>The letters r, g, b, and a,</li>
 <li>The letter s or S, followed by a zero based index.</li></ul>

 For example, with <code>int4 myVar;</code> the following are equivalent:<code><br/>
   myVar.x == myVar.r == myVar.s0 == myVar.S0<br/>
   myVar.y == myVar.g == myVar.s1 == myVar.S1<br/>
   myVar.z == myVar.b == myVar.s2 == myVar.S2<br/>
   myVar.w == myVar.a == myVar.s3 == myVar.S3</code>

 Multiple entries of a vector can be accessed at once by using an identifier that is
 the concatenation of multiple letters or indices.  The resulting vector has a size
 equal to the number of entries named.

 With the example above, the middle two entries can be accessed using
 <code>myVar.yz</code>, <code>myVar.gb</code>, <code>myVar.s12</code>, and <code>myVar.S12</code>.

 The entries don't have to be contiguous or in increasing order.  Entries can even be
 repeated, as long as we're not trying to assign to it.  You also can't mix the naming
 styles.

 Here are examples of what can or can't be done:<code><br/>
 float4 v4;<br/>
 float3 v3;<br/>
 float2 v2;<br/>
 v2 = v4.xx; // Valid<br/>
 v3 = v4.zxw; // Valid<br/>
 v3 = v4.bba; // Valid<br/>
 v3 = v4.s032; // Valid<br/>
 v3.s120 = v4.S233; // Valid<br/>
 v4.yz = v3.rg; // Valid<br/>
 v4.yzx = v3.rg; // Invalid: mismatched sizes<br/>
 v4.yzz = v3; // Invalid: z appears twice in an assignment<br/>
 v3 = v3.xas0; // Invalid: can't mix xyzw with rgba nor s0...<br/>
 v3 = v4.s034; // Invalid: the digit can only be 0, 1, 2, or 3<br/>
 </code>

 <h5>Matrices and Quaternions:</h5>

 RenderScript supports fixed size square matrices of floats of size 2x2, 3x3, and 4x4.
 The types are named @rs_matrix2x2, @rs_matrix3x3, and @rs_matrix4x4.  See
 <a href='rs_matrix.html'>Matrix Functions</a> for the list of operations.

 Quaternions are also supported via @rs_quaternion.  See <a href='rs_quaternion.html'>Quaterion Functions</a> for the list
 of operations.
end:

type: half
version: 23
simple: __fp16
summary: 16 bit floating point value
description:
 A 16 bit floating point value.
end:

type: half2
version: 23
simple: half
attrib: ext_vector_type(2)
summary: Two 16 bit floats
description:
 Vector version of the half float type. Provides two half fields packed
 into a single 32 bit field with 32 bit alignment.
end:

type: half3
version: 23
simple: half
attrib: ext_vector_type(3)
summary: Three 16 bit floats
description:
 Vector version of the half float type. Provides three half fields packed
 into a single 64 bit field with 64 bit alignment.
end:

type: half4
version: 23
simple: half
attrib: ext_vector_type(4)
summary: Four 16 bit floats
description:
 Vector version of the half float type. Provides four half fields packed
 into a single 64 bit field with 64 bit alignment.
end:


type: int8_t
simple: char
summary: 8 bit signed integer
description:
 8 bit signed integer type.
end:

type: int16_t
simple: short
summary: 16 bit signed integer
description:
 A 16 bit signed integer type.
end:

type: int32_t
simple: int
summary: 32 bit signed integer
description:
 A 32 bit signed integer type.
end:

type: int64_t
version: 9 20
simple: long long
summary: 64 bit signed integer
description:
 A 64 bit signed integer type.
end:

type: int64_t
version: 21
simple: long
end:

type: uint8_t
simple: unsigned char
summary: 8 bit unsigned integer
description:
 8 bit unsigned integer type.
end:

type: uint16_t
simple: unsigned short
summary: 16 bit unsigned integer
description:
 A 16 bit unsigned integer type.
end:

type: uint32_t
simple: unsigned int
summary: 32 bit unsigned integer
description:
 A 32 bit unsigned integer type.
end:

type: uint64_t
version: 9 20
simple: unsigned long long
summary: 64 bit unsigned integer
description:
 A 64 bit unsigned integer type.
end:

type: uint64_t
version: 21
simple: unsigned long
end:

type: uchar
simple: uint8_t
summary: 8 bit unsigned integer
description:
 8 bit unsigned integer type.
end:

type: ushort
simple: uint16_t
summary: 16 bit unsigned integer
description:
 A 16 bit unsigned integer type.
end:

type: uint
simple: uint32_t
summary: 32 bit unsigned integer
description:
 A 32 bit unsigned integer type.
end:

type: ulong
simple: uint64_t
summary: 64 bit unsigned integer
description:
 A 64 bit unsigned integer type.
end:

type: size_t
size: 64
simple: uint64_t
summary: Unsigned size type
description:
 Unsigned size type.  The number of bits depend on the compilation flags.
end:

type: size_t
size: 32
simple: uint32_t
end:

type: ssize_t
size: 64
simple: int64_t
summary: Signed size type
description:
 Signed size type.  The number of bits depend on the compilation flags.
end:

type: ssize_t
size: 32
simple: int32_t
end:

type: float2
simple: float
attrib: ext_vector_type(2)
summary: Two 32 bit floats
description:
 A vector of two floats.  These two floats are packed into a single 64 bit field
 with a 64 bit alignment.

 A vector of two floats.  These two floats are packed into a single 64 bit field
 with a 64 bit alignment.
end:

type: float3
simple: float
attrib: ext_vector_type(3)
summary: Three 32 bit floats
description:
 A vector of three floats.  These three floats are packed into a single 128 bit field
 with a 128 bit alignment.
end:

type: float4
simple: float
attrib: ext_vector_type(4)
summary: Four 32 bit floats
description:
 A vector of four floats type.  These four floats are packed into a single 128 bit field
 with a 128 bit alignment.
end:


type: double2
simple: double
attrib: ext_vector_type(2)
summary: Two 64 bit floats
description:
 A vector of two doubles.  These two double fields packed into a single 128 bit field
 with a 128 bit alignment.
end:

type: double3
simple: double
attrib: ext_vector_type(3)
summary: Three 64 bit floats
description:
 A vector of three doubles.  These three double fields packed into a single 256 bit field
 with a 256 bit alignment.
end:

type: double4
simple: double
attrib: ext_vector_type(4)
summary: Four 64 bit floats
description:
 A vector of four doubles.  These four double fields packed into a single 256 bit field
 with a 256 bit alignment.
end:


type: uchar2
simple: uchar
attrib: ext_vector_type(2)
summary: Two 8 bit unsigned integers
description:
 A vector of two uchars.  These two uchar fields packed into a single 16 bit field
 with a 16 bit alignment.
end:

type: uchar3
simple: uchar
attrib: ext_vector_type(3)
summary: Three 8 bit unsigned integers
description:
 A vector of three uchars.  These three uchar fields packed into a single 32 bit field
 with a 32 bit alignment.
end:

type: uchar4
simple: uchar
attrib: ext_vector_type(4)
summary: Four 8 bit unsigned integers
description:
 A vector of four uchars.  These four uchar fields packed into a single 32 bit field
 with a 32 bit alignment.
end:


type: ushort2
simple: ushort
attrib: ext_vector_type(2)
summary: Two 16 bit unsigned integers
description:
 A vector of two ushorts.  These two ushort fields packed into a single 32 bit field
 with a 32 bit alignment.
end:

type: ushort3
simple: ushort
attrib: ext_vector_type(3)
summary: Three 16 bit unsigned integers
description:
 A vector of three ushorts.  These three ushort fields packed into a single 64 bit field
 with a 64 bit alignment.
end:

type: ushort4
simple: ushort
attrib: ext_vector_type(4)
summary: Four 16 bit unsigned integers
description:
 A vector of four ushorts.  These four ushort fields packed into a single 64 bit field
 with a 64 bit alignment.
end:


type: uint2
simple: uint
attrib: ext_vector_type(2)
summary: Two 32 bit unsigned integers
description:
 A vector of two uints.  These two uints are packed into a single 64 bit field
 with a 64 bit alignment.
end:

type: uint3
simple: uint
attrib: ext_vector_type(3)
summary: Three 32 bit unsigned integers
description:
 A vector of three uints.  These three uints are packed into a single 128 bit field
 with a 128 bit alignment.
end:

type: uint4
simple: uint
attrib: ext_vector_type(4)
summary: Four 32 bit unsigned integers
description:
 A vector of four uints.  These four uints are packed into a single 128 bit field
 with a 128 bit alignment.
end:


type: ulong2
simple: ulong
attrib: ext_vector_type(2)
summary: Two 64 bit unsigned integers
description:
 A vector of two ulongs.  These two ulongs are packed into a single 128 bit field
 with a 128 bit alignment.
end:

type: ulong3
simple: ulong
attrib: ext_vector_type(3)
summary: Three 64 bit unsigned integers
description:
 A vector of three ulongs.  These three ulong fields packed into a single 256 bit field
 with a 256 bit alignment.
end:

type: ulong4
simple: ulong
attrib: ext_vector_type(4)
summary: Four 64 bit unsigned integers
description:
 A vector of four ulongs.  These four ulong fields packed into a single 256 bit field
 with a 256 bit alignment.
end:


type: char2
simple: char
attrib: ext_vector_type(2)
summary: Two 8 bit signed integers
description:
 A vector of two chars.  These two chars are packed into a single 16 bit field
 with a 16 bit alignment.
end:

type: char3
simple: char
attrib: ext_vector_type(3)
summary: Three 8 bit signed integers
description:
 A vector of three chars.  These three chars are packed into a single 32 bit field
 with a 32 bit alignment.
end:

type: char4
simple: char
attrib: ext_vector_type(4)
summary: Four 8 bit signed integers
description:
 A vector of four chars.  These four chars are packed into a single 32 bit field
 with a 32 bit alignment.
end:


type: short2
simple: short
attrib: ext_vector_type(2)
summary: Two 16 bit signed integers
description:
 A vector of two shorts.  These two shorts are packed into a single 32 bit field
 with a 32 bit alignment.
end:

type: short3
simple: short
attrib: ext_vector_type(3)
summary: Three 16 bit signed integers
description:
 A vector of three shorts.  These three short fields packed into a single 64 bit field
 with a 64 bit alignment.
end:

type: short4
simple: short
attrib: ext_vector_type(4)
summary: Four 16 bit signed integers
description:
 A vector of four shorts.  These four short fields packed into a single 64 bit field
 with a 64 bit alignment.
end:


type: int2
simple: int
attrib: ext_vector_type(2)
summary: Two 32 bit signed integers
description:
 A vector of two ints.  These two ints are packed into a single 64 bit field
 with a 64 bit alignment.
end:

type: int3
simple: int
attrib: ext_vector_type(3)
summary: Three 32 bit signed integers
description:
 A vector of three ints.  These three ints are packed into a single 128 bit field
 with a 128 bit alignment.
end:

type: int4
simple: int
attrib: ext_vector_type(4)
summary: Four 32 bit signed integers
description:
 A vector of four ints.  These two fours are packed into a single 128 bit field
 with a 128 bit alignment.
end:


type: long2
simple: long
attrib: ext_vector_type(2)
summary: Two 64 bit signed integers
description:
 A vector of two longs.  These two longs are packed into a single 128 bit field
 with a 128 bit alignment.
end:

type: long3
simple: long
attrib: ext_vector_type(3)
summary: Three 64 bit signed integers
description:
 A vector of three longs.  These three longs are packed into a single 256 bit field
 with a 256 bit alignment.
end:

type: long4
simple: long
attrib: ext_vector_type(4)
summary: Four 64 bit signed integers
description:
 A vector of four longs.  These four longs are packed into a single 256 bit field
 with a 256 bit alignment.
end:


type: rs_matrix2x2
struct:
field: float m[4]
summary: 2x2 matrix of 32 bit floats
description:
 A square 2x2 matrix of floats.  The entries are stored in the array at the
 location [row*2 + col].

 See <a href='rs_matrix.html'>Matrix Functions</a>.
end:

type: rs_matrix3x3
struct:
field: float m[9]
summary: 3x3 matrix of 32 bit floats
description:
 A square 3x3 matrix of floats.  The entries are stored in the array at the
 location [row*3 + col].

 See <a href='rs_matrix.html'>Matrix Functions</a>.
end:

type: rs_matrix4x4
struct:
field: float m[16]
summary: 4x4 matrix of 32 bit floats
description:
 A square 4x4 matrix of floats.  The entries are stored in the array at the
 location [row*4 + col].

 See <a href='rs_matrix.html'>Matrix Functions</a>.
end:


type: rs_quaternion
simple: float4
summary: Quaternion
description:
 A square 4x4 matrix of floats that represents a quaternion.

 See <a href='rs_quaternion.html'>Quaternion Functions</a>.
end:
