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

#ifndef WEBMELEMENT_H_
#define WEBMELEMENT_H_

#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <utils/List.h>

namespace android {

class MetaData;

struct WebmElement : public LightRefBase<WebmElement> {
    const uint64_t mId, mSize;

    WebmElement(uint64_t id, uint64_t size);
    virtual ~WebmElement();

    virtual int serializePayloadSize(uint8_t *buf);
    virtual void serializePayload(uint8_t *buf)=0;
    uint64_t totalSize();
    uint64_t serializeInto(uint8_t *buf);
    uint8_t *serialize(uint64_t& size);
    int write(int fd, uint64_t& size);

    static sp<WebmElement> EbmlHeader(
            int ver = 1,
            int readVer = 1,
            int maxIdLen = 4,
            int maxSizeLen = 8,
            int docVer = 2,
            int docReadVer = 2);

    static sp<WebmElement> SegmentInfo(uint64_t scale = 1000000, double dur = 0);

    static sp<WebmElement> AudioTrackEntry(
            const char *codec,
            int chans,
            double rate,
            const sp<ABuffer> &buf,
            int bps = 0,
            uint64_t uid = 0,
            bool lacing = false,
            const char *lang = "und");

    static sp<WebmElement> VideoTrackEntry(
            const char *codec,
            uint64_t width,
            uint64_t height,
            const sp<MetaData> &md,
            uint64_t uid = 0,
            bool lacing = false,
            const char *lang = "und");

    static sp<WebmElement> SeekEntry(uint64_t id, uint64_t off);
    static sp<WebmElement> CuePointEntry(uint64_t time, int track, uint64_t off);
    static sp<WebmElement> SimpleBlock(
            int trackNum,
            int16_t timecode,
            bool key,
            const uint8_t *data,
            uint64_t dataSize);
};

struct WebmUnsigned : public WebmElement {
    WebmUnsigned(uint64_t id, uint64_t value);
    const uint64_t mValue;
    void serializePayload(uint8_t *buf);
};

struct WebmFloat : public WebmElement {
    const double mValue;
    WebmFloat(uint64_t id, float value);
    WebmFloat(uint64_t id, double value);
    void serializePayload(uint8_t *buf);
};

struct WebmBinary : public WebmElement {
    const sp<ABuffer> mRef;
    WebmBinary(uint64_t id, const sp<ABuffer> &ref);
    void serializePayload(uint8_t *buf);
};

struct WebmString : public WebmElement {
    const char *const mStr;
    WebmString(uint64_t id, const char *str);
    void serializePayload(uint8_t *buf);
};

struct WebmSimpleBlock : public WebmElement {
    const int mTrackNum;
    const int16_t mRelTimecode;
    const bool mKey;
    const sp<ABuffer> mRef;

    WebmSimpleBlock(int trackNum, int16_t timecode, bool key, const sp<ABuffer>& orig);
    void serializePayload(uint8_t *buf);
};

struct EbmlVoid : public WebmElement {
    const uint64_t mSizeWidth;
    explicit EbmlVoid(uint64_t totalSize);
    int serializePayloadSize(uint8_t *buf);
    void serializePayload(uint8_t *buf);
};

struct WebmMaster : public WebmElement {
    const List<sp<WebmElement> > mChildren;
    explicit WebmMaster(uint64_t id);
    WebmMaster(uint64_t id, const List<sp<WebmElement> > &children);
    int serializePayloadSize(uint8_t *buf);
    void serializePayload(uint8_t *buf);
};

} /* namespace android */
#endif /* WEBMELEMENT_H_ */
