#!/usr/bin/env bash

# Simple helper for ease of development until this API is frozen.

echo "LIBBINDER_NDK { # introduced=29"
echo "  global:"
{
    grep -oP "AIBinder_[a-zA-Z0-9_]+(?=\()" include_ndk/android/binder_ibinder.h;
    grep -oP "AIBinder_[a-zA-Z0-9_]+(?=\()" include_ndk/android/binder_ibinder_jni.h;
    grep -oP "AParcel_[a-zA-Z0-9_]+(?=\()" include_ndk/android/binder_parcel.h;
    grep -oP "AStatus_[a-zA-Z0-9_]+(?=\()" include_ndk/android/binder_status.h;
} | sort | uniq | awk '{ print "    " $0 ";"; }'
{
    grep -oP "AServiceManager_[a-zA-Z0-9_]+(?=\()" include_apex/android/binder_manager.h;
    grep -oP "ABinderProcess_[a-zA-Z0-9_]+(?=\()" include_apex/android/binder_process.h;
} | sort | uniq | awk '{ print "    " $0 "; # apex"; }'
echo "  local:"
echo "    *;"
echo "};"
