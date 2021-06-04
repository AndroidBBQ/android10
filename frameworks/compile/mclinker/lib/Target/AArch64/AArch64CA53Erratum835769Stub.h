//===- AArch64CA53Erratum835769Stub.h -------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_AARCH64_AARCH64CA53ERRATUM835769STUB_H_
#define TARGET_AARCH64_AARCH64CA53ERRATUM835769STUB_H_

#include "AArch64CA53ErratumStub.h"

#include "mcld/Support/Compiler.h"
#include <llvm/Support/DataTypes.h>
#include <string>
#include <vector>

namespace mcld {

class FragmentRef;

class AArch64CA53Erratum835769Stub : public AArch64CA53ErratumStub {
 public:
  static constexpr unsigned ErratumInsnOffset = 4;

  struct ErratumSequence {
    unsigned insns[2];
  };

 public:
  AArch64CA53Erratum835769Stub();

  ~AArch64CA53Erratum835769Stub();

  bool isMyDuty(const FragmentRef& pFragRef) const;

  unsigned getErratumInsnOffset() const {
    return ErratumInsnOffset;
  }

  unsigned getErratumSequenceSize() const {
    return sizeof(ErratumSequence);
  }

 private:
  /// for doClone
  AArch64CA53Erratum835769Stub(const uint32_t* pData,
                               size_t pSize,
                               const char* pName,
                               const_fixup_iterator pBegin,
                               const_fixup_iterator pEnd);

  /// doClone
  Stub* doClone();

 private:
  DISALLOW_COPY_AND_ASSIGN(AArch64CA53Erratum835769Stub);
};

}  // namespace mcld

#endif  // TARGET_AARCH64_AARCH64CA53ERRATUM835769STUB_H_
