#
# Copyright (C) 2016 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH :=$(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libRSDriver_RSoV

LOCAL_SRC_FILES := \
    rsovAllocation.cpp \
    rsovContext.cpp \
    rsovCore.cpp \
    rsovElement.cpp \
    rsovRuntimeStubs.cpp \
    rsovSampler.cpp \
    rsovScript.cpp \
    rsovScriptGroup.cpp \
    rsovType.cpp \

LOCAL_SHARED_LIBRARIES := \
    libRS_internal \
    libRSCpuRef \
    libbcinfo \
    libc++ \
    liblog \
    libspirit \
    libvulkan \
    libgui

LOCAL_C_INCLUDES := \
    frameworks/compile/libbcc/include \
    frameworks/native/vulkan/include \
    frameworks/rs \
    frameworks/rs/cpu_ref \
    frameworks/rs/rsov/compiler \

LOCAL_C_INCLUDES += \

LOCAL_CFLAGS := -Werror -Wall -Wextra
# TODO: remove warnings on unused variables and parameters
LOCAL_CFLAGS += -Wno-unused-variable -Wno-unused-parameter

include $(BUILD_SHARED_LIBRARY)
