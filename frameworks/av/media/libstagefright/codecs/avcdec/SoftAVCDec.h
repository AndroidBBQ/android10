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

#ifndef SOFT_H264_DEC_H_

#define SOFT_H264_DEC_H_

#include <media/stagefright/omx/SoftVideoDecoderOMXComponent.h>
#include <sys/time.h>

namespace android {

/** Number of entries in the time-stamp array */
#define MAX_TIME_STAMPS 64

/** Maximum number of cores supported by the codec */
#define CODEC_MAX_NUM_CORES 4

#define CODEC_MAX_WIDTH     1920

#define CODEC_MAX_HEIGHT    1088

/** Input buffer size */
#define INPUT_BUF_SIZE (1024 * 1024)

#define MIN(a, b) ((a) < (b)) ? (a) : (b)

/** Used to remove warnings about unused parameters */
#define UNUSED(x) ((void)(x))

struct SoftAVC : public SoftVideoDecoderOMXComponent {
    SoftAVC(const char *name, const OMX_CALLBACKTYPE *callbacks,
            OMX_PTR appData, OMX_COMPONENTTYPE **component);

protected:
    virtual ~SoftAVC();

    virtual void onQueueFilled(OMX_U32 portIndex);
    virtual void onPortFlushCompleted(OMX_U32 portIndex);
    virtual void onReset();
    virtual int getColorAspectPreference();
private:
    // Number of input and output buffers
    enum {
        kNumBuffers = 8
    };

    iv_obj_t *mCodecCtx;         // Codec context

    size_t mNumCores;            // Number of cores to be uesd by the codec

    nsecs_t mTimeStart;   // Time at the start of decode()
    nsecs_t mTimeEnd;     // Time at the end of decode()

    // Internal buffer to be used to flush out the buffers from decoder
    uint8_t *mFlushOutBuffer;

    // Status of entries in the timestamp array
    bool mTimeStampsValid[MAX_TIME_STAMPS];

    // Timestamp array - Since codec does not take 64 bit timestamps,
    // they are maintained in the plugin
    OMX_S64 mTimeStamps[MAX_TIME_STAMPS];

#ifdef FILE_DUMP_ENABLE
    char mInFile[200];
#endif /* FILE_DUMP_ENABLE */

    OMX_COLOR_FORMATTYPE mOmxColorFormat;    // OMX Color format
    IV_COLOR_FORMAT_T mIvColorFormat;        // Ittiam Color format

    bool mIsInFlush;        // codec is flush mode
    bool mReceivedEOS;      // EOS is receieved on input port

    // The input stream has changed to a different resolution, which is still supported by the
    // codec. So the codec is switching to decode the new resolution.
    bool mChangingResolution;
    bool mFlushNeeded;
    bool mSignalledError;
    size_t mStride;
    size_t mInputOffset;

    status_t initDecoder();
    status_t deInitDecoder();
    status_t setFlushMode();
    status_t setParams(size_t stride);
    void logVersion();
    status_t setNumCores();
    status_t resetDecoder();
    status_t resetPlugin();


    bool setDecodeArgs(
            ivd_video_decode_ip_t *ps_dec_ip,
            ivd_video_decode_op_t *ps_dec_op,
            OMX_BUFFERHEADERTYPE *inHeader,
            OMX_BUFFERHEADERTYPE *outHeader,
            size_t timeStampIx);

    bool getVUIParams();

    DISALLOW_EVIL_CONSTRUCTORS(SoftAVC);
};

#ifdef FILE_DUMP_ENABLE

#define INPUT_DUMP_PATH     "/sdcard/media/avcd_input"
#define INPUT_DUMP_EXT      "h264"

#define GENERATE_FILE_NAMES() {                         \
    strcpy(mInFile, "");                                \
    sprintf(mInFile, "%s_%lld.%s", INPUT_DUMP_PATH,     \
            (long long) mTimeStart,                     \
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
#define DUMP_TO_FILE(m_filename, m_buf, m_size, m_offset)\
{                                                       \
    FILE *fp = fopen(m_filename, "ab");                 \
    if (fp != NULL && m_buf != NULL && m_offset == 0) { \
        int i;                                          \
        i = fwrite(m_buf, 1, m_size, fp);               \
        ALOGD("fwrite ret %d to write %d", i, m_size);  \
        if (i != (int) m_size) {                        \
            ALOGD("Error in fwrite, returned %d", i);   \
            perror("Error in write to file");           \
        }                                               \
    } else if (fp == NULL) {                            \
        ALOGD("Could not write to file %s", m_filename);\
    }                                                   \
    if (fp) {                                           \
        fclose(fp);                                     \
    }                                                   \
}
#else /* FILE_DUMP_ENABLE */
#define INPUT_DUMP_PATH
#define INPUT_DUMP_EXT
#define OUTPUT_DUMP_PATH
#define OUTPUT_DUMP_EXT
#define GENERATE_FILE_NAMES()
#define CREATE_DUMP_FILE(m_filename)
#define DUMP_TO_FILE(m_filename, m_buf, m_size, m_offset)
#endif /* FILE_DUMP_ENABLE */

} // namespace android

#endif  // SOFT_H264_DEC_H_
