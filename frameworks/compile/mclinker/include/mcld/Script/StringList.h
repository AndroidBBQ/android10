//===- StringList.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_STRINGLIST_H_
#define MCLD_SCRIPT_STRINGLIST_H_

#include "mcld/Config/Config.h"
#include "mcld/Support/Allocators.h"

#include <vector>

namespace mcld {

class StrToken;

/** \class StringList
 *  \brief This class defines the interfaces to StringList.
 */

class StringList {
 public:
  typedef std::vector<StrToken*> Tokens;
  typedef Tokens::const_iterator const_iterator;
  typedef Tokens::iterator iterator;
  typedef Tokens::const_reference const_reference;
  typedef Tokens::reference reference;

 private:
  friend class Chunk<StringList, MCLD_SYMBOLS_PER_INPUT>;
  StringList();

 public:
  ~StringList();

  const_iterator begin() const { return m_Tokens.begin(); }
  iterator begin() { return m_Tokens.begin(); }
  const_iterator end() const { return m_Tokens.end(); }
  iterator end() { return m_Tokens.end(); }

  const_reference front() const { return m_Tokens.front(); }
  reference front() { return m_Tokens.front(); }
  const_reference back() const { return m_Tokens.back(); }
  reference back() { return m_Tokens.back(); }

  bool empty() const { return m_Tokens.empty(); }

  void push_back(StrToken* pToken);

  void dump() const;

  /* factory methods */
  static StringList* create();
  static void destroy(StringList*& pStringList);
  static void clear();

 private:
  Tokens m_Tokens;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_STRINGLIST_H_
