/*
 * Copyright 2014, The Android Open Source Project
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

#ifndef MEDIA_CODEC_INFO_H_

#define MEDIA_CODEC_INFO_H_

#include <android-base/macros.h>
#include <binder/Parcel.h>
#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/AString.h>

#include <sys/types.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/StrongPointer.h>

#include <type_traits>

namespace android {

struct AMessage;
class Parcel;

typedef KeyedVector<AString, AString> CodecSettings;

struct MediaCodecInfoWriter;
struct MediaCodecListWriter;

struct MediaCodecInfo : public RefBase {
    struct ProfileLevel {
        uint32_t mProfile;
        uint32_t mLevel;
        bool operator <(const ProfileLevel &o) const {
            return mProfile < o.mProfile || (mProfile == o.mProfile && mLevel < o.mLevel);
        }
    };

    struct CapabilitiesWriter;

    enum Attributes : int32_t {
        // attribute flags
        kFlagIsEncoder = 1 << 0,
        kFlagIsVendor = 1 << 1,
        kFlagIsSoftwareOnly = 1 << 2,
        kFlagIsHardwareAccelerated = 1 << 3,
    };

    struct Capabilities : public RefBase {
        constexpr static char FEATURE_ADAPTIVE_PLAYBACK[] = "feature-adaptive-playback";
        constexpr static char FEATURE_DYNAMIC_TIMESTAMP[] = "feature-dynamic-timestamp";
        constexpr static char FEATURE_FRAME_PARSING[] = "feature-frame-parsing";
        constexpr static char FEATURE_INTRA_REFRESH[] = "feature-frame-parsing";
        constexpr static char FEATURE_MULTIPLE_FRAMES[] = "feature-multiple-frames";
        constexpr static char FEATURE_SECURE_PLAYBACK[] = "feature-secure-playback";
        constexpr static char FEATURE_TUNNELED_PLAYBACK[] = "feature-tunneled-playback";

        /**
         * Returns the supported levels for each supported profile in a target array.
         *
         * @param profileLevels target array for the profile levels.
         */
        void getSupportedProfileLevels(Vector<ProfileLevel> *profileLevels) const;

        /**
         * Returns the supported color formats in a target array. Only used for video/image
         * components.
         *
         * @param colorFormats target array for the color formats.
         */
        void getSupportedColorFormats(Vector<uint32_t> *colorFormats) const;

        /**
         * Returns metadata associated with this codec capability.
         *
         * This contains:
         * - features,
         * - performance data.
         *
         * TODO: expose this as separate API-s and wrap here.
         */
        const sp<AMessage> getDetails() const;

    protected:
        Vector<ProfileLevel> mProfileLevels;
        SortedVector<ProfileLevel> mProfileLevelsSorted;
        Vector<uint32_t> mColorFormats;
        SortedVector<uint32_t> mColorFormatsSorted;
        sp<AMessage> mDetails;

        Capabilities();

    private:
        // read object from parcel even if object creation fails
        static sp<Capabilities> FromParcel(const Parcel &parcel);
        status_t writeToParcel(Parcel *parcel) const;

        DISALLOW_COPY_AND_ASSIGN(Capabilities);

        friend struct MediaCodecInfo;
        friend struct MediaCodecInfoWriter;
        friend struct CapabilitiesWriter;
    };

    /**
     * This class is used for modifying information inside a `Capabilities`
     * object. An object of type `CapabilitiesWriter` can be obtained by calling
     * `MediaCodecInfoWriter::addMediaType()`.
     */
    struct CapabilitiesWriter {
        /**
         * Add a key-value pair to the list of details. If the key already
         * exists, the old value will be replaced.
         *
         * A pair added by this function will be accessible by
         * `Capabilities::getDetails()`. Call `AMessage::getString()` with the
         * same key to retrieve the value.
         *
         * @param key The key.
         * @param value The string value.
         */
        void addDetail(const char* key, const char* value);
        /**
         * Add a key-value pair to the list of details. If the key already
         * exists, the old value will be replaced.
         *
         * A pair added by this function will be accessible by
         * `Capabilities::getDetails()`. Call `AMessage::getInt32()` with the
         * same key to retrieve the value.
         *
         * @param key The key.
         * @param value The `int32_t` value.
         */
        void addDetail(const char* key, int32_t value);
        /**
         * Removes a key-value pair from the list of details. If the key is not
         * present, this call does nothing.
         *
         * @param key The key.
         */
        void removeDetail(const char* key);
        /**
         * Add a profile-level pair. If this profile-level pair already exists,
         * it will be ignored.
         *
         * @param profile The "profile" component.
         * @param level The "level" component.
         */
        void addProfileLevel(uint32_t profile, uint32_t level);
        /**
         * Add a color format. If this color format already exists, it will be
         * ignored.
         *
         * @param format The color format.
         */
        void addColorFormat(uint32_t format);

    private:
        /**
         * The associated `Capabilities` object.
         */
        Capabilities* mCap;
        /**
         * Construct a writer for the given `Capabilities` object.
         *
         * @param cap The `Capabilities` object to be written to.
         */
        CapabilitiesWriter(Capabilities* cap);

        friend MediaCodecInfoWriter;
    };

    inline bool isEncoder() const {
        return getAttributes() & kFlagIsEncoder;
    }

    Attributes getAttributes() const;
    void getSupportedMediaTypes(Vector<AString> *mediaTypes) const;
    const sp<Capabilities> getCapabilitiesFor(const char *mediaType) const;
    const char *getCodecName() const;

    /**
     * Returns a vector containing alternate names for the codec.
     *
     * \param aliases the destination array for the aliases. This is cleared.
     *
     * Multiple codecs may share alternate names as long as their supported media types are
     * distinct; however, these will result in different aliases for the MediaCodec user as
     * the canonical codec has to be resolved without knowing the media type in
     * MediaCodec::CreateByComponentName.
     */
    void getAliases(Vector<AString> *aliases) const;

    /**
     * Return the name of the service that hosts the codec. This value is not
     * visible at the Java level.
     *
     * Currently, this is the "instance name" of the IOmx service.
     */
    const char *getOwnerName() const;

    /**
     * Returns the rank of the component.
     *
     * Technically this is defined to be per media type, but that makes ordering the MediaCodecList
     * impossible as MediaCodecList is ordered by codec name.
     */
    uint32_t getRank() const;

    /**
     * Serialization over Binder
     */
    static sp<MediaCodecInfo> FromParcel(const Parcel &parcel);
    status_t writeToParcel(Parcel *parcel) const;

private:
    AString mName;
    AString mOwner;
    Attributes mAttributes;
    KeyedVector<AString, sp<Capabilities> > mCaps;
    Vector<AString> mAliases;
    uint32_t mRank;

    ssize_t getCapabilityIndex(const char *mediaType) const;

    /**
     * Construct an `MediaCodecInfo` object. After the construction, its
     * information can be set via an `MediaCodecInfoWriter` object obtained from
     * `MediaCodecListWriter::addMediaCodecInfo()`.
     */
    MediaCodecInfo();

    DISALLOW_COPY_AND_ASSIGN(MediaCodecInfo);

    friend class MediaCodecListOverridesTest;
    friend struct MediaCodecInfoWriter;
    friend struct MediaCodecListWriter;
};

/**
 * This class is to be used by a `MediaCodecListBuilderBase` instance to
 * populate information inside the associated `MediaCodecInfo` object.
 *
 * The only place where an instance of `MediaCodecInfoWriter` can be constructed
 * is `MediaCodecListWriter::addMediaCodecInfo()`. A `MediaCodecListBuilderBase`
 * instance should call `MediaCodecListWriter::addMediaCodecInfo()` on the given
 * `MediaCodecListWriter` object given as an input to
 * `MediaCodecListBuilderBase::buildMediaCodecList()`.
 */
struct MediaCodecInfoWriter {
    /**
     * Set the name of the codec.
     *
     * @param name The new name.
     */
    void setName(const char* name);
    /**
     * Adds an alias (alternate name) for the codec. Multiple codecs can share an alternate name
     * as long as their supported media types are distinct.
     *
     * @param name an alternate name.
     */
    void addAlias(const char* name);
    /**
     * Set the owner name of the codec.
     *
     * This "owner name" is the name of the `IOmx` instance that supports this
     * codec.
     *
     * @param owner The new owner name.
     */
    void setOwner(const char* owner);
    /**
     * Sets codec attributes.
     *
     * @param attributes Codec attributes.
     */
    void setAttributes(typename std::underlying_type<MediaCodecInfo::Attributes>::type attributes);
    /**
     * Add a media type to an indexed list and return a `CapabilitiesWriter` object
     * that can be used for modifying the associated `Capabilities`.
     *
     * If the media type already exists, this function will return the
     * `CapabilitiesWriter` associated with the media type.
     *
     * @param[in] mediaType The name of a new media type to add.
     * @return writer The `CapabilitiesWriter` object for modifying the
     * `Capabilities` associated with the media type. `writer` will be valid
     * regardless of whether `mediaType` already exists or not.
     */
    std::unique_ptr<MediaCodecInfo::CapabilitiesWriter> addMediaType(
            const char* mediaType);
    /**
     * Remove a media type.
     *
     * @param mediaType The name of the media type to remove.
     * @return `true` if `mediaType` is removed; `false` if `mediaType` is not found.
     */
    bool removeMediaType(const char* mediaType);
    /**
     * Set rank of the codec. MediaCodecList will stable-sort the list according
     * to rank in non-descending order.
     *
     * @param rank The rank of the component.
     */
    void setRank(uint32_t rank);
private:
    /**
     * The associated `MediaCodecInfo`.
     */
    MediaCodecInfo* mInfo;
    /**
     * Construct the `MediaCodecInfoWriter` object associated with the given
     * `MediaCodecInfo` object.
     *
     * @param info The underlying `MediaCodecInfo` object.
     */
    MediaCodecInfoWriter(MediaCodecInfo* info);

    DISALLOW_COPY_AND_ASSIGN(MediaCodecInfoWriter);

    friend struct MediaCodecListWriter;
};

}  // namespace android

#endif  // MEDIA_CODEC_INFO_H_


