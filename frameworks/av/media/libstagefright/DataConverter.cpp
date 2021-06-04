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

//#define LOG_NDEBUG 0
#define LOG_TAG "DataConverter"

#include "include/DataConverter.h"

#include <audio_utils/primitives.h>

#include <media/MediaCodecBuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AUtils.h>

namespace android {

status_t DataConverter::convert(const sp<MediaCodecBuffer> &source, sp<MediaCodecBuffer> &target) {
    CHECK(source->base() != target->base());
    size_t size = targetSize(source->size());
    status_t err = OK;
    if (size > target->capacity()) {
        ALOGE("data size (%zu) is greater than buffer capacity (%zu)",
                size,          // this is the data received/to be converted
                target->capacity()); // this is out buffer size
        err = FAILED_TRANSACTION;
    } else {
        err = safeConvert(source, target);
    }
    target->setRange(0, err == OK ? size : 0);
    return err;
}

status_t DataConverter::safeConvert(
        const sp<MediaCodecBuffer> &source, sp<MediaCodecBuffer> &target) {
    memcpy(target->base(), source->data(), source->size());
    return OK;
}

size_t DataConverter::sourceSize(size_t targetSize) {
    return targetSize;
}

size_t DataConverter::targetSize(size_t sourceSize) {
    return sourceSize;
}

DataConverter::~DataConverter() { }


size_t SampleConverterBase::sourceSize(size_t targetSize) {
    size_t numSamples = targetSize / mTargetSampleSize;
    if (numSamples > SIZE_MAX / mSourceSampleSize) {
        ALOGW("limiting source size due to overflow (%zu*%zu/%zu)",
                targetSize, mSourceSampleSize, mTargetSampleSize);
        return SIZE_MAX;
    }
    return numSamples * mSourceSampleSize;
}

size_t SampleConverterBase::targetSize(size_t sourceSize) {
    // we round up on conversion
    size_t numSamples = divUp(sourceSize, (size_t)mSourceSampleSize);
    if (numSamples > SIZE_MAX / mTargetSampleSize) {
        ALOGW("limiting target size due to overflow (%zu*%zu/%zu)",
                sourceSize, mTargetSampleSize, mSourceSampleSize);
        return SIZE_MAX;
    }
    return numSamples * mTargetSampleSize;
}


static size_t getAudioSampleSize(AudioEncoding e) {
    switch (e) {
        case kAudioEncodingPcm16bit: return 2;
        case kAudioEncodingPcm8bit:  return 1;
        case kAudioEncodingPcmFloat: return 4;
        default: return 0;
    }
}


// static
AudioConverter* AudioConverter::Create(AudioEncoding source, AudioEncoding target) {
    uint32_t sourceSampleSize = getAudioSampleSize(source);
    uint32_t targetSampleSize = getAudioSampleSize(target);
    if (sourceSampleSize && targetSampleSize && sourceSampleSize != targetSampleSize) {
        return new AudioConverter(source, sourceSampleSize, target, targetSampleSize);
    }
    return NULL;
}

status_t AudioConverter::safeConvert(const sp<MediaCodecBuffer> &src, sp<MediaCodecBuffer> &tgt) {
    if (mTo == kAudioEncodingPcm8bit && mFrom == kAudioEncodingPcm16bit) {
        memcpy_to_u8_from_i16((uint8_t*)tgt->base(), (const int16_t*)src->data(), src->size() / 2);
    } else if (mTo == kAudioEncodingPcm8bit && mFrom == kAudioEncodingPcmFloat) {
        memcpy_to_u8_from_float((uint8_t*)tgt->base(), (const float*)src->data(), src->size() / 4);
    } else if (mTo == kAudioEncodingPcm16bit && mFrom == kAudioEncodingPcm8bit) {
        memcpy_to_i16_from_u8((int16_t*)tgt->base(), (const uint8_t*)src->data(), src->size());
    } else if (mTo == kAudioEncodingPcm16bit && mFrom == kAudioEncodingPcmFloat) {
        memcpy_to_i16_from_float((int16_t*)tgt->base(), (const float*)src->data(), src->size() / 4);
    } else if (mTo == kAudioEncodingPcmFloat && mFrom == kAudioEncodingPcm8bit) {
        memcpy_to_float_from_u8((float*)tgt->base(), (const uint8_t*)src->data(), src->size());
    } else if (mTo == kAudioEncodingPcmFloat && mFrom == kAudioEncodingPcm16bit) {
        memcpy_to_float_from_i16((float*)tgt->base(), (const int16_t*)src->data(), src->size() / 2);
    } else {
        return INVALID_OPERATION;
    }
    return OK;
}

} // namespace android
