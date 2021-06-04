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

#include <array>
#include <iomanip>
#include <random>
#include <sstream>

#include <android/hardware_buffer.h>
#include <bufferhub/BufferHubService.h>
#include <cutils/native_handle.h>
#include <log/log.h>
#include <openssl/hmac.h>
#include <system/graphics-base.h>
#include <ui/BufferHubDefs.h>

using ::android::BufferHubDefs::MetadataHeader;
using ::android::hardware::Void;

namespace android {
namespace frameworks {
namespace bufferhub {
namespace V1_0 {
namespace implementation {

BufferHubService::BufferHubService() {
    std::mt19937_64 randomEngine;
    randomEngine.seed(time(nullptr));

    mKey = randomEngine();
}

Return<void> BufferHubService::allocateBuffer(const HardwareBufferDescription& description,
                                              const uint32_t userMetadataSize,
                                              allocateBuffer_cb _hidl_cb) {
    AHardwareBuffer_Desc desc;
    memcpy(&desc, &description, sizeof(AHardwareBuffer_Desc));

    std::shared_ptr<BufferNode> node =
            std::make_shared<BufferNode>(desc.width, desc.height, desc.layers, desc.format,
                                         desc.usage, userMetadataSize,
                                         BufferHubIdGenerator::getInstance().getId());
    if (node == nullptr || !node->isValid()) {
        ALOGE("%s: creating BufferNode failed.", __FUNCTION__);
        _hidl_cb(/*status=*/BufferHubStatus::ALLOCATION_FAILED, /*bufferClient=*/nullptr,
                 /*bufferTraits=*/{});
        return Void();
    }

    sp<BufferClient> client = BufferClient::create(this, node);
    // Add it to list for bookkeeping and dumpsys.
    std::lock_guard<std::mutex> lock(mClientSetMutex);
    mClientSet.emplace(client);

    // Allocate memory for bufferInfo of type hidl_handle on the stack. See
    // http://aosp/286282 for the usage of NATIVE_HANDLE_DECLARE_STORAGE.
    NATIVE_HANDLE_DECLARE_STORAGE(bufferInfoStorage, BufferHubDefs::kBufferInfoNumFds,
                                  BufferHubDefs::kBufferInfoNumInts);
    hidl_handle bufferInfo =
            buildBufferInfo(bufferInfoStorage, node->id(), node->addNewActiveClientsBitToMask(),
                            node->userMetadataSize(), node->metadata().ashmemFd(),
                            node->eventFd().get());
    // During the gralloc allocation carried out by BufferNode, gralloc allocator will populate the
    // fields of its HardwareBufferDescription (i.e. strides) according to the actual
    // gralloc implementation. We need to read those fields back and send them to the client via
    // BufferTraits.
    HardwareBufferDescription allocatedBufferDesc;
    memcpy(&allocatedBufferDesc, &node->bufferDesc(), sizeof(AHardwareBuffer_Desc));
    BufferTraits bufferTraits = {/*bufferDesc=*/allocatedBufferDesc,
                                 /*bufferHandle=*/hidl_handle(node->bufferHandle()),
                                 /*bufferInfo=*/std::move(bufferInfo)};

    _hidl_cb(/*status=*/BufferHubStatus::NO_ERROR, /*bufferClient=*/client,
             /*bufferTraits=*/std::move(bufferTraits));
    return Void();
}

Return<void> BufferHubService::importBuffer(const hidl_handle& tokenHandle,
                                            importBuffer_cb _hidl_cb) {
    if (!tokenHandle.getNativeHandle() || tokenHandle->numFds != 0 || tokenHandle->numInts <= 1) {
        // nullptr handle or wrong format
        _hidl_cb(/*status=*/BufferHubStatus::INVALID_TOKEN, /*bufferClient=*/nullptr,
                 /*bufferTraits=*/{});
        return Void();
    }

    int tokenId = tokenHandle->data[0];

    wp<BufferClient> originClientWp;
    {
        std::lock_guard<std::mutex> lock(mTokenMutex);
        auto iter = mTokenMap.find(tokenId);
        if (iter == mTokenMap.end()) {
            // Token Id not exist
            ALOGD("%s: token #%d not found.", __FUNCTION__, tokenId);
            _hidl_cb(/*status=*/BufferHubStatus::INVALID_TOKEN, /*bufferClient=*/nullptr,
                     /*bufferTraits=*/{});
            return Void();
        }

        const std::vector<uint8_t>& tokenHMAC = iter->second.first;

        int numIntsForHMAC = (int)ceil(tokenHMAC.size() * sizeof(uint8_t) / (double)sizeof(int));
        if (tokenHandle->numInts - 1 != numIntsForHMAC) {
            // HMAC size not match
            ALOGD("%s: token #%d HMAC size not match. Expected: %d Actual: %d", __FUNCTION__,
                  tokenId, numIntsForHMAC, tokenHandle->numInts - 1);
            _hidl_cb(/*status=*/BufferHubStatus::INVALID_TOKEN, /*bufferClient=*/nullptr,
                     /*bufferTraits=*/{});
            return Void();
        }

        size_t hmacSize = tokenHMAC.size() * sizeof(uint8_t);
        if (memcmp(tokenHMAC.data(), &tokenHandle->data[1], hmacSize) != 0) {
            // HMAC not match
            ALOGD("%s: token #%d HMAC not match.", __FUNCTION__, tokenId);
            _hidl_cb(/*status=*/BufferHubStatus::INVALID_TOKEN, /*bufferClient=*/nullptr,
                     /*bufferTraits=*/{});
            return Void();
        }

        originClientWp = iter->second.second;
        mTokenMap.erase(iter);
    }

    // Check if original client is dead
    sp<BufferClient> originClient = originClientWp.promote();
    if (!originClient) {
        // Should not happen since token should be removed if already gone
        ALOGE("%s: original client %p gone!", __FUNCTION__, originClientWp.unsafe_get());
        _hidl_cb(/*status=*/BufferHubStatus::BUFFER_FREED, /*bufferClient=*/nullptr,
                 /*bufferTraits=*/{});
        return Void();
    }

    sp<BufferClient> client = new BufferClient(*originClient);
    uint32_t clientStateMask = client->getBufferNode()->addNewActiveClientsBitToMask();
    if (clientStateMask == 0U) {
        // Reach max client count
        ALOGE("%s: import failed, BufferNode#%u reached maximum clients.", __FUNCTION__,
              client->getBufferNode()->id());
        _hidl_cb(/*status=*/BufferHubStatus::MAX_CLIENT, /*bufferClient=*/nullptr,
                 /*bufferTraits=*/{});
        return Void();
    }

    std::lock_guard<std::mutex> lock(mClientSetMutex);
    mClientSet.emplace(client);

    std::shared_ptr<BufferNode> node = client->getBufferNode();

    HardwareBufferDescription bufferDesc;
    memcpy(&bufferDesc, &node->bufferDesc(), sizeof(HardwareBufferDescription));

    // Allocate memory for bufferInfo of type hidl_handle on the stack. See
    // http://aosp/286282 for the usage of NATIVE_HANDLE_DECLARE_STORAGE.
    NATIVE_HANDLE_DECLARE_STORAGE(bufferInfoStorage, BufferHubDefs::kBufferInfoNumFds,
                                  BufferHubDefs::kBufferInfoNumInts);
    hidl_handle bufferInfo = buildBufferInfo(bufferInfoStorage, node->id(), clientStateMask,
                                             node->userMetadataSize(), node->metadata().ashmemFd(),
                                             node->eventFd().get());
    BufferTraits bufferTraits = {/*bufferDesc=*/bufferDesc,
                                 /*bufferHandle=*/hidl_handle(node->bufferHandle()),
                                 /*bufferInfo=*/std::move(bufferInfo)};

    _hidl_cb(/*status=*/BufferHubStatus::NO_ERROR, /*bufferClient=*/client,
             /*bufferTraits=*/std::move(bufferTraits));
    return Void();
}

Return<void> BufferHubService::debug(const hidl_handle& fd, const hidl_vec<hidl_string>& args) {
    if (fd.getNativeHandle() == nullptr || fd->numFds < 1) {
        ALOGE("%s: missing fd for writing.", __FUNCTION__);
        return Void();
    }

    FILE* out = fdopen(dup(fd->data[0]), "w");

    if (args.size() != 0) {
        fprintf(out,
                "Note: lshal bufferhub currently does not support args. Input arguments are "
                "ignored.\n");
    }

    std::ostringstream stream;

    // Get the number of clients of each buffer.
    // Map from bufferId to bufferNode_clientCount pair.
    std::map<int, std::pair<const std::shared_ptr<BufferNode>, uint32_t>> clientCount;
    {
        std::lock_guard<std::mutex> lock(mClientSetMutex);
        for (auto iter = mClientSet.begin(); iter != mClientSet.end(); ++iter) {
            sp<BufferClient> client = iter->promote();
            if (client != nullptr) {
                const std::shared_ptr<BufferNode> node = client->getBufferNode();
                auto mapIter = clientCount.find(node->id());
                if (mapIter != clientCount.end()) {
                    ++mapIter->second.second;
                } else {
                    clientCount.emplace(node->id(),
                                        std::pair<std::shared_ptr<BufferNode>, uint32_t>(node, 1U));
                }
            }
        }
    }

    stream << "Active Buffers:\n";
    stream << std::right;
    stream << std::setw(6) << "Id";
    stream << " ";
    stream << std::setw(9) << "#Clients";
    stream << " ";
    stream << std::setw(14) << "Geometry";
    stream << " ";
    stream << std::setw(6) << "Format";
    stream << " ";
    stream << std::setw(10) << "Usage";
    stream << " ";
    stream << std::setw(10) << "State";
    stream << " ";
    stream << std::setw(8) << "Index";
    stream << std::endl;

    for (auto iter = clientCount.begin(); iter != clientCount.end(); ++iter) {
        const std::shared_ptr<BufferNode> node = std::move(iter->second.first);
        const uint32_t clientCount = iter->second.second;
        AHardwareBuffer_Desc desc = node->bufferDesc();

        MetadataHeader* metadataHeader =
                const_cast<BufferHubMetadata*>(&node->metadata())->metadataHeader();
        const uint32_t state = metadataHeader->bufferState.load(std::memory_order_acquire);
        const uint64_t index = metadataHeader->queueIndex;

        stream << std::right;
        stream << std::setw(6) << /*Id=*/node->id();
        stream << " ";
        stream << std::setw(9) << /*#Clients=*/clientCount;
        stream << " ";
        if (desc.format == HAL_PIXEL_FORMAT_BLOB) {
            std::string size = std::to_string(desc.width) + " B";
            stream << std::setw(14) << /*Geometry=*/size;
        } else {
            std::string dimensions = std::to_string(desc.width) + "x" +
                    std::to_string(desc.height) + "x" + std::to_string(desc.layers);
            stream << std::setw(14) << /*Geometry=*/dimensions;
        }
        stream << " ";
        stream << std::setw(6) << /*Format=*/desc.format;
        stream << " ";
        stream << "0x" << std::hex << std::setfill('0');
        stream << std::setw(8) << /*Usage=*/desc.usage;
        stream << std::dec << std::setfill(' ');
        stream << " ";
        stream << "0x" << std::hex << std::setfill('0');
        stream << std::setw(8) << /*State=*/state;
        stream << std::dec << std::setfill(' ');
        stream << " ";
        stream << std::setw(8) << /*Index=*/index;
        stream << std::endl;
    }

    stream << std::endl;

    // Get the number of tokens of each buffer.
    // Map from bufferId to tokenCount
    std::map<int, uint32_t> tokenCount;
    {
        std::lock_guard<std::mutex> lock(mTokenMutex);
        for (auto iter = mTokenMap.begin(); iter != mTokenMap.end(); ++iter) {
            sp<BufferClient> client = iter->second.second.promote();
            if (client != nullptr) {
                const std::shared_ptr<BufferNode> node = client->getBufferNode();
                auto mapIter = tokenCount.find(node->id());
                if (mapIter != tokenCount.end()) {
                    ++mapIter->second;
                } else {
                    tokenCount.emplace(node->id(), 1U);
                }
            }
        }
    }

    stream << "Unused Tokens:\n";
    stream << std::right;
    stream << std::setw(8) << "Buffer Id";
    stream << " ";
    stream << std::setw(7) << "#Tokens";
    stream << std::endl;

    for (auto iter = tokenCount.begin(); iter != tokenCount.end(); ++iter) {
        stream << std::right;
        stream << std::setw(8) << /*Buffer Id=*/iter->first;
        stream << " ";
        stream << std::setw(7) << /*#Tokens=*/iter->second;
        stream << std::endl;
    }

    fprintf(out, "%s", stream.str().c_str());

    fclose(out);
    return Void();
}

hidl_handle BufferHubService::registerToken(const wp<BufferClient>& client) {
    // Find next available token id
    std::lock_guard<std::mutex> lock(mTokenMutex);
    do {
        ++mLastTokenId;
    } while (mTokenMap.find(mLastTokenId) != mTokenMap.end());

    std::array<uint8_t, EVP_MAX_MD_SIZE> hmac;
    uint32_t hmacSize = 0U;

    HMAC(/*evp_md=*/EVP_sha256(), /*key=*/&mKey, /*key_len=*/kKeyLen,
         /*data=*/(uint8_t*)&mLastTokenId, /*data_len=*/mTokenIdSize,
         /*out=*/hmac.data(), /*out_len=*/&hmacSize);

    int numIntsForHMAC = (int)ceil(hmacSize / (double)sizeof(int));
    native_handle_t* handle = native_handle_create(/*numFds=*/0, /*numInts=*/1 + numIntsForHMAC);
    handle->data[0] = mLastTokenId;
    // Set all the the bits of last int to 0 since it might not be fully overwritten
    handle->data[numIntsForHMAC] = 0;
    memcpy(&handle->data[1], hmac.data(), hmacSize);

    // returnToken owns the native_handle_t* thus doing lifecycle management
    hidl_handle returnToken;
    returnToken.setTo(handle, /*shoudOwn=*/true);

    std::vector<uint8_t> hmacVec;
    hmacVec.resize(hmacSize);
    memcpy(hmacVec.data(), hmac.data(), hmacSize);
    mTokenMap.emplace(mLastTokenId, std::pair(hmacVec, client));

    return returnToken;
}

void BufferHubService::onClientClosed(const BufferClient* client) {
    removeTokenByClient(client);

    std::lock_guard<std::mutex> lock(mClientSetMutex);
    auto iter = std::find(mClientSet.begin(), mClientSet.end(), client);
    if (iter != mClientSet.end()) {
        mClientSet.erase(iter);
    }
}

// Implementation of this function should be consistent with the definition of bufferInfo handle in
// ui/BufferHubDefs.h.
hidl_handle BufferHubService::buildBufferInfo(char* bufferInfoStorage, int bufferId,
                                              uint32_t clientBitMask, uint32_t userMetadataSize,
                                              int metadataFd, int eventFd) {
    native_handle_t* infoHandle =
            native_handle_init(bufferInfoStorage, BufferHubDefs::kBufferInfoNumFds,
                               BufferHubDefs::kBufferInfoNumInts);

    infoHandle->data[0] = metadataFd;
    infoHandle->data[1] = eventFd;
    infoHandle->data[2] = bufferId;
    // Use memcpy to convert to int without missing digit.
    // TOOD(b/121345852): use bit_cast to unpack bufferInfo when C++20 becomes available.
    memcpy(&infoHandle->data[3], &clientBitMask, sizeof(clientBitMask));
    memcpy(&infoHandle->data[4], &userMetadataSize, sizeof(userMetadataSize));

    hidl_handle bufferInfo;
    bufferInfo.setTo(infoHandle, /*shouldOwn=*/false);

    return bufferInfo;
}

void BufferHubService::removeTokenByClient(const BufferClient* client) {
    std::lock_guard<std::mutex> lock(mTokenMutex);
    auto iter = mTokenMap.begin();
    while (iter != mTokenMap.end()) {
        if (iter->second.second == client) {
            auto oldIter = iter;
            ++iter;
            mTokenMap.erase(oldIter);
        } else {
            ++iter;
        }
    }
}

} // namespace implementation
} // namespace V1_0
} // namespace bufferhub
} // namespace frameworks
} // namespace android
