""" NNAPI systrace parser - call tree data structure and manipulation

Used by parser.tracker to gather and interpret the traces for a single thread.

'SPEC: foo' refers to specific rows in the
"NNAPI systrace contract between code and parser" document

"""

from parser.naming import subphases, layer_order
from parser.naming import LAYER_APPLICATION, LAYER_RUNTIME, LAYER_UTILITY, LAYER_IGNORE
from parser.naming import LAYER_IPC
from parser.naming import PHASE_EXECUTION, PHASE_INITIALIZATION, PHASE_OVERALL, PHASE_UNSPECIFIED

class SingleThreadCallTree(object):
  """ Tree of scoped tracepoints. Implements:
    - Creation of the tree from trace data
    - Transformation of the tree into a clear representation
      of time spent per layer and phase
    - Validation that the resulting tree follows expectations
  """
  # Creation of tree from trace data
  def __init__(self):
    self.root = CallTreeNode(None, None, None, None, None, None)
    self.current = self.root
    self.stack = []

  def push(self, start_time_s, mark, layer, phase, app_phase, subtract):
    node = self.current.add(start_time_s, mark, layer, phase, app_phase, subtract)
    self.stack.append(self.current)
    self.current = node

  def push_dummy(self, start_time_s):
    node = self.current.add_dummy(start_time_s)
    self.stack.append(self.current)
    self.current = node

  def pop(self, end_time_s):
    self.current.end_time_s = end_time_s
    ret = self.current
    self.current = self.stack.pop()
    return ret

  # Transformation of the tree

  # Remove dummies created by SPEC:"Switch phase during function"
  def remove_dummies(self):
    to_be_removed = []
    def recurse(node):
      if node.is_dummy():
        to_be_removed.append(node)
      for c in node.children:
        recurse(c)
    recurse(self.root)
    for node in to_be_removed:
      node.remove()

  # Remove tracing nodes we are not interested in
  def remove_ignored(self):
    to_be_removed = []
    def recurse(node):
      if node.layer == LAYER_IGNORE:
        to_be_removed.append(node)
      for c in node.children:
        recurse(c)
    recurse(self.root)
    for node in to_be_removed:
      node.remove()


  # For nodes that are in the wrong place in the tree: create a copy of the node
  # in the right place and mark the original to be subtracted from timing.
  # SPEC: Subtracting time when nesting is violated
  # SPEC: Onetime initialization code
  def copy_subtracted_init_and_wrong_la(self):
    to_be_subtracted = []
    def recurse(node):
      if node.subtract:
        to_be_subtracted.append(node)
      elif node.phase() == PHASE_INITIALIZATION and node.parent.phase() != PHASE_INITIALIZATION:
        to_be_subtracted.append(node)
      elif (node.parent and node.parent.layer == LAYER_APPLICATION and
            (node.layer == LAYER_RUNTIME or node.layer == LAYER_IPC) and
            node.parent.phase() != node.phase() and node.parent.phase() != PHASE_OVERALL and
            node.phase() != PHASE_EXECUTION and node.phase() not in subphases[PHASE_EXECUTION]):
        # The application level phase may be wrong, we move the runtime nodes
        # if necessary.
        to_be_subtracted.append(node)
      for c in node.children:
        recurse(c)
    recurse(self.root)
    for node in to_be_subtracted:
      layer = node.layer
      # Find where to add the subtracted time
      assert node.parent
      new_parent = node.parent
      if node.subtract:
        explanation = "from [SUB]"
        # Move [SUB] up to right layer
        while ((new_parent.layer != layer or new_parent.is_added_detail()) and
               not new_parent.is_root()):
          new_parent = new_parent.parent
      elif node.phase() == PHASE_INITIALIZATION:
        explanation = "from phase PI"
        # Move PI up to root
        while not new_parent.is_root():
          new_parent = new_parent.parent
      else:
        # Move missing LA phase up one
        explanation = "for LA_" + node.phase()
        new_parent = new_parent.parent
      if not new_parent.is_root():
        new_parent = new_parent.parent
      added = new_parent.add(
          node.start_time_s, node.mark + "(copied " + explanation + ")",
          node.layer, node.phase(), node.app_phase, subtract=False)
      added.end_time_s = node.end_time_s
      node.subtract = True

  # The application may not have added tracing for all phases, this
  # adds intermediate LA nodes where needed.
  def add_missing_la_nodes(self):
    la_to_be_added = []
    def recurse(node):
      if not node.is_added_detail() and not node.subtract:
        if ((node.layer == LAYER_RUNTIME or node.layer == LAYER_IPC) and
            # Wrong LA node
            (node.parent.layer == LAYER_APPLICATION and
             node.parent.phase() != PHASE_OVERALL and
             node.parent.phase() != node.phase()) and
            # LR_PE and subphases need to be special
            node.phase() != PHASE_EXECUTION and
            node.phase() not in subphases[PHASE_EXECUTION]):
          la_to_be_added.append(node)
      for c in node.children:
        recurse(c)
    recurse(self.root)
    for node in la_to_be_added:
      node.add_intermediate_parent(LAYER_APPLICATION, node.phase(), node.app_phase)

  # Validation
  # SPEC: Local call to other layer
  def validate_nesting(self):
    self.debugstring = ""
    def recurse(node, indent):
      [mark, layer, phase] = [node.mark, node.layer, node.phase()]
      prev_layer = (node.parent and node.parent.layer)
      prev_phase = (node.parent and node.parent.phase())
      self.debugstring += " ".join(map(str, [indent, mark, layer, phase,
                                             prev_phase, prev_layer,
                                             "subtract", node.subtract,
                                             "\n"]))
      # Check that phases nest as we expect:
      assert((prev_phase is None) or             # Entering from top without application trace
             (phase == prev_phase) or            # Same phase
             (phase == PHASE_UNSPECIFIED) or     # Utility function
             (phase == PHASE_INITIALIZATION) or  # One-time initialization
             (phase in subphases.get(prev_phase, [])) or # Subphase as designed
             (phase in subphases.get(PHASE_EXECUTION) and # Nested subphase missing
              PHASE_EXECUTION in subphases.get(prev_phase, [])) or
             node.subtract                       # Marker for wrong nesting
             ), self.debugstring
      # Check that layers nest as we expect:
      assert ((prev_layer is None) or
              (layer == LAYER_UTILITY) or
              (layer == prev_layer) or
              (layer in layer_order.get(prev_layer, [])) or
              node.subtract), self.debugstring
      for c in node.children:
        recurse(c, indent + '  ')
    recurse(self.root, '')

  # Auxiliary functionality
  def print(self):
    print(self.to_str())

  def to_str(self):
    return self.root.to_str()

  def is_empty(self):
    return not self.root.children



class CallTreeNode(object):
  """ Single scoped tracepoint. """
  def __init__(self, start_time_s, mark, layer, phase, app_phase, subtract):
    self.children = []
    self.start_time_s = start_time_s
    self.mark = mark
    self.layer = layer
    self.phase_ = phase
    self.app_phase = app_phase
    self.subtract = subtract
    self.end_time_s = None
    self.parent = None

  def is_root(self):
    return self.start_time_s is None

  def is_dummy(self):
    return not self.is_root() and self.mark is None

  def phase(self):
    if self.phase_ == PHASE_UNSPECIFIED:
      return self.parent.phase()
    else:
      return self.phase_

  def is_added_detail(self):
    if self.is_root():
      return False
    if self.parent.is_root():
      return False
    if self.layer != self.parent.layer:
      return False
    if self.phase() in subphases.get(self.parent.phase(), []):
      return False
    if self.phase() == PHASE_INITIALIZATION and self.parent.phase() != PHASE_INITIALIZATION:
      return False
    if self.subtract:
      return False
    return True

  def elapsed_ms(self):
    if (self.end_time_s is None) or (self.start_time_s is None):
      return None
    return (float(self.end_time_s) - float(self.start_time_s)) * 1000.0

  def elapsed_less_subtracted_ms(self):
    ret = self.elapsed_ms()
    if ret is None:
      return None
    for c in self.children:
      ret = ret - c.subtracted_ms()
    return ret

  def subtracted_ms(self):
    subtract = 0.0
    if self.is_added_detail():
      for c in self.children:
        subtract = subtract + c.subtracted_ms()
    elif self.subtract:
      subtract = self.elapsed_ms()
    return subtract

  def add(self, start_time_s, mark, layer, phase, app_phase, subtract):
    node = CallTreeNode(start_time_s, mark, layer, phase, app_phase, subtract)
    node.parent = self
    self.children.append(node)
    return node

  def add_intermediate_parent(self, layer, phase, app_phase):
    node = CallTreeNode(self.start_time_s,
                        " ".join([self.mark, "( added intermediate",
                                  layer, phase, ")"]),
                        layer, phase, app_phase, subtract=False)
    node.end_time_s = float(self.start_time_s) + self.elapsed_less_subtracted_ms() / 1000.0
    node.parent = self.parent
    for i in range(0, len(self.parent.children)):
      if self.parent.children[i] == self:
        self.parent.children[i] = node
        break
    self.parent = node
    node.children.append(self)

  def add_dummy(self, start_time_s):
    node = CallTreeNode(start_time_s, None, None, None, None, None)
    node.parent = self
    self.children.append(node)
    return node

  def remove(self):
    self.parent.children.remove(self)
    self.parent.children.extend(self.children)
    for c in self.children:
      c.parent = self.parent
    self.parent = None

  def to_str(self, indent=''):
    if not self.is_root():
      ret = " ".join(map(str, [indent, self.app_phase, self.mark,
                               self.elapsed_less_subtracted_ms(),
                               "subtract: ", self.subtract, "\n"]))
    else:
      ret = " ".join([indent, "ROOT", "\n"])
    if self.children:
      ret += (indent + "   children:\n")
      for c in self.children:
        ret += c.to_str(indent + '    ')
    return ret
