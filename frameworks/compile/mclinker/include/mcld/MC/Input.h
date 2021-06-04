//===- Input.h ------------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  Input class inherits MCLDFile, which is used to represent a input file
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_MC_INPUT_H_
#define MCLD_MC_INPUT_H_

#include "mcld/Support/Path.h"

namespace mcld {

class AttributeProxy;
class Attribute;
class InputFactory;
class LDContext;
class MemoryArea;

/** \class Input
 *  \brief Input provides the information of a input file.
 */
class Input {
  friend class InputFactory;

 public:
  enum Type {
    Unknown,
    Binary,
    Object,
    Exec,
    DynObj,
    CoreFile,
    Script,
    Archive,
    External
  };

 public:
  explicit Input(llvm::StringRef pName);

  Input(llvm::StringRef pName, const AttributeProxy& pAttr);

  Input(llvm::StringRef pName,
        const sys::fs::Path& pPath,
        unsigned int pType = Unknown,
        off_t pFileOffset = 0);

  Input(llvm::StringRef pName,
        const sys::fs::Path& pPath,
        const AttributeProxy& pAttr,
        unsigned int pType = Unknown,
        off_t pFileOffset = 0);

  ~Input();

  const std::string& name() const { return m_Name; }

  void setName(const std::string& pName) { m_Name = pName; }

  const sys::fs::Path& path() const { return m_Path; }

  void setPath(const sys::fs::Path& pPath) { m_Path = pPath; }

  void setType(unsigned int pType) { m_Type = pType; }

  unsigned int type() const { return m_Type; }

  bool isRecognized() const { return (m_Type != Unknown); }

  bool hasAttribute() const { return (m_pAttr != NULL); }

  const Attribute* attribute() const { return m_pAttr; }

  bool isNeeded() const { return m_bNeeded; }

  void setNeeded() { m_bNeeded = true; }

  bool noExport() const { return m_bNoExport; }

  void setNoExport() { m_bNoExport = true; }

  off_t fileOffset() const { return m_fileOffset; }

  void setFileOffset(off_t pFileOffset) { m_fileOffset = pFileOffset; }

  // -----  memory area  ----- //
  void setMemArea(MemoryArea* pMemArea) { m_pMemArea = pMemArea; }

  bool hasMemArea() const { return (m_pMemArea != NULL); }

  const MemoryArea* memArea() const { return m_pMemArea; }
  MemoryArea* memArea() { return m_pMemArea; }

  // -----  context  ----- //
  void setContext(LDContext* pContext) { m_pContext = pContext; }

  bool hasContext() const { return (m_pContext != NULL); }

  const LDContext* context() const { return m_pContext; }
  LDContext* context() { return m_pContext; }

 private:
  unsigned int m_Type;
  std::string m_Name;
  sys::fs::Path m_Path;
  Attribute* m_pAttr;
  bool m_bNeeded;
  bool m_bNoExport;
  off_t m_fileOffset;
  MemoryArea* m_pMemArea;
  LDContext* m_pContext;
};

}  // namespace mcld

#endif  // MCLD_MC_INPUT_H_
