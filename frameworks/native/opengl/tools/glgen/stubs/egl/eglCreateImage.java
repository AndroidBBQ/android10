    // C function EGLImage eglCreateImage ( EGLDisplay dpy, EGLContext context, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list )

    public static native EGLImage eglCreateImage(
        EGLDisplay dpy,
        EGLContext context,
        int target,
        long buffer,
        long[] attrib_list,
        int offset
    );

