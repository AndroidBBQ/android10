/* void glGetUniformIndices ( GLuint program, GLsizei uniformCount, const GLchar *const *uniformNames, GLuint *uniformIndices ) */
static
void
android_glGetUniformIndices_array
    (JNIEnv *_env, jobject _this, jint program, jobjectArray uniformNames_ref, jintArray uniformIndices_ref, jint uniformIndicesOffset) {
    jint _exception = 0;
    const char* _exceptionType = NULL;
    const char* _exceptionMessage = NULL;
    jint _count = 0;
    jint _i;
    const char** _names = NULL;
    GLuint* _indices_base = NULL;
    GLuint* _indices = NULL;

    if (!uniformNames_ref) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "uniformNames == null";
        goto exit;
    }
    _count = _env->GetArrayLength(uniformNames_ref);
    _names = (const char**)calloc(_count, sizeof(const char*));
    for (_i = 0; _i < _count; _i++) {
        jstring _name = (jstring)_env->GetObjectArrayElement(uniformNames_ref, _i);
        if (!_name) {
            _exception = 1;
            _exceptionType = "java/lang/IllegalArgumentException";
            _exceptionMessage = "null uniformNames element";
            goto exit;
        }
        _names[_i] = _env->GetStringUTFChars(_name, 0);
    }

    if (!uniformIndices_ref) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "uniformIndices == null";
        goto exit;
    }
    if (uniformIndicesOffset < 0) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "uniformIndicesOffset < 0";
        goto exit;
    }
    if (_env->GetArrayLength(uniformIndices_ref) - uniformIndicesOffset < _count) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "not enough space in uniformIndices";
        goto exit;
    }
    _indices_base = (GLuint*)_env->GetIntArrayElements(
            uniformIndices_ref, 0);
    _indices = _indices_base + uniformIndicesOffset;

    glGetUniformIndices(program, _count, _names, _indices);

exit:
    if (_indices_base) {
        _env->ReleaseIntArrayElements(uniformIndices_ref, (jint*)_indices_base,
            _exception ? JNI_ABORT : 0);
    }
    for (_i = _count - 1; _i >= 0; _i--) {
        if (_names[_i]) {
            jstring _name = (jstring)_env->GetObjectArrayElement(uniformNames_ref, _i);
            if (_name) {
                _env->ReleaseStringUTFChars(_name, _names[_i]);
            }
        }
    }
    free(_names);
    if (_exception) {
        jniThrowException(_env, _exceptionType, _exceptionMessage);
    }
}

/* void glGetUniformIndices ( GLuint program, GLsizei uniformCount, const GLchar *const *uniformNames, GLuint *uniformIndices ) */
static
void
android_glGetUniformIndices_buffer
    (JNIEnv *_env, jobject _this, jint program, jobjectArray uniformNames_ref, jobject uniformIndices_buf) {
    jint _exception = 0;
    const char* _exceptionType = NULL;
    const char* _exceptionMessage = NULL;
    jint _count = 0;
    jint _i;
    const char** _names = NULL;
    jintArray _uniformIndicesArray = (jintArray)0;
    jint _uniformIndicesRemaining;
    jint _uniformIndicesOffset = 0;
    GLuint* _indices = NULL;
    char* _indicesBase = NULL;

    if (!uniformNames_ref) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "uniformNames == null";
        goto exit;
    }
    if (!uniformIndices_buf) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "uniformIndices == null";
        goto exit;
    }

    _count = _env->GetArrayLength(uniformNames_ref);
    _names = (const char**)calloc(_count, sizeof(const char*));
    for (_i = 0; _i < _count; _i++) {
        jstring _name = (jstring)_env->GetObjectArrayElement(uniformNames_ref, _i);
        if (!_name) {
            _exception = 1;
            _exceptionType = "java/lang/IllegalArgumentException";
            _exceptionMessage = "null uniformNames element";
            goto exit;
        }
        _names[_i] = _env->GetStringUTFChars(_name, 0);
    }

    _indices = (GLuint*)getPointer(_env, uniformIndices_buf,
            (jarray*)&_uniformIndicesArray, &_uniformIndicesRemaining,
            &_uniformIndicesOffset);
    if (!_indices) {
        _indicesBase = (char*)_env->GetIntArrayElements(
            _uniformIndicesArray, 0);
        _indices = (GLuint*)(_indicesBase + _uniformIndicesOffset);
    }
    if (_uniformIndicesRemaining < _count) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "not enough space in uniformIndices";
        goto exit;
    }

    glGetUniformIndices(program, _count, _names, _indices);

exit:
    if (_uniformIndicesArray) {
        releaseArrayPointer<jintArray, jint*, IntArrayReleaser>(
            _env, _uniformIndicesArray, (jint*)_indicesBase, JNI_TRUE);
    }
    for (_i = _count - 1; _i >= 0; _i--) {
        if (_names[_i]) {
            jstring _name = (jstring)_env->GetObjectArrayElement(uniformNames_ref, _i);
            if (_name) {
                _env->ReleaseStringUTFChars(_name, _names[_i]);
            }
        }
    }
    free(_names);
    if (_exception) {
        jniThrowException(_env, _exceptionType, _exceptionMessage);
    }
}
