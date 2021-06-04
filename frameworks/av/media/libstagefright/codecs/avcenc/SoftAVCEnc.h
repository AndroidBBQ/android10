/*
 * Copyright 2012 The Android Open Source Project
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

#ifndef __SOFT_AVC_ENC_H__
#define __SOFT_AVC_ENC_H__


#include <media/stagefright/foundation/ABase.h>
#include <utils/Vector.h>

#include <media/stagefright/omx/SoftVideoEncoderOMXComponent.h>

namespace android {

#define MAX_INPUT_BUFFER_HEADERS 4
#define MAX_CONVERSION_BUFFERS   4
#define CODEC_MAX_CORES          4
#define LEN_STATUS_BUFFER        (10  * 1024)
#define MAX_VBV_BUFF_SIZE        (120 * 16384)
#define MAX_NUM_IO_BUFS           3

#define DEFAULT_MAX_REF_FRM         2
#define DEFAULT_MAX_REORDER_FRM     0
#define DEFAULT_QP_MIN              10
#define DEFAULT_QP_MAX              40
#define DEFAULT_MAX_BITRATE         240000000
#define DEFAULT_MAX_SRCH_RANGE_X    256
#define DEFAULT_MAX_SRCH_RANGE_Y    256
#define DEFAULT_MAX_FRAMERATE       120000
#define DEFAULT_NUM_CORES           1
#define DEFAULT_NUM_CORES_PRE_ENC   0
#define DEFAULT_FPS                 30
#define DEFAULT_ENC_SPEED           IVE_NORMAL

#define DEFAULT_MEM_REC_CNT         0
#define DEFAULT_RECON_ENABLE        0
#define DEFAULT_CHKSUM_ENABLE       0
#define DEFAULT_START_FRM           0
#define DEFAULT_NUM_FRMS            0xFFFFFFFF
#define DEFAULT_INP_COLOR_FORMAT       IV_YUV_420SP_VU
#define DEFAULT_RECON_COLOR_FORMAT     IV_YUV_420P
#define DEFAULT_LOOPBACK            0
#define DEFAULT_SRC_FRAME_RATE      30
#define DEFAULT_TGT_FRAME_RATE      30
#define DEFAULT_MAX_WD              1920
#define DEFAULT_MAX_HT              1920
#define DEFAULT_MAX_LEVEL           41
#define DEFAULT_STRIDE              0
#define DEFAULT_WD                  1280
#define DEFAULT_HT                  720
#define DEFAULT_PSNR_ENABLE         0
#define DEFAULT_ME_SPEED            100
#define DEFAULT_ENABLE_FAST_SAD     0
#define DEFAULT_ENABLE_ALT_REF      0
#define DEFAULT_RC_MODE             IVE_RC_STORAGE
#define DEFAULT_BITRATE             6000000
#define DEFAULT_I_QP                22
#define DEFAULT_I_QP_MAX            DEFAULT_QP_MAX
#define DEFAULT_I_QP_MIN            DEFAULT_QP_MIN
#define DEFAULT_P_QP                28
#define DEFAULT_P_QP_MAX            DEFAULT_QP_MAX
#define DEFAULT_P_QP_MIN            DEFAULT_QP_MIN
#define DEFAULT_B_QP                22
#define DEFAULT_B_QP_MAX            DEFAULT_QP_MAX
#define DEFAULT_B_QP_MIN            DEFAULT_QP_MIN
#define DEFAULT_AIR                 IVE_AIR_MODE_NONE
#define DEFAULT_AIR_REFRESH_PERIOD  30
#define DEFAULT_SRCH_RNG_X          64
#define DEFAULT_SRCH_RNG_Y          48
#define DEFAULT_I_INTERVAL          30
#define DEFAULT_IDR_INTERVAL        1000
#define DEFAULT_B_FRAMES            0
#define DEFAULT_DISABLE_DEBLK_LEVEL 0
#define DEFAULT_HPEL                1
#define DEFAULT_QPEL                1
#define DEFAULT_I4                  1
#define DEFAULT_EPROFILE            IV_PROFILE_BASE
#define DEFAULT_ENTROPY_MODE        0
#define DEFAULT_SLICE_MODE          IVE_SLICE_MODE_NONE
#define DEFAULT_SLICE_PARAM         256
#define DEFAULT_ARCH                ARCH_ARM_A9Q
#define DEFAULT_SOC                 SOC_GENERIC
#define DEFAULT_INTRA4x4            0
#define STRLENGTH                   500
#define DEFAULT_CONSTRAINED_INTRA   0

#define MIN(a, b) ((a) < (b))? (a) : (b)
#define MAX(a, b) ((a) > (b))? (a) : (b)
#define ALIGN16(x) ((((x) + 15) >> 4) << 4)
#define ALIGN128(x) ((((x) + 127) >> 7) << 7)
#define ALIGN4096(x) ((((x) + 4095) >> 12) << 12)

/** Used to remove warnings about unused parameters */
#define UNUSED(x) ((void)(x))

/** Get time */
#define GETTIME(a, b) gettimeofday(a, b);

/** Compute difference between start and end */
#define TIME_DIFF(start, end, diff) \
    diff = (((end).tv_sec - (start).tv_sec) * 1000000) + \
            ((end).tv_usec - (start).tv_usec);

#define ive_aligned_malloc(alignment, size) memalign(alignment, size)
#define ive_aligned_free(buf) free(buf)

struct SoftAVC : public SoftVideoEncoderOMXComponent {
    SoftAVC(
            const char *name,
            const OMX_CALLBACKTYPE *callbacks,
            OMX_PTR appData,
            OMX_COMPONENTTYPE **component);

    // Override SimpleSoftOMXComponent methods
    virtual OMX_ERRORTYPE internalGetParameter(
            OMX_INDEXTYPE index, OMX_PTR params);

    virtual OMX_ERRORTYPE internalSetParameter(
            OMX_INDEXTYPE index, const OMX_PTR params);

    virtual void onQueueFilled(OMX_U32 portIndex);

protected:
    virtual ~SoftAVC();

    virtual void onReset();

private:
    enum {
        kNumBuffers = 2,
    };

    enum {
        kUpdateBitrate            = 1 << 0,
        kRequestKeyFrame          = 1 << 1,
        kUpdateAIRMode            = 1 << 2,
    };

    // OMX input buffer's timestamp and flags
    typedef struct {
        int64_t mTimeUs;
        int32_t mFlags;
    } InputBufferInfo;

    int32_t  mStride;

    struct timeval mTimeStart;   // Time at the start of decode()
    struct timeval mTimeEnd;     // Time at the end of decode()

    int mUpdateFlag;

#ifdef FILE_DUMP_ENABLE
    char mInFile[200];
    char mOutFile[200];
#endif /* FILE_DUMP_ENABLE */

    IV_COLOR_FORMAT_T mIvVideoColorFormat;

    IV_PROFILE_T mAVCEncProfile;
    WORD32   mAVCEncLevel;
    bool     mStarted;
    bool     mSpsPpsHeaderReceived;

    bool     mSawInputEOS;
    bool     mSawOutputEOS;
    bool     mSignalledError;
    bool     mIntra4x4;
    bool     mEnableFastSad;
    bool     mEnableAltRef;
    bool     mReconEnable;
    bool     mPSNREnable;
    bool     mEntropyMode;
    bool     mConstrainedIntraFlag;
    IVE_SPEED_CONFIG     mEncSpeed;

    uint8_t *mConversionBuffers[MAX_CONVERSION_BUFFERS];
    bool     mConversionBuffersFree[MAX_CONVERSION_BUFFERS];
    BufferInfo *mInputBufferInfo[MAX_INPUT_BUFFER_HEADERS];
    iv_obj_t *mCodecCtx;         // Codec context
    iv_mem_rec_t *mMemRecords;   // Memory records requested by the codec
    size_t mNumMemRecords;       // Number of memory records requested by codec
    size_t mNumCores;            // Number of cores used by the codec

    UWORD32 mHeaderGenerated;
    UWORD32 mBframes;
    IV_ARCH_T mArch;
    IVE_SLICE_MODE_T mSliceMode;
    UWORD32 mSliceParam;
    bool mHalfPelEnable;
    UWORD32 mIInterval;
    UWORD32 mIDRInterval;
    UWORD32 mDisableDeblkLevel;
    IVE_AIR_MODE_T mAIRMode;
    UWORD32 mAIRRefreshPeriod;

    void initEncParams();
    OMX_ERRORTYPE initEncoder();
    OMX_ERRORTYPE releaseEncoder();

    // Verifies the component role tried to be set to this OMX component is
    // strictly video_encoder.avc
    OMX_ERRORTYPE internalSetRoleParams(
        const OMX_PARAM_COMPONENTROLETYPE *role);

    // Updates bitrate to reflect port settings.
    OMX_ERRORTYPE internalSetBitrateParams(
        const OMX_VIDEO_PARAM_BITRATETYPE *bitrate);

    OMX_ERRORTYPE internalSetConfig(
        OMX_INDEXTYPE index, const OMX_PTR _params, bool *frameConfig);

    OMX_ERRORTYPE getConfig(
        OMX_INDEXTYPE index, const OMX_PTR _params);

    // Handles port definition changes.
    OMX_ERRORTYPE internalSetPortParams(
        const OMX_PARAM_PORTDEFINITIONTYPE *port);

    OMX_ERRORTYPE internalSetFormatParams(
        const OMX_VIDEO_PARAM_PORTFORMATTYPE *format);

    OMX_ERRORTYPE setFrameType(IV_PICTURE_CODING_TYPE_T  e_frame_type);
    OMX_ERRORTYPE setQp();
    OMX_ERRORTYPE setEncMode(IVE_ENC_MODE_T e_enc_mode);
    OMX_ERRORTYPE setDimensions();
    OMX_ERRORTYPE setNumCores();
    OMX_ERRORTYPE setFrameRate();
    OMX_ERRORTYPE setIpeParams();
    OMX_ERRORTYPE setBitRate();
    OMX_ERRORTYPE setAirParams();
    OMX_ERRORTYPE setMeParams();
    OMX_ERRORTYPE setGopParams();
    OMX_ERRORTYPE setProfileParams();
    OMX_ERRORTYPE setDeblockParams();
    OMX_ERRORTYPE setVbvParams();
    void logVersion();
    OMX_ERRORTYPE setEncodeArgs(
        ive_video_encode_ip_t *ps_encode_ip,
        ive_video_encode_op_t *ps_encode_op,
        OMX_BUFFERHEADERTYPE *inputBufferHeader,
        OMX_BUFFERHEADERTYPE *outputBufferHeader);

    DISALLOW_EVIL_CONSTRUCTORS(SoftAVC);
};

#ifdef FILE_DUMP_ENABLE

#define INPUT_DUMP_PATH     "/sdcard/media/avce_input"
#define INPUT_DUMP_EXT      "yuv"
#define OUTPUT_DUMP_PATH    "/sdcard/media/avce_output"
#define OUTPUT_DUMP_EXT     "h264"

#define GENERATE_FILE_NAMES() {                         \
    GETTIME(&mTimeStart, NULL);                         \
    strcpy(mInFile, "");                                \
    sprintf(mInFile, "%s_%ld.%ld.%s", INPUT_DUMP_PATH,  \
            mTimeStart.tv_sec, mTimeStart.tv_usec,      \
            INPUT_DUMP_EXT);                            \
    strcpy(mOutFile, "");                               \
    sprintf(mOutFile, "%s_%ld.%ld.%s", OUTPUT_DUMP_PATH,\
            mTimeStart.tv_sec, mTimeStart.tv_usec,      \
            OUTPUT_DUMP_EXT);                           \
}

#define CREATE_DUMP_FILE(m_filename) {                  \
    FILE *fp = fopen(m_filename, "wb");                 \
    if (fp != NULL) {                                   \
        ALOGD("Opened file %s", m_filename);            \
        fclose(fp);                                     \
    } else {                                            \
        ALOGD("Could not open file %s", m_filename);    \
    }                                                   \
}
#define DUMP_TO_FILE(m_filename, m_buf, m_size)         \
{                                                       \
    FILE *fp = fopen(m_filename, "ab");                 \
    if (fp != NULL && m_buf != NULL) {                  \
        int i;                                          \
        i = fwrite(m_buf, 1, m_size, fp);               \
        ALOGD("fwrite ret %d to write %d", i, m_size);  \
        if (i != (int)m_size) {                         \
            ALOGD("Error in fwrite, returned %d", i);   \
            perror("Error in write to file");           \
        }                                               \
        fclose(fp);                                     \
    } else {                                            \
        ALOGD("Could not write to file %s", m_filename);\
        if (fp != NULL)                                 \
            fclose(fp);                                 \
    }                                                   \
}
#else /* FILE_DUMP_ENABLE */
#define INPUT_DUMP_PATH
#define INPUT_DUMP_EXT
#define OUTPUT_DUMP_PATH
#define OUTPUT_DUMP_EXT
#define GENERATE_FILE_NAMES()
#define CREATE_DUMP_FILE(m_filename)
#define DUMP_TO_FILE(m_filename, m_buf, m_size)
#endif /* FILE_DUMP_ENABLE */

}  // namespace android

#endif  // __SOFT_AVC_ENC_H__
