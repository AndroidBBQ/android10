    // C function void glDrawElementsInstanced ( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instanceCount )

    public static native void glDrawElementsInstanced(
        int mode,
        int count,
        int type,
        java.nio.Buffer indices,
        int instanceCount
    );

    // C function void glDrawElementsInstanced ( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instanceCount )

    public static native void glDrawElementsInstanced(
        int mode,
        int count,
        int type,
        int indicesOffset,
        int instanceCount
    );

