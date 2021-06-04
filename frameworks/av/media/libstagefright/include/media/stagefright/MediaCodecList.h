/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MEDIA_CODEC_LIST_H_

#define MEDIA_CODEC_LIST_H_

#include <vector>

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/MediaCodecListWriter.h>
#include <media/IMediaCodecList.h>
#include <media/MediaCodecInfo.h>

#include <sys/types.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/Vector.h>
#include <utils/StrongPointer.h>

namespace android {

extern const char *kMaxEncoderInputBuffers;

struct AMessage;

struct MediaCodecList : public BnMediaCodecList {
    static sp<IMediaCodecList> getInstance();

    virtual ssize_t findCodecByType(
            const char *type, bool encoder, size_t startIndex = 0) const;

    virtual ssize_t findCodecByName(const char *name) const;

    virtual size_t countCodecs() const;

    virtual sp<MediaCodecInfo> getCodecInfo(size_t index) const {
        if (index >= mCodecInfos.size()) {
            ALOGE("b/24445127");
            return NULL;
        }
        return mCodecInfos[index];
    }

    virtual const sp<AMessage> getGlobalSettings() const;

    // to be used by MediaPlayerService alone
    static sp<IMediaCodecList> getLocalInstance();

    // only to be used by getLocalInstance
    static void *profilerThreadWrapper(void * /*arg*/);

    enum Flags {
        kPreferSoftwareCodecs   = 1,
        kHardwareCodecsOnly     = 2,
    };

    static void findMatchingCodecs(
            const char *mime,
            bool createEncoder,
            uint32_t flags,
            Vector<AString> *matchingCodecs);

    static bool isSoftwareCodec(const AString &componentName);

private:
    class BinderDeathObserver : public IBinder::DeathRecipient {
        void binderDied(const wp<IBinder> &the_late_who __unused);
    };

    static sp<BinderDeathObserver> sBinderDeathObserver;

    static sp<IMediaCodecList> sCodecList;
    static sp<IMediaCodecList> sRemoteList;

    status_t mInitCheck;

    sp<AMessage> mGlobalSettings;
    std::vector<sp<MediaCodecInfo> > mCodecInfos;

    /**
     * This constructor will call `buildMediaCodecList()` from the given
     * `MediaCodecListBuilderBase` objects.
     */
    MediaCodecList(std::vector<MediaCodecListBuilderBase*> builders);

    ~MediaCodecList();

    status_t initCheck() const;

    MediaCodecList(const MediaCodecList&) = delete;
    MediaCodecList& operator=(const MediaCodecList&) = delete;
};

}  // namespace android

#endif  // MEDIA_CODEC_LIST_H_

