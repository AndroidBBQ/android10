//===- X86PLT.cpp ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "X86GOTPLT.h"
#include "X86PLT.h"

#include "mcld/LD/LDSection.h"
#include "mcld/LinkerConfig.h"
#include "mcld/Support/MsgHandling.h"

#include <llvm/Support/ELF.h>
#include <llvm/Support/Casting.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// PLT entry data
//===----------------------------------------------------------------------===//
X86_32DynPLT0::X86_32DynPLT0(SectionData& pParent)
    : PLT::Entry<sizeof(x86_32_dyn_plt0)>(pParent) {
}

X86_32DynPLT1::X86_32DynPLT1(SectionData& pParent)
    : PLT::Entry<sizeof(x86_32_dyn_plt1)>(pParent) {
}

X86_32ExecPLT0::X86_32ExecPLT0(SectionData& pParent)
    : PLT::Entry<sizeof(x86_32_exec_plt0)>(pParent) {
}

X86_32ExecPLT1::X86_32ExecPLT1(SectionData& pParent)
    : PLT::Entry<sizeof(x86_32_exec_plt1)>(pParent) {
}

X86_64PLT0::X86_64PLT0(SectionData& pParent)
    : PLT::Entry<sizeof(x86_64_plt0)>(pParent) {
}

X86_64PLT1::X86_64PLT1(SectionData& pParent)
    : PLT::Entry<sizeof(x86_64_plt1)>(pParent) {
}

//===----------------------------------------------------------------------===//
// X86PLT
//===----------------------------------------------------------------------===//
X86PLT::X86PLT(LDSection& pSection, const LinkerConfig& pConfig, int got_size)
    : PLT(pSection), m_Config(pConfig) {
  assert(LinkerConfig::DynObj == m_Config.codeGenType() ||
         LinkerConfig::Exec == m_Config.codeGenType() ||
         LinkerConfig::Binary == m_Config.codeGenType());

  if (got_size == 32) {
    if (LinkerConfig::DynObj == m_Config.codeGenType()) {
      m_PLT0 = x86_32_dyn_plt0;
      m_PLT1 = x86_32_dyn_plt1;
      m_PLT0Size = sizeof(x86_32_dyn_plt0);
      m_PLT1Size = sizeof(x86_32_dyn_plt1);
      // create PLT0
      new X86_32DynPLT0(*m_pSectionData);
    } else {
      m_PLT0 = x86_32_exec_plt0;
      m_PLT1 = x86_32_exec_plt1;
      m_PLT0Size = sizeof(x86_32_exec_plt0);
      m_PLT1Size = sizeof(x86_32_exec_plt1);
      // create PLT0
      new X86_32ExecPLT0(*m_pSectionData);
    }
  } else {
    assert(got_size == 64);
    m_PLT0 = x86_64_plt0;
    m_PLT1 = x86_64_plt1;
    m_PLT0Size = sizeof(x86_64_plt0);
    m_PLT1Size = sizeof(x86_64_plt1);
    // create PLT0
    new X86_64PLT0(*m_pSectionData);
  }
}

X86PLT::~X86PLT() {
}

void X86PLT::finalizeSectionSize() {
  uint64_t size = 0;
  // plt0 size
  size = getPLT0()->size();

  // get first plt1 entry
  X86PLT::iterator it = begin();
  ++it;
  if (end() != it) {
    // plt1 size
    PLTEntryBase* plt1 = &(llvm::cast<PLTEntryBase>(*it));
    size += (m_pSectionData->size() - 1) * plt1->size();
  }
  m_Section.setSize(size);

  uint32_t offset = 0;
  SectionData::iterator frag, fragEnd = m_pSectionData->end();
  for (frag = m_pSectionData->begin(); frag != fragEnd; ++frag) {
    frag->setOffset(offset);
    offset += frag->size();
  }
}

bool X86PLT::hasPLT1() const {
  return (m_pSectionData->size() > 1);
}

PLTEntryBase* X86PLT::create() {
  if (LinkerConfig::DynObj == m_Config.codeGenType())
    return new X86_32DynPLT1(*m_pSectionData);
  else
    return new X86_32ExecPLT1(*m_pSectionData);
}

PLTEntryBase* X86PLT::getPLT0() const {
  iterator first = m_pSectionData->getFragmentList().begin();

  assert(first != m_pSectionData->getFragmentList().end() &&
         "FragmentList is empty, getPLT0 failed!");

  PLTEntryBase* plt0 = &(llvm::cast<PLTEntryBase>(*first));

  return plt0;
}

//===----------------------------------------------------------------------===//
// X86_32PLT
//===----------------------------------------------------------------------===//
X86_32PLT::X86_32PLT(LDSection& pSection,
                     X86_32GOTPLT& pGOTPLT,
                     const LinkerConfig& pConfig)
    : X86PLT(pSection, pConfig, 32), m_GOTPLT(pGOTPLT) {
}

// FIXME: It only works on little endian machine.
void X86_32PLT::applyPLT0() {
  PLTEntryBase* plt0 = getPLT0();

  unsigned char* data = 0;
  data = static_cast<unsigned char*>(malloc(plt0->size()));

  if (!data)
    fatal(diag::fail_allocate_memory_plt);

  memcpy(data, m_PLT0, plt0->size());

  if (m_PLT0 == x86_32_exec_plt0) {
    uint32_t* offset = reinterpret_cast<uint32_t*>(data + 2);
    *offset = m_GOTPLT.addr() + 4;
    offset = reinterpret_cast<uint32_t*>(data + 8);
    *offset = m_GOTPLT.addr() + 8;
  }

  plt0->setValue(data);
}

// FIXME: It only works on little endian machine.
void X86_32PLT::applyPLT1() {
  assert(m_Section.addr() && ".plt base address is NULL!");

  X86PLT::iterator it = m_pSectionData->begin();
  X86PLT::iterator ie = m_pSectionData->end();
  assert(it != ie && "FragmentList is empty, applyPLT1 failed!");

  uint64_t GOTEntrySize = X86_32GOTEntry::EntrySize;

  // Skip GOT0
  uint64_t GOTEntryOffset = GOTEntrySize * X86GOTPLT0Num;
  if (LinkerConfig::Exec == m_Config.codeGenType())
    GOTEntryOffset += m_GOTPLT.addr();

  // skip PLT0
  uint64_t PLTEntryOffset = m_PLT0Size;
  ++it;

  PLTEntryBase* plt1 = 0;

  uint64_t PLTRelOffset = 0;

  while (it != ie) {
    plt1 = &(llvm::cast<PLTEntryBase>(*it));
    unsigned char* data;
    data = static_cast<unsigned char*>(malloc(plt1->size()));

    if (!data)
      fatal(diag::fail_allocate_memory_plt);

    memcpy(data, m_PLT1, plt1->size());

    uint32_t* offset;

    offset = reinterpret_cast<uint32_t*>(data + 2);
    *offset = GOTEntryOffset;
    GOTEntryOffset += GOTEntrySize;

    offset = reinterpret_cast<uint32_t*>(data + 7);
    *offset = PLTRelOffset;
    PLTRelOffset += sizeof(llvm::ELF::Elf32_Rel);

    offset = reinterpret_cast<uint32_t*>(data + 12);
    *offset = -(PLTEntryOffset + 12 + 4);
    PLTEntryOffset += m_PLT1Size;

    plt1->setValue(data);
    ++it;
  }
}

//===----------------------------------------------------------------------===//
// X86_64PLT
//===----------------------------------------------------------------------===//
X86_64PLT::X86_64PLT(LDSection& pSection,
                     X86_64GOTPLT& pGOTPLT,
                     const LinkerConfig& pConfig)
    : X86PLT(pSection, pConfig, 64), m_GOTPLT(pGOTPLT) {
}

// FIXME: It only works on little endian machine.
void X86_64PLT::applyPLT0() {
  PLTEntryBase* plt0 = getPLT0();

  unsigned char* data = 0;
  data = static_cast<unsigned char*>(malloc(plt0->size()));

  if (!data)
    fatal(diag::fail_allocate_memory_plt);

  memcpy(data, m_PLT0, plt0->size());

  // pushq GOT + 8(%rip)
  uint32_t* offset = reinterpret_cast<uint32_t*>(data + 2);
  *offset = m_GOTPLT.addr() - addr() + 8 - 6;
  // jmq *GOT + 16(%rip)
  offset = reinterpret_cast<uint32_t*>(data + 8);
  *offset = m_GOTPLT.addr() - addr() + 16 - 12;

  plt0->setValue(data);
}

// FIXME: It only works on little endian machine.
void X86_64PLT::applyPLT1() {
  assert(m_Section.addr() && ".plt base address is NULL!");

  X86PLT::iterator it = m_pSectionData->begin();
  X86PLT::iterator ie = m_pSectionData->end();
  assert(it != ie && "FragmentList is empty, applyPLT1 failed!");

  uint64_t GOTEntrySize = X86_64GOTEntry::EntrySize;

  // compute sym@GOTPCREL of the PLT1 entry.
  uint64_t SymGOTPCREL = m_GOTPLT.addr();

  // Skip GOT0
  SymGOTPCREL += GOTEntrySize * X86GOTPLT0Num;

  // skip PLT0
  uint64_t PLTEntryOffset = m_PLT0Size;
  ++it;

  // PC-relative to entry in PLT section.
  SymGOTPCREL -= addr() + PLTEntryOffset + 6;

  PLTEntryBase* plt1 = 0;

  uint64_t PLTRelIndex = 0;

  while (it != ie) {
    plt1 = &(llvm::cast<PLTEntryBase>(*it));
    unsigned char* data;
    data = static_cast<unsigned char*>(malloc(plt1->size()));

    if (!data)
      fatal(diag::fail_allocate_memory_plt);

    memcpy(data, m_PLT1, plt1->size());

    uint32_t* offset;

    // jmpq *sym@GOTPCREL(%rip)
    offset = reinterpret_cast<uint32_t*>(data + 2);
    *offset = SymGOTPCREL;
    SymGOTPCREL += GOTEntrySize - m_PLT1Size;

    // pushq $index
    offset = reinterpret_cast<uint32_t*>(data + 7);
    *offset = PLTRelIndex;
    PLTRelIndex++;

    // jmpq plt0
    offset = reinterpret_cast<uint32_t*>(data + 12);
    *offset = -(PLTEntryOffset + 12 + 4);
    PLTEntryOffset += m_PLT1Size;

    plt1->setValue(data);
    ++it;
  }
}

}  // namespace mcld
