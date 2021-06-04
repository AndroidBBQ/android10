/* void glBindVertexBuffer ( GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride ) */
static void
android_glBindVertexBuffer__IIJI
  (JNIEnv *_env, jobject _this, jint bindingindex, jint buffer, jlong offset, jint stride) {
    if (sizeof(GLintptr) != sizeof(jlong) && (offset < LONG_MIN || offset > LONG_MAX)) {
        jniThrowException(_env, "java/lang/IllegalArgumentException", "offset too large");
        return;
    }
    glBindVertexBuffer(
        (GLuint)bindingindex,
        (GLuint)buffer,
        (GLintptr)offset,
        (GLsizei)stride
    );
}
