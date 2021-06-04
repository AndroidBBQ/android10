/*
 * Copyright 2015 The Android Open Source Project
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
#define LOG_TAG "SoftMPEG2"
#include <utils/Log.h>

#include "iv_datatypedef.h"
#include "iv.h"
#include "ivd.h"
#include "impeg2d.h"
#include "SoftMPEG2.h"

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaDefs.h>
#include <OMX_VideoExt.h>

namespace android {

#define componentName                   "video_decoder.mpeg2"
#define codingType                      OMX_VIDEO_CodingMPEG2
#define CODEC_MIME_TYPE                 MEDIA_MIMETYPE_VIDEO_MPEG2

/** Function and structure definitions to keep code similar for each codec */
#define ivdec_api_function              impeg2d_api_function
#define ivdext_init_ip_t                impeg2d_init_ip_t
#define ivdext_init_op_t                impeg2d_init_op_t
#define ivdext_fill_mem_rec_ip_t        impeg2d_fill_mem_rec_ip_t
#define ivdext_fill_mem_rec_op_t        impeg2d_fill_mem_rec_op_t
#define ivdext_ctl_set_num_cores_ip_t   impeg2d_ctl_set_num_cores_ip_t
#define ivdext_ctl_set_num_cores_op_t   impeg2d_ctl_set_num_cores_op_t

#define IVDEXT_CMD_CTL_SET_NUM_CORES    \
        (IVD_CONTROL_API_COMMAND_TYPE_T)IMPEG2D_CMD_CTL_SET_NUM_CORES

static const CodecProfileLevel kProfileLevels[] = {
    { OMX_VIDEO_MPEG2ProfileSimple, OMX_VIDEO_MPEG2LevelHL  },

    { OMX_VIDEO_MPEG2ProfileMain  , OMX_VIDEO_MPEG2LevelHL  },
};

SoftMPEG2::SoftMPEG2(
        const char *name,
        const OMX_CALLBACKTYPE *callbacks,
        OMX_PTR appData,
        OMX_COMPONENTTYPE **component)
    : SoftVideoDecoderOMXComponent(
            name, componentName, codingType,
            kProfileLevels, ARRAY_SIZE(kProfileLevels),
            320 /* width */, 240 /* height */, callbacks,
            appData, component),
      mCodecCtx(NULL),
      mMemRecords(NULL),
      mFlushOutBuffer(NULL),
      mOmxColorFormat(OMX_COLOR_FormatYUV420Planar),
      mIvColorFormat(IV_YUV_420P),
      mNewWidth(mWidth),
      mNewHeight(mHeight),
      mChangingResolution(false),
      mSignalledError(false),
      mStride(mWidth) {
    initPorts(kNumBuffers, INPUT_BUF_SIZE, kNumBuffers, CODEC_MIME_TYPE);

    // If input dump is enabled, then open create an empty file
    GENERATE_FILE_NAMES();
    CREATE_DUMP_FILE(mInFile);
}

SoftMPEG2::~SoftMPEG2() {
    if (OK != deInitDecoder()) {
        ALOGE("Failed to deinit decoder");
        notify(OMX_EventError, OMX_ErrorUnsupportedSetting, 0, NULL);
        mSignalledError = true;
        return;
    }
}


static ssize_t getMinTimestampIdx(OMX_S64 *pNTimeStamp, bool *pIsTimeStampValid) {
    OMX_S64 minTimeStamp = LLONG_MAX;
    ssize_t idx = -1;
    for (ssize_t i = 0; i < MAX_TIME_STAMPS; i++) {
        if (pIsTimeStampValid[i]) {
            if (pNTimeStamp[i] < minTimeStamp) {
                minTimeStamp = pNTimeStamp[i];
                idx = i;
            }
        }
    }
    return idx;
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

void SoftMPEG2::logVersion() {
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

    status = ivdec_api_function(mCodecCtx, (void *)&s_ctl_ip, (void *)&s_ctl_op);

    if (status != IV_SUCCESS) {
        ALOGE("Error in getting version number: 0x%x",
                s_ctl_op.u4_error_code);
    } else {
        ALOGV("Ittiam decoder version number: %s",
                (char *)s_ctl_ip.pv_version_buffer);
    }
    return;
}

status_t SoftMPEG2::setParams(size_t stride) {
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
    status = ivdec_api_function(mCodecCtx, (void *)&s_ctl_ip, (void *)&s_ctl_op);

    if (status != IV_SUCCESS) {
        ALOGE("Error in setting the run-time parameters: 0x%x",
                s_ctl_op.u4_error_code);

        return UNKNOWN_ERROR;
    }
    return OK;
}

status_t SoftMPEG2::resetPlugin() {
    mIsInFlush = false;
    mReceivedEOS = false;
    memset(mTimeStamps, 0, sizeof(mTimeStamps));
    memset(mTimeStampsValid, 0, sizeof(mTimeStampsValid));

    /* Initialize both start and end times */
    gettimeofday(&mTimeStart, NULL);
    gettimeofday(&mTimeEnd, NULL);

    return OK;
}

status_t SoftMPEG2::resetDecoder() {
    ivd_ctl_reset_ip_t s_ctl_ip;
    ivd_ctl_reset_op_t s_ctl_op;
    IV_API_CALL_STATUS_T status;

    s_ctl_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_ip.e_sub_cmd = IVD_CMD_CTL_RESET;
    s_ctl_ip.u4_size = sizeof(ivd_ctl_reset_ip_t);
    s_ctl_op.u4_size = sizeof(ivd_ctl_reset_op_t);

    status = ivdec_api_function(mCodecCtx, (void *)&s_ctl_ip, (void *)&s_ctl_op);
    if (IV_SUCCESS != status) {
        ALOGE("Error in reset: 0x%x", s_ctl_op.u4_error_code);
        return UNKNOWN_ERROR;
    }

    /* Set the run-time (dynamic) parameters */
    setParams(outputBufferWidth());

    /* Set number of cores/threads to be used by the codec */
    setNumCores();

    mStride = 0;
    mSignalledError = false;

    return OK;
}

status_t SoftMPEG2::setNumCores() {
    ivdext_ctl_set_num_cores_ip_t s_set_cores_ip;
    ivdext_ctl_set_num_cores_op_t s_set_cores_op;
    IV_API_CALL_STATUS_T status;
    s_set_cores_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_set_cores_ip.e_sub_cmd = IVDEXT_CMD_CTL_SET_NUM_CORES;
    s_set_cores_ip.u4_num_cores = MIN(mNumCores, CODEC_MAX_NUM_CORES);
    s_set_cores_ip.u4_size = sizeof(ivdext_ctl_set_num_cores_ip_t);
    s_set_cores_op.u4_size = sizeof(ivdext_ctl_set_num_cores_op_t);

    status = ivdec_api_function(mCodecCtx, (void *)&s_set_cores_ip, (void *)&s_set_cores_op);
    if (IV_SUCCESS != status) {
        ALOGE("Error in setting number of cores: 0x%x",
                s_set_cores_op.u4_error_code);
        return UNKNOWN_ERROR;
    }
    return OK;
}

status_t SoftMPEG2::setFlushMode() {
    IV_API_CALL_STATUS_T status;
    ivd_ctl_flush_ip_t s_video_flush_ip;
    ivd_ctl_flush_op_t s_video_flush_op;

    s_video_flush_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_video_flush_ip.e_sub_cmd = IVD_CMD_CTL_FLUSH;
    s_video_flush_ip.u4_size = sizeof(ivd_ctl_flush_ip_t);
    s_video_flush_op.u4_size = sizeof(ivd_ctl_flush_op_t);

    /* Set the decoder in Flush mode, subsequent decode() calls will flush */
    status = ivdec_api_function(
            mCodecCtx, (void *)&s_video_flush_ip, (void *)&s_video_flush_op);

    if (status != IV_SUCCESS) {
        ALOGE("Error in setting the decoder in flush mode: (%d) 0x%x", status,
                s_video_flush_op.u4_error_code);
        return UNKNOWN_ERROR;
    }

    mWaitForI = true;
    mIsInFlush = true;
    return OK;
}

status_t SoftMPEG2::initDecoder() {
    IV_API_CALL_STATUS_T status;

    UWORD32 u4_num_reorder_frames;
    UWORD32 u4_num_ref_frames;
    UWORD32 u4_share_disp_buf;

    mNumCores = GetCPUCoreCount();
    mWaitForI = true;

    /* Initialize number of ref and reorder modes (for MPEG2) */
    u4_num_reorder_frames = 16;
    u4_num_ref_frames = 16;
    u4_share_disp_buf = 0;

    uint32_t displayStride = outputBufferWidth();
    uint32_t displayHeight = outputBufferHeight();
    uint32_t displaySizeY = displayStride * displayHeight;

    {
        iv_num_mem_rec_ip_t s_num_mem_rec_ip;
        iv_num_mem_rec_op_t s_num_mem_rec_op;

        s_num_mem_rec_ip.u4_size = sizeof(s_num_mem_rec_ip);
        s_num_mem_rec_op.u4_size = sizeof(s_num_mem_rec_op);
        s_num_mem_rec_ip.e_cmd = IV_CMD_GET_NUM_MEM_REC;

        status = ivdec_api_function(
                mCodecCtx, (void *)&s_num_mem_rec_ip, (void *)&s_num_mem_rec_op);
        if (IV_SUCCESS != status) {
            ALOGE("Error in getting mem records: 0x%x",
                    s_num_mem_rec_op.u4_error_code);
            return UNKNOWN_ERROR;
        }

        mNumMemRecords = s_num_mem_rec_op.u4_num_mem_rec;
    }

    mMemRecords = (iv_mem_rec_t *)ivd_aligned_malloc(
            128, mNumMemRecords * sizeof(iv_mem_rec_t));
    if (mMemRecords == NULL) {
        ALOGE("Allocation failure");
        return NO_MEMORY;
    }

    memset(mMemRecords, 0, mNumMemRecords * sizeof(iv_mem_rec_t));

    {
        size_t i;
        ivdext_fill_mem_rec_ip_t s_fill_mem_ip;
        ivdext_fill_mem_rec_op_t s_fill_mem_op;
        iv_mem_rec_t *ps_mem_rec;

        s_fill_mem_ip.s_ivd_fill_mem_rec_ip_t.u4_size =
            sizeof(ivdext_fill_mem_rec_ip_t);

        s_fill_mem_ip.u4_share_disp_buf = u4_share_disp_buf;
        s_fill_mem_ip.e_output_format = mIvColorFormat;
        s_fill_mem_ip.u4_deinterlace = 1;
        s_fill_mem_ip.s_ivd_fill_mem_rec_ip_t.e_cmd = IV_CMD_FILL_NUM_MEM_REC;
        s_fill_mem_ip.s_ivd_fill_mem_rec_ip_t.pv_mem_rec_location = mMemRecords;
        s_fill_mem_ip.s_ivd_fill_mem_rec_ip_t.u4_max_frm_wd = displayStride;
        s_fill_mem_ip.s_ivd_fill_mem_rec_ip_t.u4_max_frm_ht = displayHeight;
        s_fill_mem_op.s_ivd_fill_mem_rec_op_t.u4_size =
            sizeof(ivdext_fill_mem_rec_op_t);

        ps_mem_rec = mMemRecords;
        for (i = 0; i < mNumMemRecords; i++) {
            ps_mem_rec[i].u4_size = sizeof(iv_mem_rec_t);
        }

        status = ivdec_api_function(
                mCodecCtx, (void *)&s_fill_mem_ip, (void *)&s_fill_mem_op);

        if (IV_SUCCESS != status) {
            ALOGE("Error in filling mem records: 0x%x",
                    s_fill_mem_op.s_ivd_fill_mem_rec_op_t.u4_error_code);
            return UNKNOWN_ERROR;
        }
        mNumMemRecords =
            s_fill_mem_op.s_ivd_fill_mem_rec_op_t.u4_num_mem_rec_filled;

        ps_mem_rec = mMemRecords;

        for (i = 0; i < mNumMemRecords; i++) {
            ps_mem_rec->pv_base = ivd_aligned_malloc(
                    ps_mem_rec->u4_mem_alignment, ps_mem_rec->u4_mem_size);
            if (ps_mem_rec->pv_base == NULL) {
                ALOGE("Allocation failure for memory record #%zu of size %u",
                        i, ps_mem_rec->u4_mem_size);
                status = IV_FAIL;
                return NO_MEMORY;
            }

            ps_mem_rec++;
        }
    }

    /* Initialize the decoder */
    {
        ivdext_init_ip_t s_init_ip;
        ivdext_init_op_t s_init_op;

        void *dec_fxns = (void *)ivdec_api_function;

        s_init_ip.s_ivd_init_ip_t.u4_size = sizeof(ivdext_init_ip_t);
        s_init_ip.s_ivd_init_ip_t.e_cmd = (IVD_API_COMMAND_TYPE_T)IV_CMD_INIT;
        s_init_ip.s_ivd_init_ip_t.pv_mem_rec_location = mMemRecords;
        s_init_ip.s_ivd_init_ip_t.u4_frm_max_wd = displayStride;
        s_init_ip.s_ivd_init_ip_t.u4_frm_max_ht = displayHeight;

        s_init_ip.u4_share_disp_buf = u4_share_disp_buf;
        s_init_ip.u4_deinterlace = 1;

        s_init_op.s_ivd_init_op_t.u4_size = sizeof(s_init_op);

        s_init_ip.s_ivd_init_ip_t.u4_num_mem_rec = mNumMemRecords;
        s_init_ip.s_ivd_init_ip_t.e_output_format = mIvColorFormat;

        mCodecCtx = (iv_obj_t *)mMemRecords[0].pv_base;
        mCodecCtx->pv_fxns = dec_fxns;
        mCodecCtx->u4_size = sizeof(iv_obj_t);

        status = ivdec_api_function(mCodecCtx, (void *)&s_init_ip, (void *)&s_init_op);
        if (status != IV_SUCCESS) {
            ALOGE("Error in init: 0x%x",
                    s_init_op.s_ivd_init_op_t.u4_error_code);
            return UNKNOWN_ERROR;
        }
    }

    /* Reset the plugin state */
    resetPlugin();

    /* Set the run time (dynamic) parameters */
    mStride = outputBufferWidth();
    setParams(mStride);

    /* Set number of cores/threads to be used by the codec */
    setNumCores();

    /* Get codec version */
    logVersion();

    /* Allocate internal picture buffer */
    uint32_t bufferSize = displaySizeY * 3 / 2;
    mFlushOutBuffer = (uint8_t *)ivd_aligned_malloc(128, bufferSize);
    if (NULL == mFlushOutBuffer) {
        ALOGE("Could not allocate flushOutputBuffer of size %u", bufferSize);
        return NO_MEMORY;
    }

    mInitNeeded = false;
    mFlushNeeded = false;
    return OK;
}

status_t SoftMPEG2::deInitDecoder() {
    size_t i;

    if (mMemRecords) {
        iv_mem_rec_t *ps_mem_rec;

        ps_mem_rec = mMemRecords;
        for (i = 0; i < mNumMemRecords; i++) {
            if (ps_mem_rec->pv_base) {
                ivd_aligned_free(ps_mem_rec->pv_base);
            }
            ps_mem_rec++;
        }
        ivd_aligned_free(mMemRecords);
        mMemRecords = NULL;
    }

    if (mFlushOutBuffer) {
        ivd_aligned_free(mFlushOutBuffer);
        mFlushOutBuffer = NULL;
    }

    mInitNeeded = true;
    mChangingResolution = false;
    mCodecCtx = NULL;

    return OK;
}

status_t SoftMPEG2::reInitDecoder() {
    status_t ret;

    deInitDecoder();

    ret = initDecoder();
    if (OK != ret) {
        ALOGE("Failed to initialize decoder");
        deInitDecoder();
        return ret;
    }
    mSignalledError = false;
    return OK;
}

void SoftMPEG2::onReset() {
    SoftVideoDecoderOMXComponent::onReset();

    mWaitForI = true;

    resetDecoder();
    resetPlugin();
}

bool SoftMPEG2::getSeqInfo() {
    IV_API_CALL_STATUS_T status;
    impeg2d_ctl_get_seq_info_ip_t s_ctl_get_seq_info_ip;
    impeg2d_ctl_get_seq_info_op_t s_ctl_get_seq_info_op;

    s_ctl_get_seq_info_ip.e_cmd = IVD_CMD_VIDEO_CTL;
    s_ctl_get_seq_info_ip.e_sub_cmd =
        (IVD_CONTROL_API_COMMAND_TYPE_T)IMPEG2D_CMD_CTL_GET_SEQ_INFO;

    s_ctl_get_seq_info_ip.u4_size = sizeof(impeg2d_ctl_get_seq_info_ip_t);
    s_ctl_get_seq_info_op.u4_size = sizeof(impeg2d_ctl_get_seq_info_op_t);

    status = ivdec_api_function(
            (iv_obj_t *)mCodecCtx, (void *)&s_ctl_get_seq_info_ip,
            (void *)&s_ctl_get_seq_info_op);

    if (status != IV_SUCCESS) {
        ALOGW("Error in getting Sequence info: 0x%x",
                s_ctl_get_seq_info_op.u4_error_code);
        return false;
    }


    int32_t primaries = s_ctl_get_seq_info_op.u1_colour_primaries;
    int32_t transfer = s_ctl_get_seq_info_op.u1_transfer_characteristics;
    int32_t coeffs = s_ctl_get_seq_info_op.u1_matrix_coefficients;
    bool fullRange = false;  // mpeg2 video has limited range.

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

OMX_ERRORTYPE SoftMPEG2::internalSetParameter(OMX_INDEXTYPE index, const OMX_PTR params) {
    const uint32_t oldWidth = mWidth;
    const uint32_t oldHeight = mHeight;
    OMX_ERRORTYPE ret = SoftVideoDecoderOMXComponent::internalSetParameter(index, params);
    if (mWidth != oldWidth || mHeight != oldHeight) {
        reInitDecoder();
    }
    return ret;
}

bool SoftMPEG2::setDecodeArgs(
        ivd_video_decode_ip_t *ps_dec_ip,
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
void SoftMPEG2::onPortFlushCompleted(OMX_U32 portIndex) {
    /* Once the output buffers are flushed, ignore any buffers that are held in decoder */
    if (kOutputPortIndex == portIndex) {
        setFlushMode();

        while (true) {
            ivd_video_decode_ip_t s_dec_ip;
            ivd_video_decode_op_t s_dec_op;
            IV_API_CALL_STATUS_T status;
            size_t sizeY, sizeUV;

            setDecodeArgs(&s_dec_ip, &s_dec_op, NULL, NULL, 0);

            status = ivdec_api_function(mCodecCtx, (void *)&s_dec_ip, (void *)&s_dec_op);
            if (0 == s_dec_op.u4_output_present) {
                resetPlugin();
                break;
            }
        }
    }
}

void SoftMPEG2::onQueueFilled(OMX_U32 portIndex) {
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

    List<BufferInfo *> &inQueue = getPortQueue(kInputPortIndex);
    List<BufferInfo *> &outQueue = getPortQueue(kOutputPortIndex);

    if (outputBufferWidth() != mStride) {
        /* Set the run-time (dynamic) parameters */
        mStride = outputBufferWidth();
        setParams(mStride);
    }

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

        // When there is an init required and the decoder is not in flush mode,
        // update output port's definition and reinitialize decoder.
        if (mInitNeeded && !mIsInFlush) {
            bool portWillReset = false;
            handlePortSettingsChange(&portWillReset, mNewWidth, mNewHeight);

            if (OK != reInitDecoder()) {
                ALOGE("Failed to reinitialize decoder");
            }
            return;
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
                return;
            }
            // If input dump is enabled, then write to file
            DUMP_TO_FILE(mInFile, s_dec_ip.pv_stream_buffer, s_dec_ip.u4_num_Bytes);

            if (s_dec_ip.u4_num_Bytes > 0) {
                char *ptr = (char *)s_dec_ip.pv_stream_buffer;
            }

            GETTIME(&mTimeStart, NULL);
            /* Compute time elapsed between end of previous decode()
             * to start of current decode() */
            TIME_DIFF(mTimeEnd, mTimeStart, timeDelay);

            IV_API_CALL_STATUS_T status;
            status = ivdec_api_function(mCodecCtx, (void *)&s_dec_ip, (void *)&s_dec_op);

            bool unsupportedDimensions = (IMPEG2D_UNSUPPORTED_DIMENSIONS == s_dec_op.u4_error_code);
            bool resChanged = (IVD_RES_CHANGED == (s_dec_op.u4_error_code & 0xFF));

            getSeqInfo();

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

            // This is needed to handle CTS DecoderTest testCodecResetsMPEG2WithoutSurface,
            // which is not sending SPS/PPS after port reconfiguration and flush to the codec.
            if (unsupportedDimensions && !mFlushNeeded) {
                bool portWillReset = false;
                handlePortSettingsChange(&portWillReset, s_dec_op.u4_pic_wd, s_dec_op.u4_pic_ht);

                if (OK != reInitDecoder()) {
                    ALOGE("Failed to reinitialize decoder");
                    return;
                }

                if (setDecodeArgs(&s_dec_ip, &s_dec_op, inHeader, outHeader, timeStampIx)) {
                    ivdec_api_function(mCodecCtx, (void *)&s_dec_ip, (void *)&s_dec_op);
                }
                return;
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

            if (unsupportedDimensions || resChanged) {
                mChangingResolution = true;
                if (mFlushNeeded) {
                    setFlushMode();
                }

                if (unsupportedDimensions) {
                    mNewWidth = s_dec_op.u4_pic_wd;
                    mNewHeight = s_dec_op.u4_pic_ht;
                    mInitNeeded = true;
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
                ssize_t timeStampIdx;
                outHeader->nFilledLen = (outputBufferWidth() * outputBufferHeight() * 3) / 2;

                timeStampIdx = getMinTimestampIdx(mTimeStamps, mTimeStampsValid);
                if (timeStampIdx < 0) {
                    ALOGE("b/62872863, Invalid timestamp index!");
                    android_errorWriteLog(0x534e4554, "62872863");
                    return;
                }
                outHeader->nTimeStamp = mTimeStamps[timeStampIdx];
                mTimeStampsValid[timeStampIdx] = false;

                /* mWaitForI waits for the first I picture. Once made FALSE, it
                   has to remain false till explicitly set to TRUE. */
                mWaitForI = mWaitForI && !(IV_I_FRAME == s_dec_op.e_pic_type);

                if (mWaitForI) {
                    s_dec_op.u4_output_present = false;
                } else {
                    ALOGV("Output timestamp: %lld, res: %ux%u",
                            (long long)outHeader->nTimeStamp, mWidth, mHeight);
                    DUMP_TO_FILE(mOutFile, outHeader->pBuffer, outHeader->nFilledLen);
                    outInfo->mOwnedByUs = false;
                    outQueue.erase(outQueue.begin());
                    outInfo = NULL;
                    notifyFillBufferDone(outHeader);
                    outHeader = NULL;
                }
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

int SoftMPEG2::getColorAspectPreference() {
    return kPreferBitstream;
}

}  // namespace android

android::SoftOMXComponent *createSoftOMXComponent(
        const char *name, const OMX_CALLBACKTYPE *callbacks, OMX_PTR appData,
        OMX_COMPONENTTYPE **component) {
    return new android::SoftMPEG2(name, callbacks, appData, component);
}
