# Copyright 2005 The Android Open Source Project
#
# Android.mk for resampler_tools
#


LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	fir.cpp

LOCAL_MODULE := fir

LOCAL_CFLAGS := -Werror -Wall

include $(BUILD_HOST_EXECUTABLE)
