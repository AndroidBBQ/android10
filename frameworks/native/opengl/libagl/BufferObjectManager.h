/*
 **
 ** Copyright 2006, The Android Open Source Project
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

#ifndef ANDROID_OPENGLES_BUFFER_OBJECT_MANAGER_H
#define ANDROID_OPENGLES_BUFFER_OBJECT_MANAGER_H

#include <atomic>
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <utils/Errors.h>

#include <GLES/gl.h>

#include "Tokenizer.h"
#include "TokenManager.h"


namespace android {

// ----------------------------------------------------------------------------

namespace gl {

struct buffer_t {
    GLsizeiptr      size;
    GLenum          usage;
    uint8_t*        data;
    uint32_t        name;
};

};

class EGLBufferObjectManager : public TokenManager
{
public:
    EGLBufferObjectManager();
    ~EGLBufferObjectManager();

    // protocol for sp<>
    inline  void    incStrong(const void* id) const;
    inline  void    decStrong(const void* id) const;
    typedef void    weakref_type;

    gl::buffer_t const* bind(GLuint buffer);
    int                 allocateStore(gl::buffer_t* bo, GLsizeiptr size, GLenum usage);
    void                deleteBuffers(GLsizei n, const GLuint* buffers);

private:
    mutable std::atomic_size_t          mCount;
    mutable Mutex                       mLock;
    KeyedVector<GLuint, gl::buffer_t*>  mBuffers;
};

void EGLBufferObjectManager::incStrong(const void* /*id*/) const {
    mCount.fetch_add(1, std::memory_order_relaxed);
}
void EGLBufferObjectManager::decStrong(const void* /*id*/) const {
    if (mCount.fetch_sub(1, std::memory_order_release) == 0) {
        std::atomic_thread_fence(std::memory_order_acquire);
        delete this;
    }
}

// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_OPENGLES_BUFFER_OBJECT_MANAGER_H

