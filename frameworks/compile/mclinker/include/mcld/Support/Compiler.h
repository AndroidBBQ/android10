//===- Compiler.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_COMPILER_H_
#define MCLD_SUPPORT_COMPILER_H_

#include <llvm/Support/Compiler.h>

// DISALLOW_COPY_AND_ASSIGN disallows the copy and operator= functions.  It goes
// in the private: declarations in a class.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete

// A macro to disallow all the implicit constructors, namely the default
// constructor, copy constructor and operator= functions.
//
// This should be used in the private: declarations for a class that wants to
// prevent anyone from instantiating it. This is especially useful for classes
// containing only static methods.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName() = delete;                           \
  DISALLOW_COPY_AND_ASSIGN(TypeName)

#endif  // MCLD_SUPPORT_COMPILER_H_
