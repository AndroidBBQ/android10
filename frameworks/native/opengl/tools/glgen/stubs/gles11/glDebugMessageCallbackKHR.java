    // C function void glDebugMessageCallbackKHR ( GLDEBUGPROCKHR callback, const void *userParam )

    public interface DebugProcKHR {
        void onMessage(int source, int type, int id, int severity, String message);
    }

    public static native void glDebugMessageCallbackKHR(DebugProcKHR callback);

