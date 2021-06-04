This is the source code for the Wilhelm project, an implementation of native audio
and multimedia for Android based on Khronos Group OpenSL ES and OpenMAX AL 1.0.1.

Top-level:
 * Makefile
 * Initial entry points
 * Common glue code

Subdirectories:
android/ Android platform-specific code, other than interfaces
autogen/ Automagically generated files, do not edit by hand
desktop/ Experimental desktop PC platform-specific code
itf/     OpenSL ES and OpenMAX AL interfaces, including Android-specific extensions
objects/ OpenSL ES and OpenMAX AL objects aka classes
ut/      Private utility toolkit
