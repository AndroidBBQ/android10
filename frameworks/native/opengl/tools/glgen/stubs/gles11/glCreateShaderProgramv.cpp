/* GLuint glCreateShaderProgramv ( GLenum type, GLsizei count, const GLchar *const *strings ) */
static jint
android_glCreateShaderProgramv
  (JNIEnv *_env, jobject _this, jint type, jobjectArray strings) {
    jint _exception = 0;
    const char * _exceptionType = NULL;
    const char * _exceptionMessage = NULL;
    GLsizei _count;
    const GLchar** _strings = NULL;
    jstring* _jstrings = NULL;
    GLuint _returnValue = 0;

    if (!strings) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "strings == null";
        goto exit;
    }

    _count = _env->GetArrayLength(strings);

    _strings = (const GLchar**) calloc(_count, sizeof(const GLchar*));
    if (!_strings) {
        _exception = 1;
        _exceptionType = "java/lang/OutOfMemoryError";
        _exceptionMessage = "out of memory";
        goto exit;
    }

    _jstrings = (jstring*) calloc(_count, sizeof(jstring));
    if (!_jstrings) {
        _exception = 1;
        _exceptionType = "java/lang/OutOfMemoryError";
        _exceptionMessage = "out of memory";
        goto exit;
    }

    for(int i = 0; i < _count; i++) {
        _jstrings[i] = (jstring) _env->GetObjectArrayElement(strings, i);
        if (!_jstrings[i]) {
            _exception = 1;
            _exceptionType = "java/lang/IllegalArgumentException";
            _exceptionMessage = "strings == null";
            goto exit;
        }
        _strings[i] = _env->GetStringUTFChars(_jstrings[i], 0);
    }

    _returnValue = glCreateShaderProgramv((GLenum)type, _count, _strings);
exit:
    if (_strings && _jstrings) {
        for(int i = 0; i < _count; i++) {
            if (_strings[i] && _jstrings[i]) {
                _env->ReleaseStringUTFChars(_jstrings[i], _strings[i]);
            }
        }
    }
    if (_strings) {
        free(_strings);
    }
    if (_jstrings) {
        free(_jstrings);
    }
    if (_exception) {
        jniThrowException(_env, _exceptionType, _exceptionMessage);
    }
    return (jint)_returnValue;
}
