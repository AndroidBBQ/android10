//===- Directory.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/Directory.h"
#include "mcld/Support/FileSystem.h"

namespace mcld {
namespace sys {
namespace fs {

namespace {  // anonymous

bool status_known(FileStatus f) {
  return f.type() != StatusError;
}

bool is_symlink(FileStatus f) {
  return f.type() == SymlinkFile;
}

const Path dot_path(".");
const Path dot_dot_path("..");

}  // anonymous namespace

//===----------------------------------------------------------------------===//
// Directory
//===----------------------------------------------------------------------===//
Directory::Directory()
    : m_Path(),
      m_FileStatus(),
      m_SymLinkStatus(),
      m_Handler(0),
      m_Cache(),
      m_CacheFull(false) {
}

Directory::Directory(const Path& pPath, FileStatus st, FileStatus symlink_st)
    : m_Path(pPath),
      m_FileStatus(st),
      m_SymLinkStatus(symlink_st),
      m_Handler(0),
      m_Cache(),
      m_CacheFull(false) {
  if (m_Path == dot_path)
    detail::get_pwd(m_Path);
  m_Path.m_append_separator_if_needed();
  detail::open_dir(*this);
}

Directory::Directory(const char* pPath, FileStatus st, FileStatus symlink_st)
    : Directory(sys::fs::Path(pPath), st, symlink_st) {
}

Directory::Directory(const Directory& pCopy)
    : m_Path(pCopy.m_Path),
      m_FileStatus(pCopy.m_FileStatus),
      m_SymLinkStatus(pCopy.m_SymLinkStatus),
      m_Handler(0),
      m_Cache(),
      m_CacheFull(false) {
  detail::open_dir(*this);
}

Directory::~Directory() {
  detail::close_dir(*this);
}

bool Directory::isGood() const {
  return (0 != m_Handler);
}

Directory& Directory::operator=(const Directory& pCopy) {
  assign(pCopy.m_Path, pCopy.m_FileStatus, pCopy.m_SymLinkStatus);
  return *this;
}

void Directory::assign(const Path& pPath,
                       FileStatus st,
                       FileStatus symlink_st) {
  if (isGood())
    clear();

  m_Path = pPath;
  if (m_Path == dot_path)
    detail::get_pwd(m_Path);
  m_Path.m_append_separator_if_needed();

  m_FileStatus = st;
  m_SymLinkStatus = symlink_st;
  detail::open_dir(*this);
}

FileStatus Directory::status() const {
  if (!status_known(m_FileStatus)) {
    // optimization: if the symlink status is known, and it isn't a symlink,
    // then status and symlink_status are identical so just copy the
    // symlink status to the regular status.
    if (status_known(m_SymLinkStatus) && !is_symlink(m_SymLinkStatus)) {
      m_FileStatus = m_SymLinkStatus;
    } else
      detail::status(m_Path, m_FileStatus);
  }
  return m_FileStatus;
}

FileStatus Directory::symlinkStatus() const {
  if (!status_known(m_SymLinkStatus))
    detail::symlink_status(m_Path, m_SymLinkStatus);
  return m_SymLinkStatus;
}

Directory::iterator Directory::begin() {
  if (m_CacheFull && m_Cache.empty())
    return end();
  PathCache::iterator iter = m_Cache.begin();
  if (iter.getEntry() == NULL)
    ++iter;
  return iterator(this, iter);
}

Directory::iterator Directory::end() {
  return iterator(0, m_Cache.end());
}

void Directory::clear() {
  m_Path.native().clear();
  m_FileStatus = FileStatus();
  m_SymLinkStatus = FileStatus();
  m_Cache.clear();
  detail::close_dir(*this);
}

//==========================
// DirIterator
DirIterator::DirIterator(Directory* pParent,
                         const DirIterator::DirCache::iterator& pIter)
    : m_pParent(pParent), m_Iter(pIter) {
  m_pEntry = m_Iter.getEntry();
}

DirIterator::DirIterator(const DirIterator& pCopy)
    : m_pParent(pCopy.m_pParent),
      m_Iter(pCopy.m_Iter),
      m_pEntry(pCopy.m_pEntry) {
}

DirIterator::~DirIterator() {
}

Path* DirIterator::path() {
  if (m_pParent == NULL)
    return NULL;
  return &m_pEntry->value();
}

const Path* DirIterator::path() const {
  if (m_pParent == NULL)
    return NULL;
  return &m_pEntry->value();
}

DirIterator& DirIterator::operator=(const DirIterator& pCopy) {
  m_pParent = pCopy.m_pParent;
  m_Iter = pCopy.m_Iter;
  m_pEntry = pCopy.m_pEntry;
  return (*this);
}

DirIterator& DirIterator::operator++() {
  if (m_pParent == 0)
    return *this;

  // move forward one step first.
  ++m_Iter;

  if (m_pParent->m_Cache.end() == m_Iter) {
    if (!m_pParent->m_CacheFull) {
      m_pEntry = detail::bring_one_into_cache(*this);
      if (m_pEntry == 0 && m_pParent->m_CacheFull)
        m_pParent = 0;
      return *this;
    }
    m_pParent = 0;
    return *this;
  }

  m_pEntry = m_Iter.getEntry();
  return *this;
}

DirIterator DirIterator::operator++(int pIn) {
  DirIterator tmp(*this);

  // move forward one step first.
  ++m_Iter;

  if (m_pParent->m_Cache.end() == m_Iter) {
    if (!m_pParent->m_CacheFull) {
      m_pEntry = detail::bring_one_into_cache(*this);
      if (m_pEntry == 0 && m_pParent->m_CacheFull)
        m_pParent = 0;
      return tmp;
    }
    m_pParent = 0;
    return tmp;
  }

  m_pEntry = m_Iter.getEntry();
  return tmp;
}

bool DirIterator::operator==(const DirIterator& y) const {
  if (m_pParent != y.m_pParent)
    return false;
  if (m_pParent == 0)
    return true;
  const Path* x_path = path();
  const Path* y_path = y.path();
  if (x_path == 0 && y_path == 0)
    return true;
  if (x_path == 0 || y_path == 0)
    return false;
  return (*x_path == *y_path);
}

bool DirIterator::operator!=(const DirIterator& y) const {
  return !this->operator==(y);
}

}  // namespace fs
}  // namespace sys
}  // namespace mcld
