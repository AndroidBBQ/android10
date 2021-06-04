//===- OutputArchCmd.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/OutputArchCmd.h"
#include "mcld/Support/raw_ostream.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// OutputArchCmd
//===----------------------------------------------------------------------===//
OutputArchCmd::OutputArchCmd(const std::string& pArch)
    : ScriptCommand(ScriptCommand::OUTPUT_ARCH), m_Arch(pArch) {
}

OutputArchCmd::~OutputArchCmd() {
}

void OutputArchCmd::dump() const {
  mcld::outs() << "OUTPUT_ARCH ( " << m_Arch << " )\n";
}

void OutputArchCmd::activate(Module& pModule) {
  // TODO
}

}  // namespace mcld
