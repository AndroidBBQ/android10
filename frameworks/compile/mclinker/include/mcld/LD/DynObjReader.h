//===- DynObjReader.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_DYNOBJREADER_H_
#define MCLD_LD_DYNOBJREADER_H_
#include "mcld/LD/LDReader.h"

namespace mcld {

class TargetLDBackend;
class Input;

/** \class DynObjReader
 *  \brief DynObjReader provides an common interface for different object
 *  formats.
 */
class DynObjReader : public LDReader {
 protected:
  DynObjReader() {}

 public:
  virtual ~DynObjReader() {}

  virtual bool readHeader(Input& pFile) = 0;

  virtual bool readSymbols(Input& pFile) = 0;
};

}  // namespace mcld

#endif  // MCLD_LD_DYNOBJREADER_H_
