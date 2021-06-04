//===- ELFBinaryReader.h --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_ELFBINARYREADER_H_
#define MCLD_LD_ELFBINARYREADER_H_

#include "mcld/LD/BinaryReader.h"

namespace mcld {

class Input;
class IRBuilder;
class LinkerConfig;

/** \lclass ELFBinaryReader
 *  \brief ELFBinaryReader reads target-independent parts of Binary file
 */
class ELFBinaryReader : public BinaryReader {
 public:
  ELFBinaryReader(IRBuilder& pBuilder, const LinkerConfig& pConfig);

  ~ELFBinaryReader();

  bool isMyFormat(Input& pInput, bool& pContinue) const;

  bool readBinary(Input& pInput);

 private:
  IRBuilder& m_Builder;
  const LinkerConfig& m_Config;
};

}  // namespace mcld

#endif  // MCLD_LD_ELFBINARYREADER_H_
