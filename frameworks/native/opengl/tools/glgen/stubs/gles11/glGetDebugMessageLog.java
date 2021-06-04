    // C function GLuint glGetDebugMessageLog ( GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog )

    public static native int glGetDebugMessageLog(
        int count,
        int bufSize,
        int[] sources,
        int sourcesOffset,
        int[] types,
        int typesOffset,
        int[] ids,
        int idsOffset,
        int[] severities,
        int severitiesOffset,
        int[] lengths,
        int lengthsOffset,
        byte[] messageLog,
        int messageLogOffset);

    // C function GLuint glGetDebugMessageLog ( GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog )

    public static native int glGetDebugMessageLog(
        int count,
        java.nio.IntBuffer sources,
        java.nio.IntBuffer types,
        java.nio.IntBuffer ids,
        java.nio.IntBuffer severities,
        java.nio.IntBuffer lengths,
        java.nio.ByteBuffer messageLog);

    // C function GLuint glGetDebugMessageLog ( GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog )

    public static native String[] glGetDebugMessageLog(
        int count,
        int[] sources,
        int sourcesOffset,
        int[] types,
        int typesOffset,
        int[] ids,
        int idsOffset,
        int[] severities,
        int severitiesOffset);

    // C function GLuint glGetDebugMessageLog ( GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog )

    public static native String[] glGetDebugMessageLog(
        int count,
        java.nio.IntBuffer sources,
        java.nio.IntBuffer types,
        java.nio.IntBuffer ids,
        java.nio.IntBuffer severities);

