    // C function void glGetProgramResourceName ( GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name )

    public static native String glGetProgramResourceName(
        int program,
        int programInterface,
        int index
    );

