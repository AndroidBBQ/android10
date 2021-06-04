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
#ifdef MPEG4
  #define LOG_TAG "C2SoftMpeg4Enc"
#else
  #define LOG_TAG "C2SoftH263Enc"
#endif
#include <log/log.h>

#include <inttypes.h>

#include <media/hardware/VideoAPI.h>
#include <media/stagefright/foundation/AUtils.h>
#include <media/stagefright/MediaDefs.h>
#include <utils/misc.h>

#include <C2Debug.h>
#include <C2PlatformSupport.h>
#include <SimpleC2Interface.h>
#include <util/C2InterfaceHelper.h>

#include "C2SoftMpeg4Enc.h"
#include "mp4enc_api.h"

namespace android {

namespace {

#ifdef MPEG4
constexpr char COMPONENT_NAME[] = "c2.android.mpeg4.encoder";
const char *MEDIA_MIMETYPE_VIDEO = MEDIA_MIMETYPE_VIDEO_MPEG4;
#else
constexpr char COMPONENT_NAME[] = "c2.android.h263.encoder";
const char *MEDIA_MIMETYPE_VIDEO = MEDIA_MIMETYPE_VIDEO_H263;
#endif

} // namepsace

class C2SoftMpeg4Enc::IntfImpl : public SimpleInterface<void>::BaseParams {
   public:
    explicit IntfImpl(const std::shared_ptr<C2ReflectorHelper> &helper)
        : SimpleInterface<void>::BaseParams(
                helper,
                COMPONENT_NAME,
                C2Component::KIND_ENCODER,
                C2Component::DOMAIN_VIDEO,
                MEDIA_MIMETYPE_VIDEO) {
        noPrivateBuffers(); // TODO: account for our buffers here
        noInputReferences();
        noOutputReferences();
        noInputLatency();
        noTimeStretch();
        setDerivedInstance(this);

        addParameter(
                DefineParam(mAttrib, C2_PARAMKEY_COMPONENT_ATTRIBUTES)
                .withConstValue(new C2ComponentAttributesSetting(
                    C2Component::ATTRIB_IS_TEMPORAL))
                .build());

        addParameter(
                DefineParam(mUsage, C2_PARAMKEY_INPUT_STREAM_USAGE)
                .withConstValue(new C2StreamUsageTuning::input(
                        0u, (uint64_t)C2MemoryUsage::CPU_READ))
                .build());

        addParameter(
            DefineParam(mSize, C2_PARAMKEY_PICTURE_SIZE)
                .withDefault(new C2StreamPictureSizeInfo::input(0u, 176, 144))
                .withFields({
#ifdef MPEG4
                    C2F(mSize, width).inRange(16, 176, 16),
                    C2F(mSize, height).inRange(16, 144, 16),
#else
                    C2F(mSize, width).oneOf({176, 352}),
                    C2F(mSize, height).oneOf({144, 288}),
#endif
                })
                .withSetter(SizeSetter)
                .build());

        addParameter(
            DefineParam(mFrameRate, C2_PARAMKEY_FRAME_RATE)
                .withDefault(new C2StreamFrameRateInfo::output(0u, 17.))
                // TODO: More restriction?
                .withFields({C2F(mFrameRate, value).greaterThan(0.)})
                .withSetter(
                    Setter<decltype(*mFrameRate)>::StrictValueWithNoDeps)
                .build());

        addParameter(
            DefineParam(mBitrate, C2_PARAMKEY_BITRATE)
                .withDefault(new C2StreamBitrateInfo::output(0u, 64000))
                .withFields({C2F(mBitrate, value).inRange(4096, 12000000)})
                .withSetter(BitrateSetter)
                .build());

        addParameter(
                DefineParam(mSyncFramePeriod, C2_PARAMKEY_SYNC_FRAME_INTERVAL)
                .withDefault(new C2StreamSyncFrameIntervalTuning::output(0u, 1000000))
                .withFields({C2F(mSyncFramePeriod, value).any()})
                .withSetter(Setter<decltype(*mSyncFramePeriod)>::StrictValueWithNoDeps)
                .build());

#ifdef MPEG4
        addParameter(
                DefineParam(mProfileLevel, C2_PARAMKEY_PROFILE_LEVEL)
                .withDefault(new C2StreamProfileLevelInfo::output(
                        0u, PROFILE_MP4V_SIMPLE, LEVEL_MP4V_2))
                .withFields({
                    C2F(mProfileLevel, profile).equalTo(
                            PROFILE_MP4V_SIMPLE),
                    C2F(mProfileLevel, level).oneOf({
                            C2Config::LEVEL_MP4V_0,
                            C2Config::LEVEL_MP4V_0B,
                            C2Config::LEVEL_MP4V_1,
                            C2Config::LEVEL_MP4V_2})
                })
                .withSetter(ProfileLevelSetter)
                .build());
#else
        addParameter(
                DefineParam(mProfileLevel, C2_PARAMKEY_PROFILE_LEVEL)
                .withDefault(new C2StreamProfileLevelInfo::output(
                        0u, PROFILE_H263_BASELINE, LEVEL_H263_45))
                .withFields({
                    C2F(mProfileLevel, profile).equalTo(
                            PROFILE_H263_BASELINE),
                    C2F(mProfileLevel, level).oneOf({
                            C2Config::LEVEL_H263_10,
                            C2Config::LEVEL_H263_20,
                            C2Config::LEVEL_H263_30,
                            C2Config::LEVEL_H263_40,
                            C2Config::LEVEL_H263_45})
                })
                .withSetter(ProfileLevelSetter)
                .build());
#endif
    }

    static C2R BitrateSetter(bool mayBlock, C2P<C2StreamBitrateInfo::output> &me) {
        (void)mayBlock;
        C2R res = C2R::Ok();
        if (me.v.value <= 4096) {
            me.set().value = 4096;
        }
        return res;
    }

    static C2R SizeSetter(bool mayBlock, const C2P<C2StreamPictureSizeInfo::input> &oldMe,
                          C2P<C2StreamPictureSizeInfo::input> &me) {
        (void)mayBlock;
        C2R res = C2R::Ok();
        if (!me.F(me.v.width).supportsAtAll(me.v.width)) {
            res = res.plus(C2SettingResultBuilder::BadValue(me.F(me.v.width)));
            me.set().width = oldMe.v.width;
        }
        if (!me.F(me.v.height).supportsAtAll(me.v.height)) {
            res = res.plus(C2SettingResultBuilder::BadValue(me.F(me.v.height)));
            me.set().height = oldMe.v.height;
        }
        return res;
    }

    static C2R ProfileLevelSetter(
            bool mayBlock,
            C2P<C2StreamProfileLevelInfo::output> &me) {
        (void)mayBlock;
        if (!me.F(me.v.profile).supportsAtAll(me.v.profile)) {
#ifdef MPEG4
            me.set().profile = PROFILE_MP4V_SIMPLE;
#else
            me.set().profile = PROFILE_H263_BASELINE;
#endif
        }
        if (!me.F(me.v.level).supportsAtAll(me.v.level)) {
#ifdef MPEG4
            me.set().level = LEVEL_MP4V_2;
#else
            me.set().level = LEVEL_H263_45;
#endif
        }
        return C2R::Ok();
    }

    // unsafe getters
    std::shared_ptr<C2StreamPictureSizeInfo::input> getSize_l() const { return mSize; }
    std::shared_ptr<C2StreamFrameRateInfo::output> getFrameRate_l() const { return mFrameRate; }
    std::shared_ptr<C2StreamBitrateInfo::output> getBitrate_l() const { return mBitrate; }
    uint32_t getSyncFramePeriod() const {
        if (mSyncFramePeriod->value < 0 || mSyncFramePeriod->value == INT64_MAX) {
            return 0;
        }
        double period = mSyncFramePeriod->value / 1e6 * mFrameRate->value;
        return (uint32_t)c2_max(c2_min(period + 0.5, double(UINT32_MAX)), 1.);
    }

   private:
    std::shared_ptr<C2StreamUsageTuning::input> mUsage;
    std::shared_ptr<C2StreamPictureSizeInfo::input> mSize;
    std::shared_ptr<C2StreamFrameRateInfo::output> mFrameRate;
    std::shared_ptr<C2StreamBitrateInfo::output> mBitrate;
    std::shared_ptr<C2StreamProfileLevelInfo::output> mProfileLevel;
    std::shared_ptr<C2StreamSyncFrameIntervalTuning::output> mSyncFramePeriod;
};

C2SoftMpeg4Enc::C2SoftMpeg4Enc(const char* name, c2_node_id_t id,
                               const std::shared_ptr<IntfImpl>& intfImpl)
    : SimpleC2Component(
          std::make_shared<SimpleInterface<IntfImpl>>(name, id, intfImpl)),
      mIntf(intfImpl),
      mHandle(nullptr),
      mEncParams(nullptr),
      mStarted(false),
      mOutBufferSize(524288) {
}

C2SoftMpeg4Enc::~C2SoftMpeg4Enc() {
    onRelease();
}

c2_status_t C2SoftMpeg4Enc::onInit() {
#ifdef MPEG4
    mEncodeMode = COMBINE_MODE_WITH_ERR_RES;
#else
    mEncodeMode = H263_MODE;
#endif
    if (!mHandle) {
        mHandle = new tagvideoEncControls;
    }

    if (!mEncParams) {
        mEncParams = new tagvideoEncOptions;
    }

    if (!(mEncParams && mHandle)) return C2_NO_MEMORY;

    mSignalledOutputEos = false;
    mSignalledError = false;

    return initEncoder();
}

c2_status_t C2SoftMpeg4Enc::onStop() {
    if (!mStarted) {
        return C2_OK;
    }
    if (mHandle) {
        (void)PVCleanUpVideoEncoder(mHandle);
    }
    mStarted = false;
    mSignalledOutputEos = false;
    mSignalledError = false;
    return C2_OK;
}

void C2SoftMpeg4Enc::onReset() {
    onStop();
    initEncoder();
}

void C2SoftMpeg4Enc::onRelease() {
    onStop();
    if (mEncParams) {
        delete mEncParams;
        mEncParams = nullptr;
    }
    if (mHandle) {
        delete mHandle;
        mHandle = nullptr;
    }
}

c2_status_t C2SoftMpeg4Enc::onFlush_sm() {
    return C2_OK;
}

static void fillEmptyWork(const std::unique_ptr<C2Work> &work) {
    uint32_t flags = 0;
    if (work->input.flags & C2FrameData::FLAG_END_OF_STREAM) {
        flags |= C2FrameData::FLAG_END_OF_STREAM;
        ALOGV("signalling eos");
    }
    work->worklets.front()->output.flags = (C2FrameData::flags_t)flags;
    work->worklets.front()->output.buffers.clear();
    work->worklets.front()->output.ordinal = work->input.ordinal;
    work->workletsProcessed = 1u;
}

c2_status_t C2SoftMpeg4Enc::initEncParams() {
    if (mHandle) {
        memset(mHandle, 0, sizeof(tagvideoEncControls));
    } else return C2_CORRUPTED;
    if (mEncParams) {
        memset(mEncParams, 0, sizeof(tagvideoEncOptions));
    } else return C2_CORRUPTED;

    if (!PVGetDefaultEncOption(mEncParams, 0)) {
        ALOGE("Failed to get default encoding parameters");
        return C2_CORRUPTED;
    }

    if (mFrameRate->value == 0) {
        ALOGE("Framerate should not be 0");
        return C2_BAD_VALUE;
    }

    mEncParams->encMode = mEncodeMode;
    mEncParams->encWidth[0] = mSize->width;
    mEncParams->encHeight[0] = mSize->height;
    mEncParams->encFrameRate[0] = mFrameRate->value + 0.5;
    mEncParams->rcType = VBR_1;
    mEncParams->vbvDelay = 5.0f;
    mEncParams->profile_level = CORE_PROFILE_LEVEL2;
    mEncParams->packetSize = 32;
    mEncParams->rvlcEnable = PV_OFF;
    mEncParams->numLayers = 1;
    mEncParams->timeIncRes = 1000;
    mEncParams->tickPerSrc = mEncParams->timeIncRes / (mFrameRate->value + 0.5);
    mEncParams->bitRate[0] = mBitrate->value;
    mEncParams->iQuant[0] = 15;
    mEncParams->pQuant[0] = 12;
    mEncParams->quantType[0] = 0;
    mEncParams->noFrameSkipped = PV_OFF;

    // PV's MPEG4 encoder requires the video dimension of multiple
    if (mSize->width % 16 != 0 || mSize->height % 16 != 0) {
        ALOGE("Video frame size %dx%d must be a multiple of 16",
              mSize->width, mSize->height);
        return C2_BAD_VALUE;
    }

    // Set IDR frame refresh interval
    mEncParams->intraPeriod = mIntf->getSyncFramePeriod();
    mEncParams->numIntraMB = 0;
    mEncParams->sceneDetect = PV_ON;
    mEncParams->searchRange = 16;
    mEncParams->mv8x8Enable = PV_OFF;
    mEncParams->gobHeaderInterval = 0;
    mEncParams->useACPred = PV_ON;
    mEncParams->intraDCVlcTh = 0;

    return C2_OK;
}

c2_status_t C2SoftMpeg4Enc::initEncoder() {
    if (mStarted) {
        return C2_OK;
    }
    {
        IntfImpl::Lock lock = mIntf->lock();
        mSize = mIntf->getSize_l();
        mBitrate = mIntf->getBitrate_l();
        mFrameRate = mIntf->getFrameRate_l();
    }
    c2_status_t err = initEncParams();
    if (C2_OK != err) {
        ALOGE("Failed to initialized encoder params");
        mSignalledError = true;
        return err;
    }
    if (!PVInitVideoEncoder(mHandle, mEncParams)) {
        ALOGE("Failed to initialize the encoder");
        mSignalledError = true;
        return C2_CORRUPTED;
    }

    // 1st buffer for codec specific data
    mNumInputFrames = -1;
    mStarted = true;
    return C2_OK;
}

void C2SoftMpeg4Enc::process(
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

    // Initialize encoder if not already initialized
    if (!mStarted && C2_OK != initEncoder()) {
        ALOGE("Failed to initialize encoder");
        mSignalledError = true;
        work->result = C2_CORRUPTED;
        return;
    }

    std::shared_ptr<C2LinearBlock> block;
    C2MemoryUsage usage = { C2MemoryUsage::CPU_READ, C2MemoryUsage::CPU_WRITE };
    c2_status_t err = pool->fetchLinearBlock(mOutBufferSize, usage, &block);
    if (err != C2_OK) {
        ALOGE("fetchLinearBlock for Output failed with status %d", err);
        work->result = C2_NO_MEMORY;
        return;
    }

    C2WriteView wView = block->map().get();
    if (wView.error()) {
        ALOGE("write view map failed %d", wView.error());
        work->result = wView.error();
        return;
    }

    uint8_t *outPtr = (uint8_t *)wView.data();
    if (mNumInputFrames < 0) {
        // The very first thing we want to output is the codec specific data.
        int32_t outputSize = mOutBufferSize;
        if (!PVGetVolHeader(mHandle, outPtr, &outputSize, 0)) {
            ALOGE("Failed to get VOL header");
            mSignalledError = true;
            work->result = C2_CORRUPTED;
            return;
        } else {
            ALOGV("Bytes Generated in header %d\n", outputSize);
        }

        ++mNumInputFrames;
        std::unique_ptr<C2StreamInitDataInfo::output> csd =
            C2StreamInitDataInfo::output::AllocUnique(outputSize, 0u);
        if (!csd) {
            ALOGE("CSD allocation failed");
            mSignalledError = true;
            work->result = C2_NO_MEMORY;
            return;
        }
        memcpy(csd->m.value, outPtr, outputSize);
        work->worklets.front()->output.configUpdate.push_back(std::move(csd));
    }

    std::shared_ptr<const C2GraphicView> rView;
    std::shared_ptr<C2Buffer> inputBuffer;
    bool eos = ((work->input.flags & C2FrameData::FLAG_END_OF_STREAM) != 0);
    if (!work->input.buffers.empty()) {
        inputBuffer = work->input.buffers[0];
        rView = std::make_shared<const C2GraphicView>(
                inputBuffer->data().graphicBlocks().front().map().get());
        if (rView->error() != C2_OK) {
            ALOGE("graphic view map err = %d", rView->error());
            work->result = rView->error();
            return;
        }
    } else {
        fillEmptyWork(work);
        if (eos) {
            mSignalledOutputEos = true;
            ALOGV("signalled EOS");
        }
        return;
    }

    uint64_t inputTimeStamp = work->input.ordinal.timestamp.peekull();
    const C2ConstGraphicBlock inBuffer = inputBuffer->data().graphicBlocks().front();
    if (inBuffer.width() < mSize->width ||
        inBuffer.height() < mSize->height) {
        /* Expect width height to be configured */
        ALOGW("unexpected Capacity Aspect %d(%d) x %d(%d)", inBuffer.width(),
              mSize->width, inBuffer.height(), mSize->height);
        work->result = C2_BAD_VALUE;
        return;
    }

    const C2PlanarLayout &layout = rView->layout();
    uint8_t *yPlane = const_cast<uint8_t *>(rView->data()[C2PlanarLayout::PLANE_Y]);
    uint8_t *uPlane = const_cast<uint8_t *>(rView->data()[C2PlanarLayout::PLANE_U]);
    uint8_t *vPlane = const_cast<uint8_t *>(rView->data()[C2PlanarLayout::PLANE_V]);
    int32_t yStride = layout.planes[C2PlanarLayout::PLANE_Y].rowInc;
    int32_t uStride = layout.planes[C2PlanarLayout::PLANE_U].rowInc;
    int32_t vStride = layout.planes[C2PlanarLayout::PLANE_V].rowInc;
    uint32_t width = mSize->width;
    uint32_t height = mSize->height;
    // width and height are always even (as block size is 16x16)
    CHECK_EQ((width & 1u), 0u);
    CHECK_EQ((height & 1u), 0u);
    size_t yPlaneSize = width * height;
    switch (layout.type) {
        case C2PlanarLayout::TYPE_RGB:
            [[fallthrough]];
        case C2PlanarLayout::TYPE_RGBA: {
            MemoryBlock conversionBuffer = mConversionBuffers.fetch(yPlaneSize * 3 / 2);
            mConversionBuffersInUse.emplace(conversionBuffer.data(), conversionBuffer);
            yPlane = conversionBuffer.data();
            uPlane = yPlane + yPlaneSize;
            vPlane = uPlane + yPlaneSize / 4;
            yStride = width;
            uStride = vStride = width / 2;
            ConvertRGBToPlanarYUV(yPlane, yStride, height, conversionBuffer.size(), *rView.get());
            break;
        }
        case C2PlanarLayout::TYPE_YUV: {
            if (!IsYUV420(*rView)) {
                ALOGE("input is not YUV420");
                work->result = C2_BAD_VALUE;
                break;
            }

            if (layout.planes[layout.PLANE_Y].colInc == 1
                    && layout.planes[layout.PLANE_U].colInc == 1
                    && layout.planes[layout.PLANE_V].colInc == 1
                    && yStride == align(width, 16)
                    && uStride == vStride
                    && yStride == 2 * vStride) {
                // I420 compatible with yStride being equal to aligned width
                // planes are already set up above
                break;
            }

            // copy to I420
            MemoryBlock conversionBuffer = mConversionBuffers.fetch(yPlaneSize * 3 / 2);
            mConversionBuffersInUse.emplace(conversionBuffer.data(), conversionBuffer);
            MediaImage2 img = CreateYUV420PlanarMediaImage2(width, height, width, height);
            status_t err = ImageCopy(conversionBuffer.data(), &img, *rView);
            if (err != OK) {
                ALOGE("Buffer conversion failed: %d", err);
                work->result = C2_BAD_VALUE;
                return;
            }
            yPlane = conversionBuffer.data();
            uPlane = yPlane + yPlaneSize;
            vPlane = uPlane + yPlaneSize / 4;
            yStride = width;
            uStride = vStride = width / 2;
            break;
        }

        case C2PlanarLayout::TYPE_YUVA:
            ALOGE("YUVA plane type is not supported");
            work->result = C2_BAD_VALUE;
            return;

        default:
            ALOGE("Unrecognized plane type: %d", layout.type);
            work->result = C2_BAD_VALUE;
            return;
    }

    CHECK(NULL != yPlane);
    /* Encode frames */
    VideoEncFrameIO vin, vout;
    memset(&vin, 0, sizeof(vin));
    memset(&vout, 0, sizeof(vout));
    vin.yChan = yPlane;
    vin.uChan = uPlane;
    vin.vChan = vPlane;
    vin.timestamp = (inputTimeStamp + 500) / 1000;  // in ms
    vin.height = align(height, 16);
    vin.pitch = align(width, 16);

    uint32_t modTimeMs = 0;
    int32_t nLayer = 0;
    MP4HintTrack hintTrack;
    int32_t outputSize = mOutBufferSize;
    if (!PVEncodeVideoFrame(mHandle, &vin, &vout, &modTimeMs, outPtr, &outputSize, &nLayer) ||
        !PVGetHintTrack(mHandle, &hintTrack)) {
        ALOGE("Failed to encode frame or get hint track at frame %" PRId64, mNumInputFrames);
        mSignalledError = true;
        work->result = C2_CORRUPTED;
        return;
    }
    ALOGV("outputSize filled : %d", outputSize);
    ++mNumInputFrames;
    CHECK(NULL == PVGetOverrunBuffer(mHandle));

    fillEmptyWork(work);
    if (outputSize) {
        std::shared_ptr<C2Buffer> buffer = createLinearBuffer(block, 0, outputSize);
        work->worklets.front()->output.ordinal.timestamp = inputTimeStamp;
        if (hintTrack.CodeType == 0) {
            buffer->setInfo(std::make_shared<C2StreamPictureTypeMaskInfo::output>(
                    0u /* stream id */, C2Config::SYNC_FRAME));
        }
        work->worklets.front()->output.buffers.push_back(buffer);
    }
    if (eos) {
        mSignalledOutputEos = true;
    }

    mConversionBuffersInUse.erase(yPlane);
}

c2_status_t C2SoftMpeg4Enc::drain(
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

class C2SoftMpeg4EncFactory : public C2ComponentFactory {
public:
    C2SoftMpeg4EncFactory()
        : mHelper(std::static_pointer_cast<C2ReflectorHelper>(
              GetCodec2PlatformComponentStore()->getParamReflector())) {}

    virtual c2_status_t createComponent(
            c2_node_id_t id,
            std::shared_ptr<C2Component>* const component,
            std::function<void(C2Component*)> deleter) override {
        *component = std::shared_ptr<C2Component>(
            new C2SoftMpeg4Enc(
                COMPONENT_NAME, id,
                std::make_shared<C2SoftMpeg4Enc::IntfImpl>(mHelper)),
            deleter);
        return C2_OK;
    }

    virtual c2_status_t createInterface(
            c2_node_id_t id,
            std::shared_ptr<C2ComponentInterface>* const interface,
            std::function<void(C2ComponentInterface*)> deleter) override {
        *interface = std::shared_ptr<C2ComponentInterface>(
            new SimpleInterface<C2SoftMpeg4Enc::IntfImpl>(
                COMPONENT_NAME, id,
                std::make_shared<C2SoftMpeg4Enc::IntfImpl>(mHelper)),
            deleter);
        return C2_OK;
    }

    virtual ~C2SoftMpeg4EncFactory() override = default;

private:
    std::shared_ptr<C2ReflectorHelper> mHelper;
};

}  // namespace android

extern "C" ::C2ComponentFactory* CreateCodec2Factory() {
    ALOGV("in %s", __func__);
    return new ::android::C2SoftMpeg4EncFactory();
}

extern "C" void DestroyCodec2Factory(::C2ComponentFactory* factory) {
    ALOGV("in %s", __func__);
    delete factory;
}
