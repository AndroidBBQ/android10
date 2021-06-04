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

#include "Element.h"
#include "Stream.h"
#include "InputSource.h"
#include <utils/Errors.h>
#include <system/audio.h>
#include <utils/Log.h>
#include <map>
#include <stdint.h>
#include <string>

namespace android {
namespace audio_policy {

/**
 * Collection of policy element as a map indexed with a their UID type.
 *
 * @tparam Key type of the policy element indexing the collection.
 *         Policy Element supported are:
 *                      - Strategy
 *                      - Stream
 *                      - InputSource
 *                      - Usage.
 */
template <typename Key>
class Collection : public std::map<Key, Element<Key> *>
{
private:
    typedef std::map<Key, Element<Key> *> Base;
    typedef Element<Key> T;
    typedef typename std::map<Key, T *>::iterator CollectionIterator;
    typedef typename std::map<Key, T *>::const_iterator CollectionConstIterator;

public:
    Collection()
    {
        collectionSupported();
    }

    /**
     * Add a policy element to the collection. Policy elements are streams, strategies, input
     * sources, ... Compile time error generated if called with not supported collection.
     * It also set the key as the unique identifier of the policy element.
     *
     * @tparam Key indexing the collection of policy element.
     * @param[in] name of the policy element to find.
     * @param[in] key to be used to index this new policy element.
     *
     * @return NO_ERROR if the policy element has been successfully added to the collection.
     */
    status_t add(const std::string &name, Key key)
    {
        if ((*this).find(key) != (*this).end()) {
            ALOGW("%s: element %s already added", __FUNCTION__, name.c_str());
            return BAD_VALUE;
        }
        (*this)[key] = new T(name);
        ALOGD("%s: adding element %s to collection", __FUNCTION__, name.c_str());
        return (*this)[key]->setIdentifier(key);
    }

    /**
     * Get a policy element from the collection by its key. Policy elements are streams, strategies,
     * input sources, ... Compile time error generated if called with not supported collection.
     *
     * @tparam Key indexing the collection of policy element.
     * @param[in] key of the policy element to find.
     *
     * @return valid pointer on policy element if found, NULL otherwise.
     */
    T *get(Key key) const
    {
        CollectionConstIterator it = (*this).find(key);
        return (it == (*this).end()) ? NULL : it->second;
    }

    /**
     * Find a policy element from the collection by its name. Policy elements are streams,
     * strategies, input sources, ...
     * Compile time error generated if called with not supported collection.
     *
     * @tparam Key indexing the collection of policy element.
     * @param[in] name of the policy element to find.
     * @param[in] elementsMap maps of policy elements to search into.
     *
     * @return valid pointer on element if found, NULL otherwise.
     */
    T *findByName(const std::string &name) const
    {

        CollectionConstIterator it;
        for (it = (*this).begin(); it != (*this).end(); ++it) {
            T *element = it->second;
            if (element->getName() == name) {
                return element;
            }
        }
        return NULL;
    }

    /**
     * Removes all the elements from the list and destroy them.
     */
    void clear()
    {
        CollectionIterator it;
        for (it = (*this).begin(); it != (*this).end(); ++it) {
            delete it->second;
        }
        Base::clear();
    }

private:
    /**
     * provide a compile time error if no specialization is provided for a given type.
     *
     * @tparam T: type of the policyElement. Policy Element supported are:
     *                      - Strategy
     *                      - Stream
     *                      - InputSource
     *                      - Usage.
     */
    struct collectionSupported;
};

template <>
struct Collection<audio_stream_type_t>::collectionSupported {};
template <>
struct Collection<std::string>::collectionSupported {};
template <>
struct Collection<audio_source_t>::collectionSupported {};

typedef Collection<audio_stream_type_t> StreamCollection;
typedef Collection<audio_source_t> InputSourceCollection;

} // namespace audio_policy
} // namespace android
