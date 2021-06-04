//===- TargetSelect.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_TARGETSELECT_H_
#define MCLD_SUPPORT_TARGETSELECT_H_

extern "C" {
// Declare all of the target-initialization functions that are available.
#define MCLD_TARGET(TargetName) void MCLDInitialize##TargetName##LDTargetInfo();
#include "mcld/Config/Targets.def"  // NOLINT [build/include] [4]

// Declare all of the available emulators.
#define MCLD_TARGET(TargetName) void MCLDInitialize##TargetName##Emulation();
#include "mcld/Config/Targets.def"  // NOLINT [build/include] [4]

// Declare all of the available target-specific linker
#define MCLD_LINKER(TargetName) void MCLDInitialize##TargetName##LDBackend();
#include "mcld/Config/Linkers.def"  // NOLINT [build/include] [4]

// Declare all of the available target-specific diagnostic line infomation
#define MCLD_LINKER(TargetName) \
  void MCLDInitialize##TargetName##DiagnosticLineInfo();
#include "mcld/Config/Linkers.def"  // NOLINT [build/include] [4]

}  // extern "C"

namespace mcld {
/// InitializeAllTargetInfos - The main program should call this function if
/// it wants access to all available targets that MCLD is configured to
/// support, to make them available via the TargetRegistry.
///
/// It is legal for a client to make multiple calls to this function.
inline void InitializeAllTargetInfos() {
#define MCLD_TARGET(TargetName) MCLDInitialize##TargetName##LDTargetInfo();
#include "mcld/Config/Targets.def"  // NOLINT [build/include] [4]
}

/// InitializeAllTargets - The main program should call this function if it
/// wants access to all available target machines that MCLD is configured to
/// support, to make them available via the TargetRegistry.
///
/// It is legal for a client to make multiple calls to this function.
inline void InitializeAllTargets() {
  mcld::InitializeAllTargetInfos();

#define MCLD_TARGET(TargetName) MCLDInitialize##TargetName##LDBackend();
#include "mcld/Config/Targets.def"  // NOLINT [build/include] [4]
}

/// InitializeAllEmulations - The main program should call this function if
/// it wants all emulations to be configured to support. This function makes
/// all emulations available via the TargetRegistry.
inline void InitializeAllEmulations() {
#define MCLD_TARGET(TargetName) MCLDInitialize##TargetName##Emulation();
#include "mcld/Config/Targets.def"  // NOLINT [build/include] [4]
}

/// InitializeMsgHandler - The main program should call this function if it
/// wants to print linker-specific messages. To make them available via the
/// TargetRegistry.
inline void InitializeAllDiagnostics() {
#define MCLD_LINKER(TargetName) \
  MCLDInitialize##TargetName##DiagnosticLineInfo();
#include "mcld/Config/Linkers.def"  // NOLINT [build/include] [4]
}

}  // namespace mcld

#endif  // MCLD_SUPPORT_TARGETSELECT_H_
