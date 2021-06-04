//===- raw_ostream.h ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_RAW_OSTREAM_H_
#define MCLD_SUPPORT_RAW_OSTREAM_H_
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include <string>

namespace mcld {

class raw_fd_ostream : public llvm::raw_fd_ostream {
 public:
  /// raw_fd_ostream - Open the specified file for writing. If an error occurs,
  /// information about the error is put into ErrorInfo, and the stream should
  /// be immediately destroyed; the string will be empty if no error occurred.
  /// This allows optional flags to control how the file will be opened.
  ///
  /// As a special case, if Filename is "-", then the stream will use
  /// STDOUT_FILENO instead of opening a file. Note that it will still consider
  /// itself to own the file descriptor. In particular, it will close the
  /// file descriptor when it is done (this is necessary to detect
  /// output errors).
  raw_fd_ostream(const char* pFilename,
                 std::error_code& pErrorCode,
                 llvm::sys::fs::OpenFlags pFlags = llvm::sys::fs::F_None);

  /// raw_fd_ostream ctor - FD is the file descriptor that this writes to.  If
  /// ShouldClose is true, this closes the file when the stream is destroyed.
  raw_fd_ostream(int pFD, bool pShouldClose, bool pUnbuffered = false);

  virtual ~raw_fd_ostream();

  void setColor(bool pEnable = true);

  llvm::raw_ostream& changeColor(enum llvm::raw_ostream::Colors pColors,
                                 bool pBold = false,
                                 bool pBackground = false);

  llvm::raw_ostream& resetColor();

  llvm::raw_ostream& reverseColor();

  bool is_displayed() const;

 private:
  bool m_bConfigColor : 1;
  bool m_bSetColor : 1;
};

/// outs() - This returns a reference to a raw_ostream for standard output.
/// Use it like: outs() << "foo" << "bar";
mcld::raw_fd_ostream& outs();

/// errs() - This returns a reference to a raw_ostream for standard error.
/// Use it like: errs() << "foo" << "bar";
mcld::raw_fd_ostream& errs();

}  // namespace mcld

#endif  // MCLD_SUPPORT_RAW_OSTREAM_H_
