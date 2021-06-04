/* EGLSurface eglCreatePlatformWindowSurface ( EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list ) */
static jobject
android_eglCreatePlatformWindowSurface
  (JNIEnv *_env, jobject _this, jobject dpy, jobject config, jobject native_window_buf, jlongArray attrib_list_ref, jint offset) {
    jint _exception = 0;
    const char * _exceptionType = NULL;
    const char * _exceptionMessage = NULL;
    jarray _array = (jarray) 0;
    jint _bufferOffset = (jint) 0;
    EGLSurface _returnValue = (EGLSurface) 0;
    EGLDisplay dpy_native = (EGLDisplay) fromEGLHandle(_env, egldisplayGetHandleID, dpy);
    EGLConfig config_native = (EGLConfig) fromEGLHandle(_env, eglconfigGetHandleID, config);
    jint _native_windowRemaining;
    void *native_window = (void *) 0;
    jlong *attrib_list_base = (jlong *) 0;
    jint _attrib_listRemaining;
    WrappedEGLAttribs attrib_list;

    if (!native_window_buf) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "native_window == null";
        goto exit;
    }
    native_window = (void *)getPointer(_env, native_window_buf, (jarray*)&_array, &_native_windowRemaining, &_bufferOffset);
    if (!attrib_list_ref) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "attrib_list == null";
        goto exit;
    }
    if (offset < 0) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "offset < 0";
        goto exit;
    }
    _attrib_listRemaining = _env->GetArrayLength(attrib_list_ref) - offset;
    attrib_list_base = (jlong *)
        _env->GetLongArrayElements(attrib_list_ref, (jboolean *)0);
    attrib_list.init(attrib_list_base + offset, _attrib_listRemaining);

    if (native_window == NULL) {
        char * _native_windowBase = (char *)_env->GetPrimitiveArrayCritical(_array, (jboolean *) 0);
        native_window = (void *) (_native_windowBase + _bufferOffset);
    }
    _returnValue = eglCreatePlatformWindowSurface(
        (EGLDisplay)dpy_native,
        (EGLConfig)config_native,
        (void *)native_window,
        attrib_list.attribs
    );

exit:
    if (attrib_list_base) {
        _env->ReleaseLongArrayElements(attrib_list_ref, (jlong*)attrib_list_base,
            JNI_ABORT);
    }
    if (_array) {
        releasePointer(_env, _array, native_window, _exception ? JNI_FALSE : JNI_TRUE);
    }
    if (_exception) {
        jniThrowException(_env, _exceptionType, _exceptionMessage);
        return nullptr;
    }
    return toEGLHandle(_env, eglsurfaceClass, eglsurfaceConstructor, _returnValue);
}

