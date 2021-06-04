//===- ArchiveReader.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_ARCHIVEREADER_H_
#define MCLD_LD_ARCHIVEREADER_H_
#include "mcld/LD/LDReader.h"

namespace mcld {

class Archive;
class LinkerConfig;

/** \class ArchiveReader
 *  \brief ArchiveReader provides an common interface for all archive readers.
 *
 *  ArchiveReader also reads the target-independent parts of an archive file.
 *  There are some property on all the archive formats.
 *  1. All archive elements star on an even boundary, new line padded;
 *  2. All archive headers are char *;
 *  3. All archive headers are the same size.
 */

class ArchiveReader : public LDReader {
 public:
  ArchiveReader();
  virtual ~ArchiveReader();

  virtual bool readArchive(const LinkerConfig& pConfig, Archive& pArchive) = 0;
};

}  // namespace mcld

#endif  // MCLD_LD_ARCHIVEREADER_H_
