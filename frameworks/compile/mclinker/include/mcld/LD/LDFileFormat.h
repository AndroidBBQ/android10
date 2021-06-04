//===- LDFileFormat.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_LDFILEFORMAT_H_
#define MCLD_LD_LDFILEFORMAT_H_

#include <cassert>
#include <cstddef>

namespace mcld {

class LDSection;
class ObjectBuilder;

/** \class LDFileFormat
 *  \brief LDFileFormat describes the common file formats.
 */
class LDFileFormat {
 public:
  enum Kind {
    Null,
    TEXT,  // Executable regular sections
    DATA,  // Non-executable regular sections
    BSS,
    NamePool,
    Relocation,
    Debug,
    DebugString,
    Target,
    EhFrame,
    EhFrameHdr,
    GCCExceptTable,
    Version,
    Note,
    MetaData,
    Group,
    LinkOnce,
    StackNote,
    Ignore,
    Exclude,
    Folded
  };

 protected:
  LDFileFormat();

 public:
  virtual ~LDFileFormat();

  /// initStdSections - initialize all standard section headers.
  /// @param [in] pBuilder The ObjectBuilder to create section headers
  /// @param [in] pBitClass The bitclass of target backend.
  virtual void initStdSections(ObjectBuilder& pBuilder,
                               unsigned int pBitClass) = 0;

  // -----  access functions  ----- //
  LDSection& getText() {
    assert(f_pTextSection != NULL);
    return *f_pTextSection;
  }

  const LDSection& getText() const {
    assert(f_pTextSection != NULL);
    return *f_pTextSection;
  }

  LDSection& getData() {
    assert(f_pDataSection != NULL);
    return *f_pDataSection;
  }

  const LDSection& getData() const {
    assert(f_pDataSection != NULL);
    return *f_pDataSection;
  }

  LDSection& getBSS() {
    assert(f_pBSSSection != NULL);
    return *f_pBSSSection;
  }

  const LDSection& getBSS() const {
    assert(f_pBSSSection != NULL);
    return *f_pBSSSection;
  }

  LDSection& getReadOnly() {
    assert(f_pReadOnlySection != NULL);
    return *f_pReadOnlySection;
  }

  const LDSection& getReadOnly() const {
    assert(f_pReadOnlySection != NULL);
    return *f_pReadOnlySection;
  }

 protected:
  //         variable name         :  ELF               MachO
  LDSection* f_pTextSection;      // .text             __text
  LDSection* f_pDataSection;      // .data             __data
  LDSection* f_pBSSSection;       // .bss              __bss
  LDSection* f_pReadOnlySection;  // .rodata           __const
};

}  // namespace mcld

#endif  // MCLD_LD_LDFILEFORMAT_H_
