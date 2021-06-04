LOCAL_PATH := $(call my-dir)

# vendor service seccomp policy
ifeq ($(TARGET_ARCH),$(filter $(TARGET_ARCH), x86 x86_64 arm arm64))
include $(CLEAR_VARS)
LOCAL_MODULE := codec2.vendor.base.policy
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/seccomp_policy
LOCAL_REQUIRED_MODULES := crash_dump.policy
ifdef TARGET_2ND_ARCH
    ifneq ($(TARGET_TRANSLATE_2ND_ARCH),true)
        LOCAL_SRC_FILES := seccomp_policy/codec2.vendor.base-$(TARGET_2ND_ARCH).policy
    else
        LOCAL_SRC_FILES := seccomp_policy/codec2.vendor.base-$(TARGET_ARCH).policy
    endif
else
    LOCAL_SRC_FILES := seccomp_policy/codec2.vendor.base-$(TARGET_ARCH).policy
endif
include $(BUILD_PREBUILT)
endif

include $(call all-makefiles-under, $(LOCAL_PATH))

