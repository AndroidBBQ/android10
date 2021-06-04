/* EGLSurface eglCreatePbufferFromClientBuffer ( EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list ) */
static jobject
android_eglCreatePbufferFromClientBuffer
  (JNIEnv *_env, jobject _this, jobject dpy, jint buftype, jlong buffer, jobject config, jintArray attrib_list_ref, jint offset) {
    jint _exception = 0;
    const char * _exceptionType = NULL;
    const char * _exceptionMessage = NULL;
    EGLSurface _returnValue = (EGLSurface) 0;
    EGLDisplay dpy_native = (EGLDisplay) fromEGLHandle(_env, egldisplayGetHandleID, dpy);
    EGLConfig config_native = (EGLConfig) fromEGLHandle(_env, eglconfigGetHandleID, config);
    bool attrib_list_sentinel = false;
    EGLint *attrib_list_base = (EGLint *) 0;
    jint _remaining;
    EGLint *attrib_list = (EGLint *) 0;

    if (attrib_list_ref) {
        if (offset < 0) {
            _exception = 1;
            _exceptionType = "java/lang/IllegalArgumentException";
            _exceptionMessage = "offset < 0";
            goto exit;
        }
        _remaining = _env->GetArrayLength(attrib_list_ref) - offset;
        attrib_list_base = (EGLint *)
            _env->GetIntArrayElements(attrib_list_ref, (jboolean *)0);
        attrib_list = attrib_list_base + offset;
        attrib_list_sentinel = false;
        for (int i = _remaining - 1; i >= 0; i--)  {
            if (attrib_list[i] == EGL_NONE){
                attrib_list_sentinel = true;
                break;
            }
        }
        if (attrib_list_sentinel == false) {
            _exception = 1;
            _exceptionType = "java/lang/IllegalArgumentException";
            _exceptionMessage = "attrib_list must contain EGL_NONE!";
            goto exit;
        }
    }

    _returnValue = eglCreatePbufferFromClientBuffer(
        (EGLDisplay)dpy_native,
        (EGLenum)buftype,
        reinterpret_cast<EGLClientBuffer>(buffer),
        (EGLConfig)config_native,
        (EGLint *)attrib_list
    );

exit:
    if (attrib_list_base) {
        _env->ReleaseIntArrayElements(attrib_list_ref, attrib_list_base,
            JNI_ABORT);
    }
    if (_exception) {
        jniThrowException(_env, _exceptionType, _exceptionMessage);
        return nullptr;
    }
    return toEGLHandle(_env, eglsurfaceClass, eglsurfaceConstructor, _returnValue);
}

static jobject
android_eglCreatePbufferFromClientBufferInt
  (JNIEnv *_env, jobject _this, jobject dpy, jint buftype, jint buffer, jobject config, jintArray attrib_list_ref, jint offset) {
    if(sizeof(void*) != sizeof(uint32_t)) {
        jniThrowException(_env, "java/lang/UnsupportedOperationException", "eglCreatePbufferFromClientBuffer");
        return 0;
    }
    return android_eglCreatePbufferFromClientBuffer(_env, _this, dpy, buftype, buffer, config, attrib_list_ref, offset);
}
