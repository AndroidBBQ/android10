    // C function EGLSurface eglCreateWindowSurface ( EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list )

    private static native EGLSurface _eglCreateWindowSurface(
        EGLDisplay dpy,
        EGLConfig config,
        Object win,
        int[] attrib_list,
        int offset
    );

    private static native EGLSurface _eglCreateWindowSurfaceTexture(
        EGLDisplay dpy,
        EGLConfig config,
        Object win,
        int[] attrib_list,
        int offset
    );

    public static EGLSurface eglCreateWindowSurface(EGLDisplay dpy,
        EGLConfig config,
        Object win,
        int[] attrib_list,
        int offset
    ){
        Surface sur = null;
        if (win instanceof SurfaceView) {
            SurfaceView surfaceView = (SurfaceView)win;
            sur = surfaceView.getHolder().getSurface();
        } else if (win instanceof SurfaceHolder) {
            SurfaceHolder holder = (SurfaceHolder)win;
            sur = holder.getSurface();
        } else if (win instanceof Surface) {
            sur = (Surface) win;
        }

        EGLSurface surface;
        if (sur != null) {
            surface = _eglCreateWindowSurface(dpy, config, sur, attrib_list, offset);
        } else if (win instanceof SurfaceTexture) {
            surface = _eglCreateWindowSurfaceTexture(dpy, config,
                    win, attrib_list, offset);
        } else {
            throw new java.lang.UnsupportedOperationException(
                "eglCreateWindowSurface() can only be called with an instance of " +
                "Surface, SurfaceView, SurfaceTexture or SurfaceHolder at the moment, " +
                "this will be fixed later.");
        }

        return surface;
    }
