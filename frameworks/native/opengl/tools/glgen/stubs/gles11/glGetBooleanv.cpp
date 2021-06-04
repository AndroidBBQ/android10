/* void glGetBooleanv ( GLenum pname, GLboolean *params ) */
static void
android_glGetBooleanv__I_3ZI
  (JNIEnv *_env, jobject _this, jint pname, jbooleanArray params_ref, jint offset) {
    get<jbooleanArray, BooleanArrayGetter, jboolean*, BooleanArrayReleaser, GLboolean, glGetBooleanv>(
        _env, _this, pname, params_ref, offset);
}

/* void glGetBooleanv ( GLenum pname, GLboolean *params ) */
static void
android_glGetBooleanv__ILjava_nio_IntBuffer_2
  (JNIEnv *_env, jobject _this, jint pname, jobject params_buf) {
    getarray<GLboolean, jintArray, IntArrayGetter, jint*, IntArrayReleaser, glGetBooleanv>(
        _env, _this, pname, params_buf);
}
