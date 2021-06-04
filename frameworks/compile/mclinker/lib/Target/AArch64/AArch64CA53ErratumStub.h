//===- AArch64CA53ErratumStub.h -------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_AARCH64_AARCH64CA53ERRATUMSTUB_H_
#define TARGET_AARCH64_AARCH64CA53ERRATUMSTUB_H_

#include "mcld/Fragment/Stub.h"
#include "mcld/Support/Compiler.h"
#include <llvm/Support/DataTypes.h>
#include <string>
#include <vector>

namespace mcld {

class BranchIsland;
class FragmentRef;
class IRBuilder;

class AArch64CA53ErratumStub : public Stub {
 public:
  AArch64CA53ErratumStub();

  AArch64CA53ErratumStub(const uint32_t* pData,
                         size_t pSize,
                         const char* pName,
                         const_fixup_iterator pBegin,
                         const_fixup_iterator pEnd);

  ~AArch64CA53ErratumStub();

  bool isMyDuty(const FragmentRef& pFragRef) const;

  void applyFixup(FragmentRef& pSrcFragRef,
                  IRBuilder& pBuilder,
                  BranchIsland& pIsland);

  const std::string& name() const;

  const uint32_t* getData() const;

  const uint8_t* getContent() const;

  size_t size() const;

  size_t alignment() const;

 public:
  virtual unsigned getErratumSequenceSize() const = 0;

  virtual unsigned getErratumInsnOffset() const = 0;

 private:
  static const uint32_t TEMPLATE[];

 private:
  const uint32_t* m_pData;
  std::string m_Name;
  size_t m_Size;

 private:
  DISALLOW_COPY_AND_ASSIGN(AArch64CA53ErratumStub);
};

}  // namespace mcld

#endif  // TARGET_AARCH64_AARCH64CA53ERRATUMSTUB_H_
