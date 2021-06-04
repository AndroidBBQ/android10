//===- AArch64CA53Erratum843419Stub2.h ------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_AARCH64_AARCH64CA53ERRATUM843419STUB2_H_
#define TARGET_AARCH64_AARCH64CA53ERRATUM843419STUB2_H_

#include "AArch64CA53ErratumStub.h"

#include "mcld/Support/Compiler.h"
#include <llvm/Support/DataTypes.h>
#include <string>
#include <vector>

namespace mcld {

class FragmentRef;

class AArch64CA53Erratum843419Stub2 : public AArch64CA53ErratumStub {
 public:
  static constexpr unsigned ErratumInsnOffset = 12;

  struct ErratumSequence {
    unsigned insns[4];
  };

 public:
  AArch64CA53Erratum843419Stub2();

  ~AArch64CA53Erratum843419Stub2();

  bool isMyDuty(const FragmentRef& pFragRef) const;

  unsigned getErratumInsnOffset() const {
    return ErratumInsnOffset;
  }

  unsigned getErratumSequenceSize() const {
    return sizeof(ErratumSequence);
  }

 private:
  /// for doClone
  AArch64CA53Erratum843419Stub2(const uint32_t* pData,
                                size_t pSize,
                                const char* pName,
                                const_fixup_iterator pBegin,
                                const_fixup_iterator pEnd);

  /// doClone
  Stub* doClone();

 private:
  DISALLOW_COPY_AND_ASSIGN(AArch64CA53Erratum843419Stub2);
};

}  // namespace mcld

#endif  // TARGET_AARCH64_AARCH64CA53ERRATUM843419STUB2_H_
