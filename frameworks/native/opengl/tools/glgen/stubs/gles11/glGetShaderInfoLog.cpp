#include <stdlib.h>

/* void glGetShaderInfoLog ( GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog ) */
static jstring android_glGetShaderInfoLog(JNIEnv *_env, jobject, jint shader) {
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    if (!infoLen) {
        infoLen = 512;
    }
    char* buf = (char*) malloc(infoLen);
    if (buf == NULL) {
        jniThrowException(_env, "java/lang/IllegalArgumentException", "out of memory");
        return NULL;
    }
    GLsizei outLen = 0;
    glGetShaderInfoLog(shader, infoLen, &outLen, buf);
    jstring result = _env->NewStringUTF(outLen == 0 ? "" : buf);
    free(buf);
    return result;
}
