/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef STAGEFRIGHT_METADATA_RETRIEVER_H_

#define STAGEFRIGHT_METADATA_RETRIEVER_H_

#include <media/IMediaExtractor.h>
#include <media/MediaMetadataRetrieverInterface.h>

#include <utils/KeyedVector.h>

namespace android {

class DataSource;
struct ImageDecoder;
struct FrameRect;

struct StagefrightMetadataRetriever : public MediaMetadataRetrieverBase {
    StagefrightMetadataRetriever();
    virtual ~StagefrightMetadataRetriever();

    virtual status_t setDataSource(
            const sp<IMediaHTTPService> &httpService,
            const char *url,
            const KeyedVector<String8, String8> *headers);

    virtual status_t setDataSource(int fd, int64_t offset, int64_t length);
    virtual status_t setDataSource(const sp<DataSource>& source, const char *mime);

    virtual sp<IMemory> getFrameAtTime(
            int64_t timeUs, int option, int colorFormat, bool metaOnly);
    virtual sp<IMemory> getImageAtIndex(
            int index, int colorFormat, bool metaOnly, bool thumbnail);
    virtual sp<IMemory> getImageRectAtIndex(
            int index, int colorFormat, int left, int top, int right, int bottom);
    virtual status_t getFrameAtIndex(
            std::vector<sp<IMemory> >* frames,
            int frameIndex, int numFrames, int colorFormat, bool metaOnly);

    virtual MediaAlbumArt *extractAlbumArt();
    virtual const char *extractMetadata(int keyCode);

private:
    sp<DataSource> mSource;
    sp<IMediaExtractor> mExtractor;

    bool mParsedMetaData;
    KeyedVector<int, String8> mMetaData;
    MediaAlbumArt *mAlbumArt;

    sp<ImageDecoder> mImageDecoder;
    int mLastImageIndex;
    void parseMetaData();
    void parseColorAspects(const sp<MetaData>& meta);
    // Delete album art and clear metadata.
    void clearMetadata();

    status_t getFrameInternal(
            int64_t timeUs, int numFrames, int option, int colorFormat, bool metaOnly,
            sp<IMemory>* outFrame, std::vector<sp<IMemory> >* outFrames);
    virtual sp<IMemory> getImageInternal(
            int index, int colorFormat, bool metaOnly, bool thumbnail, FrameRect* rect);

    StagefrightMetadataRetriever(const StagefrightMetadataRetriever &);

    StagefrightMetadataRetriever &operator=(
            const StagefrightMetadataRetriever &);
};

}  // namespace android

#endif  // STAGEFRIGHT_METADATA_RETRIEVER_H_
