/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <gui/LayerDebugInfo.h>

#include <android-base/stringprintf.h>

#include <ui/DebugUtils.h>

#include <binder/Parcel.h>

using namespace android;
using android::base::StringAppendF;

#define RETURN_ON_ERROR(X) do {status_t res = (X); if (res != NO_ERROR) return res;} while(false)

namespace android {

status_t LayerDebugInfo::writeToParcel(Parcel* parcel) const {
    RETURN_ON_ERROR(parcel->writeCString(mName.c_str()));
    RETURN_ON_ERROR(parcel->writeCString(mParentName.c_str()));
    RETURN_ON_ERROR(parcel->writeCString(mType.c_str()));
    RETURN_ON_ERROR(parcel->write(mTransparentRegion));
    RETURN_ON_ERROR(parcel->write(mVisibleRegion));
    RETURN_ON_ERROR(parcel->write(mSurfaceDamageRegion));
    RETURN_ON_ERROR(parcel->writeUint32(mLayerStack));
    RETURN_ON_ERROR(parcel->writeFloat(mX));
    RETURN_ON_ERROR(parcel->writeFloat(mY));
    RETURN_ON_ERROR(parcel->writeUint32(mZ));
    RETURN_ON_ERROR(parcel->writeInt32(mWidth));
    RETURN_ON_ERROR(parcel->writeInt32(mHeight));
    RETURN_ON_ERROR(parcel->write(mCrop));
    RETURN_ON_ERROR(parcel->writeFloat(mColor.r));
    RETURN_ON_ERROR(parcel->writeFloat(mColor.g));
    RETURN_ON_ERROR(parcel->writeFloat(mColor.b));
    RETURN_ON_ERROR(parcel->writeFloat(mColor.a));
    RETURN_ON_ERROR(parcel->writeUint32(mFlags));
    RETURN_ON_ERROR(parcel->writeInt32(mPixelFormat));
    RETURN_ON_ERROR(parcel->writeUint32(static_cast<uint32_t>(mDataSpace)));
    for (size_t index = 0; index < 4; index++) {
        RETURN_ON_ERROR(parcel->writeFloat(mMatrix[index / 2][index % 2]));
    }
    RETURN_ON_ERROR(parcel->writeInt32(mActiveBufferWidth));
    RETURN_ON_ERROR(parcel->writeInt32(mActiveBufferHeight));
    RETURN_ON_ERROR(parcel->writeInt32(mActiveBufferStride));
    RETURN_ON_ERROR(parcel->writeInt32(mActiveBufferFormat));
    RETURN_ON_ERROR(parcel->writeInt32(mNumQueuedFrames));
    RETURN_ON_ERROR(parcel->writeBool(mRefreshPending));
    RETURN_ON_ERROR(parcel->writeBool(mIsOpaque));
    RETURN_ON_ERROR(parcel->writeBool(mContentDirty));
    return NO_ERROR;
}

status_t LayerDebugInfo::readFromParcel(const Parcel* parcel) {
    mName = parcel->readCString();
    RETURN_ON_ERROR(parcel->errorCheck());
    mParentName = parcel->readCString();
    RETURN_ON_ERROR(parcel->errorCheck());
    mType = parcel->readCString();
    RETURN_ON_ERROR(parcel->errorCheck());
    RETURN_ON_ERROR(parcel->read(mTransparentRegion));
    RETURN_ON_ERROR(parcel->read(mVisibleRegion));
    RETURN_ON_ERROR(parcel->read(mSurfaceDamageRegion));
    RETURN_ON_ERROR(parcel->readUint32(&mLayerStack));
    RETURN_ON_ERROR(parcel->readFloat(&mX));
    RETURN_ON_ERROR(parcel->readFloat(&mY));
    RETURN_ON_ERROR(parcel->readUint32(&mZ));
    RETURN_ON_ERROR(parcel->readInt32(&mWidth));
    RETURN_ON_ERROR(parcel->readInt32(&mHeight));
    RETURN_ON_ERROR(parcel->read(mCrop));
    mColor.r = parcel->readFloat();
    RETURN_ON_ERROR(parcel->errorCheck());
    mColor.g = parcel->readFloat();
    RETURN_ON_ERROR(parcel->errorCheck());
    mColor.b = parcel->readFloat();
    RETURN_ON_ERROR(parcel->errorCheck());
    mColor.a = parcel->readFloat();
    RETURN_ON_ERROR(parcel->errorCheck());
    RETURN_ON_ERROR(parcel->readUint32(&mFlags));
    RETURN_ON_ERROR(parcel->readInt32(&mPixelFormat));
    // \todo [2017-07-25 kraita]: Static casting mDataSpace pointer to an uint32 does work. Better ways?
    mDataSpace = static_cast<android_dataspace>(parcel->readUint32());
    RETURN_ON_ERROR(parcel->errorCheck());
    for (size_t index = 0; index < 4; index++) {
        RETURN_ON_ERROR(parcel->readFloat(&mMatrix[index / 2][index % 2]));
    }
    RETURN_ON_ERROR(parcel->readInt32(&mActiveBufferWidth));
    RETURN_ON_ERROR(parcel->readInt32(&mActiveBufferHeight));
    RETURN_ON_ERROR(parcel->readInt32(&mActiveBufferStride));
    RETURN_ON_ERROR(parcel->readInt32(&mActiveBufferFormat));
    RETURN_ON_ERROR(parcel->readInt32(&mNumQueuedFrames));
    RETURN_ON_ERROR(parcel->readBool(&mRefreshPending));
    RETURN_ON_ERROR(parcel->readBool(&mIsOpaque));
    RETURN_ON_ERROR(parcel->readBool(&mContentDirty));
    return NO_ERROR;
}

std::string to_string(const LayerDebugInfo& info) {
    std::string result;

    StringAppendF(&result, "+ %s (%s)\n", info.mType.c_str(), info.mName.c_str());
    info.mTransparentRegion.dump(result, "TransparentRegion");
    info.mVisibleRegion.dump(result, "VisibleRegion");
    info.mSurfaceDamageRegion.dump(result, "SurfaceDamageRegion");

    StringAppendF(&result, "      layerStack=%4d, z=%9d, pos=(%g,%g), size=(%4d,%4d), ",
                  info.mLayerStack, info.mZ, static_cast<double>(info.mX),
                  static_cast<double>(info.mY), info.mWidth, info.mHeight);

    StringAppendF(&result, "crop=%s, ", to_string(info.mCrop).c_str());
    StringAppendF(&result, "isOpaque=%1d, invalidate=%1d, ", info.mIsOpaque, info.mContentDirty);
    StringAppendF(&result, "dataspace=%s, ", dataspaceDetails(info.mDataSpace).c_str());
    StringAppendF(&result, "pixelformat=%s, ", decodePixelFormat(info.mPixelFormat).c_str());
    StringAppendF(&result, "color=(%.3f,%.3f,%.3f,%.3f), flags=0x%08x, ",
                  static_cast<double>(info.mColor.r), static_cast<double>(info.mColor.g),
                  static_cast<double>(info.mColor.b), static_cast<double>(info.mColor.a),
                  info.mFlags);
    StringAppendF(&result, "tr=[%.2f, %.2f][%.2f, %.2f]", static_cast<double>(info.mMatrix[0][0]),
                  static_cast<double>(info.mMatrix[0][1]), static_cast<double>(info.mMatrix[1][0]),
                  static_cast<double>(info.mMatrix[1][1]));
    result.append("\n");
    StringAppendF(&result, "      parent=%s\n", info.mParentName.c_str());
    StringAppendF(&result, "      activeBuffer=[%4ux%4u:%4u,%s],", info.mActiveBufferWidth,
                  info.mActiveBufferHeight, info.mActiveBufferStride,
                  decodePixelFormat(info.mActiveBufferFormat).c_str());
    StringAppendF(&result, " queued-frames=%d, mRefreshPending=%d", info.mNumQueuedFrames,
                  info.mRefreshPending);
    result.append("\n");
    return result;
}

} // android
