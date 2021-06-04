/* void glGetTransformFeedbackVarying ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name ) */
static void
android_glGetTransformFeedbackVarying__III_3II_3II_3II_3BI
  (JNIEnv *_env, jobject _this, jint program, jint index, jint bufsize, jintArray length_ref, jint lengthOffset, jintArray size_ref, jint sizeOffset, jintArray type_ref, jint typeOffset, jbyteArray name_ref, jint nameOffset) {
    jint _exception = 0;
    const char * _exceptionType;
    const char * _exceptionMessage;
    GLsizei *length_base = (GLsizei *) 0;
    jint _lengthRemaining;
    GLsizei *length = (GLsizei *) 0;
    GLint *size_base = (GLint *) 0;
    jint _sizeRemaining;
    GLint *size = (GLint *) 0;
    GLenum *type_base = (GLenum *) 0;
    jint _typeRemaining;
    GLenum *type = (GLenum *) 0;
    char *name_base = (char *) 0;
    jint _nameRemaining;
    char *name = (char *) 0;

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

    if (!size_ref) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "size == null";
        goto exit;
    }
    if (sizeOffset < 0) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "sizeOffset < 0";
        goto exit;
    }
    _sizeRemaining = _env->GetArrayLength(size_ref) - sizeOffset;
    size_base = (GLint *)
        _env->GetIntArrayElements(size_ref, (jboolean *)0);
    size = size_base + sizeOffset;

    if (!type_ref) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "type == null";
        goto exit;
    }
    if (typeOffset < 0) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "typeOffset < 0";
        goto exit;
    }
    _typeRemaining = _env->GetArrayLength(type_ref) - typeOffset;
    type_base = (GLenum *)
        _env->GetIntArrayElements(type_ref, (jboolean *)0);
    type = type_base + typeOffset;

    if (!name_ref) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "name == null";
        goto exit;
    }
    if (nameOffset < 0) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "nameOffset < 0";
        goto exit;
    }
    _nameRemaining = _env->GetArrayLength(name_ref) - nameOffset;
    name_base = (char *)
        _env->GetByteArrayElements(name_ref, (jboolean *)0);
    name = name_base + nameOffset;

    glGetTransformFeedbackVarying(
        (GLuint)program,
        (GLuint)index,
        (GLsizei)bufsize,
        (GLsizei *)length,
        (GLint *)size,
        (GLenum *)type,
        (char *)name
    );

exit:
    if (name_base) {
        _env->ReleaseByteArrayElements(name_ref, (jbyte*)name_base,
            _exception ? JNI_ABORT: 0);
    }
    if (type_base) {
        _env->ReleaseIntArrayElements(type_ref, (jint*)type_base,
            _exception ? JNI_ABORT: 0);
    }
    if (size_base) {
        _env->ReleaseIntArrayElements(size_ref, (jint*)size_base,
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

/* void glGetTransformFeedbackVarying ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name ) */
static void
android_glGetTransformFeedbackVarying__IIILjava_nio_IntBuffer_2Ljava_nio_IntBuffer_2Ljava_nio_IntBuffer_2B
  (JNIEnv *_env, jobject _this, jint program, jint index, jint bufsize, jobject length_buf, jobject size_buf, jobject type_buf, jbyte name) {
    jniThrowException(_env, "java/lang/UnsupportedOperationException", "deprecated");
}

/* void glGetTransformFeedbackVarying ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name ) */
static void
android_glGetTransformFeedbackVarying__IIILjava_nio_IntBuffer_2Ljava_nio_IntBuffer_2Ljava_nio_IntBuffer_2Ljava_nio_ByteBuffer_2
  (JNIEnv *_env, jobject _this, jint program, jint index, jint bufsize, jobject length_buf, jobject size_buf, jobject type_buf, jobject name_buf) {
    jintArray _lengthArray = (jintArray) 0;
    jint _lengthBufferOffset = (jint) 0;
    jintArray _sizeArray = (jintArray) 0;
    jint _sizeBufferOffset = (jint) 0;
    jintArray _typeArray = (jintArray) 0;
    jint _typeBufferOffset = (jint) 0;
    jbyteArray _nameArray = (jbyteArray)0;
    jint _nameBufferOffset = (jint)0;
    jint _lengthRemaining;
    GLsizei *length = (GLsizei *) 0;
    jint _sizeRemaining;
    GLint *size = (GLint *) 0;
    jint _typeRemaining;
    GLenum *type = (GLenum *) 0;
    jint _nameRemaining;
    GLchar* name = (GLchar*)0;


    length = (GLsizei *)getPointer(_env, length_buf, (jarray*)&_lengthArray, &_lengthRemaining, &_lengthBufferOffset);
    size = (GLint *)getPointer(_env, size_buf, (jarray*)&_sizeArray, &_sizeRemaining, &_sizeBufferOffset);
    type = (GLenum *)getPointer(_env, type_buf, (jarray*)&_typeArray, &_typeRemaining, &_typeBufferOffset);
    name = (GLchar*)getPointer(_env, name_buf, (jarray*)&_nameArray, &_nameRemaining, &_nameBufferOffset);
    if (length == NULL) {
        char * _lengthBase = (char *)_env->GetIntArrayElements(_lengthArray, (jboolean *) 0);
        length = (GLsizei *) (_lengthBase + _lengthBufferOffset);
    }
    if (size == NULL) {
        char * _sizeBase = (char *)_env->GetIntArrayElements(_sizeArray, (jboolean *) 0);
        size = (GLint *) (_sizeBase + _sizeBufferOffset);
    }
    if (type == NULL) {
        char * _typeBase = (char *)_env->GetIntArrayElements(_typeArray, (jboolean *) 0);
        type = (GLenum *) (_typeBase + _typeBufferOffset);
    }
    if (name == NULL) {
        char* _nameBase = (char *)_env->GetByteArrayElements(_nameArray, (jboolean*)0);
        name = (GLchar *) (_nameBase + _nameBufferOffset);
    }
    glGetTransformFeedbackVarying(
        (GLuint)program,
        (GLuint)index,
        (GLsizei)bufsize,
        (GLsizei *)length,
        (GLint *)size,
        (GLenum *)type,
        (GLchar*)name
    );
    if (_typeArray) {
        releaseArrayPointer<jintArray, jint*, IntArrayReleaser>(_env, _typeArray, (jint*)type, JNI_TRUE);
    }
    if (_sizeArray) {
        releaseArrayPointer<jintArray, jint*, IntArrayReleaser>(_env, _sizeArray, (jint*)size, JNI_TRUE);
    }
    if (_lengthArray) {
        releaseArrayPointer<jintArray, jint*, IntArrayReleaser>(_env, _lengthArray, (jint*)length, JNI_TRUE);
    }
    if (_nameArray) {
        releaseArrayPointer<jbyteArray, jbyte*, ByteArrayReleaser>(_env, _nameArray, (jbyte*)name, JNI_TRUE);
    }
}

/* void glGetTransformFeedbackVarying ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name ) */
static jstring
android_glGetTransformFeedbackVarying1
  (JNIEnv *_env, jobject _this, jint program, jint index, jintArray size_ref, jint sizeOffset, jintArray type_ref, jint typeOffset) {
    jint _exception = 0;
    const char * _exceptionType;
    const char * _exceptionMessage;
    GLint *size_base = (GLint *) 0;
    jint _sizeRemaining;
    GLint *size = (GLint *) 0;
    GLenum *type_base = (GLenum *) 0;
    jint _typeRemaining;
    GLenum *type = (GLenum *) 0;

    jstring result = 0;

    GLint len = 0;
    glGetProgramiv((GLuint)program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &len);
    if (!len) {
        return _env->NewStringUTF("");
    }
    char* buf = (char*) malloc(len);

    if (buf == NULL) {
        jniThrowException(_env, "java/lang/IllegalArgumentException", "out of memory");
        return NULL;
    }
    if (!size_ref) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "size == null";
        goto exit;
    }
    if (sizeOffset < 0) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "sizeOffset < 0";
        goto exit;
    }
    _sizeRemaining = _env->GetArrayLength(size_ref) - sizeOffset;
    size_base = (GLint *)
        _env->GetIntArrayElements(size_ref, (jboolean *)0);
    size = size_base + sizeOffset;

    if (!type_ref) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "type == null";
        goto exit;
    }
    if (typeOffset < 0) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "typeOffset < 0";
        goto exit;
    }
    _typeRemaining = _env->GetArrayLength(type_ref) - typeOffset;
    type_base = (GLenum *)
        _env->GetIntArrayElements(type_ref, (jboolean *)0);
    type = type_base + typeOffset;

    glGetTransformFeedbackVarying(
        (GLuint)program,
        (GLuint)index,
        (GLsizei)len,
        NULL,
        (GLint *)size,
        (GLenum *)type,
        (char *)buf
    );
exit:
    if (type_base) {
        _env->ReleaseIntArrayElements(type_ref, (jint*)type_base,
            _exception ? JNI_ABORT: 0);
    }
    if (size_base) {
        _env->ReleaseIntArrayElements(size_ref, (jint*)size_base,
            _exception ? JNI_ABORT: 0);
    }
    if (_exception != 1) {
        result = _env->NewStringUTF(buf);
    }
    if (buf) {
        free(buf);
    }
    if (_exception) {
        jniThrowException(_env, _exceptionType, _exceptionMessage);
    }
    if (result == 0) {
        result = _env->NewStringUTF("");
    }

    return result;
}

/* void glGetTransformFeedbackVarying ( GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name ) */
static jstring
android_glGetTransformFeedbackVarying2
  (JNIEnv *_env, jobject _this, jint program, jint index, jobject size_buf, jobject type_buf) {
    jintArray _sizeArray = (jintArray) 0;
    jint _sizeBufferOffset = (jint) 0;
    jintArray _typeArray = (jintArray) 0;
    jint _typeBufferOffset = (jint) 0;
    jint _lengthRemaining;
    GLsizei *length = (GLsizei *) 0;
    jint _sizeRemaining;
    GLint *size = (GLint *) 0;
    jint _typeRemaining;
    GLenum *type = (GLenum *) 0;

    jstring result = 0;

    GLint len = 0;
    glGetProgramiv((GLuint)program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &len);
    if (!len) {
        return _env->NewStringUTF("");
    }
    char* buf = (char*) malloc(len);

    if (buf == NULL) {
        jniThrowException(_env, "java/lang/IllegalArgumentException", "out of memory");
        return NULL;
    }

    size = (GLint *)getPointer(_env, size_buf, (jarray*)&_sizeArray, &_sizeRemaining, &_sizeBufferOffset);
    type = (GLenum *)getPointer(_env, type_buf, (jarray*)&_typeArray, &_typeRemaining, &_typeBufferOffset);
    if (size == NULL) {
        char * _sizeBase = (char *)_env->GetIntArrayElements(_sizeArray, (jboolean *) 0);
        size = (GLint *) (_sizeBase + _sizeBufferOffset);
    }
    if (type == NULL) {
        char * _typeBase = (char *)_env->GetIntArrayElements(_typeArray, (jboolean *) 0);
        type = (GLenum *) (_typeBase + _typeBufferOffset);
    }
    glGetTransformFeedbackVarying(
        (GLuint)program,
        (GLuint)index,
        (GLsizei)len,
        NULL,
        (GLint *)size,
        (GLenum *)type,
        (char *)buf
    );

    if (_typeArray) {
        releaseArrayPointer<jintArray, jint*, IntArrayReleaser>(_env, _typeArray, (jint*)type, JNI_TRUE);
    }
    if (_sizeArray) {
        releaseArrayPointer<jintArray, jint*, IntArrayReleaser>(_env, _sizeArray, (jint*)size, JNI_TRUE);
    }
    result = _env->NewStringUTF(buf);
    if (buf) {
        free(buf);
    }
    return result;
}
