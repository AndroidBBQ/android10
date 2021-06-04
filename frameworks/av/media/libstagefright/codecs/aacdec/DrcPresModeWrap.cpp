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
#include "DrcPresModeWrap.h"

#include <assert.h>

#define LOG_TAG "SoftAAC2_DrcWrapper"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

//#define DRC_PRES_MODE_WRAP_DEBUG

#define GPM_ENCODER_TARGET_LEVEL 64
#define MAX_TARGET_LEVEL 40

CDrcPresModeWrapper::CDrcPresModeWrapper()
{
    mDataUpdate = true;

    /* Data from streamInfo. */
    /* Initialized to the same values as in the aac decoder */
    mStreamPRL = -1;
    mStreamDRCPresMode = -1;
    mStreamNrAACChan = 0;
    mStreamNrOutChan = 0;

    /* Desired values (set by user). */
    /* Initialized to the same values as in the aac decoder */
    mDesTarget = -1;
    mDesAttFactor = 0;
    mDesBoostFactor = 0;
    mDesHeavy = 0;

    mEncoderTarget = -1;

    /* Values from last time. */
    /* Initialized to the same values as the desired values */
    mLastTarget = -1;
    mLastAttFactor = 0;
    mLastBoostFactor = 0;
    mLastHeavy = 0;
}

CDrcPresModeWrapper::~CDrcPresModeWrapper()
{
}

void
CDrcPresModeWrapper::setDecoderHandle(const HANDLE_AACDECODER handle)
{
    mHandleDecoder = handle;
}

void
CDrcPresModeWrapper::submitStreamData(CStreamInfo* pStreamInfo)
{
    assert(pStreamInfo);

    if (mStreamPRL != pStreamInfo->drcProgRefLev) {
        mStreamPRL = pStreamInfo->drcProgRefLev;
        mDataUpdate = true;
#ifdef DRC_PRES_MODE_WRAP_DEBUG
        ALOGV("DRC presentation mode wrapper: drcProgRefLev is %d\n", mStreamPRL);
#endif
    }

    if (mStreamDRCPresMode != pStreamInfo->drcPresMode) {
        mStreamDRCPresMode = pStreamInfo->drcPresMode;
        mDataUpdate = true;
#ifdef DRC_PRES_MODE_WRAP_DEBUG
        ALOGV("DRC presentation mode wrapper: drcPresMode is %d\n", mStreamDRCPresMode);
#endif
    }

    if (mStreamNrAACChan != pStreamInfo->aacNumChannels) {
        mStreamNrAACChan = pStreamInfo->aacNumChannels;
        mDataUpdate = true;
#ifdef DRC_PRES_MODE_WRAP_DEBUG
        ALOGV("DRC presentation mode wrapper: aacNumChannels is %d\n", mStreamNrAACChan);
#endif
    }

    if (mStreamNrOutChan != pStreamInfo->numChannels) {
        mStreamNrOutChan = pStreamInfo->numChannels;
        mDataUpdate = true;
#ifdef DRC_PRES_MODE_WRAP_DEBUG
        ALOGV("DRC presentation mode wrapper: numChannels is %d\n", mStreamNrOutChan);
#endif
    }



    if (mStreamNrOutChan<mStreamNrAACChan) {
        mIsDownmix = true;
    } else {
        mIsDownmix = false;
    }

    if (mIsDownmix && (mStreamNrOutChan == 1)) {
        mIsMonoDownmix = true;
    } else {
        mIsMonoDownmix = false;
    }

    if (mIsDownmix && mStreamNrOutChan == 2){
        mIsStereoDownmix = true;
    } else {
        mIsStereoDownmix = false;
    }

}

void
CDrcPresModeWrapper::setParam(const DRC_PRES_MODE_WRAP_PARAM param, const int value)
{
    switch (param) {
    case DRC_PRES_MODE_WRAP_DESIRED_TARGET:
        mDesTarget = value;
        break;
    case DRC_PRES_MODE_WRAP_DESIRED_ATT_FACTOR:
        mDesAttFactor = value;
        break;
    case DRC_PRES_MODE_WRAP_DESIRED_BOOST_FACTOR:
        mDesBoostFactor = value;
        break;
    case DRC_PRES_MODE_WRAP_DESIRED_HEAVY:
        mDesHeavy = value;
        break;
    case DRC_PRES_MODE_WRAP_ENCODER_TARGET:
        mEncoderTarget = value;
        break;
    default:
        break;
    }
    mDataUpdate = true;
}

void
CDrcPresModeWrapper::update()
{
    // Get Data from Decoder
    int progRefLevel = mStreamPRL;
    int drcPresMode = mStreamDRCPresMode;

    // by default, do as desired
    int newTarget         = mDesTarget;
    int newAttFactor      = mDesAttFactor;
    int newBoostFactor    = mDesBoostFactor;
    int newHeavy          = mDesHeavy;

    if (mDataUpdate) {
        // sanity check
        if (mDesTarget < MAX_TARGET_LEVEL){
            mDesTarget = MAX_TARGET_LEVEL;  // limit target level to -10 dB or below
            newTarget = MAX_TARGET_LEVEL;
        }

        if (mEncoderTarget != -1) {
            if (mDesTarget<124) { // if target level > -31 dB
                if ((mIsStereoDownmix == false) && (mIsMonoDownmix == false)) {
                    // no stereo or mono downmixing, calculated scaling of light DRC
                    /* use as little compression as possible */
                    newAttFactor = 0;
                    newBoostFactor = 0;
                    if (mDesTarget<progRefLevel) { // if target level > PRL
                        if (mEncoderTarget < mDesTarget) { // if mEncoderTarget > target level
                            // mEncoderTarget > target level > PRL
                            int calcFactor;
                            float calcFactor_norm;
                            // 0.0f < calcFactor_norm < 1.0f
                            calcFactor_norm = (float)(mDesTarget - progRefLevel) /
                                    (float)(mEncoderTarget - progRefLevel);
                            calcFactor = (int)(calcFactor_norm*127.0f); // 0 <= calcFactor < 127
                            // calcFactor is the lower limit
                            newAttFactor = (calcFactor>newAttFactor) ? calcFactor : newAttFactor;
                            // new AttFactor will be always = calcFactor, as it is set to 0 before.
                            newBoostFactor = newAttFactor;
                        } else {
                            /* target level > mEncoderTarget > PRL */
                            // newTDLimiterEnable = 1;
                            // the time domain limiter must always be active in this case.
                            //     It is assumed that the framework activates it by default
                            newAttFactor = 127;
                            newBoostFactor = 127;
                        }
                    } else { // target level <= PRL
                        // no restrictions required
                        // newAttFactor = newAttFactor;
                    }
                } else { // downmixing
                    // if target level > -23 dB or mono downmix
                    if ( (mDesTarget<92) || mIsMonoDownmix ) {
                        newHeavy = 1;
                    } else {
                        // we perform a downmix, so, we need at least full light DRC
                        newAttFactor = 127;
                    }
                }
            } else { // target level <= -31 dB
                // playback -31 dB: light DRC only needed if we perform downmixing
                if (mIsDownmix) {   // we do downmixing
                    newAttFactor = 127;
                }
            }
        }
        else { // handle other used encoder target levels

            // Sanity check: DRC presentation mode is only specified for max. 5.1 channels
            if (mStreamNrAACChan > 6) {
                drcPresMode = 0;
            }

            switch (drcPresMode) {
            case 0:
            default: // presentation mode not indicated
            {

                if (mDesTarget<124) { // if target level > -31 dB
                    // no stereo or mono downmixing
                    if ((mIsStereoDownmix == false) && (mIsMonoDownmix == false)) {
                        if (mDesTarget<progRefLevel) { // if target level > PRL
                            // newTDLimiterEnable = 1;
                            // the time domain limiter must always be active in this case.
                            //    It is assumed that the framework activates it by default
                            newAttFactor = 127; // at least, use light compression
                        } else { // target level <= PRL
                            // no restrictions required
                            // newAttFactor = newAttFactor;
                        }
                    } else { // downmixing
                        // newTDLimiterEnable = 1;
                        // the time domain limiter must always be active in this case.
                        //    It is assumed that the framework activates it by default

                        // if target level > -23 dB or mono downmix
                        if ( (mDesTarget < 92) || mIsMonoDownmix ) {
                            newHeavy = 1;
                        } else{
                            // we perform a downmix, so, we need at least full light DRC
                            newAttFactor = 127;
                        }
                    }
                } else { // target level <= -31 dB
                    if (mIsDownmix) {   // we do downmixing.
                        // newTDLimiterEnable = 1;
                        // the time domain limiter must always be active in this case.
                        //    It is assumed that the framework activates it by default
                        newAttFactor = 127;
                    }
                }
            }
            break;

            // Presentation mode 1 and 2 according to ETSI TS 101 154:
            // Digital Video Broadcasting (DVB); Specification for the use of Video and Audio Coding
            // in Broadcasting Applications based on the MPEG-2 Transport Stream,
            // section C.5.4., "Decoding", and Table C.33
            // ISO DRC            -> newHeavy = 0  (Use light compression, MPEG-style)
            // Compression_value  -> newHeavy = 1  (Use heavy compression, DVB-style)
            // scaling restricted -> newAttFactor = 127

            case 1: // presentation mode 1, Light:-31/Heavy:-23
            {
                if (mDesTarget < 124) { // if target level > -31 dB
                    // playback up to -23 dB
                    newHeavy = 1;
                } else { // target level <= -31 dB
                    // playback -31 dB
                    if (mIsDownmix) {   // we do downmixing.
                        newAttFactor = 127;
                    }
                }
            }
            break;

            case 2: // presentation mode 2, Light:-23/Heavy:-23
            {
                if (mDesTarget < 124) { // if target level > -31 dB
                    // playback up to -23 dB
                    if (mIsMonoDownmix) { // if mono downmix
                        newHeavy = 1;
                    } else {
                        newHeavy = 0;
                        newAttFactor = 127;
                    }
                } else { // target level <= -31 dB
                    // playback -31 dB
                    newHeavy = 0;
                    if (mIsDownmix) {   // we do downmixing.
                        newAttFactor = 127;
                    }
                }
            }
            break;

            } // switch()
        } // if (mEncoderTarget  == GPM_ENCODER_TARGET_LEVEL)

        // sanity again
        if (newHeavy == 1) {
            newBoostFactor=127; // not really needed as the same would be done by the decoder anyway
            newAttFactor = 127;
        }

        // update the decoder
        if (newTarget != mLastTarget) {
            aacDecoder_SetParam(mHandleDecoder, AAC_DRC_REFERENCE_LEVEL, newTarget);
            mLastTarget = newTarget;
#ifdef DRC_PRES_MODE_WRAP_DEBUG
            if (newTarget != mDesTarget)
                ALOGV("DRC presentation mode wrapper: forced target level to %d (from %d)\n", newTarget, mDesTarget);
            else
                ALOGV("DRC presentation mode wrapper: set target level to %d\n", newTarget);
#endif
        }

        if (newAttFactor != mLastAttFactor) {
            aacDecoder_SetParam(mHandleDecoder, AAC_DRC_ATTENUATION_FACTOR, newAttFactor);
            mLastAttFactor = newAttFactor;
#ifdef DRC_PRES_MODE_WRAP_DEBUG
            if (newAttFactor != mDesAttFactor)
                ALOGV("DRC presentation mode wrapper: forced attenuation factor to %d (from %d)\n", newAttFactor, mDesAttFactor);
            else
                ALOGV("DRC presentation mode wrapper: set attenuation factor to %d\n", newAttFactor);
#endif
        }

        if (newBoostFactor != mLastBoostFactor) {
            aacDecoder_SetParam(mHandleDecoder, AAC_DRC_BOOST_FACTOR, newBoostFactor);
            mLastBoostFactor = newBoostFactor;
#ifdef DRC_PRES_MODE_WRAP_DEBUG
            if (newBoostFactor != mDesBoostFactor)
                ALOGV("DRC presentation mode wrapper: forced boost factor to %d (from %d)\n",
                        newBoostFactor, mDesBoostFactor);
            else
                ALOGV("DRC presentation mode wrapper: set boost factor to %d\n", newBoostFactor);
#endif
        }

        if (newHeavy != mLastHeavy) {
            aacDecoder_SetParam(mHandleDecoder, AAC_DRC_HEAVY_COMPRESSION, newHeavy);
            mLastHeavy = newHeavy;
#ifdef DRC_PRES_MODE_WRAP_DEBUG
            if (newHeavy != mDesHeavy)
                ALOGV("DRC presentation mode wrapper: forced heavy compression to %d (from %d)\n",
                        newHeavy, mDesHeavy);
            else
                ALOGV("DRC presentation mode wrapper: set heavy compression to %d\n", newHeavy);
#endif
        }

#ifdef DRC_PRES_MODE_WRAP_DEBUG
        ALOGV("DRC config: tgt_lev: %3d, cut: %3d, boost: %3d, heavy: %d\n", newTarget,
                newAttFactor, newBoostFactor, newHeavy);
#endif
        mDataUpdate = false;

    } // if (mDataUpdate)
}
