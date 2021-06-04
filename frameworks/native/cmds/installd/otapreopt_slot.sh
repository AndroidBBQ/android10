#!/system/bin/sh

#
# Copyright (C) 2016 The Android Open Source Project
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

# This script will move artifacts for the currently active slot.

SLOT_SUFFIX=$(getprop ro.boot.slot_suffix)
if test -n "$SLOT_SUFFIX" ; then
  if test -d /data/ota/$SLOT_SUFFIX/dalvik-cache ; then
    log -p i -t otapreopt_slot "Moving A/B artifacts for slot ${SLOT_SUFFIX}."
    OLD_SIZE=$(du -h -s /data/dalvik-cache)
    rm -rf /data/dalvik-cache/*
    NEW_SIZE=$(du -h -s /data/ota/$SLOT_SUFFIX/dalvik-cache)
    mv /data/ota/$SLOT_SUFFIX/dalvik-cache/* /data/dalvik-cache/
    rmdir /data/ota/$SLOT_SUFFIX/dalvik-cache
    rmdir /data/ota/$SLOT_SUFFIX
    log -p i -t otapreopt_slot "Moved ${NEW_SIZE} over ${OLD_SIZE}"
  else
    log -p i -t otapreopt_slot "No A/B artifacts found for slot ${SLOT_SUFFIX}."
  fi
  exit 0
else
  log -p w -t otapreopt_slot "Slot property empty."
  exit 1
fi
