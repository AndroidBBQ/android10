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

#ifndef ANDROID_SERVICE_UTILS_EVICTION_POLICY_MANAGER_H
#define ANDROID_SERVICE_UTILS_EVICTION_POLICY_MANAGER_H

#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/Timers.h>

#include <algorithm>
#include <utility>
#include <vector>
#include <set>
#include <map>
#include <memory>

namespace android {
namespace resource_policy {

class ClientPriority {
public:
    /**
     * Choosing to set mIsVendorClient through a parameter instead of calling
     * hardware::IPCThreadState::self()->isServingCall() to protect against the
     * case where the construction is offloaded to another thread which isn't a
     * hwbinder thread.
     */
    ClientPriority(int32_t score, int32_t state, bool isVendorClient) :
            mScore(score), mState(state), mIsVendorClient(isVendorClient) { }

    int32_t getScore() const { return mScore; }
    int32_t getState() const { return mState; }

    void setScore(int32_t score) {
        // For vendor clients, the score is set once and for all during
        // construction. Otherwise, it can get reset each time cameraserver
        // queries ActivityManagerService for oom_adj scores / states .
        if (!mIsVendorClient) {
            mScore = score;
        }
    }

    void setState(int32_t state) {
      // For vendor clients, the score is set once and for all during
      // construction. Otherwise, it can get reset each time cameraserver
      // queries ActivityManagerService for oom_adj scores / states
      // (ActivityManagerService returns a vendor process' state as
      // PROCESS_STATE_NONEXISTENT.
      if (!mIsVendorClient) {
          mState = state;
      }
    }

    bool operator==(const ClientPriority& rhs) const {
        return (this->mScore == rhs.mScore) && (this->mState == rhs.mState);
    }

    bool operator< (const ClientPriority& rhs)  const {
        if (this->mScore == rhs.mScore) {
            return this->mState < rhs.mState;
        } else {
            return this->mScore < rhs.mScore;
        }
    }

    bool operator> (const ClientPriority& rhs) const {
        return rhs < *this;
    }

    bool operator<=(const ClientPriority& rhs) const {
        return !(*this > rhs);
    }

    bool operator>=(const ClientPriority& rhs) const {
        return !(*this < rhs);
    }

private:
        int32_t mScore;
        int32_t mState;
        bool mIsVendorClient = false;
};

// --------------------------------------------------------------------------------

/**
 * The ClientDescriptor class is a container for a given key/value pair identifying a shared
 * resource, and the corresponding cost, priority, owner ID, and conflicting keys list used
 * in determining eviction behavior.
 *
 * Aside from the priority, these values are immutable once the ClientDescriptor has been
 * constructed.
 */
template<class KEY, class VALUE>
class ClientDescriptor final {
public:
    ClientDescriptor(const KEY& key, const VALUE& value, int32_t cost,
            const std::set<KEY>& conflictingKeys, int32_t score, int32_t ownerId, int32_t state,
            bool isVendorClient);
    ClientDescriptor(KEY&& key, VALUE&& value, int32_t cost, std::set<KEY>&& conflictingKeys,
            int32_t score, int32_t ownerId, int32_t state, bool isVendorClient);

    ~ClientDescriptor();

    /**
     * Return the key for this descriptor.
     */
    const KEY& getKey() const;

    /**
     * Return the value for this descriptor.
     */
    const VALUE& getValue() const;

    /**
     * Return the cost for this descriptor.
     */
    int32_t getCost() const;

    /**
     * Return the priority for this descriptor.
     */
    const ClientPriority &getPriority() const;

    /**
     * Return the owner ID for this descriptor.
     */
    int32_t getOwnerId() const;

    /**
     * Return true if the given key is in this descriptor's conflicting keys list.
     */
    bool isConflicting(const KEY& key) const;

    /**
     * Return the set of all conflicting keys for this descriptor.
     */
    std::set<KEY> getConflicting() const;

    /**
     * Set the proirity for this descriptor.
     */
    void setPriority(const ClientPriority& priority);

    // This class is ordered by key
    template<class K, class V>
    friend bool operator < (const ClientDescriptor<K, V>& a, const ClientDescriptor<K, V>& b);

private:
    KEY mKey;
    VALUE mValue;
    int32_t mCost;
    std::set<KEY> mConflicting;
    ClientPriority mPriority;
    int32_t mOwnerId;
}; // class ClientDescriptor

template<class K, class V>
bool operator < (const ClientDescriptor<K, V>& a, const ClientDescriptor<K, V>& b) {
    return a.mKey < b.mKey;
}

template<class KEY, class VALUE>
ClientDescriptor<KEY, VALUE>::ClientDescriptor(const KEY& key, const VALUE& value, int32_t cost,
        const std::set<KEY>& conflictingKeys, int32_t score, int32_t ownerId, int32_t state,
        bool isVendorClient) :
        mKey{key}, mValue{value}, mCost{cost}, mConflicting{conflictingKeys},
        mPriority(score, state, isVendorClient),
        mOwnerId{ownerId} {}

template<class KEY, class VALUE>
ClientDescriptor<KEY, VALUE>::ClientDescriptor(KEY&& key, VALUE&& value, int32_t cost,
        std::set<KEY>&& conflictingKeys, int32_t score, int32_t ownerId, int32_t state,
        bool isVendorClient) :
        mKey{std::forward<KEY>(key)}, mValue{std::forward<VALUE>(value)}, mCost{cost},
        mConflicting{std::forward<std::set<KEY>>(conflictingKeys)},
        mPriority(score, state, isVendorClient), mOwnerId{ownerId} {}

template<class KEY, class VALUE>
ClientDescriptor<KEY, VALUE>::~ClientDescriptor() {}

template<class KEY, class VALUE>
const KEY& ClientDescriptor<KEY, VALUE>::getKey() const {
    return mKey;
}

template<class KEY, class VALUE>
const VALUE& ClientDescriptor<KEY, VALUE>::getValue() const {
    return mValue;
}

template<class KEY, class VALUE>
int32_t ClientDescriptor<KEY, VALUE>::getCost() const {
    return mCost;
}

template<class KEY, class VALUE>
const ClientPriority& ClientDescriptor<KEY, VALUE>::getPriority() const {
    return mPriority;
}

template<class KEY, class VALUE>
int32_t ClientDescriptor<KEY, VALUE>::getOwnerId() const {
    return mOwnerId;
}

template<class KEY, class VALUE>
bool ClientDescriptor<KEY, VALUE>::isConflicting(const KEY& key) const {
    if (key == mKey) return true;
    for (const auto& x : mConflicting) {
        if (key == x) return true;
    }
    return false;
}

template<class KEY, class VALUE>
std::set<KEY> ClientDescriptor<KEY, VALUE>::getConflicting() const {
    return mConflicting;
}

template<class KEY, class VALUE>
void ClientDescriptor<KEY, VALUE>::setPriority(const ClientPriority& priority) {
    // We don't use the usual copy constructor here since we want to remember
    // whether a client is a vendor client or not. This could have been wiped
    // off in the incoming priority argument since an AIDL thread might have
    // called hardware::IPCThreadState::self()->isServingCall() after refreshing
    // priorities for old clients through ProcessInfoService::getProcessStatesScoresFromPids().
    mPriority.setScore(priority.getScore());
    mPriority.setState(priority.getState());
}

// --------------------------------------------------------------------------------

/**
 * A default class implementing the LISTENER interface used by ClientManager.
 */
template<class KEY, class VALUE>
class DefaultEventListener {
public:
    void onClientAdded(const ClientDescriptor<KEY, VALUE>& descriptor);
    void onClientRemoved(const ClientDescriptor<KEY, VALUE>& descriptor);
};

template<class KEY, class VALUE>
void DefaultEventListener<KEY, VALUE>::onClientAdded(
        const ClientDescriptor<KEY, VALUE>& /*descriptor*/) {}

template<class KEY, class VALUE>
void DefaultEventListener<KEY, VALUE>::onClientRemoved(
        const ClientDescriptor<KEY, VALUE>& /*descriptor*/) {}

// --------------------------------------------------------------------------------

/**
 * The ClientManager class wraps an LRU-ordered list of active clients and implements eviction
 * behavior for handling shared resource access.
 *
 * When adding a new descriptor, eviction behavior is as follows:
 *   - Keys are unique, adding a descriptor with the same key as an existing descriptor will
 *     result in the lower-priority of the two being removed.  Priority ties result in the
 *     LRU descriptor being evicted (this means the incoming descriptor be added in this case).
 *   - Any descriptors with keys that are in the incoming descriptor's 'conflicting keys' list
 *     will be removed if they have an equal or lower priority than the incoming descriptor;
 *     if any have a higher priority, the incoming descriptor is removed instead.
 *   - If the sum of all descriptors' costs, including the incoming descriptor's, is more than
 *     the max cost allowed for this ClientManager, descriptors with non-zero cost, equal or lower
 *     priority, and a different owner will be evicted in LRU order until either the cost is less
 *     than the max cost, or all descriptors meeting this criteria have been evicted and the
 *     incoming descriptor has the highest priority.  Otherwise, the incoming descriptor is
 *     removed instead.
 */
template<class KEY, class VALUE, class LISTENER=DefaultEventListener<KEY, VALUE>>
class ClientManager {
public:
    // The default maximum "cost" allowed before evicting
    static constexpr int32_t DEFAULT_MAX_COST = 100;

    ClientManager();
    explicit ClientManager(int32_t totalCost);

    /**
     * Add a given ClientDescriptor to the managed list.  ClientDescriptors for clients that
     * are evicted by this action are returned in a vector.
     *
     * This may return the ClientDescriptor passed in if it would be evicted.
     */
    std::vector<std::shared_ptr<ClientDescriptor<KEY, VALUE>>> addAndEvict(
            const std::shared_ptr<ClientDescriptor<KEY, VALUE>>& client);

    /**
     * Given a map containing owner (pid) -> priority mappings, update the priority of each
     * ClientDescriptor with an owner in this mapping.
     */
    void updatePriorities(const std::map<int32_t,ClientPriority>& ownerPriorityList);

    /**
     * Remove all ClientDescriptors.
     */
    void removeAll();

    /**
     * Remove and return the ClientDescriptor with a given key.
     */
    std::shared_ptr<ClientDescriptor<KEY, VALUE>> remove(const KEY& key);

    /**
     * Remove the given ClientDescriptor.
     */
    void remove(const std::shared_ptr<ClientDescriptor<KEY, VALUE>>& value);

    /**
     * Return a vector of the ClientDescriptors that would be evicted by adding the given
     * ClientDescriptor.
     *
     * This may return the ClientDescriptor passed in if it would be evicted.
     */
    std::vector<std::shared_ptr<ClientDescriptor<KEY, VALUE>>> wouldEvict(
            const std::shared_ptr<ClientDescriptor<KEY, VALUE>>& client) const;

    /**
     * Return a vector of active ClientDescriptors that prevent this client from being added.
     */
    std::vector<std::shared_ptr<ClientDescriptor<KEY, VALUE>>> getIncompatibleClients(
            const std::shared_ptr<ClientDescriptor<KEY, VALUE>>& client) const;

    /**
     * Return a vector containing all currently active ClientDescriptors.
     */
    std::vector<std::shared_ptr<ClientDescriptor<KEY, VALUE>>> getAll() const;

    /**
     * Return a vector containing all keys of currently active ClientDescriptors.
     */
    std::vector<KEY> getAllKeys() const;

    /**
     * Return a vector of the owner tags of all currently active ClientDescriptors (duplicates
     * will be removed).
     */
    std::vector<int32_t> getAllOwners() const;

    /**
     * Return the ClientDescriptor corresponding to the given key, or an empty shared pointer
     * if none exists.
     */
    std::shared_ptr<ClientDescriptor<KEY, VALUE>> get(const KEY& key) const;

    /**
     * Block until the given client is no longer in the active clients list, or the timeout
     * occurred.
     *
     * Returns NO_ERROR if this succeeded, -ETIMEDOUT on a timeout, or a negative error code on
     * failure.
     */
    status_t waitUntilRemoved(const std::shared_ptr<ClientDescriptor<KEY, VALUE>> client,
            nsecs_t timeout) const;

    /**
     * Set the current listener for client add/remove events.
     *
     * The listener instance must inherit from the LISTENER class and implement the following
     * methods:
     *    void onClientRemoved(const ClientDescriptor<KEY, VALUE>& descriptor);
     *    void onClientAdded(const ClientDescriptor<KEY, VALUE>& descriptor);
     *
     * These callback methods will be called with the ClientManager's lock held, and should
     * not call any further ClientManager methods.
     *
     * The onClientRemoved method will be called when the client has been removed or evicted
     * from the ClientManager that this event listener has been added to. The onClientAdded
     * method will be called when the client has been added to the ClientManager that this
     * event listener has been added to.
     */
    void setListener(const std::shared_ptr<LISTENER>& listener);

protected:
    ~ClientManager();

private:

    /**
     * Return a vector of the ClientDescriptors that would be evicted by adding the given
     * ClientDescriptor.  If returnIncompatibleClients is set to true, instead, return the
     * vector of ClientDescriptors that are higher priority than the incoming client and
     * either conflict with this client, or contribute to the resource cost if that would
     * prevent the incoming client from being added.
     *
     * This may return the ClientDescriptor passed in.
     */
    std::vector<std::shared_ptr<ClientDescriptor<KEY, VALUE>>> wouldEvictLocked(
            const std::shared_ptr<ClientDescriptor<KEY, VALUE>>& client,
            bool returnIncompatibleClients = false) const;

    int64_t getCurrentCostLocked() const;

    mutable Mutex mLock;
    mutable Condition mRemovedCondition;
    int32_t mMaxCost;
    // LRU ordered, most recent at end
    std::vector<std::shared_ptr<ClientDescriptor<KEY, VALUE>>> mClients;
    std::shared_ptr<LISTENER> mListener;
}; // class ClientManager

template<class KEY, class VALUE, class LISTENER>
ClientManager<KEY, VALUE, LISTENER>::ClientManager() :
        ClientManager(DEFAULT_MAX_COST) {}

template<class KEY, class VALUE, class LISTENER>
ClientManager<KEY, VALUE, LISTENER>::ClientManager(int32_t totalCost) : mMaxCost(totalCost) {}

template<class KEY, class VALUE, class LISTENER>
ClientManager<KEY, VALUE, LISTENER>::~ClientManager() {}

template<class KEY, class VALUE, class LISTENER>
std::vector<std::shared_ptr<ClientDescriptor<KEY, VALUE>>>
ClientManager<KEY, VALUE, LISTENER>::wouldEvict(
        const std::shared_ptr<ClientDescriptor<KEY, VALUE>>& client) const {
    Mutex::Autolock lock(mLock);
    return wouldEvictLocked(client);
}

template<class KEY, class VALUE, class LISTENER>
std::vector<std::shared_ptr<ClientDescriptor<KEY, VALUE>>>
ClientManager<KEY, VALUE, LISTENER>::getIncompatibleClients(
        const std::shared_ptr<ClientDescriptor<KEY, VALUE>>& client) const {
    Mutex::Autolock lock(mLock);
    return wouldEvictLocked(client, /*returnIncompatibleClients*/true);
}

template<class KEY, class VALUE, class LISTENER>
std::vector<std::shared_ptr<ClientDescriptor<KEY, VALUE>>>
ClientManager<KEY, VALUE, LISTENER>::wouldEvictLocked(
        const std::shared_ptr<ClientDescriptor<KEY, VALUE>>& client,
        bool returnIncompatibleClients) const {

    std::vector<std::shared_ptr<ClientDescriptor<KEY, VALUE>>> evictList;

    // Disallow null clients, return input
    if (client == nullptr) {
        evictList.push_back(client);
        return evictList;
    }

    const KEY& key = client->getKey();
    int32_t cost = client->getCost();
    ClientPriority priority = client->getPriority();
    int32_t owner = client->getOwnerId();

    int64_t totalCost = getCurrentCostLocked() + cost;

    // Determine the MRU of the owners tied for having the highest priority
    int32_t highestPriorityOwner = owner;
    ClientPriority highestPriority = priority;
    for (const auto& i : mClients) {
        ClientPriority curPriority = i->getPriority();
        if (curPriority <= highestPriority) {
            highestPriority = curPriority;
            highestPriorityOwner = i->getOwnerId();
        }
    }

    if (highestPriority == priority) {
        // Switch back owner if the incoming client has the highest priority, as it is MRU
        highestPriorityOwner = owner;
    }

    // Build eviction list of clients to remove
    for (const auto& i : mClients) {
        const KEY& curKey = i->getKey();
        int32_t curCost = i->getCost();
        ClientPriority curPriority = i->getPriority();
        int32_t curOwner = i->getOwnerId();

        bool conflicting = (curKey == key || i->isConflicting(key) ||
                client->isConflicting(curKey));

        if (!returnIncompatibleClients) {
            // Find evicted clients

            if (conflicting && curPriority < priority) {
                // Pre-existing conflicting client with higher priority exists
                evictList.clear();
                evictList.push_back(client);
                return evictList;
            } else if (conflicting || ((totalCost > mMaxCost && curCost > 0) &&
                    (curPriority >= priority) &&
                    !(highestPriorityOwner == owner && owner == curOwner))) {
                // Add a pre-existing client to the eviction list if:
                // - We are adding a client with higher priority that conflicts with this one.
                // - The total cost including the incoming client's is more than the allowable
                //   maximum, and the client has a non-zero cost, lower priority, and a different
                //   owner than the incoming client when the incoming client has the
                //   highest priority.
                evictList.push_back(i);
                totalCost -= curCost;
            }
        } else {
            // Find clients preventing the incoming client from being added

            if (curPriority < priority && (conflicting || (totalCost > mMaxCost && curCost > 0))) {
                // Pre-existing conflicting client with higher priority exists
                evictList.push_back(i);
            }
        }
    }

    // Immediately return the incompatible clients if we are calculating these instead
    if (returnIncompatibleClients) {
        return evictList;
    }

    // If the total cost is too high, return the input unless the input has the highest priority
    if (totalCost > mMaxCost && highestPriorityOwner != owner) {
        evictList.clear();
        evictList.push_back(client);
        return evictList;
    }

    return evictList;

}

template<class KEY, class VALUE, class LISTENER>
std::vector<std::shared_ptr<ClientDescriptor<KEY, VALUE>>>
ClientManager<KEY, VALUE, LISTENER>::addAndEvict(
        const std::shared_ptr<ClientDescriptor<KEY, VALUE>>& client) {
    Mutex::Autolock lock(mLock);
    auto evicted = wouldEvictLocked(client);
    auto it = evicted.begin();
    if (it != evicted.end() && *it == client) {
        return evicted;
    }

    auto iter = evicted.cbegin();

    if (iter != evicted.cend()) {

        if (mListener != nullptr) mListener->onClientRemoved(**iter);

        // Remove evicted clients from list
        mClients.erase(std::remove_if(mClients.begin(), mClients.end(),
            [&iter] (std::shared_ptr<ClientDescriptor<KEY, VALUE>>& curClientPtr) {
                if (curClientPtr->getKey() == (*iter)->getKey()) {
                    iter++;
                    return true;
                }
                return false;
            }), mClients.end());
    }

    if (mListener != nullptr) mListener->onClientAdded(*client);
    mClients.push_back(client);
    mRemovedCondition.broadcast();

    return evicted;
}

template<class KEY, class VALUE, class LISTENER>
std::vector<std::shared_ptr<ClientDescriptor<KEY, VALUE>>>
ClientManager<KEY, VALUE, LISTENER>::getAll() const {
    Mutex::Autolock lock(mLock);
    return mClients;
}

template<class KEY, class VALUE, class LISTENER>
std::vector<KEY> ClientManager<KEY, VALUE, LISTENER>::getAllKeys() const {
    Mutex::Autolock lock(mLock);
    std::vector<KEY> keys(mClients.size());
    for (const auto& i : mClients) {
        keys.push_back(i->getKey());
    }
    return keys;
}

template<class KEY, class VALUE, class LISTENER>
std::vector<int32_t> ClientManager<KEY, VALUE, LISTENER>::getAllOwners() const {
    Mutex::Autolock lock(mLock);
    std::set<int32_t> owners;
    for (const auto& i : mClients) {
        owners.emplace(i->getOwnerId());
    }
    return std::vector<int32_t>(owners.begin(), owners.end());
}

template<class KEY, class VALUE, class LISTENER>
void ClientManager<KEY, VALUE, LISTENER>::updatePriorities(
        const std::map<int32_t,ClientPriority>& ownerPriorityList) {
    Mutex::Autolock lock(mLock);
    for (auto& i : mClients) {
        auto j = ownerPriorityList.find(i->getOwnerId());
        if (j != ownerPriorityList.end()) {
            i->setPriority(j->second);
        }
    }
}

template<class KEY, class VALUE, class LISTENER>
std::shared_ptr<ClientDescriptor<KEY, VALUE>> ClientManager<KEY, VALUE, LISTENER>::get(
        const KEY& key) const {
    Mutex::Autolock lock(mLock);
    for (const auto& i : mClients) {
        if (i->getKey() == key) return i;
    }
    return std::shared_ptr<ClientDescriptor<KEY, VALUE>>(nullptr);
}

template<class KEY, class VALUE, class LISTENER>
void ClientManager<KEY, VALUE, LISTENER>::removeAll() {
    Mutex::Autolock lock(mLock);
    if (mListener != nullptr) {
        for (const auto& i : mClients) {
            mListener->onClientRemoved(*i);
        }
    }
    mClients.clear();
    mRemovedCondition.broadcast();
}

template<class KEY, class VALUE, class LISTENER>
std::shared_ptr<ClientDescriptor<KEY, VALUE>> ClientManager<KEY, VALUE, LISTENER>::remove(
    const KEY& key) {
    Mutex::Autolock lock(mLock);

    std::shared_ptr<ClientDescriptor<KEY, VALUE>> ret;

    // Remove evicted clients from list
    mClients.erase(std::remove_if(mClients.begin(), mClients.end(),
        [this, &key, &ret] (std::shared_ptr<ClientDescriptor<KEY, VALUE>>& curClientPtr) {
            if (curClientPtr->getKey() == key) {
                if (mListener != nullptr) mListener->onClientRemoved(*curClientPtr);
                ret = curClientPtr;
                return true;
            }
            return false;
        }), mClients.end());

    mRemovedCondition.broadcast();
    return ret;
}

template<class KEY, class VALUE, class LISTENER>
status_t ClientManager<KEY, VALUE, LISTENER>::waitUntilRemoved(
        const std::shared_ptr<ClientDescriptor<KEY, VALUE>> client,
        nsecs_t timeout) const {
    status_t ret = NO_ERROR;
    Mutex::Autolock lock(mLock);

    bool isRemoved = false;

    // Figure out what time in the future we should hit the timeout
    nsecs_t failTime = systemTime(SYSTEM_TIME_MONOTONIC) + timeout;

    while (!isRemoved) {
        isRemoved = true;
        for (const auto& i : mClients) {
            if (i == client) {
                isRemoved = false;
            }
        }

        if (!isRemoved) {
            ret = mRemovedCondition.waitRelative(mLock, timeout);
            if (ret != NO_ERROR) {
                break;
            }
            timeout = failTime - systemTime(SYSTEM_TIME_MONOTONIC);
        }
    }

    return ret;
}

template<class KEY, class VALUE, class LISTENER>
void ClientManager<KEY, VALUE, LISTENER>::setListener(const std::shared_ptr<LISTENER>& listener) {
    Mutex::Autolock lock(mLock);
    mListener = listener;
}

template<class KEY, class VALUE, class LISTENER>
void ClientManager<KEY, VALUE, LISTENER>::remove(
        const std::shared_ptr<ClientDescriptor<KEY, VALUE>>& value) {
    Mutex::Autolock lock(mLock);
    // Remove evicted clients from list
    mClients.erase(std::remove_if(mClients.begin(), mClients.end(),
        [this, &value] (std::shared_ptr<ClientDescriptor<KEY, VALUE>>& curClientPtr) {
            if (curClientPtr == value) {
                if (mListener != nullptr) mListener->onClientRemoved(*curClientPtr);
                return true;
            }
            return false;
        }), mClients.end());
    mRemovedCondition.broadcast();
}

template<class KEY, class VALUE, class LISTENER>
int64_t ClientManager<KEY, VALUE, LISTENER>::getCurrentCostLocked() const {
    int64_t totalCost = 0;
    for (const auto& x : mClients) {
            totalCost += x->getCost();
    }
    return totalCost;
}

// --------------------------------------------------------------------------------

}; // namespace resource_policy
}; // namespace android

#endif // ANDROID_SERVICE_UTILS_EVICTION_POLICY_MANAGER_H
