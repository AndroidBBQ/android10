/* void glGetIntegerv ( GLenum pname, GLint *params ) */
static void
android_glGetIntegerv__I_3II
  (JNIEnv *_env, jobject _this, jint pname, jintArray params_ref, jint offset) {
    get<jintArray, IntArrayGetter, jint*, IntArrayReleaser, GLint, glGetIntegerv>(
        _env, _this, pname, params_ref, offset);
}

/* void glGetIntegerv ( GLenum pname, GLint *params ) */
static void
android_glGetIntegerv__ILjava_nio_IntBuffer_2
  (JNIEnv *_env, jobject _this, jint pname, jobject params_buf) {
    getarray<GLint, jintArray, IntArrayGetter, jint*, IntArrayReleaser, glGetIntegerv>(
        _env, _this, pname, params_buf);
}
