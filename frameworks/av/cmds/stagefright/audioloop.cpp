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

#define LOG_NDEBUG 0
#define LOG_TAG "audioloop"
#include <utils/Log.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <utils/String16.h>

#include <binder/ProcessState.h>
#include <media/mediarecorder.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/AMRWriter.h>
#include <media/stagefright/AudioPlayer.h>
#include <media/stagefright/AudioSource.h>
#include <media/stagefright/MediaCodecSource.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/SimpleDecodingSource.h>
#include "SineSource.h"

using namespace android;

static void usage(const char* name)
{
    fprintf(stderr, "Usage: %s [-d du.ration] [-m] [-w] [-N name] [<output-file>]\n", name);
    fprintf(stderr, "Encodes either a sine wave or microphone input to AMR format\n");
    fprintf(stderr, "    -d    duration in seconds, default 5 seconds\n");
    fprintf(stderr, "    -m    use microphone for input, default sine source\n");
    fprintf(stderr, "    -w    use AMR wideband (default narrowband)\n");
    fprintf(stderr, "    -N    name of the encoder; must be set with -M\n");
    fprintf(stderr, "    -M    media type of the encoder; must be set with -N\n");
    fprintf(stderr, "    <output-file> output file for AMR encoding,"
            " if unspecified, decode to speaker.\n");
}

int main(int argc, char* argv[])
{
    static const int channels = 1; // not permitted to be stereo now
    unsigned duration = 5;
    bool useMic = false;
    bool outputWBAMR = false;
    bool playToSpeaker = true;
    const char* fileOut = NULL;
    AString name;
    AString mediaType;
    int ch;
    while ((ch = getopt(argc, argv, "d:mwN:M:")) != -1) {
        switch (ch) {
        case 'd':
            duration = atoi(optarg);
            break;
        case 'm':
            useMic = true;
            break;
        case 'w':
            outputWBAMR = true;
            break;
        case 'N':
            name.setTo(optarg);
            break;
        case 'M':
            mediaType.setTo(optarg);
            break;
        default:
            usage(argv[0]);
            return -1;
        }
    }
    argc -= optind;
    argv += optind;
    if (argc == 1) {
        fileOut = argv[0];
    }
    if ((name.empty() && !mediaType.empty()) || (!name.empty() && mediaType.empty())) {
        fprintf(stderr, "-N and -M must be set together\n");
        usage(argv[0]);
        return -1;
    }
    if (!name.empty() && fileOut != NULL) {
        fprintf(stderr, "-N and -M cannot be used with <output file>\n");
        usage(argv[0]);
        return -1;
    }
    int32_t sampleRate = !name.empty() ? 44100 : outputWBAMR ? 16000 : 8000;
    int32_t bitRate = sampleRate;

    android::ProcessState::self()->startThreadPool();
    sp<MediaSource> source;

    if (useMic) {
        // talk into the appropriate microphone for the duration
        source = new AudioSource(
                AUDIO_SOURCE_MIC,
                String16(),
                sampleRate,
                channels);
    } else {
        // use a sine source at 500 hz.
        source = new SineSource(sampleRate, channels);
    }

    sp<AMessage> meta = new AMessage;
    if (name.empty()) {
        meta->setString(
                "mime",
                outputWBAMR ? MEDIA_MIMETYPE_AUDIO_AMR_WB
                        : MEDIA_MIMETYPE_AUDIO_AMR_NB);
    } else {
        meta->setString("mime", mediaType);
        meta->setString("testing-name", name);
    }

    meta->setInt32("channel-count", channels);
    meta->setInt32("sample-rate", sampleRate);
    meta->setInt32("bitrate", bitRate);
    int32_t maxInputSize;
    if (source->getFormat()->findInt32(kKeyMaxInputSize, &maxInputSize)) {
        meta->setInt32("max-input-size", maxInputSize);
    }

    sp<ALooper> looper = new ALooper;
    looper->setName("audioloop");
    looper->start();

    sp<MediaSource> encoder = MediaCodecSource::Create(looper, meta, source);

    if (fileOut != NULL) {
        // target file specified, write encoded AMR output
        int fd = open(fileOut, O_CREAT | O_LARGEFILE | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
        if (fd < 0) {
            return 1;
        }
        sp<AMRWriter> writer = new AMRWriter(fd);
        close(fd);
        writer->addSource(encoder);
        writer->start();
        sleep(duration);
        writer->stop();
    } else {
        // otherwise decode to speaker
        sp<MediaSource> decoder = SimpleDecodingSource::Create(encoder);

        if (playToSpeaker) {
            AudioPlayer player(NULL);
            player.setSource(decoder);
            player.start();
            sleep(duration);

ALOGI("Line: %d", __LINE__);
            decoder.clear(); // must clear |decoder| otherwise delete player will hang.
ALOGI("Line: %d", __LINE__);
        } else {
            CHECK_EQ(decoder->start(), (status_t)OK);
            MediaBufferBase* buffer;
            while (decoder->read(&buffer) == OK) {
                // do something with buffer (save it eventually?)
                // need to stop after some count though...
                putchar('.');
                fflush(stdout);
                buffer->release();
                buffer = NULL;
            }
            CHECK_EQ(decoder->stop(), (status_t)OK);
        }
ALOGI("Line: %d", __LINE__);
    }

    return 0;
}
