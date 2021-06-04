/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef ANDROID_C2_SOFT_MPEG2_DEC_H_
#define ANDROID_C2_SOFT_MPEG2_DEC_H_

#include <atomic>
#include <SimpleC2Component.h>

#include <media/stagefright/foundation/ColorUtils.h>

#include "iv_datatypedef.h"
#include "iv.h"
#include "ivd.h"

namespace android {

#define ivdec_api_function              impeg2d_api_function
#define ivdext_init_ip_t                impeg2d_init_ip_t
#define ivdext_init_op_t                impeg2d_init_op_t
#define ivdext_fill_mem_rec_ip_t        impeg2d_fill_mem_rec_ip_t
#define ivdext_fill_mem_rec_op_t        impeg2d_fill_mem_rec_op_t
#define ivdext_ctl_set_num_cores_ip_t   impeg2d_ctl_set_num_cores_ip_t
#define ivdext_ctl_set_num_cores_op_t   impeg2d_ctl_set_num_cores_op_t
#define ivdext_ctl_get_seq_info_ip_t    impeg2d_ctl_get_seq_info_ip_t
#define ivdext_ctl_get_seq_info_op_t    impeg2d_ctl_get_seq_info_op_t
#define ALIGN64(x)                      ((((x) + 63) >> 6) << 6)
#define MAX_NUM_CORES                   4
#define IVDEXT_CMD_CTL_SET_NUM_CORES    \
        (IVD_CONTROL_API_COMMAND_TYPE_T)IMPEG2D_CMD_CTL_SET_NUM_CORES
#define MIN(a, b)                       (((a) < (b)) ? (a) : (b))
#define GETTIME(a, b)                   gettimeofday(a, b);
#define TIME_DIFF(start, end, diff)     \
    diff = (((end).tv_sec - (start).tv_sec) * 1000000) + \
            ((end).tv_usec - (start).tv_usec);

#ifdef FILE_DUMP_ENABLE
    #define INPUT_DUMP_PATH     "/sdcard/clips/mpeg2d_input"
    #define INPUT_DUMP_EXT      "m2v"
    #define GENERATE_FILE_NAMES() {                         \
        GETTIME(&mTimeStart, NULL);                         \
        strcpy(mInFile, "");                                \
        sprintf(mInFile, "%s_%ld.%ld.%s", INPUT_DUMP_PATH,  \
                mTimeStart.tv_sec, mTimeStart.tv_usec,      \
                INPUT_DUMP_EXT);                            \
    }
    #define CREATE_DUMP_FILE(m_filename) {                  \
        FILE *fp = fopen(m_filename, "wb");                 \
        if (fp != NULL) {                                   \
            fclose(fp);                                     \
        } else {                                            \
            ALOGD("Could not open file %s", m_filename);    \
        }                                                   \
    }
    #define DUMP_TO_FILE(m_filename, m_buf, m_size)         \
    {                                                       \
        FILE *fp = fopen(m_filename, "ab");                 \
        if (fp != NULL && m_buf != NULL) {                  \
            uint32_t i;                                     \
            i = fwrite(m_buf, 1, m_size, fp);               \
            ALOGD("fwrite ret %d to write %d", i, m_size);  \
            if (i != (uint32_t)m_size) {                    \
                ALOGD("Error in fwrite, returned %d", i);   \
                perror("Error in write to file");           \
            }                                               \
            fclose(fp);                                     \
        } else {                                            \
            ALOGD("Could not write to file %s", m_filename);\
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

struct C2SoftMpeg2Dec : public SimpleC2Component {
    class IntfImpl;

    C2SoftMpeg2Dec(const char* name, c2_node_id_t id,
                   const std::shared_ptr<IntfImpl>& intfImpl);
    virtual ~C2SoftMpeg2Dec();

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
 private:
    status_t getNumMemRecords();
    status_t fillMemRecords();
    status_t createDecoder();
    status_t setNumCores();
    status_t setParams(size_t stride);
    status_t getVersion();
    status_t initDecoder();
    bool setDecodeArgs(ivd_video_decode_ip_t *ps_decode_ip,
                       ivd_video_decode_op_t *ps_decode_op,
                       C2ReadView *inBuffer,
                       C2GraphicView *outBuffer,
                       size_t inOffset,
                       size_t inSize,
                       uint32_t tsMarker);
    bool getSeqInfo();
    c2_status_t ensureDecoderState(const std::shared_ptr<C2BlockPool> &pool);
    void finishWork(uint64_t index, const std::unique_ptr<C2Work> &work);
    status_t setFlushMode();
    c2_status_t drainInternal(
            uint32_t drainMode,
            const std::shared_ptr<C2BlockPool> &pool,
            const std::unique_ptr<C2Work> &work);
    status_t resetDecoder();
    void resetPlugin();
    status_t deleteDecoder();
    status_t reInitDecoder();

    // TODO:This is not the right place for this enum. These should
    // be part of c2-vndk so that they can be accessed by all video plugins
    // until then, make them feel at home
    enum {
        kNotSupported,
        kPreferBitstream,
        kPreferContainer,
    };

    std::shared_ptr<IntfImpl> mIntf;
    iv_obj_t *mDecHandle;
    iv_mem_rec_t *mMemRecords;
    size_t mNumMemRecords;
    std::shared_ptr<C2GraphicBlock> mOutBlock;
    uint8_t *mOutBufferDrain;

    size_t mNumCores;
    IV_COLOR_FORMAT_T mIvColorformat;

    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mStride;
    bool mSignalledOutputEos;
    bool mSignalledError;
    std::atomic_uint64_t mOutIndex;

    // Color aspects. These are ISO values and are meant to detect changes in aspects to avoid
    // converting them to C2 values for each frame
    struct VuiColorAspects {
        uint8_t primaries;
        uint8_t transfer;
        uint8_t coeffs;
        uint8_t fullRange;

        // default color aspects
        VuiColorAspects()
            : primaries(2), transfer(2), coeffs(2), fullRange(0) { }

        bool operator==(const VuiColorAspects &o) {
            return primaries == o.primaries && transfer == o.transfer && coeffs == o.coeffs
                    && fullRange == o.fullRange;
        }
    } mBitstreamColorAspects;

    // profile
    struct timeval mTimeStart;
    struct timeval mTimeEnd;
#ifdef FILE_DUMP_ENABLE
    char mInFile[200];
#endif /* FILE_DUMP_ENABLE */

    C2_DO_NOT_COPY(C2SoftMpeg2Dec);
};

}  // namespace android

#endif  // ANDROID_C2_SOFT_MPEG2_DEC_H_
