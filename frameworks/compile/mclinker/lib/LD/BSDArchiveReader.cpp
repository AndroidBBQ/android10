//===- BSDArchiveReader.cpp -----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/BSDArchiveReader.h"
#include "mcld/LD/Archive.h"
#include "mcld/MC/Input.h"

namespace mcld {

BSDArchiveReader::BSDArchiveReader() {
}

BSDArchiveReader::~BSDArchiveReader() {
}

bool BSDArchiveReader::readArchive(const LinkerConfig& pConfig,
                                   Archive& pArchive) {
  // TODO
  return true;
}

bool BSDArchiveReader::isMyFormat(Input& pInput, bool& pContinue) const {
  pContinue = true;
  // TODO
  return false;
}

}  // namespace mcld
