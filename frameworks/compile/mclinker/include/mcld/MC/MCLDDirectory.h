//===- MCLDDirectory.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_MC_MCLDDIRECTORY_H_
#define MCLD_MC_MCLDDIRECTORY_H_
#include "mcld/Support/Directory.h"
#include "mcld/Support/FileSystem.h"

#include <llvm/ADT/StringRef.h>

#include <string>

namespace mcld {

/** \class MCLDDirectory
 *  \brief MCLDDirectory is an directory entry for library search.
 *
 */
class MCLDDirectory : public sys::fs::Directory {
 public:
  MCLDDirectory();
  explicit MCLDDirectory(const char* pName);
  explicit MCLDDirectory(const std::string& pName);
  explicit MCLDDirectory(llvm::StringRef pName);
  virtual ~MCLDDirectory();

 public:
  MCLDDirectory& assign(llvm::StringRef pName);
  bool isInSysroot() const;

  /// setSysroot - if MCLDDirectory is in sysroot, modify the path.
  void setSysroot(const sys::fs::Path& pPath);

  const std::string& name() const { return m_Name; }

 private:
  std::string m_Name;
  bool m_bInSysroot;
};

}  // namespace mcld

#endif  // MCLD_MC_MCLDDIRECTORY_H_
