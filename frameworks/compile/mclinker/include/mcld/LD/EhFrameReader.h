//===- EhFrameReader.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_EHFRAMEREADER_H_
#define MCLD_LD_EHFRAMEREADER_H_
#include "mcld/LD/EhFrame.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/DataTypes.h>

namespace mcld {

class Input;
class LDSection;

/** \class EhFrameReader
 *  \brief EhFrameReader reads .eh_frame section
 *
 *  EhFrameReader is responsible to parse the input eh_frame sections and create
 *  the corresponding CIE and FDE entries.
 */
class EhFrameReader {
 public:
  typedef const char* ConstAddress;
  typedef char* Address;

 public:
  /// read - read an .eh_frame section and create the corresponding
  /// CIEs and FDEs
  /// @param pInput [in] the Input contains this eh_frame
  /// @param pEhFrame [inout] the input eh_frame
  /// @return if we read all CIEs and FDEs successfully, return true. Otherwise,
  /// return false;
  template <size_t BITCLASS, bool SAME_ENDIAN>
  bool read(Input& pInput, EhFrame& pEhFrame);

 private:
  enum TokenKind { CIE, FDE, Terminator, Unknown, NumOfTokenKinds };

  enum State { Q0, Q1, Accept, NumOfStates = 2, Reject = -1 };

  struct Token {
    TokenKind kind;
    size_t file_off;
    size_t data_off;
    uint64_t size;
  };

  /// Action - the transition function of autometa.
  /// @param pEhFrame - the output .eh_frame section
  /// @param pSection - the input .eh_frame section
  /// @param pRegion - the memory region that needs to handle with.
  typedef bool (*Action)(EhFrame& pEhFrame,
                         llvm::StringRef pRegion,
                         const Token& pToken);

 private:
  /// scan - scan pData from pHandler for a token.
  template <bool SAME_ENDIAN>
  Token scan(ConstAddress pHandler,
             uint64_t pOffset,
             llvm::StringRef pData) const;

  static bool addCIE(EhFrame& pEhFrame,
                     llvm::StringRef pRegion,
                     const Token& pToken);

  static bool addFDE(EhFrame& pEhFrame,
                     llvm::StringRef pRegion,
                     const Token& pToken);

  static bool addTerm(EhFrame& pEhFrame,
                      llvm::StringRef pRegion,
                      const Token& pToken);

  static bool reject(EhFrame& pEhFrame,
                     llvm::StringRef pRegion,
                     const Token& pToken);
};

template <>
bool EhFrameReader::read<32, true>(Input& pInput, EhFrame& pEhFrame);

template <>
EhFrameReader::Token EhFrameReader::scan<true>(ConstAddress pHandler,
                                               uint64_t pOffset,
                                               llvm::StringRef pData) const;

}  // namespace mcld

#endif  // MCLD_LD_EHFRAMEREADER_H_
