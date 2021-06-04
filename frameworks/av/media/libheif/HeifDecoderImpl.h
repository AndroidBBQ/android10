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

#ifndef _HEIF_DECODER_IMPL_
#define _HEIF_DECODER_IMPL_

#include "include/HeifDecoderAPI.h"
#include <system/graphics.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>

namespace android {

class IDataSource;
class IMemory;
class MediaMetadataRetriever;

/*
 * An implementation of HeifDecoder based on Android's MediaMetadataRetriever.
 */
class HeifDecoderImpl : public HeifDecoder {
public:

    HeifDecoderImpl();
    ~HeifDecoderImpl() override;

    bool init(HeifStream* stream, HeifFrameInfo* frameInfo) override;

    bool getEncodedColor(HeifEncodedColor* outColor) const override;

    bool setOutputColor(HeifColorFormat heifColor) override;

    bool decode(HeifFrameInfo* frameInfo) override;

    bool getScanline(uint8_t* dst) override;

    size_t skipScanlines(size_t count) override;

private:
    struct DecodeThread;

    sp<IDataSource> mDataSource;
    sp<MediaMetadataRetriever> mRetriever;
    sp<IMemory> mFrameMemory;
    android_pixel_format_t mOutputColor;
    size_t mCurScanline;
    uint32_t mWidth;
    uint32_t mHeight;
    bool mFrameDecoded;
    bool mHasImage;
    bool mHasVideo;

    // Slice decoding only
    Mutex mLock;
    Condition mScanlineReady;
    sp<DecodeThread> mThread;
    size_t mAvailableLines;
    size_t mNumSlices;
    uint32_t mSliceHeight;
    bool mAsyncDecodeDone;

    bool decodeAsync();
    bool getScanlineInner(uint8_t* dst);
};

} // namespace android

#endif // _HEIF_DECODER_IMPL_
