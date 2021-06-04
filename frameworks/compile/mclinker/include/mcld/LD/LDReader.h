//===- LDReader.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_LDREADER_H_
#define MCLD_LD_LDREADER_H_

namespace mcld {

class Input;

/** \class LDReader
 *  \brief LDReader provides the basic interfaces for all readers. It also
 *  provides basic functions to read data stream.
 */
class LDReader {
 public:
  enum Endian { LittleEndian, BigEndian };

 protected:
  LDReader() {}

 public:
  virtual ~LDReader() {}

  virtual bool isMyFormat(Input& pInput, bool& pContinue) const = 0;
};

}  // namespace mcld

#endif  // MCLD_LD_LDREADER_H_
