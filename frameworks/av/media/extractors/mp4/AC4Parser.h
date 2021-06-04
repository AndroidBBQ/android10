/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef AC4_PARSER_H_
#define AC4_PARSER_H_

#include <cstdint>
#include <map>
#include <string>

#include <media/stagefright/foundation/ABitReader.h>

namespace android {

class AC4Parser {
public:
    AC4Parser();
    virtual ~AC4Parser() { }

    virtual bool parse() = 0;

    struct AC4Presentation {
        int32_t mChannelMode = -1;
        int32_t mProgramID = -1;
        int32_t mGroupIndex = -1;

        // TS 103 190-1 v1.2.1 4.3.3.8.1
        enum ContentClassifiers {
            kCompleteMain,
            kMusicAndEffects,
            kVisuallyImpaired,
            kHearingImpaired,
            kDialog,
            kCommentary,
            kEmergency,
            kVoiceOver
        };

        uint32_t mContentClassifier = kCompleteMain;

        // ETSI TS 103 190-2 V1.1.1 (2015-09) Table 79: channel_mode
        enum InputChannelMode {
            kChannelMode_Mono,
            kChannelMode_Stereo,
            kChannelMode_3_0,
            kChannelMode_5_0,
            kChannelMode_5_1,
            kChannelMode_7_0_34,
            kChannelMode_7_1_34,
            kChannelMode_7_0_52,
            kChannelMode_7_1_52,
            kChannelMode_7_0_322,
            kChannelMode_7_1_322,
            kChannelMode_7_0_4,
            kChannelMode_7_1_4,
            kChannelMode_9_0_4,
            kChannelMode_9_1_4,
            kChannelMode_22_2,
            kChannelMode_Reserved,
        };

        bool mHasDialogEnhancements = false;
        bool mPreVirtualized = false;
        bool mEnabled = true;

        std::string mLanguage;
        std::string mDescription;
    };
    typedef std::map<uint32_t, AC4Presentation> AC4Presentations;

    const AC4Presentations& getPresentations() const { return mPresentations; }

protected:
    AC4Presentations mPresentations;
};

class AC4DSIParser: public AC4Parser {
public:
    explicit AC4DSIParser(ABitReader &br);
    virtual ~AC4DSIParser() { }

    bool parse();

private:
    bool parseSubstreamDSI(uint32_t presentationID, uint32_t substreamID);
    bool parseSubstreamGroupDSI(uint32_t presentationID, uint32_t groupID);
    bool parseLanguageTag(uint32_t presentationID, uint32_t substreamID);
    bool parseBitrateDsi();

    uint64_t mDSISize;
    ABitReader& mBitReader;
};

};

#endif  // AC4_PARSER_H_
