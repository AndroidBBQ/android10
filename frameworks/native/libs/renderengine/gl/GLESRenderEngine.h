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

#ifndef SF_GLESRENDERENGINE_H_
#define SF_GLESRENDERENGINE_H_

#include <stdint.h>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <android-base/thread_annotations.h>
#include <renderengine/RenderEngine.h>
#include <renderengine/private/Description.h>
#include <sys/types.h>
#include "ImageManager.h"

#define EGL_NO_CONFIG ((EGLConfig)0)

namespace android {

namespace renderengine {

class Mesh;
class Texture;

namespace gl {

class GLImage;

class GLESRenderEngine : public impl::RenderEngine {
public:
    static std::unique_ptr<GLESRenderEngine> create(int hwcFormat, uint32_t featureFlags,
                                                    uint32_t imageCacheSize);
    static EGLConfig chooseEglConfig(EGLDisplay display, int format, bool logConfig);

    GLESRenderEngine(uint32_t featureFlags, // See RenderEngine::FeatureFlag
                     EGLDisplay display, EGLConfig config, EGLContext ctxt, EGLSurface dummy,
                     EGLContext protectedContext, EGLSurface protectedDummy,
                     uint32_t imageCacheSize);
    ~GLESRenderEngine() override EXCLUDES(mRenderingMutex);

    std::unique_ptr<Framebuffer> createFramebuffer() override;
    std::unique_ptr<Image> createImage() override;

    void primeCache() const override;
    bool isCurrent() const override;
    base::unique_fd flush() override;
    bool finish() override;
    bool waitFence(base::unique_fd fenceFd) override;
    void clearWithColor(float red, float green, float blue, float alpha) override;
    void fillRegionWithColor(const Region& region, float red, float green, float blue,
                             float alpha) override;
    void genTextures(size_t count, uint32_t* names) override;
    void deleteTextures(size_t count, uint32_t const* names) override;
    void bindExternalTextureImage(uint32_t texName, const Image& image) override;
    status_t bindExternalTextureBuffer(uint32_t texName, const sp<GraphicBuffer>& buffer,
                                       const sp<Fence>& fence) EXCLUDES(mRenderingMutex);
    void cacheExternalTextureBuffer(const sp<GraphicBuffer>& buffer) EXCLUDES(mRenderingMutex);
    void unbindExternalTextureBuffer(uint64_t bufferId) EXCLUDES(mRenderingMutex);
    status_t bindFrameBuffer(Framebuffer* framebuffer) override;
    void unbindFrameBuffer(Framebuffer* framebuffer) override;
    void checkErrors() const override;

    bool isProtected() const override { return mInProtectedContext; }
    bool supportsProtectedContent() const override;
    bool useProtectedContext(bool useProtectedContext) override;
    status_t drawLayers(const DisplaySettings& display, const std::vector<LayerSettings>& layers,
                        ANativeWindowBuffer* buffer, const bool useFramebufferCache,
                        base::unique_fd&& bufferFence, base::unique_fd* drawFence) override;

    // internal to RenderEngine
    EGLDisplay getEGLDisplay() const { return mEGLDisplay; }
    EGLConfig getEGLConfig() const { return mEGLConfig; }
    // Creates an output image for rendering to
    EGLImageKHR createFramebufferImageIfNeeded(ANativeWindowBuffer* nativeBuffer, bool isProtected,
                                               bool useFramebufferCache)
            EXCLUDES(mFramebufferImageCacheMutex);

    // Test-only methods
    // Returns true iff mImageCache contains an image keyed by bufferId
    bool isImageCachedForTesting(uint64_t bufferId) EXCLUDES(mRenderingMutex);
    // Returns true iff mFramebufferImageCache contains an image keyed by bufferId
    bool isFramebufferImageCachedForTesting(uint64_t bufferId)
            EXCLUDES(mFramebufferImageCacheMutex);
    // These are wrappers around public methods above, but exposing Barrier
    // objects so that tests can block.
    std::shared_ptr<ImageManager::Barrier> cacheExternalTextureBufferForTesting(
            const sp<GraphicBuffer>& buffer);
    std::shared_ptr<ImageManager::Barrier> unbindExternalTextureBufferForTesting(uint64_t bufferId);

protected:
    Framebuffer* getFramebufferForDrawing() override;
    void dump(std::string& result) override EXCLUDES(mRenderingMutex)
            EXCLUDES(mFramebufferImageCacheMutex);
    void setViewportAndProjection(size_t vpw, size_t vph, Rect sourceCrop,
                                  ui::Transform::orientation_flags rotation) override;
    void setupLayerBlending(bool premultipliedAlpha, bool opaque, bool disableTexture,
                            const half4& color, float cornerRadius) override;
    void setupLayerTexturing(const Texture& texture) override;
    void setupLayerBlackedOut() override;
    void setupFillWithColor(float r, float g, float b, float a) override;
    void setColorTransform(const mat4& colorTransform) override;
    void disableTexturing() override;
    void disableBlending() override;
    void setupCornerRadiusCropSize(float width, float height) override;

    // HDR and color management related functions and state
    void setSourceY410BT2020(bool enable) override;
    void setSourceDataSpace(ui::Dataspace source) override;
    void setOutputDataSpace(ui::Dataspace dataspace) override;
    void setDisplayMaxLuminance(const float maxLuminance) override;

    // drawing
    void drawMesh(const Mesh& mesh) override;

    size_t getMaxTextureSize() const override;
    size_t getMaxViewportDims() const override;

private:
    enum GlesVersion {
        GLES_VERSION_1_0 = 0x10000,
        GLES_VERSION_1_1 = 0x10001,
        GLES_VERSION_2_0 = 0x20000,
        GLES_VERSION_3_0 = 0x30000,
    };

    static GlesVersion parseGlesVersion(const char* str);
    static EGLContext createEglContext(EGLDisplay display, EGLConfig config,
                                       EGLContext shareContext, bool useContextPriority,
                                       Protection protection);
    static EGLSurface createDummyEglPbufferSurface(EGLDisplay display, EGLConfig config,
                                                   int hwcFormat, Protection protection);
    void setScissor(const Rect& region);
    void disableScissor();
    bool waitSync(EGLSyncKHR sync, EGLint flags);
    status_t cacheExternalTextureBufferInternal(const sp<GraphicBuffer>& buffer)
            EXCLUDES(mRenderingMutex);
    void unbindExternalTextureBufferInternal(uint64_t bufferId) EXCLUDES(mRenderingMutex);

    // A data space is considered HDR data space if it has BT2020 color space
    // with PQ or HLG transfer function.
    bool isHdrDataSpace(const ui::Dataspace dataSpace) const;
    bool needsXYZTransformMatrix() const;
    // Defines the viewport, and sets the projection matrix to the projection
    // defined by the clip.
    void setViewportAndProjection(Rect viewport, Rect clip);
    // Evicts stale images from the buffer cache.
    void evictImages(const std::vector<LayerSettings>& layers);
    // Computes the cropping window for the layer and sets up cropping
    // coordinates for the mesh.
    FloatRect setupLayerCropping(const LayerSettings& layer, Mesh& mesh);

    // We do a special handling for rounded corners when it's possible to turn off blending
    // for the majority of the layer. The rounded corners needs to turn on blending such that
    // we can set the alpha value correctly, however, only the corners need this, and since
    // blending is an expensive operation, we want to turn off blending when it's not necessary.
    void handleRoundedCorners(const DisplaySettings& display, const LayerSettings& layer,
                              const Mesh& mesh);

    EGLDisplay mEGLDisplay;
    EGLConfig mEGLConfig;
    EGLContext mEGLContext;
    EGLSurface mDummySurface;
    EGLContext mProtectedEGLContext;
    EGLSurface mProtectedDummySurface;
    GLuint mProtectedTexName;
    GLint mMaxViewportDims[2];
    GLint mMaxTextureSize;
    GLuint mVpWidth;
    GLuint mVpHeight;
    Description mState;

    mat4 mSrgbToXyz;
    mat4 mDisplayP3ToXyz;
    mat4 mBt2020ToXyz;
    mat4 mXyzToSrgb;
    mat4 mXyzToDisplayP3;
    mat4 mXyzToBt2020;
    mat4 mSrgbToDisplayP3;
    mat4 mSrgbToBt2020;
    mat4 mDisplayP3ToSrgb;
    mat4 mDisplayP3ToBt2020;
    mat4 mBt2020ToSrgb;
    mat4 mBt2020ToDisplayP3;

    bool mInProtectedContext = false;
    // If set to true, then enables tracing flush() and finish() to systrace.
    bool mTraceGpuCompletion = false;
    // Maximum size of mFramebufferImageCache. If more images would be cached, then (approximately)
    // the last recently used buffer should be kicked out.
    uint32_t mFramebufferImageCacheSize = 0;

    // Cache of output images, keyed by corresponding GraphicBuffer ID.
    std::deque<std::pair<uint64_t, EGLImageKHR>> mFramebufferImageCache
            GUARDED_BY(mFramebufferImageCacheMutex);
    // The only reason why we have this mutex is so that we don't segfault when
    // dumping info.
    std::mutex mFramebufferImageCacheMutex;

    // Current dataspace of layer being rendered
    ui::Dataspace mDataSpace = ui::Dataspace::UNKNOWN;

    // Current output dataspace of the render engine
    ui::Dataspace mOutputDataSpace = ui::Dataspace::UNKNOWN;

    // Whether device supports color management, currently color management
    // supports sRGB, DisplayP3 color spaces.
    const bool mUseColorManagement = false;

    // Cache of GL images that we'll store per GraphicBuffer ID
    std::unordered_map<uint64_t, std::unique_ptr<Image>> mImageCache GUARDED_BY(mRenderingMutex);
    // Mutex guarding rendering operations, so that:
    // 1. GL operations aren't interleaved, and
    // 2. Internal state related to rendering that is potentially modified by
    // multiple threads is guaranteed thread-safe.
    std::mutex mRenderingMutex;

    std::unique_ptr<Framebuffer> mDrawingBuffer;

    class FlushTracer {
    public:
        FlushTracer(GLESRenderEngine* engine);
        ~FlushTracer();
        void queueSync(EGLSyncKHR sync) EXCLUDES(mMutex);

        struct QueueEntry {
            EGLSyncKHR mSync = nullptr;
            uint64_t mFrameNum = 0;
        };

    private:
        void loop();
        GLESRenderEngine* const mEngine;
        std::thread mThread;
        std::condition_variable_any mCondition;
        std::mutex mMutex;
        std::queue<QueueEntry> mQueue GUARDED_BY(mMutex);
        uint64_t mFramesQueued GUARDED_BY(mMutex) = 0;
        bool mRunning = true;
    };
    friend class FlushTracer;
    friend class ImageManager;
    std::unique_ptr<FlushTracer> mFlushTracer;
    std::unique_ptr<ImageManager> mImageManager = std::make_unique<ImageManager>(this);
};

} // namespace gl
} // namespace renderengine
} // namespace android

#endif /* SF_GLESRENDERENGINE_H_ */
