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
#define LOG_TAG "RSFilter"

#include <utils/Log.h>

#include <media/MediaCodecBuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>

#include "RSFilter.h"

namespace android {

RSFilter::RSFilter() {

}

RSFilter::~RSFilter() {

}

status_t RSFilter::configure(const sp<AMessage> &msg) {
    status_t err = SimpleFilter::configure(msg);
    if (err != OK) {
        return err;
    }

    if (!msg->findString("cacheDir", &mCacheDir)) {
        ALOGE("Failed to find cache directory in config message.");
        return NAME_NOT_FOUND;
    }

    sp<RenderScriptWrapper> wrapper;
    if (!msg->findObject("rs-wrapper", (sp<RefBase>*)&wrapper)) {
        ALOGE("Failed to find RenderScriptWrapper in config message.");
        return NAME_NOT_FOUND;
    }

    mRS = wrapper->mContext;
    mCallback = wrapper->mCallback;

    return OK;
}

status_t RSFilter::start() {
    // 32-bit elements for ARGB8888
    RSC::sp<const RSC::Element> e = RSC::Element::U8_4(mRS);

    RSC::Type::Builder tb(mRS, e);
    tb.setX(mWidth);
    tb.setY(mHeight);
    RSC::sp<const RSC::Type> t = tb.create();

    mAllocIn = RSC::Allocation::createTyped(mRS, t);
    mAllocOut = RSC::Allocation::createTyped(mRS, t);

    return OK;
}

void RSFilter::reset() {
    mCallback.clear();
    mAllocOut.clear();
    mAllocIn.clear();
    mRS.clear();
}

status_t RSFilter::setParameters(const sp<AMessage> &msg) {
    return mCallback->handleSetParameters(msg);
}

status_t RSFilter::processBuffers(
        const sp<MediaCodecBuffer> &srcBuffer, const sp<MediaCodecBuffer> &outBuffer) {
    mAllocIn->copy1DRangeFrom(0, mWidth * mHeight, srcBuffer->data());
    mCallback->processBuffers(mAllocIn.get(), mAllocOut.get());
    mAllocOut->copy1DRangeTo(0, mWidth * mHeight, outBuffer->data());

    return OK;
}

}   // namespace android
