/*
 * Copyright (C) 2011 The Android Open Source Project
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
#define LOG_TAG "SoftMPEG4"
#include <utils/Log.h>

#include "SoftMPEG4.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>

#include "mp4dec_api.h"

namespace android {

static const CodecProfileLevel kM4VProfileLevels[] = {
    { OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level3 },
};

static const CodecProfileLevel kH263ProfileLevels[] = {
    { OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level30 },
    { OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level45 },
    { OMX_VIDEO_H263ProfileISWV2,    OMX_VIDEO_H263Level30 },
    { OMX_VIDEO_H263ProfileISWV2,    OMX_VIDEO_H263Level45 },
};

SoftMPEG4::SoftMPEG4(
        const char *name,
        const char *componentRole,
        OMX_VIDEO_CODINGTYPE codingType,
        const CodecProfileLevel *profileLevels,
        size_t numProfileLevels,
        const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData,
        OMX_COMPONENTTYPE **component)
    : SoftVideoDecoderOMXComponent(
            name, componentRole, codingType, profileLevels, numProfileLevels,
            352 /* width */, 288 /* height */, callbacks, appData, component),
      mMode(codingType == OMX_VIDEO_CodingH263 ? MODE_H263 : MODE_MPEG4),
      mHandle(new tagvideoDecControls),
      mInputBufferCount(0),
      mSignalledError(false),
      mInitialized(false),
      mFramesConfigured(false),
      mNumSamplesOutput(0),
      mPvTime(0) {
    initPorts(
            kNumInputBuffers,
            352 * 288 * 3 / 2 /* minInputBufferSize */,
            kNumOutputBuffers,
            (mMode == MODE_MPEG4)
            ? MEDIA_MIMETYPE_VIDEO_MPEG4 : MEDIA_MIMETYPE_VIDEO_H263);
    CHECK_EQ(initDecoder(), (status_t)OK);
}

SoftMPEG4::~SoftMPEG4() {
    if (mInitialized) {
        PVCleanUpVideoDecoder(mHandle);
    }

    delete mHandle;
    mHandle = NULL;
}

status_t SoftMPEG4::initDecoder() {
    memset(mHandle, 0, sizeof(tagvideoDecControls));
    return OK;
}

void SoftMPEG4::onQueueFilled(OMX_U32 /* portIndex */) {
    if (mSignalledError || mOutputPortSettingsChange != NONE) {
        return;
    }

    List<BufferInfo *> &inQueue = getPortQueue(0);
    List<BufferInfo *> &outQueue = getPortQueue(1);

    while (!inQueue.empty() && outQueue.size() == kNumOutputBuffers) {
        BufferInfo *inInfo = *inQueue.begin();
        OMX_BUFFERHEADERTYPE *inHeader = inInfo->mHeader;
        if (inHeader == NULL) {
            inQueue.erase(inQueue.begin());
            inInfo->mOwnedByUs = false;
            continue;
        }

        PortInfo *port = editPortInfo(1);

        OMX_BUFFERHEADERTYPE *outHeader =
            port->mBuffers.editItemAt(mNumSamplesOutput & 1).mHeader;

        if (inHeader->nFilledLen == 0) {
            inQueue.erase(inQueue.begin());
            inInfo->mOwnedByUs = false;
            notifyEmptyBufferDone(inHeader);

            ++mInputBufferCount;

            if (inHeader->nFlags & OMX_BUFFERFLAG_EOS) {
                outHeader->nFilledLen = 0;
                outHeader->nFlags = OMX_BUFFERFLAG_EOS;

                List<BufferInfo *>::iterator it = outQueue.begin();
                while (it != outQueue.end() && (*it)->mHeader != outHeader) {
                    ++it;
                }
                if (it == outQueue.end()) {
                    ALOGE("couldn't find port buffer %d in outQueue: b/109891727", mNumSamplesOutput & 1);
                    android_errorWriteLog(0x534e4554, "109891727");
                    return;
                }

                BufferInfo *outInfo = *it;
                outInfo->mOwnedByUs = false;
                outQueue.erase(it);
                outInfo = NULL;

                notifyFillBufferDone(outHeader);
                outHeader = NULL;
            }
            return;
        }

        uint8_t *bitstream = inHeader->pBuffer + inHeader->nOffset;
        uint32_t *start_code = (uint32_t *)bitstream;
        bool volHeader = *start_code == 0xB0010000;
        if (volHeader) {
            PVCleanUpVideoDecoder(mHandle);
            mInitialized = false;
        }

        if (!mInitialized) {
            uint8_t *vol_data[1];
            int32_t vol_size = 0;

            vol_data[0] = NULL;

            if ((inHeader->nFlags & OMX_BUFFERFLAG_CODECCONFIG) || volHeader) {
                vol_data[0] = bitstream;
                vol_size = inHeader->nFilledLen;
            }

            MP4DecodingMode mode =
                (mMode == MODE_MPEG4) ? MPEG4_MODE : H263_MODE;

            Bool success = PVInitVideoDecoder(
                    mHandle, vol_data, &vol_size, 1,
                    outputBufferWidth(), outputBufferHeight(), mode);

            if (!success) {
                ALOGW("PVInitVideoDecoder failed. Unsupported content?");

                notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                mSignalledError = true;
                return;
            }

            MP4DecodingMode actualMode = PVGetDecBitstreamMode(mHandle);
            if (mode != actualMode) {
                notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                mSignalledError = true;
                return;
            }

            PVSetPostProcType((VideoDecControls *) mHandle, 0);

            bool hasFrameData = false;
            if (inHeader->nFlags & OMX_BUFFERFLAG_CODECCONFIG) {
                inInfo->mOwnedByUs = false;
                inQueue.erase(inQueue.begin());
                inInfo = NULL;
                notifyEmptyBufferDone(inHeader);
                inHeader = NULL;
            } else if (volHeader) {
                hasFrameData = true;
            }

            mInitialized = true;

            if (mode == MPEG4_MODE && handlePortSettingsChange()) {
                return;
            }

            if (!hasFrameData) {
                continue;
            }
        }

        if (!mFramesConfigured) {
            PortInfo *port = editPortInfo(1);
            OMX_BUFFERHEADERTYPE *outHeader = port->mBuffers.editItemAt(1).mHeader;

            OMX_U32 yFrameSize = sizeof(uint8) * mHandle->size;
            if ((outHeader->nAllocLen < yFrameSize) ||
                    (outHeader->nAllocLen - yFrameSize < yFrameSize / 2)) {
                ALOGE("Too small output buffer for reference frame: %lu bytes",
                        (unsigned long)outHeader->nAllocLen);
                android_errorWriteLog(0x534e4554, "30033990");
                notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                mSignalledError = true;
                return;
            }
            PVSetReferenceYUV(mHandle, outHeader->pBuffer);
            mFramesConfigured = true;
        }

        uint32_t useExtTimestamp = (inHeader->nOffset == 0);

        // decoder deals in ms (int32_t), OMX in us (int64_t)
        // so use fake timestamp instead
        uint32_t timestamp = 0xFFFFFFFF;
        if (useExtTimestamp) {
            mPvToOmxTimeMap.add(mPvTime, inHeader->nTimeStamp);
            timestamp = mPvTime;
            mPvTime++;
        }

        int32_t bufferSize = inHeader->nFilledLen;
        int32_t tmp = bufferSize;

        OMX_U32 frameSize;
        OMX_U64 yFrameSize = (OMX_U64)mWidth * (OMX_U64)mHeight;
        if (yFrameSize > ((OMX_U64)UINT32_MAX / 3) * 2) {
            ALOGE("Frame size too large");
            notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
            mSignalledError = true;
            return;
        }
        frameSize = (OMX_U32)(yFrameSize + (yFrameSize / 2));

        if (outHeader->nAllocLen < frameSize) {
            android_errorWriteLog(0x534e4554, "27833616");
            ALOGE("Insufficient output buffer size");
            notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
            mSignalledError = true;
            return;
        }

        // Need to check if header contains new info, e.g., width/height, etc.
        VopHeaderInfo header_info;
        uint8_t *bitstreamTmp = bitstream;
        if (PVDecodeVopHeader(
                    mHandle, &bitstreamTmp, &timestamp, &tmp,
                    &header_info, &useExtTimestamp,
                    outHeader->pBuffer) != PV_TRUE) {
            ALOGE("failed to decode vop header.");

            notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
            mSignalledError = true;
            return;
        }
        if (handlePortSettingsChange()) {
            return;
        }

        // The PV decoder is lying to us, sometimes it'll claim to only have
        // consumed a subset of the buffer when it clearly consumed all of it.
        // ignore whatever it says...
        if (PVDecodeVopBody(mHandle, &tmp) != PV_TRUE) {
            ALOGE("failed to decode video frame.");

            notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
            mSignalledError = true;
            return;
        }

        // H263 doesn't have VOL header, the frame size information is in short header, i.e. the
        // decoder may detect size change after PVDecodeVideoFrame.
        if (handlePortSettingsChange()) {
            return;
        }

        // decoder deals in ms, OMX in us.
        outHeader->nTimeStamp = mPvToOmxTimeMap.valueFor(timestamp);
        mPvToOmxTimeMap.removeItem(timestamp);

        inHeader->nOffset += bufferSize;
        inHeader->nFilledLen = 0;
        if (inHeader->nFlags & OMX_BUFFERFLAG_EOS) {
            outHeader->nFlags = OMX_BUFFERFLAG_EOS;
        } else {
            outHeader->nFlags = 0;
        }

        if (inHeader->nFilledLen == 0) {
            inInfo->mOwnedByUs = false;
            inQueue.erase(inQueue.begin());
            inInfo = NULL;
            notifyEmptyBufferDone(inHeader);
            inHeader = NULL;
        }

        ++mInputBufferCount;

        outHeader->nOffset = 0;
        outHeader->nFilledLen = frameSize;

        List<BufferInfo *>::iterator it = outQueue.begin();
        while ((*it)->mHeader != outHeader) {
            ++it;
        }

        BufferInfo *outInfo = *it;
        outInfo->mOwnedByUs = false;
        outQueue.erase(it);
        outInfo = NULL;

        notifyFillBufferDone(outHeader);
        outHeader = NULL;

        ++mNumSamplesOutput;
    }
}

bool SoftMPEG4::handlePortSettingsChange() {
    uint32_t disp_width, disp_height;
    PVGetVideoDimensions(mHandle, (int32 *)&disp_width, (int32 *)&disp_height);

    uint32_t buf_width, buf_height;
    PVGetBufferDimensions(mHandle, (int32 *)&buf_width, (int32 *)&buf_height);

    CHECK_LE(disp_width, buf_width);
    CHECK_LE(disp_height, buf_height);

    ALOGV("disp_width = %d, disp_height = %d, buf_width = %d, buf_height = %d",
            disp_width, disp_height, buf_width, buf_height);

    CropSettingsMode cropSettingsMode = kCropUnSet;
    if (disp_width != buf_width || disp_height != buf_height) {
        cropSettingsMode = kCropSet;

        if (mCropWidth != disp_width || mCropHeight != disp_height) {
            mCropLeft = 0;
            mCropTop = 0;
            mCropWidth = disp_width;
            mCropHeight = disp_height;
            cropSettingsMode = kCropChanged;
        }
    }

    bool portWillReset = false;
    const bool fakeStride = true;
    SoftVideoDecoderOMXComponent::handlePortSettingsChange(
            &portWillReset, buf_width, buf_height,
            OMX_COLOR_FormatYUV420Planar, cropSettingsMode, fakeStride);
    if (portWillReset) {
        if (mMode == MODE_H263) {
            PVCleanUpVideoDecoder(mHandle);

            uint8_t *vol_data[1];
            int32_t vol_size = 0;

            vol_data[0] = NULL;
            if (!PVInitVideoDecoder(
                    mHandle, vol_data, &vol_size, 1, outputBufferWidth(), outputBufferHeight(),
                    H263_MODE)) {
                notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                mSignalledError = true;
                return true;
            }
        }

        mFramesConfigured = false;
    }

    return portWillReset;
}

void SoftMPEG4::onPortFlushCompleted(OMX_U32 portIndex) {
    if (portIndex == 0 && mInitialized) {
        CHECK_EQ((int)PVResetVideoDecoder(mHandle), (int)PV_TRUE);
    }
    mFramesConfigured = false;
}

void SoftMPEG4::onReset() {
    SoftVideoDecoderOMXComponent::onReset();
    mPvToOmxTimeMap.clear();
    mSignalledError = false;
    mFramesConfigured = false;
    if (mInitialized) {
        PVCleanUpVideoDecoder(mHandle);
        mInitialized = false;
    }
}

void SoftMPEG4::updatePortDefinitions(bool updateCrop, bool updateInputSize) {
    SoftVideoDecoderOMXComponent::updatePortDefinitions(updateCrop, updateInputSize);

    /* We have to align our width and height - this should affect stride! */
    OMX_PARAM_PORTDEFINITIONTYPE *def = &editPortInfo(kOutputPortIndex)->mDef;
    def->format.video.nStride = align(def->format.video.nStride, 16);
    def->format.video.nSliceHeight = align(def->format.video.nSliceHeight, 16);
    def->nBufferSize = (def->format.video.nStride * def->format.video.nSliceHeight * 3) / 2;
}

}  // namespace android

android::SoftOMXComponent *createSoftOMXComponent(
        const char *name, const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData, OMX_COMPONENTTYPE **component) {
    using namespace android;
    if (!strcmp(name, "OMX.google.h263.decoder")) {
        return new android::SoftMPEG4(
                name, "video_decoder.h263", OMX_VIDEO_CodingH263,
                kH263ProfileLevels, ARRAY_SIZE(kH263ProfileLevels),
                callbacks, appData, component);
    } else if (!strcmp(name, "OMX.google.mpeg4.decoder")) {
        return new android::SoftMPEG4(
                name, "video_decoder.mpeg4", OMX_VIDEO_CodingMPEG4,
                kM4VProfileLevels, ARRAY_SIZE(kM4VProfileLevels),
                callbacks, appData, component);
    } else {
        CHECK(!"Unknown component");
    }
    return NULL;
}

