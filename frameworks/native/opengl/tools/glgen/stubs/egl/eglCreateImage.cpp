/* EGLImage eglCreateImage ( EGLDisplay dpy, EGLContext context, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list ) */
static jobject
android_eglCreateImage
  (JNIEnv *_env, jobject _this, jobject dpy, jobject context, jint target, jlong buffer, jlongArray attrib_list_ref, jint offset) {
    jint _exception = 0;
    const char * _exceptionType = NULL;
    const char * _exceptionMessage = NULL;
    EGLImage _returnValue = (EGLImage) 0;
    EGLDisplay dpy_native = (EGLDisplay) fromEGLHandle(_env, egldisplayGetHandleID, dpy);
    EGLContext context_native = (EGLContext) fromEGLHandle(_env, eglcontextGetHandleID, context);
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

    _returnValue = eglCreateImage(
        (EGLDisplay)dpy_native,
        (EGLContext)context_native,
        (EGLenum)target,
        (EGLClientBuffer)buffer,
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
    return toEGLHandle(_env, eglimageClass, eglimageConstructor, _returnValue);
}
