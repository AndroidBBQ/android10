// Copyright (C) 2017 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>

#include <android-base/file.h>
#include <android-base/unique_fd.h>

#include <gtest/gtest.h>

#include "minijail.h"

class WritePolicyTest : public ::testing::Test
{
  protected:
    const std::string base_policy_ =
        "read: 1\n"
        "write: 1\n"
        "rt_sigreturn: 1\n"
        "exit: 1\n";

    const std::string additional_policy_ =
        "mmap: 1\n"
        "munmap: 1\n";

    const std::string full_policy_ = base_policy_ + std::string("\n") + additional_policy_;
};

TEST_F(WritePolicyTest, OneFile)
{
    std::string final_string;
    android::base::unique_fd fd(android::WritePolicyToPipe(base_policy_, std::string()));
    EXPECT_LE(0, fd.get());
    bool success = android::base::ReadFdToString(fd.get(), &final_string);
    EXPECT_TRUE(success);
    EXPECT_EQ(final_string, base_policy_);
}

TEST_F(WritePolicyTest, TwoFiles)
{
    std::string final_string;
    android::base::unique_fd fd(android::WritePolicyToPipe(base_policy_, additional_policy_));
    EXPECT_LE(0, fd.get());
    bool success = android::base::ReadFdToString(fd.get(), &final_string);
    EXPECT_TRUE(success);
    EXPECT_EQ(final_string, full_policy_);
}
