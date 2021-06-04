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

# If you don't need to do a full clean build but would like to touch
# a file or delete some intermediate files, add a clean step to the end
# of the list.  These steps will only be run once, if they haven't been
# run before.
#
# E.g.:
#     $(call add-clean-step, touch -c external/sqlite/sqlite3.h)
#     $(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/STATIC_LIBRARIES/libz_intermediates)
#
# Always use "touch -c" and "rm -f" or "rm -rf" to gracefully deal with
# files that are missing or have been moved.
#
# Use $(PRODUCT_OUT) to get to the "out/target/product/blah/" directory.
# Use $(OUT_DIR) to refer to the "out" directory.
#
# If you need to re-do something that's already mentioned, just copy
# the command and add it to the bottom of the list.  E.g., if a change
# that you made last week required touching a file and a change you
# made today requires touching the same file, just copy the old
# touch step and add it to the end of the list.
#
# ************************************************
# NEWER CLEAN STEPS MUST BE AT THE END OF THE LIST
# ************************************************

# For example:
#$(call add-clean-step, rm -rf $(OUT_DIR)/target/common/obj/APPS/AndroidTests_intermediates)
#$(call add-clean-step, rm -rf $(OUT_DIR)/target/common/obj/JAVA_LIBRARIES/core_intermediates)
#$(call add-clean-step, find $(OUT_DIR) -type f -name "IGTalkSession*" -print0 | xargs -0 rm -f)
#$(call add-clean-step, rm -rf $(PRODUCT_OUT)/data/*)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/SHARED_LIBRARIES/libmedia_native_intermediates)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/lib/libmedia_native.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/symbols/system/lib/libmedia_native.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/libmedia_native.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/SHARED_LIBRARIES/libaudioflinger_intermediates)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/libaudioflinger.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/SHARED_LIBRARIES/libaudiopolicy_intermediates)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/libaudiopolicy.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/SHARED_LIBRARIES/libaudiopolicy_intermediates)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/libaudiopolicy.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/SHARED_LIBRARIES/libaudiopolicyservice_intermediates)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/SHARED_LIBRARIES/libaudiopolicymanager_intermediates)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/libaudiopolicyservice.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/libaudiopolicymanager.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/SHARED_LIBRARIES/libaudiopolicyservice_intermediates)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/SHARED_LIBRARIES/libaudiopolicymanager_intermediates)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/liboboe.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib64/liboboe.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/STATIC_LIBRARIES/liboboe*)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/SHARED_LIBRARIES/liboboe*)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj_arm/STATIC_LIBRARIES/liboboe*)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj_arm/SHARED_LIBRARIES/liboboe*)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/bin/mediacodec)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/etc/init/mediacodec.rc)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/libeffects.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib64/libeffects.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/obj/SHARED_LIBRARIES/libeffects_intermediates)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/soundfx/libeffectproxy.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/soundfx/libldnhncr.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/soundfx/libdownmix.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/soundfx/libvisualizer.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/soundfx/libreverbwrapper.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/soundfx/libbundlewrapper.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/soundfx/libaudiopreprocessing.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/libmediacodecservice.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/libstagefright_xmlparser@1.0.so)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/libstagefright_soft_*)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/vndk/libstagefright_soft_*)
$(call add-clean-step, rm -rf $(PRODUCT_OUT)/system/lib/libaudiopolicyengineconfig*)

# ************************************************
# NEWER CLEAN STEPS MUST BE AT THE END OF THE LIST
# ************************************************
