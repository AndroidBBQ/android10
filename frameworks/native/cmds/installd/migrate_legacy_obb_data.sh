#!/system/bin/sh

#
# Copyright (C) 2019 The Android Open Source Project
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

rm -rf /sdcard/Android/obb/test_probe
mkdir -p /sdcard/Android/obb/
touch /sdcard/Android/obb/test_probe
if ! test -f /data/media/0/Android/obb/test_probe ; then
  log -p i -t migrate_legacy_obb_data "No support for 'unshared_obb'. Not migrating"
  rm -rf /sdcard/Android/obb/test_probe
  exit 0
fi

# Delete the test file, and remove the obb folder if it is empty
rm -rf /sdcard/Android/obb/test_probe
rmdir /data/media/obb

if ! test -d /data/media/obb ; then
  log -p i -t migrate_legacy_obb_data "No legacy obb data to migrate."
  exit 0
fi

log -p i -t migrate_legacy_obb_data "Migrating legacy obb data."
rm -rf /data/media/0/Android/obb
cp -F -p -R -P -d /data/media/obb /data/media/0/Android
rm -rf /data/media/obb
log -p i -t migrate_legacy_obb_data "Done."
