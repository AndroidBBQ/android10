//===- ScriptScanner.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_SCRIPTSCANNER_H_
#define MCLD_SCRIPT_SCRIPTSCANNER_H_

#ifndef __FLEX_LEXER_H
#ifdef ANDROID
#include "mcld/Script/FlexLexer.h"
#else
#include <FlexLexer.h>
#endif
#endif

#ifndef YY_DECL
#define YY_DECL                                            \
  mcld::ScriptParser::token_type mcld::ScriptScanner::lex( \
      mcld::ScriptParser::semantic_type* yylval,           \
      mcld::ScriptParser::location_type* yylloc,           \
      const mcld::ScriptFile& pScriptFile)
#endif

#include "mcld/Script/ScriptFile.h"
#include "ScriptParser.h"
#include <stack>

namespace mcld {

/** \class ScriptScanner
 *
 */
class ScriptScanner : public yyFlexLexer {
 public:
  explicit ScriptScanner(std::istream* yyin = NULL, std::ostream* yyout = NULL);

  virtual ~ScriptScanner();

  virtual ScriptParser::token_type lex(ScriptParser::semantic_type* yylval,
                                       ScriptParser::location_type* yylloc,
                                       const ScriptFile& pScriptFile);

  void setLexState(ScriptFile::Kind pKind);

  void popLexState();

 private:
  void enterComments(ScriptParser::location_type& pLocation);

 private:
  ScriptFile::Kind m_Kind;
  std::stack<ScriptFile::Kind> m_StateStack;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_SCRIPTSCANNER_H_
