/*
 ** Copyright 2007, The Android Open Source Project
 **
 ** Licensed under the Apache License, Version 2.0 (the "License");
 ** you may not use this file except in compliance with the License.
 ** You may obtain a copy of the License at
 **
 **     http://www.apache.org/licenses/LICENSE-2.0
 **
 ** Unless required by applicable law or agreed to in writing, software
 ** distributed under the License is distributed on an "AS IS" BASIS,
 ** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ** See the License for the specific language governing permissions and
 ** limitations under the License.
 */

#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>

#include <log/log.h>
#include <cutils/properties.h>

#include <GLES/gl.h>
#include <GLES/glext.h>

#include "../hooks.h"
#include "../egl_impl.h"

using namespace android;

// ----------------------------------------------------------------------------
// extensions for the framework
// ----------------------------------------------------------------------------

extern "C" {
GL_API void GL_APIENTRY glColorPointerBounds(GLint size, GLenum type, GLsizei stride,
        const GLvoid *ptr, GLsizei count);
GL_API void GL_APIENTRY glNormalPointerBounds(GLenum type, GLsizei stride,
        const GLvoid *pointer, GLsizei count);
GL_API void GL_APIENTRY glTexCoordPointerBounds(GLint size, GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei count);
GL_API void GL_APIENTRY glVertexPointerBounds(GLint size, GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei count);
GL_API void GL_APIENTRY glPointSizePointerOESBounds(GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei count);
GL_API void GL_APIENTRY glMatrixIndexPointerOESBounds(GLint size, GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei count);
GL_API void GL_APIENTRY glWeightPointerOESBounds(GLint size, GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei count);
}

void glColorPointerBounds(GLint size, GLenum type, GLsizei stride,
        const GLvoid *ptr, GLsizei /*count*/) {
    glColorPointer(size, type, stride, ptr);
}
void glNormalPointerBounds(GLenum type, GLsizei stride,
        const GLvoid *pointer, GLsizei /*count*/) {
    glNormalPointer(type, stride, pointer);
}
void glTexCoordPointerBounds(GLint size, GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei /*count*/) {
    glTexCoordPointer(size, type, stride, pointer);
}
void glVertexPointerBounds(GLint size, GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei /*count*/) {
    glVertexPointer(size, type, stride, pointer);
}

void GL_APIENTRY glPointSizePointerOESBounds(GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei /*count*/) {
    glPointSizePointerOES(type, stride, pointer);
}

GL_API void GL_APIENTRY glMatrixIndexPointerOESBounds(GLint size, GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei /*count*/) {
    glMatrixIndexPointerOES(size, type, stride, pointer);
}

GL_API void GL_APIENTRY glWeightPointerOESBounds(GLint size, GLenum type,
        GLsizei stride, const GLvoid *pointer, GLsizei /*count*/) {
    glWeightPointerOES(size, type, stride, pointer);
}

// ----------------------------------------------------------------------------
// Actual GL entry-points
// ----------------------------------------------------------------------------

#undef API_ENTRY
#undef CALL_GL_API
#undef CALL_GL_API_INTERNAL_CALL
#undef CALL_GL_API_INTERNAL_SET_RETURN_VALUE
#undef CALL_GL_API_INTERNAL_DO_RETURN
#undef CALL_GL_API_RETURN

#if USE_SLOW_BINDING

    #define API_ENTRY(_api) _api

    #define CALL_GL_API_INTERNAL_CALL(_api, ...)                         \
        gl_hooks_t::gl_t const * const _c = &getGlThreadSpecific()->gl;  \
        if (_c) return _c->_api(__VA_ARGS__);

    #define CALL_GL_API_INTERNAL_SET_RETURN_VALUE return 0;

    // This stays blank, since void functions will implicitly return, and
    // all of the other functions will return 0 based on the previous macro.
    #define CALL_GL_API_INTERNAL_DO_RETURN

#elif defined(__arm__)

    #define GET_TLS(reg) "mrc p15, 0, " #reg ", c13, c0, 3 \n"

    #define API_ENTRY(_api) __attribute__((naked,noinline)) _api

    #define CALL_GL_API_INTERNAL_CALL(_api, ...)                 \
        asm volatile(                                            \
            GET_TLS(r12)                                         \
            "ldr   r12, [r12, %[tls]] \n"                        \
            "cmp   r12, #0            \n"                        \
            "ldrne pc,  [r12, %[api]] \n"                        \
            :                                                    \
            : [tls] "J"(TLS_SLOT_OPENGL_API*4),                  \
              [api] "J"(__builtin_offsetof(gl_hooks_t, gl._api)) \
            : "r0", "r1", "r2", "r3", "r12"                      \
        );

    #define CALL_GL_API_INTERNAL_SET_RETURN_VALUE \
        asm volatile(                             \
            "mov r0, #0 \n"                       \
            :                                     \
            :                                     \
            : "r0"                                \
        );


    #define CALL_GL_API_INTERNAL_DO_RETURN \
        asm volatile(                      \
            "bx lr \n"                     \
            :                              \
            :                              \
            : "r0"                         \
        );

#elif defined(__aarch64__)

    #define API_ENTRY(_api) __attribute__((naked,noinline)) _api

    #define CALL_GL_API_INTERNAL_CALL(_api, ...)                    \
        asm volatile(                                               \
            "mrs x16, tpidr_el0\n"                                  \
            "ldr x16, [x16, %[tls]]\n"                              \
            "cbz x16, 1f\n"                                         \
            "ldr x16, [x16, %[api]]\n"                              \
            "br  x16\n"                                             \
            "1:\n"                                                  \
            :                                                       \
            : [tls] "i" (TLS_SLOT_OPENGL_API * sizeof(void*)),      \
              [api] "i" (__builtin_offsetof(gl_hooks_t, gl._api))   \
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x16" \
        );

    #define CALL_GL_API_INTERNAL_SET_RETURN_VALUE \
        asm volatile(                             \
            "mov w0, wzr \n"                      \
            :                                     \
            :                                     \
            : "w0"                                \
        );

    #define CALL_GL_API_INTERNAL_DO_RETURN \
        asm volatile(                      \
            "ret \n"                       \
            :                              \
            :                              \
            :                              \
        );

#elif defined(__i386__)

    #define API_ENTRY(_api) __attribute__((naked,noinline)) _api

    #define CALL_GL_API_INTERNAL_CALL(_api, ...)                    \
        __asm__ volatile(                                           \
            "mov %%gs:0, %%eax\n"                                   \
            "mov %P[tls](%%eax), %%eax\n"                           \
            "test %%eax, %%eax\n"                                   \
            "je 1f\n"                                               \
            "jmp *%P[api](%%eax)\n"                                 \
            "1:\n"                                                  \
            :                                                       \
            : [tls] "i" (TLS_SLOT_OPENGL_API*sizeof(void*)),        \
              [api] "i" (__builtin_offsetof(gl_hooks_t, gl._api))   \
            : "cc", "%eax"                                          \
        );

    #define CALL_GL_API_INTERNAL_SET_RETURN_VALUE \
        __asm__ volatile(                         \
            "xor %%eax, %%eax\n"                  \
            :                                     \
            :                                     \
            : "%eax"                              \
        );

    #define CALL_GL_API_INTERNAL_DO_RETURN \
        __asm__ volatile(                  \
            "ret\n"                        \
            :                              \
            :                              \
            :                              \
        );

#elif defined(__x86_64__)

    #define API_ENTRY(_api) __attribute__((naked,noinline)) _api

    #define CALL_GL_API_INTERNAL_CALL(_api, ...)                    \
        __asm__ volatile(                                           \
            "mov %%fs:0, %%rax\n"                                   \
            "mov %P[tls](%%rax), %%rax\n"                           \
            "test %%rax, %%rax\n"                                   \
            "je 1f\n"                                               \
            "jmp *%P[api](%%rax)\n"                                 \
            "1:\n"                                                  \
            :                                                       \
            : [tls] "i" (TLS_SLOT_OPENGL_API*sizeof(void*)),        \
              [api] "i" (__builtin_offsetof(gl_hooks_t, gl._api))   \
            : "cc", "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9",   \
              "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", \
              "%xmm6", "%xmm7"                                      \
        );

    #define CALL_GL_API_INTERNAL_SET_RETURN_VALUE \
        __asm__ volatile(                         \
            "xor %%eax, %%eax\n"                  \
            :                                     \
            :                                     \
            : "%eax"                              \
        );

    #define CALL_GL_API_INTERNAL_DO_RETURN \
        __asm__ volatile(                  \
            "retq\n"                       \
            :                              \
            :                              \
            :                              \
        );

#elif defined(__mips64)

    #define API_ENTRY(_api) __attribute__((naked,noinline)) _api

    // t0:  $12
    // fn:  $25
    // tls: $3
    // v0:  $2
    #define CALL_GL_API_INTERNAL_CALL(_api, ...)                  \
        asm volatile(                                             \
            ".set  push\n\t"                                      \
            ".set  noreorder\n\t"                                 \
            "rdhwr $3, $29\n\t"                                   \
            "ld    $12, %[OPENGL_API]($3)\n\t"                    \
            "beqz  $12, 1f\n\t"                                   \
            " move $25, $ra\n\t"                                  \
            "ld    $12, %[API]($12)\n\t"                          \
            "beqz  $12, 1f\n\t"                                   \
            " nop\n\t"                                            \
            "move  $25, $12\n\t"                                  \
            "1:\n\t"                                              \
            "jalr  $0, $25\n\t"                                   \
            " move $2, $0\n\t"                                    \
            ".set  pop\n\t"                                       \
            :                                                     \
            : [OPENGL_API] "I"(TLS_SLOT_OPENGL_API*sizeof(void*)),\
              [API] "I"(__builtin_offsetof(gl_hooks_t, gl._api))  \
            : "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9",     \
              "$10", "$11", "$12", "$25"                          \
        );

    #define CALL_GL_API_INTERNAL_SET_RETURN_VALUE
    #define CALL_GL_API_INTERNAL_DO_RETURN

#elif defined(__mips__)

    #define API_ENTRY(_api) __attribute__((naked,noinline)) _api

    // t0:  $8
    // fn:  $25
    // tls: $3
    // v0:  $2
    #define CALL_GL_API_INTERNAL_CALL(_api, ...)                 \
        asm volatile(                                            \
            ".set  push\n\t"                                     \
            ".set  noreorder\n\t"                                \
            ".set  mips32r2\n\t"                                 \
            "rdhwr $3, $29\n\t"                                  \
            "lw    $3, %[OPENGL_API]($3)\n\t"                    \
            "beqz  $3, 1f\n\t"                                   \
            " move $25,$ra\n\t"                                  \
            "lw    $3, %[API]($3)\n\t"                           \
            "beqz  $3, 1f\n\t"                                   \
            " nop\n\t"                                           \
            "move  $25, $3\n\t"                                  \
            "1:\n\t"                                             \
            "jalr  $0, $25\n\t"                                  \
            " move $2, $0\n\t"                                   \
            ".set  pop\n\t"                                      \
            :                                                    \
            : [OPENGL_API] "I"(TLS_SLOT_OPENGL_API*4),           \
              [API] "I"(__builtin_offsetof(gl_hooks_t, gl._api)) \
            : "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$25"    \
        );

    #define CALL_GL_API_INTERNAL_SET_RETURN_VALUE
    #define CALL_GL_API_INTERNAL_DO_RETURN

#endif

#define CALL_GL_API(_api, ...) \
    CALL_GL_API_INTERNAL_CALL(_api, __VA_ARGS__) \
    CALL_GL_API_INTERNAL_DO_RETURN

#define CALL_GL_API_RETURN(_api, ...) \
    CALL_GL_API_INTERNAL_CALL(_api, __VA_ARGS__) \
    CALL_GL_API_INTERNAL_SET_RETURN_VALUE \
    CALL_GL_API_INTERNAL_DO_RETURN

extern "C" {
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "gl_api.in"
#include "glext_api.in"
#pragma GCC diagnostic warning "-Wunused-parameter"
}

#undef API_ENTRY
#undef CALL_GL_API
#undef CALL_GL_API_INTERNAL_CALL
#undef CALL_GL_API_INTERNAL_SET_RETURN_VALUE
#undef CALL_GL_API_INTERNAL_DO_RETURN
#undef CALL_GL_API_RETURN

/*
 * glGetString() is special because we expose some extensions in the wrapper
 */

extern "C" const GLubyte * __glGetString(GLenum name);

const GLubyte * glGetString(GLenum name) {
    const GLubyte * ret = egl_get_string_for_current_context(name);
    if (ret == NULL) {
        gl_hooks_t::gl_t const * const _c = &getGlThreadSpecific()->gl;
        ret = _c->glGetString(name);
    }
    return ret;
}
