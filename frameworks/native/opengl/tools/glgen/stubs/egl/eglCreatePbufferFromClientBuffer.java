    // C function EGLSurface eglCreatePbufferFromClientBuffer ( EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list )
    // TODO Deprecate the below method
    public static native EGLSurface eglCreatePbufferFromClientBuffer(
        EGLDisplay dpy,
        int buftype,
        int buffer,
        EGLConfig config,
        int[] attrib_list,
        int offset
    );
    // TODO Unhide the below method
    /**
     * {@hide}
     */
    public static native EGLSurface eglCreatePbufferFromClientBuffer(
        EGLDisplay dpy,
        int buftype,
        long buffer,
        EGLConfig config,
        int[] attrib_list,
        int offset
    );

