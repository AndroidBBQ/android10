//===- Archive.h ----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_ARCHIVE_H_
#define MCLD_LD_ARCHIVE_H_

#include "mcld/InputTree.h"
#include "mcld/ADT/HashEntry.h"
#include "mcld/ADT/HashTable.h"
#include "mcld/ADT/StringHash.h"
#include "mcld/Support/GCFactory.h"

#include <string>
#include <vector>

namespace mcld {

class Input;
class InputBuilder;
class InputFactory;

/** \class Archive
 *  \brief This class define the interfacee to Archive files
 */
class Archive {
 public:
  static const char MAGIC[];              ///< magic string
  static const char THIN_MAGIC[];         ///< magic of thin archive
  static const size_t MAGIC_LEN;          ///< length of magic string
  static const char SVR4_SYMTAB_NAME[];   ///< SVR4 symtab entry name
  static const char IRIX6_SYMTAB_NAME[];  ///< Irix6 symtab entry name
  static const char STRTAB_NAME[];        ///< Name of string table
  static const char PAD[];                ///< inter-file align padding
  static const char MEMBER_MAGIC[];       ///< fmag field magic #

  struct MemberHeader {
    char name[16];  ///< Name of the file member.
    char date[12];  ///< File date, decimal seconds since Epoch
    char uid[6];    ///< user id in ASCII decimal
    char gid[6];    ///< group id in ASCII decimal
    char mode[8];   ///< file mode in ASCII octal
    char size[10];  ///< file size in ASCII decimal
    char fmag[2];   ///< Always contains ARFILE_MAGIC_TERMINATOR
  };

 private:
  template <typename OFFSET_TYPE>
  struct OffsetCompare {
    bool operator()(OFFSET_TYPE X, OFFSET_TYPE Y) const { return (X == Y); }
  };

  struct MurmurHash3 {
    size_t operator()(uint32_t pKey) const {
      pKey ^= pKey >> 16;
      pKey *= 0x85ebca6b;
      pKey ^= pKey >> 13;
      pKey *= 0xc2b2ae35;
      pKey ^= pKey >> 16;
      return pKey;
    }
  };

  typedef HashEntry<uint32_t, InputTree::iterator, OffsetCompare<uint32_t> >
      ObjectMemberEntryType;

 public:
  typedef HashTable<ObjectMemberEntryType,
                    MurmurHash3,
                    EntryFactory<ObjectMemberEntryType> > ObjectMemberMapType;

  struct ArchiveMember {
    Input* file;
    InputTree::iterator lastPos;
    InputTree::Mover* move;
  };

 private:
  typedef HashEntry<const llvm::StringRef,
                    ArchiveMember,
                    hash::StringCompare<llvm::StringRef> >
      ArchiveMemberEntryType;

 public:
  typedef HashTable<ArchiveMemberEntryType,
                    hash::StringHash<hash::DJB>,
                    EntryFactory<ArchiveMemberEntryType> > ArchiveMemberMapType;

  struct Symbol {
   public:
    enum Status { Include, Exclude, Unknown };

    Symbol(const char* pName, uint32_t pOffset, enum Status pStatus)
        : name(pName), fileOffset(pOffset), status(pStatus) {}

    ~Symbol() {}

   public:
    std::string name;
    uint32_t fileOffset;
    enum Status status;
  };

  typedef std::vector<Symbol*> SymTabType;

 public:
  Archive(Input& pInputFile, InputBuilder& pBuilder);

  ~Archive();

  /// getARFile - get the Input& of the archive file
  Input& getARFile();

  /// getARFile - get the Input& of the archive file
  const Input& getARFile() const;

  /// inputs - get the input tree built from this archive
  InputTree& inputs();

  /// inputs - get the input tree built from this archive
  const InputTree& inputs() const;

  /// getObjectMemberMap - get the map that contains the included object files
  ObjectMemberMapType& getObjectMemberMap();

  /// getObjectMemberMap - get the map that contains the included object files
  const ObjectMemberMapType& getObjectMemberMap() const;

  /// numOfObjectMember - return the number of included object files
  size_t numOfObjectMember() const;

  /// addObjectMember - add a object in the object member map
  /// @param pFileOffset - file offset in symtab represents a object file
  /// @param pIter - the iterator in the input tree built from this archive
  bool addObjectMember(uint32_t pFileOffset, InputTree::iterator pIter);

  /// hasObjectMember - check if a object file is included or not
  /// @param pFileOffset - file offset in symtab represents a object file
  bool hasObjectMember(uint32_t pFileOffset) const;

  /// getArchiveMemberMap - get the map that contains the included archive files
  ArchiveMemberMapType& getArchiveMemberMap();

  /// getArchiveMemberMap - get the map that contains the included archive files
  const ArchiveMemberMapType& getArchiveMemberMap() const;

  /// addArchiveMember - add an archive in the archive member map
  /// @param pName    - the name of the new archive member
  /// @param pLastPos - this records the point to insert the next node in the
  ///                   subtree of this archive member
  /// @param pMove    - this records the direction to insert the next node in
  ///                   the subtree of this archive member
  bool addArchiveMember(const llvm::StringRef& pName,
                        InputTree::iterator pLastPos,
                        InputTree::Mover* pMove);

  /// hasArchiveMember - check if an archive file is included or not
  bool hasArchiveMember(const llvm::StringRef& pName) const;

  /// getArchiveMember - get a archive member
  ArchiveMember* getArchiveMember(const llvm::StringRef& pName);

  /// getSymbolTable - get the symtab
  SymTabType& getSymbolTable();

  /// getSymbolTable - get the symtab
  const SymTabType& getSymbolTable() const;

  /// setSymTabSize - set the memory size of symtab
  void setSymTabSize(size_t pSize);

  /// getSymTabSize - get the memory size of symtab
  size_t getSymTabSize() const;

  /// numOfSymbols - return the number of symbols in symtab
  size_t numOfSymbols() const;

  /// addSymbol - add a symtab entry to symtab
  /// @param pName - symbol name
  /// @param pFileOffset - file offset in symtab represents a object file
  void addSymbol(const char* pName,
                 uint32_t pFileOffset,
                 enum Symbol::Status pStatus = Archive::Symbol::Unknown);

  /// getSymbolName - get the symbol name with the given index
  const std::string& getSymbolName(size_t pSymIdx) const;

  /// getObjFileOffset - get the file offset that represent a object file
  uint32_t getObjFileOffset(size_t pSymIdx) const;

  /// getSymbolStatus - get the status of a symbol
  enum Symbol::Status getSymbolStatus(size_t pSymIdx) const;

  /// setSymbolStatus - set the status of a symbol
  void setSymbolStatus(size_t pSymIdx, enum Symbol::Status pStatus);

  /// getStrTable - get the extended name table
  std::string& getStrTable();

  /// getStrTable - get the extended name table
  const std::string& getStrTable() const;

  /// hasStrTable - return true if this archive has extended name table
  bool hasStrTable() const;

  /// getMemberFile       - get the member file in an archive member
  /// @param pArchiveFile - Input reference of the archive member
  /// @param pIsThinAR    - denote the archive menber is a Thin Archive or not
  /// @param pName        - the name of the member file we want to get
  /// @param pPath        - the path of the member file
  /// @param pFileOffset  - the file offset of the member file in a regular AR
  Input* getMemberFile(Input& pArchiveFile,
                       bool isThinAR,
                       const std::string& pName,
                       const sys::fs::Path& pPath,
                       off_t pFileOffset = 0);

 private:
  typedef GCFactory<Symbol, 0> SymbolFactory;

 private:
  Input& m_ArchiveFile;
  InputTree* m_pInputTree;
  ObjectMemberMapType m_ObjectMemberMap;
  ArchiveMemberMapType m_ArchiveMemberMap;
  SymbolFactory m_SymbolFactory;
  SymTabType m_SymTab;
  size_t m_SymTabSize;
  std::string m_StrTab;
  InputBuilder& m_Builder;
};

}  // namespace mcld

#endif  // MCLD_LD_ARCHIVE_H_
