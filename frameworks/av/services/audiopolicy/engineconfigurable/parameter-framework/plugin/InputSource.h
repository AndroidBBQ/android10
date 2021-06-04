/*
 * Copyright (C) 2015 The Android Open Source Project
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

#pragma once

#include "FormattedSubsystemObject.h"
#include "InstanceConfigurableElement.h"
#include "MappingContext.h"
#include <AudioPolicyPluginInterface.h>
#include <string>

class PolicySubsystem;

class InputSource : public CFormattedSubsystemObject
{
public:
    InputSource(const std::string &mappingValue,
                CInstanceConfigurableElement *instanceConfigurableElement,
                const CMappingContext &context,
                core::log::Logger& logger);

protected:
    virtual bool sendToHW(std::string &error);

private:
    const PolicySubsystem *mPolicySubsystem; /**< Route subsytem plugin. */

    /**
     * Interface to communicate with Audio Policy Engine.
     */
    android::AudioPolicyPluginInterface *mPolicyPluginInterface;

    audio_source_t mId; /**< input source identifier to link with audio.h. */
};
