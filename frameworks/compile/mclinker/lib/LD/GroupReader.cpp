//===- GroupReader.cpp ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/GroupReader.h"

#include "mcld/LD/Archive.h"
#include "mcld/LD/ArchiveReader.h"
#include "mcld/LD/BinaryReader.h"
#include "mcld/LD/DynObjReader.h"
#include "mcld/LD/ObjectReader.h"
#include "mcld/LinkerConfig.h"
#include "mcld/MC/Attribute.h"
#include "mcld/Support/MsgHandling.h"

namespace mcld {

GroupReader::GroupReader(Module& pModule,
                         ObjectReader& pObjectReader,
                         DynObjReader& pDynObjReader,
                         ArchiveReader& pArchiveReader,
                         BinaryReader& pBinaryReader)
    : m_Module(pModule),
      m_ObjectReader(pObjectReader),
      m_DynObjReader(pDynObjReader),
      m_ArchiveReader(pArchiveReader),
      m_BinaryReader(pBinaryReader) {
}

GroupReader::~GroupReader() {
}

bool GroupReader::readGroup(Module::input_iterator pRoot,
                            Module::input_iterator pEnd,
                            InputBuilder& pBuilder,
                            const LinkerConfig& pConfig) {
  // record the number of total objects included in this sub-tree
  size_t cur_obj_cnt = 0;
  size_t last_obj_cnt = 0;
  size_t non_ar_obj_cnt = 0;

  // record the archive files in this sub-tree
  typedef std::vector<ArchiveListEntry*> ArchiveListType;
  ArchiveListType ar_list;

  Module::input_iterator input = --pRoot;

  // first time read the sub-tree
  while (input != pEnd) {
    // already got type - for example, bitcode or external OIR (object
    // intermediate representation)
    if ((*input)->type() == Input::Script ||
        (*input)->type() == Input::Archive ||
        (*input)->type() == Input::External) {
      ++input;
      continue;
    }

    if (Input::Object == (*input)->type()) {
      m_Module.getObjectList().push_back(*input);
      continue;
    }

    if (Input::DynObj == (*input)->type()) {
      m_Module.getLibraryList().push_back(*input);
      continue;
    }

    bool doContinue = false;
    // is an archive
    if (m_ArchiveReader.isMyFormat(**input, doContinue)) {
      (*input)->setType(Input::Archive);
      // record the Archive used by each archive node
      Archive* ar = new Archive(**input, pBuilder);
      ArchiveListEntry* entry = new ArchiveListEntry(*ar, input);
      ar_list.push_back(entry);
      // read archive
      m_ArchiveReader.readArchive(pConfig, *ar);
      cur_obj_cnt += ar->numOfObjectMember();
    } else if (doContinue && m_BinaryReader.isMyFormat(**input, doContinue)) {
      // read input as a binary file
      (*input)->setType(Input::Object);
      m_BinaryReader.readBinary(**input);
      m_Module.getObjectList().push_back(*input);
    } else if (doContinue && m_ObjectReader.isMyFormat(**input, doContinue)) {
      // is a relocatable object file
      (*input)->setType(Input::Object);
      m_ObjectReader.readHeader(**input);
      m_ObjectReader.readSections(**input);
      m_ObjectReader.readSymbols(**input);
      m_Module.getObjectList().push_back(*input);
      ++cur_obj_cnt;
      ++non_ar_obj_cnt;
    } else if (doContinue && m_DynObjReader.isMyFormat(**input, doContinue)) {
      // is a shared object file
      (*input)->setType(Input::DynObj);
      m_DynObjReader.readHeader(**input);
      m_DynObjReader.readSymbols(**input);
      m_Module.getLibraryList().push_back(*input);
    } else {
      warning(diag::warn_unrecognized_input_file)
          << (*input)->path() << pConfig.targets().triple().str();
    }
    ++input;
  }

  // after read in all the archives, traverse the archive list in a loop until
  // there is no unresolved symbols added
  ArchiveListType::iterator it = ar_list.begin();
  ArchiveListType::iterator end = ar_list.end();
  while (cur_obj_cnt != last_obj_cnt) {
    last_obj_cnt = cur_obj_cnt;
    cur_obj_cnt = non_ar_obj_cnt;
    for (it = ar_list.begin(); it != end; ++it) {
      Archive& ar = (*it)->archive;
      // if --whole-archive is given to this archive, no need to read it again
      if (ar.getARFile().attribute()->isWholeArchive())
        continue;
      m_ArchiveReader.readArchive(pConfig, ar);
      cur_obj_cnt += ar.numOfObjectMember();
    }
  }

  // after all needed member included, merge the archive sub-tree to main
  // InputTree
  for (it = ar_list.begin(); it != end; ++it) {
    Archive& ar = (*it)->archive;
    if (ar.numOfObjectMember() > 0) {
      m_Module.getInputTree().merge<InputTree::Inclusive>((*it)->input,
                                                          ar.inputs());
    }
  }

  // cleanup ar_list
  for (it = ar_list.begin(); it != end; ++it) {
    delete &((*it)->archive);
    delete (*it);
  }
  ar_list.clear();

  return true;
}

}  // namespace mcld
