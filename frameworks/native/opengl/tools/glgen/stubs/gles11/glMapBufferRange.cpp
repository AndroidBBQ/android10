/* GLvoid * glMapBufferRange ( GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access ) */
static jobject
android_glMapBufferRange__IIII
  (JNIEnv *_env, jobject _this, jint target, jint offset, jint length, jint access) {
    GLvoid* _p = glMapBufferRange((GLenum)target,
            (GLintptr)offset, (GLsizeiptr)length, (GLbitfield)access);
    jobject _buf = (jobject)0;
    if (_p) {
        _buf = _env->NewDirectByteBuffer(_p, length);
    }
    return _buf;
}

