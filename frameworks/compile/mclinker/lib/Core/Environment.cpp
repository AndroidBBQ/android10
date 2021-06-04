//===- Environment.cpp ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Environment.h"
#include "mcld/Support/TargetSelect.h"

void mcld::Initialize() {
  static bool is_initialized = false;

  if (is_initialized)
    return;

  mcld::InitializeAllTargets();
  mcld::InitializeAllEmulations();
  mcld::InitializeAllDiagnostics();

  is_initialized = true;
}

void mcld::Finalize() {
}
