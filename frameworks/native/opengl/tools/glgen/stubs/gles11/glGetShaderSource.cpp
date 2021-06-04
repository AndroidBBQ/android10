/* void glGetShaderSource ( GLuint shader, GLsizei bufsize, GLsizei *length, char *source ) */
static void
android_glGetShaderSource__II_3II_3BI
  (JNIEnv *_env, jobject _this, jint shader, jint bufsize, jintArray length_ref, jint lengthOffset, jbyteArray source_ref, jint sourceOffset) {
    jint _exception = 0;
    const char * _exceptionType;
    const char * _exceptionMessage;
    GLsizei *length_base = (GLsizei *) 0;
    jint _lengthRemaining;
    GLsizei *length = (GLsizei *) 0;
    char *source_base = (char *) 0;
    jint _sourceRemaining;
    char *source = (char *) 0;

    if (length_ref) {
        if (lengthOffset < 0) {
            _exception = 1;
            _exceptionType = "java/lang/IllegalArgumentException";
            _exceptionMessage = "lengthOffset < 0";
            goto exit;
        }
        _lengthRemaining = _env->GetArrayLength(length_ref) - lengthOffset;
        length_base = (GLsizei *)
            _env->GetIntArrayElements(length_ref, (jboolean *)0);
        length = length_base + lengthOffset;
    }

    if (!source_ref) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "source == null";
        goto exit;
    }
    if (sourceOffset < 0) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "sourceOffset < 0";
        goto exit;
    }
    _sourceRemaining = _env->GetArrayLength(source_ref) - sourceOffset;
    source_base = (char *)
        _env->GetByteArrayElements(source_ref, (jboolean *)0);
    source = source_base + sourceOffset;

    glGetShaderSource(
        (GLuint)shader,
        (GLsizei)bufsize,
        (GLsizei *)length,
        (char *)source
    );

exit:
    if (source_base) {
        _env->ReleaseByteArrayElements(source_ref, (jbyte*)source_base,
            _exception ? JNI_ABORT: 0);
    }
    if (length_base) {
        _env->ReleaseIntArrayElements(length_ref, (jint*)length_base,
            _exception ? JNI_ABORT: 0);
    }
    if (_exception) {
        jniThrowException(_env, _exceptionType, _exceptionMessage);
    }
}

/* void glGetShaderSource ( GLuint shader, GLsizei bufsize, GLsizei *length, char *source ) */
static void
android_glGetShaderSource__IILjava_nio_IntBuffer_2B
  (JNIEnv *_env, jobject _this, jint shader, jint bufsize, jobject length_buf, jbyte source) {
    jintArray _array = (jintArray) 0;
    jint _bufferOffset = (jint) 0;
    jint _remaining;
    GLsizei *length = (GLsizei *) 0;

    length = (GLsizei *)getPointer(_env, length_buf, (jarray*)&_array, &_remaining, &_bufferOffset);
    if (length == NULL) {
        char * _lengthBase = (char *)_env->GetIntArrayElements(_array, (jboolean *) 0);
        length = (GLsizei *) (_lengthBase + _bufferOffset);
    }
    glGetShaderSource(
        (GLuint)shader,
        (GLsizei)bufsize,
        (GLsizei *)length,
        reinterpret_cast<char *>(source)
    );
    if (_array) {
        releaseArrayPointer<jintArray, jint*, IntArrayReleaser>(_env, _array, (jint*)length, JNI_TRUE);
    }
}

/* void glGetShaderSource ( GLuint shader, GLsizei bufsize, GLsizei *length, char *source ) */
static jstring android_glGetShaderSource(JNIEnv *_env, jobject, jint shader) {
    GLint shaderLen = 0;
    glGetShaderiv((GLuint)shader, GL_SHADER_SOURCE_LENGTH, &shaderLen);
    if (!shaderLen) {
        return _env->NewStringUTF("");
    }
    char* buf = (char*) malloc(shaderLen);
    if (buf == NULL) {
        jniThrowException(_env, "java/lang/IllegalArgumentException", "out of memory");
        return NULL;
    }
    glGetShaderSource(shader, shaderLen, NULL, buf);
    jstring result = _env->NewStringUTF(buf);
    free(buf);
    return result;
}
