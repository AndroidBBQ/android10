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

//#define LOG_NDEBUG 0
#define LOG_TAG "SoftHEVC"
#include <utils/Log.h>

#include "ihevc_typedefs.h"
#include "iv.h"
#include "ivd.h"
#include "ihevcd_cxa.h"
#include "SoftHEVC.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/MediaDefs.h>
#include <OMX_VideoExt.h>

namespace android {

#define componentName                   "video_decoder.hevc"
#define codingType                      OMX_VIDEO_CodingHEVC
#define CODEC_MIME_TYPE                 MEDIA_MIMETYPE_VIDEO_HEVC

/** Function and structure definitions to keep code similar for each codec */
#define ivdec_api_function              ihevcd_cxa_api_function
#define ivdext_create_ip_t              ihevcd_cxa_create_ip_t
#define ivdext_create_op_t              ihevcd_cxa_create_op_t
#define ivdext_delete_ip_t              ihevcd_cxa_delete_ip_t
#define ivdext_delete_op_t              ihevcd_cxa_delete_op_t
#define ivdext_ctl_set_num_cores_ip_t   ihevcd_cxa_ctl_set_num_cores_ip_t
#define ivdext_ctl_set_num_cores_op_t   ihevcd_cxa_ctl_set_num_cores_op_t

#define IVDEXT_CMD_CTL_SET_NUM_CORES    \
        (IVD_CONTROL_API_COMMAND_TYPE_T)IHEVCD_CXA_CMD_CTL_SET_NUM_CORES

static const CodecProfileLevel kProfileLevels[] = {
    { OMX_VIDEO_HEVCProfileMain,      OMX_VIDEO_HEVCMainTierLevel51 },
    { OMX_VIDEO_HEVCProfileMainStill, OMX_VIDEO_HEVCMainTierLevel51 },
};

SoftHEVC::SoftHEVC(
        const char *name,
        const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData,
        OMX_COMPONENTTYPE **component)
    : SoftVideoDecoderOMXComponent(name, componentName, codingType,
            kProfileLevels, ARRAY_SIZE(kProfileLevels),
            320 /* width */, 240 /* height */, callbacks,
            appData, component),
      mCodecCtx(NULL),
      mFlushOutBuffer(NULL),
      mOmxColorFormat(OMX_COLOR_FormatYUV420Planar),
      mIvColorFormat(IV_YUV_420P),
      mChangingResolution(false),
      mSignalledError(false),
      mStride(mWidth) {
    const size_t kMinCompressionRatio = 4 /* compressionRatio (for Level 4+) */;
    const size_t kMaxOutputBufferSize = 2048 * 2048 * 3 / 2;
    // INPUT_BUF_SIZE is given by HEVC codec as minimum input size
    initPorts(
            kNumBuffers, max(kMaxOutputBufferSize / kMinCompressionRatio, (size_t)INPUT_BUF_SIZE),
            kNumBuffers, CODEC_MIME_TYPE, kMinCompressionRatio);
}

status_t SoftHEVC::init() {
    return initDecoder();
}

SoftHEVC::~SoftHEVC() {
    ALOGV("In SoftHEVC::~SoftHEVC");
    CHECK_EQ(deInitDecoder(), (status_t)OK);
}

static void *ivd_aligned_malloc(void *ctxt, WORD32 alignment, WORD32 size) {
    UNUSED(ctxt);
    return memalign(alignment, size);
}

static void ivd_aligned_free(void *ctxt, void *buf) {
    UNUSED(ctxt);
    free(buf);
    return;
}

static size_t GetCPUCoreCount() {
    long cpuCoreCount = 1;
#if defined(_SC_NPROCESSORS_ONLN)
    cpuCoreCount = sysconf(_SC_NPROCESSORS_ONLN);
#else
    // _SC_NPROC_ONLN must be defined...
    cpuCoreCount = sysconf(_SC_NPROC_ONLN);
#endif
    CHECK(cpuCoreCount >= 1);
    ALOGV("Number of CPU cores: %ld", cpuCoreCount);
    return (size_t)cpuCoreCount;
}

void SoftHEVC::logVersion() {
    ivd_ctl_getversioninfo_ip_t s_ctl_ip;
    ivd_ctl_getversioninfo_op_t s_ctl_op;
    UWORD8 au1_buf[512];
    IV_API_CALL_STATUS_T status;

    s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_GETVERSION;
    s_ctl_ip.u4_size = sizeof(ivd_ctl_getversioninfo_ip_t);
    s_ctl_op.u4_size = sizeof(ivd_ctl_getversioninfo_op_t);
    s_ctl_ip.pv_version_buffer = au1_buf;
    s_ctl_ip.u4_version_buffer_size = sizeof(au1_buf);

    status = ivdec_api_function(mCodecCtx, (void *)&s_ctl_ip,
            (void *)&s_ctl_op);

    if (status != IV_SUCCESS) {
        ALOGE("Error in getting version number: 0x%x",
                s_ctl_op.u4_error_code);
    } else {
        ALOGV("Ittiam decoder version number: %s",
                (char *)s_ctl_ip.pv_version_buffer);
    }
    return;
}

status_t SoftHEVC::setParams(size_t stride) {
    ivd_ctl_set_config_ip_t s_ctl_ip;
    ivd_ctl_set_config_op_t s_ctl_op;
    IV_API_CALL_STATUS_T status;
    s_ctl_ip.u4_disp_wd = (UWORD32)stride;
    s_ctl_ip.e_frm_skip_mode = IVD_SKIP_NONE;

    s_ctl_ip.e_frm_out_mode = IVD_DISPLAY_FRAME_OUT;
    s_ctl_ip.e_vid_dec_mode = IVD_DECODE_FRAME;
    s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_SETPARAMS;
    s_ctl_ip.u4_size = sizeof(ivd_ctl_set_config_ip_t);
    s_ctl_op.u4_size = sizeof(ivd_ctl_set_config_op_t);

    ALOGV("Set the run-time (dynamic) parameters stride = %zu", stride);
    status = ivdec_api_function(mCodecCtx, (void *)&s_ctl_ip,
            (void *)&s_ctl_op);

    if (status != IV_SUCCESS) {
        ALOGE("Error in setting the run-time parameters: 0x%x",
                s_ctl_op.u4_error_code);

        return UNKNOWN_ERROR;
    }
    return OK;
}

status_t SoftHEVC::resetPlugin() {
    mIsInFlush = false;
    mReceivedEOS = false;
    memset(mTimeStamps, 0, sizeof(mTimeStamps));
    memset(mTimeStampsValid, 0, sizeof(mTimeStampsValid));

    /* Initialize both start and end times */
    gettimeofday(&mTimeStart, NULL);
    gettimeofday(&mTimeEnd, NULL);

    return OK;
}

bool SoftHEVC::getVUIParams() {
    IV_API_CALL_STATUS_T status;
    ihevcd_cxa_ctl_get_vui_params_ip_t s_ctl_get_vui_params_ip;
    ihevcd_cxa_ctl_get_vui_params_op_t s_ctl_get_vui_params_op;

    s_ctl_get_vui_params_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_get_vui_params_ip.e_sub_cmd =
        (IVD_CONTROL_API_COMMAND_TYPE_T)IHEVCD_CXA_CMD_CTL_GET_VUI_PARAMS;

    s_ctl_get_vui_params_ip.u4_size =
        sizeof(ihevcd_cxa_ctl_get_vui_params_ip_t);

    s_ctl_get_vui_params_op.u4_size = sizeof(ihevcd_cxa_ctl_get_vui_params_op_t);

    status = ivdec_api_function(
            (iv_obj_t *)mCodecCtx, (void *)&s_ctl_get_vui_params_ip,
            (void *)&s_ctl_get_vui_params_op);

    if (status != IV_SUCCESS) {
        ALOGW("Error in getting VUI params: 0x%x",
                s_ctl_get_vui_params_op.u4_error_code);
        return false;
    }

    int32_t primaries = s_ctl_get_vui_params_op.u1_colour_primaries;
    int32_t transfer = s_ctl_get_vui_params_op.u1_transfer_characteristics;
    int32_t coeffs = s_ctl_get_vui_params_op.u1_matrix_coefficients;
    bool fullRange = s_ctl_get_vui_params_op.u1_video_full_range_flag;

    ColorAspects colorAspects;
    ColorUtils::convertIsoColorAspectsToCodecAspects(
            primaries, transfer, coeffs, fullRange, colorAspects);

    // Update color aspects if necessary.
    if (colorAspectsDiffer(colorAspects, mBitstreamColorAspects)) {
        mBitstreamColorAspects = colorAspects;
        status_t err = handleColorAspectsChange();
        CHECK(err == OK);
    }
    return true;
}

status_t SoftHEVC::resetDecoder() {
    ivd_ctl_reset_ip_t s_ctl_ip;
    ivd_ctl_reset_op_t s_ctl_op;
    IV_API_CALL_STATUS_T status;

    s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_RESET;
    s_ctl_ip.u4_size = sizeof(ivd_ctl_reset_ip_t);
    s_ctl_op.u4_size = sizeof(ivd_ctl_reset_op_t);

    status = ivdec_api_function(mCodecCtx, (void *)&s_ctl_ip,
            (void *)&s_ctl_op);
    if (IV_SUCCESS != status) {
        ALOGE("Error in reset: 0x%x", s_ctl_op.u4_error_code);
        return UNKNOWN_ERROR;
    }
    mSignalledError = false;

    /* Set number of cores/threads to be used by the codec */
    setNumCores();

    mStride = 0;
    return OK;
}

status_t SoftHEVC::setNumCores() {
    ivdext_ctl_set_num_cores_ip_t s_set_cores_ip;
    ivdext_ctl_set_num_cores_op_t s_set_cores_op;
    IV_API_CALL_STATUS_T status;
    s_set_cores_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_set_cores_ip.e_sub_cmd = IVDEXT_CMD_CTL_SET_NUM_CORES;
    s_set_cores_ip.u4_num_cores = MIN(mNumCores, CODEC_MAX_NUM_CORES);
    s_set_cores_ip.u4_size = sizeof(ivdext_ctl_set_num_cores_ip_t);
    s_set_cores_op.u4_size = sizeof(ivdext_ctl_set_num_cores_op_t);
    ALOGV("Set number of cores to %u", s_set_cores_ip.u4_num_cores);
    status = ivdec_api_function(mCodecCtx, (void *)&s_set_cores_ip,
            (void *)&s_set_cores_op);
    if (IV_SUCCESS != status) {
        ALOGE("Error in setting number of cores: 0x%x",
                s_set_cores_op.u4_error_code);
        return UNKNOWN_ERROR;
    }
    return OK;
}

status_t SoftHEVC::setFlushMode() {
    IV_API_CALL_STATUS_T status;
    ivd_ctl_flush_ip_t s_video_flush_ip;
    ivd_ctl_flush_op_t s_video_flush_op;

    s_video_flush_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_video_flush_ip.e_sub_cmd = IVD_CMD_CTL_FLUSH;
    s_video_flush_ip.u4_size = sizeof(ivd_ctl_flush_ip_t);
    s_video_flush_op.u4_size = sizeof(ivd_ctl_flush_op_t);
    ALOGV("Set the decoder in flush mode ");

    /* Set the decoder in Flush mode, subsequent decode() calls will flush */
    status = ivdec_api_function(mCodecCtx, (void *)&s_video_flush_ip,
            (void *)&s_video_flush_op);

    if (status != IV_SUCCESS) {
        ALOGE("Error in setting the decoder in flush mode: (%d) 0x%x", status,
                s_video_flush_op.u4_error_code);
        return UNKNOWN_ERROR;
    }

    mIsInFlush = true;
    return OK;
}

status_t SoftHEVC::initDecoder() {
    IV_API_CALL_STATUS_T status;

    mNumCores = GetCPUCoreCount();
    mCodecCtx = NULL;

    mStride = outputBufferWidth();

    /* Initialize the decoder */
    {
        ivdext_create_ip_t s_create_ip;
        ivdext_create_op_t s_create_op;

        void *dec_fxns = (void *)ivdec_api_function;

        s_create_ip.s_ivd_create_ip_t.u4_size = sizeof(ivdext_create_ip_t);
        s_create_ip.s_ivd_create_ip_t.e_cmd = IVD_CMD_CREATE;
        s_create_ip.s_ivd_create_ip_t.u4_share_disp_buf = 0;
        s_create_op.s_ivd_create_op_t.u4_size = sizeof(ivdext_create_op_t);
        s_create_ip.s_ivd_create_ip_t.e_output_format = mIvColorFormat;
        s_create_ip.s_ivd_create_ip_t.pf_aligned_alloc = ivd_aligned_malloc;
        s_create_ip.s_ivd_create_ip_t.pf_aligned_free = ivd_aligned_free;
        s_create_ip.s_ivd_create_ip_t.pv_mem_ctxt = NULL;

        status = ivdec_api_function(mCodecCtx, (void *)&s_create_ip, (void *)&s_create_op);

        if (status != IV_SUCCESS) {
            ALOGE("Error in create: 0x%x",
                    s_create_op.s_ivd_create_op_t.u4_error_code);
            deInitDecoder();
            mCodecCtx = NULL;
            return UNKNOWN_ERROR;
        }

        mCodecCtx = (iv_obj_t*)s_create_op.s_ivd_create_op_t.pv_handle;
        mCodecCtx->pv_fxns = dec_fxns;
        mCodecCtx->u4_size = sizeof(iv_obj_t);
    }

    /* Reset the plugin state */
    resetPlugin();

    /* Set the run time (dynamic) parameters */
    setParams(mStride);

    /* Set number of cores/threads to be used by the codec */
    setNumCores();

    /* Get codec version */
    logVersion();

    mFlushNeeded = false;
    return OK;
}

status_t SoftHEVC::deInitDecoder() {
    size_t i;
    IV_API_CALL_STATUS_T status;

    if (mCodecCtx) {
        ivdext_delete_ip_t s_delete_ip;
        ivdext_delete_op_t s_delete_op;

        s_delete_ip.s_ivd_delete_ip_t.u4_size = sizeof(ivdext_delete_ip_t);
        s_delete_ip.s_ivd_delete_ip_t.e_cmd = IVD_CMD_DELETE;

        s_delete_op.s_ivd_delete_op_t.u4_size = sizeof(ivdext_delete_op_t);

        status = ivdec_api_function(mCodecCtx, (void *)&s_delete_ip, (void *)&s_delete_op);
        if (status != IV_SUCCESS) {
            ALOGE("Error in delete: 0x%x",
                    s_delete_op.s_ivd_delete_op_t.u4_error_code);
            return UNKNOWN_ERROR;
        }
    }


    mChangingResolution = false;

    return OK;
}

void SoftHEVC::onReset() {
    ALOGV("onReset called");
    SoftVideoDecoderOMXComponent::onReset();

    mSignalledError = false;
    resetDecoder();
    resetPlugin();
}

bool SoftHEVC::setDecodeArgs(ivd_video_decode_ip_t *ps_dec_ip,
        ivd_video_decode_op_t *ps_dec_op,
        OMX_BUFFERHEADERTYPE *inHeader,
        OMX_BUFFERHEADERTYPE *outHeader,
        size_t timeStampIx) {
    size_t sizeY = outputBufferWidth() * outputBufferHeight();
    size_t sizeUV;

    ps_dec_ip->u4_size = sizeof(ivd_video_decode_ip_t);
    ps_dec_op->u4_size = sizeof(ivd_video_decode_op_t);

    ps_dec_ip->e_cmd = IVD_CMD_VIDEO_DECODE;

    /* When in flush and after EOS with zero byte input,
     * inHeader is set to zero. Hence check for non-null */
    if (inHeader) {
        ps_dec_ip->u4_ts = timeStampIx;
        ps_dec_ip->pv_stream_buffer = inHeader->pBuffer
                + inHeader->nOffset;
        ps_dec_ip->u4_num_Bytes = inHeader->nFilledLen;
    } else {
        ps_dec_ip->u4_ts = 0;
        ps_dec_ip->pv_stream_buffer = NULL;
        ps_dec_ip->u4_num_Bytes = 0;
    }

    sizeUV = sizeY / 4;
    ps_dec_ip->s_out_buffer.u4_min_out_buf_size[0] = sizeY;
    ps_dec_ip->s_out_buffer.u4_min_out_buf_size[1] = sizeUV;
    ps_dec_ip->s_out_buffer.u4_min_out_buf_size[2] = sizeUV;

    uint8_t *pBuf;
    if (outHeader) {
        if (outHeader->nAllocLen < sizeY + (sizeUV * 2)) {
            android_errorWriteLog(0x534e4554, "27833616");
            return false;
        }
        pBuf = outHeader->pBuffer;
    } else {
        // mFlushOutBuffer always has the right size.
        pBuf = mFlushOutBuffer;
    }

    ps_dec_ip->s_out_buffer.pu1_bufs[0] = pBuf;
    ps_dec_ip->s_out_buffer.pu1_bufs[1] = pBuf + sizeY;
    ps_dec_ip->s_out_buffer.pu1_bufs[2] = pBuf + sizeY + sizeUV;
    ps_dec_ip->s_out_buffer.u4_num_bufs = 3;
    return true;
}
void SoftHEVC::onPortFlushCompleted(OMX_U32 portIndex) {
    /* Once the output buffers are flushed, ignore any buffers that are held in decoder */
    if (kOutputPortIndex == portIndex) {
        setFlushMode();

        /* Allocate a picture buffer to flushed data */
        uint32_t displayStride = outputBufferWidth();
        uint32_t displayHeight = outputBufferHeight();

        uint32_t bufferSize = displayStride * displayHeight * 3 / 2;
        mFlushOutBuffer = (uint8_t *)memalign(128, bufferSize);
        if (NULL == mFlushOutBuffer) {
            ALOGE("Could not allocate flushOutputBuffer of size %u", bufferSize);
            return;
        }

        while (true) {
            ivd_video_decode_ip_t s_dec_ip;
            ivd_video_decode_op_t s_dec_op;
            IV_API_CALL_STATUS_T status;
            size_t sizeY, sizeUV;

            setDecodeArgs(&s_dec_ip, &s_dec_op, NULL, NULL, 0);

            status = ivdec_api_function(mCodecCtx, (void *)&s_dec_ip,
                    (void *)&s_dec_op);
            if (0 == s_dec_op.u4_output_present) {
                resetPlugin();
                break;
            }
        }

        if (mFlushOutBuffer) {
            free(mFlushOutBuffer);
            mFlushOutBuffer = NULL;
        }

    }
}

void SoftHEVC::onQueueFilled(OMX_U32 portIndex) {
    UNUSED(portIndex);

    if (mSignalledError) {
        return;
    }
    if (mOutputPortSettingsChange != NONE) {
        return;
    }

    if (NULL == mCodecCtx) {
        if (OK != initDecoder()) {
            ALOGE("Failed to initialize decoder");
            notify(OMX_EventError, OMX_ErrorUnsupportedSetting, 0, NULL);
            mSignalledError = true;
            return;
        }
    }
    if (outputBufferWidth() != mStride) {
        /* Set the run-time (dynamic) parameters */
        mStride = outputBufferWidth();
        setParams(mStride);
    }

    List<BufferInfo *> &inQueue = getPortQueue(kInputPortIndex);
    List<BufferInfo *> &outQueue = getPortQueue(kOutputPortIndex);

    while (!outQueue.empty()) {
        BufferInfo *inInfo;
        OMX_BUFFERHEADERTYPE *inHeader;

        BufferInfo *outInfo;
        OMX_BUFFERHEADERTYPE *outHeader;
        size_t timeStampIx;

        inInfo = NULL;
        inHeader = NULL;

        if (!mIsInFlush) {
            if (!inQueue.empty()) {
                inInfo = *inQueue.begin();
                inHeader = inInfo->mHeader;
            } else {
                break;
            }
        }

        outInfo = *outQueue.begin();
        outHeader = outInfo->mHeader;
        outHeader->nFlags = 0;
        outHeader->nTimeStamp = 0;
        outHeader->nOffset = 0;

        if (inHeader != NULL && (inHeader->nFlags & OMX_BUFFERFLAG_EOS)) {
            mReceivedEOS = true;
            if (inHeader->nFilledLen == 0) {
                inQueue.erase(inQueue.begin());
                inInfo->mOwnedByUs = false;
                notifyEmptyBufferDone(inHeader);
                inHeader = NULL;
                setFlushMode();
            }
        }

        /* Get a free slot in timestamp array to hold input timestamp */
        {
            size_t i;
            timeStampIx = 0;
            for (i = 0; i < MAX_TIME_STAMPS; i++) {
                if (!mTimeStampsValid[i]) {
                    timeStampIx = i;
                    break;
                }
            }
            if (inHeader != NULL) {
                mTimeStampsValid[timeStampIx] = true;
                mTimeStamps[timeStampIx] = inHeader->nTimeStamp;
            }
        }

        {
            ivd_video_decode_ip_t s_dec_ip;
            ivd_video_decode_op_t s_dec_op;
            WORD32 timeDelay, timeTaken;
            size_t sizeY, sizeUV;

            if (!setDecodeArgs(&s_dec_ip, &s_dec_op, inHeader, outHeader, timeStampIx)) {
                ALOGE("Decoder arg setup failed");
                notify(OMX_EventError, OMX_ErrorUndefined, 0, NULL);
                mSignalledError = true;
                return;
            }

            GETTIME(&mTimeStart, NULL);
            /* Compute time elapsed between end of previous decode()
             * to start of current decode() */
            TIME_DIFF(mTimeEnd, mTimeStart, timeDelay);

            IV_API_CALL_STATUS_T status;
            status = ivdec_api_function(mCodecCtx, (void *)&s_dec_ip, (void *)&s_dec_op);

            bool unsupportedResolution =
                (IVD_STREAM_WIDTH_HEIGHT_NOT_SUPPORTED == (s_dec_op.u4_error_code & IVD_ERROR_MASK));

            /* Check for unsupported dimensions */
            if (unsupportedResolution) {
                ALOGE("Unsupported resolution : %dx%d", mWidth, mHeight);
                notify(OMX_EventError, OMX_ErrorUnsupportedSetting, 0, NULL);
                mSignalledError = true;
                return;
            }

            bool allocationFailed = 
                (IVD_MEM_ALLOC_FAILED == (s_dec_op.u4_error_code & IVD_ERROR_MASK));
            if (allocationFailed) {
                ALOGE("Allocation failure in decoder");
                notify(OMX_EventError, OMX_ErrorUnsupportedSetting, 0, NULL);
                mSignalledError = true;
                return;
            }

            if (IS_IVD_FATAL_ERROR(s_dec_op.u4_error_code)) {
                ALOGE("Fatal Error : 0x%x", s_dec_op.u4_error_code);
                notify(OMX_EventError, OMX_ErrorUnsupportedSetting, 0, NULL);
                mSignalledError = true;
                return;
            }

            bool resChanged = (IVD_RES_CHANGED == (s_dec_op.u4_error_code & IVD_ERROR_MASK));

            getVUIParams();

            GETTIME(&mTimeEnd, NULL);
            /* Compute time taken for decode() */
            TIME_DIFF(mTimeStart, mTimeEnd, timeTaken);

            ALOGV("timeTaken=%6d delay=%6d numBytes=%6d", timeTaken, timeDelay,
                   s_dec_op.u4_num_bytes_consumed);
            if (s_dec_op.u4_frame_decoded_flag && !mFlushNeeded) {
                mFlushNeeded = true;
            }

            if ((inHeader != NULL) && (1 != s_dec_op.u4_frame_decoded_flag)) {
                /* If the input did not contain picture data, then ignore
                 * the associated timestamp */
                mTimeStampsValid[timeStampIx] = false;
            }

            // If the decoder is in the changing resolution mode and there is no output present,
            // that means the switching is done and it's ready to reset the decoder and the plugin.
            if (mChangingResolution && !s_dec_op.u4_output_present) {
                mChangingResolution = false;
                resetDecoder();
                resetPlugin();
                mStride = outputBufferWidth();
                setParams(mStride);
                continue;
            }

            if (resChanged) {
                mChangingResolution = true;
                if (mFlushNeeded) {
                    setFlushMode();
                }
                continue;
            }

            // Combine the resolution change and coloraspects change in one PortSettingChange event
            // if necessary.
            if ((0 < s_dec_op.u4_pic_wd) && (0 < s_dec_op.u4_pic_ht)) {
                uint32_t width = s_dec_op.u4_pic_wd;
                uint32_t height = s_dec_op.u4_pic_ht;
                bool portWillReset = false;
                handlePortSettingsChange(&portWillReset, width, height);

                if (portWillReset) {
                    resetDecoder();
                    resetPlugin();
                    return;
                }
            } else if (mUpdateColorAspects) {
                notify(OMX_EventPortSettingsChanged, kOutputPortIndex,
                    kDescribeColorAspectsIndex, NULL);
                mUpdateColorAspects = false;
                return;
            }

            if (s_dec_op.u4_output_present) {
                outHeader->nFilledLen = (outputBufferWidth() * outputBufferHeight() * 3) / 2;

                outHeader->nTimeStamp = mTimeStamps[s_dec_op.u4_ts];
                mTimeStampsValid[s_dec_op.u4_ts] = false;

                outInfo->mOwnedByUs = false;
                outQueue.erase(outQueue.begin());
                outInfo = NULL;
                notifyFillBufferDone(outHeader);
                outHeader = NULL;
            } else if (mIsInFlush) {
                /* If in flush mode and no output is returned by the codec,
                 * then come out of flush mode */
                mIsInFlush = false;

                /* If EOS was recieved on input port and there is no output
                 * from the codec, then signal EOS on output port */
                if (mReceivedEOS) {
                    outHeader->nFilledLen = 0;
                    outHeader->nFlags |= OMX_BUFFERFLAG_EOS;

                    outInfo->mOwnedByUs = false;
                    outQueue.erase(outQueue.begin());
                    outInfo = NULL;
                    notifyFillBufferDone(outHeader);
                    outHeader = NULL;
                    resetPlugin();
                }
            }
        }

        /* If input EOS is seen and decoder is not in flush mode,
         * set the decoder in flush mode.
         * There can be a case where EOS is sent along with last picture data
         * In that case, only after decoding that input data, decoder has to be
         * put in flush. This case is handled here  */

        if (mReceivedEOS && !mIsInFlush) {
            setFlushMode();
        }

        // TODO: Handle more than one picture data
        if (inHeader != NULL) {
            inInfo->mOwnedByUs = false;
            inQueue.erase(inQueue.begin());
            inInfo = NULL;
            notifyEmptyBufferDone(inHeader);
            inHeader = NULL;
        }
    }
}

int SoftHEVC::getColorAspectPreference() {
    return kPreferBitstream;
}

}  // namespace android

android::SoftOMXComponent *createSoftOMXComponent(const char *name,
        const OMX_CALLBACKTYPE *callbacks, OMX_PTR appData,
        OMX_COMPONENTTYPE **component) {
    android::SoftHEVC *codec = new android::SoftHEVC(name, callbacks, appData, component);
    if (codec->init() != android::OK) {
        android::sp<android::SoftOMXComponent> release = codec;
        return NULL;
    }
    return codec;
}
