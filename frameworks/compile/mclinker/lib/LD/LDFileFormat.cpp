//===- LDFileFormat.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/LDFileFormat.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// LDFileInfo
//===----------------------------------------------------------------------===//
LDFileFormat::LDFileFormat()
    : f_pTextSection(NULL),
      f_pDataSection(NULL),
      f_pBSSSection(NULL),
      f_pReadOnlySection(NULL) {
}

LDFileFormat::~LDFileFormat() {
}

}  // namespace mcld
