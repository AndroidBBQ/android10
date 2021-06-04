/* EGLSurface eglCreatePixmapSurface ( EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list ) */
static jobject
android_eglCreatePixmapSurface
  (JNIEnv *_env, jobject _this, jobject dpy, jobject config, jint pixmap, jintArray attrib_list_ref, jint offset) {
    jniThrowException(_env, "java/lang/UnsupportedOperationException",
        "eglCreatePixmapSurface");
    return nullptr;
}

