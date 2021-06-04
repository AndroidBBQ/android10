//===- MemoryRegion.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_MEMORYREGION_H_
#define MCLD_SUPPORT_MEMORYREGION_H_

#include "mcld/ADT/TypeTraits.h"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/Support/DataTypes.h>

namespace mcld {

typedef NonConstTraits<uint8_t>::pointer Address;
typedef ConstTraits<uint8_t>::pointer ConstAddress;

typedef llvm::ArrayRef<uint8_t> ConstMemoryRegion;
typedef llvm::MutableArrayRef<uint8_t> MemoryRegion;

}  // namespace mcld

#endif  // MCLD_SUPPORT_MEMORYREGION_H_
