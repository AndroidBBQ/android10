""" NNAPI systrace parser - aggegation of timing from multiple threads """

# TODO:
# - phase and layer totals
import math

from parser.naming import layers, phases, subphases
from parser.naming import (PHASE_OVERALL, PHASE_TERMINATION, PHASE_WARMUP,
                           PHASE_BENCHMARK, PHASE_EXECUTION, PHASE_INITIALIZATION,
                           PHASE_INPUTS_AND_OUTPUTS, PHASE_RESULTS)
from parser.naming import LAYER_APPLICATION, LAYER_IPC, LAYER_DRIVER
LAYER_TOTAL = "LT"  # Total across layers

def aggregate_times(tracker_map, special_case_lr_pe=True):
  """ Takes the trackers for each thread and produces timing statistics for
      all layers and phases.

      Returns (times, self_times, has_warmup and has_benchmark, execution_counts),
      where:
        - times and self_times are nested dictionaries of the form
          phase -> layer -> time with the following notes:
          - phase is flattened over all phases, except PHASE_WARMUP and
            PHASE_BENCHMARK, where the structure is phase -> phase -> layer -> time
          - PHASE_WARMUP and PHASE_BENCHMARK only nest execution and its
            subphases
          - PHASE_WARMUP and PHASE_BENCHMARK are not present if the trace does
            not contain them
          - the first level phase contains total over PHASE_WARMUP and
            PHASE_BENCHMARK if present
          - time may be math.nan if the data is not present in the trace
          - in addition to the layer from parser.naming, LAYER_TOTAL holds
            the total time spent in that layer over all phases
        - execution_counts contains a dictionary of the form
          {PHASE_OVERALL, PHASE_WARMUP, PHASE_BENCHMARK} -> no of executions
  """
  all_application_phases = [PHASE_OVERALL, PHASE_WARMUP, PHASE_BENCHMARK]
  # Calculate execution counts
  execution_counts = dict()
  for app_phase in all_application_phases:
    execution_count = 0
    for pid in tracker_map:
      execution_count = max(execution_count, tracker_map[pid].get_execution_count(app_phase))
    execution_counts[app_phase] = execution_count
  has_warmup = bool(execution_counts[PHASE_WARMUP])
  has_benchmark = bool(execution_counts[PHASE_BENCHMARK])
  if not (has_warmup and has_benchmark):
    all_application_phases = [PHASE_OVERALL]

  # Create dicts
  times = {}
  self_times = {}
  if has_warmup and has_benchmark:
    for app_phase in [PHASE_WARMUP, PHASE_BENCHMARK]:
      times[app_phase] = {}
      self_times[app_phase] = {}
      for phase in _phase_and_subphases(PHASE_EXECUTION):
        times[app_phase][phase] = {}
        self_times[app_phase][phase] = {}
  for phase in phases + [PHASE_OVERALL] + subphases[PHASE_EXECUTION]:
    times[phase] = {}
    self_times[phase] = {}

  # Gather total times from all threads, calculate layer and phase totals
  for layer in layers:
    for phase0 in [PHASE_OVERALL] + phases:
      for phase in _phase_and_subphases(phase0):
        t = 0.0
        tag = layer + "_" + phase
        for app_phase in all_application_phases:
          t0 = 0.0
          if layer == LAYER_DRIVER and phase == PHASE_EXECUTION:
            # Calculate driver execution times from begins and ends
            begins = []
            ends = []
            for pid in tracker_map:
              begins = begins + tracker_map[pid].get_ld_pe_begins(app_phase)
              ends = ends + tracker_map[pid].get_ld_pe_ends(app_phase)
            assert len(begins) == len(ends)
            begins.sort()
            ends.sort()
            for i in range(0, len(begins)):
              t0 += (ends[i] - begins[i])
          else:
            for pid in tracker_map:
              t0 += tracker_map[pid].get_stat(tag, app_phase, special_case_lr_pe)
          if phase0 == PHASE_EXECUTION and (app_phase != PHASE_OVERALL):
            times[app_phase][phase][layer] = zero_to_nan_if_missing(t0, phase, layer)
          t += t0
        times[phase][layer] = zero_to_nan_if_missing(t, phase, layer)
    if not times[PHASE_OVERALL][layer]:
      times[PHASE_OVERALL][layer] = sum(nan_to_zero(times[phase][layer]) for phase in phases)
  for phase0 in [PHASE_OVERALL] + phases:
    for phase in _phase_and_subphases(phase0):
      times[phase][LAYER_TOTAL] = max_ignoring_nans(times[phase].values())
      if phase0 == PHASE_EXECUTION and (has_warmup and has_benchmark):
        for app_phase in [PHASE_WARMUP, PHASE_BENCHMARK]:
          times[app_phase][phase][LAYER_TOTAL] = max_ignoring_nans(times[app_phase][phase].values())

  # Calculate self-times for each layer
  for phase0 in [PHASE_OVERALL] + phases:
    for phase in _phase_and_subphases(phase0):
      self_times[phase][LAYER_TOTAL] = times[phase][LAYER_TOTAL]
      if phase0 == PHASE_EXECUTION and (has_warmup and has_benchmark):
        for app_phase in [PHASE_WARMUP, PHASE_BENCHMARK]:
          self_times[app_phase][phase][LAYER_TOTAL] = times[app_phase][phase][LAYER_TOTAL]
      t = 0.0
      for layer in reversed(layers):
        if math.isnan(times[phase][layer]):
          self_times[phase][layer] = math.nan
        elif times[phase][layer] == 0.0:
          self_times[phase][layer] = 0.0
        elif (phase == PHASE_OVERALL and
              (layer == LAYER_DRIVER or layer == LAYER_IPC) and
              times[PHASE_EXECUTION][LAYER_DRIVER] == 0.0):
          # Driver was only used for initialization phase, did not support
          # execution of the model
          if layer == LAYER_DRIVER:
            self_times[phase][layer] = times[phase][layer]
          else:
            self_times[phase][layer] = times[phase][layer] - times[phase][LAYER_DRIVER]
        else:
          self_times[phase][layer] = times[phase][layer] - t
          t = times[phase][layer]
      if phase0 == PHASE_EXECUTION and (has_benchmark or has_warmup):
        for app_phase in [PHASE_WARMUP, PHASE_BENCHMARK]:
          t = 0.0
          for layer in reversed(layers):
            if math.isnan(times[app_phase][phase][layer]):
              self_times[app_phase][phase][layer] = math.nan
            elif times[app_phase][phase][layer] == 0.0:
              self_times[app_phase][phase][layer] = 0.0
            else:
              self_times[app_phase][phase][layer] = times[app_phase][phase][layer] - t
              t = times[app_phase][phase][layer]

  return (times, self_times, has_warmup and has_benchmark, execution_counts)

def zero_to_nan_if_missing(f, phase, layer):
  """ Turn zero time to a NaN to indicate missing data, when we think that
      the data is really missing. Data should only be missing from the
      Application layer (applications may not have any tracing) and
      the subphases of Execution in the Driver layer (other phases are
      discernible from the automatic HIDL tracepoints)."""
  if f == 0.0:
    if layer == LAYER_APPLICATION:
      return math.nan
    if layer == LAYER_DRIVER and phase in subphases[PHASE_EXECUTION]:
      return math.nan
  return f

def nan_to_zero(f):
  if math.isnan(f):
    return 0.0
  return f

def _phase_and_subphases(phase):
  if phase == PHASE_OVERALL:
    return [phase]
  if phase == PHASE_WARMUP or phase == PHASE_BENCHMARK:
    return []
  return [phase] + subphases.get(phase, [])

def max_ignoring_nans(xs):
  return max(map(nan_to_zero, xs))
