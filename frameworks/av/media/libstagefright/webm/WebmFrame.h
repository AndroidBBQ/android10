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

#ifndef WEBMFRAME_H_
#define WEBMFRAME_H_

#include "WebmElement.h"

namespace android {

struct WebmFrame : LightRefBase<WebmFrame> {
public:
    const int mType;
    const bool mKey;
    uint64_t mAbsTimecode;
    const sp<ABuffer> mData;
    const bool mEos;

    WebmFrame();
    WebmFrame(int type, bool key, uint64_t absTimecode, MediaBufferBase *buf);
    ~WebmFrame() {}

    uint64_t getAbsTimecode();
    void updateAbsTimecode(uint64_t newAbsTimecode);
    sp<WebmElement> SimpleBlock(uint64_t baseTimecode) const;

    bool operator<(const WebmFrame &other) const;

    static const sp<WebmFrame> EOS;
private:
    DISALLOW_EVIL_CONSTRUCTORS(WebmFrame);
};

} /* namespace android */
#endif /* WEBMFRAME_H_ */
