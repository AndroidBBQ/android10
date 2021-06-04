/* void glDrawElementsInstanced ( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instanceCount ) */
static void
android_glDrawElementsInstanced__IIILjava_nio_Buffer_2I
  (JNIEnv *_env, jobject _this, jint mode, jint count, jint type, jobject indices_buf, jint instanceCount) {
    jarray _array = (jarray) 0;
    jint _bufferOffset = (jint) 0;
    jint _remaining;
    GLvoid *indices = (GLvoid *) 0;

    indices = (GLvoid *)getPointer(_env, indices_buf, &_array, &_remaining, &_bufferOffset);
    if (indices == NULL) {
        char * _indicesBase = (char *)_env->GetPrimitiveArrayCritical(_array, (jboolean *) 0);
        indices = (GLvoid *) (_indicesBase + _bufferOffset);
    }
    glDrawElementsInstanced(
        (GLenum)mode,
        (GLsizei)count,
        (GLenum)type,
        (GLvoid *)indices,
        (GLsizei)instanceCount
    );
    if (_array) {
        releasePointer(_env, _array, indices, JNI_FALSE);
    }
}

/* void glDrawElementsInstanced ( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instanceCount ) */
static void
android_glDrawElementsInstanced__IIIII
  (JNIEnv *_env, jobject _this, jint mode, jint count, jint type, jint indicesOffset, jint instanceCount) {
    glDrawElementsInstanced(
        (GLenum)mode,
        (GLsizei)count,
        (GLenum)type,
        (GLvoid *)static_cast<uintptr_t>(indicesOffset),
        (GLsizei)instanceCount
    );
}
