//===- Target.h -----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_TARGET_H_
#define MCLD_SUPPORT_TARGET_H_
#include <string>

namespace llvm {
class Target;
class Triple;
class TargetMachine;
}  // namespace llvm

namespace mcld {

class MCLDTargetMachine;
class TargetRegistry;
class MCLinker;
class LinkerScript;
class LinkerConfig;
class Module;
class FileHandle;
class DiagnosticLineInfo;
class TargetLDBackend;

/** \class Target
 *  \brief Target collects target specific information
 */
class Target {
  friend class mcld::MCLDTargetMachine;
  friend class mcld::TargetRegistry;

 public:
  typedef unsigned int (*TripleMatchQualityFnTy)(const llvm::Triple& pTriple);

  typedef MCLDTargetMachine* (*TargetMachineCtorTy)(const llvm::Target&,
                                                    const mcld::Target&,
                                                    llvm::TargetMachine&,
                                                    const std::string&);

  typedef MCLinker* (*MCLinkerCtorTy)(const std::string& pTriple,
                                      LinkerConfig&,
                                      Module&,
                                      FileHandle& pFileHandle);

  typedef bool (*EmulationFnTy)(LinkerScript&, LinkerConfig&);

  typedef TargetLDBackend* (*TargetLDBackendCtorTy)(const LinkerConfig&);

  typedef DiagnosticLineInfo* (*DiagnosticLineInfoCtorTy)(const mcld::Target&,
                                                          const std::string&);

 public:
  Target();

  /// getName - get the target name
  const char* name() const { return Name; }

  unsigned int getTripleQuality(const llvm::Triple& pTriple) const;

  /// createTargetMachine - create target-specific TargetMachine
  MCLDTargetMachine* createTargetMachine(const std::string& pTriple,
                                         const llvm::Target& pTarget,
                                         llvm::TargetMachine& pTM) const;

  /// createMCLinker - create target-specific MCLinker
  MCLinker* createMCLinker(const std::string& pTriple,
                           LinkerConfig& pConfig,
                           Module& pModule,
                           FileHandle& pFileHandle) const;

  /// emulate - given MCLinker default values for the other aspects of the
  /// target system.
  bool emulate(LinkerScript& pScript, LinkerConfig& pConfig) const;

  /// createLDBackend - create target-specific LDBackend
  TargetLDBackend* createLDBackend(const LinkerConfig& pConfig) const;

  /// createDiagnosticLineInfo - create target-specific DiagnosticLineInfo
  DiagnosticLineInfo* createDiagnosticLineInfo(
      const mcld::Target& pTarget,
      const std::string& pTriple) const;

 private:
  /// Name - The target name
  const char* Name;

  TripleMatchQualityFnTy TripleMatchQualityFn;
  TargetMachineCtorTy TargetMachineCtorFn;
  MCLinkerCtorTy MCLinkerCtorFn;
  EmulationFnTy EmulationFn;
  TargetLDBackendCtorTy TargetLDBackendCtorFn;
  DiagnosticLineInfoCtorTy DiagnosticLineInfoCtorFn;
};

}  // namespace mcld

#endif  // MCLD_SUPPORT_TARGET_H_
