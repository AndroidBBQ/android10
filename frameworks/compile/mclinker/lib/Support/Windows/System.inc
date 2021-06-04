//===- System.inc ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <string>

#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>

namespace mcld {
namespace sys {

char* strerror(int errnum) {
  return std::strerror(errnum);
}

std::string getDefaultTargetTriple() {
  return MCLD_DEFAULT_TARGET_TRIPLE;
}

int GetPageSize() {
  static int _pagesize = 0;
  if (!_pagesize) {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    _pagesize = sysinfo.dwPageSize;
  }
  return _pagesize;
}

/// random - generate a random number.
long GetRandomNum() {
  return ::rand();
}

/// srandom - set the initial seed value for future calls to random().
void SetRandomSeed(unsigned pSeed) {
  ::srand(pSeed);
}

}  // namespace sys
}  // namespace mcld
