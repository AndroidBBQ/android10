//===- StrToken.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_STRTOKEN_H_
#define MCLD_SCRIPT_STRTOKEN_H_

#include "mcld/Config/Config.h"
#include "mcld/Support/Allocators.h"

#include <string>

namespace mcld {

/** \class StrToken
 *  \brief This class defines the interfaces to a element in EXCLUDE_FILE list
 *         or in Output Section Phdr, or be a base class of other str token.
 */

class StrToken {
 public:
  enum Kind { Unknown, String, Input, Wildcard };

 private:
  friend class Chunk<StrToken, MCLD_SYMBOLS_PER_INPUT>;

 protected:
  StrToken();
  StrToken(Kind pKind, const std::string& pString);

 public:
  virtual ~StrToken();

  Kind kind() const { return m_Kind; }

  const std::string& name() const { return m_Name; }

  static bool classof(const StrToken* pToken) {
    return pToken->kind() == StrToken::String;
  }

  /* factory method */
  static StrToken* create(const std::string& pString);
  static void destroy(StrToken*& pToken);
  static void clear();

 private:
  Kind m_Kind;
  std::string m_Name;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_STRTOKEN_H_
