//===- GNUArchiveReader.h -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_GNUARCHIVEREADER_H_
#define MCLD_LD_GNUARCHIVEREADER_H_

#include "mcld/LD/Archive.h"
#include "mcld/LD/ArchiveReader.h"

namespace mcld {

class Archive;
class ELFObjectReader;
class Input;
class LinkerConfig;
class Module;

/** \class GNUArchiveReader
 *  \brief GNUArchiveReader reads GNU archive files.
 */
class GNUArchiveReader : public ArchiveReader {
 public:
  GNUArchiveReader(Module& pModule, ELFObjectReader& pELFObjectReader);

  ~GNUArchiveReader();

  /// readArchive - read an archive, include the needed members, and build up
  /// the subtree
  bool readArchive(const LinkerConfig& pConfig, Archive& pArchive);

  /// isMyFormat
  bool isMyFormat(Input& input, bool& pContinue) const;

 private:
  /// isArchive
  bool isArchive(const char* pStr) const;

  /// isThinArchive
  bool isThinArchive(const char* pStr) const;

  /// isThinArchive
  bool isThinArchive(Input& input) const;

  /// readMemberHeader - read the header of a member in a archive file and then
  /// return the corresponding archive member (it may be an input object or
  /// another archive)
  /// @param pArchiveRoot  - the archive root that holds the strtab (extended
  ///                        name table)
  /// @param pArchiveFile  - the archive that contains the needed object
  /// @param pFileOffset   - file offset of the member header in the archive
  /// @param pNestedOffset - used when we find a nested archive
  /// @param pMemberSize   - the file size of this member
  Input* readMemberHeader(Archive& pArchiveRoot,
                          Input& pArchiveFile,
                          uint32_t pFileOffset,
                          uint32_t& pNestedOffset,
                          size_t& pMemberSize);

  /// readSymbolTable - read the archive symbol map (armap)
  bool readSymbolTable(Archive& pArchive);

  /// readStringTable - read the strtab for long file name of the archive
  bool readStringTable(Archive& pArchive);

  /// shouldIncludeSymbol - given a sym name from armap and check if we should
  /// include the corresponding archive member, and then return the decision
  enum Archive::Symbol::Status shouldIncludeSymbol(
      const llvm::StringRef& pSymName) const;

  /// includeMember - include the object member in the given file offset, and
  /// return the size of the object
  /// @param pConfig - LinkerConfig
  /// @param pArchiveRoot - the archive root
  /// @param pFileOffset  - file offset of the member header in the archive
  size_t includeMember(const LinkerConfig& pConfig,
                       Archive& pArchiveRoot,
                       uint32_t pFileOffset);

  /// includeAllMembers - include all object members. This is called if
  /// --whole-archive is the attribute for this archive file.
  bool includeAllMembers(const LinkerConfig& pConfig, Archive& pArchive);

 private:
  Module& m_Module;
  ELFObjectReader& m_ELFObjectReader;
};

}  // namespace mcld

#endif  // MCLD_LD_GNUARCHIVEREADER_H_
