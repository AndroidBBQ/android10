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
summary: Object Types
description:
 The types below are used to manipulate RenderScript objects like allocations, samplers,
 elements, and scripts.  Most of these object are created using the Java RenderScript APIs.
include:
 #define NULL ((void *)0)

 // Opaque handle to a RenderScript object. Do not use this directly.
 #ifndef __LP64__
 #define _RS_OBJECT_DECL \
 {\
   const int* const p;\
 } __attribute__((packed, aligned(4)))
 #else
 #define _RS_OBJECT_DECL \
 {\
   const long* const p;\
   const long* const unused1;\
   const long* const unused2;\
   const long* const unused3;\
 }
 #endif
end:

type: rs_element
rs_object:
summary: Handle to an element
description:
 An opaque handle to a RenderScript element.

 See <a href="http://developer.android.com/reference/android/renderscript/Element.html">android.renderscript.Element</a>.
end:

type: rs_type
rs_object:
summary: Handle to a Type
description:
 An opaque handle to a RenderScript type.

 See <a href="http://developer.android.com/reference/android/renderscript/Type.html">android.renderscript.Type</a>.
end:

type: rs_allocation
rs_object:
summary: Handle to an allocation
description:
 An opaque handle to a RenderScript allocation.

 See <a href="http://developer.android.com/reference/android/renderscript/Allocation.html">android.renderscript.Allocation</a>.
end:

type: rs_sampler
rs_object:
summary: Handle to a Sampler
description:
 An opaque handle to a RenderScript sampler object.

 See <a href="http://developer.android.com/reference/android/renderscript/Sampler.html">android.renderscript.Sampler</a>.
end:

type: rs_script
rs_object:
summary: Handle to a Script
description:
 An opaque handle to a RenderScript script object.

 See <a href="http://developer.android.com/reference/android/renderscript/ScriptC.html">android.renderscript.ScriptC</a>.
end:

type: rs_allocation_cubemap_face
version: 14
enum:
value: RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X = 0
value: RS_ALLOCATION_CUBEMAP_FACE_NEGATIVE_X = 1
value: RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_Y = 2
value: RS_ALLOCATION_CUBEMAP_FACE_NEGATIVE_Y = 3
value: RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_Z = 4
value: RS_ALLOCATION_CUBEMAP_FACE_NEGATIVE_Z = 5
summary: Enum for selecting cube map faces
description:
 An enum used to specify one the six faces of a cubemap.
end:

type: rs_allocation_usage_type
version: 14
enum:
value: RS_ALLOCATION_USAGE_SCRIPT = 0x0001, "Allocation is bound to and accessed by scripts."
value: RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE = 0x0002, "Allocation is used as a texture source."
value: RS_ALLOCATION_USAGE_GRAPHICS_VERTEX = 0x0004, "Deprecated."
value: RS_ALLOCATION_USAGE_GRAPHICS_CONSTANTS = 0x0008, "Deprecated."
value: RS_ALLOCATION_USAGE_GRAPHICS_RENDER_TARGET = 0x0010, "Deprecated."
value: RS_ALLOCATION_USAGE_IO_INPUT = 0x0020, "Allocation is used as a Surface consumer."
value: RS_ALLOCATION_USAGE_IO_OUTPUT = 0x0040, "Allocation is used as a Surface producer."
value: RS_ALLOCATION_USAGE_SHARED = 0x0080, "Allocation's backing store is shared with another object (usually a Bitmap).  Copying to or from the original source Bitmap will cause a synchronization rather than a full copy."
summary: Bitfield to specify how an allocation is used
description:
 These values are ORed together to specify which usages or memory spaces are
 relevant to an allocation or an operation on an allocation.
end:

type: rs_data_type
version: 16
enum:
value: RS_TYPE_NONE = 0, "Element is a complex type, i.e. a struct."
value: RS_TYPE_FLOAT_16 = 1, "A 16 bit floating point value."
value: RS_TYPE_FLOAT_32 = 2, "A 32 bit floating point value."
value: RS_TYPE_FLOAT_64 = 3, "A 64 bit floating point value."
value: RS_TYPE_SIGNED_8 = 4, "An 8 bit signed integer."
value: RS_TYPE_SIGNED_16 = 5, "A 16 bit signed integer."
value: RS_TYPE_SIGNED_32 = 6, "A 32 bit signed integer."
value: RS_TYPE_SIGNED_64 = 7, "A 64 bit signed integer."
value: RS_TYPE_UNSIGNED_8 = 8, "An 8 bit unsigned integer."
value: RS_TYPE_UNSIGNED_16 = 9, "A 16 bit unsigned integer."
value: RS_TYPE_UNSIGNED_32 = 10, "A 32 bit unsigned integer."
value: RS_TYPE_UNSIGNED_64 = 11, "A 64 bit unsigned integer."
value: RS_TYPE_BOOLEAN = 12, "0 or 1 (false or true) stored in an 8 bit container."
value: RS_TYPE_UNSIGNED_5_6_5 = 13, "A 16 bit unsigned integer packing graphical data in 5, 6, and 5 bit sections."
value: RS_TYPE_UNSIGNED_5_5_5_1 = 14, "A 16 bit unsigned integer packing graphical data in 5, 5, 5, and 1 bit sections."
value: RS_TYPE_UNSIGNED_4_4_4_4 = 15, "A 16 bit unsigned integer packing graphical data in 4, 4, 4, and 4 bit sections."
value: RS_TYPE_MATRIX_4X4 = 16, "A 4x4 matrix of 32 bit floats, aligned on a 32 bit boundary."
value: RS_TYPE_MATRIX_3X3 = 17, "A 3x3 matrix of 32 bit floats, aligned on a 32 bit boundary."
value: RS_TYPE_MATRIX_2X2 = 18, "A 2x2 matrix of 32 bit floats, aligned on a 32 bit boundary."
value: RS_TYPE_ELEMENT = 1000, "A handle to an Element."
value: RS_TYPE_TYPE = 1001, "A handle to a Type."
value: RS_TYPE_ALLOCATION = 1002, "A handle to an Allocation."
value: RS_TYPE_SAMPLER = 1003, "A handle to a Sampler."
value: RS_TYPE_SCRIPT = 1004, "A handle to a Script."
value: RS_TYPE_MESH = 1005, "Deprecated."
value: RS_TYPE_PROGRAM_FRAGMENT = 1006, "Deprecated."
value: RS_TYPE_PROGRAM_VERTEX = 1007, "Deprecated."
value: RS_TYPE_PROGRAM_RASTER = 1008, "Deprecated."
value: RS_TYPE_PROGRAM_STORE = 1009, "Deprecated."
value: RS_TYPE_FONT = 1010, "Deprecated."
value: RS_TYPE_INVALID = 10000
summary: Element basic data type
description:
 rs_data_type is used to encode the type information of a basic element.

 RS_TYPE_UNSIGNED_5_6_5, RS_TYPE_UNSIGNED_5_5_5_1, RS_TYPE_UNSIGNED_4_4_4_4 are for packed
 graphical data formats and represent vectors with per vector member sizes which are treated
 as a single unit for packing and alignment purposes.
end:

type: rs_data_kind
version: 16
enum:
value: RS_KIND_USER         = 0, "No special interpretation."
value: RS_KIND_PIXEL_L      = 7, "Luminance."
value: RS_KIND_PIXEL_A      = 8, "Alpha."
value: RS_KIND_PIXEL_LA     = 9, "Luminance and Alpha."
value: RS_KIND_PIXEL_RGB    = 10, "Red, Green, Blue."
value: RS_KIND_PIXEL_RGBA   = 11, "Red, Green, Blue, and Alpha."
value: RS_KIND_PIXEL_DEPTH  = 12, "Depth for a depth texture."
value: RS_KIND_PIXEL_YUV    = 13, "Luminance and chrominance."
value: RS_KIND_INVALID      = 100
summary: Element data kind
description:
 This enumeration is primarly useful for graphical data.  It provides additional information to
 help interpret the rs_data_type.

 RS_KIND_USER indicates no special interpretation is expected.

 The RS_KIND_PIXEL_* values are used in conjunction with the standard data types for representing
 texture formats.

 See the <a href='http://developer.android.com/reference/android/renderscript/Element.html#createPixel(android.renderscript.RenderScript,%20android.renderscript.Element.DataType, android.renderscript.Element.DataKind)'>Element.createPixel()</a> method.
end:

type: rs_yuv_format
version: 24
enum:
value: RS_YUV_NONE = 0
value: RS_YUV_YV12 = 0x32315659
value: RS_YUV_NV21 = 0x11
value: RS_YUV_420_888 = 0x23
summary: YUV format
description:
  Android YUV formats that can be associated with a RenderScript Type.

  See <a href='http://developer.android.com/reference/android/graphics/ImageFormat.html'>android.graphics.ImageFormat</a> for a description of each format.
end:

type: rs_sampler_value
version: 16
enum:
value: RS_SAMPLER_NEAREST = 0
value: RS_SAMPLER_LINEAR = 1
value: RS_SAMPLER_LINEAR_MIP_LINEAR = 2
value: RS_SAMPLER_WRAP = 3
value: RS_SAMPLER_CLAMP = 4
value: RS_SAMPLER_LINEAR_MIP_NEAREST = 5
value: RS_SAMPLER_MIRRORED_REPEAT = 6
value: RS_SAMPLER_INVALID = 100
summary: Sampler wrap T value
description:
end:
