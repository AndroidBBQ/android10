#
# Copyright (C) 2014 The Android Open Source Project
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

# This file contains target-specific defines for projects including LLVM
# and/or libbcc directly.

LOCAL_CFLAGS_arm += -DFORCE_ARM_CODEGEN
ifeq ($(ARCH_ARM_HAVE_VFP),true)
  LOCAL_CFLAGS_arm += -DARCH_ARM_HAVE_VFP
  ifeq ($(ARCH_ARM_HAVE_VFP_D32),true)
    LOCAL_CFLAGS_arm += -DARCH_ARM_HAVE_VFP_D32
  endif
endif
ifeq ($(ARCH_ARM_HAVE_NEON),true)
  LOCAL_CFLAGS_arm += -DARCH_ARM_HAVE_NEON
endif

LOCAL_CFLAGS_arm64 += -DFORCE_ARM64_CODEGEN -DARCH_ARM_HAVE_NEON -DARCH_ARM_HAVE_VFP -DARCH_ARM_HAVE_VFP_D32 -DDISABLE_CLCORE_NEON
LOCAL_CFLAGS_mips += -DFORCE_MIPS_CODEGEN
LOCAL_CFLAGS_mips64 += -DFORCE_MIPS64_CODEGEN

LOCAL_CFLAGS_x86 += -DFORCE_X86_CODEGEN
LOCAL_CFLAGS_x86_64 += -DFORCE_X86_64_CODEGEN

ifeq ($(BUILD_ARM_FOR_X86),true)
LOCAL_CFLAGS_x86 += -DPROVIDE_ARM_CODEGEN -DFORCE_BUILD_ARM
LOCAL_CFLAGS_x86_64 += -DPROVIDE_ARM_CODEGEN -DFORCE_BUILD_ARM -DPROVIDE_ARM64_CODEGEN
endif


ifeq (,$(filter $(TARGET_ARCH),arm64 arm mips mips64 x86 x86_64))
  $(error Unsupported architecture $(TARGET_ARCH))
endif

