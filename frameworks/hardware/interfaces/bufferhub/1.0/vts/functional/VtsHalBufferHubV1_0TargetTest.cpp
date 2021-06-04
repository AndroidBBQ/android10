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

#define LOG_TAG "VtsHalBufferHubV1_0TargetTest"

#include <VtsHalHidlTargetTestBase.h>
#include <android-base/logging.h>
#include <android/frameworks/bufferhub/1.0/IBufferClient.h>
#include <android/frameworks/bufferhub/1.0/IBufferHub.h>
#include <android/hardware_buffer.h>
#include <gtest/gtest.h>
#include <hwbinder/IPCThreadState.h>
#include <ui/BufferHubDefs.h>

using ::android::frameworks::bufferhub::V1_0::BufferHubStatus;
using ::android::frameworks::bufferhub::V1_0::BufferTraits;
using ::android::frameworks::bufferhub::V1_0::IBufferClient;
using ::android::frameworks::bufferhub::V1_0::IBufferHub;
using ::android::hardware::hidl_handle;
using ::android::hardware::graphics::common::V1_2::HardwareBufferDescription;

namespace android {
namespace frameworks {
namespace bufferhub {
namespace vts {

// Stride is an output that unknown before allocation.
const AHardwareBuffer_Desc kDesc = {
    /*width=*/640UL, /*height=*/480UL,
    /*layers=*/1,    /*format=*/AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM,
    /*usage=*/0ULL,  /*stride=*/0UL,
    /*rfu0=*/0UL,    /*rfu1=*/0ULL};
const size_t kUserMetadataSize = 1;

// Test environment for BufferHub HIDL HAL.
class BufferHubHidlEnv : public ::testing::VtsHalHidlTargetTestEnvBase {
   public:
    // get the test environment singleton
    static BufferHubHidlEnv* Instance() {
        static BufferHubHidlEnv* instance = new BufferHubHidlEnv;
        return instance;
    }

    void registerTestServices() override { registerTestService<IBufferHub>(); }

   private:
    BufferHubHidlEnv() {}
};

class HalBufferHubVts : public ::testing::VtsHalHidlTargetTestBase {
   protected:
    void SetUp() override {
        VtsHalHidlTargetTestBase::SetUp();

        mBufferHub = IBufferHub::getService();
        ASSERT_NE(nullptr, mBufferHub.get());
    }

    sp<IBufferHub> mBufferHub;
};

// TOOD(b/121345852): use bit_cast to unpack bufferInfo when C++20 becomes available.
uint32_t clientStateMask(const BufferTraits& bufferTraits) {
    uint32_t clientStateMask;
    memcpy(&clientStateMask, &bufferTraits.bufferInfo->data[3], sizeof(clientStateMask));
    return clientStateMask;
}

// Helper function to verify that given bufferTrais:
// 1. is consistent with kDesc
// 2. have a non-null gralloc handle
// 3. have a non-null buffer info handle with:
//    1) metadata fd >= 0 (valid fd)
//    2) event fd >= 0 (valid fd)
//    3) buffer Id >= 0
//    4) client bit mask != 0
//    5) user metadata size = kUserMetadataSize
//
// The structure of BufferTraits.bufferInfo handle is defined in ui/BufferHubDefs.h
bool isValidTraits(const BufferTraits& bufferTraits) {
    AHardwareBuffer_Desc desc;
    memcpy(&desc, &bufferTraits.bufferDesc, sizeof(AHardwareBuffer_Desc));

    const native_handle_t* bufferInfo = bufferTraits.bufferInfo.getNativeHandle();
    if (bufferInfo == nullptr) {
        return false;
    }
    const int metadataFd = bufferInfo->data[0];
    const int eventFd = bufferInfo->data[1];
    const int bufferId = bufferInfo->data[2];
    uint32_t userMetadataSize;
    memcpy(&userMetadataSize, &bufferTraits.bufferInfo->data[4], sizeof(userMetadataSize));

    // Not comparing stride because it's unknown before allocation
    return desc.format == kDesc.format && desc.height == kDesc.height &&
           desc.layers == kDesc.layers && desc.usage == kDesc.usage && desc.width == kDesc.width &&
           bufferTraits.bufferHandle.getNativeHandle() != nullptr && metadataFd >= 0 &&
           eventFd >= 0 && bufferId >= 0 && clientStateMask(bufferTraits) != 0U &&
           userMetadataSize == kUserMetadataSize;
}

// Test IBufferHub::allocateBuffer then IBufferClient::close
TEST_F(HalBufferHubVts, AllocateAndFreeBuffer) {
    HardwareBufferDescription desc;
    memcpy(&desc, &kDesc, sizeof(HardwareBufferDescription));

    BufferHubStatus ret;
    sp<IBufferClient> client;
    BufferTraits bufferTraits = {};
    IBufferHub::allocateBuffer_cb callback = [&](const auto& status, const auto& outClient,
                                                 const auto& traits) {
        ret = status;
        client = outClient;
        bufferTraits = std::move(traits);
    };
    ASSERT_TRUE(mBufferHub->allocateBuffer(desc, kUserMetadataSize, callback).isOk());
    EXPECT_EQ(ret, BufferHubStatus::NO_ERROR);
    ASSERT_NE(nullptr, client.get());
    EXPECT_TRUE(isValidTraits(bufferTraits));

    ASSERT_EQ(BufferHubStatus::NO_ERROR, client->close());
    EXPECT_EQ(BufferHubStatus::CLIENT_CLOSED, client->close());
}

// Test destroying IBufferClient without calling close
TEST_F(HalBufferHubVts, DestroyClientWithoutClose) {
    HardwareBufferDescription desc;
    memcpy(&desc, &kDesc, sizeof(HardwareBufferDescription));

    BufferHubStatus ret;
    sp<IBufferClient> client;
    BufferTraits bufferTraits = {};
    IBufferHub::allocateBuffer_cb callback = [&](const auto& status, const auto& outClient,
                                                 const auto& traits) {
        ret = status;
        client = outClient;
        bufferTraits = std::move(traits);
    };
    ASSERT_TRUE(mBufferHub->allocateBuffer(desc, kUserMetadataSize, callback).isOk());
    EXPECT_EQ(ret, BufferHubStatus::NO_ERROR);
    ASSERT_NE(nullptr, client.get());
    EXPECT_TRUE(isValidTraits(bufferTraits));

    // Not calling client->close() before destruction here intentionally to see if anything would
    // break. User is recommended to call close() in any case.
    client.clear();

    // Flush the command to remote side, wait for 10ms, and ping service again to check if alive
    hardware::IPCThreadState::self()->flushCommands();
    usleep(10000);
    ASSERT_TRUE(mBufferHub->ping().isOk());
}

// Test IBufferClient::duplicate after IBufferClient::close
TEST_F(HalBufferHubVts, DuplicateFreedBuffer) {
    HardwareBufferDescription desc;
    memcpy(&desc, &kDesc, sizeof(HardwareBufferDescription));

    BufferHubStatus ret;
    sp<IBufferClient> client;
    BufferTraits bufferTraits = {};
    IBufferHub::allocateBuffer_cb callback = [&](const auto& status, const auto& outClient,
                                                 const auto& traits) {
        ret = status;
        client = outClient;
        bufferTraits = std::move(traits);
    };
    ASSERT_TRUE(mBufferHub->allocateBuffer(desc, kUserMetadataSize, callback).isOk());
    EXPECT_EQ(ret, BufferHubStatus::NO_ERROR);
    ASSERT_NE(nullptr, client.get());
    EXPECT_TRUE(isValidTraits(bufferTraits));

    ASSERT_EQ(BufferHubStatus::NO_ERROR, client->close());

    hidl_handle token;
    IBufferClient::duplicate_cb dupCb = [&](const auto& outToken, const auto& status) {
        token = outToken;
        ret = status;
    };
    ASSERT_TRUE(client->duplicate(dupCb).isOk());
    EXPECT_EQ(ret, BufferHubStatus::CLIENT_CLOSED);
    EXPECT_EQ(token.getNativeHandle(), nullptr);
}

// Test normal import process using IBufferHub::import function
TEST_F(HalBufferHubVts, DuplicateAndImportBuffer) {
    HardwareBufferDescription desc;
    memcpy(&desc, &kDesc, sizeof(HardwareBufferDescription));

    BufferHubStatus ret;
    sp<IBufferClient> client;
    BufferTraits bufferTraits = {};
    IBufferHub::allocateBuffer_cb callback = [&](const auto& status, const auto& outClient,
                                                 const auto& traits) {
        ret = status;
        client = outClient;
        bufferTraits = std::move(traits);
    };
    ASSERT_TRUE(mBufferHub->allocateBuffer(desc, kUserMetadataSize, callback).isOk());
    EXPECT_EQ(ret, BufferHubStatus::NO_ERROR);
    ASSERT_NE(nullptr, client.get());
    EXPECT_TRUE(isValidTraits(bufferTraits));

    hidl_handle token;
    IBufferClient::duplicate_cb dupCb = [&](const auto& outToken, const auto& status) {
        token = outToken;
        ret = status;
    };
    ASSERT_TRUE(client->duplicate(dupCb).isOk());
    EXPECT_EQ(ret, BufferHubStatus::NO_ERROR);
    ASSERT_NE(token.getNativeHandle(), nullptr);
    EXPECT_GT(token->numInts, 1);
    EXPECT_EQ(token->numFds, 0);

    sp<IBufferClient> client2;
    BufferTraits bufferTraits2 = {};
    IBufferHub::importBuffer_cb importCb = [&](const auto& status, const auto& outClient,
                                               const auto& traits) {
        ret = status;
        client2 = outClient;
        bufferTraits2 = std::move(traits);
    };
    ASSERT_TRUE(mBufferHub->importBuffer(token, importCb).isOk());
    EXPECT_EQ(ret, BufferHubStatus::NO_ERROR);
    EXPECT_NE(nullptr, client2.get());
    EXPECT_TRUE(isValidTraits(bufferTraits2));

    // Since they are two clients of one buffer, the id should be the same but client state bit mask
    // should be different.
    const int bufferId1 = bufferTraits.bufferInfo->data[2];
    const int bufferId2 = bufferTraits2.bufferInfo->data[2];
    EXPECT_EQ(bufferId1, bufferId2);
    EXPECT_NE(clientStateMask(bufferTraits), clientStateMask(bufferTraits2));

    EXPECT_EQ(BufferHubStatus::NO_ERROR, client->close());
    EXPECT_EQ(BufferHubStatus::NO_ERROR, client2->close());
}

// Test calling IBufferHub::import with nullptr. Must not crash the service
TEST_F(HalBufferHubVts, ImportNullToken) {
    hidl_handle nullToken;
    BufferHubStatus ret;
    sp<IBufferClient> client;
    BufferTraits bufferTraits = {};
    IBufferHub::importBuffer_cb importCb = [&](const auto& status, const auto& outClient,
                                               const auto& traits) {
        ret = status;
        client = outClient;
        bufferTraits = std::move(traits);
    };
    ASSERT_TRUE(mBufferHub->importBuffer(nullToken, importCb).isOk());
    EXPECT_EQ(ret, BufferHubStatus::INVALID_TOKEN);
    EXPECT_EQ(nullptr, client.get());
    EXPECT_FALSE(isValidTraits(bufferTraits));
}

// Test calling IBufferHub::import with an nonexistant token.
TEST_F(HalBufferHubVts, ImportInvalidToken) {
    native_handle_t* tokenHandle = native_handle_create(/*numFds=*/0, /*numInts=*/2);
    tokenHandle->data[0] = 0;
    // Assign a random number since we cannot know the HMAC value.
    tokenHandle->data[1] = 42;

    hidl_handle invalidToken(tokenHandle);
    BufferHubStatus ret;
    sp<IBufferClient> client;
    BufferTraits bufferTraits = {};
    IBufferHub::importBuffer_cb importCb = [&](const auto& status, const auto& outClient,
                                               const auto& traits) {
        ret = status;
        client = outClient;
        bufferTraits = std::move(traits);
    };
    ASSERT_TRUE(mBufferHub->importBuffer(invalidToken, importCb).isOk());
    EXPECT_EQ(ret, BufferHubStatus::INVALID_TOKEN);
    EXPECT_EQ(nullptr, client.get());
    EXPECT_FALSE(isValidTraits(bufferTraits));
}

// Test calling IBufferHub::import after the original IBufferClient is closed
TEST_F(HalBufferHubVts, ImportFreedBuffer) {
    HardwareBufferDescription desc;
    memcpy(&desc, &kDesc, sizeof(HardwareBufferDescription));

    BufferHubStatus ret;
    sp<IBufferClient> client;
    BufferTraits bufferTraits = {};
    IBufferHub::allocateBuffer_cb callback = [&](const auto& status, const auto& outClient,
                                                 const auto& traits) {
        ret = status;
        client = outClient;
        bufferTraits = std::move(traits);
    };
    ASSERT_TRUE(mBufferHub->allocateBuffer(desc, kUserMetadataSize, callback).isOk());
    EXPECT_EQ(ret, BufferHubStatus::NO_ERROR);
    ASSERT_NE(nullptr, client.get());
    EXPECT_TRUE(isValidTraits(bufferTraits));

    hidl_handle token;
    IBufferClient::duplicate_cb dupCb = [&](const auto& outToken, const auto& status) {
        token = outToken;
        ret = status;
    };
    ASSERT_TRUE(client->duplicate(dupCb).isOk());
    EXPECT_EQ(ret, BufferHubStatus::NO_ERROR);
    ASSERT_NE(token.getNativeHandle(), nullptr);
    EXPECT_GT(token->numInts, 1);
    EXPECT_EQ(token->numFds, 0);

    // Close the client. Now the token should be invalid.
    ASSERT_EQ(BufferHubStatus::NO_ERROR, client->close());

    sp<IBufferClient> client2;
    BufferTraits bufferTraits2 = {};
    IBufferHub::importBuffer_cb importCb = [&](const auto& status, const auto& outClient,
                                               const auto& traits) {
        ret = status;
        client2 = outClient;
        bufferTraits2 = std::move(traits);
    };
    ASSERT_TRUE(mBufferHub->importBuffer(token, importCb).isOk());
    EXPECT_EQ(ret, BufferHubStatus::INVALID_TOKEN);
    EXPECT_EQ(nullptr, client2.get());
    EXPECT_FALSE(isValidTraits(bufferTraits2));
}

}  // namespace vts
}  // namespace bufferhub
}  // namespace frameworks
}  // namespace android

int main(int argc, char** argv) {
    ::testing::AddGlobalTestEnvironment(
        android::frameworks::bufferhub::vts::BufferHubHidlEnv::Instance());
    ::testing::InitGoogleTest(&argc, argv);
    android::frameworks::bufferhub::vts::BufferHubHidlEnv::Instance()->init(&argc, argv);
    int status = RUN_ALL_TESTS();
    LOG(INFO) << "Test result = " << status;
    return status;
}
