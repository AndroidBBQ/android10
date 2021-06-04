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

#ifndef ANDROID_C2_SOFT_VPX_ENC_H__
#define ANDROID_C2_SOFT_VPX_ENC_H__

#include <media/stagefright/foundation/MediaDefs.h>

#include <C2PlatformSupport.h>
#include <Codec2BufferUtils.h>
#include <SimpleC2Component.h>
#include <SimpleC2Interface.h>
#include <util/C2InterfaceHelper.h>

#include "vpx/vpx_encoder.h"
#include "vpx/vpx_codec.h"
#include "vpx/vpx_image.h"
#include "vpx/vp8cx.h"

namespace android {

// TODO: These defs taken from deprecated OMX_VideoExt.h. Move these definitions
// to a new header file and include it.

/** Maximum number of temporal layers */
#define MAXTEMPORALLAYERS 3

/** temporal layer patterns */
typedef enum TemporalPatternType {
    VPXTemporalLayerPatternNone = 0,
    VPXTemporalLayerPatternWebRTC = 1,
    VPXTemporalLayerPatternMax = 0x7FFFFFFF
} TemporalPatternType;

// Base class for a VPX Encoder Component
//
// Only following encoder settings are available (codec specific settings might
// be available in the sub-classes):
//    - video resolution
//    - target bitrate
//    - rate control (constant / variable)
//    - frame rate
//    - error resilience
//    - reconstruction & loop filters (g_profile)
//
// Only following color formats are recognized
//    - C2PlanarLayout::TYPE_RGB
//    - C2PlanarLayout::TYPE_RGBA
//
// Following settings are not configurable by the client
//    - encoding deadline is realtime
//    - multithreaded encoding utilizes a number of threads equal
// to online cpu's available
//    - the algorithm interface for encoder is decided by the sub-class in use
//    - fractional bits of frame rate is discarded
//    - timestamps are in microseconds, therefore encoder timebase is fixed
// to 1/1000000

struct C2SoftVpxEnc : public SimpleC2Component {
    class IntfImpl;

    C2SoftVpxEnc(const char* name, c2_node_id_t id,
                 const std::shared_ptr<IntfImpl>& intfImpl);

    // From SimpleC2Component
    c2_status_t onInit() override final;
    c2_status_t onStop() override final;
    void onReset() override final;
    void onRelease() override final;
    c2_status_t onFlush_sm() override final;

    void process(
            const std::unique_ptr<C2Work> &work,
            const std::shared_ptr<C2BlockPool> &pool) override final;
    c2_status_t drain(
            uint32_t drainMode,
            const std::shared_ptr<C2BlockPool> &pool) override final;

 protected:
     std::shared_ptr<IntfImpl> mIntf;
     virtual ~C2SoftVpxEnc();

     // Initializes vpx encoder with available settings.
     status_t initEncoder();

     // Populates mCodecInterface with codec specific settings.
     virtual void setCodecSpecificInterface() = 0;

     // Sets codec specific configuration.
     virtual void setCodecSpecificConfiguration() = 0;

     // Sets codec specific encoder controls.
     virtual vpx_codec_err_t setCodecSpecificControls() = 0;

     // Get current encode flags.
     virtual vpx_enc_frame_flags_t getEncodeFlags();

     enum TemporalReferences {
         // For 1 layer case: reference all (last, golden, and alt ref), but only
         // update last.
         kTemporalUpdateLastRefAll = 12,
         // First base layer frame for 3 temporal layers, which updates last and
         // golden with alt ref dependency.
         kTemporalUpdateLastAndGoldenRefAltRef = 11,
         // First enhancement layer with alt ref dependency.
         kTemporalUpdateGoldenRefAltRef = 10,
         // First enhancement layer with alt ref dependency.
         kTemporalUpdateGoldenWithoutDependencyRefAltRef = 9,
         // Base layer with alt ref dependency.
         kTemporalUpdateLastRefAltRef = 8,
         // Highest enhacement layer without dependency on golden with alt ref
         // dependency.
         kTemporalUpdateNoneNoRefGoldenRefAltRef = 7,
         // Second layer and last frame in cycle, for 2 layers.
         kTemporalUpdateNoneNoRefAltref = 6,
         // Highest enhancement layer.
         kTemporalUpdateNone = 5,
         // Second enhancement layer.
         kTemporalUpdateAltref = 4,
         // Second enhancement layer without dependency on previous frames in
         // the second enhancement layer.
         kTemporalUpdateAltrefWithoutDependency = 3,
         // First enhancement layer.
         kTemporalUpdateGolden = 2,
         // First enhancement layer without dependency on previous frames in
         // the first enhancement layer.
         kTemporalUpdateGoldenWithoutDependency = 1,
         // Base layer.
         kTemporalUpdateLast = 0,
     };
     enum {
         kMaxTemporalPattern = 8
     };

     // vpx specific opaque data structure that
     // stores encoder state
     vpx_codec_ctx_t* mCodecContext;

     // vpx specific data structure that
     // stores encoder configuration
     vpx_codec_enc_cfg_t* mCodecConfiguration;

     // vpx specific read-only data structure
     // that specifies algorithm interface (e.g. vp8)
     vpx_codec_iface_t* mCodecInterface;

     // align stride to the power of 2
     int32_t mStrideAlign;

     // Color format for the input port
     vpx_img_fmt_t mColorFormat;

     // Bitrate control mode, either constant or variable
     vpx_rc_mode mBitrateControlMode;

     // Parameter that denotes whether error resilience
     // is enabled in encoder
     bool mErrorResilience;

     // Minimum (best quality) quantizer
     uint32_t mMinQuantizer;

     // Maximum (worst quality) quantizer
     uint32_t mMaxQuantizer;

     // Number of coding temporal layers to be used.
     size_t mTemporalLayers;

     // Temporal layer bitrare ratio in percentage
     uint32_t mTemporalLayerBitrateRatio[MAXTEMPORALLAYERS];

     // Temporal pattern type
     TemporalPatternType mTemporalPatternType;

     // Temporal pattern length
     size_t mTemporalPatternLength;

     // Temporal pattern current index
     size_t mTemporalPatternIdx;

     // Frame type temporal pattern
     TemporalReferences mTemporalPattern[kMaxTemporalPattern];

     // Last input buffer timestamp
     uint64_t mLastTimestamp;

     // Number of input frames
     int64_t mNumInputFrames;

     // Conversion buffer is needed to input to
     // yuv420 planar format.
     MemoryBlock mConversionBuffer;

     // Signalled EOS
     bool mSignalledOutputEos;

     // Signalled Error
     bool mSignalledError;

    // configurations used by component in process
    // (TODO: keep this in intf but make them internal only)
    std::shared_ptr<C2StreamPictureSizeInfo::input> mSize;
    std::shared_ptr<C2StreamIntraRefreshTuning::output> mIntraRefresh;
    std::shared_ptr<C2StreamFrameRateInfo::output> mFrameRate;
    std::shared_ptr<C2StreamBitrateInfo::output> mBitrate;
    std::shared_ptr<C2StreamBitrateModeTuning::output> mBitrateMode;
    std::shared_ptr<C2StreamRequestSyncFrameTuning::output> mRequestSync;

     C2_DO_NOT_COPY(C2SoftVpxEnc);
};

namespace {

#ifdef VP9
constexpr char COMPONENT_NAME[] = "c2.android.vp9.encoder";
const char *MEDIA_MIMETYPE_VIDEO = MEDIA_MIMETYPE_VIDEO_VP9;
#else
constexpr char COMPONENT_NAME[] = "c2.android.vp8.encoder";
const char *MEDIA_MIMETYPE_VIDEO = MEDIA_MIMETYPE_VIDEO_VP8;
#endif

} // namepsace

class C2SoftVpxEnc::IntfImpl : public SimpleInterface<void>::BaseParams {
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
                .withDefault(new C2StreamPictureSizeInfo::input(0u, 320, 240))
                .withFields({
                    C2F(mSize, width).inRange(2, 2048, 2),
                    C2F(mSize, height).inRange(2, 2048, 2),
                })
                .withSetter(SizeSetter)
                .build());

        addParameter(
            DefineParam(mBitrateMode, C2_PARAMKEY_BITRATE_MODE)
                .withDefault(new C2StreamBitrateModeTuning::output(
                        0u, C2Config::BITRATE_VARIABLE))
                .withFields({
                    C2F(mBitrateMode, value).oneOf({
                        C2Config::BITRATE_CONST, C2Config::BITRATE_VARIABLE })
                })
                .withSetter(
                    Setter<decltype(*mBitrateMode)>::StrictValueWithNoDeps)
                .build());

        addParameter(
            DefineParam(mFrameRate, C2_PARAMKEY_FRAME_RATE)
                .withDefault(new C2StreamFrameRateInfo::output(0u, 30.))
                // TODO: More restriction?
                .withFields({C2F(mFrameRate, value).greaterThan(0.)})
                .withSetter(
                    Setter<decltype(*mFrameRate)>::StrictValueWithNoDeps)
                .build());

        addParameter(
            DefineParam(mLayering, C2_PARAMKEY_TEMPORAL_LAYERING)
                .withDefault(C2StreamTemporalLayeringTuning::output::AllocShared(0u, 0, 0, 0))
                .withFields({
                    C2F(mLayering, m.layerCount).inRange(0, 4),
                    C2F(mLayering, m.bLayerCount).inRange(0, 0),
                    C2F(mLayering, m.bitrateRatios).inRange(0., 1.)
                })
                .withSetter(LayeringSetter)
                .build());

        addParameter(
                DefineParam(mSyncFramePeriod, C2_PARAMKEY_SYNC_FRAME_INTERVAL)
                .withDefault(new C2StreamSyncFrameIntervalTuning::output(0u, 1000000))
                .withFields({C2F(mSyncFramePeriod, value).any()})
                .withSetter(Setter<decltype(*mSyncFramePeriod)>::StrictValueWithNoDeps)
                .build());

        addParameter(
            DefineParam(mBitrate, C2_PARAMKEY_BITRATE)
                .withDefault(new C2StreamBitrateInfo::output(0u, 64000))
                .withFields({C2F(mBitrate, value).inRange(4096, 40000000)})
                .withSetter(BitrateSetter)
                .build());

        addParameter(
                DefineParam(mIntraRefresh, C2_PARAMKEY_INTRA_REFRESH)
                .withConstValue(new C2StreamIntraRefreshTuning::output(
                             0u, C2Config::INTRA_REFRESH_DISABLED, 0.))
                .build());

        addParameter(
        DefineParam(mProfileLevel, C2_PARAMKEY_PROFILE_LEVEL)
        .withDefault(new C2StreamProfileLevelInfo::output(
                0u, PROFILE_VP9_0, LEVEL_VP9_4_1))
        .withFields({
            C2F(mProfileLevel, profile).equalTo(
                PROFILE_VP9_0
            ),
            C2F(mProfileLevel, level).equalTo(
                LEVEL_VP9_4_1),
        })
        .withSetter(ProfileLevelSetter)
        .build());

        addParameter(
                DefineParam(mRequestSync, C2_PARAMKEY_REQUEST_SYNC_FRAME)
                .withDefault(new C2StreamRequestSyncFrameTuning::output(0u, C2_FALSE))
                .withFields({C2F(mRequestSync, value).oneOf({ C2_FALSE, C2_TRUE }) })
                .withSetter(Setter<decltype(*mRequestSync)>::NonStrictValueWithNoDeps)
                .build());
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
            me.set().profile = PROFILE_VP9_0;
        }
        if (!me.F(me.v.level).supportsAtAll(me.v.level)) {
            me.set().level = LEVEL_VP9_4_1;
        }
        return C2R::Ok();
    }

    static C2R LayeringSetter(bool mayBlock, C2P<C2StreamTemporalLayeringTuning::output>& me) {
        (void)mayBlock;
        C2R res = C2R::Ok();
        if (me.v.m.layerCount > 4) {
            me.set().m.layerCount = 4;
        }
        me.set().m.bLayerCount = 0;
        // ensure ratios are monotonic and clamped between 0 and 1
        for (size_t ix = 0; ix < me.v.flexCount(); ++ix) {
            me.set().m.bitrateRatios[ix] = c2_clamp(
                ix > 0 ? me.v.m.bitrateRatios[ix - 1] : 0, me.v.m.bitrateRatios[ix], 1.);
        }
        ALOGI("setting temporal layering %u + %u", me.v.m.layerCount, me.v.m.bLayerCount);
        return res;
    }

    // unsafe getters
    std::shared_ptr<C2StreamPictureSizeInfo::input> getSize_l() const { return mSize; }
    std::shared_ptr<C2StreamIntraRefreshTuning::output> getIntraRefresh_l() const { return mIntraRefresh; }
    std::shared_ptr<C2StreamFrameRateInfo::output> getFrameRate_l() const { return mFrameRate; }
    std::shared_ptr<C2StreamBitrateInfo::output> getBitrate_l() const { return mBitrate; }
    std::shared_ptr<C2StreamBitrateModeTuning::output> getBitrateMode_l() const { return mBitrateMode; }
    std::shared_ptr<C2StreamRequestSyncFrameTuning::output> getRequestSync_l() const { return mRequestSync; }
    std::shared_ptr<C2StreamTemporalLayeringTuning::output> getTemporalLayers_l() const { return mLayering; }
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
    std::shared_ptr<C2StreamTemporalLayeringTuning::output> mLayering;
    std::shared_ptr<C2StreamIntraRefreshTuning::output> mIntraRefresh;
    std::shared_ptr<C2StreamRequestSyncFrameTuning::output> mRequestSync;
    std::shared_ptr<C2StreamSyncFrameIntervalTuning::output> mSyncFramePeriod;
    std::shared_ptr<C2StreamBitrateInfo::output> mBitrate;
    std::shared_ptr<C2StreamBitrateModeTuning::output> mBitrateMode;
    std::shared_ptr<C2StreamProfileLevelInfo::output> mProfileLevel;
};

}  // namespace android

#endif  // ANDROID_C2_SOFT_VPX_ENC_H__
