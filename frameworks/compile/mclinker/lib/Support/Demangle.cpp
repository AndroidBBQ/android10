//===- Demangle.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Config/Config.h"
#include "mcld/Support/CXADemangle.tcc"
#include "mcld/Support/Demangle.h"

#ifdef HAVE_CXXABI_H
#include <cxxabi.h>
#endif

namespace mcld {

std::string demangleName(const std::string& pName) {
#ifdef HAVE_CXXABI_H
  // Spoil names of symbols with C linkage, so use an heuristic approach to
  // check if the name should be demangled.
  if (pName.substr(0, 2) != "_Z")
    return pName;
  // __cxa_demangle needs manually handle the memory release, so we wrap
  // it into this helper function.
  size_t output_leng;
  int status;
  char* buffer =
      abi::__cxa_demangle(pName.c_str(), /*buffer=*/0, &output_leng, &status);
  if (status != 0) {  // Failed
    return pName;
  }
  std::string result(buffer);
  free(buffer);

  return result;
#else
  return pName;
#endif
}

bool isCtorOrDtor(const char* pName, size_t pLength) {
  arena<bs> a;
  Db db(a);
  db.cv = 0;
  db.ref = 0;
  db.encoding_depth = 0;
  db.parsed_ctor_dtor_cv = false;
  db.tag_templates = true;
  db.template_param.emplace_back(a);
  db.fix_forward_references = false;
  db.try_to_parse_template_args = true;
  int internal_status = success;
  demangle(pName, pName + pLength, db, internal_status);
  if (internal_status == success && db.fix_forward_references &&
      !db.template_param.empty() && !db.template_param.front().empty()) {
    db.fix_forward_references = false;
    db.tag_templates = false;
    db.names.clear();
    db.subs.clear();
    demangle(pName, pName + pLength, db, internal_status);
    if (db.fix_forward_references)
      internal_status = invalid_mangled_name;
  }

  if (internal_status != success) {
    db.parsed_ctor_dtor_cv = false;
  }
  return db.parsed_ctor_dtor_cv;
}

}  // namespace mcld
