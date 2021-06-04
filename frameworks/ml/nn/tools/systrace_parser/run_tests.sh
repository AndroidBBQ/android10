#!/bin/bash

set -e
cd $(dirname $(realpath $0))
echo Running python unittests
python3 -m unittest discover parser/test
echo Running large tests
./parser/test/large_test.sh
echo All tests passed
