//===- BSDArchiveReader.h -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_BSDARCHIVEREADER_H_
#define MCLD_LD_BSDARCHIVEREADER_H_
#include "mcld/LD/ArchiveReader.h"

namespace mcld {

class Archive;
class Input;
class LinkerConfig;

/** \class BSDArchiveReader
 *  \brief BSDArchiveReader reads BSD-variant archive files.
 *
 */
class BSDArchiveReader : public ArchiveReader {
 public:
  BSDArchiveReader();
  ~BSDArchiveReader();

  bool readArchive(const LinkerConfig& pConfig, Archive& pArchive);
  bool isMyFormat(Input& pInput, bool& pContinue) const;
};

}  // namespace mcld

#endif  // MCLD_LD_BSDARCHIVEREADER_H_
