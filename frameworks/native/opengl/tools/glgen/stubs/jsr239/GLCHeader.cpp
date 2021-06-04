**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

// This source file is automatically generated

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"

#include "jni.h"
#include <nativehelper/JNIHelp.h>
#include <android_runtime/AndroidRuntime.h>
#include <utils/misc.h>

#include <assert.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

// Work around differences between the generated name and the actual name.

#define glBlendEquation glBlendEquationOES
#define glBlendEquationSeparate glBlendEquationSeparateOES
#define glBlendFuncSeparate glBlendFuncSeparateOES
#define glGetTexGenfv glGetTexGenfvOES
#define glGetTexGeniv glGetTexGenivOES
#define glGetTexGenxv glGetTexGenxvOES
#define glTexGenf glTexGenfOES
#define glTexGenfv glTexGenfvOES
#define glTexGeni glTexGeniOES
#define glTexGeniv glTexGenivOES
#define glTexGenx glTexGenxOES
#define glTexGenxv glTexGenxvOES



/* special calls implemented in Android's GLES wrapper used to more
 * efficiently bound-check passed arrays */
extern "C" {
GL_API void GL_APIENTRY glColorPointerBounds(GLint size, GLenum type, GLsizei stride,
        const GLvoid *ptr, GLsizei count);
GL_API void GL_APIENTRY glNormalPointerBounds(GLenum type, GLsizei stride,
        const GLvoid *pointer, GLsizei count);
GL_API void GL_APIENTRY glTexCoordPointerBounds(GLint size, GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei count);
GL_API void GL_APIENTRY glVertexPointerBounds(GLint size, GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei count);
GL_API void GL_APIENTRY glPointSizePointerOESBounds(GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei count);
GL_API void GL_APIENTRY glMatrixIndexPointerOESBounds(GLint size, GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei count);
GL_API void GL_APIENTRY glWeightPointerOESBounds(GLint size, GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei count);
}

static jclass G11ImplClass;
static jfieldID haveCheckedExtensionsID;
static jfieldID have_OES_blend_equation_separateID;
static jfieldID have_OES_blend_subtractID;
static jfieldID have_OES_framebuffer_objectID;
static jfieldID have_OES_texture_cube_mapID;

/* Cache method IDs each time the class is loaded. */

static void
nativeClassInit(JNIEnv *_env, jclass glImplClass)
{
    jclass g11impClassLocal = _env->FindClass("com/google/android/gles_jni/GLImpl");
    G11ImplClass = (jclass) _env->NewGlobalRef(g11impClassLocal);
    haveCheckedExtensionsID =  _env->GetFieldID(G11ImplClass, "haveCheckedExtensions", "Z");
    have_OES_blend_equation_separateID =  _env->GetFieldID(G11ImplClass, "have_OES_blend_equation_separate", "Z");
    have_OES_blend_subtractID =  _env->GetFieldID(G11ImplClass, "have_OES_blend_subtract", "Z");
    have_OES_framebuffer_objectID =  _env->GetFieldID(G11ImplClass, "have_OES_framebuffer_object", "Z");
    have_OES_texture_cube_mapID =  _env->GetFieldID(G11ImplClass, "have_OES_texture_cube_map", "Z");
}

static void *
getPointer(JNIEnv *_env, jobject buffer, jarray *array, jint *remaining, jint *offset)
{
    jint position;
    jint limit;
    jint elementSizeShift;
    jlong pointer;

    pointer = jniGetNioBufferFields(_env, buffer, &position, &limit, &elementSizeShift);
    *remaining = (limit - position) << elementSizeShift;
    if (pointer != 0L) {
        *array = nullptr;
        pointer += position << elementSizeShift;
        return reinterpret_cast<void*>(pointer);
    }

    *array = jniGetNioBufferBaseArray(_env, buffer);
    *offset = jniGetNioBufferBaseArrayOffset(_env, buffer);
    return nullptr;
}

static void
releasePointer(JNIEnv *_env, jarray array, void *data, jboolean commit)
{
    _env->ReleasePrimitiveArrayCritical(array, data,
					   commit ? 0 : JNI_ABORT);
}

extern "C" {
extern char*  __progname;
}

static void *
getDirectBufferPointer(JNIEnv *_env, jobject buffer) {
    if (buffer == nullptr) {
        return nullptr;
    }

    jint position;
    jint limit;
    jint elementSizeShift;
    jlong pointer;
    pointer = jniGetNioBufferFields(_env, buffer, &position, &limit, &elementSizeShift);
    if (pointer == 0) {
        jniThrowException(_env, "java/lang/IllegalArgumentException",
                          "Must use a native order direct Buffer");
        return nullptr;
    }
    pointer += position << elementSizeShift;
    return reinterpret_cast<void*>(pointer);
}

static int
getNumCompressedTextureFormats() {
    int numCompressedTextureFormats = 0;
    glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numCompressedTextureFormats);
    return numCompressedTextureFormats;
}

// Check if the extension at the head of pExtensions is pExtension. Note that pExtensions is
// terminated by either 0 or space, while pExtension is terminated by 0.

static bool
extensionEqual(const GLubyte* pExtensions, const GLubyte* pExtension) {
    while (true) {
        char a = *pExtensions++;
        char b = *pExtension++;
        bool aEnd = a == '\0' || a == ' ';
        bool bEnd = b == '\0';
        if ( aEnd || bEnd) {
            return aEnd == bEnd;
        }
        if ( a != b ) {
            return false;
        }
    }
}

static const GLubyte*
nextExtension(const GLubyte* pExtensions) {
    while (true) {
        char a = *pExtensions++;
        if ( a == '\0') {
            return pExtensions-1;
        } else if ( a == ' ') {
            return pExtensions;
        }
    }
}

static bool
checkForExtension(const GLubyte* pExtensions, const GLubyte* pExtension) {
    for (;*pExtensions != '\0'; pExtensions = nextExtension(pExtensions)) {
        if (extensionEqual(pExtensions, pExtension)) {
            return true;
        }
    }
    return false;
}

static bool
supportsExtension(JNIEnv *_env, jobject impl, jfieldID fieldId) {
    if (!_env->GetBooleanField(impl, haveCheckedExtensionsID)) {
        _env->SetBooleanField(impl, haveCheckedExtensionsID, true);
        const GLubyte* sExtensions = glGetString(GL_EXTENSIONS);
        _env->SetBooleanField(impl, have_OES_blend_equation_separateID,
            checkForExtension(sExtensions, (const GLubyte*) "GL_OES_blend_equation_separate"));
        _env->SetBooleanField(impl, have_OES_blend_subtractID,
            checkForExtension(sExtensions, (const GLubyte*) "GL_OES_blend_subtract"));
        _env->SetBooleanField(impl, have_OES_framebuffer_objectID,
            checkForExtension(sExtensions, (const GLubyte*) "GL_OES_framebuffer_object"));
        _env->SetBooleanField(impl, have_OES_texture_cube_mapID,
            checkForExtension(sExtensions, (const GLubyte*) "GL_OES_texture_cube_map"));
    }
    return _env->GetBooleanField(impl, fieldId);
}

// --------------------------------------------------------------------------
