//===- PathCache.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_PATHCACHE_H_
#define MCLD_SUPPORT_PATHCACHE_H_

#include "mcld/ADT/HashEntry.h"
#include "mcld/ADT/HashTable.h"
#include "mcld/ADT/StringHash.h"
#include "mcld/Support/Path.h"

namespace mcld {
namespace sys {
namespace fs {

typedef HashEntry<llvm::StringRef,
                  mcld::sys::fs::Path,
                  hash::StringCompare<llvm::StringRef> > HashEntryType;

typedef HashTable<HashEntryType,
                  hash::StringHash<hash::DJB>,
                  EntryFactory<HashEntryType> > PathCache;

}  // namespace fs
}  // namespace sys
}  // namespace mcld

#endif  // MCLD_SUPPORT_PATHCACHE_H_
