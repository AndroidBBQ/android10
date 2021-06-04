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
summary: Object Characteristics Functions
description:
 The functions below can be used to query the characteristics of an Allocation, Element,
 or Sampler object.  These objects are created from Java.  You can't create them from a
 script.

 <h5>Allocations:</h5>

 Allocations are the primary method used to pass data to and from RenderScript kernels.

 They are a structured collection of cells that can be used to store bitmaps, textures,
 arbitrary data points, etc.

 This collection of cells may have many dimensions (X, Y, Z, Array0, Array1, Array2, Array3),
 faces (for cubemaps), and level of details (for mipmapping).

 See the <a href='http://developer.android.com/reference/android/renderscript/Allocation.html'>android.renderscript.Allocation</a> for details on to create Allocations.

 <h5>Elements:</h5>

 The term "element" is used a bit ambiguously in RenderScript, as both type information
 for the cells of an Allocation and the instantiation of that type.  For example:<ul>
 <li>@rs_element is a handle to a type specification, and</li>
 <li>In functions like @rsGetElementAt(), "element" means the instantiation of the type,
     i.e. a cell of an Allocation.</li></ul>

 The functions below let you query the characteristics of the type specificiation.

 An Element can specify a simple data types as found in C, e.g. an integer, float, or
 boolean.  It can also specify a handle to a RenderScript object.  See @rs_data_type for
 a list of basic types.

 Elements can specify fixed size vector (of size 2, 3, or 4) versions of the basic types.
 Elements can be grouped together into complex Elements, creating the equivalent of
 C structure definitions.

 Elements can also have a kind, which is semantic information used to interpret pixel
 data.  See @rs_data_kind.

 When creating Allocations of common elements, you can simply use one of the many predefined
 Elements like <a href='http://developer.android.com/reference/android/renderscript/Element.html#F32_2(android.renderscript.RenderScript)'>F32_2</a>.

 To create complex Elements, use the <a href='http://developer.android.com/reference/android/renderscript/Element.Builder.html'>Element.Builder</a> Java class.

 <h5>Samplers:</h5>

 Samplers objects define how Allocations can be read as structure within a kernel.
 See <a href='http://developer.android.com/reference/android/renderscript/Sampler.html'>android.renderscript.S</a>.
end:

function: rsAllocationGetDimFaces
ret: uint32_t, "Returns 1 if more than one face is present, 0 otherwise."
arg: rs_allocation a
summary: Presence of more than one face
description:
 If the Allocation is a cubemap, this function returns 1 if there's more than one face
 present.  In all other cases, it returns 0.

 Use @rsGetDimHasFaces() to get the dimension of a currently running kernel.
test: none
end:

function: rsAllocationGetDimLOD
ret: uint32_t, "Returns 1 if more than one LOD is present, 0 otherwise."
arg: rs_allocation a
summary: Presence of levels of detail
description:
 Query an Allocation for the presence of more than one Level Of Detail.  This is useful
 for mipmaps.

 Use @rsGetDimLod() to get the dimension of a currently running kernel.
test: none
end:

function: rsAllocationGetDimX
ret: uint32_t, "X dimension of the Allocation."
arg: rs_allocation a
summary: Size of the X dimension
description:
 Returns the size of the X dimension of the Allocation.

 Use @rsGetDimX() to get the dimension of a currently running kernel.
test: none
end:

function: rsAllocationGetDimY
ret: uint32_t, "Y dimension of the Allocation."
arg: rs_allocation a
summary: Size of the Y dimension
description:
 Returns the size of the Y dimension of the Allocation.  If the Allocation has less
 than two dimensions, returns 0.

 Use @rsGetDimY() to get the dimension of a currently running kernel.
test: none
end:

function: rsAllocationGetDimZ
ret: uint32_t, "Z dimension of the Allocation."
arg: rs_allocation a
summary: Size of the Z dimension
description:
 Returns the size of the Z dimension of the Allocation.  If the Allocation has less
 than three dimensions, returns 0.

 Use @rsGetDimZ() to get the dimension of a currently running kernel.
test: none
end:

function: rsAllocationGetElement
ret: rs_element, "Element describing Allocation layout."
arg: rs_allocation a, "Allocation to get data from."
summary: Get the object that describes the cell of an Allocation
description:
 Get the Element object describing the type, kind, and other characteristics of a cell
 of an Allocation.  See the rsElement* functions below.
test: none
end:

function: rsClearObject
t: rs_element, rs_type, rs_allocation, rs_sampler, rs_script
ret: void
arg: #1* dst
summary: Release an object
description:
 Tells the run time that this handle will no longer be used to access the the related
 object.  If this was the last handle to that object, resource recovery may happen.

 After calling this function, *dst will be set to an empty handle.  See @rsIsObject().
test: none
end:

function: rsIsObject
t: rs_element, rs_type, rs_allocation, rs_sampler, rs_script
ret: bool
arg: #1 v
summary: Check for an empty handle
description:
 Returns true if the handle contains a non-null reference.

 This function does not validate that the internal pointer used in the handle
 points to an actual valid object; it only checks for null.

 This function can be used to check the Element returned by @rsElementGetSubElement()
 or see if @rsClearObject() has been called on a handle.
test: none
end:

function: rsElementGetBytesSize
version: 16
ret: uint32_t
arg: rs_element e
summary: Size of an Element
description:
 Returns the size in bytes that an instantiation of this Element will occupy.
test: none
end:

function: rsElementGetDataKind
version: 16
ret: rs_data_kind
arg: rs_element e
summary: Kind of an Element
description:
 Returns the Element's data kind.  This is used to interpret pixel data.

 See @rs_data_kind.
test: none
end:

function: rsElementGetDataType
version: 16
ret: rs_data_type
arg: rs_element e
summary: Data type of an Element
description:
 Returns the Element's base data type.  This can be a type similar to C/C++ (e.g.
 RS_TYPE_UNSIGNED_8), a handle (e.g. RS_TYPE_ALLOCATION and RS_TYPE_ELEMENT), or a
 more complex numerical type (e.g. RS_TYPE_UNSIGNED_5_6_5 and RS_TYPE_MATRIX_4X4).
 See @rs_data_type.

 If the Element describes a vector, this function returns the data type of one of its items.
 Use @rsElementGetVectorSize to get the size of the vector.

 If the Element describes a structure, RS_TYPE_NONE is returned.  Use the rsElementGetSub*
 functions to explore this complex Element.
test: none
end:

function: rsElementGetSubElement
version: 16
ret: rs_element, "Sub-element at the given index."
arg: rs_element e, "Element to query."
arg: uint32_t index, "Index of the sub-element to return."
summary: Sub-element of a complex Element
description:
 For Elements that represents a structure, this function returns the sub-element at the
 specified index.

 If the Element is not a structure or the index is greater or equal to the number of
 sub-elements, an invalid handle is returned.
test: none
end:

function: rsElementGetSubElementArraySize
version: 16
ret: uint32_t, "Array size of the sub-element."
arg: rs_element e, "Element to query."
arg: uint32_t index, "Index of the sub-element."
summary: Array size of a sub-element of a complex Element
description:
 For complex Elements, sub-elements can be statically sized arrays.  This function
 returns the array size of the sub-element at the index.  This sub-element repetition
 is different than fixed size vectors.
test: none
end:

function: rsElementGetSubElementCount
version: 16
ret: uint32_t, "Number of sub-elements."
arg: rs_element e, "Element to get data from."
summary: Number of sub-elements
description:
 Elements can be simple, such as an int or a float, or a structure with multiple
 sub-elements.  This function returns zero for simple Elements and the number of
 sub-elements for complex Elements.
test: none
end:

function: rsElementGetSubElementName
version: 16
ret: uint32_t, "Number of characters copied, excluding the null terminator."
arg: rs_element e, "Element to get data from."
arg: uint32_t index, "Index of the sub-element."
arg: char* name, "Address of the array to store the name into."
arg: uint32_t nameLength, "Length of the provided name array."
summary: Name of a sub-element
description:
 For complex Elements, this function returns the name of the sub-element at the
 specified index.
test: none
end:

function: rsElementGetSubElementNameLength
version: 16
ret: uint32_t, "Length of the sub-element name including the null terminator."
arg: rs_element e, "Element to get data from."
arg: uint32_t index, "Index of the sub-element."
summary: Length of the name of a sub-element
description:
 For complex Elements, this function returns the length of the name of the sub-element
 at the specified index.
test: none
end:

function: rsElementGetSubElementOffsetBytes
version: 16
ret: uint32_t, "Offset in bytes."
arg: rs_element e, "Element to get data from."
arg: uint32_t index, "Index of the sub-element."
summary: Offset of the instantiated sub-element
description:
 This function returns the relative position of the instantiation of the specified
 sub-element within the instantiation of the Element.

 For example, if the Element describes a 32 bit float followed by a 32 bit integer,
 the offset return for the first will be 0 and the second 4.
test: none
end:

function: rsElementGetVectorSize
version: 16
ret: uint32_t, "Length of the element vector."
arg: rs_element e, "Element to get data from."
summary: Vector size of the Element
description:
 Returns the Element's vector size.  If the Element does not represent a vector,
 1 is returned.
test: none
end:

function: rsGetAllocation
ret: rs_allocation
arg: const void* p
deprecated: 22, This function is deprecated and will be removed from the SDK in a future release.
summary: Return the Allocation for a given pointer
description:
 Returns the Allocation for a given pointer.  The pointer should point within a valid
 allocation.  The results are undefined if the pointer is not from a valid Allocation.
test: none
end:

function: rsSamplerGetAnisotropy
version: 16
ret: float
arg: rs_sampler s
summary: Anisotropy of the Sampler
description:
 Get the Sampler's anisotropy.

 See <a href='http://developer.android.com/reference/android/renderscript/Sampler.html'>android.renderscript.S</a>.
test: none
end:

function: rsSamplerGetMagnification
version: 16
ret: rs_sampler_value
arg: rs_sampler s
summary: Sampler magnification value
description:
 Get the Sampler's magnification value.

 See <a href='http://developer.android.com/reference/android/renderscript/Sampler.html'>android.renderscript.S</a>.
test: none
end:

function: rsSamplerGetMinification
version: 16
ret: rs_sampler_value
arg: rs_sampler s
summary: Sampler minification value
description:
 Get the Sampler's minification value.

 See <a href='http://developer.android.com/reference/android/renderscript/Sampler.html'>android.renderscript.S</a>.
test: none
end:

function: rsSamplerGetWrapS
version: 16
ret: rs_sampler_value
arg: rs_sampler s
summary: Sampler wrap S value
description:
 Get the Sampler's wrap S value.

 See <a href='http://developer.android.com/reference/android/renderscript/Sampler.html'>android.renderscript.S</a>.
test: none
end:

function: rsSamplerGetWrapT
version: 16
ret: rs_sampler_value
arg: rs_sampler s
summary: Sampler wrap T value
description:
 Get the sampler's wrap T value.

 See <a href='http://developer.android.com/reference/android/renderscript/Sampler.html'>android.renderscript.S</a>.
test: none
end:

function: rsSetObject
t: rs_element, rs_type, rs_allocation, rs_sampler, rs_script
ret: void
arg: #1* dst
arg: #1 src
hidden:
summary: For internal use.
description:
test: none
end:
