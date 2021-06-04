//===- LinkerConfig.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LINKERCONFIG_H_
#define MCLD_LINKERCONFIG_H_

#include "mcld/GeneralOptions.h"
#include "mcld/TargetOptions.h"
#include "mcld/AttributeOption.h"
#include "mcld/Support/Path.h"

#include <llvm/ADT/Triple.h>

#include <string>

namespace mcld {

/** \class LinkerConfig
 *  \brief LinkerConfig is composed of argumments of MCLinker.
 *   options()        - the general options
 *   bitcode()        - the bitcode being linked
 *   attribute()      - the attribute options
 */
class LinkerConfig {
 public:
  enum CodeGenType { Unknown, Object, DynObj, Exec, External, Binary };

  /** \enum CodePosition
   *  CodePosition indicates the ability of the generated output to be
   *  loaded at different addresses. If the output can be loaded at different
   *  addresses, we say the output is position independent. Shared libraries
   *  and position-independent executable programs (PIE) are in this category.
   *  ::Independent indicates the output is position independent.
   *  If a executable program can not be loaded at arbitrary addresses, but it
   *  can call outside functions, we say the program is dynamic dependent on
   *  the address to be loaded. ::DynamicDependent indicates the output is not
   *  only a executable program, but also dynamic dependent. In general,
   *  executable programs are dynamic dependent.
   *  If a executable program can not be loaded at different addresses, and
   *  only call inner functions, then we say the program is static dependent on
   *  its loaded address. ::StaticDependent is used to indicate this kind of
   *  output.
   */
  enum CodePosition {
    Independent,       ///< Position Independent
    DynamicDependent,  ///< Can call outside libraries
    StaticDependent,   ///< Can not call outside libraries
    Unset              ///< Undetermine code position mode
  };

 public:
  LinkerConfig();

  explicit LinkerConfig(const std::string& pTripleString);

  ~LinkerConfig();

  const GeneralOptions& options() const { return m_Options; }
  GeneralOptions& options() { return m_Options; }

  const TargetOptions& targets() const { return m_Targets; }
  TargetOptions& targets() { return m_Targets; }

  const AttributeOption& attribute() const { return m_Attribute; }
  AttributeOption& attribute() { return m_Attribute; }

  CodeGenType codeGenType() const { return m_CodeGenType; }

  void setCodeGenType(CodeGenType pType) { m_CodeGenType = pType; }

  CodePosition codePosition() const { return m_CodePosition; }
  void setCodePosition(CodePosition pPosition) { m_CodePosition = pPosition; }

  bool isCodeIndep() const { return (Independent == m_CodePosition); }
  bool isCodeDynamic() const { return (DynamicDependent == m_CodePosition); }
  bool isCodeStatic() const { return (StaticDependent == m_CodePosition); }

  static const char* version();

 private:
  // -----  General Options  ----- //
  GeneralOptions m_Options;
  TargetOptions m_Targets;
  AttributeOption m_Attribute;

  CodeGenType m_CodeGenType;
  CodePosition m_CodePosition;
};

}  // namespace mcld

#endif  // MCLD_LINKERCONFIG_H_
