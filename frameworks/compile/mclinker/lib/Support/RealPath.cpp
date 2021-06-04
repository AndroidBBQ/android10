//===- RealPath.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/RealPath.h"
#include "mcld/Support/FileSystem.h"

namespace mcld {
namespace sys {
namespace fs {

//==========================
// RealPath
RealPath::RealPath() : Path() {
}

RealPath::RealPath(const RealPath::ValueType* s) : Path(s) {
  initialize();
}

RealPath::RealPath(const RealPath::StringType& s) : Path(s) {
  initialize();
}

RealPath::RealPath(const Path& pPath) : Path(pPath) {
  initialize();
}

RealPath::~RealPath() {
}

RealPath& RealPath::assign(const Path& pPath) {
  Path::m_PathName.assign(pPath.native());
  return (*this);
}

void RealPath::initialize() {
  if (isFromRoot()) {
    detail::canonicalize(m_PathName);
  } else if (isFromPWD()) {
    Path path_name;
    detail::get_pwd(path_name);
    path_name.native() += preferred_separator;
    path_name.native() += m_PathName;
    detail::canonicalize(path_name.native());
    m_PathName = path_name.native();
  }
}

}  // namespace fs
}  // namespace sys
}  // namespace mcld
