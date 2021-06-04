//===- PathV3.inc ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Support/FileSystem.h"
#include "mcld/Support/Path.h"

#include <llvm/Support/ErrorHandling.h>

#include <cerrno>
#include <stack>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace mcld {
namespace sys {
namespace fs {

//===----------------------------------------------------------------------===//
// mcld::sys::fs::detail
//===----------------------------------------------------------------------===//
namespace detail {

// return the last charactor being handled.
size_t canonicalize(std::string& pathname) {
  // Variable Index //
  // SepTable - stack of result separators
  // LR(1) Algorithm //
  // traverse pPathName
  //   if we meet '//', '///', '////', ...
  //     -> ignore it
  //     -> push current into stack
  //     -> jump to the next not '/'
  //   if we meet '/./'
  //     -> ignore
  //     -> jump to the next not '/'
  //   if we meet '/../'
  //     -> pop previous position of '/' P
  //     -> erase P+1 to now
  //   if we meet other else
  //     -> go go go
  //   if we meet '/.../', '/..../', ... -> illegal
  if (pathname.empty())
    return 0;

  size_t handler = 0;
  std::stack<size_t> slash_stack;
  slash_stack.push(-1);
  while (handler < pathname.size()) {
    if (separator == pathname[handler]) {  // handler = 1st '/'
      size_t next = handler + 1;
      if (next >= pathname.size())
        return handler;
      switch (pathname[next]) {  // next = handler + 1;
        case separator: {        // '//'
          while (next < pathname.size() && separator == pathname[next])
            ++next;
          // next is the last not '/'
          pathname.erase(handler, next - handler - 1);
          // handler is the first '/'
          slash_stack.push(handler);
          break;
        }
        case '.': {                     // '/.'
          ++next;                       // next = handler + 2
          if (next >= pathname.size())  // '/.'
            return handler;
          switch (pathname[next]) {
            case separator: {  // '/./'
              pathname.erase(handler, 2);
              break;
            }
            case '.': {                     // '/..'
              ++next;                       // next = handler + 3;
              if (next >= pathname.size())  // '/..?'
                return handler;
              switch (pathname[next]) {
                case separator: {  // '/../'
                  handler = slash_stack.top();
                  slash_stack.pop();
                  pathname.erase(handler + 1, next - handler);
                  if (static_cast<size_t>(-1) == handler) {
                    slash_stack.push(-1);
                    handler = pathname.find_first_of(separator, handler);
                  }
                  break;
                }
                case '.': {  // '/...', illegal
                  return handler;
                  break;
                }
                default: {  // '/..a'
                  slash_stack.push(handler);
                  handler = pathname.find_first_of(separator, handler + 3);
                  break;
                }
              }
              break;
            }
            default: {  // '/.a'
              slash_stack.push(handler);
              handler = pathname.find_first_of(separator, handler + 2);
              break;
            }
          }
          break;
        }
        default: {  // '/a
          slash_stack.push(handler);
          handler = pathname.find_first_of(separator, handler + 1);
          break;
        }
      }
    } else {
      handler = pathname.find_first_of(separator, handler);
    }
  }
  return handler;
}

bool not_found_error(int perrno) {
  return perrno == ENOENT || perrno == ENOTDIR;
}

void status(const Path& p, FileStatus& pFileStatus) {
  struct stat path_stat;
  if (stat(p.c_str(), &path_stat) != 0) {
    if (not_found_error(errno)) {
      pFileStatus.setType(FileNotFound);
    } else
      pFileStatus.setType(StatusError);
  } else if (S_ISDIR(path_stat.st_mode))
    pFileStatus.setType(DirectoryFile);
  else if (S_ISREG(path_stat.st_mode))
    pFileStatus.setType(RegularFile);
  else if (S_ISBLK(path_stat.st_mode))
    pFileStatus.setType(BlockFile);
  else if (S_ISCHR(path_stat.st_mode))
    pFileStatus.setType(CharacterFile);
  else if (S_ISFIFO(path_stat.st_mode))
    pFileStatus.setType(FifoFile);
  else if (S_ISSOCK(path_stat.st_mode))
    pFileStatus.setType(SocketFile);
  else
    pFileStatus.setType(TypeUnknown);
}

void symlink_status(const Path& p, FileStatus& pFileStatus) {
  struct stat path_stat;
  if (lstat(p.c_str(), &path_stat) != 0) {
    if (errno == ENOENT || errno == ENOTDIR)  // these are not errors
    {
      pFileStatus.setType(FileNotFound);
    } else
      pFileStatus.setType(StatusError);
  }
  if (S_ISREG(path_stat.st_mode))
    pFileStatus.setType(RegularFile);
  if (S_ISDIR(path_stat.st_mode))
    pFileStatus.setType(DirectoryFile);
  if (S_ISLNK(path_stat.st_mode))
    pFileStatus.setType(SymlinkFile);
  if (S_ISBLK(path_stat.st_mode))
    pFileStatus.setType(BlockFile);
  if (S_ISCHR(path_stat.st_mode))
    pFileStatus.setType(CharacterFile);
  if (S_ISFIFO(path_stat.st_mode))
    pFileStatus.setType(FifoFile);
  if (S_ISSOCK(path_stat.st_mode))
    pFileStatus.setType(SocketFile);
  else
    pFileStatus.setType(TypeUnknown);
}

/// directory_iterator_increment - increment function implementation
//
//  iterator will call this function in two situations:
//  1. All elements have been put into cache, and iterator stays at the end
//     of cache. (a real end)
//  2. Some but not all elements had been put into cache, and we stoped.
//     An iterator now is staying at the end of cache. (a temporal end)
mcld::sys::fs::PathCache::entry_type* bring_one_into_cache(DirIterator& pIter) {
  mcld::sys::fs::PathCache::entry_type* entry = 0;
  std::string path(pIter.m_pParent->m_Path.native());
  switch (read_dir(pIter.m_pParent->m_Handler, path)) {
    case 1: {
      // read one
      bool exist = false;
      entry = pIter.m_pParent->m_Cache.insert(path, exist);
      if (!exist)
        entry->setValue(sys::fs::Path(path));
      break;
    }
    case 0:  // meet real end
      pIter.m_pParent->m_CacheFull = true;
      break;
    default:
    case -1:
      llvm::report_fatal_error(std::string("Can't read directory: ") +
                               pIter.m_pParent->path().native());
      break;
  }
  return entry;
}

}  // namespace detail
}  // namespace fs
}  // namespace sys
}  // namespace mcld
