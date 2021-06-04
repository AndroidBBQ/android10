//===- FileSystem.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Config/Config.h"
#include "mcld/Support/FileSystem.h"
#include "mcld/Support/Path.h"

//===----------------------------------------------------------------------===//
// non-member functions
//===----------------------------------------------------------------------===//
bool mcld::sys::fs::exists(const Path& pPath) {
  mcld::sys::fs::FileStatus file_status;
  mcld::sys::fs::detail::status(pPath, file_status);
  return (file_status.type() != mcld::sys::fs::StatusError) &&
         (file_status.type() != mcld::sys::fs::FileNotFound);
}

bool mcld::sys::fs::is_directory(const Path& pPath) {
  FileStatus file_status;
  detail::status(pPath, file_status);
  return (file_status.type() == mcld::sys::fs::DirectoryFile);
}

// Include the truly platform-specific parts.
#if defined(MCLD_ON_UNIX)
#include "Unix/FileSystem.inc"
#include "Unix/PathV3.inc"
#endif
#if defined(MCLD_ON_WIN32)
#include "Windows/FileSystem.inc"
#include "Windows/PathV3.inc"
#endif
