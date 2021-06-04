# `dumpstate` development tips

## To build `dumpstate`

Do a full build first:

```
m -j dumpstate
```

Then incremental ones:

```
mmm -j frameworks/native/cmds/dumpstate
```

If you're working on device-specific code, you might need to build them as well.
Example:

```
mmm -j frameworks/native/cmds/dumpstate device/acme/secret_device/dumpstate/ hardware/interfaces/dumpstate
```

## To build, deploy, and take a bugreport

```
mmm -j frameworks/native/cmds/dumpstate && adb push ${OUT}/system/bin/dumpstate system/bin && adb push ${OUT}/system/lib64/*dumpstate*.so /system/lib64/ && adb shell am bug-report
```

Make sure that the device is remounted before running the above command. * If
you're working with `userdebug` variant, you may need to run the following to
remount your device:

```
  adb root && adb remount -R && adb wait-for-device && adb root && adb remount
```

*   If you're working with `eng` variant, you may need to run the following to
    remount your device:

    ```
    adb root && adb remount
    ```

## To build, deploy, and run unit tests

First create `/data/nativetest64`:

```
adb shell mkdir /data/nativetest64
```

Then run:

```
mmm -j frameworks/native/cmds/dumpstate/ && adb push ${OUT}/data/nativetest64/dumpstate_* /data/nativetest64 && adb shell /data/nativetest64/dumpstate_test/dumpstate_test
```

And to run just one test (for example, `DumpstateTest.RunCommandNoArgs`):

```
mmm -j frameworks/native/cmds/dumpstate/ && adb push ${OUT}/data/nativetest64/dumpstate_test* /data/nativetest64 && adb shell /data/nativetest64/dumpstate_test/dumpstate_test --gtest_filter=DumpstateTest.RunCommandNoArgs
```

## To take quick bugreports

```
adb shell setprop dumpstate.dry_run true
```

## To emulate a device with user build

```
adb shell setprop dumpstate.unroot true
```

## To change the `dumpstate` version

```
adb shell setprop dumpstate.version VERSION_NAME
```

Example:

```
adb shell setprop dumpstate.version split-dumpsys && adb shell dumpstate -v
```

Then to restore the default version:

```
adb shell setprop dumpstate.version default
```

## Code style and formatting

Use the style defined at the
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) and
make sure to run the following command prior to `repo upload`:

```
git clang-format --style=file HEAD~
```

## Useful Bash tricks

```
export BR_DIR=/bugreports

alias br='adb shell cmd activity bug-report'
alias ls_bugs='adb shell ls -l ${BR_DIR}/'

unzip_bug() {
  adb pull ${BR_DIR}/$1 && emacs $1 && mv $1 /tmp
}

less_bug() {
  adb pull ${BR_DIR}/$1 && less $1 && mv $1 /tmp
}

rm_bugs() {
 if [ -z "${BR_DIR}" ] ; then echo "Variable BR_DIR not set"; else adb shell rm -rf ${BR_DIR}/*; fi
}

```
