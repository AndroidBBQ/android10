//===- Operand.h ----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_OPERAND_H_
#define MCLD_SCRIPT_OPERAND_H_

#include "mcld/Config/Config.h"
#include "mcld/Object/SectionMap.h"
#include "mcld/Script/ExprToken.h"
#include "mcld/Support/Allocators.h"

#include <llvm/Support/DataTypes.h>

#include <string>

#include <cassert>

namespace mcld {

/** \class Operand
 *  \brief This class defines the interfaces to an operand token.
 */

class Operand : public ExprToken {
 public:
  enum Type { SYMBOL, INTEGER, SECTION, SECTION_DESC, FRAGMENT };

 protected:
  explicit Operand(Type pType);
  virtual ~Operand();

 public:
  Type type() const { return m_Type; }

  virtual bool isDot() const { return false; }

  virtual uint64_t value() const = 0;

  static bool classof(const ExprToken* pToken) {
    return pToken->kind() == ExprToken::OPERAND;
  }

 private:
  Type m_Type;
};

/** \class SymOperand
 *  \brief This class defines the interfaces to a symbol operand.
 */

class SymOperand : public Operand {
 private:
  friend class Chunk<SymOperand, MCLD_SYMBOLS_PER_INPUT>;
  SymOperand();
  explicit SymOperand(const std::string& pName);

 public:
  void dump() const;

  const std::string& name() const { return m_Name; }

  bool isDot() const;

  uint64_t value() const { return m_Value; }

  void setValue(uint64_t pValue) { m_Value = pValue; }

  static bool classof(const Operand* pOperand) {
    return pOperand->type() == Operand::SYMBOL;
  }

  /* factory method */
  static SymOperand* create(const std::string& pName);
  static void destroy(SymOperand*& pOperand);
  static void clear();

 private:
  std::string m_Name;
  uint64_t m_Value;
};

/** \class IntOperand
 *  \brief This class defines the interfaces to an integer operand.
 */

class IntOperand : public Operand {
 private:
  friend class Chunk<IntOperand, MCLD_SYMBOLS_PER_INPUT>;
  IntOperand();
  explicit IntOperand(uint64_t pValue);

 public:
  void dump() const;

  uint64_t value() const { return m_Value; }

  void setValue(uint64_t pValue) { m_Value = pValue; }

  static bool classof(const Operand* pOperand) {
    return pOperand->type() == Operand::INTEGER;
  }

  /* factory method */
  static IntOperand* create(uint64_t pValue);
  static void destroy(IntOperand*& pOperand);
  static void clear();

 private:
  uint64_t m_Value;
};

/** \class SectOperand
 *  \brief This class defines the interfaces to an section name operand.
 */
class LDSection;

class SectOperand : public Operand {
 private:
  friend class Chunk<SectOperand, MCLD_SECTIONS_PER_INPUT>;
  SectOperand();
  explicit SectOperand(const std::string& pName);

 public:
  void dump() const;

  const std::string& name() const { return m_Name; }

  uint64_t value() const {
    assert(0);
    return 0;
  }

  static bool classof(const Operand* pOperand) {
    return pOperand->type() == Operand::SECTION;
  }

  /* factory method */
  static SectOperand* create(const std::string& pName);
  static void destroy(SectOperand*& pOperand);
  static void clear();

 private:
  std::string m_Name;
};

/** \class SectDescOperand
 *  \brief This class defines the interfaces to an section name operand.
 */

class SectDescOperand : public Operand {
 private:
  friend class Chunk<SectDescOperand, MCLD_SECTIONS_PER_INPUT>;
  SectDescOperand();
  explicit SectDescOperand(const SectionMap::Output* pOutputDesc);

 public:
  void dump() const;

  const SectionMap::Output* outputDesc() const { return m_pOutputDesc; }

  uint64_t value() const {
    assert(0);
    return 0;
  }

  static bool classof(const Operand* pOperand) {
    return pOperand->type() == Operand::SECTION_DESC;
  }

  /* factory method */
  static SectDescOperand* create(const SectionMap::Output* pOutputDesc);
  static void destroy(SectDescOperand*& pOperand);
  static void clear();

 private:
  const SectionMap::Output* m_pOutputDesc;
};

/** \class FragOperand
 *  \brief This class defines the interfaces to a fragment operand.
 */

class Fragment;

class FragOperand : public Operand {
 private:
  friend class Chunk<FragOperand, MCLD_SYMBOLS_PER_INPUT>;
  FragOperand();
  explicit FragOperand(Fragment& pFragment);

 public:
  void dump() const;

  const Fragment* frag() const { return m_pFragment; }
  Fragment* frag() { return m_pFragment; }

  uint64_t value() const;

  static bool classof(const Operand* pOperand) {
    return pOperand->type() == Operand::FRAGMENT;
  }

  /* factory method */
  static FragOperand* create(Fragment& pFragment);
  static void destroy(FragOperand*& pOperand);
  static void clear();

 private:
  Fragment* m_pFragment;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_OPERAND_H_
