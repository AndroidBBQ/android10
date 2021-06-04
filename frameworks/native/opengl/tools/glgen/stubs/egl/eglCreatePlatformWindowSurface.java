    // C function EGLSurface eglCreatePlatformWindowSurface ( EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list )

    public static native EGLSurface eglCreatePlatformWindowSurface(
        EGLDisplay dpy,
        EGLConfig config,
        java.nio.Buffer native_window,
        long[] attrib_list,
        int offset
    );

