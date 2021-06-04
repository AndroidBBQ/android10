#! /bin/bash

export TARGET_PRODUCT="full"
export TARGET_BUILD_VARIANT="userdebug"
export TARGET_BUILD_DENSITY="alldpi"
export TARGET_BUILD_TYPE="release"
export TARGET_BUILD_APPS="setup-wizard-lib"

./gradlew buildProjectFull test coverage --info
