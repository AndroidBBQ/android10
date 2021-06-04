# flex+bison settings
find_package(BISON)
find_package(FLEX)
BISON_TARGET(PARSER ScriptParser.yy ${CMAKE_CURRENT_BINARY_DIR}/ScriptParser.cpp)
FLEX_TARGET(LEXER ScriptScanner.ll ${CMAKE_CURRENT_BINARY_DIR}/ScriptScanner.cpp)
ADD_FLEX_BISON_DEPENDENCY(LEXER PARSER)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_llvm_library(MCLDScript
  AssertCmd.cpp
  Assignment.cpp
  BinaryOp.cpp
  EntryCmd.cpp
  FileToken.cpp
  GroupCmd.cpp
  InputCmd.cpp
  InputSectDesc.cpp
  InputToken.cpp
  NameSpec.cpp
  NullaryOp.cpp
  Operand.cpp
  Operator.cpp
  OutputArchCmd.cpp
  OutputCmd.cpp
  OutputFormatCmd.cpp
  OutputSectDesc.cpp
  RpnEvaluator.cpp
  RpnExpr.cpp
  ScriptCommand.cpp
  ScriptFile.cpp
  ScriptReader.cpp
  SearchDirCmd.cpp
  SectionsCmd.cpp
  StrToken.cpp
  StringList.cpp
  TernaryOp.cpp
  UnaryOp.cpp
  WildcardPattern.cpp
  ${BISON_PARSER_OUTPUTS}
  ${FLEX_LEXER_OUTPUTS}
  )
