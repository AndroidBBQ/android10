/*===- ScriptScanner.ll ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===*/

%{
/* C/C++ Declarations */

#include "mcld/Script/ScriptScanner.h"
#include "mcld/Script/ScriptFile.h"
#include "mcld/Support/MsgHandling.h"
#include <llvm/ADT/StringRef.h>
#include <string>

typedef mcld::ScriptParser::token token;
typedef mcld::ScriptParser::token_type token_type;

#define yyterminate() return token::END
#define YY_NO_UNISTD_H
%}

%{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#endif
%}

/* Flex Declarations and Options */
%option c++
%option batch
%option noyywrap
%option nounput
%option stack

%{
#define YY_USER_ACTION  yylloc->columns(yyleng);
%}

/* abbrev. of RE @ref binutils ld/ldlex.l */
FILENAMECHAR1   [_a-zA-Z\/\.\\\$\_\~]
SYMBOLCHARN     [_a-zA-Z\/\.\\\$\_\~0-9]
NOCFILENAMECHAR [_a-zA-Z0-9\/\.\-\_\+\$\[\]\\\~]
WILDCHAR        [_a-zA-Z0-9\/\.\-\_\+\$\[\]\\\,\~\?\*\^\!]
WS [ \t\r]

/* Start conditions */
%s LDSCRIPT
%s EXPRESSION

%% /* Regular Expressions */

 /* code to place at the beginning of yylex() */
%{
  /* reset location */
  yylloc->step();

  /* determine the initial parser state */
  if (m_Kind == ScriptFile::Unknown) {
    m_Kind = pScriptFile.getKind();
    switch (pScriptFile.getKind()) {
    case ScriptFile::LDScript:
    case ScriptFile::Expression:
      return token::LINKER_SCRIPT;
    case ScriptFile::VersionScript:
    case ScriptFile::DynamicList:
    default:
      assert(0 && "Unsupported script type!");
      break;
    }
  }
%}

 /* Entry Point */
<LDSCRIPT>"ENTRY"                      { return token::ENTRY; }
 /* File Commands */
<LDSCRIPT>"INCLUDE"                    { return token::INCLUDE; }
<LDSCRIPT>"INPUT"                      { return token::INPUT; }
<LDSCRIPT>"GROUP"                      { return token::GROUP; }
<LDSCRIPT>"AS_NEEDED"                  { return token::AS_NEEDED; }
<LDSCRIPT>"OUTPUT"                     { return token::OUTPUT; }
<LDSCRIPT>"SEARCH_DIR"                 { return token::SEARCH_DIR; }
<LDSCRIPT>"STARTUP"                    { return token::STARTUP; }
 /* Format Commands */
<LDSCRIPT>"OUTPUT_FORMAT"              { return token::OUTPUT_FORMAT; }
<LDSCRIPT>"TARGET"                     { return token::TARGET; }
 /* Misc Commands */
<LDSCRIPT>"ASSERT"                     { return token::ASSERT; }
<LDSCRIPT>"EXTERN"                     { return token::EXTERN; }
<LDSCRIPT>"FORCE_COMMON_ALLOCATION"    { return token::FORCE_COMMON_ALLOCATION; }
<LDSCRIPT>"INHIBIT_COMMON_ALLOCATION"  { return token::INHIBIT_COMMON_ALLOCATION; }
<LDSCRIPT>"INSERT"                     { return token::INSERT; }
<LDSCRIPT>"NOCROSSREFS"                { return token::NOCROSSREFS; }
<LDSCRIPT>"OUTPUT_ARCH"                { return token::OUTPUT_ARCH; }
<LDSCRIPT>"LD_FEATURE"                 { return token::LD_FEATURE; }
 /* Assignemnts */
<LDSCRIPT,EXPRESSION>"HIDDEN"          { return token::HIDDEN; }
<LDSCRIPT,EXPRESSION>"PROVIDE"         { return token::PROVIDE; }
<LDSCRIPT,EXPRESSION>"PROVIDE_HIDDEN"  { return token::PROVIDE_HIDDEN; }
 /* SECTIONS Command */
<LDSCRIPT>"SECTIONS"                   { return token::SECTIONS; }
 /* MEMORY Command */
<LDSCRIPT>"MEMORY"                     { return token::MEMORY; }
 /* PHDRS Command */
<LDSCRIPT>"PHDRS"                      { return token::PHDRS; }
 /* Builtin Functions */
<EXPRESSION>"ABSOLUTE"                 { return token::ABSOLUTE; }
<EXPRESSION>"ADDR"                     { return token::ADDR; }
<LDSCRIPT,EXPRESSION>"ALIGN"           { return token::ALIGN; }
<EXPRESSION>"ALIGNOF"                  { return token::ALIGNOF; }
<EXPRESSION>"BLOCK"                    { return token::BLOCK; }
<EXPRESSION>"DATA_SEGMENT_ALIGN"       { return token::DATA_SEGMENT_ALIGN; }
<EXPRESSION>"DATA_SEGMENT_END"         { return token::DATA_SEGMENT_END; }
<EXPRESSION>"DATA_SEGMENT_RELRO_END"   { return token::DATA_SEGMENT_RELRO_END; }
<EXPRESSION>"DEFINED"                  { return token::DEFINED; }
<EXPRESSION>"LENGTH"                   { return token::LENGTH; }
<EXPRESSION>"LOADADDR"                 { return token::LOADADDR; }
<EXPRESSION>"MAX"                      { return token::MAX; }
<EXPRESSION>"MIN"                      { return token::MIN; }
<EXPRESSION>"NEXT"                     { return token::NEXT; }
<EXPRESSION>"ORIGIN"                   { return token::ORIGIN; }
<EXPRESSION>"SEGMENT_START"            { return token::SEGMENT_START; }
<EXPRESSION>"SIZEOF"                   { return token::SIZEOF; }
<EXPRESSION>"SIZEOF_HEADERS"           { return token::SIZEOF_HEADERS; }
<EXPRESSION>"CONSTANT"                 { return token::CONSTANT; }
 /* Symbolic Constants */
<EXPRESSION>"MAXPAGESIZE"              { return token::MAXPAGESIZE; }
<EXPRESSION>"COMMONPAGESIZE"           { return token::COMMONPAGESIZE; }
 /* Input Section Description */
<LDSCRIPT>"EXCLUDE_FILE"               { return token::EXCLUDE_FILE; }
<LDSCRIPT>"KEEP"                       { return token::KEEP; }
<LDSCRIPT>"SORT"                       { return token::SORT_BY_NAME; }
<LDSCRIPT>"SORT_BY_NAME"               { return token::SORT_BY_NAME; }
<LDSCRIPT>"SORT_BY_ALIGNMENT"          { return token::SORT_BY_ALIGNMENT; }
<LDSCRIPT>"SORT_NONE"                  { return token::SORT_NONE; }
<LDSCRIPT>"SORT_BY_INIT_PRIORITY"      { return token::SORT_BY_INIT_PRIORITY; }
 /* Output Section Data */
<LDSCRIPT>"BYTE"                       { return token::BYTE; }
<LDSCRIPT>"SHORT"                      { return token::SHORT; }
<LDSCRIPT>"LONG"                       { return token::LONG; }
<LDSCRIPT>"QUAD"                       { return token::QUAD; }
<LDSCRIPT>"SQUAD"                      { return token::SQUAD; }
<LDSCRIPT>"FILL"                       { return token::FILL; }
 /* Output Section Discarding */
<LDSCRIPT>"DISCARD"                    { return token::DISCARD; }
 /* Output Section Keywords */
<LDSCRIPT>"CREATE_OBJECT_SYMBOLS"      { return token::CREATE_OBJECT_SYMBOLS; }
<LDSCRIPT>"CONSTRUCTORS"               { return token::CONSTRUCTORS; }
 /* Output Section Attributes */
 /* Output Section Type */
<LDSCRIPT,EXPRESSION>"NOLOAD"          { return token::NOLOAD; }
<LDSCRIPT,EXPRESSION>"DSECT"           { return token::DSECT; }
<LDSCRIPT,EXPRESSION>"COPY"            { return token::COPY; }
<LDSCRIPT,EXPRESSION>"INFO"            { return token::INFO; }
<LDSCRIPT,EXPRESSION>"OVERLAY"         { return token::OVERLAY; }
 /* Output Section LMA */
<LDSCRIPT>"AT"                         { return token::AT; }
 /* Forced Input Alignment */
<LDSCRIPT>"SUBALIGN"                   { return token::SUBALIGN; }
 /* Output Section Constraint */
<LDSCRIPT>"ONLY_IF_RO"                 { return token::ONLY_IF_RO; }
<LDSCRIPT>"ONLY_IF_RW"                 { return token::ONLY_IF_RW; }
 /* Operators */
<LDSCRIPT,EXPRESSION>"<<"              { return token::LSHIFT; }
<LDSCRIPT,EXPRESSION>">>"              { return token::RSHIFT; }
<LDSCRIPT,EXPRESSION>"=="              { return token::EQ; }
<LDSCRIPT,EXPRESSION>"!="              { return token::NE; }
<LDSCRIPT,EXPRESSION>"<="              { return token::LE; }
<LDSCRIPT,EXPRESSION>">="              { return token::GE; }
<LDSCRIPT,EXPRESSION>"&&"              { return token::LOGICAL_AND; }
<LDSCRIPT,EXPRESSION>"||"              { return token::LOGICAL_OR; }
<LDSCRIPT,EXPRESSION>"+="              { return token::ADD_ASSIGN; }
<LDSCRIPT,EXPRESSION>"-="              { return token::SUB_ASSIGN; }
<LDSCRIPT,EXPRESSION>"*="              { return token::MUL_ASSIGN; }
<LDSCRIPT,EXPRESSION>"/="              { return token::DIV_ASSIGN; }
<LDSCRIPT,EXPRESSION>"&="              { return token::AND_ASSIGN; }
<LDSCRIPT,EXPRESSION>"|="              { return token::OR_ASSIGN; }
<LDSCRIPT,EXPRESSION>"<<="             { return token::LS_ASSIGN; }
<LDSCRIPT,EXPRESSION>">>="             { return token::RS_ASSIGN; }
<LDSCRIPT,EXPRESSION>","               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"="               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"?"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>":"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"|"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"^"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"&"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"<"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>">"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"+"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"-"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"*"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"/"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"%"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"!"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"~"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>";"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"("               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>")"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"{"               { return static_cast<token_type>(*yytext); }
<LDSCRIPT,EXPRESSION>"}"               { return static_cast<token_type>(*yytext); }

 /* Numbers */
<LDSCRIPT,EXPRESSION>((("$"|0[xX])([0-9A-Fa-f])+)|(([0-9])+))(M|K|m|k)? {
  llvm::StringRef str(yytext, yyleng);
  switch (str.back()) {
  case 'k':
  case 'K':
    str.substr(0, yyleng - 1).getAsInteger(0, yylval->integer);
    yylval->integer *= 1024;
    break;
  case 'm':
  case 'M':
    str.substr(0, yyleng - 1).getAsInteger(0, yylval->integer);
    yylval->integer *= 1024 * 1024;
    break;
  default:
    str.getAsInteger(0, yylval->integer);
    break;
  }
  return token::INTEGER;
}

 /* Expression string */
<EXPRESSION>{FILENAMECHAR1}{SYMBOLCHARN}* {
  const std::string& str = pScriptFile.createParserStr(yytext, yyleng);
  yylval->string = &str;
  return token::STRING;
}

 /* String */
<LDSCRIPT>{FILENAMECHAR1}{NOCFILENAMECHAR}* {
  const std::string& str = pScriptFile.createParserStr(yytext, yyleng);
  yylval->string = &str;
  return token::STRING;
}

<LDSCRIPT,EXPRESSION>\"(\\.|[^\\"])*\" {
  /*" c string literal */
  const std::string& str = pScriptFile.createParserStr(yytext, yyleng);
  yylval->string = &str;
  return token::STRING;
}

 /* -l namespec */
<LDSCRIPT>"-l"{FILENAMECHAR1}{NOCFILENAMECHAR}* {
  const std::string& str = pScriptFile.createParserStr(yytext + 2, yyleng - 2);
  yylval->string = &str;
  return token::LNAMESPEC;
}

 /* WILDCHAR String */
<LDSCRIPT>{WILDCHAR}* {
  if (yytext[0] == '/' && yytext[1] == '*') {
    yyless (2);
    enterComments(*yylloc);
  } else {
    const std::string& str = pScriptFile.createParserStr(yytext, yyleng);
    yylval->string = &str;
    return token::STRING;
  }
}

 /* gobble up C comments */
<LDSCRIPT,EXPRESSION>"/*" {
  enterComments(*yylloc);
  yylloc->step();
}

 /* gobble up white-spaces */
<LDSCRIPT,EXPRESSION>{WS}+ {
  yylloc->step();
}

 /* gobble up end-of-lines */
<LDSCRIPT,EXPRESSION>\n {
  yylloc->lines(1);
  yylloc->step();
}

%% /* Additional Code */

namespace mcld {

ScriptScanner::ScriptScanner(std::istream* yyin, std::ostream* yyout)
  : yyFlexLexer(yyin, yyout), m_Kind(ScriptFile::Unknown)
{
}

ScriptScanner::~ScriptScanner()
{
}

void ScriptScanner::enterComments(ScriptParser::location_type& pLocation)
{
  const int start_line = pLocation.begin.line;
  const int start_col  = pLocation.begin.column;

  int ch = 0;

  while (true) {
    ch = yyinput();
    pLocation.columns(1);

    while (ch != '*' && ch != EOF) {
      if (ch == '\n') {
        pLocation.lines(1);
      }

      ch = yyinput();
      pLocation.columns(1);
    }

    if (ch == '*') {
      ch = yyinput();
      pLocation.columns(1);

      while (ch == '*') {
        ch = yyinput();
        pLocation.columns(1);
      }

      if (ch == '/')
        break;
    }

    if (ch == '\n')
      pLocation.lines(1);

    if (ch == EOF) {
      error(diag::err_unterminated_comment) << pLocation.begin.filename
                                            << start_line
                                            << start_col;
      break;
    }
  }
}

void ScriptScanner::setLexState(ScriptFile::Kind pKind)
{
  /* push the state into the top of stach */
  m_StateStack.push(pKind);

  switch (pKind) {
  case ScriptFile::LDScript:
    BEGIN(LDSCRIPT);
    break;
  case ScriptFile::Expression:
    BEGIN(EXPRESSION);
    break;
  case ScriptFile::VersionScript:
  case ScriptFile::DynamicList:
  default:
    assert(0 && "Unsupported script type!");
    break;
  }
}

void ScriptScanner::popLexState()
{
  /* pop the last state */
  m_StateStack.pop();

  /* resume the appropriate state */
  if (!m_StateStack.empty()) {
    switch (m_StateStack.top()) {
    case ScriptFile::LDScript:
      BEGIN(LDSCRIPT);
      break;
    case ScriptFile::Expression:
      BEGIN(EXPRESSION);
      break;
    case ScriptFile::VersionScript:
    case ScriptFile::DynamicList:
    default:
      assert(0 && "Unsupported script type!");
      break;
    }
  }
}

} /* namespace mcld */

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef yylex
#undef yylex
#endif

int yyFlexLexer::yylex()
{
  return 0;
}

