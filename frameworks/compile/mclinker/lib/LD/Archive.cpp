//===- Archive.cpp --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/Archive.h"
#include "mcld/MC/Input.h"
#include "mcld/MC/InputBuilder.h"
#include "mcld/Support/MsgHandling.h"

#include <llvm/ADT/StringRef.h>

namespace mcld {

//===----------------------------------------------------------------------===//
// Archive
const char Archive::MAGIC[] = "!<arch>\n";
const char Archive::THIN_MAGIC[] = "!<thin>\n";
const size_t Archive::MAGIC_LEN = sizeof(Archive::MAGIC) - 1;
const char Archive::SVR4_SYMTAB_NAME[] = "/               ";
const char Archive::IRIX6_SYMTAB_NAME[] = "/SYM64/         ";
const char Archive::STRTAB_NAME[] = "//              ";
const char Archive::PAD[] = "\n";
const char Archive::MEMBER_MAGIC[] = "`\n";

Archive::Archive(Input& pInputFile, InputBuilder& pBuilder)
    : m_ArchiveFile(pInputFile),
      m_pInputTree(NULL),
      m_SymbolFactory(32),
      m_Builder(pBuilder) {
  // FIXME: move creation of input tree out of Archive.
  m_pInputTree = new InputTree();
}

Archive::~Archive() {
  delete m_pInputTree;
}

/// getARFile - get the Input& of the archive file
Input& Archive::getARFile() {
  return m_ArchiveFile;
}

/// getARFile - get the Input& of the archive file
const Input& Archive::getARFile() const {
  return m_ArchiveFile;
}

/// inputs - get the input tree built from this archive
InputTree& Archive::inputs() {
  return *m_pInputTree;
}

/// inputs - get the input tree built from this archive
const InputTree& Archive::inputs() const {
  return *m_pInputTree;
}

/// getObjectMemberMap - get the map that contains the included object files
Archive::ObjectMemberMapType& Archive::getObjectMemberMap() {
  return m_ObjectMemberMap;
}

/// getObjectMemberMap - get the map that contains the included object files
const Archive::ObjectMemberMapType& Archive::getObjectMemberMap() const {
  return m_ObjectMemberMap;
}

/// numOfObjectMember - return the number of included object files
size_t Archive::numOfObjectMember() const {
  return m_ObjectMemberMap.numOfEntries();
}

/// addObjectMember - add a object in the object member map
/// @param pFileOffset - file offset in symtab represents a object file
/// @param pIter - the iterator in the input tree built from this archive
bool Archive::addObjectMember(uint32_t pFileOffset, InputTree::iterator pIter) {
  bool exist;
  ObjectMemberEntryType* entry = m_ObjectMemberMap.insert(pFileOffset, exist);
  if (!exist)
    entry->setValue(pIter);
  return !exist;
}

/// hasObjectMember - check if a object file is included or not
/// @param pFileOffset - file offset in symtab represents a object file
bool Archive::hasObjectMember(uint32_t pFileOffset) const {
  return (m_ObjectMemberMap.find(pFileOffset) != m_ObjectMemberMap.end());
}

/// getArchiveMemberMap - get the map that contains the included archive files
Archive::ArchiveMemberMapType& Archive::getArchiveMemberMap() {
  return m_ArchiveMemberMap;
}

/// getArchiveMemberMap - get the map that contains the included archive files
const Archive::ArchiveMemberMapType& Archive::getArchiveMemberMap() const {
  return m_ArchiveMemberMap;
}

/// addArchiveMember - add an archive in the archive member map
/// @param pName    - the name of the new archive member
/// @param pLastPos - this records the point to insert the next node in the
///                   subtree of this archive member
/// @param pMove    - this records the direction to insert the next node in the
///                   subtree of this archive member
bool Archive::addArchiveMember(const llvm::StringRef& pName,
                               InputTree::iterator pLastPos,
                               InputTree::Mover* pMove) {
  bool exist;
  ArchiveMemberEntryType* entry = m_ArchiveMemberMap.insert(pName, exist);
  if (!exist) {
    ArchiveMember& ar = entry->value();
    if (pLastPos == m_pInputTree->root())
      ar.file = &m_ArchiveFile;
    else
      ar.file = *pLastPos;
    ar.lastPos = pLastPos;
    ar.move = pMove;
  }
  return !exist;
}

/// hasArchiveMember - check if an archive file is included or not
bool Archive::hasArchiveMember(const llvm::StringRef& pName) const {
  return (m_ArchiveMemberMap.find(pName) != m_ArchiveMemberMap.end());
}

/// getArchiveMember - get a archive member
Archive::ArchiveMember* Archive::getArchiveMember(
    const llvm::StringRef& pName) {
  ArchiveMemberMapType::iterator it = m_ArchiveMemberMap.find(pName);
  if (it != m_ArchiveMemberMap.end())
    return &(it.getEntry()->value());
  return NULL;
}

/// getSymbolTable - get the symtab
Archive::SymTabType& Archive::getSymbolTable() {
  return m_SymTab;
}

/// getSymbolTable - get the symtab
const Archive::SymTabType& Archive::getSymbolTable() const {
  return m_SymTab;
}

/// setSymTabSize - set the memory size of symtab
void Archive::setSymTabSize(size_t pSize) {
  m_SymTabSize = pSize;
}

/// getSymTabSize - get the memory size of symtab
size_t Archive::getSymTabSize() const {
  return m_SymTabSize;
}

/// numOfSymbols - return the number of symbols in symtab
size_t Archive::numOfSymbols() const {
  return m_SymTab.size();
}

/// addSymbol - add a symtab entry to symtab
/// @param pName - symbol name
/// @param pFileOffset - file offset in symtab represents a object file
void Archive::addSymbol(const char* pName,
                        uint32_t pFileOffset,
                        enum Archive::Symbol::Status pStatus) {
  Symbol* entry = m_SymbolFactory.allocate();
  new (entry) Symbol(pName, pFileOffset, pStatus);
  m_SymTab.push_back(entry);
}

/// getSymbolName - get the symbol name with the given index
const std::string& Archive::getSymbolName(size_t pSymIdx) const {
  assert(pSymIdx < numOfSymbols());
  return m_SymTab[pSymIdx]->name;
}

/// getObjFileOffset - get the file offset that represent a object file
uint32_t Archive::getObjFileOffset(size_t pSymIdx) const {
  assert(pSymIdx < numOfSymbols());
  return m_SymTab[pSymIdx]->fileOffset;
}

/// getSymbolStatus - get the status of a symbol
enum Archive::Symbol::Status Archive::getSymbolStatus(size_t pSymIdx) const {
  assert(pSymIdx < numOfSymbols());
  return m_SymTab[pSymIdx]->status;
}

/// setSymbolStatus - set the status of a symbol
void Archive::setSymbolStatus(size_t pSymIdx,
                              enum Archive::Symbol::Status pStatus) {
  assert(pSymIdx < numOfSymbols());
  m_SymTab[pSymIdx]->status = pStatus;
}

/// getStrTable - get the extended name table
std::string& Archive::getStrTable() {
  return m_StrTab;
}

/// getStrTable - get the extended name table
const std::string& Archive::getStrTable() const {
  return m_StrTab;
}

/// hasStrTable()
bool Archive::hasStrTable() const {
  return (m_StrTab.size() > 0);
}

/// getMemberFile - get the member file in an archive member
/// @param pArchiveFile - Input reference of the archive member
/// @param pIsThinAR    - denote the archive menber is a Thin Archive or not
/// @param pName        - the name of the member file we want to get
/// @param pPath        - the path of the member file
/// @param pFileOffset  - the file offset of the member file in a regular AR
/// FIXME: maybe we should not construct input file here
Input* Archive::getMemberFile(Input& pArchiveFile,
                              bool isThinAR,
                              const std::string& pName,
                              const sys::fs::Path& pPath,
                              off_t pFileOffset) {
  Input* member = NULL;
  if (!isThinAR) {
    member = m_Builder.createInput(pName, pPath, Input::Unknown, pFileOffset);
    assert(member != NULL);
    member->setMemArea(pArchiveFile.memArea());
    m_Builder.setContext(*member);
  } else {
    member = m_Builder.createInput(pName, pPath, Input::Unknown);
    assert(member != NULL);
    if (!m_Builder.setMemory(*member,
                             FileHandle::OpenMode(FileHandle::ReadOnly),
                             FileHandle::Permission(FileHandle::System))) {
      error(diag::err_cannot_open_input) << member->name() << member->path();
      return NULL;
    }
    m_Builder.setContext(*member);
  }
  return member;
}

}  // namespace mcld
