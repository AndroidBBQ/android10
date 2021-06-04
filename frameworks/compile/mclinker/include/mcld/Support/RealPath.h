//===- RealPath.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_REALPATH_H_
#define MCLD_SUPPORT_REALPATH_H_
#include "mcld/Support/Path.h"

#include <string>

namespace mcld {
namespace sys {
namespace fs {

/** \class RealPath
 *  \brief The canonicalized absolute pathname.
 *
 */
class RealPath : public Path {
 public:
  typedef Path::ValueType ValueType;
  typedef Path::StringType StringType;

 public:
  RealPath();
  explicit RealPath(const ValueType* s);
  explicit RealPath(const StringType& s);
  explicit RealPath(const Path& pPath);

  ~RealPath();

  RealPath& assign(const Path& pPath);

 protected:
  void initialize();
};

}  // namespace fs
}  // namespace sys
}  // namespace mcld

//----------------------------------------------------------------------------//
//                              STL compatible functions                      //
//----------------------------------------------------------------------------//
namespace std {

template <>
struct less<mcld::sys::fs::RealPath>
    : public binary_function<mcld::sys::fs::RealPath,
                             mcld::sys::fs::RealPath,
                             bool> {
  bool operator()(const mcld::sys::fs::RealPath& pX,
                  const mcld::sys::fs::RealPath& pY) const {
    if (pX.native().size() < pY.native().size())
      return true;
    return (pX.native() < pY.native());
  }
};

}  // namespace std

#endif  // MCLD_SUPPORT_REALPATH_H_
