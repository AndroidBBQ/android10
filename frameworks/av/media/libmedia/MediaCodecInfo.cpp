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

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaCodecInfo"
#include <utils/Log.h>

#include <media/IOMX.h>

#include <media/MediaCodecInfo.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <binder/Parcel.h>

namespace android {

/** This redundant redeclaration is needed for C++ pre 14 */
constexpr char MediaCodecInfo::Capabilities::FEATURE_ADAPTIVE_PLAYBACK[];
constexpr char MediaCodecInfo::Capabilities::FEATURE_DYNAMIC_TIMESTAMP[];
constexpr char MediaCodecInfo::Capabilities::FEATURE_FRAME_PARSING[];
constexpr char MediaCodecInfo::Capabilities::FEATURE_INTRA_REFRESH[];
constexpr char MediaCodecInfo::Capabilities::FEATURE_MULTIPLE_FRAMES[];
constexpr char MediaCodecInfo::Capabilities::FEATURE_SECURE_PLAYBACK[];
constexpr char MediaCodecInfo::Capabilities::FEATURE_TUNNELED_PLAYBACK[];

void MediaCodecInfo::Capabilities::getSupportedProfileLevels(
        Vector<ProfileLevel> *profileLevels) const {
    profileLevels->clear();
    profileLevels->appendVector(mProfileLevels);
}

void MediaCodecInfo::Capabilities::getSupportedColorFormats(
        Vector<uint32_t> *colorFormats) const {
    colorFormats->clear();
    colorFormats->appendVector(mColorFormats);
}

const sp<AMessage> MediaCodecInfo::Capabilities::getDetails() const {
    return mDetails;
}

MediaCodecInfo::Capabilities::Capabilities() {
    mDetails = new AMessage;
}

// static
sp<MediaCodecInfo::Capabilities> MediaCodecInfo::Capabilities::FromParcel(
        const Parcel &parcel) {
    sp<MediaCodecInfo::Capabilities> caps = new Capabilities();
    size_t size = static_cast<size_t>(parcel.readInt32());
    for (size_t i = 0; i < size; i++) {
        ProfileLevel profileLevel;
        profileLevel.mProfile = static_cast<uint32_t>(parcel.readInt32());
        profileLevel.mLevel = static_cast<uint32_t>(parcel.readInt32());
        if (caps != NULL) {
            caps->mProfileLevels.push_back(profileLevel);
        }
    }
    size = static_cast<size_t>(parcel.readInt32());
    for (size_t i = 0; i < size; i++) {
        uint32_t color = static_cast<uint32_t>(parcel.readInt32());
        if (caps != NULL) {
            caps->mColorFormats.push_back(color);
        }
    }
    sp<AMessage> details = AMessage::FromParcel(parcel);
    if (details == NULL)
        return NULL;
    if (caps != NULL) {
        caps->mDetails = details;
    }
    return caps;
}

status_t MediaCodecInfo::Capabilities::writeToParcel(Parcel *parcel) const {
    CHECK_LE(mProfileLevels.size(), static_cast<size_t>(INT32_MAX));
    parcel->writeInt32(mProfileLevels.size());
    for (size_t i = 0; i < mProfileLevels.size(); i++) {
        parcel->writeInt32(mProfileLevels.itemAt(i).mProfile);
        parcel->writeInt32(mProfileLevels.itemAt(i).mLevel);
    }
    CHECK_LE(mColorFormats.size(), static_cast<size_t>(INT32_MAX));
    parcel->writeInt32(mColorFormats.size());
    for (size_t i = 0; i < mColorFormats.size(); i++) {
        parcel->writeInt32(mColorFormats.itemAt(i));
    }
    mDetails->writeToParcel(parcel);
    return OK;
}

void MediaCodecInfo::CapabilitiesWriter::addDetail(
        const char* key, const char* value) {
    mCap->mDetails->setString(key, value);
}

void MediaCodecInfo::CapabilitiesWriter::addDetail(
        const char* key, int32_t value) {
    mCap->mDetails->setInt32(key, value);
}

void MediaCodecInfo::CapabilitiesWriter::removeDetail(const char* key) {
    if (mCap->mDetails->removeEntryAt(mCap->mDetails->findEntryByName(key)) == OK) {
        ALOGD("successfully removed detail %s", key);
    } else {
        ALOGD("detail %s wasn't present to remove", key);
    }
}

void MediaCodecInfo::CapabilitiesWriter::addProfileLevel(
        uint32_t profile, uint32_t level) {
    ProfileLevel profileLevel;
    profileLevel.mProfile = profile;
    profileLevel.mLevel = level;
    if (mCap->mProfileLevelsSorted.indexOf(profileLevel) < 0) {
        mCap->mProfileLevels.push_back(profileLevel);
        mCap->mProfileLevelsSorted.add(profileLevel);
    }
}

void MediaCodecInfo::CapabilitiesWriter::addColorFormat(uint32_t format) {
    if (mCap->mColorFormatsSorted.indexOf(format) < 0) {
        mCap->mColorFormats.push(format);
        mCap->mColorFormatsSorted.add(format);
    }
}

MediaCodecInfo::CapabilitiesWriter::CapabilitiesWriter(
        MediaCodecInfo::Capabilities* cap) : mCap(cap) {
}

MediaCodecInfo::Attributes MediaCodecInfo::getAttributes() const {
    return mAttributes;
}

uint32_t MediaCodecInfo::getRank() const {
    return mRank;
}

void MediaCodecInfo::getAliases(Vector<AString> *aliases) const {
    *aliases = mAliases;
}

void MediaCodecInfo::getSupportedMediaTypes(Vector<AString> *mediaTypes) const {
    mediaTypes->clear();
    for (size_t ix = 0; ix < mCaps.size(); ix++) {
        mediaTypes->push_back(mCaps.keyAt(ix));
    }
}

const sp<MediaCodecInfo::Capabilities>
MediaCodecInfo::getCapabilitiesFor(const char *mediaType) const {
    ssize_t ix = getCapabilityIndex(mediaType);
    if (ix >= 0) {
        return mCaps.valueAt(ix);
    }
    return NULL;
}

const char *MediaCodecInfo::getCodecName() const {
    return mName.c_str();
}

const char *MediaCodecInfo::getOwnerName() const {
    return mOwner.c_str();
}

// static
sp<MediaCodecInfo> MediaCodecInfo::FromParcel(const Parcel &parcel) {
    AString name = AString::FromParcel(parcel);
    AString owner = AString::FromParcel(parcel);
    Attributes attributes = static_cast<Attributes>(parcel.readInt32());
    uint32_t rank = parcel.readUint32();
    sp<MediaCodecInfo> info = new MediaCodecInfo;
    info->mName = name;
    info->mOwner = owner;
    info->mAttributes = attributes;
    info->mRank = rank;
    size_t numAliases = static_cast<size_t>(parcel.readInt32());
    for (size_t i = 0; i < numAliases; i++) {
        AString alias = AString::FromParcel(parcel);
        info->mAliases.add(alias);
    }
    size_t size = static_cast<size_t>(parcel.readInt32());
    for (size_t i = 0; i < size; i++) {
        AString mediaType = AString::FromParcel(parcel);
        sp<Capabilities> caps = Capabilities::FromParcel(parcel);
        if (caps == NULL)
            return NULL;
        if (info != NULL) {
            info->mCaps.add(mediaType, caps);
        }
    }
    return info;
}

status_t MediaCodecInfo::writeToParcel(Parcel *parcel) const {
    mName.writeToParcel(parcel);
    mOwner.writeToParcel(parcel);
    parcel->writeInt32(mAttributes);
    parcel->writeUint32(mRank);
    parcel->writeInt32(mAliases.size());
    for (const AString &alias : mAliases) {
        alias.writeToParcel(parcel);
    }
    parcel->writeInt32(mCaps.size());
    for (size_t i = 0; i < mCaps.size(); i++) {
        mCaps.keyAt(i).writeToParcel(parcel);
        mCaps.valueAt(i)->writeToParcel(parcel);
    }
    return OK;
}

ssize_t MediaCodecInfo::getCapabilityIndex(const char *mediaType) const {
    if (mediaType) {
        for (size_t ix = 0; ix < mCaps.size(); ix++) {
            if (mCaps.keyAt(ix).equalsIgnoreCase(mediaType)) {
                return ix;
            }
        }
    }
    return -1;
}

MediaCodecInfo::MediaCodecInfo()
    : mAttributes((MediaCodecInfo::Attributes)0),
      mRank(0x100) {
}

void MediaCodecInfoWriter::setName(const char* name) {
    mInfo->mName = name;
}

void MediaCodecInfoWriter::addAlias(const char* name) {
    mInfo->mAliases.add(name);
}

void MediaCodecInfoWriter::setOwner(const char* owner) {
    mInfo->mOwner = owner;
}

void MediaCodecInfoWriter::setAttributes(
        typename std::underlying_type<MediaCodecInfo::Attributes>::type attributes) {
    mInfo->mAttributes = (MediaCodecInfo::Attributes)attributes;
}

void MediaCodecInfoWriter::setRank(uint32_t rank) {
    mInfo->mRank = rank;
}

std::unique_ptr<MediaCodecInfo::CapabilitiesWriter>
        MediaCodecInfoWriter::addMediaType(const char *mediaType) {
    ssize_t ix = mInfo->getCapabilityIndex(mediaType);
    if (ix >= 0) {
        return std::unique_ptr<MediaCodecInfo::CapabilitiesWriter>(
                new MediaCodecInfo::CapabilitiesWriter(
                mInfo->mCaps.valueAt(ix).get()));
    }
    sp<MediaCodecInfo::Capabilities> caps = new MediaCodecInfo::Capabilities();
    mInfo->mCaps.add(AString(mediaType), caps);
    return std::unique_ptr<MediaCodecInfo::CapabilitiesWriter>(
            new MediaCodecInfo::CapabilitiesWriter(caps.get()));
}

bool MediaCodecInfoWriter::removeMediaType(const char *mediaType) {
    ssize_t ix = mInfo->getCapabilityIndex(mediaType);
    if (ix >= 0) {
        mInfo->mCaps.removeItemsAt(ix);
        return true;
    }
    return false;
}

MediaCodecInfoWriter::MediaCodecInfoWriter(MediaCodecInfo* info) :
    mInfo(info) {
}

}  // namespace android
