//===- ELFObjectReader.h --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_ELFOBJECTREADER_H_
#define MCLD_LD_ELFOBJECTREADER_H_

#include "mcld/ADT/Flags.h"
#include "mcld/LD/ObjectReader.h"

namespace mcld {

class EhFrameReader;
class ELFReaderIF;
class Input;
class IRBuilder;
class GNULDBackend;
class LinkerConfig;

/** \lclass ELFObjectReader
 *  \brief ELFObjectReader reads target-independent parts of ELF object file
 */
class ELFObjectReader : public ObjectReader {
 public:
  enum ReadFlagType {
    ParseEhFrame = 0x1,  ///< parse .eh_frame section if the bit is set.
    NumOfReadFlags = 1
  };

  typedef Flags<ReadFlagType> ReadFlag;

 public:
  ELFObjectReader(GNULDBackend& pBackend,
                  IRBuilder& pBuilder,
                  const LinkerConfig& pConfig);

  ~ELFObjectReader();

  // -----  observers  ----- //
  bool isMyFormat(Input& pFile, bool& pContinue) const;

  // -----  readers  ----- //
  bool readHeader(Input& pFile);

  virtual bool readSections(Input& pFile);

  virtual bool readSymbols(Input& pFile);

  /// readRelocations - read relocation sections
  ///
  /// This function should be called after symbol resolution.
  virtual bool readRelocations(Input& pFile);

 private:
  ELFReaderIF* m_pELFReader;
  EhFrameReader* m_pEhFrameReader;
  IRBuilder& m_Builder;
  ReadFlag m_ReadFlag;
  GNULDBackend& m_Backend;
  const LinkerConfig& m_Config;
};

}  // namespace mcld

#endif  // MCLD_LD_ELFOBJECTREADER_H_
