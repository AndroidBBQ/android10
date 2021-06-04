/* const char * eglQueryString ( EGLDisplay dpy, EGLint name ) */
static jstring
android_eglQueryString__Landroind_opengl_EGLDisplay_2I
  (JNIEnv *_env, jobject _this, jobject dpy, jint name) {
    const char* chars = (const char*) eglQueryString(
        (EGLDisplay)fromEGLHandle(_env, egldisplayGetHandleID, dpy),
        (EGLint)name
    );
    return _env->NewStringUTF(chars);
}
