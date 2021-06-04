/* void glDebugMessageInsertKHR ( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf ) */
static void
android_glDebugMessageInsertKHR__IIIILjava_lang_String_2
  (JNIEnv *_env, jobject _this, jint source, jint type, jint id, jint severity, jstring buf) {
    jint _exception = 0;
    const char * _exceptionType = NULL;
    const char * _exceptionMessage = NULL;
    const char* _nativebuf = 0;
    jint _length = 0;

    if (!buf) {
        _exception = 1;
        _exceptionType = "java/lang/IllegalArgumentException";
        _exceptionMessage = "buf == null";
        goto exit;
    }
    _nativebuf = _env->GetStringUTFChars(buf, 0);
    _length = _env->GetStringUTFLength(buf);

    glDebugMessageInsertKHR(
        (GLenum)source,
        (GLenum)type,
        (GLuint)id,
        (GLenum)severity,
        (GLsizei)_length,
        (GLchar *)_nativebuf
    );

exit:
    if (_nativebuf) {
        _env->ReleaseStringUTFChars(buf, _nativebuf);
    }

    if (_exception) {
        jniThrowException(_env, _exceptionType, _exceptionMessage);
    }
}

