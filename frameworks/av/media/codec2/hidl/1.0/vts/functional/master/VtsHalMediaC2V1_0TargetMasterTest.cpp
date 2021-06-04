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

//#define LOG_NDEBUG 0
#define LOG_TAG "codec2_hidl_hal_master_test"

#include <android-base/logging.h>
#include <gtest/gtest.h>

#include <codec2/hidl/client.h>

#include <VtsHalHidlTargetTestBase.h>
#include "media_c2_hidl_test_common.h"

static ComponentTestEnvironment* gEnv = nullptr;

namespace {

// google.codec2 Master test setup
class Codec2MasterHalTest : public ::testing::VtsHalHidlTargetTestBase {
   private:
    typedef ::testing::VtsHalHidlTargetTestBase Super;

   public:
    virtual void SetUp() override {
        Super::SetUp();
        mClient = android::Codec2Client::CreateFromService(
            gEnv->getInstance().c_str());
        ASSERT_NE(mClient, nullptr);
    }

   protected:
    static void description(const std::string& description) {
        RecordProperty("description", description);
    }

    std::shared_ptr<android::Codec2Client> mClient;
};

void displayComponentInfo(const std::vector<C2Component::Traits>& compList) {
    for (size_t i = 0; i < compList.size(); i++) {
        std::cout << compList[i].name << " | " << compList[i].domain;
        std::cout << " | " << compList[i].kind << "\n";
    }
}

// List Components
TEST_F(Codec2MasterHalTest, ListComponents) {
    ALOGV("ListComponents Test");

    C2String name = mClient->getName();
    EXPECT_NE(name.empty(), true) << "Invalid Codec2Client Name";

    // Get List of components from all known services
    const std::vector<C2Component::Traits> listTraits =
        mClient->ListComponents();

    if (listTraits.size() == 0)
        ALOGE("Warning, ComponentInfo list empty");
    else {
        (void)displayComponentInfo;
        for (size_t i = 0; i < listTraits.size(); i++) {
            std::shared_ptr<android::Codec2Client::Listener> listener;
            std::shared_ptr<android::Codec2Client::Component> component;
            listener.reset(new CodecListener());
            ASSERT_NE(listener, nullptr);

            // Create component from all known services
            component = mClient->CreateComponentByName(
                listTraits[i].name.c_str(), listener, &mClient);
            ASSERT_NE(component, nullptr) << "Create component failed for "
                                          << listTraits[i].name.c_str();
        }
    }
}

}  // anonymous namespace

int main(int argc, char** argv) {
    gEnv = new ComponentTestEnvironment();
    ::testing::InitGoogleTest(&argc, argv);
    gEnv->init(&argc, argv);
    int status = gEnv->initFromOptions(argc, argv);
    if (status == 0) {
        status = RUN_ALL_TESTS();
        LOG(INFO) << "C2 Test result = " << status;
    }
    return status;
}
