/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "VtsFwkDisplayServiceV1_0TargetTest"

#include <android/frameworks/displayservice/1.0/IDisplayEventReceiver.h>
#include <android/frameworks/displayservice/1.0/IDisplayService.h>
#include <android/frameworks/displayservice/1.0/IEventCallback.h>
#include <log/log.h>
#include <VtsHalHidlTargetTestBase.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <inttypes.h>
#include <thread>

using ::android::frameworks::displayservice::V1_0::IDisplayEventReceiver;
using ::android::frameworks::displayservice::V1_0::IDisplayService;
using ::android::frameworks::displayservice::V1_0::IEventCallback;
using ::android::frameworks::displayservice::V1_0::Status;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using namespace ::std::chrono_literals;

#define ASSERT_OK(ret) ASSERT_TRUE((ret).isOk())
#define EXPECT_SUCCESS(retExpr) do { \
        Return<Status> retVal = (retExpr); \
        ASSERT_OK(retVal); \
        EXPECT_EQ(Status::SUCCESS, static_cast<Status>(retVal)); \
    } while(false)
#define EXPECT_BAD_VALUE(retExpr) do { \
        Return<Status> retVal = (retExpr); \
        ASSERT_OK(retVal); \
        EXPECT_EQ(Status::BAD_VALUE, static_cast<Status>(retVal)); \
    } while(false)

#define MAX_INACCURACY 3

class TestCallback : public IEventCallback {
public:
    Return<void> onVsync(uint64_t timestamp, uint32_t count) override {
        ALOGE("onVsync: timestamp=%" PRIu64 " count=%d", timestamp, count);

        vsyncs++;
        return Void();
    }
    Return<void> onHotplug(uint64_t timestamp, bool connected) override {
        ALOGE("onVsync: timestamp=%" PRIu64 " connected=%s", timestamp, connected ? "true" : "false");

        hotplugs++;
        return Void();
    }

    std::atomic<int> vsyncs{0};
    std::atomic<int> hotplugs{0};
};

class DisplayServiceTest : public ::testing::VtsHalHidlTargetTestBase {
public:
    ~DisplayServiceTest() {}

    virtual void SetUp() override {
        sp<IDisplayService> service = ::testing::VtsHalHidlTargetTestBase::getService<IDisplayService>();

        ASSERT_NE(service, nullptr);

        Return<sp<IDisplayEventReceiver>> ret = service->getEventReceiver();
        ASSERT_OK(ret);

        receiver = ret;
        ASSERT_NE(receiver, nullptr);


        cb = new TestCallback();
        EXPECT_SUCCESS(receiver->init(cb));
    }

    virtual void TearDown() override {
        EXPECT_SUCCESS(receiver->close());
    }

    sp<TestCallback> cb;
    sp<IDisplayEventReceiver> receiver;
};

/**
 * No vsync events should happen unless you explicitly request one.
 */
TEST_F(DisplayServiceTest, TestAttachRequestVsync) {
    EXPECT_EQ(0, cb->vsyncs);

    EXPECT_SUCCESS(receiver->requestNextVsync());

    std::this_thread::sleep_for(100ms); // framerate is not fixed on Android devices
    EXPECT_EQ(1, cb->vsyncs);
}

/**
 * Vsync rate respects count.
 */
TEST_F(DisplayServiceTest, TestSetVsyncRate) {
    ASSERT_EQ(0, cb->vsyncs);

    EXPECT_SUCCESS(receiver->setVsyncRate(1));
    std::this_thread::sleep_for(250ms);
    int at1 = cb->vsyncs;

    cb->vsyncs = 0;
    EXPECT_SUCCESS(receiver->setVsyncRate(2));
    std::this_thread::sleep_for(250ms);
    int at2 = cb->vsyncs;

    cb->vsyncs = 0;
    EXPECT_SUCCESS(receiver->setVsyncRate(4));
    std::this_thread::sleep_for(250ms);
    int at4 = cb->vsyncs;

    EXPECT_NE(0, at1);
    EXPECT_NE(0, at2);
    EXPECT_NE(0, at4);

    EXPECT_LE(std::abs(at1 - 2 * at2), 2 * MAX_INACCURACY);
    EXPECT_LE(std::abs(at1 - 4 * at4), 4 * MAX_INACCURACY);
    EXPECT_LE(std::abs(at2 - 2 * at4), 2 * MAX_INACCURACY);

    ALOGE("Vsync counts: %d %d %d", at1, at2, at4);
}

/**
 * Open/close should return proper error results.
 */
TEST_F(DisplayServiceTest, TestOpenClose) {
    EXPECT_BAD_VALUE(receiver->init(cb)); // already opened in SetUp
    EXPECT_SUCCESS(receiver->close()); // can close what was originally opened
    EXPECT_BAD_VALUE(receiver->close()); // can't close again
    EXPECT_SUCCESS(receiver->init(cb)); // open so can close again in SetUp
}

/**
 * Vsync must be given a value that is >= 0.
 */
TEST_F(DisplayServiceTest, TestVsync) {
    EXPECT_SUCCESS(receiver->setVsyncRate(0));
    EXPECT_SUCCESS(receiver->setVsyncRate(5));
    EXPECT_SUCCESS(receiver->setVsyncRate(0));
    EXPECT_BAD_VALUE(receiver->setVsyncRate(-1));
    EXPECT_BAD_VALUE(receiver->setVsyncRate(-1000));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int status = RUN_ALL_TESTS();
    ALOGE("Test status = %d", status);
    return status;
}
