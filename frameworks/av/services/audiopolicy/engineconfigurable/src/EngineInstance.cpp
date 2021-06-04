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

#include <AudioPolicyManagerInterface.h>
#include <AudioPolicyPluginInterface.h>
#include "AudioPolicyEngineInstance.h"
#include "Engine.h"

using std::string;

namespace android {
namespace audio_policy {

EngineInstance::EngineInstance()
{
}

EngineInstance *EngineInstance::getInstance()
{
    static EngineInstance instance;
    return &instance;
}

EngineInstance::~EngineInstance()
{
}

Engine *EngineInstance::getEngine() const
{
    static Engine engine;
    return &engine;
}

template <>
AudioPolicyManagerInterface *EngineInstance::queryInterface() const
{
    return getEngine()->queryInterface<AudioPolicyManagerInterface>();
}

template <>
AudioPolicyPluginInterface *EngineInstance::queryInterface() const
{
    return getEngine()->queryInterface<AudioPolicyPluginInterface>();
}

} // namespace audio_policy
} // namespace android

