/*
** Copyright 2018, The Android Open Source Project
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
#include <vector>
#include <EGL/egl.h>

#include <ui/ANativeObjectBase.h>

// classes from EGL 1.4
static jclass egldisplayClass;
static jclass eglsurfaceClass;
static jclass eglconfigClass;
static jclass eglcontextClass;

static jmethodID egldisplayGetHandleID;
static jmethodID eglconfigGetHandleID;
static jmethodID eglcontextGetHandleID;
static jmethodID eglsurfaceGetHandleID;

static jmethodID egldisplayConstructor;
static jmethodID eglcontextConstructor;
static jmethodID eglsurfaceConstructor;
static jmethodID eglconfigConstructor;

static jobject eglNoContextObject;
static jobject eglNoDisplayObject;
static jobject eglNoSurfaceObject;

// classes from EGL 1.5
static jclass eglimageClass;
static jclass eglsyncClass;

static jmethodID eglimageGetHandleID;
static jmethodID eglsyncGetHandleID;

static jmethodID eglimageConstructor;
static jmethodID eglsyncConstructor;

static jobject eglNoImageObject;
static jobject eglNoSyncObject;

/* Cache method IDs each time the class is loaded. */

static void
nativeClassInit(JNIEnv *_env, jclass glImplClass)
{
    // EGL 1.4 Init
    jclass eglconfigClassLocal = _env->FindClass("android/opengl/EGLConfig");
    eglconfigClass = (jclass) _env->NewGlobalRef(eglconfigClassLocal);
    jclass eglcontextClassLocal = _env->FindClass("android/opengl/EGLContext");
    eglcontextClass = (jclass) _env->NewGlobalRef(eglcontextClassLocal);
    jclass egldisplayClassLocal = _env->FindClass("android/opengl/EGLDisplay");
    egldisplayClass = (jclass) _env->NewGlobalRef(egldisplayClassLocal);
    jclass eglsurfaceClassLocal = _env->FindClass("android/opengl/EGLSurface");
    eglsurfaceClass = (jclass) _env->NewGlobalRef(eglsurfaceClassLocal);

    eglconfigGetHandleID = _env->GetMethodID(eglconfigClass, "getNativeHandle", "()J");
    eglcontextGetHandleID = _env->GetMethodID(eglcontextClass, "getNativeHandle", "()J");
    egldisplayGetHandleID = _env->GetMethodID(egldisplayClass, "getNativeHandle", "()J");
    eglsurfaceGetHandleID = _env->GetMethodID(eglsurfaceClass, "getNativeHandle", "()J");

    eglconfigConstructor = _env->GetMethodID(eglconfigClass, "<init>", "(J)V");
    eglcontextConstructor = _env->GetMethodID(eglcontextClass, "<init>", "(J)V");
    egldisplayConstructor = _env->GetMethodID(egldisplayClass, "<init>", "(J)V");
    eglsurfaceConstructor = _env->GetMethodID(eglsurfaceClass, "<init>", "(J)V");

    jobject localeglNoContextObject = _env->NewObject(eglcontextClass, eglcontextConstructor, reinterpret_cast<jlong>(EGL_NO_CONTEXT));
    eglNoContextObject = _env->NewGlobalRef(localeglNoContextObject);
    jobject localeglNoDisplayObject = _env->NewObject(egldisplayClass, egldisplayConstructor, reinterpret_cast<jlong>(EGL_NO_DISPLAY));
    eglNoDisplayObject = _env->NewGlobalRef(localeglNoDisplayObject);
    jobject localeglNoSurfaceObject = _env->NewObject(eglsurfaceClass, eglsurfaceConstructor, reinterpret_cast<jlong>(EGL_NO_SURFACE));
    eglNoSurfaceObject = _env->NewGlobalRef(localeglNoSurfaceObject);

    jclass eglClass = _env->FindClass("android/opengl/EGL15");
    jfieldID noContextFieldID = _env->GetStaticFieldID(eglClass, "EGL_NO_CONTEXT", "Landroid/opengl/EGLContext;");
    _env->SetStaticObjectField(eglClass, noContextFieldID, eglNoContextObject);

    jfieldID noDisplayFieldID = _env->GetStaticFieldID(eglClass, "EGL_NO_DISPLAY", "Landroid/opengl/EGLDisplay;");
    _env->SetStaticObjectField(eglClass, noDisplayFieldID, eglNoDisplayObject);

    jfieldID noSurfaceFieldID = _env->GetStaticFieldID(eglClass, "EGL_NO_SURFACE", "Landroid/opengl/EGLSurface;");
    _env->SetStaticObjectField(eglClass, noSurfaceFieldID, eglNoSurfaceObject);

    // EGL 1.5 init
    jclass eglimageClassLocal = _env->FindClass("android/opengl/EGLImage");
    eglimageClass = (jclass) _env->NewGlobalRef(eglimageClassLocal);
    jclass eglsyncClassLocal = _env->FindClass("android/opengl/EGLSync");
    eglsyncClass = (jclass) _env->NewGlobalRef(eglsyncClassLocal);

    eglimageGetHandleID = _env->GetMethodID(eglimageClass, "getNativeHandle", "()J");
    eglsyncGetHandleID = _env->GetMethodID(eglsyncClass, "getNativeHandle", "()J");

    eglimageConstructor = _env->GetMethodID(eglimageClass, "<init>", "(J)V");
    eglsyncConstructor = _env->GetMethodID(eglsyncClass, "<init>", "(J)V");

    jfieldID noImageFieldID = _env->GetStaticFieldID(eglClass, "EGL_NO_IMAGE", "Landroid/opengl/EGLImage;");
    _env->SetStaticObjectField(eglClass, noImageFieldID, eglNoImageObject);

    jfieldID noSyncFieldID = _env->GetStaticFieldID(eglClass, "EGL_NO_SYNC", "Landroid/opengl/EGLSync;");
    _env->SetStaticObjectField(eglClass, noSyncFieldID, eglNoSyncObject);
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

static void *
fromEGLHandle(JNIEnv *_env, jmethodID mid, jobject obj) {
    if (obj == NULL) {
        jniThrowException(_env, "java/lang/IllegalArgumentException",
                          "Object is set to null.");
        return nullptr;
    }

    jlong handle = _env->CallLongMethod(obj, mid);
    return reinterpret_cast<void*>(handle);
}

static jobject
toEGLHandle(JNIEnv *_env, jclass cls, jmethodID con, void *handle) {
    if (cls == eglimageClass && (EGLImage)handle == EGL_NO_IMAGE) {
        return eglNoImageObject;
    }

    return _env->NewObject(cls, con, reinterpret_cast<jlong>(handle));
}

struct WrappedEGLAttribs {
private:
    std::vector<EGLAttrib> backing; // only for 32-bit
public:
    EGLAttrib *attribs;
    WrappedEGLAttribs(): attribs(nullptr) { };
    void init(jlong *array, jint size) {
        if (sizeof(EGLAttrib) != sizeof(jlong)) {
            for (jint i = 0; i < size; ++i) {
                backing.push_back(array[i]);
            }
            attribs = backing.data();
        } else {
            attribs = (EGLAttrib*)array;
        }
    }
};

// --------------------------------------------------------------------------
