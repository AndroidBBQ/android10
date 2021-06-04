//===- ELFObjectFileFormat.h ----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_ELFOBJECTFILEFORMAT_H_
#define MCLD_LD_ELFOBJECTFILEFORMAT_H_
#include "mcld/LD/ELFFileFormat.h"

namespace mcld {

class ObjectBuilder;

/** \class ELFObjectFileFormat
 *  \brief ELFObjectFileFormat describes the format for ELF dynamic objects.
 */
class ELFObjectFileFormat : public ELFFileFormat {
  void initObjectFormat(ObjectBuilder& pBuilder, unsigned int pBitClass) {
    // do nothing
    return;
  }
};

}  // namespace mcld

#endif  // MCLD_LD_ELFOBJECTFILEFORMAT_H_
