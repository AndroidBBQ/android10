#
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

#
# GDB plugin to allow debugging of apps on remote Android systems using gdbserver.
#
# To use this plugin, source this file from a Python-enabled GDB client, then use:
#   load-android-app <app-source-dir>  to tell GDB about the app you are debugging
#   run-android-app to start the app in a running state
#   start-android-app to start the app in a paused state
#   attach-android-ap to attach to an existing (running) instance of app
#   set-android-device to select a target (only if multiple devices are attached)

import fnmatch
import gdb
import os
import shutil
import subprocess
import tempfile
import time

be_verbose = False
enable_renderscript_dumps = True
local_symbols_library_directory = os.path.join(os.getenv('ANDROID_PRODUCT_OUT', 'out'),
      'symbols', 'system', 'lib')
local_library_directory = os.path.join(os.getenv('ANDROID_PRODUCT_OUT', 'out'),
      'system', 'lib')

# ADB              - Basic ADB wrapper, far from complete
# DebugAppInfo     - App configuration struct, as far as GDB cares
# StartAndroidApp  - Implementation of GDB start (for android apps)
# RunAndroidApp    - Implementation of GDB run (for android apps)
# AttachAndroidApp - GDB command to attach to an existing android app process
# AndroidStatus    - app status query command (not needed, mostly harmless)
# LoadAndroidApp   - Sets the package and intent names for an app

def _interesting_libs():
  return ['libc', 'libbcc', 'libRS', 'libandroid_runtime', 'libart']

# In python 2.6, subprocess.check_output does not exist, so it is implemented here
def check_output(*popenargs, **kwargs):
  p = subprocess.Popen(stdout=subprocess.PIPE, stderr=subprocess.STDOUT, *popenargs, **kwargs)
  out, err = p.communicate()
  retcode = p.poll()
  if retcode != 0:
    c = kwargs.get("args")
    if c is None:
      c = popenargs[0]
    e = subprocess.CalledProcessError(retcode, c)
    e.output = str(out) + str(err)
    raise e
  return out

class DebugAppInfo:
  """Stores information from an app manifest"""

  def __init__(self):
    self.name = None
    self.intent = None

  def get_name(self):
    return self.name

  def get_intent(self):
    return self.intent

  def get_data_directory(self):
    return self.data_directory

  def get_gdbserver_path(self):
    return os.path.join(self.data_directory, "lib", "gdbserver")

  def set_info(self, name, intent, data_directory):
    self.name = name
    self.intent = intent
    self.data_directory = data_directory

  def unset_info():
    self.name = None
    self.intent = None
    self.data_directory = None

class ADB:
  """
  Python class implementing a basic ADB wrapper for useful commands.
  Uses subprocess to invoke adb.
  """

  def __init__(self, device=None, verbose=False):
    self.verbose = verbose
    self.current_device = device
    self.temp_libdir = None
    self.background_processes = []
    self.android_build_top = os.getenv('ANDROID_BUILD_TOP', None)
    if not self.android_build_top:
      raise gdb.GdbError("Unable to read ANDROID_BUILD_TOP. " \
        + "Is your environment setup correct?")

    self.adb_path = os.path.join(self.android_build_top,
                      'out', 'host', 'linux-x86', 'bin', 'adb')

    if not self.current_device:
      devices = self.devices()
      if len(devices) == 1:
        self.set_current_device(devices[0])
        return
      else:
        msg = ""
        if len(devices) == 0:
          msg = "No devices detected. Please connect a device and "
        else:
          msg = "Too many devices (" + ", ".join(devices) + ") detected. " \
              + "Please "

        print "Warning: " + msg + " use the set-android-device command."


  def _prepare_adb_args(self, args):
    largs = list(args)

    # Prepare serial number option from current_device
    if self.current_device and len(self.current_device) > 0:
      largs.insert(0, self.current_device)
      largs.insert(0, "-s")

    largs.insert(0, self.adb_path)
    return largs


  def _background_adb(self, *args):
    largs = self._prepare_adb_args(args)
    p = None
    try:
      if self.verbose:
        print "### " + str(largs)
      p = subprocess.Popen(largs)
      self.background_processes.append(p)
    except CalledProcessError, e:
      raise gdb.GdbError("Error starting background adb " + str(largs))
    except:
      raise gdb.GdbError("Unknown error starting background adb " + str(largs))

    return p

  def _call_adb(self, *args):
    output = ""
    largs = self._prepare_adb_args(args)
    try:
      if self.verbose:
        print "### " + str(largs)
      output = check_output(largs)
    except subprocess.CalledProcessError, e:
      raise gdb.GdbError("Error starting adb " + str(largs))
    except Exception as e:
      raise gdb.GdbError("Unknown error starting adb " + str(largs))

    return output

  def _shell(self, *args):
    args = ["shell"] + list(args)
    return self._call_adb(*args)

  def _background_shell(self, *args):
    args = ["shell"] + list(args)
    return self._background_adb(*args)

  def _cleanup_background_processes(self):
    for handle in self.background_processes:
      try:
        handle.terminate()
      except OSError, e:
        # Background process died already
        pass

  def _cleanup_temp(self):
    if self.temp_libdir:
      shutil.rmtree(self.temp_libdir)
      self.temp_libdir = None

  def __del__(self):
    self._cleanup_temp()
    self._cleanup_background_processes()

  def _get_local_libs(self):
    ret = []
    for lib in _interesting_libs():
      lib_path = os.path.join(local_library_directory, lib + ".so")
      if not os.path.exists(lib_path) and self.verbose:
        print "Warning: unable to find expected library " \
          + lib_path + "."
      ret.append(lib_path)

    return ret

  def _check_remote_libs_match_local_libs(self):
    ret = []
    all_remote_libs = self._shell("ls", "/system/lib/*.so").split()
    local_libs = self._get_local_libs()

    self.temp_libdir = tempfile.mkdtemp()

    for lib in _interesting_libs():
      lib += ".so"
      for remote_lib in all_remote_libs:
        if lib in remote_lib:
          # Pull lib from device and compute hash
          tmp_path = os.path.join(self.temp_libdir, lib)
          self.pull(remote_lib, tmp_path)
          remote_hash = self._md5sum(tmp_path)

          # Find local lib and compute hash
          built_library = filter(lambda l: lib in l, local_libs)[0]
          built_hash = self._md5sum(built_library)

          # Alert user if library mismatch is detected
          if built_hash != remote_hash:
            self._cleanup_temp()
            raise gdb.GdbError("Library mismatch between:\n" \
              + "\t(" + remote_hash + ") " + tmp_path + " (from target) and\n " \
              + "\t(" + built_hash + ") " + built_library + " (on host)\n" \
              + "The target is running a different build than the host." \
              + " This situation is not debuggable.")

    self._cleanup_temp()

  def _md5sum(self, file):
    try:
      return check_output(["md5sum", file]).strip().split()[0]
    except subprocess.CalledProcessError, e:
      raise gdb.GdbError("Error invoking md5sum commandline utility")

  # Returns the list of serial numbers of connected devices
  def devices(self):
    ret = []
    raw_output = self._call_adb("devices").split()
    if len(raw_output) < 5:
      return None
    else:
      for serial_num_index in range(4, len(raw_output), 2):
        ret.append(raw_output[serial_num_index])
    return ret

  def set_current_device(self, serial):
    if self.current_device == str(serial):
      print "Current device already is: " + str(serial)
      return

    # TODO: this function should probably check the serial is valid.
    self.current_device = str(serial)

    api_version = self.getprop("ro.build.version.sdk")
    if api_version < 15:
      print "Warning: untested API version. Upgrade to 15 or higher"

    # Verify the local libraries loaded by GDB are identical to those
    # sitting on the device actually executing. Alert the user if
    # this is happening
    self._check_remote_libs_match_local_libs()

  # adb getprop [property]
  # if property is not None, returns the given property, otherwise
  # returns all properties.
  def getprop(self, property=None):
    if property == None:
      # get all the props
      return self._call_adb(*["shell", "getprop"]).split('\n')
    else:
      return str(self._call_adb(*["shell", "getprop",
        str(property)]).split('\n')[0])

  # adb push
  def push(self, source, destination):
    self._call_adb(*["push", source, destination])

  # adb forward <source> <destination>
  def forward(self, source, destination):
    self._call_adb(*["forward", source, destination])

  # Returns true if filename exists on Android fs, false otherwise
  def exists(self, filename):
    raw_listing = self._shell(*["ls", filename])
    return "No such file or directory" not in raw_listing

  # adb pull <remote_path> <local_path>
  def pull(self, remote_path, local_path):
    self._call_adb(*["pull", remote_path, local_path])

  #wrapper for adb shell ps. leave process_name=None for list of all processes
  #Otherwise, returns triple with process name, pid and owner,
  def get_process_info(self, process_name=None):
    ret = []
    raw_output = self._shell("ps")
    for raw_line in raw_output.splitlines()[1:]:
      line = raw_line.split()
      name = line[-1]

      if process_name == None or name == process_name:
        user = line[0]
        pid = line[1]

        if process_name != None:
          return (pid, user)
        else:
          ret.append((pid, user))

    # No match in target process
    if process_name != None:
      return (None, None)

    return ret

  def kill_by_pid(self, pid):
    self._shell(*["kill", "-9", pid])

  def kill_by_name(self, process_name):
    (pid, user) = self.get_process_info(process_name)
    while pid != None:
      self.kill_by_pid(pid)
      (pid, user) = self.get_process_info(process_name)

class AndroidStatus(gdb.Command):
  """Implements the android-status gdb command."""

  def __init__(self, adb, name="android-status", cat=gdb.COMMAND_OBSCURE, verbose=False):
    super (AndroidStatus, self).__init__(name, cat)
    self.verbose = verbose
    self.adb = adb

  def _update_status(self, process_name, gdbserver_process_name):
    self._check_app_is_loaded()

    # Update app status
    (self.pid, self.owner_user) = \
      self.adb.get_process_info(process_name)
    self.running = self.pid != None

    # Update gdbserver status
    (self.gdbserver_pid, self.gdbserver_user) = \
      self.adb.get_process_info(gdbserver_process_name)
    self.gdbserver_running = self.gdbserver_pid != None

    # Print results
    if self.verbose:
      print "--==Android GDB Plugin Status Update==--"
      print "\tinferior name: " + process_name
      print "\trunning: " + str(self.running)
      print "\tpid: " + str(self.pid)
      print "\tgdbserver running: " + str(self.gdbserver_running)
      print "\tgdbserver pid: " + str(self.gdbserver_pid)
      print "\tgdbserver user: " + str(self.gdbserver_user)

  def _check_app_is_loaded(self):
    if not currentAppInfo.get_name():
      raise gdb.GdbError("Error: no app loaded. Try load-android-app.")

  def invoke(self, arg, from_tty):
    self._check_app_is_loaded()
    self._update_status(currentAppInfo.get_name(),
      currentAppInfo.get_gdbserver_path())
    # TODO: maybe print something if verbose is off

class StartAndroidApp (AndroidStatus):
  """Implements the 'start-android-app' gdb command."""

  def _update_status(self):
    AndroidStatus._update_status(self, self.process_name, \
      self.gdbserver_path)

  # Calls adb shell ps every retry_delay seconds and returns
  # the pid when process_name show up in output, or return 0
  # after num_retries attempts. num_retries=0 means retry
  # indefinitely.
  def _wait_for_process(self, process_name, retry_delay=1, num_retries=10):
    """ This function is a hack and should not be required"""
    (pid, user) = self.adb.get_process_info(process_name)
    retries_left = num_retries
    while pid == None and retries_left != 0:
      (pid, user) = self.adb.get_process_info(process_name)
      time.sleep(retry_delay)
      retries_left -= 1

    return pid

  def _gdbcmd(self, cmd, from_tty=False):
    if self.verbose:
      print '### GDB Command: ' + str(cmd)

    gdb.execute(cmd, from_tty)

  # Remove scratch directory if any
  def _cleanup_temp(self):
    if self.temp_dir:
      shutil.rmtree(self.temp_dir)
      self.temp_dir = None

  def _cleanup_jdb(self):
    if self.jdb_handle:
      try:
        self.jdb_handle.terminate()
      except OSError, e:
        # JDB process has likely died
        pass

      self.jdb_handle = None

  def _load_local_libs(self):
    for lib in _interesting_libs():
      self._gdbcmd("shar " + lib)

  def __del__(self):
    self._cleanup_temp()
    self._cleanup_jdb()

  def __init__ (self, adb, name="start-android-app", cat=gdb.COMMAND_RUNNING, verbose=False):
    super (StartAndroidApp, self).__init__(adb, name, cat, verbose)
    self.adb = adb

    self.jdb_handle = None
    # TODO: handle possibility that port 8700 is in use (may help with
    # Eclipse problems)
    self.jdwp_port = 8700

    # Port for gdbserver
    self.gdbserver_port = 5039

    self.temp_dir = None

  def start_process(self, start_running=False):
    #TODO: implement libbcc cache removal if needed

    args = ["am", "start"]

    # If we are to start running, we can take advantage of am's -W flag to wait
    # for the process to start before returning. That way, we don't have to
    # emulate the behaviour (poorly) through the sleep-loop below.
    if not start_running:
      args.append("-D")
    else:
      args.append("-W")

    args.append(self.process_name + "/" + self.intent)
    am_output = self.adb._shell(*args)
    if "Error:" in am_output:
      raise gdb.GdbError("Cannot start app. Activity Manager returned:\n"\
        + am_output)

    # Gotta wait until the process starts if we can't use -W
    if not start_running:
      self.pid = self._wait_for_process(self.process_name)

    if not self.pid:
      raise gdb.GdbError("Unable to detect running app remotely." \
        + "Is " + self.process_name + " installed correctly?")

    if self.verbose:
      print "--==Android App Started: " + self.process_name \
        + " (pid=" + self.pid + ")==--"

    # Forward port for java debugger to Dalvik
    self.adb.forward("tcp:" + str(self.jdwp_port), \
                     "jdwp:" + str(self.pid))

  def start_gdbserver(self):
    # TODO: adjust for architecture...
    gdbserver_local_path = os.path.join(os.getenv('ANDROID_BUILD_TOP'),
      'prebuilt', 'android-arm', 'gdbserver', 'gdbserver')

    if not self.adb.exists(self.gdbserver_path):
      # Install gdbserver
      try:
        self.adb.push(gdbserver_local_path, self.gdbserver_path)
      except gdb.GdbError, e:
        print "Unable to push gdbserver to device. Try re-installing app."
        raise e

    self.adb._background_shell(*[self.gdbserver_path, "--attach",
      ":" + str(self.gdbserver_port), self.pid])

    self._wait_for_process(self.gdbserver_path)
    self._update_status()

    if self.verbose:
      print "--==Remote gdbserver Started " \
        + " (pid=" + str(self.gdbserver_pid) \
        + " port=" + str(self.gdbserver_port) + ") ==--"

    # Forward port for gdbserver
    self.adb.forward("tcp:" + str(self.gdbserver_port), \
                     "tcp:" + str(5039))

  def attach_gdb(self, from_tty):
    self._gdbcmd("target remote :" + str(self.gdbserver_port), False)
    if self.verbose:
      print "--==GDB Plugin requested attach (port=" \
        + str(self.gdbserver_port) + ")==-"

    # If GDB has no file set, things start breaking...so grab the same
    # binary the NDK grabs from the filesystem and continue
    self._cleanup_temp()
    self.temp_dir = tempfile.mkdtemp()
    self.gdb_inferior = os.path.join(self.temp_dir, 'app_process')
    self.adb.pull("/system/bin/app_process", self.gdb_inferior)
    self._gdbcmd('file ' + self.gdb_inferior)

  def start_jdb(self, port):
    # Kill if running
    self._cleanup_jdb()

    # Start the java debugger
    args = ["jdb", "-connect",
      "com.sun.jdi.SocketAttach:hostname=localhost,port=" + str(port)]
    if self.verbose:
      self.jdb_handle = subprocess.Popen(args, \
        stdin=subprocess.PIPE)
    else:
      # Unix-only bit here..
      self.jdb_handle = subprocess.Popen(args, \
        stdin=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        stdout=open('/dev/null', 'w'))

  def invoke (self, arg, from_tty):
    # TODO: self._check_app_is_installed()
    self._check_app_is_loaded()

    self.intent = currentAppInfo.get_intent()
    self.process_name = currentAppInfo.get_name()
    self.data_directory = currentAppInfo.get_data_directory()
    self.gdbserver_path = currentAppInfo.get_gdbserver_path()

    self._update_status()

    if self.gdbserver_running:
      self.adb.kill_by_name(self.gdbserver_path)
      if self.verbose:
        print "--==Killed gdbserver process (pid=" \
          + str(self.gdbserver_pid) + ")==--"
      self._update_status()

    if self.running:
      self.adb.kill_by_name(self.process_name)
      if self.verbose:
        print "--==Killed app process (pid=" + str(self.pid) + ")==--"
      self._update_status()

    self.start_process()

    # Start remote gdbserver
    self.start_gdbserver()

    # Attach the gdb
    self.attach_gdb(from_tty)

    # Load symbolic libraries
    self._load_local_libs()

    # Set the debug output directory (for JIT debugging)
    if enable_renderscript_dumps:
      self._gdbcmd('set gDebugDumpDirectory="' + self.data_directory + '"')

    # Start app
    # unblock the gdb by connecting with jdb
    self.start_jdb(self.jdwp_port)

class RunAndroidApp(StartAndroidApp):
  """Implements the run-android-app gdb command."""

  def __init__(self, adb, name="run-android-app", cat=gdb.COMMAND_RUNNING, verbose=False):
    super (RunAndroidApp, self).__init__(adb, name, cat, verbose)

  def invoke(self, arg, from_tty):
    StartAndroidApp.invoke(self, arg, from_tty)
    self._gdbcmd("continue")

class AttachAndroidApp(StartAndroidApp):
  """Implements the attach-android-app gdb command."""

  def __init__(self, adb, name="attach-android-app", cat=gdb.COMMAND_RUNNING, verbose=False):
    super (AttachAndroidApp, self).__init__(adb, name, cat, verbose)

  def invoke(self, arg, from_tty):
    # TODO: self._check_app_is_installed()
    self._check_app_is_loaded()

    self.intent = currentAppInfo.get_intent()
    self.process_name = currentAppInfo.get_name()
    self.data_directory = currentAppInfo.get_data_directory()
    self.gdbserver_path = currentAppInfo.get_gdbserver_path()

    self._update_status()

    if self.gdbserver_running:
      self.adb.kill_by_name(self.gdbserver_path)
      if self.verbose:
        print "--==Killed gdbserver process (pid=" \
          + str(self.gdbserver_pid) + ")==--"
      self._update_status()

    # Start remote gdbserver
    self.start_gdbserver()

    # Attach the gdb
    self.attach_gdb(from_tty)

    # Load symbolic libraries
    self._load_local_libs()

    # Set the debug output directory (for JIT debugging)
    if enable_renderscript_dumps:
      self._gdbcmd('set gDebugDumpDirectory="' + self.data_directory + '"')

class LoadApp(AndroidStatus):
  """ Implements the load-android-app gbd command.
  """
  def _awk_script_path(self, script_name):
    if os.path.exists(script_name):
      return script_name

    script_root = os.path.join(os.getenv('ANDROID_BUILD_TOP'), \
      'ndk', 'build', 'awk')

    path_in_root = os.path.join(script_root, script_name)
    if os.path.exists(path_in_root):
      return path_in_root

    raise gdb.GdbError("Unable to find awk script " \
      +  str(script_name) + " in " + path_in_root)

  def _awk(self, script, command):
    args = ["awk", "-f", self._awk_script_path(script), str(command)]

    if self.verbose:
      print "### awk command: " + str(args)

    awk_output = ""
    try:
      awk_output = check_output(args)
    except subprocess.CalledProcessError, e:
      raise gdb.GdbError("### Error in subprocess awk " + str(args))
    except:
      print "### Random error calling awk " + str(args)

    return awk_output.rstrip()

  def __init__(self, adb, name="load-android-app", cat=gdb.COMMAND_RUNNING, verbose=False):
    super (LoadApp, self).__init__(adb, name, cat, verbose)
    self.manifest_name = "AndroidManifest.xml"
    self.verbose = verbose
    self.adb = adb
    self.temp_libdir = None

  def _find_manifests(self, path):
    manifests = []
    for root, dirnames, filenames in os.walk(path):
      for filename in fnmatch.filter(filenames, self.manifest_name):
        manifests.append(os.path.join(root, filename))
    return manifests

  def _usage(self):
    return "Usage: load-android-app [<path-to-AndroidManifest.xml>" \
            + " | <package-name> <intent-name>]"

  def invoke(self, arg, from_tty):
 
    package_name = ''
    launchable = ''
    args = arg.strip('"').split()
    if len(args) == 2:
      package_name = args[0]
      launchable = args[1]
    elif len(args) == 1:
      if os.path.isfile(args[0]) and os.path.basename(args[0]) == self.manifest_name:
        self.manifest_path = args[0]
      elif os.path.isdir(args[0]):
        manifests = self._find_manifests(args[0])
        if len(manifests) == 0:
          raise gdb.GdbError(self.manifest_name + " not found in: " \
            + args[0] + "\n" + self._usage())
        elif len(manifests) > 1:
          raise gdb.GdbError("Ambiguous argument! Found too many " \
            + self.manifest_name + " files found:\n" + "\n".join(manifests))
        else:
          self.manifest_path = manifests[0]
      else:
        raise gdb.GdbError("Invalid path: " + args[0] + "\n" + self._usage())

      package_name = self._awk("extract-package-name.awk",
        self.manifest_path)
      launchable = self._awk("extract-launchable.awk",
        self.manifest_path)
    else:
      raise gdb.GdbError(self._usage())


    data_directory = self.adb._shell("run-as", package_name,
      "/system/bin/sh", "-c", "pwd").rstrip()

    if not data_directory \
      or len(data_directory) == 0 \
      or not self.adb.exists(data_directory):
      data_directory = os.path.join('/data', 'data', package_name)
      print "Warning: unable to read data directory for package " \
        + package_name + ". Meh, defaulting to " + data_directory

    currentAppInfo.set_info(package_name, launchable, data_directory)

    if self.verbose:
      print "--==Android App Loaded==--"
      print "\tname=" + currentAppInfo.get_name()
      print "\tintent=" + currentAppInfo.get_intent()

    # TODO: Check status of app on device

class SetAndroidDevice (gdb.Command):
  def __init__(self, adb, name="set-android-device", cat=gdb.COMMAND_RUNNING, verbose=False):
    super (SetAndroidDevice, self).__init__(name, cat)
    self.verbose = verbose
    self.adb = adb

  def _usage(self):
    return "Usage: set-android-device <serial>"

  def invoke(self, arg, from_tty):
    if not arg or len(arg) == 0:
      raise gdb.GdbError(self._usage)

    serial = str(arg)
    devices = adb.devices()
    if serial in devices:
      adb.set_current_device(serial)
    else:
      raise gdb.GdbError("Invalid serial. Serial numbers of connected " \
        + "device(s): \n" + "\n".join(devices))

# Global initialization
def initOnce(adb):
  # Try to speed up startup by skipping most android shared objects
  gdb.execute("set auto-solib-add 0", False);

  # Set shared object search path
  gdb.execute("set solib-search-path " + local_symbols_library_directory, False)

# Global instance of the object containing the info for current app
currentAppInfo = DebugAppInfo ()

# Global instance of ADB helper
adb = ADB(verbose=be_verbose)

# Perform global initialization
initOnce(adb)

# Command registration
StartAndroidApp (adb, "start-android-app", gdb.COMMAND_RUNNING, be_verbose)
RunAndroidApp (adb, "run-android-app", gdb.COMMAND_RUNNING, be_verbose)
AndroidStatus (adb, "android-status", gdb.COMMAND_OBSCURE, be_verbose)
LoadApp (adb, "load-android-app", gdb.COMMAND_RUNNING, be_verbose)
SetAndroidDevice (adb, "set-android-device", gdb.COMMAND_RUNNING, be_verbose)
AttachAndroidApp (adb, "attach-android-app", gdb.COMMAND_RUNNING, be_verbose)
