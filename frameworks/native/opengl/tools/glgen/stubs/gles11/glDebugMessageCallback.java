    // C function void glDebugMessageCallback ( GLDEBUGPROC callback, const void *userParam )

    public interface DebugProc {
        void onMessage(int source, int type, int id, int severity, String message);
    }

    public static native void glDebugMessageCallback(DebugProc callback);

