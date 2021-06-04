//===- GroupReader.h ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_GROUPREADER_H_
#define MCLD_LD_GROUPREADER_H_

#include "mcld/Module.h"

namespace mcld {
class Archive;
class ArchiveReader;
class BinaryReader;
class DynObjReader;
class InputBuilder;
class LinkerConfig;
class ObjectReader;

/** \class GroupReader
 *  \brief GroupReader handles the Group Node in InputTree
 *
 *  Group Node is the root of sub-tree in InputTree which includes the iputs in
 *  the command line options --start-group and --end-group options
 */
class GroupReader {
 public:
  GroupReader(Module& pModule,
              ObjectReader& pObjectReader,
              DynObjReader& pDynObjReader,
              ArchiveReader& pArchiveReader,
              BinaryReader& pBinaryReader);

  ~GroupReader();

  /// readGroup - handle the input sub-tree wich its root is pRoot
  /// @param pRoot - the root Group node of the sub-tree
  bool readGroup(Module::input_iterator pRoot,
                 Module::input_iterator pEnd,
                 InputBuilder& pBuilder,
                 const LinkerConfig& pConfig);

 private:
  /// ArchiveListEntry - record the Archive and the corresponding input iterator
  /// of the archive node
  struct ArchiveListEntry {
    ArchiveListEntry(Archive& pArchive, Module::input_iterator pIterator)
        : archive(pArchive), input(pIterator) {}
    Archive& archive;
    Module::input_iterator input;
  };

 private:
  Module& m_Module;
  ObjectReader& m_ObjectReader;
  DynObjReader& m_DynObjReader;
  ArchiveReader& m_ArchiveReader;
  BinaryReader& m_BinaryReader;
};

}  // namespace mcld

#endif  // MCLD_LD_GROUPREADER_H_
