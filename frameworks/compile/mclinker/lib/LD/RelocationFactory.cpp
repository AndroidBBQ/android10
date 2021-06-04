//===- RelocationFactory.cpp ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/RelocationFactory.h"

#include "mcld/LinkerConfig.h"
#include "mcld/Support/MsgHandling.h"
#include "mcld/Target/TargetLDBackend.h"

#include <llvm/Support/Host.h>

#include <cassert>
#include <cstring>

namespace mcld {

//===----------------------------------------------------------------------===//
// RelocationFactory
//===----------------------------------------------------------------------===//
RelocationFactory::RelocationFactory()
    : GCFactory<Relocation, MCLD_RELOCATIONS_PER_INPUT>(), m_pConfig(NULL) {
}

void RelocationFactory::setConfig(const LinkerConfig& pConfig) {
  m_pConfig = &pConfig;
}

Relocation* RelocationFactory::produce(RelocationFactory::Type pType,
                                       FragmentRef& pFragRef,
                                       Address pAddend) {
  if (m_pConfig == NULL) {
    fatal(diag::reloc_factory_has_not_config);
    return NULL;
  }

  // target_data is the place where the relocation applys to.
  // Use TargetDataFactory to generate temporary data, and copy the
  // content of the fragment into this data.
  DWord target_data = 0;

  // byte swapping if the host and target have different endian
  if (llvm::sys::IsLittleEndianHost != m_pConfig->targets().isLittleEndian()) {
    uint32_t tmp_data;

    switch (m_pConfig->targets().bitclass()) {
      case 32: {
        pFragRef.memcpy(&tmp_data, 4);
        tmp_data = mcld::bswap32(tmp_data);
        target_data = tmp_data;
        break;
      }
      case 64: {
        pFragRef.memcpy(&target_data, 8);
        target_data = mcld::bswap64(target_data);
        break;
      }
      default: {
        fatal(diag::unsupported_bitclass) << m_pConfig->targets().triple().str()
                                          << m_pConfig->targets().bitclass();
        return NULL;
      }
    }  // end of switch
  } else {
    pFragRef.memcpy(&target_data, (m_pConfig->targets().bitclass() / 8));
  }

  Relocation* result = allocate();
  new (result) Relocation(pType, &pFragRef, pAddend, target_data);
  return result;
}

Relocation* RelocationFactory::produceEmptyEntry() {
  Relocation* result = allocate();
  new (result) Relocation(0, 0, 0, 0);
  return result;
}

void RelocationFactory::destroy(Relocation* pRelocation) {
  /** GCFactory will recycle the relocation **/
}

}  // namespace mcld
