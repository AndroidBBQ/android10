    // C function GLuint glGetDebugMessageLogKHR ( GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog )

    public static native int glGetDebugMessageLogKHR(
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

    // C function GLuint glGetDebugMessageLogKHR ( GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog )

    public static native int glGetDebugMessageLogKHR(
        int count,
        java.nio.IntBuffer sources,
        java.nio.IntBuffer types,
        java.nio.IntBuffer ids,
        java.nio.IntBuffer severities,
        java.nio.IntBuffer lengths,
        java.nio.ByteBuffer messageLog);

    // C function GLuint glGetDebugMessageLogKHR ( GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog )

    public static native String[] glGetDebugMessageLogKHR(
        int count,
        int[] sources,
        int sourcesOffset,
        int[] types,
        int typesOffset,
        int[] ids,
        int idsOffset,
        int[] severities,
        int severitiesOffset);

    // C function GLuint glGetDebugMessageLogKHR ( GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog )

    public static native String[] glGetDebugMessageLogKHR(
        int count,
        java.nio.IntBuffer sources,
        java.nio.IntBuffer types,
        java.nio.IntBuffer ids,
        java.nio.IntBuffer severities);

