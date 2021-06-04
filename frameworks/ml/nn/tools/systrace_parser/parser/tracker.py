""" NNAPI Systrace parser - tracking of call tree based on trace lines

    See contract-between-code-and-parser.txt for the
    specification (cases in the specification are referred to with SPEC).
"""

import re
from parser.naming import (subphases, translate_hidl_mark_to_nn_and_tag,
                           get_function_name_from_mark, make_tag)
from parser.naming import LAYER_CPU, LAYER_DRIVER, LAYER_RUNTIME, LAYER_APPLICATION
from parser.naming import MARKER_SWITCH, MARKER_SUBTRACT
from parser.naming import PHASE_EXECUTION, PHASE_OVERALL, PHASE_WARMUP, PHASE_BENCHMARK
from parser.tree import SingleThreadCallTree

class AppPhase(object):
  """ Class to track the overall phase of the program. Used to split up warmup and benchmark.

      Needs to be separate from the call trees to propagate the difference to driver.
  """
  def __init__(self):
    self.reset()

  def current(self):
    if self.stack:
      return self.stack[-1]
    else:
      return PHASE_OVERALL

  def push(self, phase):
    self.stack.append(phase)

  def pop(self):
    self.stack.pop()

  def reset(self):
    self.stack = []

class Tracker(object):
  """ Class to track the stack trace of a single thread and feed it into a SingleThreadCallTree
      as well as keeping track of entry and exit times for functions.

      Exposes statistics for a single thread, transforming the call tree as needed.
      All statistics are in milliseconds.

      Layer Runtime, Phase Execution (LR_PE) is special-cased, see comment in get_stat().

      Subphases of Execution are aggregated towards the overall Execution phase as needed.
  """
  def __init__(self, tgid, is_driver, app_phase):
    self.tgid = tgid
    self.is_driver = is_driver
    self.app_phase = app_phase

    # Match the trace string
    # "[NN_LA_PP]funcE1" in "B|<thread1>|[NN_LA_PP]funcE1"
    # "[NN_LC_PCO]funcC1" in "B|<thread1>|[SW][NN_LC_PCO]funcC1"
    self.matcher = re.compile(r"B\|\d+\|.*\[([^]]+)\]\[?([^]])\]?")

    self.reset()

  def reset(self):
    self.stats = {}
    self.items = {}
    self.mytree = SingleThreadCallTree()
    self.begins_and_ends_ms = {}
    self.la_pe_counts = {}
    self.debugstring = "\n"

  def handle_mark(self, time, mark):
    """ Handle a single trace item (scoped entry and exit).
        Translates:
          - Automatically generated HIDL traces into NNTRACE layers and phases
          - SPEC:Switch phase during function into dummy items
          - SPEC:Subtracting time when nesting is violated into "subtract"
            markers
          - CPU/Driver layer distinction based on whether the process is the
            driver or an application
        This function is called multiple times for a single application run,
        afterwards the statistics can be calculated.
    """
    if mark[0] == "B":
      switch = False
      subtract = False
      # Workarounds for wrong tracepoints in early versions
      # TODO(mikie): remove later
      if ("ANeuralNetworksEvent_free" in mark) or ("ANeuralNetworksExecution_free" in mark):
        mark = mark.replace("_PT", "_PE")
      elif ("[SW][NN_LA_PR]executeWithCompilation" in mark):
        mark = mark.replace("[SW]", "")
      if MARKER_SWITCH in mark:
        switch = True
      if MARKER_SUBTRACT in mark:
        subtract = True
      if switch:
        # End previous item
        self.handle_mark(time, "E")
        # Push a dummy item that will get popped by the 'real' end of the
        # previous item.
        self.mytree.push_dummy(time)
      m = self.matcher.search(mark)
      if m is None:
        tag = translate_hidl_mark_to_nn_and_tag(mark)
        if tag is None:
          raise Exception("Couldn't parse mark " + mark)
      else:
        tag = m.group(1)
      [_, layer, phase] = tag.split("_")
      if layer == LAYER_APPLICATION and phase in [PHASE_WARMUP, PHASE_BENCHMARK]:
        self.app_phase.push(phase)
      if not self.is_driver:
        layer = layer.replace(LAYER_DRIVER, LAYER_CPU)
      else:
        layer = layer.replace(LAYER_CPU, LAYER_DRIVER)
      if layer == LAYER_APPLICATION and phase == PHASE_EXECUTION:
        self.la_pe_counts[self.app_phase.current()] = (
            self.la_pe_counts.get(self.app_phase.current(), 0) + 1)
      self.mytree.push(time, mark, layer, phase, self.app_phase.current(), subtract)
    elif mark[0] == "E":
      node = self.mytree.pop(time)
      if node.is_dummy():  # Dummy item
        pass
      else:
        if node.layer == LAYER_APPLICATION and node.phase in [PHASE_WARMUP, PHASE_BENCHMARK]:
          self.app_phase.pop()
        function = node.app_phase + "::" + get_function_name_from_mark(node.mark)
        self.begins_and_ends_ms[function] = (self.begins_and_ends_ms.get(function, []) +
                                             [[float(node.start_time_s) * 1000.0,
                                               float(node.end_time_s) * 1000.0]])

  def is_complete(self):
    """ Checks if we've seen all end tracepoints for the begin tracepoints.
    """
    return self.mytree.current.is_root()

  def calculate_stats(self):
    assert self.is_complete()
    self.mytree.remove_ignored()
    self.mytree.remove_dummies()
    self.mytree.copy_subtracted_init_and_wrong_la()
    self.mytree.add_missing_la_nodes()
    # self.mytree.print()
    self.mytree.validate_nesting()

    def recurse(node, prev_layer, prev_phase, indent, in_pe_layers):
      [begun, mark, layer, phase] = [
          node.start_time_s, node.mark, node.layer, node.phase()]
      time = node.end_time_s
      tag = None
      elapsed0 = "DETAIL"
      elapsed1 = node.elapsed_less_subtracted_ms()
      if elapsed1 is None:
        raise Exception("Elapsed for {} returned None".format(node.to_str()))

      if not node.is_added_detail() and not node.subtract:
        tag = node.app_phase + "_" + layer + "_" + phase
        elapsed0 = elapsed1
        self.stats[tag] = self.stats.get(tag, 0.0) + elapsed0
        self.items[tag] = self.items.get(tag, []) + [
            mark + " " + str(elapsed0) + " " + str(elapsed1) + " " +  tag]
        if phase in subphases[PHASE_EXECUTION]:
          if not in_pe_layers.get(layer):
            pe_tag = node.app_phase + "_" + make_tag(layer, PHASE_EXECUTION)
            self.stats[pe_tag] = self.stats.get(pe_tag, 0.0) + elapsed0
            self.items[pe_tag] = self.items.get(pe_tag, []) + [
                mark + " " + str(elapsed0) + " " + str(elapsed1) + " " +  pe_tag]
      if phase == PHASE_EXECUTION:
        in_pe_layers[layer] = in_pe_layers.get(layer, 0) + 1
      for c in node.children:
        recurse(c, layer or prev_layer, phase or prev_phase,
                indent + "  ", in_pe_layers)
      if phase == PHASE_EXECUTION:
        in_pe_layers[layer] = in_pe_layers[layer] - 1
      return

    for top in self.mytree.root.children:
      recurse(top, None, None, "", {})
    self.debugstring = self.mytree.to_str()

  # We need to special case the driver execution time because:
  # - The existing drivers don't have tracing, so we rely on HIDL traces
  # - Best we can do is to take the start of the HIDL server side call as
  #   the starting point (which includes a bit of overhead, but not much) and
  #   the start of the callback as the end point (which should be pretty
  #   accurate)
  # Note that the begin and end may be on different threads, hence the
  # calculation needs to happen in aggregation rather than here.
  def get_ld_pe_begins(self, app_phase):
      return self.get_begins(app_phase, "HIDL::IPreparedModel::execute::server")

  def get_ld_pe_ends(self, app_phase):
      return self.get_begins(app_phase, "HIDL::IExecutionCallback::notify::client")

  def get_stat(self, tag, app_phase, special_case_pe=True):
    if not self.stats and not self.mytree.is_empty():
      self.calculate_stats()
    if tag == make_tag(LAYER_RUNTIME, PHASE_EXECUTION) and special_case_pe:
      # Execution is exposed as an asynchronous event from the runtime, we
      # calculate the runtime time as starting from when the async operation is
      # kicked off until wait finishes + synchronous setup and teardown calls.
      # This has two limitations:
      #   - multithreaded usage will not work correctly
      #   - should the application spend so much time before calling wait that
      #     execution has already finished, the time would get allocated to the
      #     runtime incorrectly
      async_starts = self.get_begins(app_phase, "ANeuralNetworksExecution_startCompute")
      async_ends = self.get_ends(app_phase, "ANeuralNetworksEvent_wait")
      elapsed = 0.0
      for i in range(0, len(async_starts)):
        elapsed = elapsed + (async_ends[i] - async_starts[i])
      for sync in ["ANeuralNetworksExecution_create", "ANeuralNetworksExecution_free",
                   "ANeuralNetworksEvent_create", "ANeuralNetworksEvent_free",
                   "ANeuralNetworksExecution_setInput", "ANeuralNetworksExecution_setOutput",
                   "ANeuralNetworksExecution_setInputFromMemory",
                   "ANeuralNetworksExecution_setOutputFromMemory"]:
        sync_starts = self.get_begins(app_phase, sync)
        sync_ends = self.get_ends(app_phase, sync)
        for i in range(0, len(sync_starts)):
          elapsed = elapsed + (sync_ends[i] - sync_starts[i])
      return elapsed
    return self.stats.get(app_phase + "_" + tag, 0.0)

  def get_execution_count(self, app_phase):
    # ANeuralNetworksExecution_create is reliable and comes from the runtime,
    # but not available pre-P
    count = len(self.get_begins(app_phase, "ANeuralNetworksExecution_create"))
    if count > 0:
      return count
    # Application may have added tracepoints
    return self.la_pe_counts.get(app_phase, 0)

  def get_begins(self, app_phase, function):
    name = app_phase + "::" + function
    return [begin_and_end[0] for begin_and_end in self.begins_and_ends_ms.get(name, [])]
  def get_ends(self, app_phase, function):
    name = app_phase + "::" + function
    return [begin_and_end[1] for begin_and_end in self.begins_and_ends_ms.get(name, [])]

  def print_stats(self):
    if not self.stats:
      self.calculate_stats()
    print(self.tgid, "Driver" if self.is_driver else "App")
    for tag in self.stats:
      print(tag, self.stats[tag])
      if self.items.get(tag):
        for item in self.items[tag]:
          print("  ", item)
      else:
        print("  ", "calculated only")

  def print(self):
    self.mytree.print()
