/* void glGetBufferPointerv ( GLenum target, GLenum pname, GLvoid** params ) */
static jobject
android_glGetBufferPointerv__II
  (JNIEnv *_env, jobject _this, jint target, jint pname) {
    GLint64 _mapLength;
    GLvoid* _p;
    glGetBufferParameteri64v((GLenum)target, GL_BUFFER_MAP_LENGTH, &_mapLength);
    glGetBufferPointerv((GLenum)target, (GLenum)pname, &_p);
    return _env->NewDirectByteBuffer(_p, _mapLength);
}

