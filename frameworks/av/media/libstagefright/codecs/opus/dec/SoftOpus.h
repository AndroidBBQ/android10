/*
 * Copyright (C) 2014 The Android Open Source Project
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

/*
 * The Opus specification is part of IETF RFC 6716:
 * http://tools.ietf.org/html/rfc6716
 */

#ifndef SOFT_OPUS_H_

#define SOFT_OPUS_H_

#include <media/stagefright/omx/SimpleSoftOMXComponent.h>

struct OpusMSDecoder;

namespace android {

struct OpusHeader {
  int channels;
  int skip_samples;
  int channel_mapping;
  int num_streams;
  int num_coupled;
  int16_t gain_db;
  uint8_t stream_map[8];
};

struct SoftOpus : public SimpleSoftOMXComponent {
    SoftOpus(const char *name,
             const OMX_CALLBACKTYPE *callbacks,
             OMX_PTR appData,
             OMX_COMPONENTTYPE **component);

protected:
    virtual ~SoftOpus();

    virtual OMX_ERRORTYPE internalGetParameter(
            OMX_INDEXTYPE index, OMX_PTR params);

    virtual OMX_ERRORTYPE internalSetParameter(
            OMX_INDEXTYPE index, const OMX_PTR params);

    virtual void onQueueFilled(OMX_U32 portIndex);
    virtual void onPortFlushCompleted(OMX_U32 portIndex);
    virtual void onPortEnableCompleted(OMX_U32 portIndex, bool enabled);
    virtual void onReset();

private:
    enum {
        kNumBuffers = 4,
        kMaxNumSamplesPerBuffer = 960 * 6
    };

    size_t mInputBufferCount;

    OpusMSDecoder *mDecoder;
    OpusHeader *mHeader;

    int64_t mCodecDelay;
    int64_t mSeekPreRoll;
    int64_t mSamplesToDiscard;
    int64_t mAnchorTimeUs;
    int64_t mNumFramesOutput;
    bool mHaveEOS;

    enum {
        NONE,
        AWAITING_DISABLED,
        AWAITING_ENABLED
    } mOutputPortSettingsChange;

    void initPorts();
    status_t initDecoder();
    bool isConfigured() const;
    void handleEOS();

    DISALLOW_EVIL_CONSTRUCTORS(SoftOpus);
};

}  // namespace android

#endif  // SOFT_OPUS_H_
