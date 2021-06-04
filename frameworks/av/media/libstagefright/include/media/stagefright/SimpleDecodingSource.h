/*
 * Copyright 2016, The Android Open Source Project
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

#ifndef SIMPLE_DECODING_SOURCE_H_
#define SIMPLE_DECODING_SOURCE_H_

#include <media/MediaSource.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/foundation/Mutexed.h>

#include <utils/Condition.h>
#include <utils/StrongPointer.h>

struct ANativeWindow;

namespace android {

struct ALooper;
struct AMessage;
class MediaBuffer;
struct MediaCodec;
class MetaData;

class SimpleDecodingSource : public MediaSource {
public:
    // Creates a MediaSource that uses MediaCodec to decode a compressed input |source|.
    // The selected codec can be influenced using |flags|. This source only supports the
    // kPreferGoogleCodec and kNonGoogleCodecsOnly |flags| - MediaCodecList.
    // You can pass in a target |nativeWindow| to render video directly onto a surface. In this
    // case the source will return empty buffers.
    // This source cannot be restarted (hence the name "Simple"), all reads are blocking, and
    // does not support secure input or pausing.
    // if |desiredCodec| is given, use this specific codec.
    static sp<SimpleDecodingSource> Create(
            const sp<MediaSource> &source, uint32_t flags,
            const sp<ANativeWindow> &nativeWindow,
            const char *desiredCodec = NULL,
            bool skipMediaCodecList = false);

    static sp<SimpleDecodingSource> Create(
            const sp<MediaSource> &source, uint32_t flags = 0);

    virtual ~SimpleDecodingSource();

    // starts this source (and it's underlying source). |params| is ignored.
    virtual status_t start(MetaData *params = NULL);

    // stops this source (and it's underlying source).
    virtual status_t stop();

    // returns the output format of this source.
    virtual sp<MetaData> getFormat();

    // reads from the source. This call always blocks.
    virtual status_t read(MediaBufferBase **buffer, const ReadOptions *options);

    // unsupported methods
    virtual status_t pause() { return INVALID_OPERATION; }

private:
    // Construct this using a codec, source and looper.
    SimpleDecodingSource(
            const sp<MediaCodec> &codec, const sp<MediaSource> &source, const sp<ALooper> &looper,
            bool usingSurface, bool isVorbis, const sp<AMessage> &format);

    sp<MediaCodec> mCodec;
    sp<MediaSource> mSource;
    sp<ALooper> mLooper;
    bool mUsingSurface;
    bool mIsVorbis;
    enum State {
        INIT,
        STARTED,
        STOPPING,
        STOPPED,
        ERROR,
    };
    AString mComponentName;

    struct ProtectedState {
        ProtectedState(const sp<AMessage> &format);
        bool mReading;
        Condition mReadCondition;

        sp<AMessage> mFormat;
        State mState;
        bool mQueuedInputEOS;
        bool mGotOutputEOS;
    };
    Mutexed<ProtectedState> mProtectedState;

    // do the actual reading
    status_t doRead(
            Mutexed<ProtectedState>::Locked &me, MediaBufferBase **buffer,
            const ReadOptions *options);
};

} // namespace android

#endif
