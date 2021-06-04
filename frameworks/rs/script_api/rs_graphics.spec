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
summary: Graphics Functions and Types
description:
 The graphics subsystem of RenderScript was removed at API level 23.
include:
 #ifdef __LP64__
 // TODO We need to fix some of the builds before enabling this error:
 // #error "RenderScript graphics is deprecated and not supported in 64bit mode."
 #endif

 // TODO we seem to assume order for the other headers too.
 #include "rs_object_types.rsh"
end:

type: rs_blend_src_func
version: 16 22
size: 32
enum:
value: RS_BLEND_SRC_ZERO = 0
value: RS_BLEND_SRC_ONE = 1
value: RS_BLEND_SRC_DST_COLOR = 2
value: RS_BLEND_SRC_ONE_MINUS_DST_COLOR = 3
value: RS_BLEND_SRC_SRC_ALPHA = 4
value: RS_BLEND_SRC_ONE_MINUS_SRC_ALPHA = 5
value: RS_BLEND_SRC_DST_ALPHA = 6
value: RS_BLEND_SRC_ONE_MINUS_DST_ALPHA = 7
value: RS_BLEND_SRC_SRC_ALPHA_SATURATE = 8
value: RS_BLEND_SRC_INVALID = 100
deprecated: 22
summary: Blend source function
description:
end:

type: rs_blend_dst_func
version: 16 22
size: 32
enum:
value: RS_BLEND_DST_ZERO = 0
value: RS_BLEND_DST_ONE = 1
value: RS_BLEND_DST_SRC_COLOR = 2
value: RS_BLEND_DST_ONE_MINUS_SRC_COLOR = 3
value: RS_BLEND_DST_SRC_ALPHA = 4
value: RS_BLEND_DST_ONE_MINUS_SRC_ALPHA = 5
value: RS_BLEND_DST_DST_ALPHA = 6
value: RS_BLEND_DST_ONE_MINUS_DST_ALPHA = 7
value: RS_BLEND_DST_INVALID = 100
deprecated: 22
summary: Blend destination function
description:
end:

type: rs_cull_mode
version: 16 22
size: 32
enum:
value: RS_CULL_BACK = 0
value: RS_CULL_FRONT = 1
value: RS_CULL_NONE = 2
value: RS_CULL_INVALID = 100
deprecated: 22
summary: Culling mode
description:
end:

type: rs_depth_func
version: 16 22
size: 32
enum:
value: RS_DEPTH_FUNC_ALWAYS = 0, "Always drawn"
value: RS_DEPTH_FUNC_LESS = 1, "Drawn if the incoming depth value is less than that in the depth buffer"
value: RS_DEPTH_FUNC_LEQUAL = 2, "Drawn if the incoming depth value is less or equal to that in the depth buffer"
value: RS_DEPTH_FUNC_GREATER = 3, "Drawn if the incoming depth value is greater than that in the depth buffer"
value: RS_DEPTH_FUNC_GEQUAL = 4, "Drawn if the incoming depth value is greater or equal to that in the depth buffer"
value: RS_DEPTH_FUNC_EQUAL = 5, "Drawn if the incoming depth value is equal to that in the depth buffer"
value: RS_DEPTH_FUNC_NOTEQUAL = 6, "Drawn if the incoming depth value is not equal to that in the depth buffer"
value: RS_DEPTH_FUNC_INVALID = 100, "Invalid depth function"
deprecated: 22
summary: Depth function
description:
 Specifies conditional drawing depending on the comparison of the incoming
 depth to that found in the depth buffer.
end:

type: rs_primitive
version: 16 22
size: 32
enum:
value: RS_PRIMITIVE_POINT = 0, "Vertex data will be rendered as a series of points"
value: RS_PRIMITIVE_LINE = 1, "Vertex pairs will be rendered as lines"
value: RS_PRIMITIVE_LINE_STRIP = 2, "Vertex data will be rendered as a connected line strip"
value: RS_PRIMITIVE_TRIANGLE = 3, "Vertices will be rendered as individual triangles"
value: RS_PRIMITIVE_TRIANGLE_STRIP = 4, "Vertices will be rendered as a connected triangle strip defined by the first three vertices with each additional triangle defined by a new vertex"
value: RS_PRIMITIVE_TRIANGLE_FAN = 5, "Vertices will be rendered as a sequence of triangles that all share first vertex as the origin"
value: RS_PRIMITIVE_INVALID = 100, "Invalid primitive"
deprecated: 22
summary: How to intepret mesh vertex data
description:
 Describes the way mesh vertex data is interpreted when rendering
end:

type: rs_font
version: 9 22
size: 32
rs_object:
deprecated: 22
summary: Handle to a Font
description:
 Opaque handle to a RenderScript font object.
 See: android.renderscript.Font
end:


type: rs_mesh
version: 9 22
size: 32
rs_object:
deprecated: 22
summary: Handle to a Mesh
description:
 Opaque handle to a RenderScript mesh object.
 See: android.renderscript.Mesh
end:

type: rs_program_fragment
version: 9 22
size: 32
rs_object:
deprecated: 22
summary: Handle to a ProgramFragment
description:
 Opaque handle to a RenderScript ProgramFragment object.
 See: android.renderscript.ProgramFragment
end:

type: rs_program_vertex
version: 9 22
size: 32
rs_object:
deprecated: 22
summary: Handle to a ProgramVertex
description:
 Opaque handle to a RenderScript ProgramVertex object.
 See: android.renderscript.ProgramVertex
end:

type: rs_program_raster
version: 9 22
size: 32
rs_object:
deprecated: 22
summary: Handle to a ProgramRaster
description:
 Opaque handle to a RenderScript ProgramRaster object.
 See: android.renderscript.ProgramRaster
end:

type: rs_program_store
version: 9 22
size: 32
rs_object:
deprecated: 22
summary: Handle to a ProgramStore
description:
 Opaque handle to a RenderScript ProgramStore object.
 See: android.renderscript.ProgramStore
end:

function: rsClearObject
version: 9 22
size: 32
t: rs_mesh, rs_program_fragment, rs_program_vertex, rs_program_raster, rs_program_store, rs_font
ret: void
arg: #1* dst
test: none
end:

function: rsIsObject
version: 9 22
size: 32
t: rs_mesh, rs_program_fragment, rs_program_vertex, rs_program_raster, rs_program_store, rs_font
ret: bool
arg: #1 v
test: none
end:

function: rsSetObject
version: 9 22
size: 32
t: rs_mesh, rs_program_fragment, rs_program_vertex, rs_program_raster, rs_program_store, rs_font
ret: void
arg: #1* dst
arg: #1 src
test: none
end:

function: rsgAllocationSyncAll
version: 9 22
size: 32
ret: void
arg: rs_allocation alloc
deprecated: 22
summary: Sync the contents of an allocation
description:
 Sync the contents of an allocation.

 If the source is specified, sync from memory space specified by source.

 If the source is not specified, sync from its SCRIPT memory space to its HW
 memory spaces.
test: none
end:

function: rsgAllocationSyncAll
version: 14 22
size: 32
ret: void
arg: rs_allocation alloc
arg: rs_allocation_usage_type source
test: none
end:

function: rsgBindColorTarget
version: 14 22
size: 32
ret: void
arg: rs_allocation colorTarget
arg: uint slot
deprecated: 22
summary: Set the color target
description:
 Set the color target used for all subsequent rendering calls
test: none
end:

function: rsgBindConstant
version: 9 22
size: 32
ret: void
arg: rs_program_fragment ps, "program fragment object"
arg: uint slot, "index of the constant buffer on the program"
arg: rs_allocation c, "constants to bind"
deprecated: 22
summary: Bind a constant allocation
description:
 Bind a new Allocation object to a ProgramFragment or ProgramVertex.
 The Allocation must be a valid constant input for the Program.
test: none
end:

function: rsgBindConstant
version: 9 22
size: 32
ret: void
arg: rs_program_vertex pv, "program vertex object"
arg: uint slot
arg: rs_allocation c
test: none
end:

function: rsgBindDepthTarget
version: 14 22
size: 32
ret: void
arg: rs_allocation depthTarget
deprecated: 22
summary: Set the depth target
description:
 Set the depth target used for all subsequent rendering calls
test: none
end:

function: rsgBindFont
version: 9 22
size: 32
ret: void
arg: rs_font font, "object to bind"
deprecated: 22
summary: Bind a font object
description:
 Binds the font object to be used for all subsequent font rendering calls
test: none
end:

function: rsgBindProgramFragment
version: 9 22
size: 32
ret: void
arg: rs_program_fragment pf
deprecated: 22
summary: Bind a ProgramFragment
description:
 Bind a new ProgramFragment to the rendering context.
test: none
end:

function: rsgBindProgramRaster
version: 9 22
size: 32
ret: void
arg: rs_program_raster pr
deprecated: 22
summary: Bind a ProgramRaster
description:
 Bind a new ProgramRaster to the rendering context.
test: none
end:

function: rsgBindProgramStore
version: 9 22
size: 32
ret: void
arg: rs_program_store ps
deprecated: 22
summary: Bind a ProgramStore
description:
 Bind a new ProgramStore to the rendering context.
test: none
end:

function: rsgBindProgramVertex
version: 9 22
size: 32
ret: void
arg: rs_program_vertex pv
deprecated: 22
summary: Bind a ProgramVertex
description:
 Bind a new ProgramVertex to the rendering context.
test: none
end:

function: rsgBindSampler
version: 9 22
size: 32
ret: void
arg: rs_program_fragment fragment
arg: uint slot
arg: rs_sampler sampler
deprecated: 22
summary: Bind a sampler
description:
 Bind a new Sampler object to a ProgramFragment.  The sampler will
 operate on the texture bound at the matching slot.
test: none
end:

function: rsgBindTexture
version: 9 22
size: 32
ret: void
arg: rs_program_fragment v
arg: uint slot
arg: rs_allocation alloc
deprecated: 22
summary: Bind a texture allocation
description:
 Bind a new Allocation object to a ProgramFragment.  The
 Allocation must be a valid texture for the Program.  The sampling
 of the texture will be controled by the Sampler bound at the
 matching slot.
test: none
end:

function: rsgClearAllRenderTargets
version: 14 22
size: 32
ret: void
deprecated: 22
summary: Clear all color and depth targets
description:
 Clear all color and depth targets and resume rendering into
 the framebuffer
test: none
end:

function: rsgClearColor
version: 9 22
size: 32
ret: void
arg: float r
arg: float g
arg: float b
arg: float a
deprecated: 22
summary: Clear the specified color from the surface
description:
 Clears the rendering surface to the specified color.
test: none
end:

function: rsgClearColorTarget
version: 14 22
size: 32
ret: void
arg: uint slot
deprecated: 22
summary: Clear the color target
description:
 Clear the previously set color target
test: none
end:

function: rsgClearDepth
version: 9 22
size: 32
ret: void
arg: float value
deprecated: 22
summary: Clear the depth surface
description:
 Clears the depth suface to the specified value.
test: none
end:

function: rsgClearDepthTarget
version: 14 22
size: 32
ret: void
deprecated: 22
summary: Clear the depth target
description:
 Clear the previously set depth target
test: none
end:

function: rsgDrawMesh
version: 9 22
size: 32
ret: void
arg: rs_mesh ism, "mesh object to render"
deprecated: 22
summary: Draw a mesh
description:
 Draw a mesh using the current context state.

 If primitiveIndex is specified, draw part of a mesh using the current context state.

 If start and len are also specified, draw specified index range of part of a mesh using the current context state.

 Otherwise the whole mesh is rendered.
test: none
end:

function: rsgDrawMesh
version: 9 22
size: 32
ret: void
arg: rs_mesh ism
arg: uint primitiveIndex, "for meshes that contain multiple primitive groups this parameter specifies the index of the group to draw."
test: none
end:

function: rsgDrawMesh
version: 9 22
size: 32
ret: void
arg: rs_mesh ism
arg: uint primitiveIndex
arg: uint start, "starting index in the range"
arg: uint len, "number of indices to draw"
test: none
end:

function: rsgDrawQuad
version: 9 22
size: 32
ret: void
arg: float x1
arg: float y1
arg: float z1
arg: float x2
arg: float y2
arg: float z2
arg: float x3
arg: float y3
arg: float z3
arg: float x4
arg: float y4
arg: float z4
deprecated: 22
summary: Draw a quad
description:
 Low performance utility function for drawing a simple quad.  Not intended for
 drawing large quantities of geometry.
test: none
end:

function: rsgDrawQuadTexCoords
version: 9 22
size: 32
ret: void
arg: float x1
arg: float y1
arg: float z1
arg: float u1
arg: float v1
arg: float x2
arg: float y2
arg: float z2
arg: float u2
arg: float v2
arg: float x3
arg: float y3
arg: float z3
arg: float u3
arg: float v3
arg: float x4
arg: float y4
arg: float z4
arg: float u4
arg: float v4
deprecated: 22
summary: Draw a textured quad
description:
 Low performance utility function for drawing a textured quad.  Not intended
 for drawing large quantities of geometry.
test: none
end:

function: rsgDrawRect
version: 9 22
size: 32
ret: void
arg: float x1
arg: float y1
arg: float x2
arg: float y2
arg: float z
deprecated: 22
summary: Draw a rectangle
description:
 Low performance utility function for drawing a simple rectangle.  Not
 intended for drawing large quantities of geometry.
test: none
end:

function: rsgDrawSpriteScreenspace
version: 9 22
size: 32
ret: void
arg: float x
arg: float y
arg: float z
arg: float w
arg: float h
deprecated: 22
summary: Draw rectangles in screenspace
description:
 Low performance function for drawing rectangles in screenspace.  This
 function uses the default passthough ProgramVertex.  Any bound ProgramVertex
 is ignored.  This function has considerable overhead and should not be used
 for drawing in shipping applications.
test: none
end:

function: rsgDrawText
version: 9 22
size: 32
ret: void
arg: const char* text
arg: int x
arg: int y
deprecated: 22
summary: Draw a text string
description:
 Draws text given a string and location
test: none
end:

function: rsgDrawText
version: 9 22
size: 32
ret: void
arg: rs_allocation alloc
arg: int x
arg: int y
test: none
end:

function: rsgFinish
version: 14 22
size: 32
ret: uint
deprecated: 22
summary: End rendering commands
description:
 Force RenderScript to finish all rendering commands
test: none
end:

function: rsgFontColor
version: 9 22
size: 32
ret: void
arg: float r, "red component"
arg: float g, "green component"
arg: float b, "blue component"
arg: float a, "alpha component"
deprecated: 22
summary: Set the font color
description:
 Sets the font color for all subsequent rendering calls
test: none
end:

function: rsgGetHeight
version: 9 22
size: 32
ret: uint
deprecated: 22
summary: Get the surface height
description:
 Get the height of the current rendering surface.
test: none
end:

function: rsgGetWidth
version: 9 22
size: 32
ret: uint
deprecated: 22
summary: Get the surface width
description:
 Get the width of the current rendering surface.
test: none
end:

function: rsgMeasureText
version: 9 22
size: 32
ret: void
arg: const char* text
arg: int* left
arg: int* right
arg: int* top
arg: int* bottom
deprecated: 22
summary: Get the bounding box for a text string
description:
 Returns the bounding box of the text relative to (0, 0)
 Any of left, right, top, bottom could be NULL
test: none
end:

function: rsgMeasureText
version: 9 22
size: 32
ret: void
arg: rs_allocation alloc
arg: int* left
arg: int* right
arg: int* top
arg: int* bottom
test: none
end:

function: rsgMeshComputeBoundingBox
version: 9 22
size: 32
ret: void
arg: rs_mesh mesh
arg: float* minX
arg: float* minY
arg: float* min
arg: float* maxX
arg: float* maxY
arg: float* maxZ
deprecated: 22
summary: Compute a bounding box
description:
 Computes an axis aligned bounding box of a mesh object
test: none
end:

function: rsgMeshComputeBoundingBox
version: 9 22
size: 32
ret: void
arg: rs_mesh mesh
arg: float3* bBoxMin
arg: float3* bBoxMax
inline:
 float x1, y1, z1, x2, y2, z2;
 rsgMeshComputeBoundingBox(mesh, &x1, &y1, &z1, &x2, &y2, &z2);
 bBoxMin->x = x1;
 bBoxMin->y = y1;
 bBoxMin->z = z1;
 bBoxMax->x = x2;
 bBoxMax->y = y2;
 bBoxMax->z = z2;
test: none
end:

function: rsgMeshGetIndexAllocation
version: 16 22
size: 32
ret: rs_allocation, "allocation containing index data"
arg: rs_mesh m, "mesh to get data from"
arg: uint32_t index, "index of the index allocation"
deprecated: 22
summary: Return an allocation containing index data
description:
 Returns an allocation containing index data or a null
 allocation if only the primitive is specified
test: none
end:

function: rsgMeshGetPrimitive
version: 16 22
size: 32
ret: rs_primitive, "primitive describing how the mesh is rendered"
arg: rs_mesh m, "mesh to get data from"
arg: uint32_t index, "index of the primitive"
deprecated: 22
summary: Return the primitive
description:
 Returns the primitive describing how a part of the mesh is
 rendered
test: none
end:

function: rsgMeshGetPrimitiveCount
version: 16 22
size: 32
ret: uint32_t, "number of primitive groups in the mesh. This would include simple primitives as well as allocations containing index data"
arg: rs_mesh m, "mesh to get data from"
deprecated: 22
summary: Return the number of index sets
description:
 Meshes could have multiple index sets, this function returns
 the number.
test: none
end:

function: rsgMeshGetVertexAllocation
version: 16 22
size: 32
ret: rs_allocation, "allocation containing vertex data"
arg: rs_mesh m, "mesh to get data from"
arg: uint32_t index, "index of the vertex allocation"
deprecated: 22
summary: Return a vertex allocation
description:
 Returns an allocation that is part of the mesh and contains
 vertex data, e.g. positions, normals, texcoords
test: none
end:

function: rsgMeshGetVertexAllocationCount
version: 16 22
size: 32
ret: uint32_t, "number of allocations in the mesh that contain vertex data"
arg: rs_mesh m, "mesh to get data from"
deprecated: 22
summary: Return the number of vertex allocations
description:
 Returns the number of allocations in the mesh that contain
 vertex data
test: none
end:

function: rsgProgramFragmentConstantColor
version: 9 22
size: 32
ret: void
arg: rs_program_fragment pf
arg: float r
arg: float g
arg: float b
arg: float a
deprecated: 22
summary: Set the constant color for a fixed function emulation program
description:
 Set the constant color for a fixed function emulation program.
test: none
end:

function: rsgProgramVertexGetProjectionMatrix
version: 9 22
size: 32
ret: void
arg: rs_matrix4x4* proj, "matrix to store the current projection matrix into"
deprecated: 22
summary: Get the projection matrix for a fixed function vertex program
description:
 Get the projection matrix for a currently bound fixed function
 vertex program. Calling this function with a custom vertex shader
 would result in an error.
test: none
end:

function: rsgProgramVertexLoadModelMatrix
version: 9 22
size: 32
ret: void
arg: const rs_matrix4x4* model, "model matrix"
deprecated: 22
summary: Load the model matrix for a bound fixed function vertex program
description:
 Load the model matrix for a currently bound fixed function
 vertex program. Calling this function with a custom vertex shader
 would result in an error.
test: none
end:

function: rsgProgramVertexLoadProjectionMatrix
version: 9 22
size: 32
ret: void
arg: const rs_matrix4x4* proj, "projection matrix"
deprecated: 22
summary: Load the projection matrix for a bound fixed function vertex program
description:
 Load the projection matrix for a currently bound fixed function
 vertex program. Calling this function with a custom vertex shader
 would result in an error.
test: none
end:

function: rsgProgramVertexLoadTextureMatrix
version: 9 22
size: 32
ret: void
arg: const rs_matrix4x4* tex, "texture matrix"
deprecated: 22
summary:  Load the texture matrix for a bound fixed function vertex program
description:
 Load the texture matrix for a currently bound fixed function
 vertex program. Calling this function with a custom vertex shader
 would result in an error.
test: none
end:

function: rsgProgramRasterGetCullMode
version: 16 22
size: 32
ret: rs_cull_mode
arg: rs_program_raster pr, "program raster to query"
deprecated: 22
summary: Get program raster cull mode
description:
 Get program raster cull mode
test: none
end:

function: rsgProgramRasterIsPointSpriteEnabled
version: 16 22
size: 32
ret: bool
arg: rs_program_raster pr, "program raster to query"
deprecated: 22
summary: Get program raster point sprite state
description:
 Get program raster point sprite state
test: none
end:

function: rsgProgramStoreGetBlendDstFunc
version: 16 22
size: 32
ret: rs_blend_dst_func
arg: rs_program_store ps, "program store to query"
deprecated: 22
summary: Get program store blend destination function
description:
 Get program store blend destination function
test: none
end:

function: rsgProgramStoreGetBlendSrcFunc
version: 16 22
size: 32
ret: rs_blend_src_func
arg: rs_program_store ps, "program store to query"
deprecated: 22
summary: Get program store blend source function
description:
 Get program store blend source function
test: none
end:

function: rsgProgramStoreGetDepthFunc
version: 16 22
size: 32
ret: rs_depth_func
arg: rs_program_store ps, "program store to query"
deprecated: 22
summary: Get program store depth function
description:
 Get program store depth function
test: none
end:

function: rsgProgramStoreIsColorMaskAlphaEnabled
version: 16 22
size: 32
ret: bool
arg: rs_program_store ps, "program store to query"
deprecated: 22
summary: Get program store alpha component color mask
description:
 Get program store alpha component color mask
test: none
end:

function: rsgProgramStoreIsColorMaskBlueEnabled
version: 16 22
size: 32
ret: bool
arg: rs_program_store ps, "program store to query"
deprecated: 22
summary: Get program store blur component color mask
description:
 Get program store blur component color mask
test: none
end:

function: rsgProgramStoreIsColorMaskGreenEnabled
version: 16 22
size: 32
ret: bool
arg: rs_program_store ps, "program store to query"
deprecated: 22
summary: Get program store green component color mask
description:
 Get program store green component color mask
test: none
end:

function: rsgProgramStoreIsColorMaskRedEnabled
version: 16 22
size: 32
ret: bool
arg: rs_program_store ps, "program store to query"
deprecated: 22
summary: Get program store red component color mask
description:
 Get program store red component color mask
test: none
end:

function: rsgProgramStoreIsDepthMaskEnabled
version: 16 22
size: 32
ret: bool
arg: rs_program_store ps, "program store to query"
deprecated: 22
summary: Get program store depth mask
description:
 Get program store depth mask
test: none
end:

function: rsgProgramStoreIsDitherEnabled
version: 16 22
size: 32
ret: bool
arg: rs_program_store ps, "program store to query"
deprecated: 22
summary: Get program store dither state
description:
 Get program store dither state
test: none
end:
