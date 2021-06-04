/* void glGetFloatv ( GLenum pname, GLfloat *params ) */
static void
android_glGetFloatv__I_3FI
  (JNIEnv *_env, jobject _this, jint pname, jfloatArray params_ref, jint offset) {
    get<jfloatArray, FloatArrayGetter, jfloat*, FloatArrayReleaser, GLfloat, glGetFloatv>(
        _env, _this, pname, params_ref, offset);
}

/* void glGetFloatv ( GLenum pname, GLfloat *params ) */
static void
android_glGetFloatv__ILjava_nio_FloatBuffer_2
  (JNIEnv *_env, jobject _this, jint pname, jobject params_buf) {
    getarray<GLfloat, jfloatArray, FloatArrayGetter, jfloat*, FloatArrayReleaser, glGetFloatv>(
        _env, _this, pname, params_buf);
}
