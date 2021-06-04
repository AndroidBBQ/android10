//===- System.inc ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <llvm/ADT/StringRef.h>

#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace mcld {
namespace sys {

char* strerror(int errnum) {
  return std::strerror(errnum);
}

static std::string getOSVersion() {
  struct utsname info;

  if (uname(&info))
    return "";

  return info.release;
}

std::string getDefaultTargetTriple() {
  llvm::StringRef TargetTripleString(MCLD_DEFAULT_TARGET_TRIPLE);
  std::pair<llvm::StringRef, llvm::StringRef> ArchSplit =
      TargetTripleString.split('-');

  // Normalize the arch, since the target triple may not actually match the
  // target.
  std::string Arch = ArchSplit.first;

  std::string Triple(Arch);
  Triple += '-';
  Triple += ArchSplit.second;

  // Force i<N>86 to i386.
  if (Triple[0] == 'i' && isdigit(Triple[1]) && Triple[2] == '8' &&
      Triple[3] == '6')
    Triple[1] = '3';

  // On darwin, we want to update the version to match that of the
  // target.
  std::string::size_type DarwinDashIdx = Triple.find("-darwin");
  if (DarwinDashIdx != std::string::npos) {
    Triple.resize(DarwinDashIdx + strlen("-darwin"));
    Triple += getOSVersion();
  }

  return Triple;
}

int GetPageSize() {
  return getpagesize();
}

/// random - generate a random number.
long GetRandomNum() {
  return ::random();
}

/// srandom - set the initial seed value for future calls to random().
void SetRandomSeed(unsigned pSeed) {
  ::srandom(pSeed);
}

}  // namespace sys
}  // namespace mcld
