    // C function void glGetShaderSource ( GLuint shader, GLsizei bufsize, GLsizei *length, char *source )

    public static native void glGetShaderSource(
        int shader,
        int bufsize,
        int[] length,
        int lengthOffset,
        byte[] source,
        int sourceOffset
    );

    // C function void glGetShaderSource ( GLuint shader, GLsizei bufsize, GLsizei *length, char *source )

    /** @hide Method is broken, but used to be public (b/6006380) */
    public static native void glGetShaderSource(
        int shader,
        int bufsize,
        java.nio.IntBuffer length,
        byte source
    );

    // C function void glGetShaderSource ( GLuint shader, GLsizei bufsize, GLsizei *length, char *source )

    public static native String glGetShaderSource(
        int shader
    );
