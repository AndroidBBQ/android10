//===- MCLDDirectory.cpp --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/MC/MCLDDirectory.h"
#include "mcld/Support/FileSystem.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// MCLDDirectory
//===----------------------------------------------------------------------===//
MCLDDirectory::MCLDDirectory() : Directory(), m_Name(), m_bInSysroot(false) {
}

MCLDDirectory::MCLDDirectory(const char* pName) : Directory(), m_Name(pName) {
  Directory::m_Path.assign(pName);

  if (!Directory::m_Path.empty())
    m_bInSysroot = (Directory::m_Path.native()[0] == '=');

  Directory::m_Path.m_append_separator_if_needed();
  if (m_bInSysroot)
    Directory::m_Path.native().erase(Directory::m_Path.native().begin());
  else
    sys::fs::detail::open_dir(*this);
}

MCLDDirectory::MCLDDirectory(const std::string& pName)
    : Directory(), m_Name(pName) {
  Directory::m_Path.assign(pName);

  if (!Directory::m_Path.empty())
    m_bInSysroot = (Directory::m_Path.native()[0] == '=');

  Directory::m_Path.m_append_separator_if_needed();
  if (m_bInSysroot)
    Directory::m_Path.native().erase(Directory::m_Path.native().begin());
  else
    sys::fs::detail::open_dir(*this);
}

MCLDDirectory::MCLDDirectory(llvm::StringRef pName)
    : Directory(), m_Name(pName.data(), pName.size()) {
  Directory::m_Path.assign(pName.str());

  if (!Directory::m_Path.empty())
    m_bInSysroot = (Directory::m_Path.native()[0] == '=');

  Directory::m_Path.m_append_separator_if_needed();
  if (m_bInSysroot)
    Directory::m_Path.native().erase(Directory::m_Path.native().begin());
  else
    sys::fs::detail::open_dir(*this);
}

MCLDDirectory& MCLDDirectory::assign(llvm::StringRef pName) {
  m_Name.assign(pName.data(), pName.size());
  Directory::m_Path.assign(pName.str());

  if (!Directory::m_Path.empty())
    m_bInSysroot = (Directory::m_Path.native()[0] == '=');

  Directory::m_Path.m_append_separator_if_needed();
  if (m_bInSysroot)
    Directory::m_Path.native().erase(Directory::m_Path.native().begin());
  else
    sys::fs::detail::open_dir(*this);
  Directory::m_FileStatus = sys::fs::FileStatus();
  Directory::m_SymLinkStatus = sys::fs::FileStatus();
  Directory::m_Cache.clear();
  Directory::m_Handler = 0;
  return (*this);
}

MCLDDirectory::~MCLDDirectory() {
}

bool MCLDDirectory::isInSysroot() const {
  return m_bInSysroot;
}

void MCLDDirectory::setSysroot(const sys::fs::Path& pSysroot) {
  if (m_bInSysroot) {
    std::string old_path = Directory::m_Path.native();
    Directory::m_Path.native() = pSysroot.native();
    Directory::m_Path.m_append_separator_if_needed();
    Directory::m_Path.native() += old_path;
    sys::fs::detail::canonicalize(Directory::m_Path.native());
    sys::fs::detail::open_dir(*this);
  }
}

}  // namespace mcld
