/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef ANDROID_AUDIO_IO_DESCRIPTOR_H
#define ANDROID_AUDIO_IO_DESCRIPTOR_H

namespace android {

enum audio_io_config_event {
    AUDIO_OUTPUT_REGISTERED,
    AUDIO_OUTPUT_OPENED,
    AUDIO_OUTPUT_CLOSED,
    AUDIO_OUTPUT_CONFIG_CHANGED,
    AUDIO_INPUT_REGISTERED,
    AUDIO_INPUT_OPENED,
    AUDIO_INPUT_CLOSED,
    AUDIO_INPUT_CONFIG_CHANGED,
    AUDIO_CLIENT_STARTED,
};

// audio input/output descriptor used to cache output configurations in client process to avoid
// frequent calls through IAudioFlinger
class AudioIoDescriptor : public RefBase {
public:
    AudioIoDescriptor() :
        mIoHandle(AUDIO_IO_HANDLE_NONE),
        mSamplingRate(0), mFormat(AUDIO_FORMAT_DEFAULT), mChannelMask(AUDIO_CHANNEL_NONE),
        mFrameCount(0), mFrameCountHAL(0), mLatency(0), mPortId(AUDIO_PORT_HANDLE_NONE)
    {
        memset(&mPatch, 0, sizeof(struct audio_patch));
    }

    virtual ~AudioIoDescriptor() {}

    audio_port_handle_t getDeviceId() {
        if (mPatch.num_sources != 0 && mPatch.num_sinks != 0) {
            if (mPatch.sources[0].type == AUDIO_PORT_TYPE_MIX) {
                // this is an output mix
                // FIXME: the API only returns the first device in case of multiple device selection
                return mPatch.sinks[0].id;
            } else {
                // this is an input mix
                return mPatch.sources[0].id;
            }
        }
        return AUDIO_PORT_HANDLE_NONE;
    }

    audio_io_handle_t       mIoHandle;
    struct audio_patch      mPatch;
    uint32_t                mSamplingRate;
    audio_format_t          mFormat;
    audio_channel_mask_t    mChannelMask;
    size_t                  mFrameCount;
    size_t                  mFrameCountHAL;
    uint32_t                mLatency;   // only valid for output
    audio_port_handle_t     mPortId;    // valid for event AUDIO_CLIENT_STARTED
};


};  // namespace android

#endif  /*ANDROID_AUDIO_IO_DESCRIPTOR_H*/
