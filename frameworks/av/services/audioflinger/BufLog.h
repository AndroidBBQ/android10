/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef ANDROID_AUDIO_BUFLOG_H
#define ANDROID_AUDIO_BUFLOG_H

/*
 * BUFLOG creates up to BUFLOG_MAXSTREAMS simultaneous streams [0:15] of audio buffer data
 * and saves them to disk. The files are stored in the path specified in BUFLOG_BASE_PATH and
 * are named following this format:
 *   YYYYMMDDHHMMSS_id_format_channels_samplingrate.raw
 *
 * Normally we strip BUFLOG dumps from release builds.
 * You can modify this (for example with "#define BUFLOG_NDEBUG 0"
 * at the top of your source file) to change that behavior.
 *
 * usage:
 * - Add this to the top of the source file you want to debug:
 *   #define BUFLOG_NDEBUG 0
 *   #include "BufLog.h"
 *
 * - dump an audio buffer
 *  BUFLOG(buff_id, buff_tag, format, channels, sampling_rate, max_bytes, buff_pointer, buff_size);
 *
 *  buff_id:   int [0:15]   buffer id. If a buffer doesn't exist, it is created the first time.
 *  buff_tag:  char*        string tag used on stream filename and logs
 *  format:    int          Audio format (audio_format_t see audio.h)
 *  channels:  int          Channel Count
 *  sampling_rate:  int     Sampling rate in Hz. e.g. 8000, 16000, 44100, 48000, etc
 *  max_bytes: int [0 or positive number]
 *                          Maximum size of the file (in bytes) to be output.
 *                          If the value is 0, no limit.
 *  buff_pointer: void *    Pointer to audio buffer.
 *  buff_size:  int         Size (in bytes) of the current audio buffer to be stored.
 *
 *
 *  Example usage:
 *    int format       = mConfig.outputCfg.format;
 *    int channels     = audio_channel_count_from_out_mask(mConfig.outputCfg.channels);
 *    int samplingRate = mConfig.outputCfg.samplingRate;
 *    int frameCount   = mConfig.outputCfg.buffer.frameCount;
 *    int frameSize    = audio_bytes_per_sample((audio_format_t)format) * channels;
 *    int buffSize     = frameCount * frameSize;
 *    long maxBytes = 10 * samplingRate * frameSize; //10 seconds max
 *  BUFLOG(11, "loudnes_enhancer_out", format, channels, samplingRate, maxBytes,
 *                               mConfig.outputCfg.buffer.raw, buffSize);
 *
 *  Other macros:
 *  BUFLOG_EXISTS       returns true if there is an instance of BufLog
 *
 *  BUFLOG_RESET        If an instance of BufLog exists, it stops the capture and closes all
 *                      streams.
 *                      If a new call to BUFLOG(..) is done, new streams are created.
 */

#ifndef BUFLOG_NDEBUG
#ifdef NDEBUG
#define BUFLOG_NDEBUG 1
#else
#define BUFLOG_NDEBUG 0
#endif
#endif

/*
 * Simplified macro to send a buffer.
 */
#ifndef BUFLOG
#define __BUFLOG(STREAMID, TAG, FORMAT, CHANNELS, SAMPLINGRATE, MAXBYTES, BUF, SIZE) \
    BufLogSingleton::instance()->write(STREAMID, TAG, FORMAT, CHANNELS, SAMPLINGRATE, MAXBYTES, \
            BUF, SIZE)
#if BUFLOG_NDEBUG
#define BUFLOG(STREAMID, TAG, FORMAT, CHANNELS, SAMPLINGRATE, MAXBYTES, BUF, SIZE) \
    do { if (0) {  } } while (0)
#else
#define BUFLOG(STREAMID, TAG, FORMAT, CHANNELS, SAMPLINGRATE, MAXBYTES, BUF, SIZE) \
    __BUFLOG(STREAMID, TAG, FORMAT, CHANNELS, SAMPLINGRATE, MAXBYTES, BUF, SIZE)
#endif
#endif

#ifndef BUFLOG_EXISTS
#define BUFLOG_EXISTS BufLogSingleton::instanceExists()
#endif

#ifndef BUFLOG_RESET
#define BUFLOG_RESET do { if (BufLogSingleton::instanceExists()) { \
    BufLogSingleton::instance()->reset(); } } while (0)
#endif


#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <utils/Mutex.h>

//BufLog configuration
#define BUFLOGSTREAM_MAX_TAGSIZE    32
#define BUFLOG_BASE_PATH            "/data/misc/audioserver"
#define BUFLOG_MAX_PATH_SIZE        300

class BufLogStream {
public:
    BufLogStream(unsigned int id,
            const char *tag,
            unsigned int format,
            unsigned int channels,
            unsigned int samplingRate,
            size_t maxBytes);
    ~BufLogStream();

    // write buffer to stream
    //  buf:  pointer to buffer
    //  size: number of bytes to write
    size_t          write(const void *buf, size_t size);

    // pause/resume stream
    //  pause: true = paused, false = not paused
    //  return value: previous state of stream (paused or not).
    bool            setPause(bool pause);

    // will stop the stream and close any open file
    // the stream can't be reopen. Instead, a new stream (and file) should be created.
    void            finalize();

private:
    bool                mPaused;
    const unsigned int  mId;
    char                mTag[BUFLOGSTREAM_MAX_TAGSIZE + 1];
    const unsigned int  mFormat;
    const unsigned int  mChannels;
    const unsigned int  mSamplingRate;
    const size_t        mMaxBytes;
    size_t              mByteCount;
    FILE                *mFile;
    mutable android::Mutex mLock;

    void            closeStream_l();
};


class BufLog {
public:
    BufLog();
    ~BufLog();
    BufLog(BufLog const&) {};

    //  streamid:      int [0:BUFLOG_MAXSTREAMS-1]   buffer id.
    //                  If a buffer doesn't exist, it is created the first time is referenced
    //  tag:           char*  string tag used on stream filename and logs
    //  format:        int Audio format (audio_format_t see audio.h)
    //  channels:      int          Channel Count
    //  samplingRate:  int Sampling rate in Hz. e.g. 8000, 16000, 44100, 48000, etc
    //  maxBytes:      int [0 or positive number]
    //                  Maximum size of the file (in bytes) to be output.
    //                  If the value is 0, no limit.
    //  size:          int Size (in bytes) of the current audio buffer to be written.
    //  buf:           void *    Pointer to audio buffer.
    size_t          write(int streamid,
                        const char *tag,
                        int format,
                        int channels,
                        int samplingRate,
                        size_t maxBytes,
                        const void *buf,
                        size_t size);

    // reset will stop and close all active streams, thus finalizing any open file.
    //  New streams will be created if write() is called again.
    void            reset();

protected:
    static const unsigned int BUFLOG_MAXSTREAMS = 16;
    BufLogStream    *mStreams[BUFLOG_MAXSTREAMS];
    mutable android::Mutex mLock;
};

class BufLogSingleton {
public:
    static BufLog   *instance();
    static bool     instanceExists();

private:
    static void     initOnce();
    static BufLog   *mInstance;
};

#endif //ANDROID_AUDIO_BUFLOG_H
