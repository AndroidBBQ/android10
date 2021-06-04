/* const GLubyte * glGetStringi ( GLenum name, GLuint index ) */
static jstring
android_glGetStringi__II
  (JNIEnv *_env, jobject _this, jint name, jint index) {
    const GLubyte* _chars = glGetStringi((GLenum)name, (GLuint)index);
    return _env->NewStringUTF((const char*)_chars);
}

