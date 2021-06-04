/* EGLSync eglCreateSync ( EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list ) */
static jobject
android_eglCreateSync
  (JNIEnv *_env, jobject _this, jobject dpy, jint type, jlongArray attrib_list_ref, jint offset) {
    jint _exception = 0;
    const char * _exceptionType = NULL;
    const char * _exceptionMessage = NULL;
    EGLSync _returnValue = (EGLSync) 0;
    EGLDisplay dpy_native = (EGLDisplay) fromEGLHandle(_env, egldisplayGetHandleID, dpy);
    jlong *attrib_list_base = (jlong *) 0;
    jint _remaining;
    WrappedEGLAttribs attrib_list;

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
    _remaining = _env->GetArrayLength(attrib_list_ref) - offset;
    attrib_list_base = (jlong *)
        _env->GetLongArrayElements(attrib_list_ref, (jboolean *)0);
    attrib_list.init(attrib_list_base + offset, _remaining);

    _returnValue = eglCreateSync(
        (EGLDisplay)dpy_native,
        (EGLenum)type,
        attrib_list.attribs
    );

exit:
    if (attrib_list_base) {
        _env->ReleaseLongArrayElements(attrib_list_ref, (jlong*)attrib_list_base,
            JNI_ABORT);
    }
    if (_exception) {
        jniThrowException(_env, _exceptionType, _exceptionMessage);
        return nullptr;
    }
    return toEGLHandle(_env, eglsyncClass, eglsyncConstructor, _returnValue);
}

/* EGLBoolean eglGetSyncAttrib ( EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib *value ) */
static jboolean
android_eglGetSyncAttrib
  (JNIEnv *_env, jobject _this, jobject dpy, jobject sync, jint attribute, jlongArray value_ref, jint offset) {
    jint _exception = 0;
    const char * _exceptionType = NULL;
    const char * _exceptionMessage = NULL;
    EGLBoolean _returnValue = (EGLBoolean) 0;
    EGLDisplay dpy_native = (EGLDisplay) fromEGLHandle(_env, egldisplayGetHandleID, dpy);
    EGLSync sync_native = (EGLSync) fromEGLHandle(_env, eglsyncGetHandleID, sync);
    jlong *value_base = (jlong *) 0;
    jint _remaining;
    EGLAttrib value;

    if (!value_ref) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "value == null";
        goto exit;
    }
    if (offset < 0) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "offset < 0";
        goto exit;
    }
    _remaining = _env->GetArrayLength(value_ref) - offset;
    value_base = (jlong *)
        _env->GetLongArrayElements(value_ref, (jboolean *)0);

    _returnValue = eglGetSyncAttrib(
        (EGLDisplay)dpy_native,
        (EGLSync)sync_native,
        (EGLint)attribute,
        &value
    );

    if (value_base && _returnValue == EGL_TRUE) {
        *(value_base + offset) = (jlong) value;
    }

exit:
    if (value_base) {
        _env->ReleaseLongArrayElements(value_ref, (jlong*)value_base,
            _exception ? JNI_ABORT: 0);
    }
    if (_exception) {
        jniThrowException(_env, _exceptionType, _exceptionMessage);
        return JNI_FALSE;
    }
    return (jboolean)_returnValue;
}

