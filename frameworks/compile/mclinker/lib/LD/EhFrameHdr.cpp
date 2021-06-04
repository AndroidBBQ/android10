//===- EhFrameHdr.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/EhFrameHdr.h"

#include "mcld/LD/EhFrame.h"
#include "mcld/LD/LDSection.h"

#include <llvm/Support/Dwarf.h>
#include <llvm/Support/DataTypes.h>

#include <algorithm>
#include <cstring>

namespace mcld {

//===----------------------------------------------------------------------===//
// Helper Function
//===----------------------------------------------------------------------===//
namespace bit32 {

typedef std::pair<SizeTraits<32>::Address, SizeTraits<32>::Address> Entry;

bool EntryCompare(const Entry& pX, const Entry& pY) {
  return (pX.first < pY.first);
}

}  // namespace bit32

//===----------------------------------------------------------------------===//
// Template Specification Functions
//===----------------------------------------------------------------------===//
/// emitOutput<32> - write out eh_frame_hdr
template <>
void EhFrameHdr::emitOutput<32>(FileOutputBuffer& pOutput) {
  MemoryRegion ehframehdr_region =
      pOutput.request(m_EhFrameHdr.offset(), m_EhFrameHdr.size());

  MemoryRegion ehframe_region =
      pOutput.request(m_EhFrame.offset(), m_EhFrame.size());

  uint8_t* data = ehframehdr_region.begin();
  // version
  data[0] = 1;
  // eh_frame_ptr_enc
  data[1] = llvm::dwarf::DW_EH_PE_pcrel | llvm::dwarf::DW_EH_PE_sdata4;

  // eh_frame_ptr
  uint32_t* eh_frame_ptr = reinterpret_cast<uint32_t*>(data + 4);
  *eh_frame_ptr = m_EhFrame.addr() - (m_EhFrameHdr.addr() + 4);

  // fde_count
  uint32_t* fde_count = reinterpret_cast<uint32_t*>(data + 8);
  if (m_EhFrame.hasEhFrame())
    *fde_count = m_EhFrame.getEhFrame()->numOfFDEs();
  else
    *fde_count = 0;

  if (*fde_count == 0) {
    // fde_count_enc
    data[2] = llvm::dwarf::DW_EH_PE_omit;
    // table_enc
    data[3] = llvm::dwarf::DW_EH_PE_omit;
  } else {
    // fde_count_enc
    data[2] = llvm::dwarf::DW_EH_PE_udata4;
    // table_enc
    data[3] = llvm::dwarf::DW_EH_PE_datarel | llvm::dwarf::DW_EH_PE_sdata4;

    // prepare the binary search table
    typedef std::vector<bit32::Entry> SearchTableType;
    SearchTableType search_table;

    for (EhFrame::const_cie_iterator i = m_EhFrame.getEhFrame()->cie_begin(),
                                     e = m_EhFrame.getEhFrame()->cie_end();
         i != e;
         ++i) {
      EhFrame::CIE& cie = **i;
      for (EhFrame::const_fde_iterator fi = cie.begin(), fe = cie.end();
           fi != fe;
           ++fi) {
        EhFrame::FDE& fde = **fi;
        SizeTraits<32>::Offset offset;
        SizeTraits<32>::Address fde_pc;
        SizeTraits<32>::Address fde_addr;
        offset = fde.getOffset();
        fde_pc = computePCBegin(fde, ehframe_region);
        fde_addr = m_EhFrame.addr() + offset;
        search_table.push_back(std::make_pair(fde_pc, fde_addr));
      }
    }

    std::sort(search_table.begin(), search_table.end(), bit32::EntryCompare);

    // write out the binary search table
    uint32_t* bst = reinterpret_cast<uint32_t*>(data + 12);
    SearchTableType::const_iterator entry, entry_end = search_table.end();
    size_t id = 0;
    for (entry = search_table.begin(); entry != entry_end; ++entry) {
      bst[id++] = (*entry).first - m_EhFrameHdr.addr();
      bst[id++] = (*entry).second - m_EhFrameHdr.addr();
    }
  }
}

//===----------------------------------------------------------------------===//
// EhFrameHdr
//===----------------------------------------------------------------------===//

EhFrameHdr::EhFrameHdr(LDSection& pEhFrameHdr, const LDSection& pEhFrame)
    : m_EhFrameHdr(pEhFrameHdr), m_EhFrame(pEhFrame) {
}

EhFrameHdr::~EhFrameHdr() {
}

/// @ref lsb core generic 4.1
/// .eh_frame_hdr section format
/// uint8_t : version
/// uint8_t : eh_frame_ptr_enc
/// uint8_t : fde_count_enc
/// uint8_t : table_enc
/// uint32_t : eh_frame_ptr
/// uint32_t : fde_count
/// __________________________ when fde_count > 0
/// <uint32_t, uint32_t>+ : binary search table
/// sizeOutput - base on the fde count to size output
void EhFrameHdr::sizeOutput() {
  size_t size = 12;
  if (m_EhFrame.hasEhFrame())
    size += 8 * m_EhFrame.getEhFrame()->numOfFDEs();
  m_EhFrameHdr.setSize(size);
}

/// computePCBegin - return the address of FDE's pc
uint32_t EhFrameHdr::computePCBegin(const EhFrame::FDE& pFDE,
                                    const MemoryRegion& pEhFrameRegion) {
  uint8_t fde_encoding = pFDE.getCIE().getFDEEncode();
  unsigned int eh_value = fde_encoding & 0x7;

  // check the size to read in
  if (eh_value == llvm::dwarf::DW_EH_PE_absptr) {
    eh_value = llvm::dwarf::DW_EH_PE_udata4;
  }

  size_t pc_size = 0x0;
  switch (eh_value) {
    case llvm::dwarf::DW_EH_PE_udata2:
      pc_size = 2;
      break;
    case llvm::dwarf::DW_EH_PE_udata4:
      pc_size = 4;
      break;
    case llvm::dwarf::DW_EH_PE_udata8:
      pc_size = 8;
      break;
    default:
      // TODO
      break;
  }

  SizeTraits<32>::Address pc = 0x0;
  const uint8_t* offset = (const uint8_t*)pEhFrameRegion.begin() +
                          pFDE.getOffset() + EhFrame::getDataStartOffset<32>();
  std::memcpy(&pc, offset, pc_size);

  // adjust the signed value
  bool is_signed = (fde_encoding & llvm::dwarf::DW_EH_PE_signed) != 0x0;
  if (llvm::dwarf::DW_EH_PE_udata2 == eh_value && is_signed)
    pc = (pc ^ 0x8000) - 0x8000;

  // handle eh application
  switch (fde_encoding & 0x70) {
    case llvm::dwarf::DW_EH_PE_absptr:
      break;
    case llvm::dwarf::DW_EH_PE_pcrel:
      pc += m_EhFrame.addr() + pFDE.getOffset() +
            EhFrame::getDataStartOffset<32>();
      break;
    case llvm::dwarf::DW_EH_PE_datarel:
      // TODO
      break;
    default:
      // TODO
      break;
  }
  return pc;
}

}  // namespace mcld
