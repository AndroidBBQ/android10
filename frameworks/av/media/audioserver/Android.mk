LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	main_audioserver.cpp \

LOCAL_SHARED_LIBRARIES := \
	libaaudioservice \
	libaudioflinger \
	libaudiopolicyservice \
	libbinder \
	libcutils \
	liblog \
	libhidlbase \
	libhidltransport \
	libhwbinder \
	libmedia \
	libmedialogservice \
	libmediautils \
	libnbaio \
	libnblog \
	libsoundtriggerservice \
	libutils \
	libvibrator

# TODO oboeservice is the old folder name for aaudioservice. It will be changed.
LOCAL_C_INCLUDES := \
	frameworks/av/services/audioflinger \
	frameworks/av/services/audiopolicy \
	frameworks/av/services/audiopolicy/common/managerdefinitions/include \
	frameworks/av/services/audiopolicy/common/include \
	frameworks/av/services/audiopolicy/engine/interface \
	frameworks/av/services/audiopolicy/service \
	frameworks/av/services/medialog \
	frameworks/av/services/oboeservice \
	frameworks/av/services/radio \
	frameworks/av/services/soundtrigger \
	frameworks/av/media/libaaudio/include \
	frameworks/av/media/libaaudio/src \
	frameworks/av/media/libaaudio/src/binding \
	frameworks/av/media/libmedia \
	$(call include-path-for, audio-utils) \
	external/sonic \

# If AUDIOSERVER_MULTILIB in device.mk is non-empty then it is used to control
# the LOCAL_MULTILIB for all audioserver exclusive libraries.
# This is relevant for 64 bit architectures where either or both
# 32 and 64 bit libraries may be built.
#
# AUDIOSERVER_MULTILIB may be set as follows:
#   32      to build 32 bit audioserver libraries and 32 bit audioserver.
#   64      to build 64 bit audioserver libraries and 64 bit audioserver.
#   both    to build both 32 bit and 64 bit libraries,
#           and use primary target architecture (32 or 64) for audioserver.
#   first   to build libraries and audioserver for the primary target architecture only.
#   <empty> to build both 32 and 64 bit libraries and primary target audioserver.

LOCAL_MULTILIB := $(AUDIOSERVER_MULTILIB)

LOCAL_MODULE := audioserver

LOCAL_INIT_RC := audioserver.rc

LOCAL_CFLAGS := -Werror -Wall

include $(BUILD_EXECUTABLE)
