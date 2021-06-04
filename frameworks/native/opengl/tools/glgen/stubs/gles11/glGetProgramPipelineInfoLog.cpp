#include <stdlib.h>

/* void glGetProgramPipelineInfoLog ( GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog ) */
static jstring android_glGetProgramPipelineInfoLog(JNIEnv *_env, jobject, jint shader) {
    GLint infoLen = 0;
    glGetProgramPipelineiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    if (!infoLen) {
        return _env->NewStringUTF("");
    }
    char* buf = (char*) malloc(infoLen);
    if (buf == NULL) {
        jniThrowException(_env, "java/lang/OutOfMemoryError", "out of memory");
        return NULL;
    }
    glGetProgramPipelineInfoLog(shader, infoLen, NULL, buf);
    jstring result = _env->NewStringUTF(buf);
    free(buf);
    return result;
}
