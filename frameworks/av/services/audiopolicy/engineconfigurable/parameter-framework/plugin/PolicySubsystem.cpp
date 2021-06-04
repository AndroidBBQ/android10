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

#include "PolicySubsystem.h"
#include "SubsystemObjectFactory.h"
#include "PolicyMappingKeys.h"
#include "Stream.h"
#include "InputSource.h"
#include "ProductStrategy.h"
#include <AudioPolicyPluginInterface.h>
#include <AudioPolicyEngineInstance.h>
#include <utils/Log.h>

using android::audio_policy::EngineInstance;

const char *const PolicySubsystem::mKeyName = "Name";
const char *const PolicySubsystem::mKeyIdentifier = "Identifier";
const char *const PolicySubsystem::mKeyCategory = "Category";
const char *const PolicySubsystem::mKeyAmend1 = "Amend1";
const char *const PolicySubsystem::mKeyAmend2 = "Amend2";
const char *const PolicySubsystem::mKeyAmend3 = "Amend3";


const char *const PolicySubsystem::mStreamComponentName = "Stream";
const char *const PolicySubsystem::mInputSourceComponentName = "InputSource";
const char *const PolicySubsystem::mProductStrategyComponentName = "ProductStrategy";

PolicySubsystem::PolicySubsystem(const std::string &name, core::log::Logger &logger)
    : CSubsystem(name, logger),
      mPluginInterface(NULL)
{
    // Try to connect a Plugin Interface from Audio Policy Engine
    EngineInstance *engineInstance = EngineInstance::getInstance();

    ALOG_ASSERT(engineInstance != NULL, "NULL Plugin Interface");

    // Retrieve the Route Interface
    mPluginInterface = engineInstance->queryInterface<android::AudioPolicyPluginInterface>();
    ALOG_ASSERT(mPluginInterface != NULL, "NULL Plugin Interface");

    // Provide mapping keys to the core, necessary when parsing the XML Structure files.
    addContextMappingKey(mKeyName);
    addContextMappingKey(mKeyCategory);
    addContextMappingKey(mKeyIdentifier);
    addContextMappingKey(mKeyAmend1);
    addContextMappingKey(mKeyAmend2);
    addContextMappingKey(mKeyAmend3);

    // Provide creators to upper layer
    addSubsystemObjectFactory(
        new TSubsystemObjectFactory<Stream>(
            mStreamComponentName,
            (1 << MappingKeyName))
        );
    addSubsystemObjectFactory(
        new TSubsystemObjectFactory<InputSource>(
            mInputSourceComponentName,
            (1 << MappingKeyName))
        );
    addSubsystemObjectFactory(
        new TSubsystemObjectFactory<ProductStrategy>(
            mProductStrategyComponentName, (1 << MappingKeyName))
        );
}

// Retrieve Route interface
android::AudioPolicyPluginInterface *PolicySubsystem::getPolicyPluginInterface() const
{
    ALOG_ASSERT(mPluginInterface != NULL, "NULL Plugin Interface");
    return mPluginInterface;
}
