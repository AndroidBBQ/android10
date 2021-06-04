    // C function void glGetActiveUniformBlockName ( GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName )

    public static native void glGetActiveUniformBlockName(
        int program,
        int uniformBlockIndex,
        int bufSize,
        int[] length,
        int lengthOffset,
        byte[] uniformBlockName,
        int uniformBlockNameOffset
    );

    // C function void glGetActiveUniformBlockName ( GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName )

    public static native void glGetActiveUniformBlockName(
        int program,
        int uniformBlockIndex,
        java.nio.Buffer length,
        java.nio.Buffer uniformBlockName
    );

    // C function void glGetActiveUniformBlockName ( GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName )

    public static native String glGetActiveUniformBlockName(
        int program,
        int uniformBlockIndex
    );

