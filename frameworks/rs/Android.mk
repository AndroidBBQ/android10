
LOCAL_PATH:=$(call my-dir)

.PHONY: rs-prebuilts-full
rs-prebuilts-full: \
    bcc_compat \
    llvm-rs-cc \
    libRSSupport \
    libRSSupportIO \
    libRScpp_static \
    libblasV8 \
    libcompiler_rt \
    librsrt_arm.bc \
    librsrt_arm64.bc \
    librsrt_mips.bc \
    librsrt_x86.bc \
    librsrt_x86_64.bc \
    libLLVM_android-host64 \
    libclang_android-host64

ifneq ($(HOST_OS),darwin)
rs-prebuilts-full: \
    host_cross_llvm-rs-cc \
    host_cross_bcc_compat
endif

include $(call all-makefiles-under,$(LOCAL_PATH))

