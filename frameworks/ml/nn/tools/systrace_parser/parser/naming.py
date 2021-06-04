""" NNAPI systrace parser: naming conventions and translations """


# Application phases
PHASE_OVERALL = "PO"      # Overall program, e.g., one benchmark case
PHASE_WARMUP = "PWU"      # Executions done for warmup
PHASE_BENCHMARK = "PBM"   # Executions done to benchmark after warmup
# Main phases
PHASE_INITIALIZATION = "PI" # Initialization - not related to a model
PHASE_PREPARATION = "PP"  # Model construction
PHASE_COMPILATION = "PC"  # Model compilation
PHASE_EXECUTION = "PE"    # Executing the model
PHASE_TERMINATION = "PT"  # Tearing down
PHASE_UNSPECIFIED = "PU"  # Helper code called from multiple phases
# Subphases of execution
PHASE_INPUTS_AND_OUTPUTS = "PIO"  # Setting inputs/outputs and allocating buffers
PHASE_TRANSFORMATION = "PTR"      # Transforming data for computation
PHASE_COMPUTATION = "PCO"         # Computing operations' outputs
PHASE_RESULTS = "PR"              # Reading out results
# Layers
LAYER_APPLICATION = "LA"
LAYER_RUNTIME = "LR"
LAYER_IPC = "LI"
LAYER_DRIVER = "LD"
LAYER_CPU = "LC"
LAYER_OTHER = "LO"
LAYER_UTILITY = "LU"              # Code used from multiple layers
LAYER_IGNORE = "LX"               # Don't count time
# Markers
MARKER_SWITCH = "[SW]"
MARKER_SUBTRACT = "[SUB]"

layers = [LAYER_APPLICATION, LAYER_RUNTIME, LAYER_IPC, LAYER_DRIVER, LAYER_CPU]
phases = [PHASE_INITIALIZATION, PHASE_PREPARATION, PHASE_COMPILATION,
          PHASE_EXECUTION, PHASE_TERMINATION]
subphases = dict(PE=[PHASE_INPUTS_AND_OUTPUTS, PHASE_TRANSFORMATION,
                     PHASE_COMPUTATION, PHASE_RESULTS],
                 PO=([PHASE_WARMUP, PHASE_BENCHMARK] + phases),
                 PWU=[PHASE_EXECUTION],
                 PBM=[PHASE_EXECUTION])
names = { PHASE_INITIALIZATION : "Initialization", PHASE_PREPARATION : "Preparation",
          PHASE_COMPILATION : "Compilation", PHASE_INPUTS_AND_OUTPUTS: "I/O",
          PHASE_EXECUTION: "Execution", PHASE_RESULTS: "Results",
          PHASE_WARMUP: "Warmup", PHASE_BENCHMARK: "Benchmark",
          PHASE_TERMINATION: "Termination",
          LAYER_APPLICATION : "Application", LAYER_RUNTIME: "Runtime",
          LAYER_IPC: "IPC", LAYER_DRIVER: "Driver", LAYER_CPU: "CPU",
          "total": "Total", "NONE": "" }
layer_order = { LAYER_APPLICATION: [LAYER_RUNTIME],
                LAYER_RUNTIME: [LAYER_IPC, LAYER_CPU],
                LAYER_IPC: [LAYER_DRIVER] }


def make_tag(layer, phase):
  return "_".join([layer, phase])

def translate_hidl_mark_to_nn_and_tag(mark):
  layer = None
  if "::client" in mark:
    if "notify" in mark:
      # Call "up" into runtime from IPC. Ignore for now to not double-count
      layer = LAYER_IGNORE
    else:
      # Call "down" into IPC
      layer = LAYER_IPC
  elif "::server" in mark:
    if "notify" in mark:
      # Call "up" into IPC. Ignore for now to not double-count
      layer = LAYER_IGNORE
    else:
      # Call "down" in to driver
      layer = LAYER_DRIVER
  elif ("getCapabilities" in mark) or ("getSupportedOperations" in mark):
    layer = LAYER_DRIVER
  elif "HIDL" not in mark:
    layer = LAYER_IGNORE
  assert layer, mark

  phase = PHASE_INITIALIZATION
  if "IAllocator" in mark:
    # Used in both preparation and execution phases
    phase = "PU"
  elif ("getCapabilities" in mark):
    phase = PHASE_INITIALIZATION
  elif ("prepareModel" in mark) or ("getSupportedOperations" in mark):
    phase = PHASE_COMPILATION
  elif ("IPreparedModelCallback") in mark:
    phase = PHASE_COMPILATION
  elif ("execute" in mark) or ("IExecutionCallback" in mark):
    phase = PHASE_EXECUTION

  return "NN_" + make_tag(layer, phase)

def get_function_name_from_mark(mark):
  function = mark.split("|")[2]
  if "[NN_" in function:
    function = function.replace(MARKER_SUBTRACT, "")
    function = function.replace(MARKER_SWITCH, "")
    function = function.split("]")[1]
    function = function.replace("[", "")
    function = function.replace("]", "")
  return function
