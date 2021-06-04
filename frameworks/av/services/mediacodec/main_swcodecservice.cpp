/*
**
** Copyright 2018, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#include <android-base/logging.h>

// from LOCAL_C_INCLUDES
#include "minijail.h"
#include <hidl/HidlTransportSupport.h>

using namespace android;

// kSystemSeccompPolicyPath points to the policy for the swcodecs themselves and
// is part of the updates. kVendorSeccompPolicyPath points to any additional
// policies that the vendor may need for the device.
static const char kSystemSeccompPolicyPath[] =
        "/apex/com.android.media.swcodec/etc/seccomp_policy/mediaswcodec.policy";
static const char kVendorSeccompPolicyPath[] =
        "/vendor/etc/seccomp_policy/mediaswcodec.policy";

extern "C" void RegisterCodecServices();

int main(int argc __unused, char** argv)
{
    LOG(INFO) << "media swcodec service starting";
    signal(SIGPIPE, SIG_IGN);
    SetUpMinijail(kSystemSeccompPolicyPath, kVendorSeccompPolicyPath);
    strcpy(argv[0], "media.swcodec");

    ::android::hardware::configureRpcThreadpool(64, false);

    RegisterCodecServices();

    ::android::hardware::joinRpcThreadpool();
}
