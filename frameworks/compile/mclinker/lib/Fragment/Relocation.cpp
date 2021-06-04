//===- Relocation.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Fragment/Relocation.h"

#include "mcld/LD/LDSection.h"
#include "mcld/LD/LDSymbol.h"
#include "mcld/LD/RelocationFactory.h"
#include "mcld/LD/Relocator.h"
#include "mcld/LD/ResolveInfo.h"
#include "mcld/LD/SectionData.h"
#include "mcld/Support/MsgHandling.h"

#include <llvm/Support/ManagedStatic.h>

namespace mcld {

static llvm::ManagedStatic<RelocationFactory> g_RelocationFactory;

//===----------------------------------------------------------------------===//
// Relocation Factory Methods
//===----------------------------------------------------------------------===//
/// Initialize - set up the relocation factory
void Relocation::SetUp(const LinkerConfig& pConfig) {
  g_RelocationFactory->setConfig(pConfig);
}

/// Clear - Clean up the relocation factory
void Relocation::Clear() {
  g_RelocationFactory->clear();
}

/// Create - produce an empty relocation entry
Relocation* Relocation::Create() {
  return g_RelocationFactory->produceEmptyEntry();
}

/// Create - produce a relocation entry
/// @param pType    [in] the type of the relocation entry
/// @param pFragRef [in] the place to apply the relocation
/// @param pAddend  [in] the addend of the relocation entry
Relocation* Relocation::Create(Type pType,
                               FragmentRef& pFragRef,
                               Address pAddend) {
  return g_RelocationFactory->produce(pType, pFragRef, pAddend);
}

/// Destroy - destroy a relocation entry
void Relocation::Destroy(Relocation*& pRelocation) {
  g_RelocationFactory->destroy(pRelocation);
  pRelocation = NULL;
}

//===----------------------------------------------------------------------===//
// Relocation
//===----------------------------------------------------------------------===//
Relocation::Relocation()
    : m_Type(0x0), m_TargetData(0x0), m_pSymInfo(NULL), m_Addend(0x0) {
}

Relocation::Relocation(Relocation::Type pType,
                       FragmentRef* pTargetRef,
                       Relocation::Address pAddend,
                       Relocation::DWord pTargetData)
    : m_Type(pType),
      m_TargetData(pTargetData),
      m_pSymInfo(NULL),
      m_Addend(pAddend) {
  if (pTargetRef != NULL)
    m_TargetAddress.assign(*pTargetRef->frag(), pTargetRef->offset());
}

Relocation::~Relocation() {
}

Relocation::Address Relocation::place() const {
  Address sect_addr = m_TargetAddress.frag()->getParent()->getSection().addr();
  return sect_addr + m_TargetAddress.getOutputOffset();
}

Relocation::Address Relocation::symValue() const {
  if (m_pSymInfo->type() == ResolveInfo::Section &&
      m_pSymInfo->outSymbol()->hasFragRef()) {
    const FragmentRef* fragRef = m_pSymInfo->outSymbol()->fragRef();
    return fragRef->frag()->getParent()->getSection().addr() +
           fragRef->getOutputOffset();
  }
  return m_pSymInfo->outSymbol()->value();
}

void Relocation::apply(Relocator& pRelocator) {
  Relocator::Result result = pRelocator.applyRelocation(*this);

  switch (result) {
    case Relocator::OK: {
      // do nothing
      return;
    }
    case Relocator::Overflow: {
      error(diag::result_overflow) << pRelocator.getName(type())
                                   << symInfo()->name();
      return;
    }
    case Relocator::BadReloc: {
      error(diag::result_badreloc) << pRelocator.getName(type())
                                   << symInfo()->name();
      return;
    }
    case Relocator::Unsupported: {
      fatal(diag::unsupported_relocation) << type()
                                          << "mclinker@googlegroups.com";
      return;
    }
    case Relocator::Unknown: {
      fatal(diag::unknown_relocation) << type() << symInfo()->name();
      return;
    }
  }  // end of switch
}

void Relocation::setType(Type pType) {
  m_Type = pType;
}

void Relocation::setAddend(Address pAddend) {
  m_Addend = pAddend;
}

void Relocation::setSymInfo(ResolveInfo* pSym) {
  m_pSymInfo = pSym;
}

Relocation::Size Relocation::size(Relocator& pRelocator) const {
  return pRelocator.getSize(m_Type);
}

void Relocation::updateAddend() {
  // Update value keep in addend if we meet a section symbol
  if (m_pSymInfo->type() == ResolveInfo::Section) {
    uint32_t offset = m_pSymInfo->outSymbol()->fragRef()->getOutputOffset();
    m_Addend += offset;
  }
}

}  // namespace mcld
