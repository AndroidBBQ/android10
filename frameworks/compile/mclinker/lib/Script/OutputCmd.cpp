//===- OutputCmd.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/OutputCmd.h"
#include "mcld/Support/raw_ostream.h"
#include "mcld/LinkerScript.h"
#include "mcld/Module.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// OutputCmd
//===----------------------------------------------------------------------===//
OutputCmd::OutputCmd(const std::string& pOutputFile)
    : ScriptCommand(ScriptCommand::OUTPUT), m_OutputFile(pOutputFile) {
}

OutputCmd::~OutputCmd() {
}

void OutputCmd::dump() const {
  mcld::outs() << "OUTPUT ( " << m_OutputFile << " )\n";
}

void OutputCmd::activate(Module& pModule) {
  pModule.getScript().setOutputFile(m_OutputFile);
  // TODO: set the output name if there is no `-o filename' on the cmdline.
  // This option is to define a default name for the output file other than the
  // usual default of a.out.
}

}  // namespace mcld
