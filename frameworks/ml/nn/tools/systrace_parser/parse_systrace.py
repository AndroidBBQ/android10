#!/usr/bin/python3

""" Parse a systrace file with NNAPI traces to calculate timing statistics

This is script to be run from the command line.

Usage:
  $ cd <location of script>
  $ ./parse_systrace.py <systrace html file>
  $ ./parse_systrace.py --help

For the parsing logic, see contract-between-code-and-parser.txt

"""

import argparse
import sys

from parser.input import get_trace_part, parse_trace_part
from parser.output import print_stats, reset_trackers
from parser.tracker import Tracker, AppPhase


def produce_stats(trace, print_detail=False, total_times=False, per_execution=False, json=False):
  """ Take a string with the systrace html file's trace part,
      possibly containing multiple application runs, feed the trace to
      Tracker objects and print stats for each run."""
  tracked_pids, driver_tgids, parsed = parse_trace_part(trace)
  tracker_map = {}
  app_phase = AppPhase()
  for pid in tracked_pids:
    tgid = tracked_pids[pid]
    tracker_map[pid] = Tracker(pid, driver_tgids.get(tgid, False), app_phase)

  first = True
  starting_mark = ''
  printed_one = False
  if json:
    sep = "["
  else:
    sep = ""
  for [task, pid, tgid, time, mark, line, lineno] in parsed:
    if ("HIDL::IDevice" in mark) or ("[NN_" in mark):
        assert tracker_map.get(pid)
    if tracker_map.get(pid):
        if "[NN_LA_PO]" in mark:
          # Next run
          if not first:
            if json and printed_one:
              sep = ","
            printed_one = True
            print_stats(tracker_map, print_detail, total_times, per_execution, json,
                        starting_mark, sep)
            reset_trackers(tracker_map)
            app_phase.reset()
          starting_mark = mark
          first = False
        try:
          tracker_map[pid].handle_mark(time, mark)
        except Exception as e:
          print("failed on line", lineno, line)
          raise
  if json and printed_one:
    sep = ","
  print_stats(tracker_map, print_detail, total_times, per_execution, json, starting_mark, sep)
  if json:
    print("]")

if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument('--print-detail', action='store_true')
  parser.add_argument('--total-times', action='store_true')
  parser.add_argument('--per-execution', action='store_true')
  parser.add_argument('--json', action='store_true')
  parser.add_argument('filename')
  args = parser.parse_args()
  trace = get_trace_part(args.filename)
  produce_stats(trace, args.print_detail, args.total_times, args.per_execution, args.json)
