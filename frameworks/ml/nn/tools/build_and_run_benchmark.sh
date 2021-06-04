#!/bin/bash
#
# Build benchmark app and run it, mimicking a user-initiated run
#
# Output is logged to a temporary folder and summarized in txt and JSON formats.
#
# Parameters
# - number of runs

NUMBER_RUNS=10

if [ ! -z $1 ]; then
  NUMBER_RUNS=$1
fi

if [[ -z "$ANDROID_BUILD_TOP" ]]; then
  echo ANDROID_BUILD_TOP not set, bailing out
  echo you must run lunch before running this script
  exit 1
fi

set -e
cd $ANDROID_BUILD_TOP

LOGDIR=$(mktemp -d)/nnapi-logs
mkdir -p $LOGDIR
echo Creating logs in $LOGDIR

adb -d root

# Skip setup wizard and remount (read-write)
if ! adb -d shell test -f /data/local.prop; then
  adb -d shell 'echo ro.setupwizard.mode=DISABLED > /data/local.prop'
  adb -d shell 'chmod 644 /data/local.prop'
  adb -d shell 'settings put global device_provisioned 1*'
  adb -d shell 'settings put secure user_setup_complete 1'
  adb -d disable-verity
  adb -d reboot
  sleep 5
  adb wait-for-usb-device remount
fi

# Build and install NNAPI runtime shared library
make libneuralnetworks
adb -d shell stop
adb -d remount
adb -d push $OUT/system/lib/libneuralnetworks.so /system/lib/libneuralnetworks.so
adb -d push $OUT/system/lib64/libneuralnetworks.so /system/lib64/libneuralnetworks.so
adb -d shell start

# Build and install benchmark app
make NeuralNetworksApiBenchmark
adb -d install $OUT/data/app/NeuralNetworksApiBenchmark/NeuralNetworksApiBenchmark.apk

# Enable menu key press through adb
adb -d shell 'echo testing > /data/local/enable_menu_key'
# Leave screen on (affects scheduling)
adb -d shell settings put system screen_off_timeout 86400000
# Stop background apps, seem to take ~10% CPU otherwise
set +e
adb -d shell 'pm disable com.google.android.googlequicksearchbox'
adb shell 'pm list packages -f' | sed -e 's/.*=//' | sed 's/\r//g' | grep "com.breel.wallpapers" | while read pkg; do adb -d shell "pm disable $pkg"; done;
set -e
adb -d shell setprop debug.nn.cpuonly 0
adb -d shell setprop debug.nn.vlog 0

echo running $NUMBER_RUNS times

# Run on CPU only
LOGFILE=$LOGDIR/perf-cpu.txt
echo "TFLite" | tee $LOGFILE
for i in $(seq 1 $NUMBER_RUNS); do
  echo "Run $i / $NUMBER_RUNS" | tee -a $LOGFILE
  # Menukey - make sure screen is on
  adb shell "input keyevent 82"
  # Show homescreen
  adb shell wm dismiss-keyguard
  # Set the shell pid as a top-app and run tests
  adb shell 'echo $$ > /dev/stune/top-app/tasks; am instrument -w -e size large -e class com.android.nn.benchmark.app.TFLiteTest com.android.nn.benchmark.app/androidx.test.runner.AndroidJUnitRunner' | tee -a $LOGFILE
  sleep 10  # let CPU cool down
done

echo "CPU run data from 'parse_benchmark.py $LOGFILE'"
$ANDROID_BUILD_TOP/frameworks/ml/nn/tools/parse_benchmark.py $LOGFILE

# Run with driver
LOGFILE=$LOGDIR/perf-driver.txt
echo "Driver" | tee $LOGFILE
for i in $(seq 1 $NUMBER_RUNS); do
  echo "Run $i / $NUMBER_RUNS" | tee -a $LOGFILE
  # Menukey - make sure screen is on
  adb shell "input keyevent 82"
  # Show homescreen
  adb shell wm dismiss-keyguard
  # Set the shell pid as a top-app and run tests
  adb shell 'echo $$ > /dev/stune/top-app/tasks; am instrument -w -e size large -e class com.android.nn.benchmark.app.NNTest com.android.nn.benchmark.app/androidx.test.runner.AndroidJUnitRunner' | tee -a $LOGFILE
done

echo "Driver run data from 'parse_benchmark.py $LOGFILE'"
$ANDROID_BUILD_TOP/frameworks/ml/nn/tools/parse_benchmark.py $LOGFILE
