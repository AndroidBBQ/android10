/*
 * Copyright 2017 The Android Open Source Project
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

#ifndef AAUDIO_STREAM_PARAMETERS_H
#define AAUDIO_STREAM_PARAMETERS_H

#include <stdint.h>

#include <aaudio/AAudio.h>
#include <utility/AAudioUtilities.h>

namespace aaudio {

class AAudioStreamParameters {
public:
    AAudioStreamParameters();
    virtual ~AAudioStreamParameters();

    int32_t getDeviceId() const {
        return mDeviceId;
    }

    void setDeviceId(int32_t deviceId) {
        mDeviceId = deviceId;
    }

    int32_t getSampleRate() const {
        return mSampleRate;
    }

    void setSampleRate(int32_t sampleRate) {
        mSampleRate = sampleRate;
    }

    int32_t getSamplesPerFrame() const {
        return mSamplesPerFrame;
    }

    /**
     * This is also known as channelCount.
     */
    void setSamplesPerFrame(int32_t samplesPerFrame) {
        mSamplesPerFrame = samplesPerFrame;
    }

    audio_format_t getFormat() const {
        return mAudioFormat;
    }

    void setFormat(audio_format_t audioFormat) {
        mAudioFormat = audioFormat;
    }

    aaudio_sharing_mode_t getSharingMode() const {
        return mSharingMode;
    }

    void setSharingMode(aaudio_sharing_mode_t sharingMode) {
        mSharingMode = sharingMode;
    }

    int32_t getBufferCapacity() const {
        return mBufferCapacity;
    }

    void setBufferCapacity(int32_t frames) {
        mBufferCapacity = frames;
    }

    aaudio_direction_t getDirection() const {
        return mDirection;
    }

    void setDirection(aaudio_direction_t direction) {
        mDirection = direction;
    }

    aaudio_usage_t getUsage() const {
        return mUsage;
    }

    void setUsage(aaudio_usage_t usage) {
        mUsage = usage;
    }

    aaudio_content_type_t getContentType() const {
        return mContentType;
    }

    void setContentType(aaudio_content_type_t contentType) {
        mContentType = contentType;
    }

    aaudio_input_preset_t getInputPreset() const {
        return mInputPreset;
    }

    void setInputPreset(aaudio_input_preset_t inputPreset) {
        mInputPreset = inputPreset;
    }

    aaudio_allowed_capture_policy_t getAllowedCapturePolicy() const {
        return mAllowedCapturePolicy;
    }

    void setAllowedCapturePolicy(aaudio_allowed_capture_policy_t policy) {
        mAllowedCapturePolicy = policy;
    }

    aaudio_session_id_t getSessionId() const {
        return mSessionId;
    }

    void setSessionId(aaudio_session_id_t sessionId) {
        mSessionId = sessionId;
    }

    /**
     * @return bytes per frame of getFormat()
     */
    int32_t calculateBytesPerFrame() const {
        return getSamplesPerFrame() * audio_bytes_per_sample(getFormat());
    }

    /**
     * Copy variables defined in other AAudioStreamParameters instance to this one.
     * @param other
     */
    void copyFrom(const AAudioStreamParameters &other);

    virtual aaudio_result_t validate() const;

    void dump() const;

private:
    int32_t                         mSamplesPerFrame      = AAUDIO_UNSPECIFIED;
    int32_t                         mSampleRate           = AAUDIO_UNSPECIFIED;
    int32_t                         mDeviceId             = AAUDIO_UNSPECIFIED;
    aaudio_sharing_mode_t           mSharingMode          = AAUDIO_SHARING_MODE_SHARED;
    audio_format_t                  mAudioFormat          = AUDIO_FORMAT_DEFAULT;
    aaudio_direction_t              mDirection            = AAUDIO_DIRECTION_OUTPUT;
    aaudio_usage_t                  mUsage                = AAUDIO_UNSPECIFIED;
    aaudio_content_type_t           mContentType          = AAUDIO_UNSPECIFIED;
    aaudio_input_preset_t           mInputPreset          = AAUDIO_UNSPECIFIED;
    int32_t                         mBufferCapacity       = AAUDIO_UNSPECIFIED;
    aaudio_allowed_capture_policy_t mAllowedCapturePolicy = AAUDIO_UNSPECIFIED;
    aaudio_session_id_t             mSessionId            = AAUDIO_SESSION_ID_NONE;
};

} /* namespace aaudio */

#endif //AAUDIO_STREAM_PARAMETERS_H
