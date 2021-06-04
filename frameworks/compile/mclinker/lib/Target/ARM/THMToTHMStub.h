//===- THMToTHMStub.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_ARM_THMTOTHMSTUB_H_
#define TARGET_ARM_THMTOTHMSTUB_H_

#include "mcld/Fragment/Stub.h"
#include <llvm/Support/DataTypes.h>
#include <string>

namespace mcld {

class Relocation;
class ResolveInfo;

/** \class THMToTHMStub
 *  \brief ARM stub for long call from ARM source to ARM target
 *
 */
class THMToTHMStub : public Stub {
 public:
  THMToTHMStub(bool pIsOutputPIC, bool pUsingThumb2);

  ~THMToTHMStub();

  // isMyDuty
  bool isMyDuty(const class Relocation& pReloc,
                uint64_t pSource,
                uint64_t pTargetSymValue) const;

  // observers
  const std::string& name() const;

  const uint8_t* getContent() const;

  size_t size() const;

  size_t alignment() const;

  // for T bit of this stub
  uint64_t initSymValue() const;

 private:
  THMToTHMStub(const THMToTHMStub&);

  THMToTHMStub& operator=(const THMToTHMStub&);

  /// for doClone
  THMToTHMStub(const uint32_t* pData,
               size_t pSize,
               const_fixup_iterator pBegin,
               const_fixup_iterator pEnd,
               bool pUsingThumb2);

  /// doClone
  Stub* doClone();

 private:
  static const uint32_t PIC_TEMPLATE[];
  static const uint32_t TEMPLATE[];
  const uint32_t* m_pData;
  std::string m_Name;
  size_t m_Size;
  bool m_bUsingThumb2;
};

}  // namespace mcld

#endif  // TARGET_ARM_THMTOTHMSTUB_H_
