    // C function EGLDisplay eglGetDisplay ( EGLNativeDisplayType display_id )

    public static native EGLDisplay eglGetDisplay(
        int display_id
    );

    /**
     * {@hide}
     */
    @UnsupportedAppUsage
    public static native EGLDisplay eglGetDisplay(
        long display_id
    );

