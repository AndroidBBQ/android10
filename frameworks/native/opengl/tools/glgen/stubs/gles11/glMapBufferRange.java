    // C function GLvoid * glMapBufferRange ( GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access )

    /**
     * The {@link java.nio.Buffer} instance returned by this method is guaranteed
     * to be an instance of {@link java.nio.ByteBuffer}.
     */
    public static native java.nio.Buffer glMapBufferRange(
        int target,
        int offset,
        int length,
        int access
    );

