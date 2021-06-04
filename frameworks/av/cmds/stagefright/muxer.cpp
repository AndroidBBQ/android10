/*
 * Copyright (C) 2013 The Android Open Source Project
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
#define LOG_TAG "muxer"
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utils/Log.h>

#include <binder/ProcessState.h>
#include <media/IMediaHTTPService.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaMuxer.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/NuMediaExtractor.h>

static void usage(const char *me) {
    fprintf(stderr, "usage: %s [-a] [-v] [-s <trim start time>]"
                    " [-e <trim end time>] [-o <output file>]"
                    " <input video file>\n", me);
    fprintf(stderr, "       -h help\n");
    fprintf(stderr, "       -a use audio\n");
    fprintf(stderr, "       -v use video\n");
    fprintf(stderr, "       -w mux into WebM container (default is MP4)\n");
    fprintf(stderr, "       -s Time in milli-seconds when the trim should start\n");
    fprintf(stderr, "       -e Time in milli-seconds when the trim should end\n");
    fprintf(stderr, "       -o output file name. Default is /sdcard/muxeroutput.mp4\n");

    exit(1);
}

using namespace android;

static int muxing(
        const char *path,
        bool useAudio,
        bool useVideo,
        const char *outputFileName,
        bool enableTrim,
        int trimStartTimeMs,
        int trimEndTimeMs,
        int rotationDegrees,
        MediaMuxer::OutputFormat container = MediaMuxer::OUTPUT_FORMAT_MPEG_4) {
    sp<NuMediaExtractor> extractor = new NuMediaExtractor;
    if (extractor->setDataSource(NULL /* httpService */, path) != OK) {
        fprintf(stderr, "unable to instantiate extractor. %s\n", path);
        return 1;
    }

    if (outputFileName == NULL) {
        outputFileName = "/sdcard/muxeroutput.mp4";
    }

    ALOGV("input file %s, output file %s", path, outputFileName);
    ALOGV("useAudio %d, useVideo %d", useAudio, useVideo);

    int fd = open(outputFileName, O_CREAT | O_LARGEFILE | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);

    if (fd < 0) {
        ALOGE("couldn't open file");
        return fd;
    }
    sp<MediaMuxer> muxer = new MediaMuxer(fd, container);
    close(fd);

    size_t trackCount = extractor->countTracks();
    // Map the extractor's track index to the muxer's track index.
    KeyedVector<size_t, ssize_t> trackIndexMap;
    size_t bufferSize = 1 * 1024 * 1024;  // default buffer size is 1MB.

    bool haveAudio = false;
    bool haveVideo = false;

    int64_t trimStartTimeUs = trimStartTimeMs * 1000;
    int64_t trimEndTimeUs = trimEndTimeMs * 1000;
    bool trimStarted = false;
    int64_t trimOffsetTimeUs = 0;

    for (size_t i = 0; i < trackCount; ++i) {
        sp<AMessage> format;
        status_t err = extractor->getTrackFormat(i, &format);
        CHECK_EQ(err, (status_t)OK);
        ALOGV("extractor getTrackFormat: %s", format->debugString().c_str());

        AString mime;
        CHECK(format->findString("mime", &mime));

        bool isAudio = !strncasecmp(mime.c_str(), "audio/", 6);
        bool isVideo = !strncasecmp(mime.c_str(), "video/", 6);

        if (useAudio && !haveAudio && isAudio) {
            haveAudio = true;
        } else if (useVideo && !haveVideo && isVideo) {
            haveVideo = true;
        } else {
            continue;
        }

        if (isVideo) {
            int width , height;
            CHECK(format->findInt32("width", &width));
            CHECK(format->findInt32("height", &height));
            bufferSize = width * height * 4;  // Assuming it is maximally 4BPP
        }

        int64_t duration;
        CHECK(format->findInt64("durationUs", &duration));

        // Since we got the duration now, correct the start time.
        if (enableTrim) {
            if (trimStartTimeUs > duration) {
                fprintf(stderr, "Warning: trimStartTimeUs > duration,"
                                " reset to 0\n");
                trimStartTimeUs = 0;
            }
        }

        ALOGV("selecting track %zu", i);

        err = extractor->selectTrack(i);
        CHECK_EQ(err, (status_t)OK);

        ssize_t newTrackIndex = muxer->addTrack(format);
        if (newTrackIndex < 0) {
            fprintf(stderr, "%s track (%zu) unsupported by muxer\n",
                    isAudio ? "audio" : "video",
                    i);
        } else {
            trackIndexMap.add(i, newTrackIndex);
        }
    }

    int64_t muxerStartTimeUs = ALooper::GetNowUs();

    bool sawInputEOS = false;

    size_t trackIndex = -1;
    sp<ABuffer> newBuffer = new ABuffer(bufferSize);

    muxer->setOrientationHint(rotationDegrees);
    muxer->start();

    while (!sawInputEOS) {
        status_t err = extractor->getSampleTrackIndex(&trackIndex);
        if (err != OK) {
            ALOGV("saw input eos, err %d", err);
            sawInputEOS = true;
            break;
        } else if (trackIndexMap.indexOfKey(trackIndex) < 0) {
            // ALOGV("skipping input from unsupported track %zu", trackIndex);
            extractor->advance();
            continue;
        } else {
            // ALOGV("reading sample from track index %zu\n", trackIndex);
            err = extractor->readSampleData(newBuffer);
            CHECK_EQ(err, (status_t)OK);

            int64_t timeUs;
            err = extractor->getSampleTime(&timeUs);
            CHECK_EQ(err, (status_t)OK);

            sp<MetaData> meta;
            err = extractor->getSampleMeta(&meta);
            CHECK_EQ(err, (status_t)OK);

            uint32_t sampleFlags = 0;
            int32_t val;
            if (meta->findInt32(kKeyIsSyncFrame, &val) && val != 0) {
                // We only support BUFFER_FLAG_SYNCFRAME in the flag for now.
                sampleFlags |= MediaCodec::BUFFER_FLAG_SYNCFRAME;

                // We turn on trimming at the sync frame.
                if (enableTrim && timeUs > trimStartTimeUs &&
                    timeUs <= trimEndTimeUs) {
                    if (trimStarted == false) {
                        trimOffsetTimeUs = timeUs;
                    }
                    trimStarted = true;
                }
            }
            // Trim can end at any non-sync frame.
            if (enableTrim && timeUs > trimEndTimeUs) {
                trimStarted = false;
            }

            if (!enableTrim || (enableTrim && trimStarted)) {
                err = muxer->writeSampleData(newBuffer,
                                             trackIndexMap.valueFor(trackIndex),
                                             timeUs - trimOffsetTimeUs, sampleFlags);
            }

            extractor->advance();
        }
    }

    muxer->stop();
    newBuffer.clear();
    trackIndexMap.clear();

    int64_t elapsedTimeUs = ALooper::GetNowUs() - muxerStartTimeUs;
    fprintf(stderr, "SUCCESS: muxer generate the video in %" PRId64 " ms\n",
            elapsedTimeUs / 1000);

    return 0;
}

int main(int argc, char **argv) {
    const char *me = argv[0];

    bool useAudio = false;
    bool useVideo = false;
    char *outputFileName = NULL;
    int trimStartTimeMs = -1;
    int trimEndTimeMs = -1;
    int rotationDegrees = 0;
    // When trimStartTimeMs and trimEndTimeMs seems valid, we turn this switch
    // to true.
    bool enableTrim = false;
    MediaMuxer::OutputFormat container = MediaMuxer::OUTPUT_FORMAT_MPEG_4;

    int res;
    while ((res = getopt(argc, argv, "h?avo:s:e:r:w")) >= 0) {
        switch (res) {
            case 'a':
            {
                useAudio = true;
                break;
            }

            case 'v':
            {
                useVideo = true;
                break;
            }

            case 'w':
            {
                container = MediaMuxer::OUTPUT_FORMAT_WEBM;
                break;
            }

            case 'o':
            {
                outputFileName = optarg;
                break;
            }

            case 's':
            {
                trimStartTimeMs = atoi(optarg);
                break;
            }

            case 'e':
            {
                trimEndTimeMs = atoi(optarg);
                break;
            }

            case 'r':
            {
                rotationDegrees = atoi(optarg);
                break;
            }

            case '?':
            case 'h':
            default:
            {
                usage(me);
            }
        }
    }

    argc -= optind;
    argv += optind;

    if (argc != 1) {
        usage(me);
    }

    if (trimStartTimeMs < 0 || trimEndTimeMs < 0) {
        // If no input on either 's' or 'e', or they are obviously wrong input,
        // then turn off trimming.
        ALOGV("Trimming is disabled, copying the whole length video.");
        enableTrim = false;
    } else if (trimStartTimeMs > trimEndTimeMs) {
        fprintf(stderr, "ERROR: start time is bigger\n");
        return 1;
    } else {
        enableTrim = true;
    }

    if (!useAudio && !useVideo) {
        fprintf(stderr, "ERROR: Missing both -a and -v, no track to mux then.\n");
        return 1;
    }
    ProcessState::self()->startThreadPool();

    sp<ALooper> looper = new ALooper;
    looper->start();

    int result = muxing(argv[0], useAudio, useVideo, outputFileName,
                        enableTrim, trimStartTimeMs, trimEndTimeMs, rotationDegrees, container);

    looper->stop();

    return result;
}
