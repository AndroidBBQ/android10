/*
 * Copyright (C) 2009 The Android Open Source Project
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
#define LOG_TAG "MediaExtractor"
#include <utils/Log.h>
#include <pwd.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaExtractor.h>
#include <media/stagefright/MetaData.h>
#include <media/stagefright/Utils.h>
#include <media/NdkMediaFormatPriv.h>
#include <media/NdkMediaErrorPriv.h>

namespace android {

MediaExtractor::MediaExtractor() {
    if (!LOG_NDEBUG) {
        uid_t uid = getuid();
        struct passwd *pw = getpwuid(uid);
        ALOGV("extractor created in uid: %d (%s)", getuid(), pw->pw_name);
    }
}

MediaExtractor::~MediaExtractor() {}

uint32_t MediaExtractor::flags() const {
    return CAN_SEEK_BACKWARD | CAN_SEEK_FORWARD | CAN_PAUSE | CAN_SEEK;
}

// --------------------------------------------------------------------------------
MediaExtractorCUnwrapper::MediaExtractorCUnwrapper(CMediaExtractor *plugin) {
    this->plugin = plugin;
}

MediaExtractorCUnwrapper::~MediaExtractorCUnwrapper() {
    plugin->free(plugin->data);
    free(plugin);
}

size_t MediaExtractorCUnwrapper::countTracks() {
    return plugin->countTracks(plugin->data);
}

MediaTrack *MediaExtractorCUnwrapper::getTrack(size_t index) {
    return MediaTrackCUnwrapper::create(plugin->getTrack(plugin->data, index));
}

status_t MediaExtractorCUnwrapper::getTrackMetaData(
        MetaDataBase& meta, size_t index, uint32_t flags) {
    sp<AMessage> msg = new AMessage();
    AMediaFormat *format =  AMediaFormat_fromMsg(&msg);
    media_status_t ret = plugin->getTrackMetaData(plugin->data, format, index, flags);
    sp<MetaData> newMeta = new MetaData();
    convertMessageToMetaData(msg, newMeta);
    delete format;
    meta = *newMeta;
    return reverse_translate_error(ret);
}

status_t MediaExtractorCUnwrapper::getMetaData(MetaDataBase& meta) {
    sp<AMessage> msg = new AMessage();
    AMediaFormat *format =  AMediaFormat_fromMsg(&msg);
    media_status_t ret = plugin->getMetaData(plugin->data, format);
    sp<MetaData> newMeta = new MetaData();
    convertMessageToMetaData(msg, newMeta);
    delete format;
    meta = *newMeta;
    return reverse_translate_error(ret);
}

const char * MediaExtractorCUnwrapper::name() {
    return plugin->name(plugin->data);
}

uint32_t MediaExtractorCUnwrapper::flags() const {
    return plugin->flags(plugin->data);
}

status_t MediaExtractorCUnwrapper::setMediaCas(const uint8_t* casToken, size_t size) {
    return plugin->setMediaCas(plugin->data, casToken, size);
}

}  // namespace android
