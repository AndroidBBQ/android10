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

//#define LOG_NDEBUG 0
#define LOG_TAG "NuMediaExtractor"
#include <utils/Log.h>

#include <media/stagefright/NuMediaExtractor.h>

#include "include/ESDS.h"

#include <media/DataSource.h>
#include <media/MediaSource.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/DataSourceFactory.h>
#include <media/stagefright/FileSource.h>
#include <media/stagefright/MediaBuffer.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/MediaExtractorFactory.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>

namespace android {

NuMediaExtractor::Sample::Sample()
    : mBuffer(NULL),
      mSampleTimeUs(-1LL) {
}

NuMediaExtractor::Sample::Sample(MediaBufferBase *buffer, int64_t timeUs)
    : mBuffer(buffer),
      mSampleTimeUs(timeUs) {
}

NuMediaExtractor::NuMediaExtractor()
    : mTotalBitrate(-1LL),
      mDurationUs(-1LL) {
}

NuMediaExtractor::~NuMediaExtractor() {
    releaseAllTrackSamples();

    for (size_t i = 0; i < mSelectedTracks.size(); ++i) {
        TrackInfo *info = &mSelectedTracks.editItemAt(i);

        status_t err = info->mSource->stop();
        ALOGE_IF(err != OK, "error %d stopping track %zu", err, i);
    }

    mSelectedTracks.clear();
    if (mDataSource != NULL) {
        mDataSource->close();
    }
}

status_t NuMediaExtractor::setDataSource(
        const sp<MediaHTTPService> &httpService,
        const char *path,
        const KeyedVector<String8, String8> *headers) {
    Mutex::Autolock autoLock(mLock);

    if (mImpl != NULL || path == NULL) {
        return -EINVAL;
    }

    sp<DataSource> dataSource =
        DataSourceFactory::CreateFromURI(httpService, path, headers);

    if (dataSource == NULL) {
        return -ENOENT;
    }

    mImpl = MediaExtractorFactory::Create(dataSource);

    if (mImpl == NULL) {
        return ERROR_UNSUPPORTED;
    }

    if (!mCasToken.empty()) {
        mImpl->setMediaCas(mCasToken);
    }

    status_t err = updateDurationAndBitrate();
    if (err == OK) {
        mDataSource = dataSource;
    }

    return OK;
}

status_t NuMediaExtractor::setDataSource(int fd, off64_t offset, off64_t size) {

    ALOGV("setDataSource fd=%d (%s), offset=%lld, length=%lld",
            fd, nameForFd(fd).c_str(), (long long) offset, (long long) size);

    Mutex::Autolock autoLock(mLock);

    if (mImpl != NULL) {
        return -EINVAL;
    }

    sp<FileSource> fileSource = new FileSource(dup(fd), offset, size);

    status_t err = fileSource->initCheck();
    if (err != OK) {
        return err;
    }

    mImpl = MediaExtractorFactory::Create(fileSource);

    if (mImpl == NULL) {
        return ERROR_UNSUPPORTED;
    }

    if (!mCasToken.empty()) {
        mImpl->setMediaCas(mCasToken);
    }

    err = updateDurationAndBitrate();
    if (err == OK) {
        mDataSource = fileSource;
    }

    return OK;
}

status_t NuMediaExtractor::setDataSource(const sp<DataSource> &source) {
    Mutex::Autolock autoLock(mLock);

    if (mImpl != NULL) {
        return -EINVAL;
    }

    status_t err = source->initCheck();
    if (err != OK) {
        return err;
    }

    mImpl = MediaExtractorFactory::Create(source);

    if (mImpl == NULL) {
        return ERROR_UNSUPPORTED;
    }

    if (!mCasToken.empty()) {
        mImpl->setMediaCas(mCasToken);
    }

    err = updateDurationAndBitrate();
    if (err == OK) {
        mDataSource = source;
    }

    return err;
}

static String8 arrayToString(const std::vector<uint8_t> &array) {
    String8 result;
    for (size_t i = 0; i < array.size(); i++) {
        result.appendFormat("%02x ", array[i]);
    }
    if (result.isEmpty()) {
        result.append("(null)");
    }
    return result;
}

status_t NuMediaExtractor::setMediaCas(const HInterfaceToken &casToken) {
    ALOGV("setMediaCas: casToken={%s}", arrayToString(casToken).c_str());

    Mutex::Autolock autoLock(mLock);

    if (casToken.empty()) {
        return BAD_VALUE;
    }

    mCasToken = casToken;

    if (mImpl != NULL) {
        mImpl->setMediaCas(casToken);
        status_t err = updateDurationAndBitrate();
        if (err != OK) {
            return err;
        }
    }

    return OK;
}

status_t NuMediaExtractor::updateDurationAndBitrate() {
    if (mImpl->countTracks() > kMaxTrackCount) {
        return ERROR_UNSUPPORTED;
    }

    mTotalBitrate = 0LL;
    mDurationUs = -1LL;

    for (size_t i = 0; i < mImpl->countTracks(); ++i) {
        sp<MetaData> meta = mImpl->getTrackMetaData(i);
        if (meta == NULL) {
            ALOGW("no metadata for track %zu", i);
            continue;
        }

        int32_t bitrate;
        if (!meta->findInt32(kKeyBitRate, &bitrate)) {
            const char *mime;
            CHECK(meta->findCString(kKeyMIMEType, &mime));
            ALOGV("track of type '%s' does not publish bitrate", mime);

            mTotalBitrate = -1LL;
        } else if (mTotalBitrate >= 0LL) {
            mTotalBitrate += bitrate;
        }

        int64_t durationUs;
        if (meta->findInt64(kKeyDuration, &durationUs)
                && durationUs > mDurationUs) {
            mDurationUs = durationUs;
        }
    }
    return OK;
}

size_t NuMediaExtractor::countTracks() const {
    Mutex::Autolock autoLock(mLock);

    return mImpl == NULL ? 0 : mImpl->countTracks();
}

status_t NuMediaExtractor::getTrackFormat(
        size_t index, sp<AMessage> *format, uint32_t flags) const {
    Mutex::Autolock autoLock(mLock);

    *format = NULL;

    if (mImpl == NULL) {
        return -EINVAL;
    }

    if (index >= mImpl->countTracks()) {
        return -ERANGE;
    }

    sp<MetaData> meta = mImpl->getTrackMetaData(index, flags);
    // Extractors either support trackID-s or not, so either all tracks have trackIDs or none.
    // Generate trackID if missing.
    int32_t trackID;
    if (meta != NULL && !meta->findInt32(kKeyTrackID, &trackID)) {
        meta->setInt32(kKeyTrackID, (int32_t)index + 1);
    }
    return convertMetaDataToMessage(meta, format);
}

status_t NuMediaExtractor::getFileFormat(sp<AMessage> *format) const {
    Mutex::Autolock autoLock(mLock);

    *format = NULL;

    if (mImpl == NULL) {
        return -EINVAL;
    }

    sp<MetaData> meta = mImpl->getMetaData();

    const char *mime;
    CHECK(meta->findCString(kKeyMIMEType, &mime));
    *format = new AMessage();
    (*format)->setString("mime", mime);

    uint32_t type;
    const void *pssh;
    size_t psshsize;
    if (meta->findData(kKeyPssh, &type, &pssh, &psshsize)) {
        sp<ABuffer> buf = new ABuffer(psshsize);
        memcpy(buf->data(), pssh, psshsize);
        (*format)->setBuffer("pssh", buf);
    }

    return OK;
}

status_t NuMediaExtractor::getExifOffsetSize(off64_t *offset, size_t *size) const {
    Mutex::Autolock autoLock(mLock);

    if (mImpl == NULL) {
        return -EINVAL;
    }

    sp<MetaData> meta = mImpl->getMetaData();

    int64_t exifOffset, exifSize;
    if (meta->findInt64(kKeyExifOffset, &exifOffset)
     && meta->findInt64(kKeyExifSize, &exifSize)) {
        *offset = (off64_t) exifOffset;
        *size = (size_t) exifSize;

        return OK;
    }
    return ERROR_UNSUPPORTED;
}

status_t NuMediaExtractor::selectTrack(size_t index,
        int64_t startTimeUs, MediaSource::ReadOptions::SeekMode mode) {
    Mutex::Autolock autoLock(mLock);

    if (mImpl == NULL) {
        return -EINVAL;
    }

    if (index >= mImpl->countTracks()) {
        return -ERANGE;
    }

    for (size_t i = 0; i < mSelectedTracks.size(); ++i) {
        TrackInfo *info = &mSelectedTracks.editItemAt(i);

        if (info->mTrackIndex == index) {
            // This track has already been selected.
            return OK;
        }
    }

    sp<IMediaSource> source = mImpl->getTrack(index);

    if (source == nullptr) {
        ALOGE("track %zu is empty", index);
        return ERROR_MALFORMED;
    }

    status_t ret = source->start();
    if (ret != OK) {
        ALOGE("track %zu failed to start", index);
        return ret;
    }

    sp<MetaData> meta = source->getFormat();
    if (meta == NULL) {
        ALOGE("track %zu has no meta data", index);
        return ERROR_MALFORMED;
    }

    const char *mime;
    if (!meta->findCString(kKeyMIMEType, &mime)) {
        ALOGE("track %zu has no mime type in meta data", index);
        return ERROR_MALFORMED;
    }
    ALOGV("selectTrack, track[%zu]: %s", index, mime);

    mSelectedTracks.push();
    TrackInfo *info = &mSelectedTracks.editItemAt(mSelectedTracks.size() - 1);

    info->mSource = source;
    info->mTrackIndex = index;
    if (!strncasecmp(mime, "audio/", 6)) {
        info->mTrackType = MEDIA_TRACK_TYPE_AUDIO;
        info->mMaxFetchCount = 64;
    } else if (!strncasecmp(mime, "video/", 6)) {
        info->mTrackType = MEDIA_TRACK_TYPE_VIDEO;
        info->mMaxFetchCount = 8;
    } else {
        info->mTrackType = MEDIA_TRACK_TYPE_UNKNOWN;
        info->mMaxFetchCount = 1;
    }
    info->mFinalResult = OK;
    releaseTrackSamples(info);
    info->mTrackFlags = 0;

    if (!strcasecmp(mime, MEDIA_MIMETYPE_AUDIO_VORBIS)) {
        info->mTrackFlags |= kIsVorbis;
    }

    if (startTimeUs >= 0) {
        fetchTrackSamples(info, startTimeUs, mode);
    }

    return OK;
}

status_t NuMediaExtractor::unselectTrack(size_t index) {
    Mutex::Autolock autoLock(mLock);

    if (mImpl == NULL) {
        return -EINVAL;
    }

    if (index >= mImpl->countTracks()) {
        return -ERANGE;
    }

    size_t i;
    for (i = 0; i < mSelectedTracks.size(); ++i) {
        TrackInfo *info = &mSelectedTracks.editItemAt(i);

        if (info->mTrackIndex == index) {
            break;
        }
    }

    if (i == mSelectedTracks.size()) {
        // Not selected.
        return OK;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(i);

    releaseTrackSamples(info);

    CHECK_EQ((status_t)OK, info->mSource->stop());

    mSelectedTracks.removeAt(i);

    return OK;
}

void NuMediaExtractor::releaseTrackSamples(TrackInfo *info) {
    if (info == NULL) {
        return;
    }

    auto it = info->mSamples.begin();
    while (it != info->mSamples.end()) {
        if (it->mBuffer != NULL) {
            it->mBuffer->release();
        }
        it = info->mSamples.erase(it);
    }
}

void NuMediaExtractor::releaseAllTrackSamples() {
    for (size_t i = 0; i < mSelectedTracks.size(); ++i) {
        releaseTrackSamples(&mSelectedTracks.editItemAt(i));
    }
}

ssize_t NuMediaExtractor::fetchAllTrackSamples(
        int64_t seekTimeUs, MediaSource::ReadOptions::SeekMode mode) {
    TrackInfo *minInfo = NULL;
    ssize_t minIndex = ERROR_END_OF_STREAM;

    for (size_t i = 0; i < mSelectedTracks.size(); ++i) {
        TrackInfo *info = &mSelectedTracks.editItemAt(i);
        fetchTrackSamples(info, seekTimeUs, mode);

        status_t err = info->mFinalResult;
        if (err != OK && err != ERROR_END_OF_STREAM && info->mSamples.empty()) {
            return err;
        }

        if (info->mSamples.empty()) {
            continue;
        }

        if (minInfo == NULL) {
            minInfo = info;
            minIndex = i;
        } else {
            auto it = info->mSamples.begin();
            auto itMin = minInfo->mSamples.begin();
            if (it->mSampleTimeUs < itMin->mSampleTimeUs) {
                minInfo = info;
                minIndex = i;
            }
        }
    }

    return minIndex;
}

void NuMediaExtractor::fetchTrackSamples(TrackInfo *info,
        int64_t seekTimeUs, MediaSource::ReadOptions::SeekMode mode) {
    if (info == NULL) {
        return;
    }

    MediaSource::ReadOptions options;
    if (seekTimeUs >= 0LL) {
        options.setSeekTo(seekTimeUs, mode);
        info->mFinalResult = OK;
        releaseTrackSamples(info);
    } else if (info->mFinalResult != OK || !info->mSamples.empty()) {
        return;
    }

    status_t err = OK;
    Vector<MediaBufferBase *> mediaBuffers;
    if (info->mSource->supportReadMultiple()) {
        options.setNonBlocking();
        err = info->mSource->readMultiple(&mediaBuffers, info->mMaxFetchCount, &options);
    } else {
        MediaBufferBase *mbuf = NULL;
        err = info->mSource->read(&mbuf, &options);
        if (err == OK && mbuf != NULL) {
            mediaBuffers.push_back(mbuf);
        }
    }

    info->mFinalResult = err;
    if (err != OK && err != ERROR_END_OF_STREAM) {
        ALOGW("read on track %zu failed with error %d", info->mTrackIndex, err);
    }

    size_t count = mediaBuffers.size();
    bool releaseRemaining = false;
    for (size_t id = 0; id < count; ++id) {
        int64_t timeUs;
        MediaBufferBase *mbuf = mediaBuffers[id];
        if (mbuf == NULL) {
            continue;
        }
        if (releaseRemaining) {
            mbuf->release();
            continue;
        }
        if (mbuf->meta_data().findInt64(kKeyTime, &timeUs)) {
            info->mSamples.emplace_back(mbuf, timeUs);
        } else {
            mbuf->meta_data().dumpToLog();
            info->mFinalResult = ERROR_MALFORMED;
            mbuf->release();
            releaseRemaining = true;
        }
    }
}

status_t NuMediaExtractor::seekTo(
        int64_t timeUs, MediaSource::ReadOptions::SeekMode mode) {
    Mutex::Autolock autoLock(mLock);

    ssize_t minIndex = fetchAllTrackSamples(timeUs, mode);

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }

    return OK;
}

status_t NuMediaExtractor::advance() {
    Mutex::Autolock autoLock(mLock);

    ssize_t minIndex = fetchAllTrackSamples();

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(minIndex);

    if (info == NULL || info->mSamples.empty()) {
        return ERROR_END_OF_STREAM;
    }

    auto it = info->mSamples.begin();
    if (it->mBuffer != NULL) {
        it->mBuffer->release();
    }
    info->mSamples.erase(it);

    if (info->mSamples.empty()) {
        minIndex = fetchAllTrackSamples();
        if (minIndex < 0) {
            return ERROR_END_OF_STREAM;
        }
        info = &mSelectedTracks.editItemAt(minIndex);
        if (info == NULL || info->mSamples.empty()) {
            return ERROR_END_OF_STREAM;
        }
    }
    return OK;
}

status_t NuMediaExtractor::appendVorbisNumPageSamples(
        MediaBufferBase *mbuf, const sp<ABuffer> &buffer) {
    int32_t numPageSamples;
    if (!mbuf->meta_data().findInt32(
            kKeyValidSamples, &numPageSamples)) {
        numPageSamples = -1;
    }

    memcpy((uint8_t *)buffer->data() + mbuf->range_length(),
           &numPageSamples,
           sizeof(numPageSamples));

    uint32_t type;
    const void *data;
    size_t size, size2;
    if (mbuf->meta_data().findData(kKeyEncryptedSizes, &type, &data, &size)) {
        // Signal numPageSamples (a plain int32_t) is appended at the end,
        // i.e. sizeof(numPageSamples) plain bytes + 0 encrypted bytes
        if (SIZE_MAX - size < sizeof(int32_t)) {
            return -ENOMEM;
        }

        size_t newSize = size + sizeof(int32_t);
        sp<ABuffer> abuf = new ABuffer(newSize);
        uint8_t *adata = static_cast<uint8_t *>(abuf->data());
        if (adata == NULL) {
            return -ENOMEM;
        }

        // append 0 to encrypted sizes
        int32_t zero = 0;
        memcpy(adata, data, size);
        memcpy(adata + size, &zero, sizeof(zero));
        mbuf->meta_data().setData(kKeyEncryptedSizes, type, adata, newSize);

        if (mbuf->meta_data().findData(kKeyPlainSizes, &type, &data, &size2)) {
            if (size2 != size) {
                return ERROR_MALFORMED;
            }
            memcpy(adata, data, size);
        } else {
            // if sample meta data does not include plain size array, assume filled with zeros,
            // i.e. entire buffer is encrypted
            memset(adata, 0, size);
        }
        // append sizeof(numPageSamples) to plain sizes.
        int32_t int32Size = sizeof(numPageSamples);
        memcpy(adata + size, &int32Size, sizeof(int32Size));
        mbuf->meta_data().setData(kKeyPlainSizes, type, adata, newSize);
    }

    return OK;
}

status_t NuMediaExtractor::readSampleData(const sp<ABuffer> &buffer) {
    Mutex::Autolock autoLock(mLock);

    ssize_t minIndex = fetchAllTrackSamples();

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(minIndex);

    auto it = info->mSamples.begin();
    size_t sampleSize = it->mBuffer->range_length();

    if (info->mTrackFlags & kIsVorbis) {
        // Each sample's data is suffixed by the number of page samples
        // or -1 if not available.
        sampleSize += sizeof(int32_t);
    }

    if (buffer->capacity() < sampleSize) {
        return -ENOMEM;
    }

    const uint8_t *src =
        (const uint8_t *)it->mBuffer->data()
            + it->mBuffer->range_offset();

    memcpy((uint8_t *)buffer->data(), src, it->mBuffer->range_length());

    status_t err = OK;
    if (info->mTrackFlags & kIsVorbis) {
        err = appendVorbisNumPageSamples(it->mBuffer, buffer);
    }

    if (err == OK) {
        buffer->setRange(0, sampleSize);
    }

    return err;
}

status_t NuMediaExtractor::getSampleSize(size_t *sampleSize) {
    Mutex::Autolock autoLock(mLock);

    ssize_t minIndex = fetchAllTrackSamples();

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(minIndex);
    auto it = info->mSamples.begin();
    *sampleSize = it->mBuffer->range_length();

    if (info->mTrackFlags & kIsVorbis) {
        // Each sample's data is suffixed by the number of page samples
        // or -1 if not available.
        *sampleSize += sizeof(int32_t);
    }

    return OK;
}

status_t NuMediaExtractor::getSampleTrackIndex(size_t *trackIndex) {
    Mutex::Autolock autoLock(mLock);

    ssize_t minIndex = fetchAllTrackSamples();

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(minIndex);
    *trackIndex = info->mTrackIndex;

    return OK;
}

status_t NuMediaExtractor::getSampleTime(int64_t *sampleTimeUs) {
    Mutex::Autolock autoLock(mLock);

    ssize_t minIndex = fetchAllTrackSamples();

    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(minIndex);
    *sampleTimeUs = info->mSamples.begin()->mSampleTimeUs;

    return OK;
}

status_t NuMediaExtractor::getSampleMeta(sp<MetaData> *sampleMeta) {
    Mutex::Autolock autoLock(mLock);

    *sampleMeta = NULL;

    ssize_t minIndex = fetchAllTrackSamples();

    if (minIndex < 0) {
        status_t err = minIndex;
        return err;
    }

    TrackInfo *info = &mSelectedTracks.editItemAt(minIndex);
    *sampleMeta = new MetaData(info->mSamples.begin()->mBuffer->meta_data());

    return OK;
}

status_t NuMediaExtractor::getMetrics(Parcel *reply) {
    if (mImpl == NULL) {
        return -EINVAL;
    }
    status_t status = mImpl->getMetrics(reply);
    return status;
}

bool NuMediaExtractor::getTotalBitrate(int64_t *bitrate) const {
    if (mTotalBitrate > 0) {
        *bitrate = mTotalBitrate;
        return true;
    }

    off64_t size;
    if (mDurationUs > 0 && mDataSource->getSize(&size) == OK) {
        *bitrate = size * 8000000LL / mDurationUs;  // in bits/sec
        return true;
    }

    return false;
}

// Returns true iff cached duration is available/applicable.
bool NuMediaExtractor::getCachedDuration(
        int64_t *durationUs, bool *eos) const {
    Mutex::Autolock autoLock(mLock);

    off64_t cachedDataRemaining = -1;
    status_t finalStatus = mDataSource->getAvailableSize(-1, &cachedDataRemaining);

    int64_t bitrate;
    if (cachedDataRemaining >= 0
            && getTotalBitrate(&bitrate)) {
        *durationUs = cachedDataRemaining * 8000000ll / bitrate;
        *eos = (finalStatus != OK);
        return true;
    }

    return false;
}

// Return OK if we have received an audio presentation info.
// Return ERROR_END_OF_STREAM if no tracks are available.
// Return ERROR_UNSUPPORTED if the track has no audio presentation.
// Return INVALID_OPERATION if audio presentation metadata version does not match.
status_t NuMediaExtractor::getAudioPresentations(
        size_t trackIndex, AudioPresentationCollection *presentations) {
    Mutex::Autolock autoLock(mLock);
    ssize_t minIndex = fetchAllTrackSamples();
    if (minIndex < 0) {
        return ERROR_END_OF_STREAM;
    }
    for (size_t i = 0; i < mSelectedTracks.size(); ++i) {
        TrackInfo *info = &mSelectedTracks.editItemAt(i);

        if (info->mTrackIndex == trackIndex) {
            sp<MetaData> meta = new MetaData(info->mSamples.begin()->mBuffer->meta_data());

            uint32_t type;
            const void *data;
            size_t size;
            if (meta != NULL && meta->findData(kKeyAudioPresentationInfo, &type, &data, &size)) {
                std::istringstream inStream(std::string(static_cast<const char*>(data), size));
                return deserializeAudioPresentations(&inStream, presentations);
            }
            ALOGV("Track %zu does not contain any audio presentation", trackIndex);
            return ERROR_UNSUPPORTED;
        }
    }
    ALOGV("Source does not contain any audio presentation");
    return ERROR_UNSUPPORTED;
}

}  // namespace android
