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
summary: Kernel Invocation Functions and Types
description:
 The @rsForEach() function can be used to invoke the root kernel of a script.

 The other functions are used to get the characteristics of the invocation of
 an executing kernel, like dimensions and current indices.  These functions take
 a @rs_kernel_context as argument.
end:

type: rs_for_each_strategy_t
enum: rs_for_each_strategy
value: RS_FOR_EACH_STRATEGY_SERIAL = 0, "Prefer contiguous memory regions."
value: RS_FOR_EACH_STRATEGY_DONT_CARE = 1, "No prefrences."
#TODO explain this better
value: RS_FOR_EACH_STRATEGY_DST_LINEAR = 2, "Prefer DST."
value: RS_FOR_EACH_STRATEGY_TILE_SMALL = 3, "Prefer processing small rectangular regions."
value: RS_FOR_EACH_STRATEGY_TILE_MEDIUM = 4, "Prefer processing medium rectangular regions."
value: RS_FOR_EACH_STRATEGY_TILE_LARGE = 5, "Prefer processing large rectangular regions."
summary: Suggested cell processing order
description:
 This type is used to suggest how the invoked kernel should iterate over the cells of the
 allocations.  This is a hint only.  Implementations may not follow the suggestion.

 This specification can help the caching behavior of the running kernel, e.g. the cache
 locality when the processing is distributed over multiple cores.
end:

type: rs_kernel_context
version: 23
simple: const struct rs_kernel_context_t *
summary: Handle to a kernel invocation context
description:
 The kernel context contains common characteristics of the allocations being iterated
 over, like dimensions.  It also contains rarely used indices of the currently processed
 cell, like the Array0 index or the current level of detail.

 You can access the kernel context by adding a special parameter named "context" of type
 rs_kernel_context to your kernel function.  See @rsGetDimX() and @rsGetArray0() for examples.
end:

type: rs_script_call_t
struct: rs_script_call
field: rs_for_each_strategy_t strategy, "Currently ignored.  In the future, will be suggested cell iteration strategy."
field: uint32_t xStart, "Starting index in the X dimension."
field: uint32_t xEnd, "Ending index (exclusive) in the X dimension."
field: uint32_t yStart, "Starting index in the Y dimension."
field: uint32_t yEnd, "Ending index (exclusive) in the Y dimension."
field: uint32_t zStart, "Starting index in the Z dimension."
field: uint32_t zEnd, "Ending index (exclusive) in the Z dimension."
field: uint32_t arrayStart, "Starting index in the Array0 dimension."
field: uint32_t arrayEnd, "Ending index (exclusive) in the Array0 dimension."
field: uint32_t array1Start, "Starting index in the Array1 dimension."
field: uint32_t array1End, "Ending index (exclusive) in the Array1 dimension."
field: uint32_t array2Start, "Starting index in the Array2 dimension."
field: uint32_t array2End, "Ending index (exclusive) in the Array2 dimension."
field: uint32_t array3Start, "Starting index in the Array3 dimension."
field: uint32_t array3End, "Ending index (exclusive) in the Array3 dimension."
summary: Cell iteration information
description:
 This structure is used to provide iteration information to a rsForEach call.
 It is currently used to restrict processing to a subset of cells.  In future
 versions, it will also be used to provide hint on how to best iterate over
 the cells.

 The Start fields are inclusive and the End fields are exclusive.  E.g. to iterate
 over cells 4, 5, 6, and 7 in the X dimension, set xStart to 4 and xEnd to 8.
end:

type: rs_kernel
version: 24
simple: void*
summary: Handle to a kernel function
description:
  An opaque type for a function that is defined with the kernel attribute.  A value
  of this type can be used in a @rsForEach call to launch a kernel.
end:

function: rsForEach
version: 9 13
ret: void
arg: rs_script script, "Script to call."
arg: rs_allocation input, "Allocation to source data from."
arg: rs_allocation output, "Allocation to write date into."
arg: const void* usrData, "User defined data to pass to the script.  May be NULL."
arg: const rs_script_call_t* sc, "Extra control information used to select a sub-region of the allocation to be processed or suggest a walking strategy.  May be NULL."
summary: Launches a kernel
description:
 Runs the kernel over zero or more input allocations. They are passed after the
 @rs_kernel argument. If the specified kernel returns a value, an output allocation
 must be specified as the last argument. All input allocations,
 and the output allocation if it exists, must have the same dimensions.

 This is a synchronous function. A call to this function only returns after all
 the work has completed for all cells of the input allocations. If the kernel
 function returns any value, the call waits until all results have been written
 to the output allocation.

 Up to API level 23, the kernel is implicitly specified as the kernel named
 "root" in the specified script, and only a single input allocation can be used.
 Starting in API level 24, an arbitrary kernel function can be used,
 as specified by the kernel argument. The script argument is removed.
 The kernel must be defined in the current script. In addition, more than one
 input can be used.

 E.g.<code><br/>
 float __attribute__((kernel)) square(float a) {<br/>
 &nbsp;&nbsp;return a * a;<br/>
 }<br/>
 <br/>
 void compute(rs_allocation ain, rs_allocation aout) {<br/>
 &nbsp;&nbsp;rsForEach(square, ain, aout);<br/>
 }<br/>
 <br/></code>
test: none
end:

function: rsForEach
version: 9 13
ret: void
arg: rs_script script
arg: rs_allocation input
arg: rs_allocation output
arg: const void* usrData
test: none
end:

function: rsForEach
version: 14 20
ret: void
arg: rs_script script
arg: rs_allocation input
arg: rs_allocation output
arg: const void* usrData
arg: size_t usrDataLen, "Size of the userData structure.  This will be used to perform a shallow copy of the data if necessary."
arg: const rs_script_call_t* sc
test: none
end:

function: rsForEach
version: 14 20
ret: void
arg: rs_script script
arg: rs_allocation input
arg: rs_allocation output
arg: const void* usrData
arg: size_t usrDataLen
test: none
end:

function: rsForEach
version: 14 23
ret: void
arg: rs_script script
arg: rs_allocation input
arg: rs_allocation output
test: none
end:

function: rsForEach
version: 24
intrinsic: true
# Not overloadable
attrib: =
ret: void
arg: rs_kernel kernel, "Function designator to a function that is defined with the kernel attribute."
arg: ..., "Input and output allocations"
test: none
end:

function: rsForEachWithOptions
version: 24
intrinsic: true
# Not overloadable
attrib: =
ret: void
arg: rs_kernel kernel, "Function designator to a function that is defined with the kernel attribute."
arg: rs_script_call_t* options, "Launch options"
arg: ..., "Input and output allocations"
summary: Launches a kernel with options
description:
 Launches kernel in a way similar to @rsForEach. However, instead of processing
 all cells in the input, this function only processes cells in the subspace of
 the index space specified in options. With the index space explicitly specified
 by options, no input or output allocation is required for a kernel launch using
 this API. If allocations are passed in, they must match the number of arguments
 and return value expected by the kernel function. The output allocation is
 present if and only if the kernel has a non-void return value.

 E.g.,<code><br/>
    rs_script_call_t opts = {0};<br/>
    opts.xStart = 0;<br/>
    opts.xEnd = dimX;<br/>
    opts.yStart = 0;<br/>
    opts.yEnd = dimY / 2;<br/>
    rsForEachWithOptions(foo, &opts, out, out);<br/>
 </code>
test: none
end:

function: rsForEachInternal
version: 24
internal: true
ret: void
arg: int slot
arg: rs_script_call_t* options
arg: int hasOutput, "Indicates whether the kernel generates output"
arg: int numInputs, "Number of input allocations"
arg: rs_allocation* allocs, "Input and output allocations"
summary: (Internal API) Launch a kernel in the current Script (with the slot number)
description:
 Internal API to launch a kernel.
test: none
end:

function: rsGetArray0
version: 23
ret: uint32_t
arg: rs_kernel_context context
summary: Index in the Array0 dimension for the specified kernel context
description:
 Returns the index in the Array0 dimension of the cell being processed, as specified
 by the supplied kernel context.

 The kernel context contains common characteristics of the allocations being iterated
 over and rarely used indices, like the Array0 index.

 You can access the kernel context by adding a special parameter named "context" of
 type rs_kernel_context to your kernel function.  E.g.<br/>
 <code>short RS_KERNEL myKernel(short value, uint32_t x, rs_kernel_context context) {<br/>
 &nbsp;&nbsp;// The current index in the common x, y, z dimensions are accessed by<br/>
 &nbsp;&nbsp;// adding these variables as arguments.  For the more rarely used indices<br/>
 &nbsp;&nbsp;// to the other dimensions, extract them from the kernel context:<br/>
 &nbsp;&nbsp;uint32_t index_a0 = rsGetArray0(context);<br/>
 &nbsp;&nbsp;//...<br/>
 }<br/></code>

 This function returns 0 if the Array0 dimension is not present.
test: none
end:

function: rsGetArray1
version: 23
ret: uint32_t
arg: rs_kernel_context context
summary: Index in the Array1 dimension for the specified kernel context
description:
 Returns the index in the Array1 dimension of the cell being processed, as specified
 by the supplied kernel context.  See @rsGetArray0() for an explanation of the context.

 Returns 0 if the Array1 dimension is not present.
test: none
end:

function: rsGetArray2
version: 23
ret: uint32_t
arg: rs_kernel_context context
summary: Index in the Array2 dimension for the specified kernel context
description:
 Returns the index in the Array2 dimension of the cell being processed,
 as specified by the supplied kernel context.  See @rsGetArray0() for an explanation
 of the context.

 Returns 0 if the Array2 dimension is not present.
test: none
end:

function: rsGetArray3
version: 23
ret: uint32_t
arg: rs_kernel_context context
summary: Index in the Array3 dimension for the specified kernel context
description:
 Returns the index in the Array3 dimension of the cell being processed, as specified
 by the supplied kernel context.  See @rsGetArray0() for an explanation of the context.

 Returns 0 if the Array3 dimension is not present.
test: none
end:

function: rsGetDimArray0
version: 23
ret: uint32_t
arg: rs_kernel_context context
summary: Size of the Array0 dimension for the specified kernel context
description:
 Returns the size of the Array0 dimension for the specified kernel context.
 See @rsGetDimX() for an explanation of the context.

 Returns 0 if the Array0 dimension is not present.
#TODO Add an hyperlink to something that explains Array0/1/2/3
# for the relevant functions.
test: none
end:

function: rsGetDimArray1
version: 23
ret: uint32_t
arg: rs_kernel_context context
summary: Size of the Array1 dimension for the specified kernel context
description:
 Returns the size of the Array1 dimension for the specified kernel context.
 See @rsGetDimX() for an explanation of the context.

 Returns 0 if the Array1 dimension is not present.
test: none
end:

function: rsGetDimArray2
version: 23
ret: uint32_t
arg: rs_kernel_context context
summary: Size of the Array2 dimension for the specified kernel context
description:
 Returns the size of the Array2 dimension for the specified kernel context.
 See @rsGetDimX() for an explanation of the context.

 Returns 0 if the Array2 dimension is not present.
test: none
end:

function: rsGetDimArray3
version: 23
ret: uint32_t
arg: rs_kernel_context context
summary: Size of the Array3 dimension for the specified kernel context
description:
 Returns the size of the Array3 dimension for the specified kernel context.
 See @rsGetDimX() for an explanation of the context.

 Returns 0 if the Array3 dimension is not present.
test: none
end:

function: rsGetDimHasFaces
version: 23
ret: bool, "Returns true if more than one face is present, false otherwise."
arg: rs_kernel_context context
summary: Presence of more than one face for the specified kernel context
description:
 If the kernel is iterating over a cubemap, this function returns true if there's more
 than one face present.  In all other cases, it returns false.  See @rsGetDimX() for an
 explanation of the context.

 @rsAllocationGetDimFaces() is similar but returns 0 or 1 instead of a bool.
test: none
end:

function: rsGetDimLod
version: 23
ret: uint32_t
arg: rs_kernel_context context
summary: Number of levels of detail for the specified kernel context
description:
 Returns the number of levels of detail for the specified kernel context.  This is useful
 for mipmaps.  See @rsGetDimX() for an explanation of the context.

 Returns 0 if Level of Detail is not used.

 @rsAllocationGetDimLOD() is similar but returns 0 or 1 instead the actual
 number of levels.
test: none
end:

function: rsGetDimX
version: 23
ret: uint32_t
arg: rs_kernel_context context
summary: Size of the X dimension for the specified kernel context
description:
 Returns the size of the X dimension for the specified kernel context.

 The kernel context contains common characteristics of the allocations being iterated
 over and rarely used indices, like the Array0 index.

 You can access it by adding a special parameter named "context" of
 type rs_kernel_context to your kernel function.  E.g.<br/>
 <code>int4 RS_KERNEL myKernel(int4 value, rs_kernel_context context) {<br/>
 &nbsp;&nbsp;uint32_t size = rsGetDimX(context); //...<br/></code>

 To get the dimension of specific allocation, use @rsAllocationGetDimX().
test: none
end:

function: rsGetDimY
version: 23
ret: uint32_t
arg: rs_kernel_context context
summary: Size of the Y dimension for the specified kernel context
description:
 Returns the size of the X dimension for the specified kernel context.
 See @rsGetDimX() for an explanation of the context.

 Returns 0 if the Y dimension is not present.

 To get the dimension of specific allocation, use @rsAllocationGetDimY().
test: none
end:

function: rsGetDimZ
version: 23
ret: uint32_t
arg: rs_kernel_context context
summary: Size of the Z dimension for the specified kernel context
description:
 Returns the size of the Z dimension for the specified kernel context.
 See @rsGetDimX() for an explanation of the context.

 Returns 0 if the Z dimension is not present.

 To get the dimension of specific allocation, use @rsAllocationGetDimZ().
test: none
end:

function: rsGetFace
version: 23
ret: rs_allocation_cubemap_face
arg: rs_kernel_context context
summary: Coordinate of the Face for the specified kernel context
description:
 Returns the face on which the cell being processed is found, as specified by the
 supplied kernel context.  See @rsGetArray0() for an explanation of the context.

 Returns RS_ALLOCATION_CUBEMAP_FACE_POSITIVE_X if the face dimension is not
 present.
test: none
end:

function: rsGetLod
version: 23
ret: uint32_t
arg: rs_kernel_context context
summary: Index in the Levels of Detail dimension for the specified kernel context
description:
 Returns the index in the Levels of Detail dimension of the cell being processed,
 as specified by the supplied kernel context.  See @rsGetArray0() for an explanation of
 the context.

 Returns 0 if the Levels of Detail dimension is not present.
test: none
end:
