/* void glGetActiveUniformBlockName ( GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName ) */
static void
android_glGetActiveUniformBlockName_III_3II_3BI
    (JNIEnv* _env, jobject _this, jint program, jint uniformBlockIndex, int bufSize, jintArray length_ref, jint lengthOffset, jbyteArray name_ref, jint nameOffset) {
    jint _exception = 0;
    const char* _exceptionType;
    const char* _exceptionMessage;
    GLsizei* _length_base = (GLsizei*)0;
    jint _lengthRemaining;
    GLsizei* _length = (GLsizei*)0;
    GLchar* _name_base = (GLchar*)0;
    jint _nameRemaining;
    GLchar* _name = (GLchar*)0;

    if (length_ref) {
        if (lengthOffset < 0) {
            _exception = 1;
            _exceptionType = "java/lang/IllegalArgumentException";
            _exceptionMessage = "lengthOffset < 0";
            goto exit;
        }
        _lengthRemaining = _env->GetArrayLength(length_ref) - lengthOffset;
        _length_base = (GLsizei*)_env->GetIntArrayElements(
                length_ref, (jboolean*)0);
        _length = _length_base + lengthOffset;
    }

    if (!name_ref) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "uniformBlockName == null";
        goto exit;
    }
    if (nameOffset < 0) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "uniformBlockNameOffset < 0";
        goto exit;
    }
    _nameRemaining = _env->GetArrayLength(name_ref) - nameOffset;
    _name_base = (GLchar*)_env->GetByteArrayElements(
            name_ref, (jboolean*)0);
    _name = _name_base + nameOffset;

    glGetActiveUniformBlockName(
        (GLuint)program,
        (GLuint)uniformBlockIndex,
        (GLsizei)bufSize,
        (GLsizei*)_length,
        (GLchar*)_name
    );

exit:
    if (_name_base) {
        _env->ReleaseByteArrayElements(name_ref, (jbyte*)_name_base,
            _exception ? JNI_ABORT: 0);
    }
    if (_length_base) {
        _env->ReleaseIntArrayElements(length_ref, (jint*)_length_base,
            _exception ? JNI_ABORT: 0);
    }
    if (_exception) {
        jniThrowException(_env, _exceptionType, _exceptionMessage);
    }
}

/* void glGetActiveUniformBlockName ( GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName ) */
static void
android_glGetActiveUniformBlockName_IILjava_nio_Buffer_2Ljava_nio_Buffer_2
    (JNIEnv* _env, jobject _this, jint program, jint uniformBlockIndex, jobject length_buf, jobject uniformBlockName_buf) {
    jint _exception = 0;
    const char* _exceptionType;
    const char* _exceptionMessage;
    jarray _lengthArray = (jarray)0;
    jint _lengthBufferOffset = (jint)0;
    GLsizei* _length = (GLsizei*)0;
    jint _lengthRemaining;
    jarray _nameArray = (jarray)0;
    jint _nameBufferOffset = (jint)0;
    GLchar* _name = (GLchar*)0;
    jint _nameRemaining;

    _length = (GLsizei*)getPointer(_env, length_buf, &_lengthArray, &_lengthRemaining, &_lengthBufferOffset);
    if (_length == NULL) {
        GLsizei* _lengthBase = (GLsizei*)_env->GetPrimitiveArrayCritical(_lengthArray, (jboolean*)0);
        _length = (GLsizei*)(_lengthBase + _lengthBufferOffset);
    }

    _name = (GLchar*)getPointer(_env, uniformBlockName_buf, &_nameArray, &_nameRemaining, &_nameBufferOffset);
    if (_name == NULL) {
        GLchar* _nameBase = (GLchar*)_env->GetPrimitiveArrayCritical(_nameArray, (jboolean*)0);
        _name = (GLchar*)(_nameBase + _nameBufferOffset);
    }

    glGetActiveUniformBlockName(
        (GLuint)program,
        (GLuint)uniformBlockIndex,
        (GLsizei)_nameRemaining,
        _length, _name
    );
    if (_nameArray) {
        releasePointer(_env, _nameArray, _name, JNI_TRUE);
    }
    if (_lengthArray) {
        releasePointer(_env, _lengthArray, _length, JNI_TRUE);
    }
}

/* void glGetActiveUniformBlockName ( GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName ) */
static jstring
android_glGetActiveUniformBlockName_II
    (JNIEnv *_env, jobject _this, jint program, jint uniformBlockIndex) {
    GLint len = 0;
    glGetActiveUniformBlockiv((GLuint)program, (GLuint)uniformBlockIndex,
            GL_UNIFORM_BLOCK_NAME_LENGTH, &len);
    GLchar* name = (GLchar*)malloc(len);
    glGetActiveUniformBlockName((GLuint)program, (GLuint)uniformBlockIndex,
        len, NULL, name);
    jstring result = _env->NewStringUTF(name);
    free(name);
    return result;
}
