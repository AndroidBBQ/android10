/*
 * Copyright 2019 The Android Open Source Project
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

#ifndef ANDROID_C2_SOFT_HEVC_ENC_H_
#define ANDROID_C2_SOFT_HEVC_ENC_H_

#include <SimpleC2Component.h>
#include <algorithm>
#include <map>
#include <media/stagefright/foundation/ColorUtils.h>
#include <utils/Vector.h>

#include "ihevc_typedefs.h"

namespace android {

/** Get time */
#define GETTIME(a, b) gettimeofday(a, b)

/** Compute difference between start and end */
#define TIME_DIFF(start, end, diff)                      \
    diff = (((end).tv_sec - (start).tv_sec) * 1000000) + \
           ((end).tv_usec - (start).tv_usec);

#define CODEC_MAX_CORES  4
#define MAX_B_FRAMES     1
#define MAX_RC_LOOKAHEAD 1

#define DEFAULT_B_FRAMES     0
#define DEFAULT_RC_LOOKAHEAD 0

struct C2SoftHevcEnc : public SimpleC2Component {
    class IntfImpl;

    C2SoftHevcEnc(const char* name, c2_node_id_t id,
                  const std::shared_ptr<IntfImpl>& intfImpl);

    // From SimpleC2Component
    c2_status_t onInit() override;
    c2_status_t onStop() override;
    void onReset() override;
    void onRelease() override;
    c2_status_t onFlush_sm() override;
    void process(const std::unique_ptr<C2Work>& work,
                 const std::shared_ptr<C2BlockPool>& pool) override;
    c2_status_t drain(uint32_t drainMode,
                      const std::shared_ptr<C2BlockPool>& pool) override;

   protected:
    ~C2SoftHevcEnc() override;

   private:
    std::shared_ptr<IntfImpl> mIntf;
    ihevce_static_cfg_params_t mEncParams;
    size_t mNumCores;
    UWORD32 mIDRInterval;
    IV_COLOR_FORMAT_T mIvVideoColorFormat;
    UWORD32 mHevcEncProfile;
    UWORD32 mHevcEncLevel;
    bool mStarted;
    bool mSpsPpsHeaderReceived;
    bool mSignalledEos;
    bool mSignalledError;
    void* mCodecCtx;
    MemoryBlockPool mConversionBuffers;
    std::map<void*, MemoryBlock> mConversionBuffersInUse;
    // configurations used by component in process
    // (TODO: keep this in intf but make them internal only)
    std::shared_ptr<C2StreamPictureSizeInfo::input> mSize;
    std::shared_ptr<C2StreamFrameRateInfo::output> mFrameRate;
    std::shared_ptr<C2StreamBitrateInfo::output> mBitrate;
    std::shared_ptr<C2StreamBitrateModeTuning::output> mBitrateMode;
    std::shared_ptr<C2StreamComplexityTuning::output> mComplexity;
    std::shared_ptr<C2StreamQualityTuning::output> mQuality;

#ifdef FILE_DUMP_ENABLE
    char mInFile[200];
    char mOutFile[200];
#endif /* FILE_DUMP_ENABLE */

    // profile
    struct timeval mTimeStart;
    struct timeval mTimeEnd;

    c2_status_t initEncParams();
    c2_status_t initEncoder();
    c2_status_t releaseEncoder();
    c2_status_t setEncodeArgs(ihevce_inp_buf_t* ps_encode_ip,
                              const C2GraphicView* const input,
                              uint64_t workIndex);
    void finishWork(uint64_t index, const std::unique_ptr<C2Work>& work,
                    const std::shared_ptr<C2BlockPool>& pool,
                    ihevce_out_buf_t* ps_encode_op);
    c2_status_t drainInternal(uint32_t drainMode,
                              const std::shared_ptr<C2BlockPool>& pool,
                              const std::unique_ptr<C2Work>& work);
    C2_DO_NOT_COPY(C2SoftHevcEnc);
};
#ifdef FILE_DUMP_ENABLE

#define INPUT_DUMP_PATH "/data/local/tmp/hevc"
#define INPUT_DUMP_EXT "yuv"
#define OUTPUT_DUMP_PATH "/data/local/tmp/hevc"
#define OUTPUT_DUMP_EXT "h265"
#define GENERATE_FILE_NAMES()                                             \
{                                                                         \
    GETTIME(&mTimeStart, NULL);                                           \
    strcpy(mInFile, "");                                                  \
    ALOGD("GENERATE_FILE_NAMES");                                         \
    sprintf(mInFile, "%s_%ld.%ld.%s", INPUT_DUMP_PATH, mTimeStart.tv_sec, \
            mTimeStart.tv_usec, INPUT_DUMP_EXT);                          \
    strcpy(mOutFile, "");                                                 \
    sprintf(mOutFile, "%s_%ld.%ld.%s", OUTPUT_DUMP_PATH,                  \
            mTimeStart.tv_sec, mTimeStart.tv_usec, OUTPUT_DUMP_EXT);      \
}

#define CREATE_DUMP_FILE(m_filename)                 \
{                                                    \
    FILE* fp = fopen(m_filename, "wb");              \
    if (fp != NULL) {                                \
        ALOGD("Opened file %s", m_filename);         \
        fclose(fp);                                  \
    } else {                                         \
        ALOGD("Could not open file %s", m_filename); \
    }                                                \
}
#define DUMP_TO_FILE(m_filename, m_buf, m_size)          \
{                                                        \
    FILE* fp = fopen(m_filename, "ab");                  \
    if (fp != NULL && m_buf != NULL) {                   \
        int i;                                           \
        ALOGD("Dump to file!");                          \
        i = fwrite(m_buf, 1, m_size, fp);                \
        if (i != (int)m_size) {                          \
            ALOGD("Error in fwrite, returned %d", i);    \
            perror("Error in write to file");            \
        }                                                \
        fclose(fp);                                      \
    } else {                                             \
        ALOGD("Could not write to file %s", m_filename); \
        if (fp != NULL) fclose(fp);                      \
    }                                                    \
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

#endif  // C2_SOFT_HEVC_ENC_H__
