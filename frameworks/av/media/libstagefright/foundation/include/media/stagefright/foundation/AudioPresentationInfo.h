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

#ifndef AUDIO_PRESENTATION_INFO_H_
#define AUDIO_PRESENTATION_INFO_H_

#include <map>
#include <sstream>
#include <stdint.h>
#include <vector>

#include <utils/Errors.h>

namespace android {

enum AudioPresentationVersion {
    PRESENTATION_VERSION_UNDEFINED = 0,
    PRESENTATION_VERSION_1,
};

enum MasteringIndication {
    MASTERING_NOT_INDICATED,
    MASTERED_FOR_STEREO,
    MASTERED_FOR_SURROUND,
    MASTERED_FOR_3D,
    MASTERED_FOR_HEADPHONE,
};

struct AudioPresentation {
    uint32_t mVersion = PRESENTATION_VERSION_UNDEFINED;
    int32_t mPresentationId = -1;
    int32_t mProgramId = -1;
    std::map<std::string, std::string> mLabels;
    std::string mLanguage;
    MasteringIndication mMasteringIndication = MASTERING_NOT_INDICATED;
    bool mAudioDescriptionAvailable = false;
    bool mSpokenSubtitlesAvailable = false;
    bool mDialogueEnhancementAvailable = false;
};

struct AudioPresentationV1 : public AudioPresentation {
    AudioPresentationV1() {
        mVersion = PRESENTATION_VERSION_1;
    }
};

typedef std::vector<AudioPresentation> AudioPresentationCollection;

void serializeAudioPresentations(const AudioPresentationCollection& presentations,
                                               std::ostream* serializedOutput);
status_t deserializeAudioPresentations(std::istream* serializedInput,
                                                AudioPresentationCollection *presentations);
}  // namespace android

#endif  // AUDIO_PRESENTATION_INFO_H_
