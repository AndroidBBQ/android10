/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef ANDROID_SERVERS_CAMERA_HEICENCODER_INFO_MANAGER_H
#define ANDROID_SERVERS_CAMERA_HEICENCODER_INFO_MANAGER_H

#include <unordered_map>
#include <utility>
#include <utils/Errors.h>
#include <utils/StrongPointer.h>

#include <media/IMediaCodecList.h>
#include <media/stagefright/foundation/AMessage.h>

namespace android {
namespace camera3 {

class HeicEncoderInfoManager {
public:
    static HeicEncoderInfoManager& getInstance() {
        static HeicEncoderInfoManager instance;
        return instance;
    }

    bool isSizeSupported(int32_t width, int32_t height,
            bool* useHeic, bool* useGrid, int64_t* stall, AString* hevcName) const;

    static const auto kGridWidth = 512;
    static const auto kGridHeight = 512;
private:
    struct SizePairHash {
        std::size_t operator () (const std::pair<int32_t,int32_t> &p) const {
            return p.first * 31 + p.second;
        }
    };

    typedef std::unordered_map<std::pair<int32_t, int32_t>,
            std::pair<int32_t, int32_t>, SizePairHash> FrameRateMaps;

    HeicEncoderInfoManager();
    virtual ~HeicEncoderInfoManager();

    status_t initialize();
    status_t getFrameRateMaps(sp<AMessage> details, FrameRateMaps* maps);
    status_t getCodecSizeRange(const char* codecName, sp<AMessage> details,
            std::pair<int32_t, int32_t>* minSize, std::pair<int32_t, int32_t>* maxSize,
            FrameRateMaps* frameRateMaps);
    FrameRateMaps::const_iterator findClosestSize(const FrameRateMaps& maps,
            int32_t width, int32_t height) const;
    sp<AMessage> getCodecDetails(sp<IMediaCodecList> codecsList, const char* name);
    bool getHevcCodecDetails(sp<IMediaCodecList> codecsList, const char* mime);

    bool mIsInited;
    std::pair<int32_t, int32_t> mMinSizeHeic, mMaxSizeHeic;
    std::pair<int32_t, int32_t> mMinSizeHevc, mMaxSizeHevc;
    bool mHasHEVC, mHasHEIC;
    AString mHevcName;
    FrameRateMaps mHeicFrameRateMaps, mHevcFrameRateMaps;
    bool mDisableGrid;

};

} // namespace camera3
} // namespace android

#endif // ANDROID_SERVERS_CAMERA_HEICENCODER_INFO_MANAGER_H
