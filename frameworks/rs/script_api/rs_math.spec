#
# Copyright (C) 2014 The Android Open Source Project
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
summary: Mathematical Constants and Functions
description:
 The mathematical functions below can be applied to scalars and vectors.   When applied
 to vectors, the returned value is a vector of the function applied to each entry of the input.

 For example:<code><br/>
 float3 a, b;<br/>
 // The following call sets<br/>
 //   a.x to sin(b.x),<br/>
 //   a.y to sin(b.y), and<br/>
 //   a.z to sin(b.z).<br/>
 a = sin(b);<br/>
 </code>

 See <a href='rs_vector_math.html'>Vector Math Functions</a> for functions like @distance() and @length() that interpret
 instead the input as a single vector in n-dimensional space.

 The precision of the mathematical operations on 32 bit floats is affected by the pragmas
 rs_fp_relaxed and rs_fp_full.  Under rs_fp_relaxed, subnormal values may be flushed to zero and
 rounding may be done towards zero.  In comparison, rs_fp_full requires correct handling of
 subnormal values, i.e. smaller than 1.17549435e-38f.  rs_fp_rull also requires round to nearest
 with ties to even.

 Different precision/speed tradeoffs can be achieved by using variants of the common math
 functions.  Functions with a name starting with<ul>
 <li>native_: May have custom hardware implementations with weaker precision.  Additionally,
   subnormal values may be flushed to zero, rounding towards zero may be used, and NaN and
   infinity input may not be handled correctly.</li>
 <li>half_: May perform internal computations using 16 bit floats.  Additionally, subnormal
   values may be flushed to zero, and rounding towards zero may be used.</li>
 </ul>
end:

# TODO Add f16 versions of these constants.
constant: M_1_PI
value: 0.318309886183790671537767526745028724f
type: float
summary: 1 / pi, as a 32 bit float
description:
 The inverse of pi, as a 32 bit float.
end:

constant: M_2_PI
value: 0.636619772367581343075535053490057448f
type: float
summary: 2 / pi, as a 32 bit float
description:
 2 divided by pi, as a 32 bit float.
end:

constant: M_2_PIl
value: 0.636619772367581343075535053490057448f
type: float
hidden:
deprecated: 22, Use M_2_PI instead.
summary: 2 / pi, as a 32 bit float
description:
 2 divided by pi, as a 32 bit float.
end:

constant: M_2_SQRTPI
value: 1.128379167095512573896158903121545172f
type: float
summary:  2 / sqrt(pi), as a 32 bit float
description:
 2 divided by the square root of pi, as a 32 bit float.
end:

constant: M_E
value: 2.718281828459045235360287471352662498f
type: float
summary: e, as a 32 bit float
description:
 The number e, the base of the natural logarithm, as a 32 bit float.
end:

constant: M_LN10
value: 2.302585092994045684017991454684364208f
type: float
summary: log_e(10), as a 32 bit float
description:
 The natural logarithm of 10, as a 32 bit float.
end:

constant: M_LN2
value: 0.693147180559945309417232121458176568f
type: float
summary: log_e(2), as a 32 bit float
description:
 The natural logarithm of 2, as a 32 bit float.
end:

constant: M_LOG10E
value: 0.434294481903251827651128918916605082f
type: float
summary: log_10(e), as a 32 bit float
description:
 The logarithm base 10 of e, as a 32 bit float.
end:

constant: M_LOG2E
value: 1.442695040888963407359924681001892137f
type: float
summary: log_2(e), as a 32 bit float
description:
 The logarithm base 2 of e, as a 32 bit float.
end:

constant: M_PI
value: 3.141592653589793238462643383279502884f
type: float
summary: pi, as a 32 bit float
description:
 The constant pi, as a 32 bit float.
end:

constant: M_PI_2
value: 1.570796326794896619231321691639751442f
type: float
summary: pi / 2, as a 32 bit float
description:
 Pi divided by 2, as a 32 bit float.
end:

constant: M_PI_4
value: 0.785398163397448309615660845819875721f
type: float
summary: pi / 4, as a 32 bit float
description:
 Pi divided by 4, as a 32 bit float.
end:

constant: M_SQRT1_2
value: 0.707106781186547524400844362104849039f
type: float
summary: 1 / sqrt(2), as a 32 bit float
description:
 The inverse of the square root of 2, as a 32 bit float.
end:

constant: M_SQRT2
value: 1.414213562373095048801688724209698079f
type: float
summary: sqrt(2), as a 32 bit float
description:
 The square root of 2, as a 32 bit float.
end:

function: abs
version: 9
attrib: const
w: 1, 2, 3, 4
t: i8, i16, i32
ret: u#2#1
arg: #2#1 v
summary: Absolute value of an integer
description:
 Returns the absolute value of an integer.

 For floats, use @fabs().
end:

function: acos
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Inverse cosine
description:
 Returns the inverse cosine, in radians.

 See also @native_acos().
end:

function: acos
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1,1)
end:

function: acosh
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Inverse hyperbolic cosine
description:
 Returns the inverse hyperbolic cosine, in radians.

 See also @native_acosh().
end:

function: acosh
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: acospi
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Inverse cosine divided by pi
description:
 Returns the inverse cosine in radians, divided by pi.

 To get an inverse cosine measured in degrees, use <code>acospi(a) * 180.f</code>.

 See also @native_acospi().
end:

function: acospi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1,1)
end:

function: asin
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Inverse sine
description:
 Returns the inverse sine, in radians.

 See also @native_asin().
end:

function: asin
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1,1)
end:

function: asinh
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Inverse hyperbolic sine
description:
 Returns the inverse hyperbolic sine, in radians.

 See also @native_asinh().
end:

function: asinh
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: asinpi
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Inverse sine divided by pi
description:
 Returns the inverse sine in radians, divided by pi.

 To get an inverse sine measured in degrees, use <code>asinpi(a) * 180.f</code>.

 See also @native_asinpi().
end:

function: asinpi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1,1)
end:

function: atan
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Inverse tangent
description:
 Returns the inverse tangent, in radians.

 See also @native_atan().
end:

function: atan
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1,1)
end:

function: atan2
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 numerator, "Numerator."
arg: #2#1 denominator, "Denominator.  Can be 0."
summary: Inverse tangent of a ratio
description:
 Returns the inverse tangent of <code>(numerator / denominator)</code>, in radians.

 See also @native_atan2().
end:

function: atan2
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
end:

function: atan2pi
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 numerator, "Numerator."
arg: #2#1 denominator, "Denominator.  Can be 0."
summary: Inverse tangent of a ratio, divided by pi
description:
 Returns the inverse tangent of <code>(numerator / denominator)</code>, in radians, divided by pi.

 To get an inverse tangent measured in degrees, use <code>atan2pi(n, d) * 180.f</code>.

 See also @native_atan2pi().
end:

function: atan2pi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
end:

function: atanh
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Inverse hyperbolic tangent
description:
 Returns the inverse hyperbolic tangent, in radians.

 See also @native_atanh().
end:

function: atanh
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1,1)
end:

function: atanpi
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Inverse tangent divided by pi
description:
 Returns the inverse tangent in radians, divided by pi.

 To get an inverse tangent measured in degrees, use <code>atanpi(a) * 180.f</code>.

 See also @native_atanpi().
end:

function: atanpi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1,1)
end:

function: cbrt
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Cube root
description:
 Returns the cube root.

 See also @native_cbrt().
end:

function: cbrt
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: ceil
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Smallest integer not less than a value
description:
 Returns the smallest integer not less than a value.

 For example, <code>ceil(1.2f)</code> returns 2.f, and <code>ceil(-1.2f)</code> returns -1.f.

 See also @floor().
end:

function: ceil
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: clamp
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 value, "Value to be clamped."
arg: #2#1 min_value, "Lower bound, a scalar or matching vector."
arg: #2#1 max_value, above(min_value), "High bound, must match the type of low."
summary: Restrain a value to a range
description:
 Clamps a value to a specified high and low bound.  clamp() returns min_value
 if value &lt; min_value, max_value if value &gt; max_value, otherwise value.

 There are two variants of clamp: one where the min and max are scalars applied
 to all entries of the value, the other where the min and max are also vectors.

 If min_value is greater than max_value, the results are undefined.
end:

function: clamp
version: 9
attrib: const
w: 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 value
arg: #2 min_value
arg: #2 max_value, above(min_value)
end:

function: clamp
version: 19
attrib: const
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64
ret: #2#1
arg: #2#1 value
arg: #2#1 min_value
arg: #2#1 max_value, above(min_value)
end:

function: clamp
version: 19
attrib: const
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64
ret: #2#1
arg: #2#1 value
arg: #2 min_value
arg: #2 max_value, above(min_value)
end:

function: clamp
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 value
arg: #2#1 min_value
arg: #2#1 max_value, above(min_value)
end:

function: clamp
version: 24
attrib: const
w: 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 value
arg: #2 min_value
arg: #2 max_value, above(min_value)
end:

function: clz
version: 9
attrib: const
w: 1, 2, 3, 4
t: u8, u16, u32, i8, i16, i32
ret: #2#1
arg: #2#1 value
summary: Number of leading 0 bits
description:
 Returns the number of leading 0-bits in a value.

 For example, <code>clz((char)0x03)</code> returns 6.
end:

function: copysign
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 magnitude_value
arg: #2#1 sign_value
summary: Copies the sign of a number to another
description:
 Copies the sign from sign_value to magnitude_value.

 The value returned is either magnitude_value or -magnitude_value.

 For example, <code>copysign(4.0f, -2.7f)</code> returns -4.0f and <code>copysign(-4.0f, 2.7f)</code> returns 4.0f.
end:

function: copysign
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 magnitude_value
arg: #2#1 sign_value
end:

function: cos
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Cosine
description:
 Returns the cosine of an angle measured in radians.

 See also @native_cos().
end:

function: cos
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: cosh
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Hypebolic cosine
description:
 Returns the hypebolic cosine of v, where v is measured in radians.

 See also @native_cosh().
end:

function: cosh
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: cospi
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Cosine of a number multiplied by pi
description:
 Returns the cosine of <code>(v * pi)</code>, where <code>(v * pi)</code> is measured in radians.

 To get the cosine of a value measured in degrees, call <code>cospi(v / 180.f)</code>.

 See also @native_cospi().
end:

function: cospi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: degrees
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Converts radians into degrees
description:
 Converts from radians to degrees.
end:

function: degrees
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: erf
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Mathematical error function
description:
 Returns the error function.
end:

function: erf
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: erfc
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Mathematical complementary error function
description:
 Returns the complementary error function.
end:

function: erfc
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: exp
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: e raised to a number
description:
 Returns e raised to v, i.e. e ^ v.

 See also @native_exp().
end:

function: exp
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: exp10
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: 10 raised to a number
description:
 Returns 10 raised to v, i.e. 10.f ^ v.

 See also @native_exp10().
end:

function: exp10
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: exp2
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: 2 raised to a number
description:
 Returns 2 raised to v, i.e. 2.f ^ v.

 See also @native_exp2().
end:

function: exp2
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: expm1
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: e raised to a number minus one
description:
 Returns e raised to v minus 1, i.e. (e ^ v) - 1.

 See also @native_expm1().
end:

function: expm1
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: fabs
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Absolute value of a float
description:
 Returns the absolute value of the float v.

 For integers, use @abs().
end:

function: fabs
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: fdim
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
summary: Positive difference between two values
description:
 Returns the positive difference between two values.

 If a &gt; b, returns (a - b) otherwise returns 0f.
end:

function: fdim
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 a
arg: #2#1 b
end:

function: floor
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Smallest integer not greater than a value
description:
 Returns the smallest integer not greater than a value.

 For example, <code>floor(1.2f)</code> returns 1.f, and <code>floor(-1.2f)</code> returns -2.f.

 See also @ceil().
end:

function: floor
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: fma
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 multiplicand1
arg: #2#1 multiplicand2
arg: #2#1 offset
summary: Multiply and add
description:
 Multiply and add.  Returns <code>(multiplicand1 * multiplicand2) + offset</code>.

 This function is similar to @mad().  fma() retains full precision of the multiplied result
 and rounds only after the addition.  @mad() rounds after the multiplication and the addition.
 This extra precision is not guaranteed in rs_fp_relaxed mode.
end:

function: fma
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 multiplicand1
arg: #2#1 multiplicand2
arg: #2#1 offset
end:

function: fmax
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
summary: Maximum of two floats
description:
 Returns the maximum of a and b, i.e. <code>(a &lt; b ? b : a)</code>.

 The @max() function returns identical results but can be applied to more data types.
end:

function: fmax
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 a
arg: #2#1 b
end:

function: fmax
version: 9
attrib: const
w: 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 a
arg: #2 b
end:

function: fmax
version: 24
attrib: const
w: 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 a
arg: #2 b
end:

function: fmin
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
summary: Minimum of two floats
description:
 Returns the minimum of a and b, i.e. <code>(a &gt; b ? b : a)</code>.

 The @min() function returns identical results but can be applied to more data types.
end:

function: fmin
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 a
arg: #2#1 b
end:

function: fmin
version: 9
attrib: const
w: 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 a
arg: #2 b
end:

function: fmin
version: 24
attrib: const
w: 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 a
arg: #2 b
end:

function: fmod
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
summary: Modulo
description:
 Returns the remainder of (numerator / denominator), where the quotient is rounded towards zero.

 The function @remainder() is similar but rounds toward the closest interger.
 For example, <code>fmod(-3.8f, 2.f)</code> returns -1.8f (-3.8f - -1.f * 2.f)
 while <code>@remainder(-3.8f, 2.f)</code> returns 0.2f (-3.8f - -2.f * 2.f).
end:

function: fmod
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
end:

function: fract
version: 9
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, "Input value."
arg: #2#1* floor, "If floor is not null, *floor will be set to the floor of v."
summary: Positive fractional part
description:
 Returns the positive fractional part of v, i.e. <code>v - floor(v)</code>.

 For example, <code>fract(1.3f, &amp;val)</code> returns 0.3f and sets val to 1.f.
 <code>fract(-1.3f, &amp;val)</code> returns 0.7f and sets val to -2.f.
end:

function: fract
version: 9 23
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
inline:
 #2#1 unused;
 return fract(v, &unused);
end:

function: fract
version: 24
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
end:

function: fract
version: 24
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
arg: #2#1* floor
end:

function: fract
version: 24
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: frexp
version: 9
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, "Input value."
arg: int#1* exponent, "If exponent is not null, *exponent will be set to the exponent of v."
summary: Binary mantissa and exponent
description:
 Returns the binary mantissa and exponent of v, i.e. <code>v == mantissa * 2 ^ exponent</code>.

 The mantissa is always between 0.5 (inclusive) and 1.0 (exclusive).

 See @ldexp() for the reverse operation.  See also @logb() and @ilogb().
end:

function: frexp
version: 24
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
arg: int#1* exponent
test: none
end:

function: half_recip
version: 17
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Reciprocal computed to 16 bit precision
description:
 Returns the approximate reciprocal of a value.

 The precision is that of a 16 bit floating point value.

 See also @native_recip().
end:

function: half_rsqrt
version: 17
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Reciprocal of a square root computed to 16 bit precision
description:
 Returns the approximate value of <code>(1.f / sqrt(value))</code>.

 The precision is that of a 16 bit floating point value.

 See also @rsqrt(), @native_rsqrt().
end:

function: half_sqrt
version: 17
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Square root computed to 16 bit precision
description:
 Returns the approximate square root of a value.

 The precision is that of a 16 bit floating point value.

 See also @sqrt(), @native_sqrt().
end:

function: hypot
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
summary: Hypotenuse
description:
 Returns the hypotenuse, i.e. <code>sqrt(a * a + b * b)</code>.

 See also @native_hypot().
end:

function: hypot
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 a
arg: #2#1 b
end:

function: ilogb
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: int#1
arg: float#1 v
summary: Base two exponent
description:
 Returns the base two exponent of a value, where the mantissa is between
 1.f (inclusive) and 2.f (exclusive).

 For example, <code>ilogb(8.5f)</code> returns 3.

 Because of the difference in mantissa, this number is one less than is returned by @frexp().

 @logb() is similar but returns a float.
test: custom
end:

function: ilogb
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: int#1
arg: half#1 v
test: none
end:

function: ldexp
version: 9
attrib: const
w: 1, 2, 3, 4
ret: float#1
arg: float#1 mantissa, "Mantissa."
arg: int#1 exponent, "Exponent, a single component or matching vector."
summary: Creates a floating point from mantissa and exponent
description:
 Returns the floating point created from the mantissa and exponent,
 i.e. (mantissa * 2 ^ exponent).

 See @frexp() for the reverse operation.
end:

function: ldexp
version: 24
attrib: const
w: 1, 2, 3, 4
ret: half#1
arg: half#1 mantissa
arg: int#1 exponent
test: none
end:

function: ldexp
version: 9
attrib: const
w: 2, 3, 4
ret: float#1
arg: float#1 mantissa
arg: int exponent
end:

function: ldexp
version: 24
attrib: const
w: 2, 3, 4
ret: half#1
arg: half#1 mantissa
arg: int exponent
test: none
end:

function: lgamma
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Natural logarithm of the gamma function
description:
 Returns the natural logarithm of the absolute value of the gamma function,
 i.e. <code>@log(@fabs(@tgamma(v)))</code>.

 See also @tgamma().
end:

function: lgamma
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
test: none
end:

function: lgamma
version: 9
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
arg: int#1* sign_of_gamma, "If sign_of_gamma is not null, *sign_of_gamma will be set to -1.f if the gamma of v is negative, otherwise to 1.f."
test: custom
#TODO Temporary until bionic & associated drivers are fixed
end:

function: lgamma
version: 24
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
arg: int#1* sign_of_gamma
test: none
end:

function: log
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Natural logarithm
description:
 Returns the natural logarithm.

 See also @native_log().
end:

function: log
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: log10
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Base 10 logarithm
description:
 Returns the base 10 logarithm.

 See also @native_log10().
end:

function: log10
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: log1p
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Natural logarithm of a value plus 1
description:
 Returns the natural logarithm of <code>(v + 1.f)</code>.

 See also @native_log1p().
end:

function: log1p
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: log2
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Base 2 logarithm
description:
 Returns the base 2 logarithm.

 See also @native_log2().
end:

function: log2
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: logb
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Base two exponent
description:
 Returns the base two exponent of a value, where the mantissa is between
 1.f (inclusive) and 2.f (exclusive).

 For example, <code>logb(8.5f)</code> returns 3.f.

 Because of the difference in mantissa, this number is one less than is returned by frexp().

 @ilogb() is similar but returns an integer.
end:

function: logb
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: mad
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 multiplicand1
arg: #2#1 multiplicand2
arg: #2#1 offset
summary: Multiply and add
description:
 Multiply and add.  Returns <code>(multiplicand1 * multiplicand2) + offset</code>.

 This function is similar to @fma().  @fma() retains full precision of the multiplied result
 and rounds only after the addition.  mad() rounds after the multiplication and the addition.
 In rs_fp_relaxed mode, mad() may not do the rounding after multiplicaiton.
end:

function: mad
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 multiplicand1
arg: #2#1 multiplicand2
arg: #2#1 offset
end:

function: max
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
summary: Maximum
description:
 Returns the maximum value of two arguments.
end:

function: max
version:24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 a
arg: #2#1 b
end:

function: max
version: 9
attrib: const
w: 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 a
arg: #2 b
end:

function: max
version: 24
attrib: const
w: 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 a
arg: #2 b
end:

function: max
version: 9 20
attrib: const
w: 1
t: i8, i16, i32, u8, u16, u32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
inline:
 return (a > b ? a : b);
end:

function: max
version: 9 20
attrib: const
w: 2
t: i8, i16, i32, u8, u16, u32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
inline:
 #2#1 tmp;
 tmp.x = (a.x > b.x ? a.x : b.x);
 tmp.y = (a.y > b.y ? a.y : b.y);
 return tmp;
end:

function: max
version: 9 20
attrib: const
w: 3
t: i8, i16, i32, u8, u16, u32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
inline:
 #2#1 tmp;
 tmp.x = (a.x > b.x ? a.x : b.x);
 tmp.y = (a.y > b.y ? a.y : b.y);
 tmp.z = (a.z > b.z ? a.z : b.z);
 return tmp;
end:

function: max
version: 9 20
attrib: const
w: 4
t: i8, i16, i32, u8, u16, u32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
inline:
 #2#1 tmp;
 tmp.x = (a.x > b.x ? a.x : b.x);
 tmp.y = (a.y > b.y ? a.y : b.y);
 tmp.z = (a.z > b.z ? a.z : b.z);
 tmp.w = (a.w > b.w ? a.w : b.w);
 return tmp;
end:

function: max
version: 21
attrib: const
w: 1, 2, 3, 4
t: i8, i16, i32, i64, u8, u16, u32, u64
ret: #2#1
arg: #2#1 a
arg: #2#1 b
end:

function: min
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
summary: Minimum
description:
 Returns the minimum value of two arguments.
end:

function: min
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 a
arg: #2#1 b
end:

function: min
version: 9
attrib: const
w: 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 a
arg: #2 b
end:

function: min
version: 24
attrib: const
w: 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 a
arg: #2 b
end:

function: min
version: 9 20
attrib: const
w: 1
t: i8, i16, i32, u8, u16, u32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
inline:
 return (a < b ? a : b);
end:

function: min
version: 9 20
attrib: const
w: 2
t: i8, i16, i32, u8, u16, u32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
inline:
 #2#1 tmp;
 tmp.x = (a.x < b.x ? a.x : b.x);
 tmp.y = (a.y < b.y ? a.y : b.y);
 return tmp;
end:

function: min
version: 9 20
attrib: const
w: 3
t: i8, i16, i32, u8, u16, u32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
inline:
 #2#1 tmp;
 tmp.x = (a.x < b.x ? a.x : b.x);
 tmp.y = (a.y < b.y ? a.y : b.y);
 tmp.z = (a.z < b.z ? a.z : b.z);
 return tmp;
end:

function: min
version: 9 20
attrib: const
w: 4
t: i8, i16, i32, u8, u16, u32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
inline:
 #2#1 tmp;
 tmp.x = (a.x < b.x ? a.x : b.x);
 tmp.y = (a.y < b.y ? a.y : b.y);
 tmp.z = (a.z < b.z ? a.z : b.z);
 tmp.w = (a.w < b.w ? a.w : b.w);
 return tmp;
end:

function: min
version: 21
attrib: const
w: 1, 2, 3, 4
t: i8, i16, i32, i64, u8, u16, u32, u64
ret: #2#1
arg: #2#1 a
arg: #2#1 b
end:

function: mix
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 start
arg: #2#1 stop
arg: #2#1 fraction
summary: Mixes two values
description:
 Returns start + ((stop - start) * fraction).

 This can be useful for mixing two values.  For example, to create a new color that is
 40% color1 and 60% color2, use <code>mix(color1, color2, 0.6f)</code>.
end:

function: mix
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 start
arg: #2#1 stop
arg: #2#1 fraction
end:

function: mix
version: 9
attrib: const
w: 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 start
arg: #2#1 stop
arg: #2 fraction
end:

function: mix
version: 24
attrib: const
w: 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 start
arg: #2#1 stop
arg: #2 fraction
end:

function: modf
version: 9
w: 1, 2, 3, 4
t: f32
ret: #2#1, "Floating point portion of the value."
arg: #2#1 v, "Source value."
arg: #2#1* integral_part, "*integral_part will be set to the integral portion of the number."
summary: Integral and fractional components
description:
 Returns the integral and fractional components of a number.

 Both components will have the same sign as x.  For example, for an input of -3.72f,
 *integral_part will be set to -3.f and .72f will be returned.
end:

function: modf
version: 24
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
arg: #2#1* integral_part
test: none
end:

function: nan
version: 9
attrib: const
w: 1
t: f32
ret: #2#1
arg: uint#1 v, "Not used."
#TODO We're not using the argument.  Once we do, add this documentation line:
# The argument is embedded into the return value and can be used to distinguish various NaNs.
summary: Not a Number
description:
 Returns a NaN value (Not a Number).
end:

function: nan_half
version: 24
attrib: const
t: f16
ret: #1
summary: Not a Number
description:
  Returns a half-precision floating point NaN value (Not a Number).
end:

function: native_acos
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Approximate inverse cosine
description:
 Returns the approximate inverse cosine, in radians.

 This function yields undefined results from input values less than -1 or greater than 1.

 See also @acos().
# TODO Temporary
test: limited(0.0005)
end:

function: native_acos
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1,1)
# Absolute error of 2^-11, i.e. 0.00048828125
test: limited(0.00048828125)
end:

function: native_acosh
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate inverse hyperbolic cosine
description:
 Returns the approximate inverse hyperbolic cosine, in radians.

 See also @acosh().
# TODO Temporary
test: limited(0.0005)
end:

function: native_acosh
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: native_acospi
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Approximate inverse cosine divided by pi
description:
 Returns the approximate inverse cosine in radians, divided by pi.

 To get an inverse cosine measured in degrees, use <code>acospi(a) * 180.f</code>.

 This function yields undefined results from input values less than -1 or greater than 1.

 See also @acospi().
# TODO Temporary
test: limited(0.0005)
end:

function: native_acospi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1,1)
# Absolute error of 2^-11, i.e. 0.00048828125
test: limited(0.00048828125)
end:

function: native_asin
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Approximate inverse sine
description:
 Returns the approximate inverse sine, in radians.

 This function yields undefined results from input values less than -1 or greater than 1.

 See also @asin().
# TODO Temporary
test: limited(0.0005)
end:

function: native_asin
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1,1)
# Absolute error of 2^-11, i.e. 0.00048828125
test: limited(0.00048828125)
end:

function: native_asinh
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate inverse hyperbolic sine
description:
 Returns the approximate inverse hyperbolic sine, in radians.

 See also @asinh().
# TODO Temporary
test: limited(0.0005)
end:

function: native_asinh
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: native_asinpi
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Approximate inverse sine divided by pi
description:
 Returns the approximate inverse sine in radians, divided by pi.

 To get an inverse sine measured in degrees, use <code>asinpi(a) * 180.f</code>.

 This function yields undefined results from input values less than -1 or greater than 1.

 See also @asinpi().
# TODO Temporary
test: limited(0.0005)
end:

function: native_asinpi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1,1)
# Absolute error of 2^-11, i.e. 0.00048828125
test: limited(0.00048828125)
end:

function: native_atan
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Approximate inverse tangent
description:
 Returns the approximate inverse tangent, in radians.

 See also @atan().
# TODO Temporary
test: limited(0.0005)
end:

function: native_atan
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1, 1)
end:

function: native_atan2
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 numerator, "Numerator."
arg: #2#1 denominator, "Denominator.  Can be 0."
summary: Approximate inverse tangent of a ratio
description:
 Returns the approximate inverse tangent of <code>(numerator / denominator)</code>, in radians.

 See also @atan2().
# TODO Temporary
test: limited(0.0005)
end:

function: native_atan2
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
end:

function: native_atan2pi
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 numerator, "Numerator."
arg: #2#1 denominator, "Denominator.  Can be 0."
summary: Approximate inverse tangent of a ratio, divided by pi
description:
 Returns the approximate inverse tangent of <code>(numerator / denominator)</code>,
 in radians, divided by pi.

 To get an inverse tangent measured in degrees, use <code>atan2pi(n, d) * 180.f</code>.

 See also @atan2pi().
# TODO Temporary
test: limited(0.0005)
end:

function: native_atan2pi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
end:

function: native_atanh
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Approximate inverse hyperbolic tangent
description:
 Returns the approximate inverse hyperbolic tangent, in radians.

 See also @atanh().
# TODO Temporary
test: limited(0.0005)
end:

function: native_atanh
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1,1)
end:

function: native_atanpi
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-1,1)
summary: Approximate inverse tangent divided by pi
description:
 Returns the approximate inverse tangent in radians, divided by pi.

 To get an inverse tangent measured in degrees, use <code>atanpi(a) * 180.f</code>.

 See also @atanpi().
# TODO Temporary
test: limited(0.0005)
end:

function: native_atanpi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-1,1)
end:

function: native_cbrt
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate cube root
description:
 Returns the approximate cubic root.

 See also @cbrt().
end:

function: native_cbrt
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: native_cos
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate cosine
description:
 Returns the approximate cosine of an angle measured in radians.

 See also @cos().
end:

function: native_cos
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-314,314)
# Absolute error of 2^-11, i.e. 0.00048828125
test: limited(0.00048828125)
end:

function: native_cosh
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate hypebolic cosine
description:
 Returns the approximate hypebolic cosine.

 See also @cosh().
end:

function: native_cosh
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: native_cospi
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate cosine of a number multiplied by pi
description:
 Returns the approximate cosine of (v * pi), where (v * pi) is measured in radians.

 To get the cosine of a value measured in degrees, call <code>cospi(v / 180.f)</code>.

 See also @cospi().
end:

function: native_cospi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-100,100)
# Absolute error of 2^-11, i.e. 0.00048828125
test: limited(0.00048828125)
end:

function: native_divide
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 left_vector
arg: #2#1 right_vector
summary: Approximate division
description:
 Computes the approximate division of two values.
end:

function: native_divide
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 left_vector
arg: #2#1 right_vector
end:

function: native_exp
version: 18
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-86,86)
summary: Approximate e raised to a number
description:
 Fast approximate exp.

 It is valid for inputs from -86.f to 86.f.  The precision is no worse than what would be
 expected from using 16 bit floating point values.

 See also @exp().
test: limited
end:

function: native_exp
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-86,86)
end:

function: native_exp10
version: 18
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-37,37)
summary: Approximate 10 raised to a number
description:
 Fast approximate exp10.

 It is valid for inputs from -37.f to 37.f.  The precision is no worse than what would be
 expected from using 16 bit floating point values.

 See also @exp10().
test: limited
end:

function: native_exp10
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-37,37)
end:

function: native_exp2
version: 18
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(-125,125)
summary: Approximate 2 raised to a number
description:
 Fast approximate exp2.

 It is valid for inputs from -125.f to 125.f.  The precision is no worse than what would be
 expected from using 16 bit floating point values.

 See also @exp2().
test: limited
end:

function: native_exp2
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-125,125)
end:

function: native_expm1
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate e raised to a number minus one
description:
 Returns the approximate (e ^ v) - 1.

 See also @expm1().
end:

function: native_expm1
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
test: custom
end:

function: native_hypot
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 a
arg: #2#1 b
summary: Approximate hypotenuse
description:
 Returns the approximate native_sqrt(a * a + b * b)

 See also @hypot().
end:

function: native_hypot
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 a
arg: #2#1 b
end:

function: native_log
version: 18
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(10e-10,10e10)
summary: Approximate natural logarithm
description:
 Fast approximate log.

 It is not accurate for values very close to zero.

 See also @log().
test: limited
end:

function: native_log
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(10e-5,65504)
end:

function: native_log10
version: 18
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(10e-10,10e10)
summary: Approximate base 10 logarithm
description:
 Fast approximate log10.

 It is not accurate for values very close to zero.

 See also @log10().
test: limited
end:

function: native_log10
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(10e-5,65504)
end:

function: native_log1p
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate natural logarithm of a value plus 1
description:
 Returns the approximate natural logarithm of (v + 1.0f)

 See also @log1p().
end:

function: native_log1p
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: native_log2
version: 18
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v, range(10e-10,10e10)
summary: Approximate base 2 logarithm
description:
 Fast approximate log2.

 It is not accurate for values very close to zero.

 See also @log2().
test: limited
end:

function: native_log2
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(10e-5,65504)
end:

function: native_powr
version: 18
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 base, range(0,256), "Must be between 0.f and 256.f.  The function is not accurate for values very close to zero."
arg: #2#1 exponent, range(-15,15), "Must be between -15.f and 15.f."
summary: Approximate positive base raised to an exponent
description:
 Fast approximate (base ^ exponent).

 See also @powr().
test: limited
end:

function: native_powr
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 base, range(0,256)
arg: #2#1 exponent, range(-15,15)
end:

function: native_recip
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate reciprocal
description:
 Returns the approximate approximate reciprocal of a value.

 See also @half_recip().
end:

function: native_recip
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: native_rootn
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
arg: int#1 n
summary: Approximate nth root
description:
 Compute the approximate Nth root of a value.

 See also @rootn().
end:

function: native_rootn
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
arg: int#1 n
test: none
end:

function: native_rsqrt
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate reciprocal of a square root
description:
 Returns approximate (1 / sqrt(v)).

 See also @rsqrt(), @half_rsqrt().
end:

function: native_rsqrt
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: native_sin
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate sine
description:
 Returns the approximate sine of an angle measured in radians.

 See also @sin().
end:

function: native_sin
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-314,314)
# Absolute error of 2^-11, i.e. 0.00048828125
test: limited(0.00048828125)
end:

function: native_sincos
version: 21
w: 1, 2, 3, 4
t: f32
ret: #2#1, "Sine."
arg: #2#1 v, "Incoming value in radians."
arg: #2#1* cos, "*cos will be set to the cosine value."
summary: Approximate sine and cosine
description:
 Returns the approximate sine and cosine of a value.

 See also @sincos().
# TODO Temporary
test: limited(0.0005)
end:

function: native_sincos
version: 24
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
arg: #2#1* cos, range(-314,314)
# Absolute error of 2^-11, i.e. 0.00048828125
test: limited(0.00048828125)
end:

function: native_sinh
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate hyperbolic sine
description:
 Returns the approximate hyperbolic sine of a value specified in radians.

 See also @sinh().
end:

function: native_sinh
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: native_sinpi
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate sine of a number multiplied by pi
description:
 Returns the approximate sine of (v * pi), where (v * pi) is measured in radians.

 To get the sine of a value measured in degrees, call <code>sinpi(v / 180.f)</code>.

 See also @sinpi().
end:

function: native_sinpi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-100,100)
# Absolute error of 2^-11, i.e. 0.00048828125
test: limited(0.00048828125)
end:

function: native_sqrt
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate square root
description:
 Returns the approximate sqrt(v).

 See also @sqrt(), @half_sqrt().
end:

function: native_sqrt
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: native_tan
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate tangent
description:
 Returns the approximate tangent of an angle measured in radians.
end:

function: native_tan
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-314,314)
test: custom
end:

function: native_tanh
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate hyperbolic tangent
description:
 Returns the approximate hyperbolic tangent of a value.

 See also @tanh().
end:

function: native_tanh
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: native_tanpi
version: 21
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Approximate tangent of a number multiplied by pi
description:
 Returns the approximate tangent of (v * pi), where (v * pi) is measured in radians.

 To get the tangent of a value measured in degrees, call <code>tanpi(v / 180.f)</code>.

 See also @tanpi().
end:

function: native_tanpi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v, range(-100,100)
test: custom
end:

function: nextafter
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
arg: #2#1 target
summary: Next floating point number
description:
 Returns the next representable floating point number from v towards target.

 In rs_fp_relaxed mode, a denormalized input value may not yield the next denormalized
 value, as support of denormalized values is optional in relaxed mode.
end:

function: nextafter
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
arg: #2#1 target
test: none
end:

function: pow
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 base
arg: #2#1 exponent
summary: Base raised to an exponent
description:
 Returns base raised to the power exponent, i.e. base ^ exponent.

 @pown() and @powr() are similar.  @pown() takes an integer exponent. @powr() assumes the
 base to be non-negative.
end:

function: pow
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 base
arg: #2#1 exponent
end:

function: pown
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 base
arg: int#1 exponent
summary: Base raised to an integer exponent
description:
 Returns base raised to the power exponent, i.e. base ^ exponent.

 @pow() and @powr() are similar.  The both take a float exponent. @powr() also assumes the
 base to be non-negative.
end:

function: pown
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 base
arg: int#1 exponent
end:

function: powr
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 base, range(0,3000)
arg: #2#1 exponent
summary: Positive base raised to an exponent
description:
 Returns base raised to the power exponent, i.e. base ^ exponent.  base must be &gt;= 0.

 @pow() and @pown() are similar.  They both make no assumptions about the base.
 @pow() takes a float exponent while @pown() take an integer.

 See also @native_powr().
end:

function: powr
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 base, range(0,300)
arg: #2#1 exponent
end:

function: radians
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Converts degrees into radians
description:
 Converts from degrees to radians.
end:

function: radians
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: remainder
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
summary: Remainder of a division
description:
 Returns the remainder of (numerator / denominator), where the quotient is rounded towards
 the nearest integer.

 The function @fmod() is similar but rounds toward the closest interger.
 For example, <code>@fmod(-3.8f, 2.f)</code> returns -1.8f (-3.8f - -1.f * 2.f)
 while <code>remainder(-3.8f, 2.f)</code> returns 0.2f (-3.8f - -2.f * 2.f).
end:

function: remainder
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
end:

function: remquo
version: 9
w: 1, 2, 3, 4
t: f32
ret: #2#1, "Remainder, precise only for the low three bits."
arg: #2#1 numerator, "Numerator."
arg: #2#1 denominator, "Denominator."
arg: int#1* quotient, "*quotient will be set to the integer quotient."
summary: Remainder and quotient of a division
description:
 Returns the quotient and the remainder of (numerator / denominator).

 Only the sign and lowest three bits of the quotient are guaranteed to be accurate.

 This function is useful for implementing periodic functions.  The low three bits of the
 quotient gives the quadrant and the remainder the distance within the quadrant.
 For example, an implementation of @sin(x) could call <code>remquo(x, PI / 2.f, &amp;quadrant)</code>
 to reduce very large value of x to something within a limited range.

 Example: <code>remquo(-23.5f, 8.f, &amp;quot)</code> sets the lowest three bits of quot to 3
 and the sign negative.  It returns 0.5f.
test: custom
end:

function: remquo
version: 24
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 numerator
arg: #2#1 denominator
arg: int#1* quotient
test: none
end:

function: rint
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Round to even
description:
 Rounds to the nearest integral value.

 rint() rounds half values to even.  For example, <code>rint(0.5f)</code> returns 0.f and
 <code>rint(1.5f)</code> returns 2.f.  Similarly, <code>rint(-0.5f)</code> returns -0.f and
 <code>rint(-1.5f)</code> returns -2.f.

 @round() is similar but rounds away from zero.  @trunc() truncates the decimal fraction.
end:

function: rint
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: rootn
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
arg: int#1 n
summary: Nth root
description:
 Compute the Nth root of a value.

 See also @native_rootn().
end:

function: rootn
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
arg: int#1 n
test: none
end:

function: round
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Round away from zero
description:
 Round to the nearest integral value.

 round() rounds half values away from zero.  For example, <code>round(0.5f)</code> returns 1.f
 and <code>round(1.5f)</code> returns 2.f.  Similarly, <code>round(-0.5f)</code> returns -1.f
 and <code>round(-1.5f)</code> returns -2.f.

 @rint() is similar but rounds half values toward even.  @trunc() truncates the decimal fraction.
end:

function: round
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: rsqrt
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Reciprocal of a square root
description:
 Returns (1 / sqrt(v)).

 See also @half_rsqrt(), @native_rsqrt().
end:

function: rsqrt
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: sign
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Sign of a value
description:
 Returns the sign of a value.

 if (v &lt; 0) return -1.f;
 else if (v &gt; 0) return 1.f;
 else return 0.f;
end:

function: sign
version:24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: sin
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Sine
description:
 Returns the sine of an angle measured in radians.

 See also @native_sin().
end:

function: sin
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: sincos
version: 9
w: 1, 2, 3, 4
t: f32
ret: #2#1, "Sine of v."
arg: #2#1 v, "Incoming value in radians."
arg: #2#1* cos, "*cos will be set to the cosine value."
summary: Sine and cosine
description:
 Returns the sine and cosine of a value.

 See also @native_sincos().
end:

function: sincos
version: 24
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
arg: #2#1* cos
end:

function: sinh
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Hyperbolic sine
description:
 Returns the hyperbolic sine of v, where v is measured in radians.

 See also @native_sinh().
end:

function: sinh
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: sinpi
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Sine of a number multiplied by pi
description:
 Returns the sine of (v * pi), where (v * pi) is measured in radians.

 To get the sine of a value measured in degrees, call <code>sinpi(v / 180.f)</code>.

 See also @native_sinpi().
end:

function: sinpi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: sqrt
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Square root
description:
 Returns the square root of a value.

 See also @half_sqrt(), @native_sqrt().
end:

function: sqrt
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: step
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 edge
arg: #2#1 v
summary: 0 if less than a value, 0 otherwise
description:
 Returns 0.f if v &lt; edge, 1.f otherwise.

 This can be useful to create conditional computations without using loops and branching
 instructions.  For example, instead of computing <code>(a[i] &lt; b[i]) ? 0.f : @atan2(a[i], b[i])</code>
 for the corresponding elements of a vector, you could instead use <code>step(a, b) * @atan2(a, b)</code>.
end:

function: step
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 edge
arg: #2#1 v
end:

function: step
version: 9
attrib: const
w: 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 edge
arg: #2 v
end:

function: step
version: 24
attrib: const
w: 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 edge
arg: #2 v
end:

function: step
version: 21
attrib: const
w: 2, 3, 4
t: f32
ret: #2#1
arg: #2 edge
arg: #2#1 v
end:

function: step
version: 24
attrib: const
w: 2, 3, 4
t: f16
ret: #2#1
arg: #2 edge
arg: #2#1 v
end:

function: tan
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Tangent
description:
 Returns the tangent of an angle measured in radians.

 See also @native_tan().
end:

function: tan
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: tanh
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Hyperbolic tangent
description:
 Returns the hyperbolic tangent of a value.

 See also @native_tanh().
end:

function: tanh
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: tanpi
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Tangent of a number multiplied by pi
description:
 Returns the tangent of (v * pi), where (v * pi) is measured in radians.

 To get the tangent of a value measured in degrees, call <code>tanpi(v / 180.f)</code>.

 See also @native_tanpi().
end:

function: tanpi
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: tgamma
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Gamma function
description:
 Returns the gamma function of a value.

 See also @lgamma().
end:

function: tgamma
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: trunc
version: 9
attrib: const
w: 1, 2, 3, 4
t: f32
ret: #2#1
arg: #2#1 v
summary: Truncates a floating point
description:
 Rounds to integral using truncation.

 For example, <code>trunc(1.7f)</code> returns 1.f and <code>trunc(-1.7f)</code> returns -1.f.

 See @rint() and @round() for other rounding options.
end:

function: trunc
version: 24
attrib: const
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: #2#1 v
end:

function: rsClamp
attrib: const
t: i8, i16, i32, u8, u16, u32
ret: #1
arg: #1 amount, "Value to clamp."
arg: #1 low, "Lower bound."
arg: #1 high, "Upper bound."
deprecated: 22, Use @clamp() instead.
summary: Restrain a value to a range
description:
 Clamp a value between low and high.
test: none
end:

function: rsFrac
attrib: const
ret: float
arg: float v
deprecated: 22, Use @fract() instead.
summary: Returns the fractional part of a float
description:
 Returns the fractional part of a float
test: none
end:

function: rsRand
ret: int
arg: int max_value
summary: Pseudo-random number
description:
 Return a random value between 0 (or min_value) and max_malue.
test: none
end:

function: rsRand
ret: int
arg: int min_value
arg: int max_value
test: none
end:

function: rsRand
ret: float
arg: float max_value
test: none
end:

function: rsRand
ret: float
arg: float min_value
arg: float max_value
test: none
end:
