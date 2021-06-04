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

#ifndef MEDIA_CODEC_LIST_WRITER_H_

#define MEDIA_CODEC_LIST_WRITER_H_

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/MediaCodecListWriter.h>
#include <media/MediaCodecInfo.h>

#include <utils/Errors.h>
#include <utils/StrongPointer.h>

namespace android {

/**
 * This class is to be used by a `MediaCodecListBuilderBase` instance to add
 * information to the destination `MediaCodecList` object.
 */
struct MediaCodecListWriter {
    /**
     * Add a key-value pair to a `MediaCodecList`'s global settings.
     *
     * @param key Key.
     * @param value Value.
     */
    void addGlobalSetting(const char* key, const char* value);
    /**
     * Create an add a new `MediaCodecInfo` object for a `MediaCodecList`, and
     * return a `MediaCodecInfoWriter` object associated with the newly added
     * `MediaCodecInfo`.
     *
     * @return The `MediaCodecInfoWriter` object associated with the newly
     * added `MediaCodecInfo` object.
     */
    std::unique_ptr<MediaCodecInfoWriter> addMediaCodecInfo();
    /**
     * Find an existing `MediaCodecInfo` object for a codec name and return a
     * `MediaCodecInfoWriter` object associated with the found added `MediaCodecInfo`.
     *
     * @return The `MediaCodecInfoWriter` object if found, or nullptr if not found.
     */
    std::unique_ptr<MediaCodecInfoWriter> findMediaCodecInfo(const char *codecName);
private:
    MediaCodecListWriter() = default;

    void writeGlobalSettings(const sp<AMessage> &globalSettings) const;
    void writeCodecInfos(std::vector<sp<MediaCodecInfo>> *codecInfos) const;

    std::vector<std::pair<std::string, std::string>> mGlobalSettings;
    std::vector<sp<MediaCodecInfo>> mCodecInfos;

    friend struct MediaCodecList;
};

/**
 * This interface is to be used by `MediaCodecList` to fill its members with
 * appropriate information. `buildMediaCodecList()` will be called from a
 * `MediaCodecList` object during its construction.
 */
struct MediaCodecListBuilderBase {
    /**
     * Build the `MediaCodecList` via the given `MediaCodecListWriter` interface.
     *
     * @param writer The writer interface.
     * @return The status of the construction. `NO_ERROR` means success.
     */
    virtual status_t buildMediaCodecList(MediaCodecListWriter* writer) = 0;

    /**
     * The default destructor does nothing.
     */
    virtual ~MediaCodecListBuilderBase() = default;

    typedef MediaCodecListBuilderBase *(*CreateBuilderFunc)(void);
};

}  // namespace android

#endif  // MEDIA_CODEC_LIST_WRITER_H_

