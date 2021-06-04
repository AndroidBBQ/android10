/* void glDispatchComputeIndirect ( GLintptr indirect ) */
static void android_glDispatchComputeIndirect(JNIEnv *_env, jobject, jlong indirect) {
    // 'indirect' is a byte offset, not a pointer. GL checks for negative and too-large values.
    // Here we only need to check for successful 64-bit to 32-bit conversion.
    // - jlong is a int64_t (jni.h)
    // - GLintptr is a long (khrplatform.h)
    if (sizeof(GLintptr) != sizeof(jlong) && (indirect < LONG_MIN || indirect > LONG_MAX)) {
        jniThrowException(_env, "java/lang/IllegalArgumentException", "indirect offset too large");
        return;
    }
    glDispatchComputeIndirect((GLintptr)indirect);
}

