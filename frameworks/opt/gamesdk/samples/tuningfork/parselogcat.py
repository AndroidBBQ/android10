# This script parses the logcat lines produced by the Tuning Fork DebugBackend
#  which are base64 encoded serializations of TuningForkLogEvent protos.
# Usage:
#  adb logcat -d | python parselogcat.py

import sys
import re

# To generate python files from the proto files:
# export TF_PROTO_DIR=../../src/tuningfork/proto/
# protoc --python_out=. -I$TF_PROTO_DIR $TF_PROTO_DIR/tuningfork.proto
# protoc --python_out=. -I$TF_PROTO_DIR $TF_PROTO_DIR/tuningfork_clearcut_log.proto
# export TF_DEV_PROTO_DIR=<somewhere>
# protoc --python_out=. -I$TF_DEV_PROTO_DIR $TF_DEV_PROTO_DIR/dev_tuningfork.proto

# 'pip install protobuf' if you get a 'No module named protobuf' error

import tuningfork_clearcut_log_pb2 as tcl
import dev_tuningfork_pb2 as tf

# Example logcat line:
#11-30 15:32:22.892 13781 16553 I TuningFork.Clearcut: (TCL1/1)GgAqHAgAEgAaFgAAAAAAAAAAAAAAAAAAAAAAAAAAAEg=
tflogcat_regex = r"(\S+ \S+).*TuningFork.*\(TCL(.+)/(.+)\)(.*)"

def flatten(s):
  return ', '.join(s.strip().split('\n'))

def prettyPrint(tclevent):
  fp =tf.FidelityParams()
  fp.ParseFromString(tclevent.fidelityparams)
  print "fidelityparams: ", flatten(str(fp))
  for h in tclevent.histograms:
    print "histograms {"
    print "  instrument_id: ", h.instrument_id
    a = tf.Annotation()
    a.ParseFromString(h.annotation)
    print "  annotation: ", flatten(str(a))
    for c in h.counts:
      print "  counts: ", c
    print "}"

ser = ""
def getTCLEvent(i, n, ser_in):
  global ser
  if i==1:
    ser = ""
  ser += ser_in
  if i<>n:
    return
  l = tcl.TuningForkLogEvent()
  l.ParseFromString(ser.decode("base64"))
  return l

def readStdin():
  for logcat_lines in sys.stdin.readlines():
    m = re.match(tflogcat_regex, logcat_lines)
    if m:
      subparts = m.groups()
      tstamp = subparts[0]
      tclevent = getTCLEvent(int(subparts[1]),int(subparts[2]),subparts[3])
      if tclevent:
        prettyPrint(tclevent)

def main():
  readStdin()

if __name__ == "__main__":
  main()
