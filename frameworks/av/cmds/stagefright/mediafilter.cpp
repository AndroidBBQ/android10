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

//#define LOG_NDEBUG 0
#define LOG_TAG "mediafilterTest"

#include <inttypes.h>

#include <binder/ProcessState.h>
#include <filters/ColorConvert.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/Surface.h>
#include <media/ICrypto.h>
#include <media/IMediaHTTPService.h>
#include <media/MediaCodecBuffer.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/NuMediaExtractor.h>
#include <media/stagefright/RenderScriptWrapper.h>
#include <OMX_IVCommon.h>
#include <ui/DisplayInfo.h>

#include "RenderScript.h"
#include "ScriptC_argbtorgba.h"
#include "ScriptC_nightvision.h"
#include "ScriptC_saturation.h"

// test parameters
static const bool kTestFlush = true;        // Note: true will drop 1 out of
static const int kFlushAfterFrames = 25;    // kFlushAfterFrames output frames
static const int64_t kTimeout = 500ll;

// built-in filter parameters
static const int32_t kInvert = false;   // ZeroFilter param
static const float kBlurRadius = 15.0f; // IntrinsicBlurFilter param
static const float kSaturation = 0.0f;  // SaturationFilter param

static void usage(const char *me) {
    fprintf(stderr, "usage: [flags] %s\n"
                    "\t[-b] use IntrinsicBlurFilter\n"
                    "\t[-c] use argb to rgba conversion RSFilter\n"
                    "\t[-n] use night vision RSFilter\n"
                    "\t[-r] use saturation RSFilter\n"
                    "\t[-s] use SaturationFilter\n"
                    "\t[-z] use ZeroFilter (copy filter)\n"
                    "\t[-R] render output to surface (enables -S)\n"
                    "\t[-S] allocate buffers from a surface\n"
                    "\t[-T] use render timestamps (enables -R)\n",
                    me);
    exit(1);
}

namespace android {

struct SaturationRSFilter : RenderScriptWrapper::RSFilterCallback {
    void init(const RSC::sp<RSC::RS> &context) {
        mScript = new ScriptC_saturation(context);
        mScript->set_gSaturation(3.f);
    }

    virtual status_t processBuffers(
            RSC::Allocation *inBuffer, RSC::Allocation *outBuffer) {
        mScript->forEach_root(inBuffer, outBuffer);

        return OK;
    }

    status_t handleSetParameters(const sp<AMessage> &msg __unused) {
        return OK;
    }

private:
    RSC::sp<ScriptC_saturation> mScript;
};

struct NightVisionRSFilter : RenderScriptWrapper::RSFilterCallback {
    void init(const RSC::sp<RSC::RS> &context) {
        mScript = new ScriptC_nightvision(context);
    }

    virtual status_t processBuffers(
            RSC::Allocation *inBuffer, RSC::Allocation *outBuffer) {
        mScript->forEach_root(inBuffer, outBuffer);

        return OK;
    }

    status_t handleSetParameters(const sp<AMessage> &msg __unused) {
        return OK;
    }

private:
    RSC::sp<ScriptC_nightvision> mScript;
};

struct ARGBToRGBARSFilter : RenderScriptWrapper::RSFilterCallback {
    void init(const RSC::sp<RSC::RS> &context) {
        mScript = new ScriptC_argbtorgba(context);
    }

    virtual status_t processBuffers(
            RSC::Allocation *inBuffer, RSC::Allocation *outBuffer) {
        mScript->forEach_root(inBuffer, outBuffer);

        return OK;
    }

    status_t handleSetParameters(const sp<AMessage> &msg __unused) {
        return OK;
    }

private:
    RSC::sp<ScriptC_argbtorgba> mScript;
};

struct CodecState {
    sp<MediaCodec> mCodec;
    Vector<sp<MediaCodecBuffer> > mInBuffers;
    Vector<sp<MediaCodecBuffer> > mOutBuffers;
    bool mSignalledInputEOS;
    bool mSawOutputEOS;
    int64_t mNumBuffersDecoded;
};

struct DecodedFrame {
    size_t index;
    size_t offset;
    size_t size;
    int64_t presentationTimeUs;
    uint32_t flags;
};

enum FilterType {
    FILTERTYPE_ZERO,
    FILTERTYPE_INTRINSIC_BLUR,
    FILTERTYPE_SATURATION,
    FILTERTYPE_RS_SATURATION,
    FILTERTYPE_RS_NIGHT_VISION,
    FILTERTYPE_RS_ARGB_TO_RGBA,
};

size_t inputFramesSinceFlush = 0;
void tryCopyDecodedBuffer(
        List<DecodedFrame> *decodedFrameIndices,
        CodecState *filterState,
        CodecState *vidState) {
    if (decodedFrameIndices->empty()) {
        return;
    }

    size_t filterIndex;
    status_t err = filterState->mCodec->dequeueInputBuffer(
            &filterIndex, kTimeout);
    if (err != OK) {
        return;
    }

    ++inputFramesSinceFlush;

    DecodedFrame frame = *decodedFrameIndices->begin();

    // only consume a buffer if we are not going to flush, since we expect
    // the dequeue -> flush -> queue operation to cause an error and
    // not produce an output frame
    if (!kTestFlush || inputFramesSinceFlush < kFlushAfterFrames) {
        decodedFrameIndices->erase(decodedFrameIndices->begin());
    }
    size_t outIndex = frame.index;

    const sp<MediaCodecBuffer> &srcBuffer =
        vidState->mOutBuffers.itemAt(outIndex);
    const sp<MediaCodecBuffer> &destBuffer =
        filterState->mInBuffers.itemAt(filterIndex);

    sp<AMessage> srcFormat, destFormat;
    vidState->mCodec->getOutputFormat(&srcFormat);
    filterState->mCodec->getInputFormat(&destFormat);

    int32_t srcWidth, srcHeight, srcStride, srcSliceHeight;
    int32_t srcColorFormat, destColorFormat;
    int32_t destWidth, destHeight, destStride, destSliceHeight;
    CHECK(srcFormat->findInt32("stride", &srcStride)
            && srcFormat->findInt32("slice-height", &srcSliceHeight)
            && srcFormat->findInt32("width", &srcWidth)
            && srcFormat->findInt32("height", & srcHeight)
            && srcFormat->findInt32("color-format", &srcColorFormat));
    CHECK(destFormat->findInt32("stride", &destStride)
            && destFormat->findInt32("slice-height", &destSliceHeight)
            && destFormat->findInt32("width", &destWidth)
            && destFormat->findInt32("height", & destHeight)
            && destFormat->findInt32("color-format", &destColorFormat));

    CHECK(srcWidth <= destStride && srcHeight <= destSliceHeight);

    convertYUV420spToARGB(
            srcBuffer->data(),
            srcBuffer->data() + srcStride * srcSliceHeight,
            srcWidth,
            srcHeight,
            destBuffer->data());

    // copy timestamp
    int64_t timeUs;
    CHECK(srcBuffer->meta()->findInt64("timeUs", &timeUs));
    destBuffer->meta()->setInt64("timeUs", timeUs);

    if (kTestFlush && inputFramesSinceFlush >= kFlushAfterFrames) {
        inputFramesSinceFlush = 0;

        // check that queueing a buffer that was dequeued before flush
        // fails with expected error EACCES
        filterState->mCodec->flush();

        err = filterState->mCodec->queueInputBuffer(
                filterIndex, 0 /* offset */, destBuffer->size(),
                timeUs, frame.flags);

        if (err == OK) {
            ALOGE("FAIL: queue after flush returned OK");
        } else if (err != -EACCES) {
            ALOGE("queueInputBuffer after flush returned %d, "
                    "expected -EACCES (-13)", err);
        }
    } else {
        err = filterState->mCodec->queueInputBuffer(
                filterIndex, 0 /* offset */, destBuffer->size(),
                timeUs, frame.flags);
        CHECK(err == OK);

        err = vidState->mCodec->releaseOutputBuffer(outIndex);
        CHECK(err == OK);
    }
}

size_t outputFramesSinceFlush = 0;
void tryDrainOutputBuffer(
        CodecState *filterState,
        const sp<Surface> &surface, bool renderSurface,
        bool useTimestamp, int64_t *startTimeRender) {
    size_t index;
    size_t offset;
    size_t size;
    int64_t presentationTimeUs;
    uint32_t flags;
    status_t err = filterState->mCodec->dequeueOutputBuffer(
            &index, &offset, &size, &presentationTimeUs, &flags,
            kTimeout);

    if (err != OK) {
        return;
    }

    ++outputFramesSinceFlush;

    if (kTestFlush && outputFramesSinceFlush >= kFlushAfterFrames) {
        filterState->mCodec->flush();
    }

    if (surface == NULL || !renderSurface) {
        err = filterState->mCodec->releaseOutputBuffer(index);
    } else if (useTimestamp) {
        if (*startTimeRender == -1) {
            // begin rendering 2 vsyncs after first decode
            *startTimeRender = systemTime(SYSTEM_TIME_MONOTONIC)
                    + 33000000 - (presentationTimeUs * 1000);
        }
        presentationTimeUs =
                (presentationTimeUs * 1000) + *startTimeRender;
        err = filterState->mCodec->renderOutputBufferAndRelease(
                index, presentationTimeUs);
    } else {
        err = filterState->mCodec->renderOutputBufferAndRelease(index);
    }

    if (kTestFlush && outputFramesSinceFlush >= kFlushAfterFrames) {
        outputFramesSinceFlush = 0;

        // releasing the buffer dequeued before flush should cause an error
        // if so, the frame will also be skipped in output stream
        if (err == OK) {
            ALOGE("FAIL: release after flush returned OK");
        } else if (err != -EACCES) {
            ALOGE("releaseOutputBuffer after flush returned %d, "
                    "expected -EACCES (-13)", err);
        }
    } else {
        CHECK(err == OK);
    }

    if (flags & MediaCodec::BUFFER_FLAG_EOS) {
        ALOGV("reached EOS on output.");
        filterState->mSawOutputEOS = true;
    }
}

static int decode(
        const sp<android::ALooper> &looper,
        const char *path,
        const sp<Surface> &surface,
        bool renderSurface,
        bool useTimestamp,
        FilterType filterType) {

    static int64_t kTimeout = 500ll;

    sp<NuMediaExtractor> extractor = new NuMediaExtractor;
    if (extractor->setDataSource(NULL /* httpService */, path) != OK) {
        fprintf(stderr, "unable to instantiate extractor.\n");
        return 1;
    }

    KeyedVector<size_t, CodecState> stateByTrack;

    CodecState *vidState = NULL;
    for (size_t i = 0; i < extractor->countTracks(); ++i) {
        sp<AMessage> format;
        status_t err = extractor->getTrackFormat(i, &format);
        CHECK(err == OK);

        AString mime;
        CHECK(format->findString("mime", &mime));
        bool isVideo = !strncasecmp(mime.c_str(), "video/", 6);
        if (!isVideo) {
            continue;
        }

        ALOGV("selecting track %zu", i);

        err = extractor->selectTrack(i);
        CHECK(err == OK);

        CodecState *state =
            &stateByTrack.editValueAt(stateByTrack.add(i, CodecState()));

        vidState = state;

        state->mNumBuffersDecoded = 0;

        state->mCodec = MediaCodec::CreateByType(
                looper, mime.c_str(), false /* encoder */);

        CHECK(state->mCodec != NULL);

        err = state->mCodec->configure(
                format, NULL /* surface */, NULL /* crypto */, 0 /* flags */);

        CHECK(err == OK);

        state->mSignalledInputEOS = false;
        state->mSawOutputEOS = false;

        break;
    }
    CHECK(!stateByTrack.isEmpty());
    CHECK(vidState != NULL);
    sp<AMessage> vidFormat;
    vidState->mCodec->getOutputFormat(&vidFormat);

    // set filter to use ARGB8888
    vidFormat->setInt32("color-format", OMX_COLOR_Format32bitARGB8888);
    // set app cache directory path
    vidFormat->setString("cacheDir", "/system/bin");

    // create RenderScript context for RSFilters
    RSC::sp<RSC::RS> context = new RSC::RS();
    context->init("/system/bin");

    sp<RenderScriptWrapper::RSFilterCallback> rsFilter;

    // create renderscript wrapper for RSFilters
    sp<RenderScriptWrapper> rsWrapper = new RenderScriptWrapper;
    rsWrapper->mContext = context.get();

    CodecState *filterState = new CodecState();
    filterState->mNumBuffersDecoded = 0;

    sp<AMessage> params = new AMessage();

    switch (filterType) {
        case FILTERTYPE_ZERO:
        {
            filterState->mCodec = MediaCodec::CreateByComponentName(
                    looper, "android.filter.zerofilter");
            params->setInt32("invert", kInvert);
            break;
        }
        case FILTERTYPE_INTRINSIC_BLUR:
        {
            filterState->mCodec = MediaCodec::CreateByComponentName(
                    looper, "android.filter.intrinsicblur");
            params->setFloat("blur-radius", kBlurRadius);
            break;
        }
        case FILTERTYPE_SATURATION:
        {
            filterState->mCodec = MediaCodec::CreateByComponentName(
                    looper, "android.filter.saturation");
            params->setFloat("saturation", kSaturation);
            break;
        }
        case FILTERTYPE_RS_SATURATION:
        {
            SaturationRSFilter *satFilter = new SaturationRSFilter;
            satFilter->init(context);
            rsFilter = satFilter;
            rsWrapper->mCallback = rsFilter;
            vidFormat->setObject("rs-wrapper", rsWrapper);

            filterState->mCodec = MediaCodec::CreateByComponentName(
                    looper, "android.filter.RenderScript");
            break;
        }
        case FILTERTYPE_RS_NIGHT_VISION:
        {
            NightVisionRSFilter *nightVisionFilter = new NightVisionRSFilter;
            nightVisionFilter->init(context);
            rsFilter = nightVisionFilter;
            rsWrapper->mCallback = rsFilter;
            vidFormat->setObject("rs-wrapper", rsWrapper);

            filterState->mCodec = MediaCodec::CreateByComponentName(
                    looper, "android.filter.RenderScript");
            break;
        }
        case FILTERTYPE_RS_ARGB_TO_RGBA:
        {
            ARGBToRGBARSFilter *argbToRgbaFilter = new ARGBToRGBARSFilter;
            argbToRgbaFilter->init(context);
            rsFilter = argbToRgbaFilter;
            rsWrapper->mCallback = rsFilter;
            vidFormat->setObject("rs-wrapper", rsWrapper);

            filterState->mCodec = MediaCodec::CreateByComponentName(
                    looper, "android.filter.RenderScript");
            break;
        }
        default:
        {
            LOG_ALWAYS_FATAL("mediacodec.cpp error: unrecognized FilterType");
            break;
        }
    }
    CHECK(filterState->mCodec != NULL);

    status_t err = filterState->mCodec->configure(
            vidFormat /* format */, surface, NULL /* crypto */, 0 /* flags */);
    CHECK(err == OK);

    filterState->mSignalledInputEOS = false;
    filterState->mSawOutputEOS = false;

    int64_t startTimeUs = android::ALooper::GetNowUs();
    int64_t startTimeRender = -1;

    for (size_t i = 0; i < stateByTrack.size(); ++i) {
        CodecState *state = &stateByTrack.editValueAt(i);

        sp<MediaCodec> codec = state->mCodec;

        CHECK_EQ((status_t)OK, codec->start());

        CHECK_EQ((status_t)OK, codec->getInputBuffers(&state->mInBuffers));
        CHECK_EQ((status_t)OK, codec->getOutputBuffers(&state->mOutBuffers));

        ALOGV("got %zu input and %zu output buffers",
                state->mInBuffers.size(), state->mOutBuffers.size());
    }

    CHECK_EQ((status_t)OK, filterState->mCodec->setParameters(params));

    if (kTestFlush) {
        status_t flushErr = filterState->mCodec->flush();
        if (flushErr == OK) {
            ALOGE("FAIL: Flush before start returned OK");
        } else {
            ALOGV("Flush before start returned status %d, usually ENOSYS (-38)",
                    flushErr);
        }
    }

    CHECK_EQ((status_t)OK, filterState->mCodec->start());
    CHECK_EQ((status_t)OK, filterState->mCodec->getInputBuffers(
            &filterState->mInBuffers));
    CHECK_EQ((status_t)OK, filterState->mCodec->getOutputBuffers(
            &filterState->mOutBuffers));

    if (kTestFlush) {
        status_t flushErr = filterState->mCodec->flush();
        if (flushErr != OK) {
            ALOGE("FAIL: Flush after start returned %d, expect OK (0)",
                    flushErr);
        } else {
            ALOGV("Flush immediately after start OK");
        }
    }

    List<DecodedFrame> decodedFrameIndices;

    // loop until decoder reaches EOS
    bool sawInputEOS = false;
    bool sawOutputEOSOnAllTracks = false;
    while (!sawOutputEOSOnAllTracks) {
        if (!sawInputEOS) {
            size_t trackIndex;
            status_t err = extractor->getSampleTrackIndex(&trackIndex);

            if (err != OK) {
                ALOGV("saw input eos");
                sawInputEOS = true;
            } else {
                CodecState *state = &stateByTrack.editValueFor(trackIndex);

                size_t index;
                err = state->mCodec->dequeueInputBuffer(&index, kTimeout);

                if (err == OK) {
                    ALOGV("filling input buffer %zu", index);

                    const sp<MediaCodecBuffer> &buffer = state->mInBuffers.itemAt(index);
                    sp<ABuffer> abuffer = new ABuffer(buffer->base(), buffer->capacity());

                    err = extractor->readSampleData(abuffer);
                    CHECK(err == OK);
                    buffer->setRange(abuffer->offset(), abuffer->size());

                    int64_t timeUs;
                    err = extractor->getSampleTime(&timeUs);
                    CHECK(err == OK);

                    uint32_t bufferFlags = 0;

                    err = state->mCodec->queueInputBuffer(
                            index, 0 /* offset */, buffer->size(),
                            timeUs, bufferFlags);

                    CHECK(err == OK);

                    extractor->advance();
                } else {
                    CHECK_EQ(err, -EAGAIN);
                }
            }
        } else {
            for (size_t i = 0; i < stateByTrack.size(); ++i) {
                CodecState *state = &stateByTrack.editValueAt(i);

                if (!state->mSignalledInputEOS) {
                    size_t index;
                    status_t err =
                        state->mCodec->dequeueInputBuffer(&index, kTimeout);

                    if (err == OK) {
                        ALOGV("signalling input EOS on track %zu", i);

                        err = state->mCodec->queueInputBuffer(
                                index, 0 /* offset */, 0 /* size */,
                                0ll /* timeUs */, MediaCodec::BUFFER_FLAG_EOS);

                        CHECK(err == OK);

                        state->mSignalledInputEOS = true;
                    } else {
                        CHECK_EQ(err, -EAGAIN);
                    }
                }
            }
        }

        sawOutputEOSOnAllTracks = true;
        for (size_t i = 0; i < stateByTrack.size(); ++i) {
            CodecState *state = &stateByTrack.editValueAt(i);

            if (state->mSawOutputEOS) {
                continue;
            } else {
                sawOutputEOSOnAllTracks = false;
            }

            DecodedFrame frame;
            status_t err = state->mCodec->dequeueOutputBuffer(
                    &frame.index, &frame.offset, &frame.size,
                    &frame.presentationTimeUs, &frame.flags, kTimeout);

            if (err == OK) {
                ALOGV("draining decoded buffer %zu, time = %lld us",
                        frame.index, (long long)frame.presentationTimeUs);

                ++(state->mNumBuffersDecoded);

                decodedFrameIndices.push_back(frame);

                if (frame.flags & MediaCodec::BUFFER_FLAG_EOS) {
                    ALOGV("reached EOS on decoder output.");
                    state->mSawOutputEOS = true;
                }

            } else if (err == INFO_OUTPUT_BUFFERS_CHANGED) {
                ALOGV("INFO_OUTPUT_BUFFERS_CHANGED");
                CHECK_EQ((status_t)OK, state->mCodec->getOutputBuffers(
                        &state->mOutBuffers));

                ALOGV("got %zu output buffers", state->mOutBuffers.size());
            } else if (err == INFO_FORMAT_CHANGED) {
                sp<AMessage> format;
                CHECK_EQ((status_t)OK, state->mCodec->getOutputFormat(&format));

                ALOGV("INFO_FORMAT_CHANGED: %s",
                        format->debugString().c_str());
            } else {
                CHECK_EQ(err, -EAGAIN);
            }

            tryCopyDecodedBuffer(&decodedFrameIndices, filterState, vidState);

            tryDrainOutputBuffer(
                    filterState, surface, renderSurface,
                    useTimestamp, &startTimeRender);
        }
    }

    // after EOS on decoder, let filter reach EOS
    while (!filterState->mSawOutputEOS) {
        tryCopyDecodedBuffer(&decodedFrameIndices, filterState, vidState);

        tryDrainOutputBuffer(
                filterState, surface, renderSurface,
                useTimestamp, &startTimeRender);
    }

    int64_t elapsedTimeUs = android::ALooper::GetNowUs() - startTimeUs;

    for (size_t i = 0; i < stateByTrack.size(); ++i) {
        CodecState *state = &stateByTrack.editValueAt(i);

        CHECK_EQ((status_t)OK, state->mCodec->release());

        printf("track %zu: %" PRId64 " frames decoded and filtered, "
                "%.2f fps.\n", i, state->mNumBuffersDecoded,
                state->mNumBuffersDecoded * 1E6 / elapsedTimeUs);
    }

    return 0;
}

}  // namespace android

int main(int argc, char **argv) {
    using namespace android;

    const char *me = argv[0];

    bool useSurface = false;
    bool renderSurface = false;
    bool useTimestamp = false;
    FilterType filterType = FILTERTYPE_ZERO;

    int res;
    while ((res = getopt(argc, argv, "bcnrszTRSh")) >= 0) {
        switch (res) {
            case 'b':
            {
                filterType = FILTERTYPE_INTRINSIC_BLUR;
                break;
            }
            case 'c':
            {
                filterType = FILTERTYPE_RS_ARGB_TO_RGBA;
                break;
            }
            case 'n':
            {
                filterType = FILTERTYPE_RS_NIGHT_VISION;
                break;
            }
            case 'r':
            {
                filterType = FILTERTYPE_RS_SATURATION;
                break;
            }
            case 's':
            {
                filterType = FILTERTYPE_SATURATION;
                break;
            }
            case 'z':
            {
                filterType = FILTERTYPE_ZERO;
                break;
            }
            case 'T':
            {
                useTimestamp = true;
                FALLTHROUGH_INTENDED;
            }
            case 'R':
            {
                renderSurface = true;
                FALLTHROUGH_INTENDED;
            }
            case 'S':
            {
                useSurface = true;
                break;
            }
            case '?':
            case 'h':
            default:
            {
                usage(me);
                break;
            }
        }
    }

    argc -= optind;
    argv += optind;

    if (argc != 1) {
        usage(me);
    }

    ProcessState::self()->startThreadPool();

    android::sp<android::ALooper> looper = new android::ALooper;
    looper->start();

    android::sp<SurfaceComposerClient> composerClient;
    android::sp<SurfaceControl> control;
    android::sp<Surface> surface;

    if (useSurface) {
        composerClient = new SurfaceComposerClient;
        CHECK_EQ((status_t)OK, composerClient->initCheck());

        const android::sp<IBinder> display = SurfaceComposerClient::getInternalDisplayToken();
        CHECK(display != nullptr);

        DisplayInfo info;
        CHECK_EQ(SurfaceComposerClient::getDisplayInfo(display, &info), NO_ERROR);

        ssize_t displayWidth = info.w;
        ssize_t displayHeight = info.h;

        ALOGV("display is %zd x %zd", displayWidth, displayHeight);

        control = composerClient->createSurface(
                String8("A Surface"), displayWidth, displayHeight,
                PIXEL_FORMAT_RGBA_8888, 0);

        CHECK(control != NULL);
        CHECK(control->isValid());

        SurfaceComposerClient::Transaction{}
                .setLayer(control, INT_MAX)
                .show(control)
                .apply();

        surface = control->getSurface();
        CHECK(surface != NULL);
    }

    decode(looper, argv[0], surface, renderSurface, useTimestamp, filterType);

    if (useSurface) {
        composerClient->dispose();
    }

    looper->stop();

    return 0;
}
