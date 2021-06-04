# Copyright (C) 2013 The Android Open Source Project
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

#
# ifcheck defaults to 1 so we don't explicitly list pnames requiring
# a single value. unknown pnames will be validated against one value, which
# is not perfect but better than nothing.
#

glBufferData nullAllowed data check data size
glBufferSubData check data size
# glCompressedTexImage2D
# glCompressedTexSubImage2D
glClipPlanef check eqn 4
glClipPlanex check eqn 4
glDebugMessageControl check ids count
glDebugMessageInsert check message length
glDeleteBuffers check buffers n
glDeleteFramebuffers check framebuffers n
glDeleteFramebuffersOES check framebuffers n
glDeleteRenderbuffers check renderbuffers n
glDeleteRenderbuffersOES check renderbuffers n
glDeleteTextures check textures n
glDrawElements check_AIOOBE indices count
glDrawElementsBaseVertex check_AIOOBE indices count-basevertex
glDrawRangeElementsBaseVertex check_AIOOBE indices count-basevertex
glDrawTexfvOES check coords 5
glDrawTexivOES check coords 5
glDrawTexsvOES check coords 5
glDrawTexxvOES check coords 5
glFog ifcheck params 4 pname GL_FOG_COLOR
glGenBuffers check buffers n
glGenFramebuffersOES check framebuffers n
glGenFramebuffers check framebuffers n
glGenRenderbuffersOES check renderbuffers n
glGenRenderbuffers check renderbuffers n
glGenTextures check textures n
// glGetActiveAttrib
// glGetActiveUniform
glGetAttachedShaders nullAllowed count check count 1 check shaders maxcount
// glGetBooleanv
glGetBufferParameter check params 1
glGetClipPlanef check eqn 4
glGetClipPlanex check eqn 4
glGetClipPlanefOES check eqn 4
glGetClipPlanexOES check eqn 4
// glGetFloatv
glGetFramebufferAttachmentParameterivOES check params 1
// glGetIntegerv
glGetLight ifcheck params 3 pname GL_SPOT_DIRECTION ifcheck params 4 pname GL_AMBIENT,GL_DIFFUSE,GL_SPECULAR,GL_EMISSION
glGetMaterial ifcheck params 4 pname GL_AMBIENT,GL_DIFFUSE,GL_SPECULAR,GL_EMISSION,GL_AMBIENT_AND_DIFFUSE
glGetProgramBinary nullAllowed length
// glGetProgramInfoLog
glGetProgramiv check params 1
glGetProgramResourceiv nullAllowed length
glGetRenderbufferParameteriv check params 1
glGetRenderbufferParameterivOES check params 1
// glGetShaderInfoLog
glGetShaderiv check params 1
glGetShaderPrecisionFormat check range 1 check precision 1
// glGetShaderSource
// glGetString
glGetSynciv nullAllowed length
glGetTexEnv ifcheck params 4 pname GL_TEXTURE_ENV_COLOR
glGetTexGen ifcheck params 4 pname GL_OBJECT_PLANE,GL_EYE_PLANE
glGetTexParameter check params 1
glGetnUniformfv check params bufSize
glGetnUniformiv check params bufSize
glGetnUniformuiv check params bufSize
glGetUniform check params 1
glGetVertexAttrib ifcheck params 4 pname GL_CURRENT_VERTEX_ATTRIB
glLight ifcheck params 3 pname GL_SPOT_DIRECTION ifcheck params 4 pname GL_AMBIENT,GL_DIFFUSE,GL_SPECULAR,GL_EMISSION
glLightModel ifcheck params 4 pname GL_LIGHT_MODEL_AMBIENT
glLoadMatrix check m 16
glMaterial ifcheck params 4 pname GL_AMBIENT,GL_DIFFUSE,GL_SPECULAR,GL_EMISSION,GL_AMBIENT_AND_DIFFUSE
glMultMatrix check m 16
glObjectLabelKHR nullAllowed label
glObjectLabel nullAllowed label check label length
glPointParameter check params 1
glPushDebugGroup check message length
glQueryMatrixxOES check mantissa 16 check exponent 16 return -1
# glReadPixels
glReadnPixels check data bufSize
glShaderBinary check binary length
// glShaderSource
glTexEnv ifcheck params 4 pname GL_TEXTURE_ENV_COLOR
glTexImage2D nullAllowed pixels
glTexImage3D nullAllowed pixels
glTexParameter check params 1
glTexSubImage2D nullAllowed pixels
glUniform1 check v count
glUniform2 check v count*2
glUniform3 check v count*3
glUniform4 check v count*4
glUniformMatrix2 check value count*4
glUniformMatrix3 check value count*9
glUniformMatrix4 check value count*16
glVertexAttrib1 check values 1
glVertexAttrib2 check values 2
glVertexAttrib3 check values 3
glVertexAttrib4 check values 4
# glVertexAttribPointer
