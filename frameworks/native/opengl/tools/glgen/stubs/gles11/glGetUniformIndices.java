    // C function void glGetUniformIndices ( GLuint program, GLsizei uniformCount, const GLchar *const *uniformNames, GLuint *uniformIndices )

    public static native void glGetUniformIndices(
        int program,
        String[] uniformNames,
        int[] uniformIndices,
        int uniformIndicesOffset
    );

    // C function void glGetUniformIndices ( GLuint program, GLsizei uniformCount, const GLchar *const *uniformNames, GLuint *uniformIndices )

    public static native void glGetUniformIndices(
        int program,
        String[] uniformNames,
        java.nio.IntBuffer uniformIndices
    );

