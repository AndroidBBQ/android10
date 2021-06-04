//===- BinaryReader.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_BINARYREADER_H_
#define MCLD_LD_BINARYREADER_H_
#include "mcld/LD/LDReader.h"

namespace mcld {

class Input;

/** \class BinaryReader
 *  \brief BinaryReader provides an common interface for different Binary
 *  formats.
 */
class BinaryReader : public LDReader {
 public:
  virtual ~BinaryReader() = 0;

  virtual bool isMyFormat(Input& pInput, bool& pContinue) const {
    pContinue = true;
    return false;
  }

  virtual bool readBinary(Input& pFile) = 0;
};

}  // namespace mcld

#endif  // MCLD_LD_BINARYREADER_H_
