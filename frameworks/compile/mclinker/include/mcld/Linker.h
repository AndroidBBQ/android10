//===- Linker.h -----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LINKER_H_
#define MCLD_LINKER_H_

#include <string>

namespace mcld {

class FileHandle;
class FileOutputBuffer;
class IRBuilder;
class LinkerConfig;
class LinkerScript;
class Module;
class ObjectLinker;
class Target;
class TargetLDBackend;

/** \class Linker
*  \brief Linker is a modular linker.
*/
class Linker {
 public:
  Linker();

  ~Linker();

  /// emulate - To set up target-dependent options and default linker script.
  bool emulate(LinkerScript& pScript, LinkerConfig& pConfig);

  /// normalize - To normalize the command line language into mcld::Module.
  bool normalize(Module& pModule, IRBuilder& pBuilder);

  /// resolve - To build up the topology of mcld::Module.
  bool resolve(Module& pModule);

  /// layout - To serialize the final result of the output mcld::Module.
  bool layout();

  /// link - A convenient way to resolve and to layout the output mcld::Module.
  bool link(Module& pModule, IRBuilder& pBuilder);

  /// emit - To emit output mcld::Module to a FileOutputBuffer.
  bool emit(FileOutputBuffer& pOutput);

  /// emit - To open a file for output in pPath and to emit output mcld::Module
  /// to the file.
  bool emit(const Module& pModule, const std::string& pPath);

  /// emit - To emit output mcld::Module in the pFileDescriptor.
  bool emit(const Module& pModule, int pFileDescriptor);

  bool reset();

 private:
  bool initTarget();

  bool initBackend();

  bool initOStream();

  bool initEmulator(LinkerScript& pScript);

 private:
  LinkerConfig* m_pConfig;
  IRBuilder* m_pIRBuilder;

  const Target* m_pTarget;
  TargetLDBackend* m_pBackend;
  ObjectLinker* m_pObjLinker;
};

}  // namespace mcld

#endif  // MCLD_LINKER_H_
