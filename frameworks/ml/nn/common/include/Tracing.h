/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_ML_NN_COMMON_TRACING_H
#define ANDROID_ML_NN_COMMON_TRACING_H

#define ATRACE_TAG ATRACE_TAG_NNAPI
#include "utils/Trace.h"

// Neural Networks API (NNAPI) systracing
//
// Primary goal of the tracing is to capture and present timings for NNAPI.
// (Other uses include providing visibility to split of execution between
// drivers and the CPU fallback, and the ability to visualize call sequences).
//
// The tracing has three parts:
//  1 Trace macros defined in this file and used throughout the codebase,
//    modelled after and using atrace. These implement a naming convention for
//    the tracepoints, interpreted by the systrace parser.
//  2 Android systrace (atrace) on-device capture and host-based analysis.
//  3 A systrace parser (TODO) to summarize the timings.
//
// For an overview and introduction, please refer to the "NNAPI Systrace design
// and HOWTO" (internal Docs for now). This header doesn't try to replicate all
// the information in that document. For the contract between traces in code and
// the statistics created by the systrace parser, see
// tools/systrace-parser/contract-between-code-and-parser.txt.
//
// Glossary:
// - Phase: stage in processing (e.g., Preparation, Compilation, Execution);
//   Overall phase nests rest, Execution nests Input/Output, Transformation,
//   Computation and Results; optionally Executions can be nested in a
//   Warmup and Benchmark - otherwise not nested (Initialization phase
//   functions may occur inside other phases but will be counted out during
//   analysis). Nested phases (other than Initialization) are analysed as a
//   breakdown of the parent phase.
// - Layer: component in the stack (from top to bottom: App, Runtime, IPC,
//   Driver/CPU). Calls to lower layers are typically nested within calls to upper
//   layers.
// - Bucket: unit of timing analysis, the combination of Phase and Layer (and
//   thus also typically nested).
// - Detail: specific unit being executed, typically a function.

// Convenience macros to be used in the code (phases defined below).
// (Macros so that string concatenation is done at compile time).
//
// These exist in three variants:
// - Simple (NNTRACE_<layer and potentially phase>) - to be used when only one
//   Phase is active within a scope
// - "Switch" (NNTRACE_<...>_SWITCH) - to be used when multiple Phases
//   share a scope (e.g., transformation of data and computation in same
//   function).
// - "Subtract" (NNTRACE_<...>_SUBTRACT) - to be used when nesting is violated
//   and the time should be subtracted from the parent scope
// Arguments:
// - phase: one of the NNTRACE_PHASE_* macros defined below.
// - detail: free-form string constant, typically function name.
// Example usage:
//   // Simple
//   int ANeuralNetworksMemory_createFromFd(...) {
//     NNTRACE_RT(NNTRACE_PHASE_PREPARATION, "ANeuralNetworksMemory_createFromFd");
//   }
//   // Switch
//   bool concatenationFloat32(...) {
//     NNTRACE_TRANS("concatenationFloat32");  // Transformation of data begins
//     ...
//     NNTRACE_COMP_SWITCH("optimized_ops::Concatenation"); // Transformation
//                                                          // ends and computation
//                                                          // begins
//   }
//   // Subtract
//   static int compile(...) {
//     NNTRACE_FULL(NNTRACE_LAYER_IPC, NNTRACE_PHASE_COMPILATION, "prepareModel");
//     device->getInterface()->prepareModel(..., preparedModelCallback);
//     preparedModelCallback->wait()
//   }
//   ErrorStatus VersionedIDevice::prepareModel(...) {
//     ... IPC work ...
//     {
//       NNTRACE_FULL_SUBTRACT(NNTRACE_LAYER_RUNTIME, NNTRACE_PHASE_COMPILATION,
//                             "VersionedIDevice::prepareModel");
//       ... Runtime work ...
//     }
//     ... IPC work ...
//   }
//
// Layer Application - For native applications (e.g., unit tests)
#define NNTRACE_APP(phase, detail) NNTRACE_FULL(NNTRACE_LAYER_APPLICATION, phase, detail)
#define NNTRACE_APP_SWITCH(phase, detail) \
        NNTRACE_FULL_SWITCH(NNTRACE_LAYER_APPLICATION, phase, detail)
// Layer Runtime - For the NNAPI runtime
#define NNTRACE_RT(phase, detail) NNTRACE_FULL(NNTRACE_LAYER_RUNTIME, phase, detail)
#define NNTRACE_RT_SWITCH(phase, detail) NNTRACE_FULL_SWITCH(NNTRACE_LAYER_RUNTIME, phase, detail)
// Layer CPU - CPU executor
#define NNTRACE_CPU(phase, detail) NNTRACE_FULL(NNTRACE_LAYER_CPU, phase, detail)
#define NNTRACE_COMP(detail) NNTRACE_FULL(NNTRACE_LAYER_CPU, \
                                          NNTRACE_PHASE_COMPUTATION, detail)
#define NNTRACE_COMP_SWITCH(detail) NNTRACE_FULL_SWITCH(NNTRACE_LAYER_CPU, \
                                                        NNTRACE_PHASE_COMPUTATION, detail)
#define NNTRACE_TRANS(detail) NNTRACE_FULL(NNTRACE_LAYER_CPU, \
                                           NNTRACE_PHASE_TRANSFORMATION, detail)

// Fully specified macros to be used when no convenience wrapper exists for your
// need.
#define NNTRACE_FULL(layer, phase, detail) NNTRACE_NAME_1(("[NN_" layer "_" phase "]" detail))
#define NNTRACE_FULL_SWITCH(layer, phase, detail) \
        NNTRACE_NAME_SWITCH(("[SW][NN_" layer "_" phase "]" detail))
#define NNTRACE_FULL_SUBTRACT(layer, phase, detail) \
        NNTRACE_NAME_1(("[SUB][NN_" layer "_" phase "]" detail))
// Raw macro without scoping requirements, for special cases
#define NNTRACE_FULL_RAW(layer, phase, detail) android::ScopedTrace PASTE(___tracer, __LINE__) \
        (ATRACE_TAG, ("[NN_" layer "_" phase "]" detail))

// Tracing buckets - for calculating timing summaries over.
//
// Application-only phases
#define NNTRACE_PHASE_OVERALL   "PO"    // Overall program, e.g., one benchmark case
#define NNTRACE_PHASE_WARMUP    "PWU"   // Warmup (nesting multiple executions)
#define NNTRACE_PHASE_BENCHMARK "PBM"   // Benchmark (nesting multiple executions)
// Main phases, usable by all layers
#define NNTRACE_PHASE_INITIALIZATION "PI" // Initialization - not related to a model
#define NNTRACE_PHASE_PREPARATION "PP"  // Model construction
#define NNTRACE_PHASE_COMPILATION "PC"  // Model compilation
#define NNTRACE_PHASE_EXECUTION "PE"    // Executing the model
#define NNTRACE_PHASE_TERMINATION "PT"  // Tearing down
#define NNTRACE_PHASE_UNSPECIFIED "PU"  // Helper code called from multiple phases
// Subphases of execution
#define NNTRACE_PHASE_INPUTS_AND_OUTPUTS "PIO"  // Setting inputs/outputs and allocating buffers
#define NNTRACE_PHASE_TRANSFORMATION "PTR"      // Transforming data for computation
#define NNTRACE_PHASE_COMPUTATION "PCO"         // Computing operations' outputs
#define NNTRACE_PHASE_RESULTS "PR"              // Reading out results
// Layers
#define NNTRACE_LAYER_APPLICATION "LA"
#define NNTRACE_LAYER_RUNTIME "LR"
#define NNTRACE_LAYER_IPC "LI"
#define NNTRACE_LAYER_DRIVER "LD"
#define NNTRACE_LAYER_CPU "LC"
#define NNTRACE_LAYER_OTHER "LO"
#define NNTRACE_LAYER_UTILITY "LU"              // Code used from multiple layers


// Implementation
//
// Almost same as ATRACE_NAME, but enforcing explicit distinction between
// phase-per-scope and switching phases.
//
// Basic trace, one per scope allowed to enforce disjointness
#define NNTRACE_NAME_1(name) android::ScopedTrace ___tracer_1(ATRACE_TAG, name)
// Switching trace, more than one per scope allowed, translated by
// systrace_parser.py. This is mainly useful for tracing multiple phases through
// one function / scope.
#define NNTRACE_NAME_SWITCH(name) android::ScopedTrace PASTE(___tracer, __LINE__) \
        (ATRACE_TAG, name); \
        (void)___tracer_1  // ensure switch is only used after a basic trace


// Disallow use of raw ATRACE macros
#undef ATRACE_NAME
#undef ATRACE_CALL

#endif // ANDROID_ML_NN_COMMON_TRACING_H
