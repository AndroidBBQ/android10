/* EGLDisplay eglGetDisplay ( EGLNativeDisplayType display_id ) */
static jobject
android_eglGetDisplay
  (JNIEnv *_env, jobject _this, jlong display_id) {
    EGLDisplay _returnValue = (EGLDisplay) 0;
    _returnValue = eglGetDisplay(
        reinterpret_cast<EGLNativeDisplayType>(display_id)
    );
    return toEGLHandle(_env, egldisplayClass, egldisplayConstructor, _returnValue);
}

/* EGLDisplay eglGetDisplay ( EGLNativeDisplayType display_id ) */
static jobject
android_eglGetDisplayInt
  (JNIEnv *_env, jobject _this, jint display_id) {

    if (static_cast<uintptr_t>(display_id) !=
        reinterpret_cast<uintptr_t>(EGL_DEFAULT_DISPLAY)) {
        jniThrowException(_env, "java/lang/UnsupportedOperationException", "eglGetDisplay");
        return 0;
    }
    return android_eglGetDisplay(_env, _this, display_id);
}

