//===- Path.cpp -----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/Path.h"

#include "mcld/Config/Config.h"
#include "mcld/Support/FileSystem.h"

#include <llvm/ADT/StringRef.h>

#include <istream>
#include <locale>
#include <ostream>
#include <string.h>

namespace mcld {
namespace sys {
namespace fs {

//===--------------------------------------------------------------------===//
// Helper
//===--------------------------------------------------------------------===//
namespace {
#if defined(MCLD_ON_WIN32)
bool is_separator(char value) {
  return (value == separator || value == preferred_separator);
}

const Path::StringType separator_str("/");

#else
bool is_separator(char value) {
  return (value == separator);
}

const Path::StringType separator_str("/");

#endif
}  // anonymous namespace

//===--------------------------------------------------------------------===//
// Path
//===--------------------------------------------------------------------===//
Path::Path() : m_PathName() {
}

Path::Path(const Path::ValueType* s) : m_PathName(s) {
}

Path::Path(const Path::StringType& s) : m_PathName(s) {
}

Path::Path(const Path& pCopy) : m_PathName(pCopy.m_PathName) {
}

Path::~Path() {
}

bool Path::isFromRoot() const {
  if (m_PathName.empty())
    return false;
  return (separator == m_PathName[0]);
}

bool Path::isFromPWD() const {
  if (m_PathName.size() < 2)
    return false;
  return ('.' == m_PathName[0] && separator == m_PathName[1]);
}

Path& Path::assign(const Path::StringType& s) {
  m_PathName.assign(s);
  return *this;
}

Path& Path::assign(const Path::ValueType* s, unsigned int length) {
  if (s == 0 || length == 0)
    assert(0 && "assign a null or empty string to Path");
  m_PathName.assign(s, length);
  return *this;
}

// a,/b a/,b a/,b/ a,b is a/b
Path& Path::append(const Path& pPath) {
  // first path is a/,second path is /b
  if (m_PathName[m_PathName.length() - 1] == separator &&
      pPath.native()[0] == separator) {
    llvm::StringRef path(pPath.native());
    m_PathName.append(path.begin() + 1, path.end());
  } else if (this->native()[this->native().size() - 1] != separator &&
             pPath.native()[0] != separator) {
    // first path is a,second path is b
    m_PathName.append(separator_str);
    m_PathName.append(pPath.native());
  } else {
    // a/,b or a,/b just append
    m_PathName.append(pPath.native());
  }
  return *this;
}

// a,/b a/,b a/,b/ a,b is a/b
Path& Path::append(const StringType& pPath) {
  Path path(pPath);
  this->append(path);
  return *this;
}

bool Path::empty() const {
  return m_PathName.empty();
}

Path::StringType Path::generic_string() const {
  StringType result = m_PathName;
  detail::canonicalize(result);
  return result;
}

bool Path::canonicalize() {
  return detail::canonicalize(m_PathName);
}

Path::StringType::size_type Path::m_append_separator_if_needed() {
#if defined(MCLD_ON_WIN32)
  // On Windows platform, path can not append separator.
  return 0;
#endif

  StringType::value_type last_char = m_PathName[m_PathName.size() - 1];
  if (!m_PathName.empty() && !is_separator(last_char)) {
    StringType::size_type tmp(m_PathName.size());
    m_PathName += separator_str;
    return tmp;
  }
  return 0;
}

void Path::m_erase_redundant_separator(Path::StringType::size_type pSepPos) {
  size_t begin = pSepPos;
  // skip '/' or '\\'
  while (separator == m_PathName[pSepPos]) {
#if defined(MCLD_ON_WIN32)
    pSepPos += 2;
#else
    ++pSepPos;
#endif
  }

  if (begin != pSepPos)
    m_PathName.erase(begin + 1, pSepPos - begin - 1);
}

Path Path::parent_path() const {
  size_t end_pos = m_PathName.find_last_of(separator);
  if (end_pos != StringType::npos)
    return Path(m_PathName.substr(0, end_pos));
  return Path();
}

Path Path::filename() const {
  size_t pos = m_PathName.find_last_of(separator);
  if (pos != StringType::npos) {
    ++pos;
    return Path(m_PathName.substr(pos));
  }
  return Path(*this);
}

Path Path::stem() const {
  size_t begin_pos = m_PathName.find_last_of(separator) + 1;
  size_t end_pos = m_PathName.find_last_of(dot);
  Path result_path(m_PathName.substr(begin_pos, end_pos - begin_pos));
  return result_path;
}

Path Path::extension() const {
  size_t pos = m_PathName.find_last_of('.');
  if (pos == StringType::npos)
    return Path();
  return Path(m_PathName.substr(pos));
}

//===--------------------------------------------------------------------===//
// non-member functions
//===--------------------------------------------------------------------===//
bool operator==(const Path& pLHS, const Path& pRHS) {
  return (pLHS.generic_string() == pRHS.generic_string());
}

bool operator!=(const Path& pLHS, const Path& pRHS) {
  return !(pLHS == pRHS);
}

Path operator+(const Path& pLHS, const Path& pRHS) {
  mcld::sys::fs::Path result = pLHS;
  result.append(pRHS);
  return result;
}

}  // namespace fs
}  // namespace sys
}  // namespace mcld
