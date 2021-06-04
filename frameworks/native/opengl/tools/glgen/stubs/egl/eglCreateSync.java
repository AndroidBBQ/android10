    // C function EGLSync eglCreateSync ( EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list )

    public static native EGLSync eglCreateSync(
        EGLDisplay dpy,
        int type,
        long[] attrib_list,
        int offset
    );

    /**
    * C function EGLBoolean eglGetSyncAttrib ( EGLDisplay dpy, EGLSync sync, EGLint attribute,
    *                                          EGLAttrib *value )
    */

    public static native boolean eglGetSyncAttrib(
            EGLDisplay dpy,
            EGLSync sync,
            int attribute,
            long[] value,
            int offset
    );

