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
    ClearKeyCasPlugin.cpp \
    ClearKeyFetcher.cpp \
    ClearKeyLicenseFetcher.cpp \
    ClearKeySessionLibrary.cpp \
    ecm.cpp \
    ecm_generator.cpp \
    JsonAssetLoader.cpp \
    protos/license_protos.proto \

LOCAL_MODULE := libclearkeycasplugin

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := mediacas

LOCAL_SHARED_LIBRARIES := \
    libutils \
    liblog \
    libcrypto \
    libstagefright_foundation \
    libprotobuf-cpp-lite \

LOCAL_HEADER_LIBRARIES := \
    media_plugin_headers

LOCAL_STATIC_LIBRARIES := \
    libjsmn \

LOCAL_MODULE_CLASS := SHARED_LIBRARIES

LOCAL_PROTOC_OPTIMIZE_TYPE := full

define proto_includes
$(call local-generated-sources-dir)/proto/$(LOCAL_PATH)
endef

LOCAL_C_INCLUDES += \
    external/jsmn \
    frameworks/av/include \
    frameworks/native/include/media \
    $(call proto_includes)

LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(call proto_includes)

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

#########################################################################
# Build unit tests

include $(LOCAL_PATH)/tests/Android.mk
