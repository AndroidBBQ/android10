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
summary: Conversion Functions
description:
 The functions below convert from a numerical vector type to another, or from one color
 representation to another.
end:

function: convert_#3#1
version: 9
attrib: const
w: 2, 3, 4
t: u8, u16, u32, i8, i16, i32, f32
t: u8, u16, u32, i8, i16, i32, f32
ret: #3#1
arg: #2#1 v, compatible(#3)
summary: Convert numerical vectors
description:
 Converts a vector from one numerical type to another.  The conversion are done entry per entry.

 E.g calling <code>a = convert_short3(b);</code> is equivalent to doing
 <code>a.x = (short)b.x; a.y = (short)b.y; a.z = (short)b.z;</code>.

 Converting floating point values to integer types truncates.

 Converting numbers too large to fit the destination type yields undefined results.
 For example, converting a float that contains 1.0e18 to a short is undefined.
 Use @clamp() to avoid this.
end:

function: convert_#3#1
version: 21
attrib: const
w: 2, 3, 4
t: u64, i64, f64
t: u64, i64, f64
ret: #3#1
arg: #2#1 v, compatible(#3)
end:

function: convert_#3#1
version: 21
attrib: const
w: 2, 3, 4
t: u64, i64, f64
t: u8, u16, u32, i8, i16, i32, f32
ret: #3#1
arg: #2#1 v, compatible(#3)
end:

function: convert_#3#1
version: 21
attrib: const
w: 2, 3, 4
t: u8, u16, u32, i8, i16, i32, f32
t: u64, i64, f64
ret: #3#1
arg: #2#1 v, compatible(#3)
end:

function: convert_#3#1
version: 24
attrib: const
w: 2, 3, 4
t: f16
t: u8, u16, u32, u64, i8, i16, i32, i64, f16, f32, f64
ret: #3#1
arg: #2#1 v, compatible(#3)
end:

function: convert_#3#1
version: 24
attrib: const
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
t: f16
ret: #3#1
arg: #2#1 v, compatible(#3)
end:

function: rsPackColorTo8888
attrib: const
ret: uchar4
arg: float r, "Red component."
arg: float g, "Green component."
arg: float b, "Blue component."
summary: Create a uchar4 RGBA from floats
description:
 Packs three or four floating point RGBA values into a uchar4.

 The input values are typically between 0.0f and 1.0f inclusive.  For input values outside
 of this range, the resulting outputs will be clamped to be between 0 and 255.  As this
 clamping may be done after the input is multiplied by 255.f and converted to an integer,
 input numbers greater than INT_MAX/255.f or less than INT_MIN/255.f result in
 undefined behavior.

 If the alpha component is not specified, it is assumed to be 1.0, i.e. the result will
 have an alpha set to 255.
test: none
end:

function: rsPackColorTo8888
attrib: const
ret: uchar4
arg: float r
arg: float g
arg: float b
arg: float a, "Alpha component."
test: none
end:

function: rsPackColorTo8888
attrib: const
ret: uchar4
arg: float3 color, "Vector of 3 or 4 floats containing the R, G, B, and A values."
test: none
end:

function: rsPackColorTo8888
attrib: const
ret: uchar4
arg: float4 color
test: none
end:

function: rsUnpackColor8888
# NOTE: The = below indicates that the generator should not add "overloadable" by default.
# We're doing this to stay backward compatible with the unusual declaration used when this
# function was introduced.
attrib: =const
ret: float4
arg: uchar4 c
summary: Create a float4 RGBA from uchar4
description:
 Unpacks a uchar4 color to float4.  The resulting floats will be between 0.0 and 1.0 inclusive.
test: none
end:

function: rsYuvToRGBA_#2#1
attrib: const
w: 4
t: u8, f32
ret: #2#1
arg: uchar y, "Luminance component."
arg: uchar u, "U chrominance component."
arg: uchar v, "V chrominance component."
summary: Convert a YUV value to RGBA
description:
 Converts a color from a YUV representation to RGBA.

 We currently don't provide a function to do the reverse conversion.
test: none
end:
