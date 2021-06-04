//===- WildcardPattern.h --------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_WILDCARDPATTERN_H_
#define MCLD_SCRIPT_WILDCARDPATTERN_H_

#include "mcld/Config/Config.h"
#include "mcld/Script/StrToken.h"
#include "mcld/Support/Allocators.h"

#include <llvm/ADT/StringRef.h>

namespace mcld {

/** \class WildcardPattern
 *  \brief This class defines the interfaces to Input Section Wildcard Patterns
 */

class WildcardPattern : public StrToken {
 public:
  enum SortPolicy {
    SORT_NONE,
    SORT_BY_NAME,
    SORT_BY_ALIGNMENT,
    SORT_BY_NAME_ALIGNMENT,
    SORT_BY_ALIGNMENT_NAME,
    SORT_BY_INIT_PRIORITY
  };

 private:
  friend class Chunk<WildcardPattern, MCLD_SYMBOLS_PER_INPUT>;
  WildcardPattern();
  WildcardPattern(const std::string& pPattern, SortPolicy pPolicy);

 public:
  ~WildcardPattern();

  SortPolicy sortPolicy() const { return m_SortPolicy; }

  bool isPrefix() const { return m_bIsPrefix; }

  llvm::StringRef prefix() const;

  static bool classof(const StrToken* pToken) {
    return pToken->kind() == StrToken::Wildcard;
  }

  /* factory method */
  static WildcardPattern* create(const std::string& pPattern,
                                 SortPolicy pPolicy);
  static void destroy(WildcardPattern*& pToken);
  static void clear();

 private:
  SortPolicy m_SortPolicy;
  bool m_bIsPrefix;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_WILDCARDPATTERN_H_
