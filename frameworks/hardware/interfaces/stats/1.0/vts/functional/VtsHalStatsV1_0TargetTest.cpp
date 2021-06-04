/*
 * Copyright (C) 2017 The Android Open Source Project
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

#define LOG_TAG "stats_hidl_hal_test"
#include <android-base/logging.h>
#include <android/frameworks/stats/1.0/IStats.h>

#include <VtsHalHidlTargetTestBase.h>
#include <VtsHalHidlTargetTestEnvBase.h>

#include <utils/StrongPointer.h>

using android::sp;
using android::frameworks::stats::V1_0::BatteryCausedShutdown;
using android::frameworks::stats::V1_0::BatteryHealthSnapshotArgs;
using android::frameworks::stats::V1_0::ChargeCycles;
using android::frameworks::stats::V1_0::HardwareFailed;
using android::frameworks::stats::V1_0::IStats;
using android::frameworks::stats::V1_0::SlowIo;
using android::frameworks::stats::V1_0::SpeakerImpedance;
using android::frameworks::stats::V1_0::UsbPortOverheatEvent;
using android::frameworks::stats::V1_0::VendorAtom;
using Value = android::frameworks::stats::V1_0::VendorAtom::Value;
using android::hardware::Return;

// Test environment for Power HIDL HAL.
class StatsHidlEnvironment : public ::testing::VtsHalHidlTargetTestEnvBase {
   public:
    // get the test environment singleton
    static StatsHidlEnvironment* Instance() {
        static StatsHidlEnvironment* instance = new StatsHidlEnvironment;
        return instance;
    }

    virtual void registerTestServices() override { registerTestService<IStats>(); }
};

class StatsHidlTest : public ::testing::VtsHalHidlTargetTestBase {
   public:
    virtual void SetUp() override {
        client = ::testing::VtsHalHidlTargetTestBase::getService<IStats>(
            StatsHidlEnvironment::Instance()->getServiceName<IStats>());
        ASSERT_NE(client, nullptr);
    }

    virtual void TearDown() override {}

    sp<IStats> client;
};

// Sanity check IStats::reportSpeakerImpedance.
TEST_F(StatsHidlTest, reportSpeakerImpedance) {
    SpeakerImpedance impedance = {.speakerLocation = 0,
                                  .milliOhms = static_cast<int32_t>(1234 * 1000)};
    Return<void> ret;
    ret = client->reportSpeakerImpedance(impedance);
    ASSERT_TRUE(ret.isOk());
}

// Sanity check IStats::reportHardwareFailed.
TEST_F(StatsHidlTest, reportHardwareFailed) {
    HardwareFailed failed = {.hardwareType = HardwareFailed::HardwareType::CODEC,
                             .hardwareLocation = 0,
                             .errorCode = HardwareFailed::HardwareErrorCode::COMPLETE};
    Return<void> ret;

    ret = client->reportHardwareFailed(failed);
    ASSERT_TRUE(ret.isOk());
}

// Sanity check IStats::reportChargeCycles.
TEST_F(StatsHidlTest, reportChargeCycles) {
    std::vector<int> charge_cycles = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    ChargeCycles cycles;
    cycles.cycleBucket = charge_cycles;

    Return<void> ret;
    ret = client->reportChargeCycles(cycles);
    ASSERT_TRUE(ret.isOk());
}

// Sanity check IStats::reportBatteryHealthSnapshot.
TEST_F(StatsHidlTest, reportBatteryHealthSnapshot) {
    BatteryHealthSnapshotArgs args{.temperatureDeciC = 3000,
                                   .voltageMicroV = 1,
                                   .currentMicroA = 2,
                                   .openCircuitVoltageMicroV = 3,
                                   .resistanceMicroOhm = 5,
                                   .levelPercent = 101};

    Return<void> ret;
    ret = client->reportBatteryHealthSnapshot(args);
    /* TODO: Test all enum values and/or a bad enum value */
    ASSERT_TRUE(ret.isOk());
}

// Sanity check IStats::reportSlowIo.
TEST_F(StatsHidlTest, reportSlowIo) {
    SlowIo slowio = {.operation = SlowIo::IoOperation::READ, .count = 5};

    Return<void> ret;
    ret = client->reportSlowIo(slowio);
    /* TODO: Test all enum values and/or a bad enum value */
    ASSERT_TRUE(ret.isOk());
}

// Sanity check IStats::reportBatteryCausedShutdown.
TEST_F(StatsHidlTest, reportBatteryCausedShutdown) {
    BatteryCausedShutdown shutdown = {.voltageMicroV = 3};

    Return<void> ret;
    ret = client->reportBatteryCausedShutdown(shutdown);
    ASSERT_TRUE(ret.isOk());
}

// Sanity check IStats::reportUsbPortOverheatEvent.
TEST_F(StatsHidlTest, reportUsbPortOverheatEvent) {
    UsbPortOverheatEvent event = {.maxTemperatureDeciC = 220,
                                  .plugTemperatureDeciC = 210,
                                  .timeToOverheat = 1,
                                  .timeToHysteresis = 2,
                                  .timeToInactive = 3};

    Return<void> ret;
    ret = client->reportUsbPortOverheatEvent(event);
    ASSERT_TRUE(ret.isOk());
}

// Sanity check IStats::reportVendorAtom.
TEST_F(StatsHidlTest, reportVendorAtom) {
    std::vector<Value> values;
    Value tmp;
    tmp.longValue(70000);
    values.push_back(tmp);
    tmp.intValue(7);
    values.push_back(tmp);
    tmp.floatValue(8.5);
    values.push_back(tmp);
    tmp.stringValue("test");
    values.push_back(tmp);
    tmp.intValue(3);
    values.push_back(tmp);
    VendorAtom atom = {.reverseDomainName = "com.google.pixel", .atomId = 100001, .values = values};

    Return<void> ret;
    client->reportVendorAtom(atom);
    ASSERT_TRUE(ret.isOk());
}

int main(int argc, char** argv) {
    ::testing::AddGlobalTestEnvironment(StatsHidlEnvironment::Instance());
    ::testing::InitGoogleTest(&argc, argv);
    StatsHidlEnvironment::Instance()->init(&argc, argv);
    int status = RUN_ALL_TESTS();
    LOG(INFO) << "Test result = " << status;
    return status;
}
