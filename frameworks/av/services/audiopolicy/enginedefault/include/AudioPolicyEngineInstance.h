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

class AudioPolicyManagerInterface;

namespace android
{
namespace audio_policy
{

class Engine;

class EngineInstance
{
protected:
    EngineInstance();

public:
    virtual ~EngineInstance();

    /**
     * Get Audio Policy Engine instance.
     *
     * @return pointer to Route Manager Instance object.
     */
    static EngineInstance *getInstance();

    /**
     * Interface query.
     * The first client of an interface of the policy engine will start the singleton.
     *
     * @tparam RequestedInterface: interface that the client is wishing to retrieve.
     *
     * @return interface handle.
     */
    template <class RequestedInterface>
    RequestedInterface *queryInterface() const;

protected:
    /**
     * Get Audio Policy Engine instance.
     *
     * @return Audio Policy Engine singleton.
     */
    Engine *getEngine() const;

private:
    /* Copy facilities are put private to disable copy. */
    EngineInstance(const EngineInstance &object);
    EngineInstance &operator=(const EngineInstance &object);
};

/**
 * Limit template instantation to supported type interfaces.
 * Compile time error will claim if invalid interface is requested.
 */
template <>
AudioPolicyManagerInterface *EngineInstance::queryInterface() const;

} // namespace audio_policy
} // namespace android
