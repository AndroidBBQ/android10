local_target_dir := $(TARGET_OUT_DATA)/local/tmp
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:=   \
    Composers.cpp   \
    GLHelper.cpp    \
    Renderers.cpp   \
    Main.cpp        \

LOCAL_CFLAGS := -Wall -Werror

LOCAL_MODULE:= flatland

LOCAL_MODULE_TAGS := tests

LOCAL_MODULE_PATH := $(local_target_dir)
LOCAL_MULTILIB := both
LOCAL_MODULE_STEM_32 := flatland
LOCAL_MODULE_STEM_64 := flatland64
LOCAL_SHARED_LIBRARIES := \
    libEGL      \
    libGLESv2   \
    libcutils   \
    libgui      \
    libui       \
    libutils    \

include $(BUILD_EXECUTABLE)
