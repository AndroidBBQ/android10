/*
 * Copyright (C) 2012 The Android Open Source Project
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
#define LOG_TAG "SoftMPEG4Encoder"
#include <utils/Log.h>
#include <utils/misc.h>

#include "mp4enc_api.h"
#include "OMX_Video.h"

#include <media/hardware/HardwareAPI.h>
#include <media/hardware/MetadataBufferType.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MetaData.h>

#include "SoftMPEG4Encoder.h"

#include <inttypes.h>

#ifndef INT32_MAX
#define INT32_MAX   2147483647
#endif

namespace android {

template<class T>
static void InitOMXParams(T *params) {
    params->nSize = sizeof(T);
    params->nVersion.s.nVersionMajor = 1;
    params->nVersion.s.nVersionMinor = 0;
    params->nVersion.s.nRevision = 0;
    params->nVersion.s.nStep = 0;
}

static const CodecProfileLevel kMPEG4ProfileLevels[] = {
    { OMX_VIDEO_MPEG4ProfileCore, OMX_VIDEO_MPEG4Level2 },
};

static const CodecProfileLevel kH263ProfileLevels[] = {
    { OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level45 },
};

SoftMPEG4Encoder::SoftMPEG4Encoder(
            const char *name,
            const char *componentRole,
            OMX_VIDEO_CODINGTYPE codingType,
            const char *mime,
            const CodecProfileLevel *profileLevels,
            size_t numProfileLevels,
            const OMX_CALLBACKTYPE *callbacks,
            OMX_PTR appData,
            OMX_COMPONENTTYPE **component)
    : SoftVideoEncoderOMXComponent(
            name, componentRole, codingType,
            profileLevels, numProfileLevels,
            176 /* width */, 144 /* height */,
            callbacks, appData, component),
      mEncodeMode(COMBINE_MODE_WITH_ERR_RES),
      mKeyFrameInterval(30),
      mNumInputFrames(-1),
      mStarted(false),
      mSawInputEOS(false),
      mSignalledError(false),
      mHandle(new tagvideoEncControls),
      mEncParams(new tagvideoEncOptions),
      mInputFrameData(NULL) {

    if (codingType == OMX_VIDEO_CodingH263) {
        mEncodeMode = H263_MODE;
    }

    // 256 * 1024 is a magic number for PV's encoder, not sure why
    const size_t kOutputBufferSize = 256 * 1024;

    initPorts(kNumBuffers, kNumBuffers, kOutputBufferSize, mime);

    ALOGI("Construct SoftMPEG4Encoder");
}

SoftMPEG4Encoder::~SoftMPEG4Encoder() {
    ALOGV("Destruct SoftMPEG4Encoder");
    onReset();
    releaseEncoder();
    List<BufferInfo *> &outQueue = getPortQueue(1);
    List<BufferInfo *> &inQueue = getPortQueue(0);
    CHECK(outQueue.empty());
    CHECK(inQueue.empty());
}

OMX_ERRORTYPE SoftMPEG4Encoder::initEncParams() {
    CHECK(mHandle != NULL);
    memset(mHandle, 0, sizeof(tagvideoEncControls));

    CHECK(mEncParams != NULL);
    memset(mEncParams, 0, sizeof(tagvideoEncOptions));
    if (!PVGetDefaultEncOption(mEncParams, 0)) {
        ALOGE("Failed to get default encoding parameters");
        return OMX_ErrorUndefined;
    }
    if (mFramerate == 0) {
        ALOGE("Framerate should not be 0");
        return OMX_ErrorUndefined;
    }
    mEncParams->encMode = mEncodeMode;
    mEncParams->encWidth[0] = mWidth;
    mEncParams->encHeight[0] = mHeight;
    mEncParams->encFrameRate[0] = mFramerate >> 16; // mFramerate is in Q16 format
    mEncParams->rcType = VBR_1;
    mEncParams->vbvDelay = 5.0f;

    // FIXME:
    // Add more profile and level support for MPEG4 encoder
    mEncParams->profile_level = CORE_PROFILE_LEVEL2;
    mEncParams->packetSize = 32;
    mEncParams->rvlcEnable = PV_OFF;
    mEncParams->numLayers = 1;
    mEncParams->timeIncRes = 1000;
    mEncParams->tickPerSrc = ((int64_t)mEncParams->timeIncRes << 16) / mFramerate;

    mEncParams->bitRate[0] = mBitrate;
    mEncParams->iQuant[0] = 15;
    mEncParams->pQuant[0] = 12;
    mEncParams->quantType[0] = 0;
    mEncParams->noFrameSkipped = PV_OFF;

    if (mColorFormat != OMX_COLOR_FormatYUV420Planar || mInputDataIsMeta) {
        // Color conversion is needed.
        free(mInputFrameData);
        mInputFrameData = NULL;
        if (((uint64_t)mWidth * mHeight) > ((uint64_t)INT32_MAX / 3)) {
            ALOGE("b/25812794, Buffer size is too big.");
            return OMX_ErrorBadParameter;
        }
        mInputFrameData =
            (uint8_t *) malloc((mWidth * mHeight * 3 ) >> 1);
        CHECK(mInputFrameData != NULL);
    }

    // PV's MPEG4 encoder requires the video dimension of multiple
    if (mWidth % 16 != 0 || mHeight % 16 != 0) {
        ALOGE("Video frame size %dx%d must be a multiple of 16",
            mWidth, mHeight);
        return OMX_ErrorBadParameter;
    }

    // Set IDR frame refresh interval
    mEncParams->intraPeriod = mKeyFrameInterval;

    mEncParams->numIntraMB = 0;
    mEncParams->sceneDetect = PV_ON;
    mEncParams->searchRange = 16;
    mEncParams->mv8x8Enable = PV_OFF;
    mEncParams->gobHeaderInterval = 0;
    mEncParams->useACPred = PV_ON;
    mEncParams->intraDCVlcTh = 0;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftMPEG4Encoder::initEncoder() {
    CHECK(!mStarted);

    OMX_ERRORTYPE errType = OMX_ErrorNone;
    if (OMX_ErrorNone != (errType = initEncParams())) {
        ALOGE("Failed to initialized encoder params");
        mSignalledError = true;
        notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
        return errType;
    }

    if (!PVInitVideoEncoder(mHandle, mEncParams)) {
        ALOGE("Failed to initialize the encoder");
        mSignalledError = true;
        notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
        return OMX_ErrorUndefined;
    }

    mNumInputFrames = -1;  // 1st buffer for codec specific data
    mStarted = true;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftMPEG4Encoder::releaseEncoder() {
    if (mEncParams) {
        delete mEncParams;
        mEncParams = NULL;
    }

    if (mHandle) {
        delete mHandle;
        mHandle = NULL;
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE SoftMPEG4Encoder::internalGetParameter(
        OMX_INDEXTYPE index, OMX_PTR params) {
    switch (index) {
        case OMX_IndexParamVideoBitrate:
        {
            OMX_VIDEO_PARAM_BITRATETYPE *bitRate =
                (OMX_VIDEO_PARAM_BITRATETYPE *) params;

            if (!isValidOMXParam(bitRate)) {
                return OMX_ErrorBadParameter;
            }

            if (bitRate->nPortIndex != 1) {
                return OMX_ErrorUndefined;
            }

            bitRate->eControlRate = OMX_Video_ControlRateVariable;
            bitRate->nTargetBitrate = mBitrate;
            return OMX_ErrorNone;
        }

        case OMX_IndexParamVideoH263:
        {
            OMX_VIDEO_PARAM_H263TYPE *h263type =
                (OMX_VIDEO_PARAM_H263TYPE *)params;

            if (!isValidOMXParam(h263type)) {
                return OMX_ErrorBadParameter;
            }

            if (h263type->nPortIndex != 1) {
                return OMX_ErrorUndefined;
            }

            h263type->nAllowedPictureTypes =
                (OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP);
            h263type->eProfile = OMX_VIDEO_H263ProfileBaseline;
            h263type->eLevel = OMX_VIDEO_H263Level45;
            h263type->bPLUSPTYPEAllowed = OMX_FALSE;
            h263type->bForceRoundingTypeToZero = OMX_FALSE;
            h263type->nPictureHeaderRepetition = 0;
            h263type->nGOBHeaderInterval = 0;

            return OMX_ErrorNone;
        }

        case OMX_IndexParamVideoMpeg4:
        {
            OMX_VIDEO_PARAM_MPEG4TYPE *mpeg4type =
                (OMX_VIDEO_PARAM_MPEG4TYPE *)params;

            if (!isValidOMXParam(mpeg4type)) {
                return OMX_ErrorBadParameter;
            }

            if (mpeg4type->nPortIndex != 1) {
                return OMX_ErrorUndefined;
            }

            mpeg4type->eProfile = OMX_VIDEO_MPEG4ProfileCore;
            mpeg4type->eLevel = OMX_VIDEO_MPEG4Level2;
            mpeg4type->nAllowedPictureTypes =
                (OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP);
            mpeg4type->nBFrames = 0;
            mpeg4type->nIDCVLCThreshold = 0;
            mpeg4type->bACPred = OMX_TRUE;
            mpeg4type->nMaxPacketSize = 256;
            mpeg4type->nTimeIncRes = 1000;
            mpeg4type->nHeaderExtension = 0;
            mpeg4type->bReversibleVLC = OMX_FALSE;

            return OMX_ErrorNone;
        }

        default:
            return SoftVideoEncoderOMXComponent::internalGetParameter(index, params);
    }
}

OMX_ERRORTYPE SoftMPEG4Encoder::internalSetParameter(
        OMX_INDEXTYPE index, const OMX_PTR params) {
    int32_t indexFull = index;

    switch (indexFull) {
        case OMX_IndexParamVideoBitrate:
        {
            OMX_VIDEO_PARAM_BITRATETYPE *bitRate =
                (OMX_VIDEO_PARAM_BITRATETYPE *) params;

            if (!isValidOMXParam(bitRate)) {
                return OMX_ErrorBadParameter;
            }

            if (bitRate->nPortIndex != 1 ||
                bitRate->eControlRate != OMX_Video_ControlRateVariable) {
                return OMX_ErrorUndefined;
            }

            mBitrate = bitRate->nTargetBitrate;
            return OMX_ErrorNone;
        }

        case OMX_IndexParamVideoH263:
        {
            OMX_VIDEO_PARAM_H263TYPE *h263type =
                (OMX_VIDEO_PARAM_H263TYPE *)params;

            if (!isValidOMXParam(h263type)) {
                return OMX_ErrorBadParameter;
            }

            if (h263type->nPortIndex != 1) {
                return OMX_ErrorUndefined;
            }

            if (h263type->eProfile != OMX_VIDEO_H263ProfileBaseline ||
                h263type->eLevel != OMX_VIDEO_H263Level45 ||
                (h263type->nAllowedPictureTypes & OMX_VIDEO_PictureTypeB) ||
                h263type->bPLUSPTYPEAllowed != OMX_FALSE ||
                h263type->bForceRoundingTypeToZero != OMX_FALSE ||
                h263type->nPictureHeaderRepetition != 0 ||
                h263type->nGOBHeaderInterval != 0) {
                return OMX_ErrorUndefined;
            }

            return OMX_ErrorNone;
        }

        case OMX_IndexParamVideoMpeg4:
        {
            OMX_VIDEO_PARAM_MPEG4TYPE *mpeg4type =
                (OMX_VIDEO_PARAM_MPEG4TYPE *)params;

            if (!isValidOMXParam(mpeg4type)) {
                return OMX_ErrorBadParameter;
            }

            if (mpeg4type->nPortIndex != 1) {
                return OMX_ErrorUndefined;
            }

            if (mpeg4type->eProfile != OMX_VIDEO_MPEG4ProfileCore ||
                mpeg4type->eLevel != OMX_VIDEO_MPEG4Level2 ||
                (mpeg4type->nAllowedPictureTypes & OMX_VIDEO_PictureTypeB) ||
                mpeg4type->nBFrames != 0 ||
                mpeg4type->nIDCVLCThreshold != 0 ||
                mpeg4type->bACPred != OMX_TRUE ||
                mpeg4type->nMaxPacketSize != 256 ||
                mpeg4type->nTimeIncRes != 1000 ||
                mpeg4type->nHeaderExtension != 0 ||
                mpeg4type->bReversibleVLC != OMX_FALSE) {
                return OMX_ErrorUndefined;
            }

            mKeyFrameInterval = int32_t(mpeg4type->nPFrames + 1);

            return OMX_ErrorNone;
        }

        default:
            return SoftVideoEncoderOMXComponent::internalSetParameter(index, params);
    }
}

void SoftMPEG4Encoder::onQueueFilled(OMX_U32 /* portIndex */) {
    if (mSignalledError || mSawInputEOS) {
        return;
    }

    if (!mStarted) {
        if (OMX_ErrorNone != initEncoder()) {
            return;
        }
    }

    List<BufferInfo *> &inQueue = getPortQueue(0);
    List<BufferInfo *> &outQueue = getPortQueue(1);

    while (!mSawInputEOS && !inQueue.empty() && !outQueue.empty()) {
        BufferInfo *inInfo = *inQueue.begin();
        OMX_BUFFERHEADERTYPE *inHeader = inInfo->mHeader;
        BufferInfo *outInfo = *outQueue.begin();
        OMX_BUFFERHEADERTYPE *outHeader = outInfo->mHeader;

        outHeader->nTimeStamp = 0;
        outHeader->nFlags = 0;
        outHeader->nOffset = 0;
        outHeader->nFilledLen = 0;
        outHeader->nOffset = 0;

        uint8_t *outPtr = (uint8_t *) outHeader->pBuffer;
        int32_t dataLength = outHeader->nAllocLen;

        if (mNumInputFrames < 0) {
            if (!PVGetVolHeader(mHandle, outPtr, &dataLength, 0)) {
                ALOGE("Failed to get VOL header");
                mSignalledError = true;
                notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
                return;
            }
            ALOGV("Output VOL header: %d bytes", dataLength);
            ++mNumInputFrames;
            outHeader->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;
            outHeader->nFilledLen = dataLength;
            outQueue.erase(outQueue.begin());
            outInfo->mOwnedByUs = false;
            notifyFillBufferDone(outHeader);
            return;
        }

        // Save the input buffer info so that it can be
        // passed to an output buffer
        InputBufferInfo info;
        info.mTimeUs = inHeader->nTimeStamp;
        info.mFlags = inHeader->nFlags;
        mInputBufferInfoVec.push(info);

        if (inHeader->nFlags & OMX_BUFFERFLAG_EOS) {
            mSawInputEOS = true;
        }

        if (inHeader->nFilledLen > 0) {
            OMX_ERRORTYPE error = validateInputBuffer(inHeader);
            if (error != OMX_ErrorNone) {
                ALOGE("b/69065651");
                android_errorWriteLog(0x534e4554, "69065651");
                mSignalledError = true;
                notify(OMX_EventError, error, 0, 0);
                return;
            }
            const uint8_t *inputData = NULL;
            if (mInputDataIsMeta) {
                inputData =
                    extractGraphicBuffer(
                            mInputFrameData, (mWidth * mHeight * 3) >> 1,
                            inHeader->pBuffer + inHeader->nOffset, inHeader->nFilledLen,
                            mWidth, mHeight);
                if (inputData == NULL) {
                    ALOGE("Unable to extract gralloc buffer in metadata mode");
                    mSignalledError = true;
                    notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
                        return;
                }
            } else {
                inputData = (const uint8_t *)inHeader->pBuffer + inHeader->nOffset;
                if (mColorFormat != OMX_COLOR_FormatYUV420Planar) {
                    ConvertYUV420SemiPlanarToYUV420Planar(
                        inputData, mInputFrameData, mWidth, mHeight);
                    inputData = mInputFrameData;
                }
            }

            CHECK(inputData != NULL);

            VideoEncFrameIO vin, vout;
            memset(&vin, 0, sizeof(vin));
            memset(&vout, 0, sizeof(vout));
            vin.height = align(mHeight, 16);
            vin.pitch = align(mWidth, 16);
            vin.timestamp = (inHeader->nTimeStamp + 500) / 1000;  // in ms
            vin.yChan = (uint8_t *)inputData;
            vin.uChan = vin.yChan + vin.height * vin.pitch;
            vin.vChan = vin.uChan + ((vin.height * vin.pitch) >> 2);

            ULong modTimeMs = 0;
            int32_t nLayer = 0;
            MP4HintTrack hintTrack;
            if (!PVEncodeVideoFrame(mHandle, &vin, &vout,
                    &modTimeMs, outPtr, &dataLength, &nLayer) ||
                !PVGetHintTrack(mHandle, &hintTrack)) {
                ALOGE("Failed to encode frame or get hink track at frame %" PRId64,
                    mNumInputFrames);
                mSignalledError = true;
                notify(OMX_EventError, OMX_ErrorUndefined, 0, 0);
            }
            CHECK(NULL == PVGetOverrunBuffer(mHandle));
            if (hintTrack.CodeType == 0) {  // I-frame serves as sync frame
                outHeader->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
            }

            ++mNumInputFrames;
        } else {
            dataLength = 0;
        }

        inQueue.erase(inQueue.begin());
        inInfo->mOwnedByUs = false;
        notifyEmptyBufferDone(inHeader);

        outQueue.erase(outQueue.begin());
        CHECK(!mInputBufferInfoVec.empty());
        InputBufferInfo *inputBufInfo = mInputBufferInfoVec.begin();
        outHeader->nTimeStamp = inputBufInfo->mTimeUs;
        outHeader->nFlags |= (inputBufInfo->mFlags | OMX_BUFFERFLAG_ENDOFFRAME);
        outHeader->nFilledLen = dataLength;
        mInputBufferInfoVec.erase(mInputBufferInfoVec.begin());
        outInfo->mOwnedByUs = false;
        notifyFillBufferDone(outHeader);
    }
}

void SoftMPEG4Encoder::onReset() {
    if (!mStarted) {
        return;
    }

    PVCleanUpVideoEncoder(mHandle);

    free(mInputFrameData);
    mInputFrameData = NULL;

    mStarted = false;
}

}  // namespace android

android::SoftOMXComponent *createSoftOMXComponent(
        const char *name, const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData, OMX_COMPONENTTYPE **component) {
    using namespace android;
    if (!strcmp(name, "OMX.google.h263.encoder")) {
        return new android::SoftMPEG4Encoder(
                name, "video_encoder.h263", OMX_VIDEO_CodingH263, MEDIA_MIMETYPE_VIDEO_H263,
                kH263ProfileLevels, NELEM(kH263ProfileLevels),
                callbacks, appData, component);
    } else if (!strcmp(name, "OMX.google.mpeg4.encoder")) {
        return new android::SoftMPEG4Encoder(
                name, "video_encoder.mpeg4", OMX_VIDEO_CodingMPEG4, MEDIA_MIMETYPE_VIDEO_MPEG4,
                kMPEG4ProfileLevels, NELEM(kMPEG4ProfileLevels),
                callbacks, appData, component);
    } else {
        CHECK(!"Unknown component");
    }
    return NULL;
}
