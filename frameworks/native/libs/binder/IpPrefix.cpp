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

#define LOG_TAG "IpPrefix"

#include <binder/IpPrefix.h>
#include <vector>

#include <binder/IBinder.h>
#include <binder/Parcel.h>
#include <log/log.h>
#include <utils/Errors.h>

using android::BAD_TYPE;
using android::BAD_VALUE;
using android::NO_ERROR;
using android::Parcel;
using android::status_t;
using android::UNEXPECTED_NULL;
using namespace ::android::binder;

namespace android {

namespace net {

#define RETURN_IF_FAILED(calledOnce)                                     \
    {                                                                    \
        status_t returnStatus = calledOnce;                              \
        if (returnStatus) {                                              \
            ALOGE("Failed at %s:%d (%s)", __FILE__, __LINE__, __func__); \
            return returnStatus;                                         \
         }                                                               \
    }

status_t IpPrefix::writeToParcel(Parcel* parcel) const {
    /*
     * Keep implementation in sync with writeToParcel() in
     * frameworks/base/core/java/android/net/IpPrefix.java.
     */
    std::vector<uint8_t> byte_vector;

    if (mIsIpv6) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&mUnion.mIn6Addr);
        byte_vector.insert(byte_vector.end(), bytes, bytes+sizeof(mUnion.mIn6Addr));
    } else {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&mUnion.mInAddr);
        byte_vector.insert(byte_vector.end(), bytes, bytes+sizeof(mUnion.mIn6Addr));
    }

    RETURN_IF_FAILED(parcel->writeByteVector(byte_vector));
    RETURN_IF_FAILED(parcel->writeInt32(static_cast<int32_t>(mPrefixLength)));

    return NO_ERROR;
}

status_t IpPrefix::readFromParcel(const Parcel* parcel) {
    /*
     * Keep implementation in sync with readFromParcel() in
     * frameworks/base/core/java/android/net/IpPrefix.java.
     */
    std::vector<uint8_t> byte_vector;

    RETURN_IF_FAILED(parcel->readByteVector(&byte_vector));
    RETURN_IF_FAILED(parcel->readInt32(&mPrefixLength));

    if (byte_vector.size() == 16) {
        mIsIpv6 = true;
        memcpy((void*)&mUnion.mIn6Addr, &byte_vector[0], sizeof(mUnion.mIn6Addr));

    } else if (byte_vector.size() == 4) {
        mIsIpv6 = false;
        memcpy((void*)&mUnion.mInAddr, &byte_vector[0], sizeof(mUnion.mInAddr));

    } else {
        ALOGE("Failed at %s:%d (%s)", __FILE__, __LINE__, __func__); \
        return BAD_VALUE;
    }

    return NO_ERROR;
}

const struct in6_addr& IpPrefix::getAddressAsIn6Addr() const
{
    return mUnion.mIn6Addr;
}

const struct in_addr& IpPrefix::getAddressAsInAddr() const
{
    return mUnion.mInAddr;
}

bool IpPrefix::getAddressAsIn6Addr(struct in6_addr* addr) const
{
    if (isIpv6()) {
        *addr = mUnion.mIn6Addr;
        return true;
    }
    return false;
}

bool IpPrefix::getAddressAsInAddr(struct in_addr* addr) const
{
    if (isIpv4()) {
        *addr = mUnion.mInAddr;
        return true;
    }
    return false;
}

bool IpPrefix::isIpv6() const
{
    return mIsIpv6;
}

bool IpPrefix::isIpv4() const
{
    return !mIsIpv6;
}

int32_t IpPrefix::getPrefixLength() const
{
    return mPrefixLength;
}

void IpPrefix::setAddress(const struct in6_addr& addr)
{
    mUnion.mIn6Addr = addr;
    mIsIpv6 = true;
}

void IpPrefix::setAddress(const struct in_addr& addr)
{
    mUnion.mInAddr = addr;
    mIsIpv6 = false;
}

void IpPrefix::setPrefixLength(int32_t prefix)
{
    mPrefixLength = prefix;
}

bool operator==(const IpPrefix& lhs, const IpPrefix& rhs)
{
    if (lhs.mIsIpv6 != rhs.mIsIpv6) {
        return false;
    }

    if (lhs.mPrefixLength != rhs.mPrefixLength) {
        return false;
    }

    if (lhs.mIsIpv6) {
        return 0 == memcmp(lhs.mUnion.mIn6Addr.s6_addr, rhs.mUnion.mIn6Addr.s6_addr, sizeof(struct in6_addr));
    }

    return 0 == memcmp(&lhs.mUnion.mInAddr, &rhs.mUnion.mInAddr, sizeof(struct in_addr));
}

}  // namespace net

}  // namespace android
