//===- Diagnostic.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/Diagnostic.h"

#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ADT/Twine.h>

#include <algorithm>

#include <ctype.h>

namespace mcld {

//===----------------------------------------------------------------------===//
//  Diagnostic
Diagnostic::Diagnostic(DiagnosticEngine& pEngine) : m_Engine(pEngine) {
}

Diagnostic::~Diagnostic() {
}

// format - format this diagnostic into string, subsituting the formal
// arguments. The result is appended at on the pOutStr.
void Diagnostic::format(std::string& pOutStr) const {
  // we've not implemented DWARF LOC messages yet. So, keep pIsLoC false
  llvm::StringRef desc = m_Engine.infoMap().getDescription(getID(), false);

  format(desc.begin(), desc.end(), pOutStr);
}

const char* Diagnostic::findMatch(char pVal,
                                  const char* pBegin,
                                  const char* pEnd) const {
  unsigned int depth = 0;
  for (; pBegin != pEnd; ++pBegin) {
    if (depth == 0 && *pBegin == pVal)
      return pBegin;
    if (depth != 0 && *pBegin == '}')
      --depth;

    if (*pBegin == '%') {
      ++pBegin;
      if (pBegin == pEnd)
        break;

      if (!isdigit(*pBegin) && !ispunct(*pBegin)) {
        ++pBegin;
        while (pBegin != pEnd && !isdigit(*pBegin) && *pBegin != '{')
          ++pBegin;

        if (pBegin == pEnd)
          break;
        if (*pBegin == '{')
          ++depth;
      }
    }
  }  // end of for
  return pEnd;
}

// format - format the given formal string, subsituting the formal
// arguments. The result is appended at on the pOutStr.
void Diagnostic::format(const char* pBegin,
                        const char* pEnd,
                        std::string& pOutStr) const {
  const char* cur_char = pBegin;
  while (cur_char != pEnd) {
    if (*cur_char != '%') {
      const char* new_end = std::find(cur_char, pEnd, '%');
      pOutStr.append(cur_char, new_end);
      cur_char = new_end;
      continue;
    } else if (ispunct(cur_char[1])) {
      pOutStr.push_back(cur_char[1]);  // %% -> %.
      cur_char += 2;
      continue;
    }

    // skip the %.
    ++cur_char;

    const char* modifier = NULL;
    size_t modifier_len = 0;

    // we get a modifier
    if (!isdigit(*cur_char)) {
      modifier = cur_char;
      while (*cur_char == '-' || (*cur_char >= 'a' && *cur_char <= 'z'))
        ++cur_char;
      modifier_len = cur_char - modifier;

      // we get an argument
      if (*cur_char == '{') {
        ++cur_char;  // skip '{'
        cur_char = findMatch('}', cur_char, pEnd);

        if (cur_char == pEnd) {
          // DIAG's format error
          llvm::report_fatal_error(
              llvm::Twine("Mismatched {} in the diagnostic: ") +
              llvm::Twine(getID()));
        }

        ++cur_char;  // skip '}'
      }
    }
    if (!isdigit(*cur_char)) {
      llvm::report_fatal_error(llvm::Twine("In diagnostic: ") +
                               llvm::Twine(getID()) + llvm::Twine(": ") +
                               llvm::Twine(pBegin) +
                               llvm::Twine("\nNo given arugment number:\n"));
    }

    unsigned int arg_no = *cur_char - '0';
    ++cur_char;  // skip argument number

    DiagnosticEngine::ArgumentKind kind = getArgKind(arg_no);
    switch (kind) {
      case DiagnosticEngine::ak_std_string: {
        if (modifier_len != 0) {
          llvm::report_fatal_error(
              llvm::Twine("In diagnostic: ") + llvm::Twine(getID()) +
              llvm::Twine(": ") + llvm::Twine(pBegin) +
              llvm::Twine("\nNo modifiers for strings yet\n"));
        }
        const std::string& str = getArgStdStr(arg_no);
        pOutStr.append(str.begin(), str.end());
        break;
      }
      case DiagnosticEngine::ak_c_string: {
        if (modifier_len != 0) {
          llvm::report_fatal_error(
              llvm::Twine("In diagnostic: ") + llvm::Twine(getID()) +
              llvm::Twine(": ") + llvm::Twine(pBegin) +
              llvm::Twine("\nNo modifiers for strings yet\n"));
        }
        const char* str = getArgCStr(arg_no);
        if (str == NULL)
          str = "(null)";
        pOutStr.append(str);
        break;
      }
      case DiagnosticEngine::ak_sint: {
        int val = getArgSInt(arg_no);
        llvm::raw_string_ostream(pOutStr) << val;
        break;
      }
      case DiagnosticEngine::ak_uint: {
        unsigned int val = getArgUInt(arg_no);
        llvm::raw_string_ostream(pOutStr) << val;
        break;
      }
      case DiagnosticEngine::ak_ulonglong: {
        unsigned long long val = getArgUInt(arg_no);
        llvm::raw_string_ostream(pOutStr) << val;
        break;
      }
      case DiagnosticEngine::ak_bool: {
        bool val = getArgBool(arg_no);
        if (val)
          pOutStr.append("true");
        else
          pOutStr.append("false");
        break;
      }
    }  // end of switch
  }    // end of while
}

}  // namespace mcld
