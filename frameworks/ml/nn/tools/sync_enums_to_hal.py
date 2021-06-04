#!/usr/bin/python3
""" Synchronizes enums and their comments from the NeuralNetworks.h to types.hal

Workflow:
  - Don't try to make other changes to types.hal in the same branch, as this
    will check out and overwrite files
  - Edit NeuralNetworks.h
  - run sync_enums_to_hal.py
    - can be run from anywhere, but ANDROID_BUILD_TOP must be set
    - this resets 1.[0-2]/types.hal to last commit (so you can run
      the script multiple times with changes to it in-between), and
    - overwrites types.hal in-place
  - Check the output (git diff)
  - Recreate hashes
  - commit and upload for review

Note:
This is somewhat brittle in terms of ordering and formatting of the
relevant files. It's the author's opinion that it's not worth spending a lot of
time upfront coming up with better mechanisms, but to improve it when needed.
For example, currently Operations have differences between 1.0 and 1.1,
but Operands do not, so the script is explicit rather than generic.

There are asserts in the code to make sure the expectations on the ordering and
formatting of the headers are met, so this should fail rather than produce
completely unexpected output.

The alternative would be to add explicit section markers to the files.

"""

import os
import re
import subprocess

class HeaderReader(object):
  """ Simple base class facilitates reading a file into sections and writing it
      back out
  """
  def __init__(self):
    self.sections = []
    self.current = -1
    self.next_section()

  def put_back(self, no_of_lines=1):
    assert not self.sections[self.current]
    for i in range(no_of_lines):
      line = self.sections[self.current - 1].pop()
      self.sections[self.current].insert(0, line)

  def pop_back(self, no_of_lines=1):
    for i in range(no_of_lines):
      self.sections[self.current].pop()

  def next_section(self):
    self.current = self.current + 1
    self.sections.append([])

  def get_contents(self):
    return "".join([ "".join(s) for s in self.sections])

  def get_section(self, which):
    return "".join(self.sections[which])

  def handle_line(self, line):
    assert False

  def read(self, filename):
    assert self.current == 0
    self.filename = filename
    with open(filename) as f:
      lines = f.readlines()
    for line in lines:
      self.sections[self.current].append(line)
      if self.current == self.REST:
        continue
      self.handle_line(line)
    assert self.current == self.REST

  def write(self):
    with open(self.filename, "w") as f:
      f.write(self.get_contents())

class Types10Reader(HeaderReader):
  """ Reader for 1.0 types.hal

      The structure of the file is:
      - preamble
      - enum OperandType ... {
        < this becomes the OPERAND section >
        OEM operands
        };
      - comments
      - enum OperationType ... {
        < this becomes the OPERATION section >
        OEM operarions
        };
      - rest
  """
  BEFORE_OPERAND = 0
  OPERAND = 1
  BEFORE_OPERATION = 2
  OPERATION = 3
  REST = 4

  def __init__(self):
    super(Types10Reader, self).__init__()
    self.read("hardware/interfaces/neuralnetworks/1.0/types.hal")

  def handle_line(self, line):
    if "enum OperandType" in line:
      assert self.current == self.BEFORE_OPERAND
      self.next_section()
    elif "enum OperationType" in line:
      assert self.current == self.BEFORE_OPERATION
      self.next_section()
    elif "OEM" in line and self.current == self.OPERAND:
      self.next_section()
      self.put_back(2)
    elif "OEM specific" in line and self.current == self.OPERATION:
      self.next_section()
      self.put_back(2)

class Types11Reader(HeaderReader):
  """ Reader for 1.1 types.hal

      The structure of the file is:
      - preamble
      - enum OperationType ... {
        < this becomes the OPERATION section >
        };
      - rest
  """

  BEFORE_OPERATION = 0
  OPERATION = 1
  REST = 2

  FILENAME = "hardware/interfaces/neuralnetworks/1.1/types.hal"

  def __init__(self):
    super(Types11Reader, self).__init__()
    self.read(self.FILENAME)

  def handle_line(self, line):
    if "enum OperationType" in line:
      assert self.current == self.BEFORE_OPERATION
      self.next_section()
    # there is more content after the enums we are interested in so
    # it cannot be the last line, can match with \n
    elif line == "};\n":
      self.next_section()
      self.put_back()

class Types12Reader(Types11Reader):
  """ Reader for 1.2 types.hal

      Assumes the structure of the file is the same as in v1.1.
  """

  FILENAME = "hardware/interfaces/neuralnetworks/1.2/types.hal"

class NeuralNetworksReader(HeaderReader):
  """ Reader for NeuralNetworks.h

      The structure of the file is:
      - preamble
      - typedef enum {
        < this becomes the OPERAND section >
        } OperandCode;
      - comments
      - typedef enum {
        // Operations below are available since API level 27.
        < this becomes the OPERATION_V1_0 section >
        // Operations below are available since API level 28.
        < this becomes the OPERATION_V1_1 section >
        // Operations below are available since API level 29.
        < this becomes the OPERATION_V1_2 section >
        } OperationCode;
      - rest
  """

  BEFORE_OPERAND = 0
  OPERAND = 1
  BEFORE_OPERATION = 2
  OPERATION_V1_0 = 3
  OPERATION_V1_1 = 4
  OPERATION_V1_2 = 5
  REST = 6

  def __init__(self):
    super(NeuralNetworksReader, self).__init__()
    self.read("frameworks/ml/nn/runtime/include/NeuralNetworks.h")

  def handle_line(self, line):
    if line == "typedef enum {\n":
      self.next_section()
    elif line == "} OperandCode;\n":
      assert self.current == self.OPERAND
      self.next_section()
      self.put_back()
    elif "// Operations below are available since API level 27." in line:
      assert self.current == self.OPERATION_V1_0
      self.pop_back()
    elif "// Operations below are available since API level 28." in line:
      assert self.current == self.OPERATION_V1_0
      self.pop_back()
      self.next_section()
    elif "// Operations below are available since API level 29." in line:
      assert self.current == self.OPERATION_V1_1
      self.pop_back()
      self.next_section()
    elif line == "} OperationCode;\n":
      assert self.current == self.OPERATION_V1_2
      self.next_section()
      self.put_back()


if __name__ == "__main__":
  # Reset
  assert os.environ["ANDROID_BUILD_TOP"]
  os.chdir(os.environ["ANDROID_BUILD_TOP"])
  subprocess.run(
      "cd hardware/interfaces/neuralnetworks && git checkout */types.hal",
      shell=True)

  # Read existing contents
  types10 = Types10Reader()
  types11 = Types11Reader()
  types12 = Types12Reader()
  nn = NeuralNetworksReader()

  # Rewrite from header syntax to HAL and replace types.hal contents
  operand = []
  for line in nn.sections[nn.OPERAND]:
    line = line.replace("ANEURALNETWORKS_", "")
    operand.append(line)
  types10.sections[types10.OPERAND] = operand
  def rewrite_operation(from_nn):
    hal = []
    for line in from_nn:
      if "TODO" in line:
        continue

      # Match multiline comment style
      if re.match("^    */\*\* \w.*[^/]$", line):
        hal.append("    /**\n")
        line = line.replace("/** ", " * ")
      # Match naming changes in HAL vs framework
      line = line.replace("@link OperandCode", "@link OperandType")
      line = line.replace("@link ANEURALNETWORKS_", "@link OperandType::")
      line = line.replace("ANEURALNETWORKS_", "")
      line = line.replace("FuseCode", "FusedActivationFunc")
      # PaddingCode is not part of HAL, rewrite
      line = line.replace("{@link PaddingCode} values",
                          "following values: {0 (NONE), 1 (SAME), 2 (VALID)}")
      hal.append(line)
    return hal
  types10.sections[types10.OPERATION] = rewrite_operation(nn.sections[nn.OPERATION_V1_0])
  types11.sections[types11.OPERATION] = rewrite_operation(nn.sections[nn.OPERATION_V1_1])
  types12.sections[types12.OPERATION] = rewrite_operation(nn.sections[nn.OPERATION_V1_2])

  # Write synced contents
  types10.write()
  types11.write()
  types12.write()

  print("")
  print("The files")
  print("  " + types10.filename + " and")
  print("  " + types11.filename + " and")
  print("  " + types12.filename)
  print("have been rewritten")
  print("")
  print("Check that the change matches your expectations and regenerate the hashes")
  print("")
