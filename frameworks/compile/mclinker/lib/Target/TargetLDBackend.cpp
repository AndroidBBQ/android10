//===- TargetLDBackend.cpp ------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Target/TargetLDBackend.h"

#include "mcld/LinkerConfig.h"

namespace mcld {

TargetLDBackend::TargetLDBackend(const LinkerConfig& pConfig)
    : m_Config(pConfig) {
}

TargetLDBackend::~TargetLDBackend() {
}

}  // namespace mcld
