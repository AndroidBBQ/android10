//===- MipsGOT.cpp --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "mcld/LD/ResolveInfo.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Target/OutputRelocSection.h"

#include "MipsGOT.h"
#include "MipsRelocator.h"

#include <llvm/Support/Casting.h>
#include <llvm/Support/ELF.h>

namespace {
const uint32_t Mips32ModulePtr = 1 << 31;
const uint64_t Mips64ModulePtr = 1ull << 63;
const size_t MipsGOT0Num = 2;
const size_t MipsGOTGpOffset = 0x7FF0;
const size_t MipsGOTSize = MipsGOTGpOffset + 0x7FFF;
}

namespace mcld {

//===----------------------------------------------------------------------===//
// MipsGOT::GOTMultipart
//===----------------------------------------------------------------------===//
MipsGOT::GOTMultipart::GOTMultipart(size_t local, size_t global)
    : m_LocalNum(local),
      m_GlobalNum(global),
      m_TLSNum(0),
      m_TLSDynNum(0),
      m_ConsumedLocal(0),
      m_ConsumedGlobal(0),
      m_ConsumedTLS(0),
      m_pLastLocal(NULL),
      m_pLastGlobal(NULL),
      m_pLastTLS(NULL) {
}

bool MipsGOT::GOTMultipart::isConsumed() const {
  return m_LocalNum == m_ConsumedLocal && m_GlobalNum == m_ConsumedGlobal &&
         m_TLSNum == m_ConsumedTLS;
}

void MipsGOT::GOTMultipart::consumeLocal() {
  assert(m_ConsumedLocal < m_LocalNum && "Consumed too many local GOT entries");
  ++m_ConsumedLocal;
  m_pLastLocal = m_pLastLocal->getNextNode();
}

void MipsGOT::GOTMultipart::consumeGlobal() {
  assert(m_ConsumedGlobal < m_GlobalNum &&
         "Consumed too many global GOT entries");
  ++m_ConsumedGlobal;
  m_pLastGlobal = m_pLastGlobal->getNextNode();
}

void MipsGOT::GOTMultipart::consumeTLS(Relocation::Type pType) {
  assert(m_ConsumedTLS < m_TLSNum &&
         "Consumed too many TLS GOT entries");
  m_ConsumedTLS += pType == llvm::ELF::R_MIPS_TLS_GOTTPREL ? 1 : 2;
  m_pLastTLS = m_pLastTLS->getNextNode();
}

//===----------------------------------------------------------------------===//
// MipsGOT::LocalEntry
//===----------------------------------------------------------------------===//
MipsGOT::LocalEntry::LocalEntry(const ResolveInfo* pInfo,
                                Relocation::DWord addend,
                                bool isGot16)
    : m_pInfo(pInfo), m_Addend(addend), m_IsGot16(isGot16) {
}

bool MipsGOT::LocalEntry::operator<(const LocalEntry& O) const {
  if (m_pInfo != O.m_pInfo)
    return m_pInfo < O.m_pInfo;

  if (m_Addend != O.m_Addend)
    return m_Addend < O.m_Addend;

  return m_IsGot16 < O.m_IsGot16;
}

//===----------------------------------------------------------------------===//
// MipsGOT
//===----------------------------------------------------------------------===//
MipsGOT::MipsGOT(LDSection& pSection)
    : GOT(pSection),
      m_pInput(NULL),
      m_HasTLSLdmSymbol(false),
      m_CurrentGOTPart(0),
      m_GotTLSLdmEntry(nullptr) {
}

uint64_t MipsGOT::getGPDispAddress() const {
  return addr() + MipsGOTGpOffset;
}

void MipsGOT::reserve(size_t pNum) {
  for (size_t i = 0; i < pNum; ++i)
    createEntry(0, m_SectionData);
}

bool MipsGOT::hasGOT1() const {
  return !m_MultipartList.empty();
}

bool MipsGOT::hasMultipleGOT() const {
  return m_MultipartList.size() > 1;
}

void MipsGOT::finalizeScanning(OutputRelocSection& pRelDyn) {
  for (MultipartListType::iterator it = m_MultipartList.begin();
       it != m_MultipartList.end();
       ++it) {
    reserveHeader();
    it->m_pLastLocal = &m_SectionData->back();
    reserve(it->m_LocalNum);
    it->m_pLastGlobal = &m_SectionData->back();
    reserve(it->m_GlobalNum);
    it->m_pLastTLS = &m_SectionData->back();
    reserve(it->m_TLSNum);

    if (it == m_MultipartList.begin()) {
      // Reserve entries in the second part of the primary GOT.
      // These entries correspond to the global symbols in all
      // non-primary GOTs.
      reserve(getGlobalNum() - it->m_GlobalNum);
    } else {
      // Reserve reldyn entries for R_MIPS_REL32 relocations
      // for all global entries of secondary GOTs.
      // FIXME: (simon) Do not count local entries for non-pic.
      size_t count = it->m_GlobalNum + it->m_LocalNum;
      for (size_t i = 0; i < count; ++i)
        pRelDyn.reserveEntry();
    }

    for (size_t i = 0; i < it->m_TLSDynNum; ++i)
      pRelDyn.reserveEntry();
  }
}

bool MipsGOT::dynSymOrderCompare(const LDSymbol* pX, const LDSymbol* pY) const {
  SymbolOrderMapType::const_iterator itX = m_SymbolOrderMap.find(pX);
  SymbolOrderMapType::const_iterator itY = m_SymbolOrderMap.find(pY);

  if (itX != m_SymbolOrderMap.end() && itY != m_SymbolOrderMap.end())
    return itX->second < itY->second;

  return itX == m_SymbolOrderMap.end() && itY != m_SymbolOrderMap.end();
}

void MipsGOT::initGOTList() {
  m_SymbolOrderMap.clear();

  m_MultipartList.clear();
  m_MultipartList.push_back(GOTMultipart());

  m_MultipartList.back().m_Inputs.insert(m_pInput);

  m_MergedGlobalSymbols.clear();
  m_InputGlobalSymbols.clear();
  m_MergedLocalSymbols.clear();
  m_InputLocalSymbols.clear();
  m_InputTLSGdSymbols.clear();
  m_HasTLSLdmSymbol = false;
}

void MipsGOT::changeInput() {
  m_MultipartList.back().m_Inputs.insert(m_pInput);

  for (LocalSymbolSetType::iterator it = m_InputLocalSymbols.begin(),
                                    end = m_InputLocalSymbols.end();
       it != end;
       ++it)
    m_MergedLocalSymbols.insert(*it);

  m_InputLocalSymbols.clear();

  for (SymbolUniqueMapType::iterator it = m_InputGlobalSymbols.begin(),
                                     end = m_InputGlobalSymbols.end();
       it != end;
       ++it)
    m_MergedGlobalSymbols.insert(it->first);

  m_InputGlobalSymbols.clear();
}

bool MipsGOT::isGOTFull() const {
  uint64_t gotCount = MipsGOT0Num + m_MultipartList.back().m_LocalNum +
                      m_MultipartList.back().m_GlobalNum;

  gotCount += 1;

  return gotCount * getEntrySize() > MipsGOTSize;
}

void MipsGOT::split() {
  m_MergedLocalSymbols.clear();
  m_MergedGlobalSymbols.clear();

  size_t uniqueCount = 0;
  for (SymbolUniqueMapType::const_iterator it = m_InputGlobalSymbols.begin(),
                                           end = m_InputGlobalSymbols.end();
       it != end;
       ++it) {
    if (it->second)
      ++uniqueCount;
  }

  m_MultipartList.back().m_LocalNum -= m_InputLocalSymbols.size();
  m_MultipartList.back().m_GlobalNum -= uniqueCount;
  m_MultipartList.back().m_Inputs.erase(m_pInput);

  m_MultipartList.push_back(
      GOTMultipart(m_InputLocalSymbols.size(), m_InputGlobalSymbols.size()));
  m_MultipartList.back().m_Inputs.insert(m_pInput);
}

void MipsGOT::initializeScan(const Input& pInput) {
  if (m_pInput == NULL) {
    m_pInput = &pInput;
    initGOTList();
  } else {
    m_pInput = &pInput;
    changeInput();
  }
}

void MipsGOT::finalizeScan(const Input& pInput) {
}

bool MipsGOT::reserveLocalEntry(ResolveInfo& pInfo,
                                int reloc,
                                Relocation::DWord pAddend) {
  LocalEntry entry(&pInfo, pAddend, reloc == llvm::ELF::R_MIPS_GOT16);

  if (m_InputLocalSymbols.count(entry))
    // Do nothing, if we have seen this symbol
    // in the current input already.
    return false;

  if (m_MergedLocalSymbols.count(entry)) {
    // We have seen this symbol in previous inputs.
    // Remember that it exists in the current input too.
    m_InputLocalSymbols.insert(entry);
    return false;
  }

  if (isGOTFull())
    split();

  m_InputLocalSymbols.insert(entry);

  ++m_MultipartList.back().m_LocalNum;
  return true;
}

bool MipsGOT::reserveGlobalEntry(ResolveInfo& pInfo) {
  if (m_InputGlobalSymbols.count(&pInfo))
    return false;

  if (m_MergedGlobalSymbols.count(&pInfo)) {
    m_InputGlobalSymbols[&pInfo] = false;
    return false;
  }

  if (isGOTFull())
    split();

  m_InputGlobalSymbols[&pInfo] = true;
  ++m_MultipartList.back().m_GlobalNum;

  if (!(pInfo.reserved() & MipsRelocator::ReserveGot)) {
    m_SymbolOrderMap[pInfo.outSymbol()] = m_SymbolOrderMap.size();
    pInfo.setReserved(pInfo.reserved() | MipsRelocator::ReserveGot);
  }

  return true;
}

bool MipsGOT::reserveTLSGdEntry(ResolveInfo& pInfo) {
  if (m_InputTLSGdSymbols.count(&pInfo))
    return false;

  m_InputTLSGdSymbols.insert(&pInfo);
  m_MultipartList.back().m_TLSNum += 2;
  m_MultipartList.back().m_TLSDynNum += 2;

  return true;
}

bool MipsGOT::reserveTLSLdmEntry() {
  if (m_HasTLSLdmSymbol)
    return false;

  m_HasTLSLdmSymbol = true;
  m_MultipartList.back().m_TLSNum += 2;
  m_MultipartList.back().m_TLSDynNum += 1;

  return true;
}

bool MipsGOT::reserveTLSGotEntry(ResolveInfo& pInfo) {
  if (m_InputTLSGotSymbols.count(&pInfo))
    return false;

  m_InputTLSGotSymbols.insert(&pInfo);
  m_MultipartList.back().m_TLSNum += 1;
  m_MultipartList.back().m_TLSDynNum += 1;

  return true;
}

bool MipsGOT::isPrimaryGOTConsumed() {
  return m_CurrentGOTPart > 0;
}

Fragment* MipsGOT::consumeLocal() {
  assert(m_CurrentGOTPart < m_MultipartList.size() &&
         "GOT number is out of range!");

  if (m_MultipartList[m_CurrentGOTPart].isConsumed())
    ++m_CurrentGOTPart;

  m_MultipartList[m_CurrentGOTPart].consumeLocal();

  return m_MultipartList[m_CurrentGOTPart].m_pLastLocal;
}

Fragment* MipsGOT::consumeGlobal() {
  assert(m_CurrentGOTPart < m_MultipartList.size() &&
         "GOT number is out of range!");

  if (m_MultipartList[m_CurrentGOTPart].isConsumed())
    ++m_CurrentGOTPart;

  m_MultipartList[m_CurrentGOTPart].consumeGlobal();

  return m_MultipartList[m_CurrentGOTPart].m_pLastGlobal;
}

Fragment* MipsGOT::consumeTLS(Relocation::Type pType) {
  assert(m_CurrentGOTPart < m_MultipartList.size() &&
         "GOT number is out of range!");

  if (m_MultipartList[m_CurrentGOTPart].isConsumed())
    ++m_CurrentGOTPart;

  m_MultipartList[m_CurrentGOTPart].consumeTLS(pType);

  return m_MultipartList[m_CurrentGOTPart].m_pLastTLS;
}

uint64_t MipsGOT::getGPAddr(const Input& pInput) const {
  uint64_t gotSize = 0;
  for (MultipartListType::const_iterator it = m_MultipartList.begin(),
                                         ie = m_MultipartList.end();
       it != ie;
       ++it) {
    if (it->m_Inputs.count(&pInput))
      break;

    gotSize += (MipsGOT0Num + it->m_LocalNum + it->m_GlobalNum);
    if (it == m_MultipartList.begin())
      gotSize += getGlobalNum() - it->m_GlobalNum;
  }

  return addr() + gotSize * getEntrySize() + MipsGOTGpOffset;
}

uint64_t MipsGOT::getGPRelOffset(const Input& pInput,
                                 const Fragment& pEntry) const {
  return addr() + pEntry.getOffset() - getGPAddr(pInput);
}

void MipsGOT::recordGlobalEntry(const ResolveInfo* pInfo, Fragment* pEntry) {
  GotEntryKey key;
  key.m_GOTPage = m_CurrentGOTPart;
  key.m_pInfo = pInfo;
  key.m_Addend = 0;
  m_GotGlobalEntriesMap[key] = pEntry;
}

Fragment* MipsGOT::lookupGlobalEntry(const ResolveInfo* pInfo) {
  GotEntryKey key;
  key.m_GOTPage = m_CurrentGOTPart;
  key.m_pInfo = pInfo;
  key.m_Addend = 0;
  GotEntryMapType::iterator it = m_GotGlobalEntriesMap.find(key);

  if (it == m_GotGlobalEntriesMap.end())
    return NULL;

  return it->second;
}

void MipsGOT::recordTLSEntry(const ResolveInfo* pInfo, Fragment* pEntry,
                             Relocation::Type pType) {
  if (pType == llvm::ELF::R_MIPS_TLS_LDM) {
    m_GotTLSLdmEntry = pEntry;
  } else if (pType == llvm::ELF::R_MIPS_TLS_GD) {
    GotEntryKey key;
    key.m_GOTPage = m_CurrentGOTPart;
    key.m_pInfo = pInfo;
    key.m_Addend = 0;
    m_GotTLSGdEntriesMap[key] = pEntry;
  } else if (pType == llvm::ELF::R_MIPS_TLS_GOTTPREL) {
    GotEntryKey key;
    key.m_GOTPage = m_CurrentGOTPart;
    key.m_pInfo = pInfo;
    key.m_Addend = 0;
    m_GotTLSGotEntriesMap[key] = pEntry;
  } else {
    llvm_unreachable("Unexpected relocation");
  }
}

Fragment* MipsGOT::lookupTLSEntry(const ResolveInfo* pInfo,
                                  Relocation::Type pType) {
  if (pType == llvm::ELF::R_MIPS_TLS_LDM)
    return m_GotTLSLdmEntry;
  if (pType == llvm::ELF::R_MIPS_TLS_GD) {
    GotEntryKey key;
    key.m_GOTPage = m_CurrentGOTPart;
    key.m_pInfo = pInfo;
    key.m_Addend = 0;
    GotEntryMapType::iterator it = m_GotTLSGdEntriesMap.find(key);
    return it == m_GotTLSGdEntriesMap.end() ? nullptr : it->second;
  }
  if (pType == llvm::ELF::R_MIPS_TLS_GOTTPREL) {
    GotEntryKey key;
    key.m_GOTPage = m_CurrentGOTPart;
    key.m_pInfo = pInfo;
    key.m_Addend = 0;
    GotEntryMapType::iterator it = m_GotTLSGotEntriesMap.find(key);
    return it == m_GotTLSGotEntriesMap.end() ? nullptr : it->second;
  }
  llvm_unreachable("Unexpected relocation");
}

void MipsGOT::recordLocalEntry(const ResolveInfo* pInfo,
                               Relocation::DWord pAddend,
                               Fragment* pEntry) {
  GotEntryKey key;
  key.m_GOTPage = m_CurrentGOTPart;
  key.m_pInfo = pInfo;
  key.m_Addend = pAddend;
  m_GotLocalEntriesMap[key] = pEntry;
}

Fragment* MipsGOT::lookupLocalEntry(const ResolveInfo* pInfo,
                                    Relocation::DWord pAddend) {
  GotEntryKey key;
  key.m_GOTPage = m_CurrentGOTPart;
  key.m_pInfo = pInfo;
  key.m_Addend = pAddend;
  GotEntryMapType::iterator it = m_GotLocalEntriesMap.find(key);

  if (it == m_GotLocalEntriesMap.end())
    return NULL;

  return it->second;
}

size_t MipsGOT::getLocalNum() const {
  assert(!m_MultipartList.empty() && "GOT is empty!");
  return m_MultipartList[0].m_LocalNum + MipsGOT0Num;
}

size_t MipsGOT::getGlobalNum() const {
  return m_SymbolOrderMap.size();
}

//===----------------------------------------------------------------------===//
// Mips32GOT
//===----------------------------------------------------------------------===//
Mips32GOT::Mips32GOT(LDSection& pSection) : MipsGOT(pSection) {
}

void Mips32GOT::setEntryValue(Fragment* entry, uint64_t pValue) {
  llvm::cast<Mips32GOTEntry>(entry)->setValue(pValue);
}

uint64_t Mips32GOT::emit(MemoryRegion& pRegion) {
  uint32_t* buffer = reinterpret_cast<uint32_t*>(pRegion.begin());

  uint64_t result = 0;
  for (iterator it = begin(), ie = end(); it != ie; ++it, ++buffer) {
    Mips32GOTEntry* got = &(llvm::cast<Mips32GOTEntry>((*it)));
    *buffer = static_cast<uint32_t>(got->getValue());
    result += got->size();
  }
  return result;
}

Fragment* Mips32GOT::createEntry(uint64_t pValue, SectionData* pParent) {
  return new Mips32GOTEntry(pValue, pParent);
}

size_t Mips32GOT::getEntrySize() const {
  return Mips32GOTEntry::EntrySize;
}

void Mips32GOT::reserveHeader() {
  createEntry(0, m_SectionData);
  createEntry(Mips32ModulePtr, m_SectionData);
}

//===----------------------------------------------------------------------===//
// Mips64GOT
//===----------------------------------------------------------------------===//
Mips64GOT::Mips64GOT(LDSection& pSection) : MipsGOT(pSection) {
}

void Mips64GOT::setEntryValue(Fragment* entry, uint64_t pValue) {
  llvm::cast<Mips64GOTEntry>(entry)->setValue(pValue);
}

uint64_t Mips64GOT::emit(MemoryRegion& pRegion) {
  uint64_t* buffer = reinterpret_cast<uint64_t*>(pRegion.begin());

  uint64_t result = 0;
  for (iterator it = begin(), ie = end(); it != ie; ++it, ++buffer) {
    Mips64GOTEntry* got = &(llvm::cast<Mips64GOTEntry>((*it)));
    *buffer = static_cast<uint64_t>(got->getValue());
    result += got->size();
  }
  return result;
}

Fragment* Mips64GOT::createEntry(uint64_t pValue, SectionData* pParent) {
  return new Mips64GOTEntry(pValue, pParent);
}

size_t Mips64GOT::getEntrySize() const {
  return Mips64GOTEntry::EntrySize;
}

void Mips64GOT::reserveHeader() {
  createEntry(0, m_SectionData);
  createEntry(Mips64ModulePtr, m_SectionData);
}

}  // namespace mcld
