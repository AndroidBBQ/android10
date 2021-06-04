    // C function EGLDisplay eglGetPlatformDisplay ( EGLenum platform, EGLAttrib native_display, const EGLAttrib *attrib_list )

    public static native EGLDisplay eglGetPlatformDisplay(
        int platform,
        long native_display,
        long[] attrib_list,
        int offset
    );

