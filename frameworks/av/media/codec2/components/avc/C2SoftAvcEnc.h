/*
 * Copyright 2018 The Android Open Source Project
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

#ifndef ANDROID_C2_SOFT_AVC_ENC_H__
#define ANDROID_C2_SOFT_AVC_ENC_H__

#include <map>

#include <utils/Vector.h>

#include <SimpleC2Component.h>

#include "ih264_typedefs.h"
#include "iv2.h"
#include "ive2.h"

namespace android {

#define CODEC_MAX_CORES          4
#define LEN_STATUS_BUFFER        (10  * 1024)
#define MAX_VBV_BUFF_SIZE        (120 * 16384)
#define MAX_NUM_IO_BUFS           3
#define MAX_B_FRAMES              1

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

struct C2SoftAvcEnc : public SimpleC2Component {
    class IntfImpl;

    C2SoftAvcEnc(const char *name, c2_node_id_t id, const std::shared_ptr<IntfImpl> &intfImpl);

    // From SimpleC2Component
    c2_status_t onInit() override;
    c2_status_t onStop() override;
    void onReset() override;
    void onRelease() override;
    c2_status_t onFlush_sm() override;
    void process(
            const std::unique_ptr<C2Work> &work,
            const std::shared_ptr<C2BlockPool> &pool) override;
    c2_status_t drain(
            uint32_t drainMode,
            const std::shared_ptr<C2BlockPool> &pool) override;

protected:
    virtual ~C2SoftAvcEnc();

private:
    // OMX input buffer's timestamp and flags
    typedef struct {
        int64_t mTimeUs;
        int32_t mFlags;
    } InputBufferInfo;

    std::shared_ptr<IntfImpl> mIntf;

    int32_t mStride;

    struct timeval mTimeStart;   // Time at the start of decode()
    struct timeval mTimeEnd;     // Time at the end of decode()

#ifdef FILE_DUMP_ENABLE
    char mInFile[200];
    char mOutFile[200];
#endif /* FILE_DUMP_ENABLE */

    IV_COLOR_FORMAT_T mIvVideoColorFormat;

    IV_PROFILE_T mAVCEncProfile __unused;
    WORD32   mAVCEncLevel;
    bool     mStarted;
    bool     mSpsPpsHeaderReceived;

    bool     mSawInputEOS;
    bool     mSignalledError;
    bool     mIntra4x4;
    bool     mEnableFastSad;
    bool     mEnableAltRef;
    bool     mReconEnable;
    bool     mPSNREnable;
    bool     mEntropyMode;
    bool     mConstrainedIntraFlag;
    IVE_SPEED_CONFIG     mEncSpeed;

    iv_obj_t *mCodecCtx;         // Codec context
    iv_mem_rec_t *mMemRecords;   // Memory records requested by the codec
    size_t mNumMemRecords;       // Number of memory records requested by codec
    size_t mNumCores;            // Number of cores used by the codec

    std::shared_ptr<C2LinearBlock> mOutBlock;

    // configurations used by component in process
    // (TODO: keep this in intf but make them internal only)
    std::shared_ptr<C2StreamPictureSizeInfo::input> mSize;
    std::shared_ptr<C2StreamIntraRefreshTuning::output> mIntraRefresh;
    std::shared_ptr<C2StreamFrameRateInfo::output> mFrameRate;
    std::shared_ptr<C2StreamBitrateInfo::output> mBitrate;
    std::shared_ptr<C2StreamRequestSyncFrameTuning::output> mRequestSync;

    uint32_t mOutBufferSize;
    UWORD32 mHeaderGenerated;
    UWORD32 mBframes;
    IV_ARCH_T mArch;
    IVE_SLICE_MODE_T mSliceMode;
    UWORD32 mSliceParam;
    bool mHalfPelEnable;
    UWORD32 mIInterval;
    UWORD32 mIDRInterval;
    UWORD32 mDisableDeblkLevel;
    std::map<const void *, std::shared_ptr<C2Buffer>> mBuffers;
    MemoryBlockPool mConversionBuffers;
    std::map<const void *, MemoryBlock> mConversionBuffersInUse;

    void initEncParams();
    c2_status_t initEncoder();
    c2_status_t releaseEncoder();

    c2_status_t setFrameType(IV_PICTURE_CODING_TYPE_T  e_frame_type);
    c2_status_t setQp();
    c2_status_t setEncMode(IVE_ENC_MODE_T e_enc_mode);
    c2_status_t setDimensions();
    c2_status_t setNumCores();
    c2_status_t setFrameRate();
    c2_status_t setIpeParams();
    c2_status_t setBitRate();
    c2_status_t setAirParams();
    c2_status_t setMeParams();
    c2_status_t setGopParams();
    c2_status_t setProfileParams();
    c2_status_t setDeblockParams();
    c2_status_t setVbvParams();
    void logVersion();
    c2_status_t setEncodeArgs(
            ive_video_encode_ip_t *ps_encode_ip,
            ive_video_encode_op_t *ps_encode_op,
            const C2GraphicView *const input,
            uint8_t *base,
            uint32_t capacity,
            uint64_t workIndex);
    void finishWork(uint64_t workIndex,
            const std::unique_ptr<C2Work> &work,
            ive_video_encode_op_t *ps_encode_op);
    c2_status_t drainInternal(uint32_t drainMode,
            const std::shared_ptr<C2BlockPool> &pool,
            const std::unique_ptr<C2Work> &work);

    C2_DO_NOT_COPY(C2SoftAvcEnc);
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

#endif  // ANDROID_C2_SOFT_AVC_ENC_H__
