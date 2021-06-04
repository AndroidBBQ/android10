/*
 * Copyright (C) 2010 The Android Open Source Project
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

#ifndef M3U_PARSER_H_

#define M3U_PARSER_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/mediaplayer.h>
#include <utils/Vector.h>

namespace android {

struct M3UParser : public RefBase {
    M3UParser(const char *baseURI, const void *data, size_t size);

    status_t initCheck() const;

    bool isExtM3U() const;
    bool isVariantPlaylist() const;
    bool isComplete() const;
    bool isEvent() const;
    size_t getDiscontinuitySeq() const;
    int64_t getTargetDuration() const;
    int32_t getFirstSeqNumber() const;
    void getSeqNumberRange(int32_t *firstSeq, int32_t *lastSeq) const;

    sp<AMessage> meta();

    size_t size();
    bool itemAt(size_t index, AString *uri, sp<AMessage> *meta = NULL);

    void pickRandomMediaItems();
    status_t selectTrack(size_t index, bool select);
    size_t getTrackCount() const;
    sp<AMessage> getTrackInfo(size_t index) const;
    ssize_t getSelectedIndex() const;
    ssize_t getSelectedTrack(media_track_type /* type */) const;

    bool getTypeURI(size_t index, const char *key, AString *uri) const;
    bool hasType(size_t index, const char *key) const;

protected:
    virtual ~M3UParser();

private:
    struct MediaGroup;

    struct Item {
        AString mURI;
        sp<AMessage> mMeta;
        AString makeURL(const char *baseURL) const;
    };

    status_t mInitCheck;

    AString mBaseURI;
    bool mIsExtM3U;
    bool mIsVariantPlaylist;
    bool mIsComplete;
    bool mIsEvent;
    int32_t mFirstSeqNumber;
    int32_t mLastSeqNumber;
    int64_t mTargetDurationUs;
    size_t mDiscontinuitySeq;
    int32_t mDiscontinuityCount;

    sp<AMessage> mMeta;
    Vector<Item> mItems;
    ssize_t mSelectedIndex;

    // Media groups keyed by group ID.
    KeyedVector<AString, sp<MediaGroup> > mMediaGroups;

    status_t parse(const void *data, size_t size);

    static status_t parseMetaData(
            const AString &line, sp<AMessage> *meta, const char *key);

    static status_t parseMetaDataDuration(
            const AString &line, sp<AMessage> *meta, const char *key);

    status_t parseStreamInf(
            const AString &line, sp<AMessage> *meta) const;

    static status_t parseCipherInfo(
            const AString &line, sp<AMessage> *meta, const AString &baseURI);

    static status_t parseByteRange(
            const AString &line, uint64_t curOffset,
            uint64_t *length, uint64_t *offset);

    status_t parseMedia(const AString &line);

    static status_t parseDiscontinuitySequence(const AString &line, size_t *seq);

    static status_t ParseInt32(const char *s, int32_t *x);
    static status_t ParseDouble(const char *s, double *x);

    static bool isQuotedString(const AString &str);
    static AString unquoteString(const AString &str);
    static bool codecIsType(const AString &codec, const char *type);

    DISALLOW_EVIL_CONSTRUCTORS(M3UParser);
};

}  // namespace android

#endif  // M3U_PARSER_H_
