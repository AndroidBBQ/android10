Copyright 2017 The Android Open Source Project

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
------------------------------------------------------------------

This directory contains files for the Android Neural Networks API.

CONTENTS OF THIS DIRECTORY

./runtime: Implementation of the NN API runtime.
           Includes source code and internal header files.
./runtime/include: The header files that an external developer would use.
                   These will be packaged with the NDK.  Includes a
                   C++ wrapper around the C API to make it easier to use.
./runtime/test: Test files.

./sample_driver: Sample driver that uses the CPU to execute queries.
                 NOT TO BE SHIPPED.  Only to be used as a testing and
                 learning tool.

./common: Contains files that can be useful for multiple components,
          e.g. runtime, driver, or tests.  Includes source code and
          internal header files.
./common/include: Header files to be used by the components using common.
./common/operations: CPU implementation of the operations.

RELATED DIRECTORIES

/hardware/interfaces/neuralnetworks: Definition of the HAL.
/hardware/interfaces/neuralnetworks/*/vts: The VTS tests.
/test/vts-testcase/hal/neuralnetworks: Configuration for the VTS tests

THE FOLLOWING SUBDIRECTORIES ARE EXPECTED IN LATER RELEASES:

./tools: Tools used to develop the API, i.e. not external developer tools
./tools/benchmark: To test performance.
/cts/tests/tests/ml/nn: The CTS tests
