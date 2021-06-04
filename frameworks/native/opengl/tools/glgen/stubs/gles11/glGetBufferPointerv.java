    // C function void glGetBufferPointerv ( GLenum target, GLenum pname, GLvoid** params )

    /**
     * The {@link java.nio.Buffer} instance returned by this method is guaranteed
     * to be an instance of {@link java.nio.ByteBuffer}.
     */
    public static native java.nio.Buffer glGetBufferPointerv(
        int target,
        int pname
    );

