//===- EhFrameReader.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/EhFrameReader.h"

#include "mcld/Fragment/NullFragment.h"
#include "mcld/MC/Input.h"
#include "mcld/LD/LDSection.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Support/MemoryArea.h"

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Dwarf.h>
#include <llvm/Support/LEB128.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// Helper Functions
//===----------------------------------------------------------------------===//
/// skip_LEB128 - skip the first LEB128 encoded value from *pp, update *pp
/// to the next character.
/// @return - false if we ran off the end of the string.
static bool skip_LEB128(EhFrameReader::ConstAddress* pp,
                        EhFrameReader::ConstAddress pend) {
  for (EhFrameReader::ConstAddress p = *pp; p < pend; ++p) {
    if ((*p & 0x80) == 0x0) {
      *pp = p + 1;
      return true;
    }
  }
  return false;
}

//===----------------------------------------------------------------------===//
// EhFrameReader
//===----------------------------------------------------------------------===//
template <>
EhFrameReader::Token EhFrameReader::scan<true>(ConstAddress pHandler,
                                               uint64_t pOffset,
                                               llvm::StringRef pData) const {
  Token result;
  result.file_off = pOffset;

  const uint32_t* data = (const uint32_t*)pHandler;
  size_t cur_idx = 0;

  // Length Field
  uint32_t length = data[cur_idx++];
  if (length == 0x0) {
    // terminator
    result.kind = Terminator;
    result.data_off = 4;
    result.size = 4;
    return result;
  }

  // Extended Field
  uint64_t extended = 0x0;
  if (length == 0xFFFFFFFF) {
    extended = data[cur_idx++];
    extended <<= 32;
    extended |= data[cur_idx++];
    result.size = extended + 12;
    result.data_off = 16;
    // 64-bit obj file still uses 32-bit eh_frame.
    assert(false && "We don't support 64-bit eh_frame.");
  } else {
    result.size = length + 4;
    result.data_off = 8;
  }

  // ID Field
  uint32_t ID = data[cur_idx++];
  if (ID == 0x0)
    result.kind = CIE;
  else
    result.kind = FDE;

  return result;
}

template <>
bool EhFrameReader::read<32, true>(Input& pInput, EhFrame& pEhFrame) {
  // Alphabet:
  //   {CIE, FDE, CIEt}
  //
  // Regular Expression:
  //   (CIE FDE*)+ CIEt
  //
  // Autometa:
  //   S = {Q0, Q1, Q2}, Start = Q0, Accept = Q2
  //
  //              FDE
  //             +---+
  //        CIE   \ /   CIEt
  //   Q0 -------> Q1 -------> Q2
  //    |         / \           ^
  //    |        +---+          |
  //    |         CIE           |
  //    +-----------------------+
  //              CIEt
  const State autometa[NumOfStates][NumOfTokenKinds] = {
      //     CIE     FDE    Term  Unknown
      {Q1, Reject, Accept, Reject},  // Q0
      {Q1, Q1, Accept, Reject},      // Q1
  };

  const Action transition[NumOfStates][NumOfTokenKinds] = {
      /*    CIE     FDE     Term Unknown */
      {addCIE, reject, addTerm, reject},  // Q0
      {addCIE, addFDE, addTerm, reject},  // Q1
  };

  LDSection& section = pEhFrame.getSection();
  if (section.size() == 0x0) {
    NullFragment* frag = new NullFragment();
    pEhFrame.addFragment(*frag);
    return true;
  }

  // get file offset and address
  uint64_t file_off = pInput.fileOffset() + section.offset();
  llvm::StringRef sect_reg =
      pInput.memArea()->request(file_off, section.size());
  ConstAddress handler = (ConstAddress)sect_reg.begin();

  State cur_state = Q0;
  while (Reject != cur_state && Accept != cur_state) {
    Token token = scan<true>(handler, file_off, sect_reg);
    llvm::StringRef entry =
        pInput.memArea()->request(token.file_off, token.size);

    if (!transition[cur_state][token.kind](pEhFrame, entry, token)) {
      // fail to scan
      debug(diag::debug_cannot_scan_eh) << pInput.name();
      return false;
    }

    file_off += token.size;
    handler += token.size;

    if (handler == sect_reg.end()) {
      cur_state = Accept;
    } else if (handler > sect_reg.end()) {
      cur_state = Reject;
    } else {
      cur_state = autometa[cur_state][token.kind];
    }
  }  // end of while

  if (Reject == cur_state) {
    // fail to parse
    debug(diag::debug_cannot_parse_eh) << pInput.name();
    return false;
  }
  return true;
}

bool EhFrameReader::addCIE(EhFrame& pEhFrame,
                           llvm::StringRef pRegion,
                           const EhFrameReader::Token& pToken) {
  // skip Length, Extended Length and CIE ID.
  ConstAddress handler = pRegion.begin() + pToken.data_off;
  ConstAddress cie_end = pRegion.end();
  ConstAddress handler_start = handler;
  uint64_t pr_ptr_data_offset = pToken.data_off;

  // the version should be 1 or 3
  uint8_t version = *handler++;
  if (version != 1 && version != 3) {
    return false;
  }

  // Set up the Augumentation String
  ConstAddress aug_str_front = handler;
  ConstAddress aug_str_back = static_cast<ConstAddress>(
      memchr(aug_str_front, '\0', cie_end - aug_str_front));
  if (aug_str_back == NULL) {
    return false;
  }

  // skip the Augumentation String field
  handler = aug_str_back + 1;

  // skip the Code Alignment Factor
  if (!skip_LEB128(&handler, cie_end)) {
    return false;
  }
  // skip the Data Alignment Factor
  if (!skip_LEB128(&handler, cie_end)) {
    return false;
  }
  // skip the Return Address Register
  if (version == 1) {
    if (cie_end - handler < 1)
      return false;
    ++handler;
  } else {
    if (!skip_LEB128(&handler, cie_end))
      return false;
  }

  llvm::StringRef augment((const char*)aug_str_front);

  // we discard this CIE if the augumentation string is '\0'
  if (augment.size() == 0) {
    EhFrame::CIE* cie = new EhFrame::CIE(pRegion);
    cie->setFDEEncode(llvm::dwarf::DW_EH_PE_absptr);
    pEhFrame.addCIE(*cie);
    pEhFrame.getCIEMap().insert(std::make_pair(pToken.file_off, cie));
    return true;
  }

  // the Augmentation String start with 'eh' is a CIE from gcc before 3.0,
  // in LSB Core Spec 3.0RC1. We do not support it.
  if (augment.size() > 1 && augment[0] == 'e' && augment[1] == 'h') {
    return false;
  }

  // parse the Augmentation String to get the FDE encodeing if 'z' existed
  uint8_t fde_encoding = llvm::dwarf::DW_EH_PE_absptr;
  std::string augdata;
  std::string pr_ptr_data;
  if (augment[0] == 'z') {
    unsigned offset;
    size_t augdata_size = llvm::decodeULEB128((const uint8_t*)handler, &offset);
    handler += offset;
    augdata = std::string((const char*)handler, augdata_size);

    // parse the Augmentation String
    for (size_t i = 1; i < augment.size(); ++i) {
      switch (augment[i]) {
        // LDSA encoding (1 byte)
        case 'L': {
          if (cie_end - handler < 1) {
            return false;
          }
          ++handler;
          break;
        }
        // Two arguments, the first one represents the encoding of the second
        // argument (1 byte). The second one is the address of personality
        // routine.
        case 'P': {
          // the first argument
          if (cie_end - handler < 1) {
            return false;
          }
          uint8_t per_encode = *handler;
          ++handler;
          // get the length of the second argument
          uint32_t per_length = 0;
          if ((per_encode & 0x60) == 0x60) {
            return false;
          }
          switch (per_encode & 7) {
            default:
              return false;
            case llvm::dwarf::DW_EH_PE_udata2:
              per_length = 2;
              break;
            case llvm::dwarf::DW_EH_PE_udata4:
              per_length = 4;
              break;
            case llvm::dwarf::DW_EH_PE_udata8:
              per_length = 8;
              break;
            case llvm::dwarf::DW_EH_PE_absptr:
              per_length = 4;  // pPkg.bitclass / 8;
              break;
          }
          // skip the alignment
          if (llvm::dwarf::DW_EH_PE_aligned == (per_encode & 0xf0)) {
            uint32_t per_align = handler - cie_end;
            per_align += per_length - 1;
            per_align &= ~(per_length - 1);
            if (static_cast<uint32_t>(cie_end - handler) < per_align) {
              return false;
            }
            handler += per_align;
          }
          // skip the second argument
          if (static_cast<uint32_t>(cie_end - handler) < per_length) {
            return false;
          }
          pr_ptr_data_offset += handler - handler_start;
          pr_ptr_data = std::string((const char*)handler, per_length);
          handler += per_length;
          break;
        }  // end of case 'P'

        // FDE encoding (1 byte)
        case 'R': {
          if (cie_end - handler < 1) {
            return false;
          }
          fde_encoding = *handler;
          switch (fde_encoding & 7) {
            case llvm::dwarf::DW_EH_PE_udata2:
            case llvm::dwarf::DW_EH_PE_udata4:
            case llvm::dwarf::DW_EH_PE_udata8:
            case llvm::dwarf::DW_EH_PE_absptr:
              break;
            default:
              return false;
          }
          ++handler;
          break;
        }
        default:
          return false;
      }  // end switch
    }    // the rest chars.
  }      // first char is 'z'

  // create and push back the CIE entry
  EhFrame::CIE* cie = new EhFrame::CIE(pRegion);
  cie->setFDEEncode(fde_encoding);
  cie->setPersonalityOffset(pr_ptr_data_offset);
  cie->setPersonalityName(pr_ptr_data);
  cie->setAugmentationData(augdata);
  pEhFrame.addCIE(*cie);
  pEhFrame.getCIEMap().insert(std::make_pair(pToken.file_off, cie));
  return true;
}

bool EhFrameReader::addFDE(EhFrame& pEhFrame,
                           llvm::StringRef pRegion,
                           const EhFrameReader::Token& pToken) {
  if (pToken.data_off == pRegion.size())
    return false;

  const int32_t offset =
      *(const int32_t*)(pRegion.begin() + pToken.data_off - 4);
  size_t cie_offset =
      (size_t)((int64_t)(pToken.file_off + 4) - (int32_t)offset);

  EhFrame::CIEMap::iterator iter = pEhFrame.getCIEMap().find(cie_offset);
  if (iter == pEhFrame.getCIEMap().end())
    return false;

  // create and push back the FDE entry
  EhFrame::FDE* fde = new EhFrame::FDE(pRegion, *iter->second);
  pEhFrame.addFDE(*fde);
  return true;
}

bool EhFrameReader::addTerm(EhFrame& pEhFrame,
                            llvm::StringRef pRegion,
                            const EhFrameReader::Token& pToken) {
  return true;
}

bool EhFrameReader::reject(EhFrame& pEhFrame,
                           llvm::StringRef pRegion,
                           const EhFrameReader::Token& pToken) {
  return true;
}

}  // namespace mcld
