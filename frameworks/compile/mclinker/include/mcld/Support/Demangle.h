//===- Demangle.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_DEMANGLE_H_
#define MCLD_SUPPORT_DEMANGLE_H_

#include <string>

namespace mcld {

std::string demangleName(const std::string& mangled_name);

bool isCtorOrDtor(const char* pName, size_t pLength);

}  // namespace mcld

#endif  // MCLD_SUPPORT_DEMANGLE_H_
