/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include <inttypes.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

//#define LOG_NDEBUG 0
#define LOG_TAG "stagefright"
#include <media/stagefright/foundation/ADebug.h>

#include "jpeg.h"
#include "SineSource.h"

#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <media/DataSource.h>
#include <media/MediaSource.h>
#include <media/ICrypto.h>
#include <media/IMediaHTTPService.h>
#include <media/IMediaPlayerService.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AUtils.h>
#include "include/NuCachedSource2.h"
#include <media/stagefright/AudioPlayer.h>
#include <media/stagefright/DataSourceFactory.h>
#include <media/stagefright/JPEGSource.h>
#include <media/stagefright/InterfaceUtils.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaCodecConstants.h>
#include <media/stagefright/MediaCodecList.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/MediaExtractorFactory.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/SimpleDecodingSource.h>
#include <media/stagefright/Utils.h>
#include <media/mediametadataretriever.h>

#include <media/stagefright/foundation/hexdump.h>
#include <media/stagefright/MPEG2TSWriter.h>
#include <media/stagefright/MPEG4Writer.h>

#include <private/media/VideoFrame.h>

#include <gui/GLConsumer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>

#include <android/hardware/media/omx/1.0/IOmx.h>

using namespace android;

static long gNumRepetitions;
static long gMaxNumFrames;  // 0 means decode all available.
static long gReproduceBug;  // if not -1.
static bool gPreferSoftwareCodec;
static bool gForceToUseHardwareCodec;
static bool gPlaybackAudio;
static bool gWriteMP4;
static bool gDisplayHistogram;
static bool gVerbose = false;
static bool showProgress = true;
static String8 gWriteMP4Filename;
static String8 gComponentNameOverride;

static sp<ANativeWindow> gSurface;

static int64_t getNowUs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (int64_t)tv.tv_usec + tv.tv_sec * 1000000ll;
}

static int CompareIncreasing(const int64_t *a, const int64_t *b) {
    return (*a) < (*b) ? -1 : (*a) > (*b) ? 1 : 0;
}

static void displayDecodeHistogram(Vector<int64_t> *decodeTimesUs) {
    printf("decode times:\n");

    decodeTimesUs->sort(CompareIncreasing);

    size_t n = decodeTimesUs->size();
    int64_t minUs = decodeTimesUs->itemAt(0);
    int64_t maxUs = decodeTimesUs->itemAt(n - 1);

    printf("min decode time %" PRId64 " us (%.2f secs)\n", minUs, minUs / 1E6);
    printf("max decode time %" PRId64 " us (%.2f secs)\n", maxUs, maxUs / 1E6);

    size_t counts[100];
    for (size_t i = 0; i < 100; ++i) {
        counts[i] = 0;
    }

    for (size_t i = 0; i < n; ++i) {
        int64_t x = decodeTimesUs->itemAt(i);

        size_t slot = ((x - minUs) * 100) / (maxUs - minUs);
        if (slot == 100) { slot = 99; }

        ++counts[slot];
    }

    for (size_t i = 0; i < 100; ++i) {
        int64_t slotUs = minUs + (i * (maxUs - minUs) / 100);

        double fps = 1E6 / slotUs;
        printf("[%.2f fps]: %zu\n", fps, counts[i]);
    }
}

static void displayAVCProfileLevelIfPossible(const sp<MetaData>& meta) {
    uint32_t type;
    const void *data;
    size_t size;
    if (meta->findData(kKeyAVCC, &type, &data, &size)) {
        const uint8_t *ptr = (const uint8_t *)data;
        CHECK(size >= 7);
        CHECK(ptr[0] == 1);  // configurationVersion == 1
        uint8_t profile = ptr[1];
        uint8_t level = ptr[3];
        fprintf(stderr, "AVC video profile %d and level %d\n", profile, level);
    }
}

static void dumpSource(const sp<MediaSource> &source, const String8 &filename) {
    FILE *out = fopen(filename.string(), "wb");

    CHECK_EQ((status_t)OK, source->start());

    status_t err;
    for (;;) {
        MediaBufferBase *mbuf;
        err = source->read(&mbuf);

        if (err == INFO_FORMAT_CHANGED) {
            continue;
        } else if (err != OK) {
            break;
        }

        if (gVerbose) {
            MetaDataBase &meta = mbuf->meta_data();
            fprintf(stdout, "sample format: %s\n", meta.toString().c_str());
        }

        CHECK_EQ(
                fwrite((const uint8_t *)mbuf->data() + mbuf->range_offset(),
                       1,
                       mbuf->range_length(),
                       out),
                mbuf->range_length());

        mbuf->release();
        mbuf = NULL;
    }

    CHECK_EQ((status_t)OK, source->stop());

    fclose(out);
    out = NULL;
}

static void playSource(sp<MediaSource> &source) {
    sp<MetaData> meta = source->getFormat();

    const char *mime;
    CHECK(meta->findCString(kKeyMIMEType, &mime));

    sp<MediaSource> rawSource;
    if (!strcasecmp(MEDIA_MIMETYPE_AUDIO_RAW, mime)) {
        rawSource = source;
    } else {
        int flags = 0;
        if (gPreferSoftwareCodec) {
            flags |= MediaCodecList::kPreferSoftwareCodecs;
        }
        if (gForceToUseHardwareCodec) {
            CHECK(!gPreferSoftwareCodec);
            flags |= MediaCodecList::kHardwareCodecsOnly;
        }
        rawSource = SimpleDecodingSource::Create(
                source, flags, gSurface,
                gComponentNameOverride.isEmpty() ? nullptr : gComponentNameOverride.c_str(),
                !gComponentNameOverride.isEmpty());
        if (rawSource == NULL) {
            return;
        }
        displayAVCProfileLevelIfPossible(meta);
    }

    source.clear();

    status_t err = rawSource->start();

    if (err != OK) {
        fprintf(stderr, "rawSource returned error %d (0x%08x)\n", err, err);
        return;
    }

    if (gPlaybackAudio) {
        AudioPlayer *player = new AudioPlayer(NULL);
        player->setSource(rawSource);
        rawSource.clear();

        err = player->start(true /* sourceAlreadyStarted */);

        if (err == OK) {
            status_t finalStatus;
            while (!player->reachedEOS(&finalStatus)) {
                usleep(100000ll);
            }
        } else {
            fprintf(stderr, "unable to start playback err=%d (0x%08x)\n", err, err);
        }

        delete player;
        player = NULL;

        return;
    } else if (gReproduceBug >= 3 && gReproduceBug <= 5) {
        int64_t durationUs;
        CHECK(meta->findInt64(kKeyDuration, &durationUs));

        status_t err;
        MediaBufferBase *buffer;
        MediaSource::ReadOptions options;
        int64_t seekTimeUs = -1;
        for (;;) {
            err = rawSource->read(&buffer, &options);
            options.clearSeekTo();

            bool shouldSeek = false;
            if (err == INFO_FORMAT_CHANGED) {
                CHECK(buffer == NULL);

                printf("format changed.\n");
                continue;
            } else if (err != OK) {
                printf("reached EOF.\n");

                shouldSeek = true;
            } else {
                int64_t timestampUs;
                CHECK(buffer->meta_data().findInt64(kKeyTime, &timestampUs));

                bool failed = false;

                if (seekTimeUs >= 0) {
                    int64_t diff = timestampUs - seekTimeUs;

                    if (diff < 0) {
                        diff = -diff;
                    }

                    if ((gReproduceBug == 4 && diff > 500000)
                        || (gReproduceBug == 5 && timestampUs < 0)) {
                        printf("wanted: %.2f secs, got: %.2f secs\n",
                               seekTimeUs / 1E6, timestampUs / 1E6);

                        printf("ERROR: ");
                        failed = true;
                    }
                }

                printf("buffer has timestamp %" PRId64 " us (%.2f secs)\n",
                       timestampUs, timestampUs / 1E6);

                buffer->release();
                buffer = NULL;

                if (failed) {
                    break;
                }

                shouldSeek = ((double)rand() / RAND_MAX) < 0.1;

                if (gReproduceBug == 3) {
                    shouldSeek = false;
                }
            }

            seekTimeUs = -1;

            if (shouldSeek) {
                seekTimeUs = (rand() * (float)durationUs) / RAND_MAX;
                options.setSeekTo(seekTimeUs);

                printf("seeking to %" PRId64 " us (%.2f secs)\n",
                       seekTimeUs, seekTimeUs / 1E6);
            }
        }

        rawSource->stop();

        return;
    }

    int n = 0;
    int64_t startTime = getNowUs();

    long numIterationsLeft = gNumRepetitions;
    MediaSource::ReadOptions options;

    int64_t sumDecodeUs = 0;
    int64_t totalBytes = 0;

    Vector<int64_t> decodeTimesUs;

    while (numIterationsLeft-- > 0) {
        long numFrames = 0;

        MediaBufferBase *buffer;

        for (;;) {
            int64_t startDecodeUs = getNowUs();
            status_t err = rawSource->read(&buffer, &options);
            int64_t delayDecodeUs = getNowUs() - startDecodeUs;

            options.clearSeekTo();

            if (err != OK) {
                CHECK(buffer == NULL);

                if (err == INFO_FORMAT_CHANGED) {
                    printf("format changed.\n");
                    continue;
                }

                break;
            }

            if (buffer->range_length() > 0) {
                if (gDisplayHistogram && n > 0) {
                    // Ignore the first time since it includes some setup
                    // cost.
                    decodeTimesUs.push(delayDecodeUs);
                }

                if (gVerbose) {
                    MetaDataBase &meta = buffer->meta_data();
                    fprintf(stdout, "%ld sample format: %s\n", numFrames, meta.toString().c_str());
                } else if (showProgress && (n++ % 16) == 0) {
                    printf(".");
                    fflush(stdout);
                }
            }

            sumDecodeUs += delayDecodeUs;
            totalBytes += buffer->range_length();

            buffer->release();
            buffer = NULL;

            ++numFrames;
            if (gMaxNumFrames > 0 && numFrames == gMaxNumFrames) {
                break;
            }

            if (gReproduceBug == 1 && numFrames == 40) {
                printf("seeking past the end now.");
                options.setSeekTo(0x7fffffffL);
            } else if (gReproduceBug == 2 && numFrames == 40) {
                printf("seeking to 5 secs.");
                options.setSeekTo(5000000);
            }
        }

        if (showProgress) {
            printf("$");
            fflush(stdout);
        }

        options.setSeekTo(0);
    }

    rawSource->stop();
    printf("\n");

    int64_t delay = getNowUs() - startTime;
    if (!strncasecmp("video/", mime, 6)) {
        printf("avg. %.2f fps\n", n * 1E6 / delay);

        printf("avg. time to decode one buffer %.2f usecs\n",
               (double)sumDecodeUs / n);

        printf("decoded a total of %d frame(s).\n", n);

        if (gDisplayHistogram) {
            displayDecodeHistogram(&decodeTimesUs);
        }
    } else if (!strncasecmp("audio/", mime, 6)) {
        // Frame count makes less sense for audio, as the output buffer
        // sizes may be different across decoders.
        printf("avg. %.2f KB/sec\n", totalBytes / 1024 * 1E6 / delay);

        printf("decoded a total of %" PRId64 " bytes\n", totalBytes);
    }
}

////////////////////////////////////////////////////////////////////////////////

struct DetectSyncSource : public MediaSource {
    explicit DetectSyncSource(const sp<MediaSource> &source);

    virtual status_t start(MetaData *params = NULL);
    virtual status_t stop();
    virtual sp<MetaData> getFormat();

    virtual status_t read(
            MediaBufferBase **buffer, const ReadOptions *options);

private:
    enum StreamType {
        AVC,
        MPEG4,
        H263,
        OTHER,
    };

    sp<MediaSource> mSource;
    StreamType mStreamType;
    bool mSawFirstIDRFrame;

    DISALLOW_EVIL_CONSTRUCTORS(DetectSyncSource);
};

DetectSyncSource::DetectSyncSource(const sp<MediaSource> &source)
    : mSource(source),
      mStreamType(OTHER),
      mSawFirstIDRFrame(false) {
    const char *mime;
    CHECK(mSource->getFormat()->findCString(kKeyMIMEType, &mime));

    if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_AVC)) {
        mStreamType = AVC;
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_MPEG4)) {
        mStreamType = MPEG4;
        CHECK(!"sync frame detection not implemented yet for MPEG4");
    } else if (!strcasecmp(mime, MEDIA_MIMETYPE_VIDEO_H263)) {
        mStreamType = H263;
        CHECK(!"sync frame detection not implemented yet for H.263");
    }
}

status_t DetectSyncSource::start(MetaData *params) {
    mSawFirstIDRFrame = false;

    return mSource->start(params);
}

status_t DetectSyncSource::stop() {
    return mSource->stop();
}

sp<MetaData> DetectSyncSource::getFormat() {
    return mSource->getFormat();
}

static bool isIDRFrame(MediaBufferBase *buffer) {
    const uint8_t *data =
        (const uint8_t *)buffer->data() + buffer->range_offset();
    size_t size = buffer->range_length();
    for (size_t i = 0; i + 3 < size; ++i) {
        if (!memcmp("\x00\x00\x01", &data[i], 3)) {
            uint8_t nalType = data[i + 3] & 0x1f;
            if (nalType == 5) {
                return true;
            }
        }
    }

    return false;
}

status_t DetectSyncSource::read(
        MediaBufferBase **buffer, const ReadOptions *options) {
    for (;;) {
        status_t err = mSource->read(buffer, options);

        if (err != OK) {
            return err;
        }

        if (mStreamType == AVC) {
            bool isIDR = isIDRFrame(*buffer);
            (*buffer)->meta_data().setInt32(kKeyIsSyncFrame, isIDR);
            if (isIDR) {
                mSawFirstIDRFrame = true;
            }
        } else {
            (*buffer)->meta_data().setInt32(kKeyIsSyncFrame, true);
        }

        if (mStreamType != AVC || mSawFirstIDRFrame) {
            break;
        }

        // Ignore everything up to the first IDR frame.
        (*buffer)->release();
        *buffer = NULL;
    }

    return OK;
}

////////////////////////////////////////////////////////////////////////////////

static void writeSourcesToMP4(
        Vector<sp<MediaSource> > &sources, bool syncInfoPresent) {
#if 0
    sp<MPEG4Writer> writer =
        new MPEG4Writer(gWriteMP4Filename.string());
#else
    int fd = open(gWriteMP4Filename.string(), O_CREAT | O_LARGEFILE | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        fprintf(stderr, "couldn't open file");
        return;
    }
    sp<MPEG2TSWriter> writer =
        new MPEG2TSWriter(fd);
#endif

    // at most one minute.
    writer->setMaxFileDuration(60000000ll);

    for (size_t i = 0; i < sources.size(); ++i) {
        sp<MediaSource> source = sources.editItemAt(i);

        CHECK_EQ(writer->addSource(
                    syncInfoPresent ? source : new DetectSyncSource(source)),
                (status_t)OK);
    }

    sp<MetaData> params = new MetaData;
    params->setInt32(kKeyRealTimeRecording, false);
    CHECK_EQ(writer->start(params.get()), (status_t)OK);

    while (!writer->reachedEOS()) {
        usleep(100000);
    }
    writer->stop();
}

static void performSeekTest(const sp<MediaSource> &source) {
    CHECK_EQ((status_t)OK, source->start());

    int64_t durationUs;
    CHECK(source->getFormat()->findInt64(kKeyDuration, &durationUs));

    for (int64_t seekTimeUs = 0; seekTimeUs <= durationUs;
            seekTimeUs += 60000ll) {
        MediaSource::ReadOptions options;
        options.setSeekTo(
                seekTimeUs, MediaSource::ReadOptions::SEEK_PREVIOUS_SYNC);

        MediaBufferBase *buffer;
        status_t err;
        for (;;) {
            err = source->read(&buffer, &options);

            options.clearSeekTo();

            if (err == INFO_FORMAT_CHANGED) {
                CHECK(buffer == NULL);
                continue;
            }

            if (err != OK) {
                CHECK(buffer == NULL);
                break;
            }

            CHECK(buffer != NULL);

            if (buffer->range_length() > 0) {
                break;
            }

            buffer->release();
            buffer = NULL;
        }

        if (err == OK) {
            int64_t timeUs;
            CHECK(buffer->meta_data().findInt64(kKeyTime, &timeUs));

            printf("%" PRId64 "\t%" PRId64 "\t%" PRId64 "\n",
                   seekTimeUs, timeUs, seekTimeUs - timeUs);

            buffer->release();
            buffer = NULL;
        } else {
            printf("ERROR\n");
            break;
        }
    }

    CHECK_EQ((status_t)OK, source->stop());
}

static void usage(const char *me) {
    fprintf(stderr, "usage: %s [options] [input_filename]\n", me);
    fprintf(stderr, "       -h(elp)\n");
    fprintf(stderr, "       -a(udio)\n");
    fprintf(stderr, "       -n repetitions\n");
    fprintf(stderr, "       -l(ist) components\n");
    fprintf(stderr, "       -m max-number-of-frames-to-decode in each pass\n");
    fprintf(stderr, "       -b bug to reproduce\n");
    fprintf(stderr, "       -i(nfo) dump codec info (profiles and color formats supported, details)\n");
    fprintf(stderr, "       -t(humbnail) extract video thumbnail or album art\n");
    fprintf(stderr, "       -s(oftware) prefer software codec\n");
    fprintf(stderr, "       -r(hardware) force to use hardware codec\n");
    fprintf(stderr, "       -o playback audio\n");
    fprintf(stderr, "       -w(rite) filename (write to .mp4 file)\n");
    fprintf(stderr, "       -k seek test\n");
    fprintf(stderr, "       -N(ame) of the component\n");
    fprintf(stderr, "       -x display a histogram of decoding times/fps "
                    "(video only)\n");
    fprintf(stderr, "       -q don't show progress indicator\n");
    fprintf(stderr, "       -S allocate buffers from a surface\n");
    fprintf(stderr, "       -T allocate buffers from a surface texture\n");
    fprintf(stderr, "       -d(ump) output_filename (raw stream data to a file)\n");
    fprintf(stderr, "       -D(ump) output_filename (decoded PCM data to a file)\n");
    fprintf(stderr, "       -v be more verbose\n");
}

static void dumpCodecDetails(bool queryDecoders) {
    const char *codecType = queryDecoders? "Decoder" : "Encoder";
    printf("\n%s infos by media types:\n"
           "=============================\n", codecType);

    sp<IMediaCodecList> list = MediaCodecList::getInstance();
    size_t numCodecs = list->countCodecs();

    // gather all media types supported by codec class, and link to codecs that support them
    KeyedVector<AString, Vector<sp<MediaCodecInfo>>> allMediaTypes;
    for (size_t codec_ix = 0; codec_ix < numCodecs; ++codec_ix) {
        sp<MediaCodecInfo> info = list->getCodecInfo(codec_ix);
        if (info->isEncoder() == !queryDecoders) {
            Vector<AString> supportedMediaTypes;
            info->getSupportedMediaTypes(&supportedMediaTypes);
            if (!supportedMediaTypes.size()) {
                printf("warning: %s does not support any media types\n",
                        info->getCodecName());
            } else {
                for (const AString &mediaType : supportedMediaTypes) {
                    if (allMediaTypes.indexOfKey(mediaType) < 0) {
                        allMediaTypes.add(mediaType, Vector<sp<MediaCodecInfo>>());
                    }
                    allMediaTypes.editValueFor(mediaType).add(info);
                }
            }
        }
    }

    KeyedVector<AString, bool> visitedCodecs;
    for (size_t type_ix = 0; type_ix < allMediaTypes.size(); ++type_ix) {
        const AString &mediaType = allMediaTypes.keyAt(type_ix);
        printf("\nMedia type '%s':\n", mediaType.c_str());

        for (const sp<MediaCodecInfo> &info : allMediaTypes.valueAt(type_ix)) {
            sp<MediaCodecInfo::Capabilities> caps = info->getCapabilitiesFor(mediaType.c_str());
            if (caps == NULL) {
                printf("warning: %s does not have capabilities for type %s\n",
                        info->getCodecName(), mediaType.c_str());
                continue;
            }
            printf("  %s \"%s\" supports\n",
                       codecType, info->getCodecName());

            auto printList = [](const char *type, const Vector<AString> &values){
                printf("    %s: [", type);
                for (size_t j = 0; j < values.size(); ++j) {
                    printf("\n      %s%s", values[j].c_str(),
                            j == values.size() - 1 ? " " : ",");
                }
                printf("]\n");
            };

            if (visitedCodecs.indexOfKey(info->getCodecName()) < 0) {
                visitedCodecs.add(info->getCodecName(), true);
                {
                    Vector<AString> aliases;
                    info->getAliases(&aliases);
                    // quote alias
                    for (AString &alias : aliases) {
                        alias.insert("\"", 1, 0);
                        alias.append('"');
                    }
                    printList("aliases", aliases);
                }
                {
                    uint32_t attrs = info->getAttributes();
                    Vector<AString> list;
                    list.add(AStringPrintf("encoder: %d", !!(attrs & MediaCodecInfo::kFlagIsEncoder)));
                    list.add(AStringPrintf("vendor: %d", !!(attrs & MediaCodecInfo::kFlagIsVendor)));
                    list.add(AStringPrintf("software-only: %d", !!(attrs & MediaCodecInfo::kFlagIsSoftwareOnly)));
                    list.add(AStringPrintf("hw-accelerated: %d", !!(attrs & MediaCodecInfo::kFlagIsHardwareAccelerated)));
                    printList(AStringPrintf("attributes: %#x", attrs).c_str(), list);
                }

                printf("    owner: \"%s\"\n", info->getOwnerName());
                printf("    rank: %u\n", info->getRank());
            } else {
                printf("    aliases, attributes, owner, rank: see above\n");
            }

            {
                Vector<AString> list;
                Vector<MediaCodecInfo::ProfileLevel> profileLevels;
                caps->getSupportedProfileLevels(&profileLevels);
                for (const MediaCodecInfo::ProfileLevel &pl : profileLevels) {
                    const char *niceProfile =
                        mediaType.equalsIgnoreCase(MIMETYPE_AUDIO_AAC)   ? asString_AACObject(pl.mProfile) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_MPEG2) ? asString_MPEG2Profile(pl.mProfile) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_H263)  ? asString_H263Profile(pl.mProfile) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_MPEG4) ? asString_MPEG4Profile(pl.mProfile) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_AVC)   ? asString_AVCProfile(pl.mProfile) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_VP8)   ? asString_VP8Profile(pl.mProfile) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_HEVC)  ? asString_HEVCProfile(pl.mProfile) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_VP9)   ? asString_VP9Profile(pl.mProfile) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_AV1)   ? asString_AV1Profile(pl.mProfile) :"??";
                    const char *niceLevel =
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_MPEG2) ? asString_MPEG2Level(pl.mLevel) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_H263)  ? asString_H263Level(pl.mLevel) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_MPEG4) ? asString_MPEG4Level(pl.mLevel) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_AVC)   ? asString_AVCLevel(pl.mLevel) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_VP8)   ? asString_VP8Level(pl.mLevel) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_HEVC)  ? asString_HEVCTierLevel(pl.mLevel) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_VP9)   ? asString_VP9Level(pl.mLevel) :
                        mediaType.equalsIgnoreCase(MIMETYPE_VIDEO_AV1)   ? asString_AV1Level(pl.mLevel) :
                        "??";

                    list.add(AStringPrintf("% 5u/% 5u (%s/%s)",
                            pl.mProfile, pl.mLevel, niceProfile, niceLevel));
                }
                printList("profile/levels", list);
            }

            {
                Vector<AString> list;
                Vector<uint32_t> colors;
                caps->getSupportedColorFormats(&colors);
                for (uint32_t color : colors) {
                    list.add(AStringPrintf("%#x (%s)", color,
                            asString_ColorFormat((int32_t)color)));
                }
                printList("colors", list);
            }

            printf("    details: %s\n", caps->getDetails()->debugString(6).c_str());
        }
    }
}

int main(int argc, char **argv) {
    android::ProcessState::self()->startThreadPool();

    bool audioOnly = false;
    bool listComponents = false;
    bool dumpCodecInfo = false;
    bool extractThumbnail = false;
    bool seekTest = false;
    bool useSurfaceAlloc = false;
    bool useSurfaceTexAlloc = false;
    bool dumpStream = false;
    bool dumpPCMStream = false;
    String8 dumpStreamFilename;
    gNumRepetitions = 1;
    gMaxNumFrames = 0;
    gReproduceBug = -1;
    gPreferSoftwareCodec = false;
    gForceToUseHardwareCodec = false;
    gPlaybackAudio = false;
    gWriteMP4 = false;
    gDisplayHistogram = false;

    sp<android::ALooper> looper;

    int res;
    while ((res = getopt(argc, argv, "vhaqn:lm:b:itsrow:kN:xSTd:D:")) >= 0) {
        switch (res) {
            case 'a':
            {
                audioOnly = true;
                break;
            }

            case 'q':
            {
                showProgress = false;
                break;
            }

            case 'd':
            {
                dumpStream = true;
                dumpStreamFilename.setTo(optarg);
                break;
            }

            case 'D':
            {
                dumpPCMStream = true;
                audioOnly = true;
                dumpStreamFilename.setTo(optarg);
                break;
            }

            case 'N':
            {
                gComponentNameOverride.setTo(optarg);
                break;
            }

            case 'l':
            {
                listComponents = true;
                break;
            }

            case 'm':
            case 'n':
            case 'b':
            {
                char *end;
                long x = strtol(optarg, &end, 10);

                if (*end != '\0' || end == optarg || x <= 0) {
                    x = 1;
                }

                if (res == 'n') {
                    gNumRepetitions = x;
                } else if (res == 'm') {
                    gMaxNumFrames = x;
                } else {
                    CHECK_EQ(res, 'b');
                    gReproduceBug = x;
                }
                break;
            }

            case 'w':
            {
                gWriteMP4 = true;
                gWriteMP4Filename.setTo(optarg);
                break;
            }

            case 'i':
            {
                dumpCodecInfo = true;
                break;
            }

            case 't':
            {
                extractThumbnail = true;
                break;
            }

            case 's':
            {
                gPreferSoftwareCodec = true;
                break;
            }

            case 'r':
            {
                gForceToUseHardwareCodec = true;
                break;
            }

            case 'o':
            {
                gPlaybackAudio = true;
                break;
            }

            case 'k':
            {
                seekTest = true;
                break;
            }

            case 'x':
            {
                gDisplayHistogram = true;
                break;
            }

            case 'S':
            {
                useSurfaceAlloc = true;
                break;
            }

            case 'T':
            {
                useSurfaceTexAlloc = true;
                break;
            }

            case 'v':
            {
                gVerbose = true;
                break;
            }

            case '?':
            case 'h':
            default:
            {
                usage(argv[0]);
                exit(1);
                break;
            }
        }
    }

    if (gPlaybackAudio && !audioOnly) {
        // This doesn't make any sense if we're decoding the video track.
        gPlaybackAudio = false;
    }

    argc -= optind;
    argv += optind;

    if (extractThumbnail) {
        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder = sm->getService(String16("media.player"));
        sp<IMediaPlayerService> service =
            interface_cast<IMediaPlayerService>(binder);

        CHECK(service.get() != NULL);

        sp<IMediaMetadataRetriever> retriever =
            service->createMetadataRetriever();

        CHECK(retriever != NULL);

        for (int k = 0; k < argc; ++k) {
            const char *filename = argv[k];

            bool failed = true;

            int fd = open(filename, O_RDONLY | O_LARGEFILE);
            CHECK_GE(fd, 0);

            off64_t fileSize = lseek64(fd, 0, SEEK_END);
            CHECK_GE(fileSize, 0ll);

            CHECK_EQ(retriever->setDataSource(fd, 0, fileSize), (status_t)OK);

            close(fd);
            fd = -1;

            sp<IMemory> mem =
                    retriever->getFrameAtTime(-1,
                            MediaSource::ReadOptions::SEEK_PREVIOUS_SYNC,
                            HAL_PIXEL_FORMAT_RGB_565,
                            false /*metaOnly*/);

            if (mem != NULL) {
                failed = false;
                printf("getFrameAtTime(%s) => OK\n", filename);

                VideoFrame *frame = (VideoFrame *)mem->pointer();

                CHECK_EQ(writeJpegFile("/sdcard/out.jpg",
                            frame->getFlattenedData(),
                            frame->mWidth, frame->mHeight), 0);
            }

            {
                mem = retriever->extractAlbumArt();

                if (mem != NULL) {
                    failed = false;
                    printf("extractAlbumArt(%s) => OK\n", filename);
                }
            }

            if (failed) {
                printf("both getFrameAtTime and extractAlbumArt "
                    "failed on file '%s'.\n", filename);
            }
        }

        return 0;
    }

    if (dumpCodecInfo) {
        dumpCodecDetails(true /* queryDecoders */);
        dumpCodecDetails(false /* queryDecoders */);
    }

    if (listComponents) {
        using ::android::hardware::hidl_vec;
        using ::android::hardware::hidl_string;
        using namespace ::android::hardware::media::omx::V1_0;
        sp<IOmx> omx = IOmx::getService();
        CHECK(omx.get() != nullptr);

        hidl_vec<IOmx::ComponentInfo> nodeList;
        auto transStatus = omx->listNodes([](
                const auto& status, const auto& nodeList) {
                    CHECK(status == Status::OK);
                    for (const auto& info : nodeList) {
                        printf("%s\t Roles: ", info.mName.c_str());
                        for (const auto& role : info.mRoles) {
                            printf("%s\t", role.c_str());
                        }
                    }
                });
        CHECK(transStatus.isOk());
    }

    sp<SurfaceComposerClient> composerClient;
    sp<SurfaceControl> control;

    if ((useSurfaceAlloc || useSurfaceTexAlloc) && !audioOnly) {
        if (useSurfaceAlloc) {
            composerClient = new SurfaceComposerClient;
            CHECK_EQ(composerClient->initCheck(), (status_t)OK);

            control = composerClient->createSurface(
                    String8("A Surface"),
                    1280,
                    800,
                    PIXEL_FORMAT_RGB_565,
                    0);

            CHECK(control != NULL);
            CHECK(control->isValid());

            SurfaceComposerClient::Transaction{}
                    .setLayer(control, INT_MAX)
                    .show(control)
                    .apply();

            gSurface = control->getSurface();
            CHECK(gSurface != NULL);
        } else {
            CHECK(useSurfaceTexAlloc);

            sp<IGraphicBufferProducer> producer;
            sp<IGraphicBufferConsumer> consumer;
            BufferQueue::createBufferQueue(&producer, &consumer);
            sp<GLConsumer> texture = new GLConsumer(consumer, 0 /* tex */,
                    GLConsumer::TEXTURE_EXTERNAL, true /* useFenceSync */,
                    false /* isControlledByApp */);
            gSurface = new Surface(producer);
        }
    }

    status_t err = OK;

    for (int k = 0; k < argc && err == OK; ++k) {
        bool syncInfoPresent = true;

        const char *filename = argv[k];

        sp<DataSource> dataSource =
            DataSourceFactory::CreateFromURI(NULL /* httpService */, filename);

        if (strncasecmp(filename, "sine:", 5) && dataSource == NULL) {
            fprintf(stderr, "Unable to create data source.\n");
            return 1;
        }

        bool isJPEG = false;

        size_t len = strlen(filename);
        if (len >= 4 && !strcasecmp(filename + len - 4, ".jpg")) {
            isJPEG = true;
        }

        Vector<sp<MediaSource> > mediaSources;
        sp<MediaSource> mediaSource;

        if (isJPEG) {
            mediaSource = new JPEGSource(dataSource);
            if (gWriteMP4) {
                mediaSources.push(mediaSource);
            }
        } else if (!strncasecmp("sine:", filename, 5)) {
            char *end;
            long sampleRate = strtol(filename + 5, &end, 10);

            if (end == filename + 5) {
                sampleRate = 44100;
            }
            mediaSource = new SineSource(sampleRate, 1);
            if (gWriteMP4) {
                mediaSources.push(mediaSource);
            }
        } else {
            sp<IMediaExtractor> extractor = MediaExtractorFactory::Create(dataSource);

            if (extractor == NULL) {
                fprintf(stderr, "could not create extractor.\n");
                return -1;
            }

            sp<MetaData> meta = extractor->getMetaData();

            if (meta != NULL) {
                const char *mime;
                if (!meta->findCString(kKeyMIMEType, &mime)) {
                    fprintf(stderr, "extractor did not provide MIME type.\n");
                    return -1;
                }

                if (!strcasecmp(mime, MEDIA_MIMETYPE_CONTAINER_MPEG2TS)) {
                    syncInfoPresent = false;
                }
            }

            size_t numTracks = extractor->countTracks();

            if (gWriteMP4) {
                bool haveAudio = false;
                bool haveVideo = false;
                for (size_t i = 0; i < numTracks; ++i) {
                    sp<MediaSource> source = CreateMediaSourceFromIMediaSource(
                            extractor->getTrack(i));
                    if (source == nullptr) {
                        fprintf(stderr, "skip NULL track %zu, track count %zu.\n", i, numTracks);
                        continue;
                    }

                    const char *mime;
                    CHECK(source->getFormat()->findCString(
                                kKeyMIMEType, &mime));

                    bool useTrack = false;
                    if (!haveAudio && !strncasecmp("audio/", mime, 6)) {
                        haveAudio = true;
                        useTrack = true;
                    } else if (!haveVideo && !strncasecmp("video/", mime, 6)) {
                        haveVideo = true;
                        useTrack = true;
                    }

                    if (useTrack) {
                        mediaSources.push(source);

                        if (haveAudio && haveVideo) {
                            break;
                        }
                    }
                }
            } else {
                sp<MetaData> meta;
                size_t i;
                for (i = 0; i < numTracks; ++i) {
                    meta = extractor->getTrackMetaData(
                            i, MediaExtractor::kIncludeExtensiveMetaData);

                    if (meta == NULL) {
                        continue;
                    }
                    const char *mime;
                    meta->findCString(kKeyMIMEType, &mime);

                    if (audioOnly && !strncasecmp(mime, "audio/", 6)) {
                        break;
                    }

                    if (!audioOnly && !strncasecmp(mime, "video/", 6)) {
                        break;
                    }

                    meta = NULL;
                }

                if (meta == NULL) {
                    fprintf(stderr,
                            "No suitable %s track found. The '-a' option will "
                            "target audio tracks only, the default is to target "
                            "video tracks only.\n",
                            audioOnly ? "audio" : "video");
                    return -1;
                }

                int64_t thumbTimeUs;
                if (meta->findInt64(kKeyThumbnailTime, &thumbTimeUs)) {
                    printf("thumbnailTime: %" PRId64 " us (%.2f secs)\n",
                           thumbTimeUs, thumbTimeUs / 1E6);
                }

                mediaSource = CreateMediaSourceFromIMediaSource(extractor->getTrack(i));
                if (mediaSource == nullptr) {
                    fprintf(stderr, "skip NULL track %zu, total tracks %zu.\n", i, numTracks);
                    return -1;
                }
            }
        }

        if (gWriteMP4) {
            writeSourcesToMP4(mediaSources, syncInfoPresent);
        } else if (dumpStream) {
            dumpSource(mediaSource, dumpStreamFilename);
        } else if (dumpPCMStream) {
            sp<MediaSource> decSource = SimpleDecodingSource::Create(mediaSource);
            dumpSource(decSource, dumpStreamFilename);
        } else if (seekTest) {
            performSeekTest(mediaSource);
        } else {
            playSource(mediaSource);
        }
    }

    if ((useSurfaceAlloc || useSurfaceTexAlloc) && !audioOnly) {
        gSurface.clear();

        if (useSurfaceAlloc) {
            composerClient->dispose();
        }
    }

    return 0;
}
