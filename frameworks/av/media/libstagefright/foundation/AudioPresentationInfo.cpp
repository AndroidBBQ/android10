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

// #define LOG_NDEBUG 0
#define LOG_TAG "AudioPresentationInfo"

#include <vector>

#include "AudioPresentationInfo.h"

#include <utils/Log.h>

namespace android {

void serializeAudioPresentations(const AudioPresentationCollection& presentations,
        std::ostream* serializedOutput) {
    uint32_t numPresentations = presentations.size();
    serializedOutput->write(reinterpret_cast<char*>(&numPresentations), sizeof(numPresentations));
    for (const auto& ap : presentations) {
        if (ap.mVersion == PRESENTATION_VERSION_1) {
            serializedOutput->write(
                    const_cast<char*>(reinterpret_cast<const char*>(&ap.mVersion)),
                    sizeof(ap.mVersion));
            serializedOutput->write(
                    const_cast<char*>(reinterpret_cast<const char*>(&ap.mPresentationId)),
                    sizeof(ap.mPresentationId));
            serializedOutput->write(
                    const_cast<char*>(reinterpret_cast<const char*>(&ap.mProgramId)),
                    sizeof(ap.mProgramId));

            uint32_t numLabels = ap.mLabels.size();
            serializedOutput->write(
                    const_cast<char*>(reinterpret_cast<const char*>(&numLabels)),
                    sizeof(numLabels));
            for (const auto& label : ap.mLabels) {
                uint32_t labelKeySize = label.first.size();
                serializedOutput->write(
                        const_cast<char*>(reinterpret_cast<const char*>(&labelKeySize)),
                        sizeof(labelKeySize));
                serializedOutput->write(label.first.c_str(), labelKeySize);

                uint32_t labelValSize = label.second.size();
                serializedOutput->write(
                        const_cast<char*>(reinterpret_cast<const char*>(&labelValSize)),
                        sizeof(labelValSize));
                serializedOutput->write(label.second.c_str(), labelValSize);
            }

            uint32_t langSize = ap.mLanguage.size();
            serializedOutput->write(
                    const_cast<char*>(reinterpret_cast<const char*>(&langSize)),
                    sizeof(langSize));
            serializedOutput->write(ap.mLanguage.c_str(), langSize);

            serializedOutput->write(
                    const_cast<char*>(reinterpret_cast<const char*>(&ap.mMasteringIndication)),
                    sizeof(ap.mMasteringIndication));
            serializedOutput->write(
                    const_cast<char*>(reinterpret_cast<const char*>(&ap.mAudioDescriptionAvailable)),
                    sizeof(ap.mAudioDescriptionAvailable));
            serializedOutput->write(
                    const_cast<char*>(reinterpret_cast<const char*>(&ap.mSpokenSubtitlesAvailable)),
                    sizeof(ap.mSpokenSubtitlesAvailable));
            serializedOutput->write(
                    const_cast<char*>(reinterpret_cast<const char*>(&ap.mDialogueEnhancementAvailable)),
                    sizeof(ap.mDialogueEnhancementAvailable));
        }
    }
}

status_t deserializeAudioPresentations(std::istream* serializedInput,
        AudioPresentationCollection *presentations) {
    uint32_t numPresentations;
    serializedInput->read(reinterpret_cast<char*>(&numPresentations), sizeof(numPresentations));
    for (uint32_t i = 0; i < numPresentations; ++i) {
        uint32_t version;
        serializedInput->read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version == PRESENTATION_VERSION_1) {
            AudioPresentationV1 ap;
            serializedInput->read(
                    reinterpret_cast<char*>(&ap.mPresentationId),
                    sizeof(ap.mPresentationId));
            serializedInput->read(reinterpret_cast<char*>(&ap.mProgramId), sizeof(ap.mProgramId));

            uint32_t numLabels;
            serializedInput->read(reinterpret_cast<char*>(&numLabels), sizeof(numLabels));
            for (uint32_t j = 0; j < numLabels; ++j) {
                uint32_t labelKeySize;
                serializedInput->read(reinterpret_cast<char*>(&labelKeySize), sizeof(labelKeySize));
                std::vector<char> labelKey(labelKeySize);
                serializedInput->read(labelKey.data(), labelKeySize);

                uint32_t labelValSize;
                serializedInput->read(reinterpret_cast<char*>(&labelValSize), sizeof(labelValSize));
                std::vector<char> labelVal(labelValSize);
                serializedInput->read(labelVal.data(), labelValSize);
                ap.mLabels.emplace(
                        std::string(reinterpret_cast<char*>(labelKey.data()), labelKeySize),
                        std::string(reinterpret_cast<char*>(labelVal.data()), labelValSize));
            }
            uint32_t languageSize;
            serializedInput->read(reinterpret_cast<char*>(&languageSize), sizeof(languageSize));
            std::vector<char> language(languageSize);
            serializedInput->read(language.data(), languageSize);
            ap.mLanguage = std::string(reinterpret_cast<char*>(language.data()), languageSize);
            serializedInput->read(reinterpret_cast<char*>(&ap.mMasteringIndication),
                                 sizeof(ap.mMasteringIndication));
            serializedInput->read(reinterpret_cast<char*>(&ap.mAudioDescriptionAvailable),
                                 sizeof(ap.mAudioDescriptionAvailable));
            serializedInput->read(reinterpret_cast<char*>(&ap.mSpokenSubtitlesAvailable),
                                 sizeof(ap.mSpokenSubtitlesAvailable));
            serializedInput->read(reinterpret_cast<char*>(&ap.mDialogueEnhancementAvailable),
                                 sizeof(ap.mDialogueEnhancementAvailable));
            presentations->push_back(std::move(ap));
        } else {
            ALOGE("Audio presentation info version is not supported");
            return INVALID_OPERATION;
        }
    }
    return OK;
}

}  // namespace android
