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

# This script will run as a postinstall step to drive otapreopt.

TARGET_SLOT="$1"
STATUS_FD="$2"

# Maximum number of packages/steps.
MAXIMUM_PACKAGES=1000

# First ensure the system is booted. This is to work around issues when cmd would
# infinitely loop trying to get a service manager (which will never come up in that
# mode). b/30797145
BOOT_PROPERTY_NAME="dev.bootcomplete"

BOOT_COMPLETE=$(getprop $BOOT_PROPERTY_NAME)
if [ "$BOOT_COMPLETE" != "1" ] ; then
  echo "Error: boot-complete not detected."
  # We must return 0 to not block sideload.
  exit 0
fi


# Compute target slot suffix.
# TODO: Once bootctl is not restricted, we should query from there. Or get this from
#       update_engine as a parameter.
if [ "$TARGET_SLOT" = "0" ] ; then
  TARGET_SLOT_SUFFIX="_a"
elif [ "$TARGET_SLOT" = "1" ] ; then
  TARGET_SLOT_SUFFIX="_b"
else
  echo "Unknown target slot $TARGET_SLOT"
  exit 1
fi


PREPARE=$(cmd otadexopt prepare)
# Note: Ignore preparation failures. Step and done will fail and exit this.
#       This is necessary to support suspends - the OTA service will keep
#       the state around for us.

PROGRESS=$(cmd otadexopt progress)
print -u${STATUS_FD} "global_progress $PROGRESS"

i=0
while ((i<MAXIMUM_PACKAGES)) ; do
  DEXOPT_PARAMS=$(cmd otadexopt next)

  /system/bin/otapreopt_chroot $STATUS_FD $TARGET_SLOT_SUFFIX $DEXOPT_PARAMS >&- 2>&-

  PROGRESS=$(cmd otadexopt progress)
  print -u${STATUS_FD} "global_progress $PROGRESS"

  DONE=$(cmd otadexopt done)
  if [ "$DONE" = "OTA incomplete." ] ; then
    sleep 1
    i=$((i+1))
    continue
  fi
  break
done

DONE=$(cmd otadexopt done)
if [ "$DONE" = "OTA incomplete." ] ; then
  echo "Incomplete."
else
  echo "Complete or error."
fi

print -u${STATUS_FD} "global_progress 1.0"
cmd otadexopt cleanup

exit 0
