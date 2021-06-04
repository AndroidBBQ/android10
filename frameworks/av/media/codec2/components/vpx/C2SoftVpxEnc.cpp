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

//#define LOG_NDEBUG 0
#define LOG_TAG "C2SoftVpxEnc"
#include <log/log.h>
#include <utils/misc.h>

#include <media/hardware/VideoAPI.h>

#include <Codec2BufferUtils.h>
#include <C2Debug.h>
#include "C2SoftVpxEnc.h"

#ifndef INT32_MAX
#define INT32_MAX   2147483647
#endif

namespace android {

#if 0
static size_t getCpuCoreCount() {
    long cpuCoreCount = 1;
#if defined(_SC_NPROCESSORS_ONLN)
    cpuCoreCount = sysconf(_SC_NPROCESSORS_ONLN);
#else
    // _SC_NPROC_ONLN must be defined...
    cpuCoreCount = sysconf(_SC_NPROC_ONLN);
#endif
    CHECK(cpuCoreCount >= 1);
    ALOGV("Number of CPU cores: %ld", cpuCoreCount);
    return (size_t)cpuCoreCount;
}
#endif

C2SoftVpxEnc::C2SoftVpxEnc(const char* name, c2_node_id_t id,
                           const std::shared_ptr<IntfImpl>& intfImpl)
    : SimpleC2Component(
          std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mCodecContext(nullptr),
      mCodecConfiguration(nullptr),
      mCodecInterface(nullptr),
      mStrideAlign(2),
      mColorFormat(VPX_IMG_FMT_I420),
      mBitrateControlMode(VPX_VBR),
      mErrorResilience(false),
      mMinQuantizer(0),
      mMaxQuantizer(0),
      mTemporalLayers(0),
      mTemporalPatternType(VPXTemporalLayerPatternNone),
      mTemporalPatternLength(0),
      mTemporalPatternIdx(0),
      mLastTimestamp(0x7FFFFFFFFFFFFFFFull),
      mSignalledOutputEos(false),
      mSignalledError(false) {
    memset(mTemporalLayerBitrateRatio, 0, sizeof(mTemporalLayerBitrateRatio));
    mTemporalLayerBitrateRatio[0] = 100;
}

C2SoftVpxEnc::~C2SoftVpxEnc() {
    onRelease();
}

c2_status_t C2SoftVpxEnc::onInit() {
    status_t err = initEncoder();
    return err == OK ? C2_OK : C2_CORRUPTED;
}

void C2SoftVpxEnc::onRelease() {
    if (mCodecContext) {
        vpx_codec_destroy(mCodecContext);
        delete mCodecContext;
        mCodecContext = nullptr;
    }

    if (mCodecConfiguration) {
        delete mCodecConfiguration;
        mCodecConfiguration = nullptr;
    }

    // this one is not allocated by us
    mCodecInterface = nullptr;
}

c2_status_t C2SoftVpxEnc::onStop() {
    onRelease();
    mLastTimestamp = 0x7FFFFFFFFFFFFFFFLL;
    mSignalledOutputEos = false;
    mSignalledError = false;
    return C2_OK;
}

void C2SoftVpxEnc::onReset() {
    (void)onStop();
}

c2_status_t C2SoftVpxEnc::onFlush_sm() {
    return onStop();
}

status_t C2SoftVpxEnc::initEncoder() {
    vpx_codec_err_t codec_return;
    status_t result = UNKNOWN_ERROR;
    {
        IntfImpl::Lock lock = mIntf->lock();
        mSize = mIntf->getSize_l();
        mBitrate = mIntf->getBitrate_l();
        mBitrateMode = mIntf->getBitrateMode_l();
        mFrameRate = mIntf->getFrameRate_l();
        mIntraRefresh = mIntf->getIntraRefresh_l();
        mRequestSync = mIntf->getRequestSync_l();
        mTemporalLayers = mIntf->getTemporalLayers_l()->m.layerCount;
    }

    switch (mBitrateMode->value) {
        case C2Config::BITRATE_CONST:
            mBitrateControlMode = VPX_CBR;
            break;
        case C2Config::BITRATE_VARIABLE:
        [[fallthrough]];
        default:
            mBitrateControlMode = VPX_VBR;
            break;
    }

    setCodecSpecificInterface();
    if (!mCodecInterface) goto CleanUp;

    ALOGD("VPx: initEncoder. BRMode: %u. TSLayers: %zu. KF: %u. QP: %u - %u",
          (uint32_t)mBitrateControlMode, mTemporalLayers, mIntf->getSyncFramePeriod(),
          mMinQuantizer, mMaxQuantizer);

    mCodecConfiguration = new vpx_codec_enc_cfg_t;
    if (!mCodecConfiguration) goto CleanUp;
    codec_return = vpx_codec_enc_config_default(mCodecInterface,
                                                mCodecConfiguration,
                                                0);
    if (codec_return != VPX_CODEC_OK) {
        ALOGE("Error populating default configuration for vpx encoder.");
        goto CleanUp;
    }

    mCodecConfiguration->g_w = mSize->width;
    mCodecConfiguration->g_h = mSize->height;
    //mCodecConfiguration->g_threads = getCpuCoreCount();
    mCodecConfiguration->g_threads = 0;
    mCodecConfiguration->g_error_resilient = mErrorResilience;

    // timebase unit is microsecond
    // g_timebase is in seconds (i.e. 1/1000000 seconds)
    mCodecConfiguration->g_timebase.num = 1;
    mCodecConfiguration->g_timebase.den = 1000000;
    // rc_target_bitrate is in kbps, mBitrate in bps
    mCodecConfiguration->rc_target_bitrate = (mBitrate->value + 500) / 1000;
    mCodecConfiguration->rc_end_usage = mBitrateControlMode;
    // Disable frame drop - not allowed in MediaCodec now.
    mCodecConfiguration->rc_dropframe_thresh = 0;
    // Disable lagged encoding.
    mCodecConfiguration->g_lag_in_frames = 0;
    if (mBitrateControlMode == VPX_CBR) {
        // Disable spatial resizing.
        mCodecConfiguration->rc_resize_allowed = 0;
        // Single-pass mode.
        mCodecConfiguration->g_pass = VPX_RC_ONE_PASS;
        // Maximum amount of bits that can be subtracted from the target
        // bitrate - expressed as percentage of the target bitrate.
        mCodecConfiguration->rc_undershoot_pct = 100;
        // Maximum amount of bits that can be added to the target
        // bitrate - expressed as percentage of the target bitrate.
        mCodecConfiguration->rc_overshoot_pct = 15;
        // Initial value of the buffer level in ms.
        mCodecConfiguration->rc_buf_initial_sz = 500;
        // Amount of data that the encoder should try to maintain in ms.
        mCodecConfiguration->rc_buf_optimal_sz = 600;
        // The amount of data that may be buffered by the decoding
        // application in ms.
        mCodecConfiguration->rc_buf_sz = 1000;
        // Enable error resilience - needed for packet loss.
        mCodecConfiguration->g_error_resilient = 1;
        // Maximum key frame interval - for CBR boost to 3000
        mCodecConfiguration->kf_max_dist = 3000;
        // Encoder determines optimal key frame placement automatically.
        mCodecConfiguration->kf_mode = VPX_KF_AUTO;
    }

    // Frames temporal pattern - for now WebRTC like pattern is only supported.
    switch (mTemporalLayers) {
        case 0:
            mTemporalPatternLength = 0;
            break;
        case 1:
            mCodecConfiguration->ts_number_layers = 1;
            mCodecConfiguration->ts_rate_decimator[0] = 1;
            mCodecConfiguration->ts_periodicity = 1;
            mCodecConfiguration->ts_layer_id[0] = 0;
            mTemporalPattern[0] = kTemporalUpdateLastRefAll;
            mTemporalPatternLength = 1;
            break;
        case 2:
            mCodecConfiguration->ts_number_layers = 2;
            mCodecConfiguration->ts_rate_decimator[0] = 2;
            mCodecConfiguration->ts_rate_decimator[1] = 1;
            mCodecConfiguration->ts_periodicity = 2;
            mCodecConfiguration->ts_layer_id[0] = 0;
            mCodecConfiguration->ts_layer_id[1] = 1;
            mTemporalPattern[0] = kTemporalUpdateLastAndGoldenRefAltRef;
            mTemporalPattern[1] = kTemporalUpdateGoldenWithoutDependencyRefAltRef;
            mTemporalPattern[2] = kTemporalUpdateLastRefAltRef;
            mTemporalPattern[3] = kTemporalUpdateGoldenRefAltRef;
            mTemporalPattern[4] = kTemporalUpdateLastRefAltRef;
            mTemporalPattern[5] = kTemporalUpdateGoldenRefAltRef;
            mTemporalPattern[6] = kTemporalUpdateLastRefAltRef;
            mTemporalPattern[7] = kTemporalUpdateNone;
            mTemporalPatternLength = 8;
            break;
        case 3:
            mCodecConfiguration->ts_number_layers = 3;
            mCodecConfiguration->ts_rate_decimator[0] = 4;
            mCodecConfiguration->ts_rate_decimator[1] = 2;
            mCodecConfiguration->ts_rate_decimator[2] = 1;
            mCodecConfiguration->ts_periodicity = 4;
            mCodecConfiguration->ts_layer_id[0] = 0;
            mCodecConfiguration->ts_layer_id[1] = 2;
            mCodecConfiguration->ts_layer_id[2] = 1;
            mCodecConfiguration->ts_layer_id[3] = 2;
            mTemporalPattern[0] = kTemporalUpdateLastAndGoldenRefAltRef;
            mTemporalPattern[1] = kTemporalUpdateNoneNoRefGoldenRefAltRef;
            mTemporalPattern[2] = kTemporalUpdateGoldenWithoutDependencyRefAltRef;
            mTemporalPattern[3] = kTemporalUpdateNone;
            mTemporalPattern[4] = kTemporalUpdateLastRefAltRef;
            mTemporalPattern[5] = kTemporalUpdateNone;
            mTemporalPattern[6] = kTemporalUpdateGoldenRefAltRef;
            mTemporalPattern[7] = kTemporalUpdateNone;
            mTemporalPatternLength = 8;
            break;
        default:
            ALOGE("Wrong number of temporal layers %zu", mTemporalLayers);
            goto CleanUp;
    }
    // Set bitrate values for each layer
    for (size_t i = 0; i < mCodecConfiguration->ts_number_layers; i++) {
        mCodecConfiguration->ts_target_bitrate[i] =
            mCodecConfiguration->rc_target_bitrate *
            mTemporalLayerBitrateRatio[i] / 100;
    }
    if (mIntf->getSyncFramePeriod() >= 0) {
        mCodecConfiguration->kf_max_dist = mIntf->getSyncFramePeriod();
        mCodecConfiguration->kf_min_dist = mIntf->getSyncFramePeriod();
        mCodecConfiguration->kf_mode = VPX_KF_AUTO;
    }
    if (mMinQuantizer > 0) {
        mCodecConfiguration->rc_min_quantizer = mMinQuantizer;
    }
    if (mMaxQuantizer > 0) {
        mCodecConfiguration->rc_max_quantizer = mMaxQuantizer;
    }
    setCodecSpecificConfiguration();
    mCodecContext = new vpx_codec_ctx_t;
    if (!mCodecContext) goto CleanUp;
    codec_return = vpx_codec_enc_init(mCodecContext,
                                      mCodecInterface,
                                      mCodecConfiguration,
                                      0);  // flags
    if (codec_return != VPX_CODEC_OK) {
        ALOGE("Error initializing vpx encoder");
        goto CleanUp;
    }

    // Extra CBR settings
    if (mBitrateControlMode == VPX_CBR) {
        codec_return = vpx_codec_control(mCodecContext,
                                         VP8E_SET_STATIC_THRESHOLD,
                                         1);
        if (codec_return == VPX_CODEC_OK) {
            uint32_t rc_max_intra_target =
                (uint32_t)(mCodecConfiguration->rc_buf_optimal_sz * mFrameRate->value / 20 + 0.5);
            // Don't go below 3 times per frame bandwidth.
            if (rc_max_intra_target < 300) {
                rc_max_intra_target = 300;
            }
            codec_return = vpx_codec_control(mCodecContext,
                                             VP8E_SET_MAX_INTRA_BITRATE_PCT,
                                             rc_max_intra_target);
        }
        if (codec_return == VPX_CODEC_OK) {
            codec_return = vpx_codec_control(mCodecContext,
                                             VP8E_SET_CPUUSED,
                                             -8);
        }
        if (codec_return != VPX_CODEC_OK) {
            ALOGE("Error setting cbr parameters for vpx encoder.");
            goto CleanUp;
        }
    }

    codec_return = setCodecSpecificControls();
    if (codec_return != VPX_CODEC_OK) goto CleanUp;

    {
        uint32_t width = mSize->width;
        uint32_t height = mSize->height;
        if (((uint64_t)width * height) >
            ((uint64_t)INT32_MAX / 3)) {
            ALOGE("b/25812794, Buffer size is too big, width=%u, height=%u.", width, height);
        } else {
            uint32_t stride = (width + mStrideAlign - 1) & ~(mStrideAlign - 1);
            uint32_t vstride = (height + mStrideAlign - 1) & ~(mStrideAlign - 1);
            mConversionBuffer = MemoryBlock::Allocate(stride * vstride * 3 / 2);
            if (!mConversionBuffer.size()) {
                ALOGE("Allocating conversion buffer failed.");
            } else {
                mNumInputFrames = -1;
                return OK;
            }
        }
    }

CleanUp:
    onRelease();
    return result;
}

vpx_enc_frame_flags_t C2SoftVpxEnc::getEncodeFlags() {
    vpx_enc_frame_flags_t flags = 0;
    if (mTemporalPatternLength > 0) {
      int patternIdx = mTemporalPatternIdx % mTemporalPatternLength;
      mTemporalPatternIdx++;
      switch (mTemporalPattern[patternIdx]) {
          case kTemporalUpdateLast:
              flags |= VP8_EFLAG_NO_UPD_GF;
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_REF_GF;
              flags |= VP8_EFLAG_NO_REF_ARF;
              break;
          case kTemporalUpdateGoldenWithoutDependency:
              flags |= VP8_EFLAG_NO_REF_GF;
              [[fallthrough]];
          case kTemporalUpdateGolden:
              flags |= VP8_EFLAG_NO_REF_ARF;
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_UPD_LAST;
              break;
          case kTemporalUpdateAltrefWithoutDependency:
              flags |= VP8_EFLAG_NO_REF_ARF;
              flags |= VP8_EFLAG_NO_REF_GF;
              [[fallthrough]];
          case kTemporalUpdateAltref:
              flags |= VP8_EFLAG_NO_UPD_GF;
              flags |= VP8_EFLAG_NO_UPD_LAST;
              break;
          case kTemporalUpdateNoneNoRefAltref:
              flags |= VP8_EFLAG_NO_REF_ARF;
              [[fallthrough]];
          case kTemporalUpdateNone:
              flags |= VP8_EFLAG_NO_UPD_GF;
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_UPD_LAST;
              flags |= VP8_EFLAG_NO_UPD_ENTROPY;
              break;
          case kTemporalUpdateNoneNoRefGoldenRefAltRef:
              flags |= VP8_EFLAG_NO_REF_GF;
              flags |= VP8_EFLAG_NO_UPD_GF;
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_UPD_LAST;
              flags |= VP8_EFLAG_NO_UPD_ENTROPY;
              break;
          case kTemporalUpdateGoldenWithoutDependencyRefAltRef:
              flags |= VP8_EFLAG_NO_REF_GF;
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_UPD_LAST;
              break;
          case kTemporalUpdateLastRefAltRef:
              flags |= VP8_EFLAG_NO_UPD_GF;
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_REF_GF;
              break;
          case kTemporalUpdateGoldenRefAltRef:
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_UPD_LAST;
              break;
          case kTemporalUpdateLastAndGoldenRefAltRef:
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_REF_GF;
              break;
          case kTemporalUpdateLastRefAll:
              flags |= VP8_EFLAG_NO_UPD_ARF;
              flags |= VP8_EFLAG_NO_UPD_GF;
              break;
      }
    }
    return flags;
}

// TODO: add support for YUV input color formats
// TODO: add support for SVC, ARF. SVC and ARF returns multiple frames
// (hierarchical / noshow) in one call. These frames should be combined in to
// a single buffer and sent back to the client
void C2SoftVpxEnc::process(
        const std::unique_ptr<C2Work> &work,
        const std::shared_ptr<C2BlockPool> &pool) {
    // Initialize output work
    work->result = C2_OK;
    work->workletsProcessed = 1u;
    work->worklets.front()->output.flags = work->input.flags;

    if (mSignalledError || mSignalledOutputEos) {
        work->result = C2_BAD_VALUE;
        return;
    }
    // Initialize encoder if not already
    if (!mCodecContext && OK != initEncoder()) {
        ALOGE("Failed to initialize encoder");
        mSignalledError = true;
        work->result = C2_CORRUPTED;
        return;
    }

    std::shared_ptr<const C2GraphicView> rView;
    std::shared_ptr<C2Buffer> inputBuffer;
    if (!work->input.buffers.empty()) {
        inputBuffer = work->input.buffers[0];
        rView = std::make_shared<const C2GraphicView>(
                    inputBuffer->data().graphicBlocks().front().map().get());
        if (rView->error() != C2_OK) {
            ALOGE("graphic view map err = %d", rView->error());
            work->result = C2_CORRUPTED;
            return;
        }
    } else {
        ALOGV("Empty input Buffer");
        uint32_t flags = 0;
        if (work->input.flags & C2FrameData::FLAG_END_OF_STREAM) {
            flags |= C2FrameData::FLAG_END_OF_STREAM;
        }
        work->worklets.front()->output.flags = (C2FrameData::flags_t)flags;
        work->worklets.front()->output.buffers.clear();
        work->worklets.front()->output.ordinal = work->input.ordinal;
        work->workletsProcessed = 1u;
        return;
    }

    const C2ConstGraphicBlock inBuffer =
        inputBuffer->data().graphicBlocks().front();
    if (inBuffer.width() != mSize->width ||
        inBuffer.height() != mSize->height) {
        ALOGE("unexpected Input buffer attributes %d(%d) x %d(%d)",
              inBuffer.width(), mSize->width, inBuffer.height(),
              mSize->height);
        mSignalledError = true;
        work->result = C2_BAD_VALUE;
        return;
    }
    bool eos = ((work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0);
    vpx_image_t raw_frame;
    const C2PlanarLayout &layout = rView->layout();
    uint32_t width = rView->width();
    uint32_t height = rView->height();
    if (width > 0x8000 || height > 0x8000) {
        ALOGE("Image too big: %u x %u", width, height);
        work->result = C2_BAD_VALUE;
        return;
    }
    uint32_t stride = (width + mStrideAlign - 1) & ~(mStrideAlign - 1);
    uint32_t vstride = (height + mStrideAlign - 1) & ~(mStrideAlign - 1);
    switch (layout.type) {
        case C2PlanarLayout::TYPE_RGB:
        case C2PlanarLayout::TYPE_RGBA: {
            ConvertRGBToPlanarYUV(mConversionBuffer.data(), stride, vstride,
                                  mConversionBuffer.size(), *rView.get());
            vpx_img_wrap(&raw_frame, VPX_IMG_FMT_I420, width, height,
                         mStrideAlign, mConversionBuffer.data());
            break;
        }
        case C2PlanarLayout::TYPE_YUV: {
            if (!IsYUV420(*rView)) {
                ALOGE("input is not YUV420");
                work->result = C2_BAD_VALUE;
                return;
            }

            if (layout.planes[layout.PLANE_Y].colInc == 1
                    && layout.planes[layout.PLANE_U].colInc == 1
                    && layout.planes[layout.PLANE_V].colInc == 1) {
                // I420 compatible - though with custom offset and stride
                vpx_img_wrap(&raw_frame, VPX_IMG_FMT_I420, width, height,
                             mStrideAlign, (uint8_t*)rView->data()[0]);
                raw_frame.planes[1] = (uint8_t*)rView->data()[1];
                raw_frame.planes[2] = (uint8_t*)rView->data()[2];
                raw_frame.stride[0] = layout.planes[layout.PLANE_Y].rowInc;
                raw_frame.stride[1] = layout.planes[layout.PLANE_U].rowInc;
                raw_frame.stride[2] = layout.planes[layout.PLANE_V].rowInc;
            } else {
                // copy to I420
                MediaImage2 img = CreateYUV420PlanarMediaImage2(width, height, stride, vstride);
                if (mConversionBuffer.size() >= stride * vstride * 3 / 2) {
                    status_t err = ImageCopy(mConversionBuffer.data(), &img, *rView);
                    if (err != OK) {
                        ALOGE("Buffer conversion failed: %d", err);
                        work->result = C2_BAD_VALUE;
                        return;
                    }
                    vpx_img_wrap(&raw_frame, VPX_IMG_FMT_I420, stride, vstride,
                                 mStrideAlign, (uint8_t*)rView->data()[0]);
                    vpx_img_set_rect(&raw_frame, 0, 0, width, height);
                } else {
                    ALOGE("Conversion buffer is too small: %u x %u for %zu",
                            stride, vstride, mConversionBuffer.size());
                    work->result = C2_BAD_VALUE;
                    return;
                }
            }
            break;
        }
        default:
            ALOGE("Unrecognized plane type: %d", layout.type);
            work->result = C2_BAD_VALUE;
            return;
    }

    vpx_enc_frame_flags_t flags = getEncodeFlags();
    // handle dynamic config parameters
    {
        IntfImpl::Lock lock = mIntf->lock();
        std::shared_ptr<C2StreamIntraRefreshTuning::output> intraRefresh = mIntf->getIntraRefresh_l();
        std::shared_ptr<C2StreamBitrateInfo::output> bitrate = mIntf->getBitrate_l();
        std::shared_ptr<C2StreamRequestSyncFrameTuning::output> requestSync = mIntf->getRequestSync_l();
        lock.unlock();

        if (intraRefresh != mIntraRefresh) {
            mIntraRefresh = intraRefresh;
            ALOGV("Got mIntraRefresh request");
        }

        if (requestSync != mRequestSync) {
            // we can handle IDR immediately
            if (requestSync->value) {
                // unset request
                C2StreamRequestSyncFrameTuning::output clearSync(0u, C2_FALSE);
                std::vector<std::unique_ptr<C2SettingResult>> failures;
                mIntf->config({ &clearSync }, C2_MAY_BLOCK, &failures);
                ALOGV("Got sync request");
                flags |= VPX_EFLAG_FORCE_KF;
            }
            mRequestSync = requestSync;
        }

        if (bitrate != mBitrate) {
            mBitrate = bitrate;
            mCodecConfiguration->rc_target_bitrate =
                (mBitrate->value + 500) / 1000;
            vpx_codec_err_t res = vpx_codec_enc_config_set(mCodecContext,
                                                           mCodecConfiguration);
            if (res != VPX_CODEC_OK) {
                ALOGE("vpx encoder failed to update bitrate: %s",
                      vpx_codec_err_to_string(res));
                mSignalledError = true;
                work->result = C2_CORRUPTED;
                return;
            }
        }
    }

    uint64_t inputTimeStamp = work->input.ordinal.timestamp.peekull();
    uint32_t frameDuration;
    if (inputTimeStamp > mLastTimestamp) {
        frameDuration = (uint32_t)(inputTimeStamp - mLastTimestamp);
    } else {
        // Use default of 30 fps in case of 0 frame rate.
        float frameRate = mFrameRate->value;
        if (frameRate < 0.001) {
            frameRate = 30;
        }
        frameDuration = (uint32_t)(1000000 / frameRate + 0.5);
    }
    mLastTimestamp = inputTimeStamp;

    vpx_codec_err_t codec_return = vpx_codec_encode(mCodecContext, &raw_frame,
                                                    inputTimeStamp,
                                                    frameDuration, flags,
                                                    VPX_DL_REALTIME);
    if (codec_return != VPX_CODEC_OK) {
        ALOGE("vpx encoder failed to encode frame");
        mSignalledError = true;
        work->result = C2_CORRUPTED;
        return;
    }

    bool populated = false;
    vpx_codec_iter_t encoded_packet_iterator = nullptr;
    const vpx_codec_cx_pkt_t* encoded_packet;
    while ((encoded_packet = vpx_codec_get_cx_data(
                    mCodecContext, &encoded_packet_iterator))) {
        if (encoded_packet->kind == VPX_CODEC_CX_FRAME_PKT) {
            std::shared_ptr<C2LinearBlock> block;
            C2MemoryUsage usage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
            c2_status_t err = pool->fetchLinearBlock(encoded_packet->data.frame.sz, usage, &block);
            if (err != C2_OK) {
                ALOGE("fetchLinearBlock for Output failed with status %d", err);
                work->result = C2_NO_MEMORY;
                return;
            }
            C2WriteView wView = block->map().get();
            if (wView.error()) {
                ALOGE("write view map failed %d", wView.error());
                work->result = C2_CORRUPTED;
                return;
            }

            memcpy(wView.data(), encoded_packet->data.frame.buf, encoded_packet->data.frame.sz);
            ++mNumInputFrames;

            ALOGD("bytes generated %zu", encoded_packet->data.frame.sz);
            uint32_t flags = 0;
            if (eos) {
                flags |= C2FrameData::FLAG_END_OF_STREAM;
            }
            work->worklets.front()->output.flags = (C2FrameData::flags_t)flags;
            work->worklets.front()->output.buffers.clear();
            std::shared_ptr<C2Buffer> buffer = createLinearBuffer(block);
            if (encoded_packet->data.frame.flags & VPX_FRAME_IS_KEY) {
                buffer->setInfo(std::make_shared<C2StreamPictureTypeMaskInfo::output>(
                        0u /* stream id */, C2Config::SYNC_FRAME));
            }
            work->worklets.front()->output.buffers.push_back(buffer);
            work->worklets.front()->output.ordinal = work->input.ordinal;
            work->worklets.front()->output.ordinal.timestamp = encoded_packet->data.frame.pts;
            work->workletsProcessed = 1u;
            populated = true;
            if (eos) {
                mSignalledOutputEos = true;
                ALOGV("signalled EOS");
            }
        }
    }
    if (!populated) {
        work->workletsProcessed = 0u;
    }
}

c2_status_t C2SoftVpxEnc::drain(
        uint32_t drainMode,
        const std::shared_ptr<C2BlockPool> &pool) {
    (void)pool;
    if (drainMode == NO_DRAIN) {
        ALOGW("drain with NO_DRAIN: no-op");
        return C2_OK;
    }
    if (drainMode == DRAIN_CHAIN) {
        ALOGW("DRAIN_CHAIN not supported");
        return C2_OMITTED;
    }

    return C2_OK;
}

}  // namespace android
