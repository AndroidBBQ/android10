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
summary: Allocation Creation Functions
description:
 The functions below can be used to create Allocations from a Script.

 These functions can be called directly or indirectly from an invokable
 function.  If some control-flow path can result in a call to these functions
 from a RenderScript kernel function, a compiler error will be generated.
end:

function: rsCreateElement
version: 24
ret: rs_element
arg: rs_data_type data_type, "Data type of the Element"
summary: Creates an rs_element object of the specified data type
description:
  Creates an rs_element object of the specified data type.  The data kind of
  the Element will be set to RS_KIND_USER and vector_width will be set to 1,
  indicating non-vector.
test: none
end:

function: rsCreateVectorElement
version: 24
ret: rs_element
arg: rs_data_type data_type, "Data type of the Element"
arg: uint32_t vector_width, "Vector width (either 2, 3, or 4)"
summary: Creates an rs_element object of the specified data type and vector width
description:
  Creates an rs_element object of the specified data type and vector width.
  Value of vector_width must be 2, 3 or 4.  The data kind of the Element will
  be set to RS_KIND_USER.
test: none
end:

function: rsCreatePixelElement
version: 24
ret: rs_element
arg: rs_data_type data_type, "Data type of the Element"
arg: rs_data_kind data_kind, "Data kind of the Element"
summary: Creates an rs_element object of the specified data type and data kind
description:
  Creates an rs_element object of the specified data type and data kind.  The
  vector_width of the Element will be set to 1, indicating non-vector.
test: none
end:

function: rsCreateElement
version: 24
internal: true
ret: rs_element
arg: int32_t data_type
arg: int32_t data_kind
arg: bool isNormalized
arg: uint32_t vecSize
test: none
end:

function: rsCreateType
version: 24
ret: rs_type
arg: rs_element element, "Element to be associated with the Type"
arg: uint32_t dimX, "Size along the X dimension"
arg: uint32_t dimY, "Size along the Y dimension"
arg: uint32_t dimZ, "Size along the Z dimension"
arg: bool mipmaps, "Flag indicating if the Type has a mipmap chain"
arg: bool faces, "Flag indicating if the Type is a cubemap"
arg: rs_yuv_format yuv_format, "YUV layout for the Type"
summary: Creates an rs_type object with the specified Element and shape attributes
description:
  Creates an rs_type object with the specified Element and shape attributes.

  dimX specifies the size of the X dimension.

  dimY, if present and non-zero, indicates that the Y dimension is present and
  indicates its size.

  dimZ, if present and non-zero, indicates that the Z dimension is present and
  indicates its size.

  mipmaps indicates the presence of level of detail (LOD).

  faces indicates the  presence of cubemap faces.

  yuv_format indicates the associated YUV format (or RS_YUV_NONE).
test: none
end:

function: rsCreateType
version: 24
ret: rs_type
arg: rs_element element
arg: uint32_t dimX
arg: uint32_t dimY
arg: uint32_t dimZ
test:none
end:

function: rsCreateType
version: 24
ret: rs_type
arg: rs_element element
arg: uint32_t dimX
arg: uint32_t dimY
test:none
end:

function: rsCreateType
version: 24
ret: rs_type
arg: rs_element element
arg: uint32_t dimX
test:none
end:

function: rsCreateAllocation
version: 24
ret: rs_allocation
arg: rs_type type, "Type of the Allocation"
arg: uint32_t usage, "Usage flag for the allocation"
summary: Create an rs_allocation object of given Type.
description:
  Creates an rs_allocation object of the given Type and usage.

  RS_ALLOCATION_USAGE_SCRIPT and RS_ALLOCATION_USAGE_GRAPHICS_TEXTURE are the
  only supported usage flags for Allocations created from within a RenderScript
  Script.

  You can also use rsCreateAllocation_<type><width> wrapper functions to directly
  create Allocations of scalar and vector numerical types without creating
  intermediate rs_element or rs_type objects.

  E.g. rsCreateAllocation_int4() returns an Allocation of int4 data type of
  specified dimensions.
test: none
end:

function: rsCreateAllocation
version: 24
ret: rs_allocation
arg: rs_type type
test: none
end:

function: rsCreateAllocation
version: 24
internal: true
ret: rs_allocation
arg: rs_type type
arg: rs_allocation_mipmap_control mipmap
arg: uint32_t usages
arg: void* ptr
test: none
end:

function: rsCreateAllocation_#1
version: 24
t: u8, u16, u32, u64, i8, i16, i32, i64, f16, f32, f64
ret: rs_allocation
arg: uint32_t dimX
arg: uint32_t dimY
arg: uint32_t dimZ
inline:
  rs_element e = rsCreateElement(RS_TYPE_#RST_1);
  rs_type t = rsCreateType(e, dimX, dimY, dimZ);
  return rsCreateAllocation(t);
test: none
end:

function: rsCreateAllocation_#2#1
version: 24
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f16, f32, f64
ret: rs_allocation
arg: uint32_t dimX
arg: uint32_t dimY
arg: uint32_t dimZ
inline:
  rs_element e = rsCreateVectorElement(RS_TYPE_#RST_2, #1);
  rs_type t = rsCreateType(e, dimX, dimY, dimZ);
  return rsCreateAllocation(t);
test: none
end:

function: rsCreateAllocation_#1
version: 24
t: u8, u16, u32, u64, i8, i16, i32, i64, f16, f32, f64
ret: rs_allocation
arg: uint32_t dimX
arg: uint32_t dimY
inline:
  rs_element e = rsCreateElement(RS_TYPE_#RST_1);
  rs_type t = rsCreateType(e, dimX, dimY);
  return rsCreateAllocation(t);
test: none
end:

function: rsCreateAllocation_#2#1
version: 24
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f16, f32, f64
ret: rs_allocation
arg: uint32_t dimX
arg: uint32_t dimY
inline:
  rs_element e = rsCreateVectorElement(RS_TYPE_#RST_2, #1);
  rs_type t = rsCreateType(e, dimX, dimY);
  return rsCreateAllocation(t);
test: none
end:

function: rsCreateAllocation_#1
version: 24
t: u8, u16, u32, u64, i8, i16, i32, i64, f16, f32, f64
ret: rs_allocation
arg: uint32_t dimX
inline:
  rs_element e = rsCreateElement(RS_TYPE_#RST_1);
  rs_type t = rsCreateType(e, dimX);
  return rsCreateAllocation(t);
test: none
end:

function: rsCreateAllocation_#2#1
version: 24
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f16, f32, f64
ret: rs_allocation
arg: uint32_t dimX
inline:
  rs_element e = rsCreateVectorElement(RS_TYPE_#RST_2, #1);
  rs_type t = rsCreateType(e, dimX);
  return rsCreateAllocation(t);
test: none
end:
