#!/bin/bash

source $ANDROID_BUILD_TOP/system/tools/hidl/update-makefiles-helper.sh

do_makefiles_update \
    "android.frameworks:frameworks/hardware/interfaces" \
    "android.hardware:hardware/interfaces" \
    "android.hidl:system/libhidl/transport"

