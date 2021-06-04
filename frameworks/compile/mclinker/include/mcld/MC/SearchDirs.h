//===- SearchDirs.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_MC_SEARCHDIRS_H_
#define MCLD_MC_SEARCHDIRS_H_

#include "mcld/MC/Input.h"
#include "mcld/Support/Path.h"
#include "mcld/Support/Compiler.h"

#include <llvm/ADT/StringRef.h>

#include <string>
#include <vector>

namespace mcld {

class MCLDDirectory;
class MCLDFile;

/** \class SearchDirs
 *  \brief SearchDirs contains the list of paths that MCLinker will search for
 *  archive libraries and control scripts.
 *
 *  SearchDirs is customized for linking. It handles -L on the command line
 *  and SEARCH_DIR macro in the link script.
 *
 *  @see MCLDDirectory.
 */
class SearchDirs {
 public:
  typedef std::vector<MCLDDirectory*> DirList;
  typedef DirList::iterator iterator;
  typedef DirList::const_iterator const_iterator;

 public:
  SearchDirs();

  explicit SearchDirs(const sys::fs::Path& pSysRoot);

  ~SearchDirs();

  // find - give a namespec, return a real path of the shared object.
  sys::fs::Path* find(const std::string& pNamespec,
                      mcld::Input::Type pPreferType);

  const sys::fs::Path* find(const std::string& pNamespec,
                            mcld::Input::Type pPreferType) const;

  void setSysRoot(const sys::fs::Path& pSysRoot) { m_SysRoot = pSysRoot; }
  const sys::fs::Path& sysroot() const { return m_SysRoot; }

  // -----  iterators  ----- //
  const_iterator begin() const { return m_DirList.begin(); }
  iterator begin() { return m_DirList.begin(); }
  const_iterator end() const { return m_DirList.end(); }
  iterator end() { return m_DirList.end(); }

  // -----  modifiers  ----- //
  bool insert(const char* pDirectory);

  bool insert(const std::string& pDirectory);

  bool insert(const sys::fs::Path& pDirectory);

 private:
  DirList m_DirList;
  sys::fs::Path m_SysRoot;

 private:
  DISALLOW_COPY_AND_ASSIGN(SearchDirs);
};

}  // namespace mcld

#endif  // MCLD_MC_SEARCHDIRS_H_
