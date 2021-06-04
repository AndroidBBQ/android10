//===- ELFDynObjReader.h --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_ELFDYNOBJREADER_H_
#define MCLD_LD_ELFDYNOBJREADER_H_
#include "mcld/LD/DynObjReader.h"

namespace mcld {

class ELFReaderIF;
class GNULDBackend;
class Input;
class IRBuilder;
class LinkerConfig;

/** \class ELFDynObjReader
 *  \brief ELFDynObjReader reads ELF dynamic shared objects.
 *
 */
class ELFDynObjReader : public DynObjReader {
 public:
  ELFDynObjReader(GNULDBackend& pBackend,
                  IRBuilder& pBuilder,
                  const LinkerConfig& pConfig);
  ~ELFDynObjReader();

  // -----  observers  ----- //
  bool isMyFormat(Input& pFile, bool& pContinue) const;

  // -----  readers  ----- //
  bool readHeader(Input& pFile);

  bool readSymbols(Input& pInput);

 private:
  ELFReaderIF* m_pELFReader;
  IRBuilder& m_Builder;
};

}  // namespace mcld

#endif  // MCLD_LD_ELFDYNOBJREADER_H_
