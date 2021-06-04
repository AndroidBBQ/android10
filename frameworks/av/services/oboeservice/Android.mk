LOCAL_PATH:= $(call my-dir)

# AAudio Service
include $(CLEAR_VARS)

LOCAL_MODULE := libaaudioservice
LOCAL_MODULE_TAGS := optional

LIBAAUDIO_DIR := ../../media/libaaudio
LIBAAUDIO_SRC_DIR := $(LIBAAUDIO_DIR)/src

LOCAL_C_INCLUDES := \
    $(TOPDIR)frameworks/av/services/audioflinger \
    $(call include-path-for, audio-utils) \
    frameworks/native/include \
    system/core/base/include \
    $(TOP)/frameworks/native/media/libaaudio/include/include \
    $(TOP)/frameworks/av/media/libaaudio/include \
    $(TOP)/frameworks/av/media/utils/include \
    frameworks/native/include \
    $(TOP)/external/tinyalsa/include \
    $(TOP)/frameworks/av/media/libaaudio/src

LOCAL_SRC_FILES += \
    SharedMemoryProxy.cpp \
    SharedRingBuffer.cpp \
    AAudioClientTracker.cpp \
    AAudioEndpointManager.cpp \
    AAudioMixer.cpp \
    AAudioService.cpp \
    AAudioServiceEndpoint.cpp \
    AAudioServiceEndpointCapture.cpp \
    AAudioServiceEndpointMMAP.cpp \
    AAudioServiceEndpointPlay.cpp \
    AAudioServiceEndpointShared.cpp \
    AAudioServiceStreamBase.cpp \
    AAudioServiceStreamMMAP.cpp \
    AAudioServiceStreamShared.cpp \
    AAudioStreamTracker.cpp \
    TimestampScheduler.cpp \
    AAudioThread.cpp

LOCAL_MULTILIB := $(AUDIOSERVER_MULTILIB)

# LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_CFLAGS += -Wno-unused-parameter
LOCAL_CFLAGS += -Wall -Werror

LOCAL_SHARED_LIBRARIES :=  \
    libaaudio \
    libaudioflinger \
    libaudioclient \
    libbinder \
    libcutils \
    libmediautils \
    libutils \
    liblog

include $(BUILD_SHARED_LIBRARY)


