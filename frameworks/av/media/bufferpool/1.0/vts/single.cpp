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

#define LOG_TAG "buffferpool_unit_test"

#include <gtest/gtest.h>

#include <android-base/logging.h>
#include <binder/ProcessState.h>
#include <bufferpool/ClientManager.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <hidl/LegacySupport.h>
#include <hidl/Status.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <vector>
#include "allocator.h"

using android::hardware::hidl_handle;
using android::hardware::media::bufferpool::V1_0::ResultStatus;
using android::hardware::media::bufferpool::V1_0::implementation::BufferId;
using android::hardware::media::bufferpool::V1_0::implementation::ClientManager;
using android::hardware::media::bufferpool::V1_0::implementation::ConnectionId;
using android::hardware::media::bufferpool::V1_0::implementation::TransactionId;
using android::hardware::media::bufferpool::BufferPoolData;

namespace {

// Number of iteration for buffer allocation test.
constexpr static int kNumAllocationTest = 3;

// Number of iteration for buffer recycling test.
constexpr static int kNumRecycleTest = 3;

// media.bufferpool test setup
class BufferpoolSingleTest : public ::testing::Test {
 public:
  virtual void SetUp() override {
    ResultStatus status;
    mConnectionValid = false;

    mManager = ClientManager::getInstance();
    ASSERT_NE(mManager, nullptr);

    mAllocator = std::make_shared<TestBufferPoolAllocator>();
    ASSERT_TRUE((bool)mAllocator);

    status = mManager->create(mAllocator, &mConnectionId);
    ASSERT_TRUE(status == ResultStatus::OK);

    mConnectionValid = true;

    status = mManager->registerSender(mManager, mConnectionId, &mReceiverId);
    ASSERT_TRUE(status == ResultStatus::ALREADY_EXISTS &&
                mReceiverId == mConnectionId);
  }

  virtual void TearDown() override {
    if (mConnectionValid) {
      mManager->close(mConnectionId);
    }
  }

 protected:
  static void description(const std::string& description) {
    RecordProperty("description", description);
  }

  android::sp<ClientManager> mManager;
  std::shared_ptr<BufferPoolAllocator> mAllocator;
  bool mConnectionValid;
  ConnectionId mConnectionId;
  ConnectionId mReceiverId;

};

// Buffer allocation test.
// Check whether each buffer allocation is done successfully with
// unique buffer id.
TEST_F(BufferpoolSingleTest, AllocateBuffer) {
  ResultStatus status;
  std::vector<uint8_t> vecParams;
  getTestAllocatorParams(&vecParams);

  std::shared_ptr<BufferPoolData> buffer[kNumAllocationTest];
  native_handle_t *allocHandle = nullptr;
  for (int i = 0; i < kNumAllocationTest; ++i) {
    status = mManager->allocate(mConnectionId, vecParams, &allocHandle, &buffer[i]);
    ASSERT_TRUE(status == ResultStatus::OK);
  }
  for (int i = 0; i < kNumAllocationTest; ++i) {
    for (int j = i + 1; j < kNumAllocationTest; ++j) {
      ASSERT_TRUE(buffer[i]->mId != buffer[j]->mId);
    }
  }
  EXPECT_TRUE(kNumAllocationTest > 1);
}

// Buffer recycle test.
// Check whether de-allocated buffers are recycled.
TEST_F(BufferpoolSingleTest, RecycleBuffer) {
  ResultStatus status;
  std::vector<uint8_t> vecParams;
  getTestAllocatorParams(&vecParams);

  BufferId bid[kNumRecycleTest];
  for (int i = 0; i < kNumRecycleTest; ++i) {
    std::shared_ptr<BufferPoolData> buffer;
    native_handle_t *allocHandle = nullptr;
    status = mManager->allocate(mConnectionId, vecParams, &allocHandle, &buffer);
    ASSERT_TRUE(status == ResultStatus::OK);
    bid[i] = buffer->mId;
  }
  for (int i = 1; i < kNumRecycleTest; ++i) {
    ASSERT_TRUE(bid[i - 1] == bid[i]);
  }
  EXPECT_TRUE(kNumRecycleTest > 1);
}

// Buffer transfer test.
// Check whether buffer is transferred to another client successfully.
TEST_F(BufferpoolSingleTest, TransferBuffer) {
  ResultStatus status;
  std::vector<uint8_t> vecParams;
  getTestAllocatorParams(&vecParams);
  std::shared_ptr<BufferPoolData> sbuffer, rbuffer;
  native_handle_t *allocHandle = nullptr;
  native_handle_t *recvHandle = nullptr;

  TransactionId transactionId;
  int64_t postUs;

  status = mManager->allocate(mConnectionId, vecParams, &allocHandle, &sbuffer);
  ASSERT_TRUE(status == ResultStatus::OK);
  ASSERT_TRUE(TestBufferPoolAllocator::Fill(allocHandle, 0x77));
  status = mManager->postSend(mReceiverId, sbuffer, &transactionId, &postUs);
  ASSERT_TRUE(status == ResultStatus::OK);
  status = mManager->receive(mReceiverId, transactionId, sbuffer->mId, postUs,
                             &recvHandle, &rbuffer);
  EXPECT_TRUE(status == ResultStatus::OK);
  ASSERT_TRUE(TestBufferPoolAllocator::Verify(recvHandle, 0x77));
}

}  // anonymous namespace

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int status = RUN_ALL_TESTS();
  LOG(INFO) << "Test result = " << status;
  return status;
}
