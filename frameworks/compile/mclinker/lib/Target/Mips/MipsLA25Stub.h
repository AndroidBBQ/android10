//===- MipsLA25Stub.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef TARGET_MIPS_MIPSLA25STUB_H_
#define TARGET_MIPS_MIPSLA25STUB_H_

#include "mcld/Fragment/Stub.h"

namespace mcld {

class MipsGNULDBackend;
class Relocation;

//===----------------------------------------------------------------------===//
// MipsLA25Stub
//===----------------------------------------------------------------------===//
/** \class MipsLA25Stub
 *  \brief Mips stub for a non-PIC interface to a PIC function.
 */
class MipsLA25Stub : public Stub {
 public:
  explicit MipsLA25Stub(const MipsGNULDBackend& pTarget);

 private:
  // Stub
  Stub* doClone();
  bool isMyDuty(const Relocation& pReloc,
                uint64_t pSource,
                uint64_t pTargetSymValue) const;
  const std::string& name() const;
  const uint8_t* getContent() const;
  size_t size() const;
  size_t alignment() const;

 private:
  MipsLA25Stub(const MipsLA25Stub&);
  MipsLA25Stub& operator=(const MipsLA25Stub&);

  MipsLA25Stub(const MipsGNULDBackend& pTarget,
               const uint32_t* pData,
               size_t pSize,
               const_fixup_iterator pBegin,
               const_fixup_iterator pEnd);

 private:
  const MipsGNULDBackend& m_Target;
  const std::string m_Name;
  const uint32_t* m_pData;
  const size_t m_Size;
};

}  // namespace mcld

#endif  // TARGET_MIPS_MIPSLA25STUB_H_
