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
summary: Allocation Data Access Functions
description:
 The functions below can be used to get and set the cells that comprise
 an allocation.
 <ul>
 <li>Individual cells are accessed using the rsGetElementAt* and
   @rsSetElementAt functions.</li>
 <li>Multiple cells can be copied using the rsAllocationCopy* and
   rsAllocationV* functions.</li>
 <li>For getting values through a sampler, use @rsSample.</li>
 </ul>
 The @rsGetElementAt and rsSetElement* functions are somewhat misnamed.
 They don't get or set elements, which are akin to data types; they get
 or set cells.  Think of them as rsGetCellAt and and rsSetCellAt.
end:

function: rsAllocationCopy1DRange
version: 14
ret: void
arg: rs_allocation dstAlloc, "Allocation to copy cells into."
arg: uint32_t dstOff, "Offset in the destination of the first cell to be copied into."
arg: uint32_t dstMip, "Mip level in the destination allocation.  0 if mip mapping is not used."
arg: uint32_t count, "Number of cells to be copied."
arg: rs_allocation srcAlloc, "Source allocation."
arg: uint32_t srcOff, "Offset in the source of the first cell to be copied."
arg: uint32_t srcMip, "Mip level in the source allocation.  0 if mip mapping is not used."
summary: Copy consecutive cells between allocations
description:
 Copies the specified number of cells from one allocation to another.

 The two allocations must be different.  Using this function to copy whithin
 the same allocation yields undefined results.

 The function does not validate whether the offset plus count exceeds the size
 of either allocation.  Be careful!

 This function should only be called between 1D allocations.  Calling it
 on other allocations is undefined.

 This function should not be called from inside a kernel, or from any function
 that may be called directly or indirectly from a kernel. Doing so would cause a
 runtime error.
test: none
end:

function: rsAllocationCopy2DRange
version: 14
ret: void
arg: rs_allocation dstAlloc, "Allocation to copy cells into."
arg: uint32_t dstXoff, "X offset in the destination of the region to be set."
arg: uint32_t dstYoff, "Y offset in the destination of the region to be set."
arg: uint32_t dstMip, "Mip level in the destination allocation.  0 if mip mapping is not used."
arg: rs_allocation_cubemap_face dstFace, "Cubemap face of the destination allocation.  Ignored for allocations that aren't cubemaps."
arg: uint32_t width, "Width of the incoming region to update."
arg: uint32_t height, "Height of the incoming region to update."
arg: rs_allocation srcAlloc, "Source allocation."
arg: uint32_t srcXoff, "X offset in the source."
arg: uint32_t srcYoff, "Y offset in the source."
arg: uint32_t srcMip, "Mip level in the source allocation.  0 if mip mapping is not used."
arg: rs_allocation_cubemap_face srcFace, "Cubemap face of the source allocation.  Ignored for allocations that aren't cubemaps."
summary: Copy a rectangular region of cells between allocations
description:
 Copies a rectangular region of cells from one allocation to another.
 (width * heigth) cells are copied.

 The two allocations must be different.  Using this function to copy whithin
 the same allocation yields undefined results.

 The function does not validate whether the the source or destination region
 exceeds the size of its respective allocation.  Be careful!

 This function should only be called between 2D allocations.  Calling it
 on other allocations is undefined.

 This function should not be called from inside a kernel, or from any function
 that may be called directly or indirectly from a kernel. Doing so would cause a
 runtime error.
test: none
end:

function: rsAllocationVLoadX_#2#1
version: 22
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a, "Allocation to get the data from."
arg: uint32_t x, "X offset in the allocation of the first cell to be copied from."
summary: Get a vector from an allocation of scalars
description:
 This function returns a vector composed of successive cells of the allocation.
 It assumes that the allocation contains scalars.

 The "X" in the name indicates that successive values are extracted by
 increasing the X index.  There are currently no functions to get successive
 values incrementing other dimensions.  Use multiple calls to rsGetElementAt()
 instead.

 For example, when calling rsAllocationVLoadX_int4(a, 20, 30), an int4 composed
 of a[20, 30], a[21, 30], a[22, 30], and a[23, 30] is returned.

 When retrieving from a three dimensional allocations, use the x, y, z variant.
 Similarly, use the x, y variant for two dimensional allocations and x for the
 mono dimensional allocations.

 For efficiency, this function does not validate the inputs.  Trying to wrap
 the X index, exceeding the size of the allocation, or using indices incompatible
 with the dimensionality of the allocation yields undefined results.

 See also @rsAllocationVStoreX().
test: none
end:

function: rsAllocationVLoadX_#2#1
version: 22
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y, "Y offset in the allocation of the first cell to be copied from."
test: none
end:

function: rsAllocationVLoadX_#2#1
version: 22
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z, "Z offset in the allocation of the first cell to be copied from."
test: none
end:

function: rsAllocationVStoreX_#2#1
version: 22
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: void
arg: rs_allocation a, "Allocation to store the data into."
arg: #2#1 val, "Value to be stored."
arg: uint32_t x, "X offset in the allocation of the first cell to be copied into."
summary: Store a vector into an allocation of scalars
description:
 This function stores the entries of a vector into successive cells of an allocation.
 It assumes that the allocation contains scalars.

 The "X" in the name indicates that successive values are stored by increasing
 the X index.  There are currently no functions to store successive values
 incrementing other dimensions.  Use multiple calls to rsSetElementAt() instead.

 For example, when calling rsAllocationVStoreX_int3(a, v, 20, 30), v.x is stored
 at a[20, 30], v.y at a[21, 30], and v.z at a[22, 30].

 When storing into a three dimensional allocations, use the x, y, z variant.
 Similarly, use the x, y variant for two dimensional allocations and x for the
 mono dimensional allocations.

 For efficiency, this function does not validate the inputs.  Trying to wrap the
 X index, exceeding the size of the allocation, or using indices incompatible
 with the dimensionality of the allocation yiels undefined results.

 See also @rsAllocationVLoadX().
test: none
end:

function: rsAllocationVStoreX_#2#1
version: 22
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
arg: uint32_t y, "Y offset in the allocation of the first cell to be copied into."
test: none
end:

function: rsAllocationVStoreX_#2#1
version: 22
w: 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z, "Z offset in the allocation of the first cell to be copied into."
test: none
end:

function: rsGetElementAt
ret: const void*
arg: rs_allocation a
arg: uint32_t x
summary: Return a cell from an allocation
description:
 This function extracts a single cell from an allocation.

 When retrieving from a three dimensional allocations, use the x, y, z variant.
 Similarly, use the x, y variant for two dimensional allocations and x for the
 mono dimensional allocations.

 This function has two styles.  One returns the address of the value using a void*,
 the other returns the actual value, e.g. rsGetElementAt() vs. rsGetElementAt_int4().
 For primitive types, always use the latter as it is more efficient.
test: none
end:

function: rsGetElementAt
ret: const void*
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsGetElementAt
ret: const void*
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
test: none
end:

function: rsGetElementAt_#2#1
version: 9 17
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
inline:
 return ((#2#1 *)rsGetElementAt(a, x))[0];
test: none
end:

function: rsGetElementAt_#2#1
version: 9 17
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
inline:
 return ((#2#1 *)rsGetElementAt(a, x, y))[0];
test: none
end:

function: rsGetElementAt_#2#1
version: 9 17
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
inline:
 return ((#2#1 *)rsGetElementAt(a, x, y, z))[0];
test: none
end:

function: rsGetElementAt_#2#1
version: 18
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
test: none
end:

function: rsGetElementAt_#2#1
version: 18
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsGetElementAt_#2#1
version: 18
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
test: none
end:

function: rsGetElementAt_#2#1
version: 23
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
test: none
end:

function: rsGetElementAt_#2#1
version: 23
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsGetElementAt_#2#1
version: 23
w: 1, 2, 3, 4
t: f16
ret: #2#1
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
test: none
end:

function: rsGetElementAtYuv_uchar_U
version: 18
ret: uchar
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
summary: Get the U component of an allocation of YUVs
description:
 Extracts the U component of a single YUV value from a 2D allocation of YUVs.

 Inside an allocation, Y, U, and V components may be stored if different planes
 and at different resolutions.  The x, y coordinates provided here are in the
 dimensions of the Y plane.

 See @rsGetElementAtYuv_uchar_Y().
test: none
end:

function: rsGetElementAtYuv_uchar_V
version: 18
ret: uchar
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
summary: Get the V component of an allocation of YUVs
description:
 Extracts the V component of a single YUV value from a 2D allocation of YUVs.

 Inside an allocation, Y, U, and V components may be stored if different planes
 and at different resolutions.  The x, y coordinates provided here are in the
 dimensions of the Y plane.

 See @rsGetElementAtYuv_uchar_Y().
test: none
end:

function: rsGetElementAtYuv_uchar_Y
version: 18
ret: uchar
arg: rs_allocation a
arg: uint32_t x
arg: uint32_t y
summary: Get the Y component of an allocation of YUVs
description:
 Extracts the Y component of a single YUV value from a 2D allocation of YUVs.

 Inside an allocation, Y, U, and V components may be stored if different planes
 and at different resolutions.  The x, y coordinates provided here are in the
 dimensions of the Y plane.

 See @rsGetElementAtYuv_uchar_U() and @rsGetElementAtYuv_uchar_V().
test: none
end:

function: rsSample
version: 16
ret: float4
arg: rs_allocation a, "Allocation to sample from."
arg: rs_sampler s, "Sampler state."
arg: float location, "Location to sample from."
summary: Sample a value from a texture allocation
description:
 Fetches a value from a texture allocation in a way described by the sampler.

 If your allocation is 1D, use the variant with float for location.  For 2D,
 use the float2 variant.

 See <a href='http://developer.android.com/reference/android/renderscript/Sampler.html'>android.renderscript.Sampler</a> for more details.
test: none
end:

function: rsSample
version: 16
ret: float4
arg: rs_allocation a
arg: rs_sampler s
arg: float location
arg: float lod, "Mip level to sample from, for fractional values mip levels will be interpolated if RS_SAMPLER_LINEAR_MIP_LINEAR is used."
test: none
end:

function: rsSample
version: 16
ret: float4
arg: rs_allocation a
arg: rs_sampler s
arg: float2 location
test: none
end:

function: rsSample
version: 16
ret: float4
arg: rs_allocation a
arg: rs_sampler s
arg: float2 location
arg: float lod
test: none
end:

function: rsSetElementAt
version: 18
ret: void
arg: rs_allocation a
arg: void* ptr
arg: uint32_t x
summary: Set a cell of an allocation
description:
 This function stores a value into a single cell of an allocation.

 When storing into a three dimensional allocations, use the x, y, z variant.
 Similarly, use the x, y variant for two dimensional allocations and x for
 the mono dimensional allocations.

 This function has two styles.  One passes the value to be stored using a void*,
 the other has the actual value as an argument, e.g. rsSetElementAt() vs.
 rsSetElementAt_int4().  For primitive types, always use the latter as it is
 more efficient.

 See also @rsGetElementAt().
test: none
end:

function: rsSetElementAt
version: 18
ret: void
arg: rs_allocation a
arg: void* ptr
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsSetElementAt_#2#1
version: 18
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
test: none
end:

function: rsSetElementAt_#2#1
version: 18
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsSetElementAt_#2#1
version: 18
w: 1, 2, 3, 4
t: u8, u16, u32, u64, i8, i16, i32, i64, f32, f64
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
test: none
end:

function: rsSetElementAt_#2#1
version: 23
w: 1, 2, 3, 4
t: f16
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
test: none
end:

function: rsSetElementAt_#2#1
version: 23
w: 1, 2, 3, 4
t: f16
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
arg: uint32_t y
test: none
end:

function: rsSetElementAt_#2#1
version: 23
w: 1, 2, 3, 4
t: f16
ret: void
arg: rs_allocation a
arg: #2#1 val
arg: uint32_t x
arg: uint32_t y
arg: uint32_t z
test: none
end:
