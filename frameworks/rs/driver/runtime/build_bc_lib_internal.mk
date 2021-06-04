#
# Copyright (C) 2012 The Android Open Source Project
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

ifndef BCC_RS_TRIPLE
BCC_RS_TRIPLE := $($(LOCAL_2ND_ARCH_VAR_PREFIX)RS_TRIPLE)
endif

# Set these values always by default
LOCAL_MODULE_CLASS := RENDERSCRIPT_BITCODE

include $(BUILD_SYSTEM)/base_rules.mk

BCC_STRIP_ATTR := $(BUILD_OUT_EXECUTABLES)/bcc_strip_attr$(BUILD_EXECUTABLE_SUFFIX)

bc_clang := $(RS_CLANG)
ifdef RS_DRIVER_CLANG_EXE
bc_clang := $(RS_DRIVER_CLANG_EXE)
endif

# Disable deprecated warnings, because we have to support even legacy APIs.
bc_warning_flags := -Wno-deprecated -Werror

bc_cflags := -MD \
             $(RS_VERSION_DEFINE) \
             -std=c99 \
             -c \
             -O3 \
             -fno-builtin \
             -emit-llvm \
             -target $(BCC_RS_TRIPLE) \
             -fsigned-char \
             $($(LOCAL_2ND_ARCH_VAR_PREFIX)RS_TRIPLE_CFLAGS) \
             $(bc_warning_flags) \
             $(LOCAL_CFLAGS) \
             $(LOCAL_CFLAGS_$(my_32_64_bit_suffix)) \
             -x renderscript

ifeq ($(rs_debug_runtime),1)
    bc_cflags += -DRS_DEBUG_RUNTIME
endif

ifeq ($(rs_g_runtime),1)
    bc_cflags += -DRS_G_RUNTIME
endif

bc_src_files := $(LOCAL_SRC_FILES)
bc_src_files += $(LOCAL_SRC_FILES_$(TARGET_$(LOCAL_2ND_ARCH_VAR_PREFIX)ARCH)) $(LOCAL_SRC_FILES_$(my_32_64_bit_suffix))

c_sources := $(filter %.c,$(bc_src_files))
ll_sources := $(filter %.ll,$(bc_src_files))

c_bc_files := $(patsubst %.c,%.bc, \
    $(addprefix $(intermediates)/, $(c_sources)))

ll_bc_files := $(patsubst %.ll,%.bc, \
    $(addprefix $(intermediates)/, $(ll_sources)))

$(c_bc_files): PRIVATE_INCLUDES := \
    frameworks/rs/script_api/include \
    external/clang/lib/Headers
$(c_bc_files): PRIVATE_CFLAGS := $(bc_cflags)

$(c_bc_files): $(intermediates)/%.bc: $(LOCAL_PATH)/%.c $(bc_clang)
	@echo "bc: $(PRIVATE_MODULE) <= $<"
	@mkdir -p $(dir $@)
	$(hide) $(RELATIVE_PWD) $(bc_clang) $(addprefix -I, $(PRIVATE_INCLUDES)) $(PRIVATE_CFLAGS) $< -o $@

$(ll_bc_files): $(intermediates)/%.bc: $(LOCAL_PATH)/%.ll $(RS_LLVM_AS)
	@mkdir -p $(dir $@)
	$(hide) $(RELATIVE_PWD) $(RS_LLVM_AS) $< -o $@

$(foreach f,$(c_bc_files),$(call include-depfile,$(f:%.bc=%.d),$(f)))

$(LOCAL_BUILT_MODULE): PRIVATE_BC_FILES := $(c_bc_files) $(ll_bc_files)
$(LOCAL_BUILT_MODULE): $(c_bc_files) $(ll_bc_files)
$(LOCAL_BUILT_MODULE): $(RS_LLVM_LINK)
$(LOCAL_BUILT_MODULE): $(RS_LLVM_AS) $(BCC_STRIP_ATTR)
	@echo "bc lib: $(PRIVATE_MODULE) ($@)"
	@mkdir -p $(dir $@)
	# Strip useless known warning about combining mismatched modules, as well as
	# any blank lines that llvm-link inserts.
	$(hide) $(RELATIVE_PWD) $(RS_LLVM_LINK) $(PRIVATE_BC_FILES) -o $@.unstripped 2> >(grep -v "\(modules of different\)\|^$$" >&2)
	$(hide) $(RELATIVE_PWD) $(BCC_STRIP_ATTR) -o $@ $@.unstripped

BCC_RS_TRIPLE :=
