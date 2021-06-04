""" NNAPI systrace parser - getting data in from a systrace html output """

import re
import sys

def get_trace_part(filename):
  """ Finds the text trace in the given html file, returns as a string. """
  with open(filename) as f:
    lines = f.readlines()
  seen_begin = False
  trace = []
  lineno = 0
  for line in lines:
    lineno = lineno + 1
    if ("#           TASK-PID    TGID   CPU#  ||||    TIMESTAMP  FUNCTION" in line or
        "#           TASK-PID   CPU#  ||||    TIMESTAMP  FUNCTION" in line):
      seen_begin = True
    if seen_begin:
      if "</script>" in line:
        break
      trace.append([line, lineno])
  return trace

MATCHER = re.compile(r"^\s*([^ ].{1,15})-(\d+)\s+\(\s*([-0-9]+)\) .* (\d+\.\d+): tracing_mark_write: ([BE].*)$")
MATCHER_FOR_OLD = re.compile(r"^\s*([^ ].{1,15})-(\d+) .* (\d+\.\d+): tracing_mark_write: ([BE].*)$")

def parse_trace_part(trace):
  """ Takes a string containing the text trace form systrace, parses the rows
      and selects which threads we are interested in.

      Returns (tracked_pids, driver_tgids, parsed), where:
         - tracked_pids: map from pids we are interested in to their tgids
         - driver_tgids: map that contains tgids of NNAPI driver processes
         - parsed: list of parsed rows, each containing
              TASK, PID, TGID, TIMESTAMP and FUNCTION
           as shown below
  """
  # Row format
  #  #           TASK-PID    TGID   CPU#  ||||    TIMESTAMP  FUNCTION" in line:
  #  #              | |        |      |   ||||       |         |
  #    NeuralNetworks-5143  ( 5143) [005] ...1   142.924145: tracing_mark_write: B|5143|[NN_L
  #  <...>-5149 ( 774) [000] ...1   143.103773: tracing_mark_write: B|774|[NN_LDV_PC][validat
  #  <...>-756 ( 756) [000] ...1   143.140553: tracing_mark_write: B|756|HIDL::IDevice::prepa
  #  <...>-5149  (-----) [001] ...1   143.149856: tracing_mark_write: B|756|[NN_LCC_PE][optim
  #    HwBinder:784_1-5236  (  784) [001] ...1   397.528915: tracing_mark_write: B|784|HIDL::
  #    GLThread 35-1739  ( 1500) [001] ...1   277.001798: tracing_mark_write: B|1500|HIDL::IMapper::importBuffer::passthrough
  # Notes:
  #    - systrace enter/exit marks are per PID, which is really a thread id on Linux
  #    - TGIDs identify processes
  #
  mark_matcher = re.compile(r"([BE])\|(\d+).*")
  tracked_pids = {}
  driver_tgids = {}
  pid_to_tgid = {}
  parsed = []
  seen_nnapi_runtime = False
  for [line, lineno] in trace:
    m = MATCHER.match(line)
    m_old = MATCHER_FOR_OLD.match(line)
    if not m and not m_old:
      # Check parsing doesn't discard interesting lines
      assert not "HIDL::IDevice" in line, line
      assert not "[NN_" in line, line
      assert not "tracing_mark_write: B" in line, line
      assert not "tracing_mark_write: E" in line, line
      continue
    if m:
      [task, pid, tgid, time, mark] = m.groups()
    else:
      [task, pid, time, mark] = m_old.groups()
      if "|" in mark:
        tgid = mark.split("|")[1]
      else:
        tgid = pid_to_tgid[pid]
    assert pid
    pid_to_tgid[pid] = tgid
    if tgid == "-----":
      mm = mark_matcher.match(mark)
      tgid = mm.group(2)
      assert tgid
    parsed.append( [task, pid, tgid, time, mark, line, lineno] )
    if "[NN" in mark:
      tracked_pids[pid] = tgid
      if "NN_LR" in mark:
        seen_nnapi_runtime = True
    if "HIDL::IDevice" in mark and "::server" in mark:
      tracked_pids[pid] = tgid
      driver_tgids[tgid] = True

  if not seen_nnapi_runtime:
    sys.stderr.write("\n*** No NNAPI Runtime trace present - check your systrace setup ***\n\n")
  return tracked_pids, driver_tgids, parsed

