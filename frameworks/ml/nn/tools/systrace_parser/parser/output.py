from parser.aggregate import aggregate_times, nan_to_zero, LAYER_TOTAL
from parser.naming import layers, names, phases, make_tag, subphases
from parser.naming import LAYER_APPLICATION, LAYER_CPU
from parser.naming import (PHASE_INITIALIZATION, PHASE_PREPARATION, PHASE_COMPILATION,
                           PHASE_INPUTS_AND_OUTPUTS, PHASE_EXECUTION, PHASE_RESULTS,
                           PHASE_TERMINATION, PHASE_OVERALL, PHASE_WARMUP,
                           PHASE_BENCHMARK)
import json
import math
import sys

def print_stats(tracker_map, print_detail=True, total_times=False, per_execution=False,
                json_output=False, starting_mark='', sep=''):
  """ Prints statistics for a single Overall phase as text or json.

      For text output:
        By default prints the self-time for each layer, prints total times instead if
        given total_times=True.

        By default prints stats for all phases, prints only the Execution and its
        subphases (as per-execution times) if per_execution=True.

        If per_execution=True and the trace contains separate Warmup and Benchmark
        phases, prints only the Benchmark phase.

      For json output:
        The json output is internal to NNAPI and is not guaranteed stable or
        extensively defined. It does however contain a version field so that
        backwards-compativle tools can be created on top of it.

        The json output includes both the statistics themselves produced by
        aggregate_times as well as the values used to create the text output
        so that those are easily available.

        Look at the end of the function for the fields included in the json.
  """
  PHASE_EXECUTION_LESS_IO_AND_RESULTS = "PEO"
  phases_to_pick = phases + [PHASE_INPUTS_AND_OUTPUTS, PHASE_RESULTS]

  for tracker in tracker_map.values():
    if not tracker.is_complete():
      sys.stderr.write("Incomplete trace, not able to print all statistics\n")
      return
  if sep:
    print(sep)

  # Select template and statistics to use
  times, self_times, has_warmup_and_benchmark, execution_counts = aggregate_times(tracker_map)
  if not per_execution:
    template = TEMPLATE_ALL_PHASES
  else:
    template = TEMPLATE_EXECUTION_ONLY
  if total_times:
    template = template.replace("self-times", "total time")
    times_to_use = times
  else:
    times_to_use = self_times
  if has_warmup_and_benchmark and per_execution:
    template = template.replace("Execution", "Benchmark")
    for phase in [PHASE_EXECUTION] + subphases[PHASE_EXECUTION]:
      for layer in layers + [LAYER_TOTAL]:
        times_to_use[phase][layer] = times_to_use[PHASE_BENCHMARK][phase][layer]

  # Rewrite template shorthand
  template = template.replace(":fl", ":>11.2f")
  template = template.replace(":f", ":>9.2f")

  # Gather template inputs from statistics
  values = dict()
  full_total = 0.0
  has_cpu = False
  for layer in layers:
    for phase in phases_to_pick:
      t = times_to_use[phase][layer]
      values[make_tag(layer, phase)] = t
      if layer == LAYER_CPU:
        has_cpu = (has_cpu or t > 0.0)

  # Calculate layer totals and PHASE_EXECUTION_LESS_IO_AND_RESULTS
  for phase in phases_to_pick:
    values[make_tag(LAYER_TOTAL, phase)] = times_to_use[phase][LAYER_TOTAL]
  for layer in layers + [LAYER_TOTAL]:
    values[make_tag(layer, PHASE_EXECUTION_LESS_IO_AND_RESULTS)] = (
        values[make_tag(layer, PHASE_EXECUTION)] -
        values[make_tag(layer, PHASE_INPUTS_AND_OUTPUTS)] -
        values[make_tag(layer, PHASE_RESULTS)])
    values[make_tag(layer, PHASE_OVERALL)] = times_to_use[PHASE_OVERALL][layer]
  # Calculate layer execution percentages
  for layer in layers:
    if values[make_tag(LAYER_TOTAL, PHASE_EXECUTION)] > 0.0:
      values[make_tag(layer, "PEp")] = (values[make_tag(layer, PHASE_EXECUTION)] * 100.0 /
                                        values[make_tag(LAYER_TOTAL, PHASE_EXECUTION)])
    else:
      values[make_tag(layer, "PEp")] = math.nan

  # Make output numbers per-execution if desired
  if per_execution:
    if has_warmup_and_benchmark:
      divide_by = execution_counts[PHASE_BENCHMARK]
    else:
      divide_by = execution_counts[PHASE_OVERALL]
    for layer in (layers + [LAYER_TOTAL]):
      for phase in [PHASE_INPUTS_AND_OUTPUTS, PHASE_EXECUTION_LESS_IO_AND_RESULTS, PHASE_RESULTS, PHASE_EXECUTION]:
        if divide_by != 0:
          values[layer + "_" + phase] = values[layer + "_" + phase] / divide_by
        else:
          values[layer + "_" + phase] = math.nan

  # Generate and print output
  if not json_output:
    # Apply template and prettify numbers
    output = template.format(**values)
    output = output.replace(" 0.00%", "     -")
    output = output.replace(" 0.00", "    -")
    output = output.replace(" nan", " n/a")

    # Print output
    print(starting_mark)
    for line in output.splitlines():
      if line[0:3] == "CPU" and not has_cpu:
        continue
      print(line)
    if print_detail:
      for pid in tracker_map:
        tracker = tracker_map[pid]
        tracker.print_stats()
      for pid in tracker_map:
        tracker = tracker_map[pid]
        tracker.print()
  else:
    output = dict(times=times, self_times=self_times, execution_counts=execution_counts,
                  template_inputs=values, version=1, starting_mark=starting_mark)
    output = json.dumps(output, indent=2, sort_keys=True)
    # JSON doesn't recognize NaN
    output = output.replace("NaN", "null")
    print(output)

def reset_trackers(tracker_map):
  for pid in tracker_map:
    tracker = tracker_map[pid]
    tracker.reset()

TEMPLATE_ALL_PHASES = """
===========================================================================================================================================
NNAPI timing summary (self-times, ms wall-clock)                                                      Execution
                                                           ----------------------------------------------------
              Initialization   Preparation   Compilation           I/O       Compute      Results     Ex. total   Termination        Total
              --------------   -----------   -----------   -----------  ------------  -----------   -----------   -----------   ----------
Application        {LA_PI:f}     {LA_PP:f}     {LA_PC:f}   {LA_PIO:fl}   {LA_PEO:fl}    {LA_PR:f}     {LA_PE:f}     {LA_PT:f}    {LA_PO:f}*
Runtime            {LR_PI:f}     {LR_PP:f}     {LR_PC:f}   {LR_PIO:fl}   {LR_PEO:fl}    {LR_PR:f}     {LR_PE:f}     {LR_PT:f}    {LR_PO:f}
IPC                {LI_PI:f}     {LI_PP:f}     {LI_PC:f}   {LI_PIO:fl}   {LI_PEO:fl}    {LI_PR:f}     {LI_PE:f}     {LI_PT:f}    {LI_PO:f}
Driver             {LD_PI:f}     {LD_PP:f}     {LD_PC:f}   {LD_PIO:fl}   {LD_PEO:fl}    {LD_PR:f}     {LD_PE:f}     {LD_PT:f}    {LD_PO:f}
CPU                {LC_PI:f}     {LC_PP:f}     {LC_PC:f}   {LC_PIO:fl}   {LC_PEO:fl}    {LC_PR:f}     {LC_PE:f}     {LC_PT:f}    {LC_PO:f}

Total              {LT_PI:f}*    {LT_PP:f}*    {LT_PC:f}*  {LT_PIO:fl}*  {LT_PEO:fl}*   {LT_PR:f}*    {LT_PE:f}*    {LT_PT:f}*   {LT_PO:f}*
===========================================================================================================================================
* This total ignores missing (n/a) values and thus is not necessarily consistent with the rest of the numbers
"""

TEMPLATE_EXECUTION_ONLY = """
================================================================================
NNAPI timing summary (self-times, ms wall-clock)                       Execution
              ------------------------------------------------------------------
                      I/O       Compute      Results         Total    Percentage
              -----------  ------------  -----------   -----------   -----------
Application   {LA_PIO:fl}   {LA_PEO:fl}    {LA_PR:f}     {LA_PE:f}  {LA_PEp:fl}%
Runtime       {LR_PIO:fl}   {LR_PEO:fl}    {LR_PR:f}     {LR_PE:f}  {LR_PEp:fl}%
IPC           {LI_PIO:fl}   {LI_PEO:fl}    {LI_PR:f}     {LI_PE:f}  {LI_PEp:fl}%
Driver        {LD_PIO:fl}   {LD_PEO:fl}    {LD_PR:f}     {LD_PE:f}  {LD_PEp:fl}%
CPU           {LC_PIO:fl}   {LC_PEO:fl}    {LC_PR:f}     {LC_PE:f}  {LC_PEp:fl}%

Total         {LT_PIO:fl}*  {LT_PEO:fl}*   {LT_PR:f}*    {LT_PE:f}          100%
================================================================================
* This total ignores missing (n/a) values and thus is not necessarily consistent
  with the rest of the numbers
"""
