/* void glDrawElementsInstancedBaseVertex ( GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instanceCount, GLint basevertex ) */
static void
android_glDrawElementsInstancedBaseVertex__IIILjava_nio_Buffer_2II
  (JNIEnv *_env, jobject _this, jint mode, jint count, jint type, jobject indices_buf, jint instanceCount, jint basevertex) {
    jint _exception = 0;
    const char * _exceptionType = NULL;
    const char * _exceptionMessage = NULL;
    jarray _array = (jarray) 0;
    jint _bufferOffset = (jint) 0;
    jint _remaining;
    void *indices = (void *) 0;

    indices = (void *)getPointer(_env, indices_buf, &_array, &_remaining, &_bufferOffset);
    if (_remaining < count-basevertex) {
        _exception = 1;
        _exceptionType = "java/lang/ArrayIndexOutOfBoundsException";
        _exceptionMessage = "remaining() < count-basevertex < needed";
        goto exit;
    }
    if (indices == NULL) {
        char * _indicesBase = (char *)_env->GetPrimitiveArrayCritical(_array, (jboolean *) 0);
        indices = (void *) (_indicesBase + _bufferOffset);
    }
    glDrawElementsInstancedBaseVertex(
        (GLenum)mode,
        (GLsizei)count,
        (GLenum)type,
        (void *)indices,
        (GLsizei)instanceCount,
        (GLint) basevertex
    );

exit:
    if (_array) {
        releasePointer(_env, _array, indices, JNI_FALSE);
    }
}

/* void glDrawElementsInstancedBaseVertex ( GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instanceCount, GLint basevertex ) */
static void
android_glDrawElementsInstancedBaseVertex__IIIIII
  (JNIEnv *_env, jobject _this, jint mode, jint count, jint type, jint indicesOffset, jint instanceCount, jint basevertex) {
    glDrawElementsInstancedBaseVertex(
        (GLenum)mode,
        (GLsizei)count,
        (GLenum)type,
        (void *)static_cast<uintptr_t>(indicesOffset),
        (GLsizei)instanceCount,
        (GLint)basevertex
    );
}
