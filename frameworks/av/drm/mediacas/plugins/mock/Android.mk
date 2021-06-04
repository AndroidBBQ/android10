#
# Copyright (C) 2017 The Android Open Source Project
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
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    MockCasPlugin.cpp \
    MockSessionLibrary.cpp \

LOCAL_MODULE := libmockcasplugin

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := mediacas

LOCAL_SHARED_LIBRARIES := \
    libutils liblog

LOCAL_HEADER_LIBRARIES := media_plugin_headers

LOCAL_C_INCLUDES += \
    $(TOP)/frameworks/av/include \
    $(TOP)/frameworks/native/include/media \

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
