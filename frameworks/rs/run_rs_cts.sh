#!/bin/bash -x

# Run the general RS CTS tests. We can expand this script to run more tests
# as we see fit, but this essentially should be a reasonable smoke test of
# RenderScript (to be run on build/test bots).

CTS_TRADEFED=$ANDROID_BUILD_TOP/out/host/linux-x86/bin/cts-tradefed
TMP_PATH=`mktemp -d`

#$CTS_TRADEFED run commandAndExit cts --force-abi 64 -p android.renderscript
#$CTS_TRADEFED run commandAndExit cts --force-abi 32 -p android.renderscript
$CTS_TRADEFED run commandAndExit cts --output-file-path $TMP_PATH -p android.renderscript
RESULT=$?

CTS_RESULTS=$ANDROID_BUILD_TOP/cts-results
RESULTS=$CTS_RESULTS/renderscript
mkdir -p $CTS_RESULTS
rm -rf $RESULTS
mkdir $RESULTS
find $TMP_PATH -name 'testResult.xml' -exec cp {} $RESULTS/ \;
rm -rf $TMP_PATH

exit $RESULT
