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
#define LOG_TAG "MediaCodecListWriter"
#include <utils/Log.h>

#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/MediaCodecListWriter.h>
#include <media/MediaCodecInfo.h>

namespace android {

void MediaCodecListWriter::addGlobalSetting(
        const char* key, const char* value) {
    mGlobalSettings.emplace_back(key, value);
}

std::unique_ptr<MediaCodecInfoWriter>
        MediaCodecListWriter::addMediaCodecInfo() {
    sp<MediaCodecInfo> info = new MediaCodecInfo();
    mCodecInfos.push_back(info);
    return std::unique_ptr<MediaCodecInfoWriter>(
            new MediaCodecInfoWriter(info.get()));
}

std::unique_ptr<MediaCodecInfoWriter>
        MediaCodecListWriter::findMediaCodecInfo(const char *name) {
    for (const sp<MediaCodecInfo> &info : mCodecInfos) {
        if (!strcmp(info->getCodecName(), name)) {
            return std::unique_ptr<MediaCodecInfoWriter>(new MediaCodecInfoWriter(info.get()));
        }
    }
    return nullptr;
}

void MediaCodecListWriter::writeGlobalSettings(
        const sp<AMessage> &globalSettings) const {
    for (const std::pair<std::string, std::string> &kv : mGlobalSettings) {
        globalSettings->setString(kv.first.c_str(), kv.second.c_str());
    }
}

void MediaCodecListWriter::writeCodecInfos(
        std::vector<sp<MediaCodecInfo>> *codecInfos) const {
    for (const sp<MediaCodecInfo> &info : mCodecInfos) {
        codecInfos->push_back(info);
    }
}

}  // namespace android
