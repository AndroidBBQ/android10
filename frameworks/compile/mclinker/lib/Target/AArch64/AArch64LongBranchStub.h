//===- AArch64LongBranchStub.h --------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_AARCH64_AARCH64LONGBRANCHSTUB_H_
#define TARGET_AARCH64_AARCH64LONGBRANCHSTUB_H_

#include "mcld/Fragment/Stub.h"
#include "mcld/Support/Compiler.h"
#include <llvm/Support/DataTypes.h>
#include <string>
#include <vector>

namespace mcld {

class BranchIsland;
class IRBuilder;
class Relocation;

class AArch64LongBranchStub : public Stub {
 public:
  explicit AArch64LongBranchStub(bool pIsOutputPIC);

  ~AArch64LongBranchStub();

  bool isMyDuty(const Relocation& pReloc,
                uint64_t pSource,
                uint64_t pTargetSymValue) const;

  void applyFixup(Relocation& pSrcReloc,
                  IRBuilder& pBuilder,
                  BranchIsland& pIsland);

  const std::string& name() const;

  const uint8_t* getContent() const;

  size_t size() const;

  size_t alignment() const;

 private:
  /// for doClone
  AArch64LongBranchStub(const uint32_t* pData,
                        size_t pSize,
                        const_fixup_iterator pBegin,
                        const_fixup_iterator pEnd);

  /// doClone
  Stub* doClone();

 private:
  static const uint32_t PIC_TEMPLATE[];
  static const uint32_t TEMPLATE[];
  static const uint32_t ADRP_TEMPLATE[];
  const uint32_t* m_pData;
  std::string m_Name;
  size_t m_Size;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(AArch64LongBranchStub);
};

}  // namespace mcld

#endif  // TARGET_AARCH64_AARCH64LONGBRANCHSTUB_H_
