    // C function EGLSurface eglCreatePixmapSurface ( EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list )

    @Deprecated
    public static native EGLSurface eglCreatePixmapSurface(
        EGLDisplay dpy,
        EGLConfig config,
        int pixmap,
        int[] attrib_list,
        int offset
    );

