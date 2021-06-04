    // C function void glGetTransformFeedbackVarying ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name )

    public static native void glGetTransformFeedbackVarying(
        int program,
        int index,
        int bufsize,
        int[] length,
        int lengthOffset,
        int[] size,
        int sizeOffset,
        int[] type,
        int typeOffset,
        byte[] name,
        int nameOffset
    );

    // C function void glGetTransformFeedbackVarying ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name )
    /**
     * @deprecated
     * Use the version that takes a ByteBuffer as the last argument, or the versions that return a String.
     * */
    public static native void glGetTransformFeedbackVarying(
        int program,
        int index,
        int bufsize,
        java.nio.IntBuffer length,
        java.nio.IntBuffer size,
        java.nio.IntBuffer type,
        byte name
    );

    // C function void glGetTransformFeedbackVarying ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name )

    public static native void glGetTransformFeedbackVarying(
        int program,
        int index,
        int bufsize,
        java.nio.IntBuffer length,
        java.nio.IntBuffer size,
        java.nio.IntBuffer type,
        java.nio.ByteBuffer name
    );

    // C function void glGetTransformFeedbackVarying ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name )

    public static native String glGetTransformFeedbackVarying(
        int program,
        int index,
        int[] size,
        int sizeOffset,
        int[] type,
        int typeOffset
    );

    // C function void glGetTransformFeedbackVarying ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name )

    public static native String glGetTransformFeedbackVarying(
        int program,
        int index,
        java.nio.IntBuffer size,
        java.nio.IntBuffer type
    );

