/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef STAGEFRIGHT_DATACONVERTER_H_
#define STAGEFRIGHT_DATACONVERTER_H_

#include <utils/Errors.h>
#include <utils/RefBase.h>

#include <media/stagefright/MediaDefs.h>

namespace android {

class MediaCodecBuffer;

// DataConverter base class, defaults to memcpy
struct DataConverter : public RefBase {
    virtual size_t sourceSize(size_t targetSize); // will clamp to SIZE_MAX
    virtual size_t targetSize(size_t sourceSize); // will clamp to SIZE_MAX

    status_t convert(const sp<MediaCodecBuffer> &source, sp<MediaCodecBuffer> &target);
    virtual ~DataConverter();

protected:
    virtual status_t safeConvert(const sp<MediaCodecBuffer> &source, sp<MediaCodecBuffer> &target);
};

// SampleConverterBase uses a ratio to calculate the source and target sizes
// based on source and target sample sizes.
struct SampleConverterBase : public DataConverter {
    virtual size_t sourceSize(size_t targetSize);
    virtual size_t targetSize(size_t sourceSize);

protected:
    virtual status_t safeConvert(const sp<MediaCodecBuffer> &source, sp<MediaCodecBuffer> &target) = 0;

    // sourceSize = sourceSampleSize / targetSampleSize * targetSize
    SampleConverterBase(uint32_t sourceSampleSize, uint32_t targetSampleSize)
        : mSourceSampleSize(sourceSampleSize),
          mTargetSampleSize(targetSampleSize) { }
    size_t mSourceSampleSize;
    size_t mTargetSampleSize;
};

// AudioConverter converts between audio PCM formats
struct AudioConverter : public SampleConverterBase {
    // return nullptr if conversion is not needed or not supported
    static AudioConverter *Create(AudioEncoding source, AudioEncoding target);

protected:
    virtual status_t safeConvert(const sp<MediaCodecBuffer> &source, sp<MediaCodecBuffer> &target);

private:
    AudioConverter(
            AudioEncoding source, size_t sourceSample,
            AudioEncoding target, size_t targetSample)
        : SampleConverterBase(sourceSample, targetSample),
          mFrom(source),
          mTo(target) { }
    AudioEncoding mFrom;
    AudioEncoding mTo;
};

} // namespace android

#endif
