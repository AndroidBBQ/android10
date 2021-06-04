//===- ELFEmulation.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_TARGET_ELFEMULATION_H_
#define MCLD_TARGET_ELFEMULATION_H_

namespace mcld {

class LinkerConfig;
class LinkerScript;

bool MCLDEmulateELF(LinkerScript& pScript, LinkerConfig& pConfig);

}  // namespace mcld

#endif  // MCLD_TARGET_ELFEMULATION_H_
