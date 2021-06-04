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
#define LOG_TAG "WebmFrame"

#include "WebmFrame.h"
#include "WebmConstants.h"

#include <media/stagefright/foundation/ADebug.h>
#include <unistd.h>

using namespace android;
using namespace webm;

namespace {
sp<ABuffer> toABuffer(MediaBufferBase *mbuf) {
    sp<ABuffer> abuf = new ABuffer(mbuf->range_length());
    memcpy(abuf->data(), (uint8_t*) mbuf->data() + mbuf->range_offset(), mbuf->range_length());
    return abuf;
}
}

namespace android {

const sp<WebmFrame> WebmFrame::EOS = new WebmFrame();

WebmFrame::WebmFrame()
    : mType(kInvalidType),
      mKey(false),
      mAbsTimecode(UINT64_MAX),
      mData(new ABuffer(0)),
      mEos(true) {
}

WebmFrame::WebmFrame(int type, bool key, uint64_t absTimecode, MediaBufferBase *mbuf)
    : mType(type),
      mKey(key),
      mAbsTimecode(absTimecode),
      mData(toABuffer(mbuf)),
      mEos(false) {
}

sp<WebmElement> WebmFrame::SimpleBlock(uint64_t baseTimecode) const {
    return new WebmSimpleBlock(
            mType == kVideoType ? kVideoTrackNum : kAudioTrackNum,
            mAbsTimecode - baseTimecode,
            mKey,
            mData);
}

uint64_t WebmFrame::getAbsTimecode() {
    return mAbsTimecode;
}

void WebmFrame::updateAbsTimecode(uint64_t newAbsTimecode) {
    mAbsTimecode = newAbsTimecode;
}

bool WebmFrame::operator<(const WebmFrame &other) const {
    if (this->mEos) {
        return false;
    }
    if (other.mEos) {
        return true;
    }
    if (this->mAbsTimecode == other.mAbsTimecode) {
        if (this->mType == kAudioType && other.mType == kVideoType) {
            return true;
        }
        if (this->mType == kVideoType && other.mType == kAudioType) {
            return false;
        }
        return false;
    }
    return this->mAbsTimecode < other.mAbsTimecode;
}
} /* namespace android */
