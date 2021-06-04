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

#include <stdint.h>
#include <string>
#include <utils/Errors.h>
#include <system/audio.h>
#include <utils/Log.h>

namespace android {
namespace audio_policy {

template <typename Key>
class Element
{
public:
    Element(const std::string &name)
        : mName(name)
    {}
    ~Element() {}

    /**
     * Returns identifier of this policy element
     *
     * @returns string representing the name of this policy element
     */
    const std::string &getName() const { return mName; }

    /**
    * Set the unique identifier for this policy element.
    *
    * @tparam Key type of the unique identifier.
    * @param[in] identifier to be set.
    *
    * @return NO_ERROR if the identifier is valid and set correctly, error code otherwise.
    */
    status_t setIdentifier(Key identifier)
    {
        mIdentifier = identifier;
        return NO_ERROR;
    }

    /**
     * @return the unique identifier of this policy element.
     */
    const Key &getIdentifier() const { return mIdentifier; }

    /**
     * A Policy element may implement getter/setter function for a given property.
     * Property may be audio_stream_type_t, audio_usage_t, audio_source_t
     * or a string.
     *
     * @tparam Property for which this policy element has setter / getter.
     * @return the property kept track by this policy base element.
     */
    template <typename Property>
    Property get() const;

    /**
     * A Policy element may implement getter/setter function for a given property.
     * Property may be audio_stream_type_t, audio_usage_t, audio_source_t
     * or a string.
     *
     * @tparam Property for which this policy element has setter / getter.
     *
     * @param[in] property value to be assigned for this policy base element.
     *
     * @return the property kept track by this policy base element.
     */
    template <typename Property>
    status_t set(Property property);

private:
    /* Copy facilities are put private to disable copy. */
    Element(const Element &object);
    Element &operator=(const Element &object);

    std::string mName; /**< Unique literal Identifier of a policy base element*/
    Key mIdentifier; /**< Unique numerical Identifier of a policy base element*/
};
} // namespace audio_policy
} // namespace android
