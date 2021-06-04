This re-generates the file MPH_to_*.h.
You need to do this after modifying any of the MPH-related files.

Prerequisites:
 * GNU make

Usage:
Type 'make'.
Diff the new files in ../../src/autogen vs. the new ones as sanity check.
Build and test using the new header files.
Then do 'make clean' here.
