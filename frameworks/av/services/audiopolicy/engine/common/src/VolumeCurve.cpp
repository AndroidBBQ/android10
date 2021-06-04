/*
 * Copyright (C) 2015 The Android Open Source Project
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

#define LOG_TAG "APM::VolumeCurve"
//#define LOG_NDEBUG 0

#include "VolumeCurve.h"
#include "TypeConverter.h"
#include <media/TypeConverter.h>

namespace android {

float VolumeCurve::volIndexToDb(int indexInUi, int volIndexMin, int volIndexMax) const
{
    ALOG_ASSERT(!mCurvePoints.isEmpty(), "Invalid volume curve");
    if (volIndexMin < 0 || volIndexMax < 0) {
        // In order to let AudioService initialize the min and max, convention is to use -1
        return NAN;
    }
    if (indexInUi < volIndexMin) {
        // an index of 0 means mute request when volIndexMin > 0
        if (indexInUi == 0) {
            ALOGV("VOLUME forcing mute for index 0 with min index %d", volIndexMin);
            return VOLUME_MIN_DB;
        }
        ALOGV("VOLUME remapping index from %d to min index %d", indexInUi, volIndexMin);
        indexInUi = volIndexMin;
    } else if (indexInUi > volIndexMax) {
        ALOGV("VOLUME remapping index from %d to max index %d", indexInUi, volIndexMax);
        indexInUi = volIndexMax;
    }

    size_t nbCurvePoints = mCurvePoints.size();
    // the volume index in the UI is relative to the min and max volume indices for this stream
    int nbSteps = 1 + mCurvePoints[nbCurvePoints - 1].mIndex - mCurvePoints[0].mIndex;
    int volIdx = (nbSteps * (indexInUi - volIndexMin)) / (volIndexMax - volIndexMin);

    // Where would this volume index been inserted in the curve point
    size_t indexInUiPosition = mCurvePoints.orderOf(CurvePoint(volIdx, 0));
    if (indexInUiPosition >= nbCurvePoints) {
        //use last point of table
        return mCurvePoints[nbCurvePoints - 1].mAttenuationInMb / 100.0f;
    }
    if (indexInUiPosition == 0) {
        if (indexInUiPosition != mCurvePoints[0].mIndex) {
            return VOLUME_MIN_DB; // out of bounds
        }
        return mCurvePoints[0].mAttenuationInMb / 100.0f;
    }
    // linear interpolation in the attenuation table in dB
    float decibels = (mCurvePoints[indexInUiPosition - 1].mAttenuationInMb / 100.0f) +
            ((float)(volIdx - mCurvePoints[indexInUiPosition - 1].mIndex)) *
                ( ((mCurvePoints[indexInUiPosition].mAttenuationInMb / 100.0f) -
                        (mCurvePoints[indexInUiPosition - 1].mAttenuationInMb / 100.0f)) /
                    ((float)(mCurvePoints[indexInUiPosition].mIndex -
                            mCurvePoints[indexInUiPosition - 1].mIndex)) );

    ALOGV("VOLUME vol index=[%d %d %d], dB=[%.1f %.1f %.1f]",
            mCurvePoints[indexInUiPosition - 1].mIndex, volIdx,
            mCurvePoints[indexInUiPosition].mIndex,
            ((float)mCurvePoints[indexInUiPosition - 1].mAttenuationInMb / 100.0f), decibels,
            ((float)mCurvePoints[indexInUiPosition].mAttenuationInMb / 100.0f));

    return decibels;
}

void VolumeCurve::dump(String8 *dst, int spaces, bool curvePoints) const
{
    if (!curvePoints) {
        return;
    }
    dst->append(" {");
    for (size_t i = 0; i < mCurvePoints.size(); i++) {
        dst->appendFormat("%*s(%3d, %5d)", spaces, "", mCurvePoints[i].mIndex,
                          mCurvePoints[i].mAttenuationInMb);
        dst->appendFormat(i == (mCurvePoints.size() - 1) ? " }\n" : ", ");
    }
}

void VolumeCurves::dump(String8 *dst, int spaces, bool curvePoints) const
{
    if (!curvePoints) {
//        dst->appendFormat("%*s%02d      %s         %03d        %03d        ", spaces, "",
//                          mStream, mCanBeMuted ? "true " : "false", mIndexMin, mIndexMax);
        dst->appendFormat("%*s Can be muted  Index Min  Index Max  Index Cur [device : index]...\n",
                          spaces + 1, "");
        dst->appendFormat("%*s %s         %02d         %02d         ", spaces + 1, "",
                          mCanBeMuted ? "true " : "false", mIndexMin, mIndexMax);
        for (const auto &pair : mIndexCur) {
            dst->appendFormat("%04x : %02d, ", pair.first, pair.second);
        }
        dst->appendFormat("\n");
        return;
    }
    std::string streamNames;
    for (const auto &stream : mStreams) {
        streamNames += android::toString(stream) + "("+std::to_string(stream)+") ";
    }
    dst->appendFormat("%*sVolume Curves Streams/Attributes, Curve points Streams for device"
                      " category (index, attenuation in millibel)\n", spaces, "");
    dst->appendFormat("%*s Streams: %s \n", spaces, "", streamNames.c_str());
    if (!mAttributes.empty()) dst->appendFormat("%*s Attributes:", spaces, "");
    for (const auto &attributes : mAttributes) {
        std::string attStr = attributes == defaultAttr ? "{ Any }" : android::toString(attributes);
        dst->appendFormat("%*s %s\n", attributes == mAttributes.front() ? 0 : spaces + 13, "",
                          attStr.c_str());
    }
    for (size_t i = 0; i < size(); i++) {
        std::string deviceCatLiteral;
        DeviceCategoryConverter::toString(keyAt(i), deviceCatLiteral);
        dst->appendFormat("%*s %s :", spaces, "", deviceCatLiteral.c_str());
        valueAt(i)->dump(dst, 1, true);
    }
}

} // namespace android
