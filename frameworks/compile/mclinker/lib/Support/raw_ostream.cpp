//===- raw_ostream.cpp ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Config/Config.h"
#include "mcld/Support/raw_ostream.h"

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#if defined(__CYGWIN__) || defined(_MSC_VER) || defined(__MINGW32__)
#include <io.h>
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif
#endif

namespace mcld {

//===----------------------------------------------------------------------===//
// raw_ostream
//===----------------------------------------------------------------------===//
raw_fd_ostream::raw_fd_ostream(const char* pFilename,
                               std::error_code& pErrorCode,
                               llvm::sys::fs::OpenFlags pFlags)
    : llvm::raw_fd_ostream(pFilename, pErrorCode, pFlags),
      m_bConfigColor(false),
      m_bSetColor(false) {
}

raw_fd_ostream::raw_fd_ostream(int pFD,
                               bool pShouldClose,
                               bool pUnbuffered)
    : llvm::raw_fd_ostream(pFD, pShouldClose, pUnbuffered),
      m_bConfigColor(false),
      m_bSetColor(false) {
}

raw_fd_ostream::~raw_fd_ostream() {
}

void raw_fd_ostream::setColor(bool pEnable) {
  m_bConfigColor = true;
  m_bSetColor = pEnable;
}

llvm::raw_ostream& raw_fd_ostream::changeColor(
    enum llvm::raw_ostream::Colors pColor,
    bool pBold,
    bool pBackground) {
  if (!is_displayed())
    return *this;
  return llvm::raw_fd_ostream::changeColor(pColor, pBold, pBackground);
}

llvm::raw_ostream& raw_fd_ostream::resetColor() {
  if (!is_displayed())
    return *this;
  return llvm::raw_fd_ostream::resetColor();
}

llvm::raw_ostream& raw_fd_ostream::reverseColor() {
  if (!is_displayed())
    return *this;
  return llvm::raw_ostream::reverseColor();
}

bool raw_fd_ostream::is_displayed() const {
  if (m_bConfigColor)
    return m_bSetColor;

  return llvm::raw_fd_ostream::is_displayed();
}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//
raw_fd_ostream& outs() {
  // Set buffer settings to model stdout behavior.
  static raw_fd_ostream S(STDOUT_FILENO, false);
  return S;
}

raw_fd_ostream& errs() {
  // Set standard error to be unbuffered by default.
  static raw_fd_ostream S(STDERR_FILENO, false, true);
  return S;
}

}  // namespace mcld
