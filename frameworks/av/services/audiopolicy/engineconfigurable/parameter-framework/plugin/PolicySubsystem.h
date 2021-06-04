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

#include "Subsystem.h"
#include <string>

namespace android {

class AudioPolicyPluginInterface;

}

class PolicySubsystem : public CSubsystem
{
public:
    PolicySubsystem(const std::string &strName, core::log::Logger& logger);

    /**
     * Retrieve Route Manager interface.
     *
     * @return RouteManager interface for the route plugin.
     */
    android::AudioPolicyPluginInterface *getPolicyPluginInterface() const;

private:
    /* Copy facilities are put private to disable copy. */
    PolicySubsystem(const PolicySubsystem &object);
    PolicySubsystem &operator=(const PolicySubsystem &object);

    android::AudioPolicyPluginInterface *mPluginInterface; /**< Audio Policy Plugin Interface. */

    static const char *const mKeyName; /**< name key mapping string. */
    static const char *const mKeyIdentifier;
    static const char *const mKeyCategory;

    static const char *const mKeyAmend1; /**< amend1 key mapping string. */
    static const char *const mKeyAmend2; /**< amend2 key mapping string. */
    static const char *const mKeyAmend3; /**< amend3 key mapping string. */

    static const char *const mStreamComponentName;
    static const char *const mInputSourceComponentName;
    static const char *const mProductStrategyComponentName;
};
