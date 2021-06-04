/*
 * Copyright (C) 2017 The Android Open Source Project
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

//#define LOG_NDEBUG 0
#define LOG_TAG "HeifDecoderImpl"

#include "HeifDecoderImpl.h"

#include <stdio.h>

#include <binder/IMemory.h>
#include <binder/MemoryDealer.h>
#include <drm/drm_framework_common.h>
#include <media/IDataSource.h>
#include <media/mediametadataretriever.h>
#include <media/MediaSource.h>
#include <media/stagefright/foundation/ADebug.h>
#include <private/media/VideoFrame.h>
#include <utils/Log.h>
#include <utils/RefBase.h>

HeifDecoder* createHeifDecoder() {
    return new android::HeifDecoderImpl();
}

namespace android {

/*
 * HeifDataSource
 *
 * Proxies data requests over IDataSource interface from MediaMetadataRetriever
 * to the HeifStream interface we received from the heif decoder client.
 */
class HeifDataSource : public BnDataSource {
public:
    /*
     * Constructs HeifDataSource; will take ownership of |stream|.
     */
    HeifDataSource(HeifStream* stream)
        : mStream(stream), mEOS(false),
          mCachedOffset(0), mCachedSize(0), mCacheBufferSize(0) {}

    ~HeifDataSource() override {}

    /*
     * Initializes internal resources.
     */
    bool init();

    sp<IMemory> getIMemory() override { return mMemory; }
    ssize_t readAt(off64_t offset, size_t size) override;
    status_t getSize(off64_t* size) override ;
    void close() {}
    uint32_t getFlags() override { return 0; }
    String8 toString() override { return String8("HeifDataSource"); }
    sp<DecryptHandle> DrmInitialization(const char*) override {
        return nullptr;
    }

private:
    enum {
        /*
         * Buffer size for passing the read data to mediaserver. Set to 64K
         * (which is what MediaDataSource Java API's jni implementation uses).
         */
        kBufferSize = 64 * 1024,
        /*
         * Initial and max cache buffer size.
         */
        kInitialCacheBufferSize = 4 * 1024 * 1024,
        kMaxCacheBufferSize = 64 * 1024 * 1024,
    };
    sp<IMemory> mMemory;
    std::unique_ptr<HeifStream> mStream;
    bool mEOS;
    std::unique_ptr<uint8_t[]> mCache;
    off64_t mCachedOffset;
    size_t mCachedSize;
    size_t mCacheBufferSize;
};

bool HeifDataSource::init() {
    sp<MemoryDealer> memoryDealer =
            new MemoryDealer(kBufferSize, "HeifDataSource");
    mMemory = memoryDealer->allocate(kBufferSize);
    if (mMemory == nullptr) {
        ALOGE("Failed to allocate shared memory!");
        return false;
    }
    mCache.reset(new uint8_t[kInitialCacheBufferSize]);
    if (mCache.get() == nullptr) {
        ALOGE("mFailed to allocate cache!");
        return false;
    }
    mCacheBufferSize = kInitialCacheBufferSize;
    return true;
}

ssize_t HeifDataSource::readAt(off64_t offset, size_t size) {
    ALOGV("readAt: offset=%lld, size=%zu", (long long)offset, size);

    if (offset < mCachedOffset) {
        // try seek, then rewind/skip, fail if none worked
        if (mStream->seek(offset)) {
            ALOGV("readAt: seek to offset=%lld", (long long)offset);
            mCachedOffset = offset;
            mCachedSize = 0;
            mEOS = false;
        } else if (mStream->rewind()) {
            ALOGV("readAt: rewind to offset=0");
            mCachedOffset = 0;
            mCachedSize = 0;
            mEOS = false;
        } else {
            ALOGE("readAt: couldn't seek or rewind!");
            mEOS = true;
        }
    }

    if (mEOS && (offset < mCachedOffset ||
                 offset >= (off64_t)(mCachedOffset + mCachedSize))) {
        ALOGV("readAt: EOS");
        return ERROR_END_OF_STREAM;
    }

    // at this point, offset must be >= mCachedOffset, other cases should
    // have been caught above.
    CHECK(offset >= mCachedOffset);

    off64_t resultOffset;
    if (__builtin_add_overflow(offset, size, &resultOffset)) {
        return ERROR_IO;
    }

    if (size == 0) {
        return 0;
    }

    // Can only read max of kBufferSize
    if (size > kBufferSize) {
        size = kBufferSize;
    }

    // copy from cache if the request falls entirely in cache
    if (offset + size <= mCachedOffset + mCachedSize) {
        memcpy(mMemory->pointer(), mCache.get() + offset - mCachedOffset, size);
        return size;
    }

    // need to fetch more, check if we need to expand the cache buffer.
    if ((off64_t)(offset + size) > mCachedOffset + kMaxCacheBufferSize) {
        // it's reaching max cache buffer size, need to roll window, and possibly
        // expand the cache buffer.
        size_t newCacheBufferSize = mCacheBufferSize;
        std::unique_ptr<uint8_t[]> newCache;
        uint8_t* dst = mCache.get();
        if (newCacheBufferSize < kMaxCacheBufferSize) {
            newCacheBufferSize = kMaxCacheBufferSize;
            newCache.reset(new uint8_t[newCacheBufferSize]);
            dst = newCache.get();
        }

        // when rolling the cache window, try to keep about half the old bytes
        // in case that the client goes back.
        off64_t newCachedOffset = offset - (off64_t)(newCacheBufferSize / 2);
        if (newCachedOffset < mCachedOffset) {
            newCachedOffset = mCachedOffset;
        }

        int64_t newCachedSize = (int64_t)(mCachedOffset + mCachedSize) - newCachedOffset;
        if (newCachedSize > 0) {
            // in this case, the new cache region partially overlop the old cache,
            // move the portion of the cache we want to save to the beginning of
            // the cache buffer.
            memcpy(dst, mCache.get() + newCachedOffset - mCachedOffset, newCachedSize);
        } else if (newCachedSize < 0){
            // in this case, the new cache region is entirely out of the old cache,
            // in order to guarantee sequential read, we need to skip a number of
            // bytes before reading.
            size_t bytesToSkip = -newCachedSize;
            size_t bytesSkipped = mStream->read(nullptr, bytesToSkip);
            if (bytesSkipped != bytesToSkip) {
                // bytesSkipped is invalid, there is not enough bytes to reach
                // the requested offset.
                ALOGE("readAt: skip failed, EOS");

                mEOS = true;
                mCachedOffset = newCachedOffset;
                mCachedSize = 0;
                return ERROR_END_OF_STREAM;
            }
            // set cache size to 0, since we're not keeping any old cache
            newCachedSize = 0;
        }

        if (newCache.get() != nullptr) {
            mCache.reset(newCache.release());
            mCacheBufferSize = newCacheBufferSize;
        }
        mCachedOffset = newCachedOffset;
        mCachedSize = newCachedSize;

        ALOGV("readAt: rolling cache window to (%lld, %zu), cache buffer size %zu",
                (long long)mCachedOffset, mCachedSize, mCacheBufferSize);
    } else {
        // expand cache buffer, but no need to roll the window
        size_t newCacheBufferSize = mCacheBufferSize;
        while (offset + size > mCachedOffset + newCacheBufferSize) {
            newCacheBufferSize *= 2;
        }
        CHECK(newCacheBufferSize <= kMaxCacheBufferSize);
        if (mCacheBufferSize < newCacheBufferSize) {
            uint8_t* newCache = new uint8_t[newCacheBufferSize];
            memcpy(newCache, mCache.get(), mCachedSize);
            mCache.reset(newCache);
            mCacheBufferSize = newCacheBufferSize;

            ALOGV("readAt: current cache window (%lld, %zu), new cache buffer size %zu",
                    (long long) mCachedOffset, mCachedSize, mCacheBufferSize);
        }
    }
    size_t bytesToRead = offset + size - mCachedOffset - mCachedSize;
    size_t bytesRead = mStream->read(mCache.get() + mCachedSize, bytesToRead);
    if (bytesRead > bytesToRead || bytesRead == 0) {
        // bytesRead is invalid
        mEOS = true;
        bytesRead = 0;
    } else if (bytesRead < bytesToRead) {
        // read some bytes but not all, set EOS
        mEOS = true;
    }
    mCachedSize += bytesRead;
    ALOGV("readAt: current cache window (%lld, %zu)",
            (long long) mCachedOffset, mCachedSize);

    // here bytesAvailable could be negative if offset jumped past EOS.
    int64_t bytesAvailable = mCachedOffset + mCachedSize - offset;
    if (bytesAvailable <= 0) {
        return ERROR_END_OF_STREAM;
    }
    if (bytesAvailable < (int64_t)size) {
        size = bytesAvailable;
    }
    memcpy(mMemory->pointer(), mCache.get() + offset - mCachedOffset, size);
    return size;
}

status_t HeifDataSource::getSize(off64_t* size) {
    if (!mStream->hasLength()) {
        *size = -1;
        ALOGE("getSize: not supported!");
        return ERROR_UNSUPPORTED;
    }
    *size = mStream->getLength();
    ALOGV("getSize: size=%lld", (long long)*size);
    return OK;
}

/////////////////////////////////////////////////////////////////////////

struct HeifDecoderImpl::DecodeThread : public Thread {
    explicit DecodeThread(HeifDecoderImpl *decoder) : mDecoder(decoder) {}

private:
    HeifDecoderImpl* mDecoder;

    bool threadLoop();

    DISALLOW_EVIL_CONSTRUCTORS(DecodeThread);
};

bool HeifDecoderImpl::DecodeThread::threadLoop() {
    return mDecoder->decodeAsync();
}

/////////////////////////////////////////////////////////////////////////

HeifDecoderImpl::HeifDecoderImpl() :
    // output color format should always be set via setOutputColor(), in case
    // it's not, default to HAL_PIXEL_FORMAT_RGB_565.
    mOutputColor(HAL_PIXEL_FORMAT_RGB_565),
    mCurScanline(0),
    mWidth(0),
    mHeight(0),
    mFrameDecoded(false),
    mHasImage(false),
    mHasVideo(false),
    mAvailableLines(0),
    mNumSlices(1),
    mSliceHeight(0),
    mAsyncDecodeDone(false) {
}

HeifDecoderImpl::~HeifDecoderImpl() {
    if (mThread != nullptr) {
        mThread->join();
    }
}

bool HeifDecoderImpl::init(HeifStream* stream, HeifFrameInfo* frameInfo) {
    mFrameDecoded = false;
    mFrameMemory.clear();

    sp<HeifDataSource> dataSource = new HeifDataSource(stream);
    if (!dataSource->init()) {
        return false;
    }
    mDataSource = dataSource;

    mRetriever = new MediaMetadataRetriever();
    status_t err = mRetriever->setDataSource(mDataSource, "image/heif");
    if (err != OK) {
        ALOGE("failed to set data source!");

        mRetriever.clear();
        mDataSource.clear();
        return false;
    }
    ALOGV("successfully set data source.");

    const char* hasImage = mRetriever->extractMetadata(METADATA_KEY_HAS_IMAGE);
    const char* hasVideo = mRetriever->extractMetadata(METADATA_KEY_HAS_VIDEO);

    mHasImage = hasImage && !strcasecmp(hasImage, "yes");
    mHasVideo = hasVideo && !strcasecmp(hasVideo, "yes");
    sp<IMemory> sharedMem;
    if (mHasImage) {
        // image index < 0 to retrieve primary image
        sharedMem = mRetriever->getImageAtIndex(
                -1, mOutputColor, true /*metaOnly*/);
    } else if (mHasVideo) {
        sharedMem = mRetriever->getFrameAtTime(0,
                MediaSource::ReadOptions::SEEK_PREVIOUS_SYNC,
                mOutputColor, true /*metaOnly*/);
    }

    if (sharedMem == nullptr || sharedMem->pointer() == nullptr) {
        ALOGE("getFrameAtTime: videoFrame is a nullptr");
        return false;
    }

    VideoFrame* videoFrame = static_cast<VideoFrame*>(sharedMem->pointer());

    ALOGV("Meta dimension %dx%d, display %dx%d, angle %d, iccSize %d",
            videoFrame->mWidth,
            videoFrame->mHeight,
            videoFrame->mDisplayWidth,
            videoFrame->mDisplayHeight,
            videoFrame->mRotationAngle,
            videoFrame->mIccSize);

    if (frameInfo != nullptr) {
        frameInfo->set(
                videoFrame->mWidth,
                videoFrame->mHeight,
                videoFrame->mRotationAngle,
                videoFrame->mBytesPerPixel,
                videoFrame->mIccSize,
                videoFrame->getFlattenedIccData());
    }
    mWidth = videoFrame->mWidth;
    mHeight = videoFrame->mHeight;
    if (mHasImage && videoFrame->mTileHeight >= 512 && mWidth >= 3000 && mHeight >= 2000 ) {
        // Try decoding in slices only if the image has tiles and is big enough.
        mSliceHeight = videoFrame->mTileHeight;
        mNumSlices = (videoFrame->mHeight + mSliceHeight - 1) / mSliceHeight;
        ALOGV("mSliceHeight %u, mNumSlices %zu", mSliceHeight, mNumSlices);
    }
    return true;
}

bool HeifDecoderImpl::getEncodedColor(HeifEncodedColor* /*outColor*/) const {
    ALOGW("getEncodedColor: not implemented!");
    return false;
}

bool HeifDecoderImpl::setOutputColor(HeifColorFormat heifColor) {
    switch(heifColor) {
        case kHeifColorFormat_RGB565:
        {
            mOutputColor = HAL_PIXEL_FORMAT_RGB_565;
            return true;
        }
        case kHeifColorFormat_RGBA_8888:
        {
            mOutputColor = HAL_PIXEL_FORMAT_RGBA_8888;
            return true;
        }
        case kHeifColorFormat_BGRA_8888:
        {
            mOutputColor = HAL_PIXEL_FORMAT_BGRA_8888;
            return true;
        }
        default:
            break;
    }
    ALOGE("Unsupported output color format %d", heifColor);
    return false;
}

bool HeifDecoderImpl::decodeAsync() {
    for (size_t i = 1; i < mNumSlices; i++) {
        ALOGV("decodeAsync(): decoding slice %zu", i);
        size_t top = i * mSliceHeight;
        size_t bottom = (i + 1) * mSliceHeight;
        if (bottom > mHeight) {
            bottom = mHeight;
        }
        sp<IMemory> frameMemory = mRetriever->getImageRectAtIndex(
                -1, mOutputColor, 0, top, mWidth, bottom);
        {
            Mutex::Autolock autolock(mLock);

            if (frameMemory == nullptr || frameMemory->pointer() == nullptr) {
                mAsyncDecodeDone = true;
                mScanlineReady.signal();
                break;
            }
            mFrameMemory = frameMemory;
            mAvailableLines = bottom;
            ALOGV("decodeAsync(): available lines %zu", mAvailableLines);
            mScanlineReady.signal();
        }
    }
    // Aggressive clear to avoid holding on to resources
    mRetriever.clear();
    mDataSource.clear();
    return false;
}

bool HeifDecoderImpl::decode(HeifFrameInfo* frameInfo) {
    // reset scanline pointer
    mCurScanline = 0;

    if (mFrameDecoded) {
        return true;
    }

    // See if we want to decode in slices to allow client to start
    // scanline processing in parallel with decode. If this fails
    // we fallback to decoding the full frame.
    if (mHasImage && mNumSlices > 1) {
        // get first slice and metadata
        sp<IMemory> frameMemory = mRetriever->getImageRectAtIndex(
                -1, mOutputColor, 0, 0, mWidth, mSliceHeight);

        if (frameMemory == nullptr || frameMemory->pointer() == nullptr) {
            ALOGE("decode: metadata is a nullptr");
            return false;
        }

        VideoFrame* videoFrame = static_cast<VideoFrame*>(frameMemory->pointer());

        if (frameInfo != nullptr) {
            frameInfo->set(
                    videoFrame->mWidth,
                    videoFrame->mHeight,
                    videoFrame->mRotationAngle,
                    videoFrame->mBytesPerPixel,
                    videoFrame->mIccSize,
                    videoFrame->getFlattenedIccData());
        }

        mFrameMemory = frameMemory;
        mAvailableLines = mSliceHeight;
        mThread = new DecodeThread(this);
        if (mThread->run("HeifDecode", ANDROID_PRIORITY_FOREGROUND) == OK) {
            mFrameDecoded = true;
            return true;
        }

        // Fallback to decode without slicing
        mThread.clear();
        mNumSlices = 1;
        mSliceHeight = 0;
        mAvailableLines = 0;
        mFrameMemory.clear();
    }

    if (mHasImage) {
        // image index < 0 to retrieve primary image
        mFrameMemory = mRetriever->getImageAtIndex(-1, mOutputColor);
    } else if (mHasVideo) {
        mFrameMemory = mRetriever->getFrameAtTime(0,
                MediaSource::ReadOptions::SEEK_PREVIOUS_SYNC, mOutputColor);
    }

    if (mFrameMemory == nullptr || mFrameMemory->pointer() == nullptr) {
        ALOGE("decode: videoFrame is a nullptr");
        return false;
    }

    VideoFrame* videoFrame = static_cast<VideoFrame*>(mFrameMemory->pointer());
    if (videoFrame->mSize == 0 ||
            mFrameMemory->size() < videoFrame->getFlattenedSize()) {
        ALOGE("decode: videoFrame size is invalid");
        return false;
    }

    ALOGV("Decoded dimension %dx%d, display %dx%d, angle %d, rowbytes %d, size %d",
            videoFrame->mWidth,
            videoFrame->mHeight,
            videoFrame->mDisplayWidth,
            videoFrame->mDisplayHeight,
            videoFrame->mRotationAngle,
            videoFrame->mRowBytes,
            videoFrame->mSize);

    if (frameInfo != nullptr) {
        frameInfo->set(
                videoFrame->mWidth,
                videoFrame->mHeight,
                videoFrame->mRotationAngle,
                videoFrame->mBytesPerPixel,
                videoFrame->mIccSize,
                videoFrame->getFlattenedIccData());
    }
    mFrameDecoded = true;

    // Aggressively clear to avoid holding on to resources
    mRetriever.clear();
    mDataSource.clear();
    return true;
}

bool HeifDecoderImpl::getScanlineInner(uint8_t* dst) {
    if (mFrameMemory == nullptr || mFrameMemory->pointer() == nullptr) {
        return false;
    }
    VideoFrame* videoFrame = static_cast<VideoFrame*>(mFrameMemory->pointer());
    uint8_t* src = videoFrame->getFlattenedData() + videoFrame->mRowBytes * mCurScanline++;
    memcpy(dst, src, videoFrame->mBytesPerPixel * videoFrame->mWidth);
    return true;
}

bool HeifDecoderImpl::getScanline(uint8_t* dst) {
    if (mCurScanline >= mHeight) {
        ALOGE("no more scanline available");
        return false;
    }

    if (mNumSlices > 1) {
        Mutex::Autolock autolock(mLock);

        while (!mAsyncDecodeDone && mCurScanline >= mAvailableLines) {
            mScanlineReady.wait(mLock);
        }
        return (mCurScanline < mAvailableLines) ? getScanlineInner(dst) : false;
    }

    return getScanlineInner(dst);
}

size_t HeifDecoderImpl::skipScanlines(size_t count) {
    uint32_t oldScanline = mCurScanline;
    mCurScanline += count;
    if (mCurScanline > mHeight) {
        mCurScanline = mHeight;
    }
    return (mCurScanline > oldScanline) ? (mCurScanline - oldScanline) : 0;
}

} // namespace android
