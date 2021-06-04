//===- Directory.h --------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SUPPORT_DIRECTORY_H_
#define MCLD_SUPPORT_DIRECTORY_H_

#include "mcld/ADT/TypeTraits.h"
#include "mcld/Support/FileSystem.h"
#include "mcld/Support/Path.h"
#include "mcld/Support/PathCache.h"

#include <llvm/Support/Allocator.h>
#include <cstddef>

namespace mcld {
namespace sys {
namespace fs {

class DirIterator;

/** \class Directory
 *  \brief A Directory object stores a Path object, a FileStatus object for
 *   non-symbolic link status, and a FileStatus object for symbolic link
 *   status. The FileStatus objects act as value caches.
 */
class Directory {
  friend mcld::sys::fs::PathCache::entry_type* detail::bring_one_into_cache(
      DirIterator& pIter);
  friend void detail::open_dir(Directory& pDir);
  friend void detail::close_dir(Directory& pDir);

 private:
  friend class DirIterator;

 public:
  typedef DirIterator iterator;

 public:
  /// default constructor
  Directory();

  /// constructor - a directory whose path is pPath
  explicit Directory(const Path& pPath,
                     FileStatus st = FileStatus(),
                     FileStatus symlink_st = FileStatus());

  explicit Directory(const char* pPath,
                     FileStatus st = FileStatus(),
                     FileStatus symlink_st = FileStatus());

  /// copy constructor
  /// when a copying construction happens, the cache is not copied.
  Directory(const Directory& pCopy);

  /// assignment
  /// When an assignment occurs, the cache is clear.
  Directory& operator=(const Directory& pCopy);

  /// destructor, inheritable.
  virtual ~Directory();

  /// Since we have default construtor, we must provide assign.
  void assign(const Path& pPath,
              FileStatus st = FileStatus(),
              FileStatus symlink_st = FileStatus());

  /// clear - clear the cache and close the directory handler
  void clear();

  bool isGood() const;

  /// path - the path of the directory
  const Path& path() const { return m_Path; }

  FileStatus status() const;
  FileStatus symlinkStatus() const;

  // -----  iterators  ----- //
  // While the iterators move, the direcotry is modified.
  // Thus, we only provide non-constant iterator.
  iterator begin();
  iterator end();

 protected:
  mcld::sys::fs::Path m_Path;
  mutable FileStatus m_FileStatus;
  mutable FileStatus m_SymLinkStatus;
  intptr_t m_Handler;
  // the cache of directory
  mcld::sys::fs::PathCache m_Cache;
  bool m_CacheFull;
};

/** \class DirIterator
 *  \brief A DirIterator object can traverse all entries in a Directory
 *
 *  DirIterator will open the directory and add entry into Directory::m_Cache
 *  DirIterator() is the end of a directory.
 *  If the end of the directory elements is reached, the iterator becomes
 *  equal to the end iterator value - DirIterator().
 *
 *  @see Directory
 */
class DirIterator {
  friend mcld::sys::fs::PathCache::entry_type* detail::bring_one_into_cache(
      DirIterator& pIter);
  friend class Directory;

 public:
  typedef mcld::sys::fs::PathCache DirCache;

 public:
  typedef Directory value_type;
  typedef ConstTraits<Directory> const_traits;
  typedef NonConstTraits<Directory> non_const_traits;
  typedef std::input_iterator_tag iterator_category;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

 private:
  explicit DirIterator(Directory* pParent, const DirCache::iterator& pIter);

 public:
  // Since StringMapIterator has no default constructor, we also have none.
  DirIterator(const DirIterator& X);
  ~DirIterator();
  DirIterator& operator=(const DirIterator& pCopy);

  DirIterator& operator++();
  DirIterator operator++(int);

  Path* generic_path();

  Path* path();
  const Path* path() const;

  bool operator==(const DirIterator& y) const;
  bool operator!=(const DirIterator& y) const;

 private:
  Directory* m_pParent;       // get handler
  DirCache::iterator m_Iter;  // for full situation
  DirCache::entry_type* m_pEntry;
};

}  // namespace fs
}  // namespace sys
}  // namespace mcld

#endif  // MCLD_SUPPORT_DIRECTORY_H_
