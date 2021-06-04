//===- SearchDirs.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/MC/SearchDirs.h"

#include "mcld/MC/MCLDDirectory.h"
#include "mcld/Support/FileSystem.h"

namespace mcld {

//===----------------------------------------------------------------------===//
// Non-member functions
//===----------------------------------------------------------------------===//
static inline void SpecToFilename(const std::string& pSpec,
                                  std::string& pFile) {
  pFile = "lib";
  pFile += pSpec;
}

//===----------------------------------------------------------------------===//
// SearchDirs
//===----------------------------------------------------------------------===//
SearchDirs::SearchDirs() {
  // a magic number 8, no why.
  // please prove it or change it
  m_DirList.reserve(8);
}

SearchDirs::SearchDirs(const sys::fs::Path& pSysRoot) : m_SysRoot(pSysRoot) {
  // a magic number 8, no why.
  // please prove it or change it
  m_DirList.reserve(8);
}

SearchDirs::~SearchDirs() {
  iterator dir, dirEnd = end();
  for (dir = begin(); dir != dirEnd; ++dir) {
    delete (*dir);
  }
}

bool SearchDirs::insert(const std::string& pPath) {
  MCLDDirectory* dir = new MCLDDirectory(pPath);
  if (dir->isInSysroot())
    dir->setSysroot(m_SysRoot);

  if (exists(dir->path()) && is_directory(dir->path())) {
    m_DirList.push_back(dir);
    return true;
  } else {
    delete dir;
    return false;
  }
  return true;
}

bool SearchDirs::insert(const char* pPath) {
  return insert(std::string(pPath));
}

bool SearchDirs::insert(const sys::fs::Path& pPath) {
  return insert(pPath.native());
}

mcld::sys::fs::Path* SearchDirs::find(const std::string& pNamespec,
                                      mcld::Input::Type pType) {
  assert(Input::DynObj == pType || Input::Archive == pType ||
         Input::Script == pType);

  std::string file;
  switch (pType) {
    case Input::Script:
      file.assign(pNamespec);
      break;
    case Input::DynObj:
    case Input::Archive:
      SpecToFilename(pNamespec, file);
      break;
    default:
      break;
  }  // end of switch

  // for all MCLDDirectorys
  DirList::iterator mcld_dir, mcld_dir_end = m_DirList.end();
  for (mcld_dir = m_DirList.begin(); mcld_dir != mcld_dir_end; ++mcld_dir) {
    // for all entries in MCLDDirectory
    MCLDDirectory::iterator entry = (*mcld_dir)->begin();
    MCLDDirectory::iterator enEnd = (*mcld_dir)->end();

    switch (pType) {
      case Input::Script: {
        while (entry != enEnd) {
          if (file == entry.path()->filename().native())
            return entry.path();
          ++entry;
        }
        break;
      }
      case Input::DynObj: {
        while (entry != enEnd) {
          if (file == entry.path()->stem().native()) {
            if (mcld::sys::fs::detail::shared_library_extension ==
                entry.path()->extension().native()) {
              return entry.path();
            }
          }
          ++entry;
        }
      }
      /** Fall through **/
      case Input::Archive: {
        entry = (*mcld_dir)->begin();
        enEnd = (*mcld_dir)->end();
        while (entry != enEnd) {
          if (file == entry.path()->stem().native() &&
              mcld::sys::fs::detail::static_library_extension ==
                  entry.path()->extension().native()) {
            return entry.path();
          }
          ++entry;
        }
      }
      default:
        break;
    }  // end of switch
  }    // end of for
  return NULL;
}

const mcld::sys::fs::Path* SearchDirs::find(const std::string& pNamespec,
                                            mcld::Input::Type pType) const {
  assert(Input::DynObj == pType || Input::Archive == pType ||
         Input::Script == pType);

  std::string file;
  switch (pType) {
    case Input::Script:
      file.assign(pNamespec);
      break;
    case Input::DynObj:
    case Input::Archive:
      SpecToFilename(pNamespec, file);
      break;
    default:
      break;
  }  // end of switch

  // for all MCLDDirectorys
  DirList::const_iterator mcld_dir, mcld_dir_end = m_DirList.end();
  for (mcld_dir = m_DirList.begin(); mcld_dir != mcld_dir_end; ++mcld_dir) {
    // for all entries in MCLDDirectory
    MCLDDirectory::iterator entry = (*mcld_dir)->begin();
    MCLDDirectory::iterator enEnd = (*mcld_dir)->end();

    switch (pType) {
      case Input::Script: {
        while (entry != enEnd) {
          if (file == entry.path()->filename().native())
            return entry.path();
          ++entry;
        }
        break;
      }
      case Input::DynObj: {
        while (entry != enEnd) {
          if (file == entry.path()->stem().native()) {
            if (mcld::sys::fs::detail::shared_library_extension ==
                entry.path()->extension().native()) {
              return entry.path();
            }
          }
          ++entry;
        }
      }
      /** Fall through **/
      case Input::Archive: {
        entry = (*mcld_dir)->begin();
        enEnd = (*mcld_dir)->end();
        while (entry != enEnd) {
          if (file == entry.path()->stem().native() &&
              mcld::sys::fs::detail::static_library_extension ==
                  entry.path()->extension().native()) {
            return entry.path();
          }
          ++entry;
        }
      }
      default:
        break;
    }  // end of switch
  }    // end of for
  return NULL;
}

}  // namespace mcld
