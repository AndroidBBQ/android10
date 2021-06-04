/* void glTransformFeedbackVaryings ( GLuint program, GLsizei count, const GLchar *varyings, GLenum bufferMode ) */
static
void
android_glTransformFeedbackVaryings
    (JNIEnv *_env, jobject _this, jint program, jobjectArray varyings_ref, jint bufferMode) {
    jint _exception = 0;
    const char* _exceptionType = NULL;
    const char* _exceptionMessage = NULL;
    jint _count = 0, _i;
    const char** _varyings = NULL;
    const char* _varying = NULL;

    if (!varyings_ref) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "varyings == null";
        goto exit;
    }

    _count = _env->GetArrayLength(varyings_ref);
    _varyings = (const char**)calloc(_count, sizeof(const char*));
    for (_i = 0; _i < _count; _i++) {
        jstring _varying = (jstring)_env->GetObjectArrayElement(varyings_ref, _i);
        if (!_varying) {
            _exception = 1;
            _exceptionType = "java/lang/IllegalArgumentException";
            _exceptionMessage = "null varyings element";
            goto exit;
        }
        _varyings[_i] = _env->GetStringUTFChars(_varying, 0);
    }

    glTransformFeedbackVaryings(program, _count, _varyings, bufferMode);

exit:
    for (_i = _count - 1; _i >= 0; _i--) {
        if (_varyings[_i]) {
            jstring _varying = (jstring)_env->GetObjectArrayElement(varyings_ref, _i);
            if (_varying) {
                _env->ReleaseStringUTFChars(_varying, _varyings[_i]);
            }
        }
    }
    free(_varyings);
    if (_exception) {
        jniThrowException(_env, _exceptionType, _exceptionMessage);
    }
}

