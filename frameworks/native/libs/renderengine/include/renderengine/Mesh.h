/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SF_RENDER_ENGINE_MESH_H
#define SF_RENDER_ENGINE_MESH_H

#include <vector>

#include <stdint.h>

namespace android {
namespace renderengine {

class Mesh {
public:
    enum Primitive {
        TRIANGLES = 0x0004,      // GL_TRIANGLES
        TRIANGLE_STRIP = 0x0005, // GL_TRIANGLE_STRIP
        TRIANGLE_FAN = 0x0006    // GL_TRIANGLE_FAN
    };

    Mesh(Primitive primitive, size_t vertexCount, size_t vertexSize, size_t texCoordsSize = 0);
    ~Mesh() = default;

    /*
     * VertexArray handles the stride automatically.
     */
    template <typename TYPE>
    class VertexArray {
        friend class Mesh;
        float* mData;
        size_t mStride;
        VertexArray(float* data, size_t stride) : mData(data), mStride(stride) {}

    public:
        TYPE& operator[](size_t index) { return *reinterpret_cast<TYPE*>(&mData[index * mStride]); }
        TYPE const& operator[](size_t index) const {
            return *reinterpret_cast<TYPE const*>(&mData[index * mStride]);
        }
    };

    template <typename TYPE>
    VertexArray<TYPE> getPositionArray() {
        return VertexArray<TYPE>(getPositions(), mStride);
    }

    template <typename TYPE>
    VertexArray<TYPE> getTexCoordArray() {
        return VertexArray<TYPE>(getTexCoords(), mStride);
    }

    template <typename TYPE>
    VertexArray<TYPE> getCropCoordArray() {
        return VertexArray<TYPE>(getCropCoords(), mStride);
    }

    Primitive getPrimitive() const;

    // returns a pointer to the vertices positions
    float const* getPositions() const;

    // returns a pointer to the vertices texture coordinates
    float const* getTexCoords() const;

    // returns a pointer to the vertices crop coordinates
    float const* getCropCoords() const;

    // number of vertices in this mesh
    size_t getVertexCount() const;

    // dimension of vertices
    size_t getVertexSize() const;

    // dimension of texture coordinates
    size_t getTexCoordsSize() const;

    // return stride in bytes
    size_t getByteStride() const;

    // return stride in floats
    size_t getStride() const;

private:
    Mesh(const Mesh&);
    Mesh& operator=(const Mesh&);
    Mesh const& operator=(const Mesh&) const;

    float* getPositions();
    float* getTexCoords();
    float* getCropCoords();

    std::vector<float> mVertices;
    size_t mVertexCount;
    size_t mVertexSize;
    size_t mTexCoordsSize;
    size_t mStride;
    Primitive mPrimitive;
};

} // namespace renderengine
} // namespace android
#endif /* SF_RENDER_ENGINE_MESH_H */
