//===- Resolver.h ---------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_RESOLVER_H_
#define MCLD_LD_RESOLVER_H_
#include "mcld/LD/LDSymbol.h"

#include <string>
#include <utility>

namespace mcld {

class NamePool;
class ResolveInfo;

/** \class Resolver
 *  \brief Resolver binds a symbol reference from one file to a symbol
 *   definition of another file.
 *
 *  Resolver seals up the algorithm of symbol resolution. The resolution of
 *  two symbols depends on their type, binding and whether it is belonging to
 *  a shared object.
 */
class Resolver {
 public:
  enum Action { Success, Warning, Abort, LastAction };

  /** \class Resolver::Result
   *  \brief the result of symbol resolution
   *   - info, the pointer to overrided info
   *   - existent, if true, the info is existent
   *   - overriden, if true, the info is being overriden.
   */
  struct Result {
    ResolveInfo* info;
    bool existent;
    bool overriden;
  };

 public:
  virtual ~Resolver();

  /// shouldOverride - Can resolver override the symbol pOld by the symbol pNew?
  /// @return the action should be taken.
  /// @param pOld the symbol which may be overridden.
  /// @param pNew the symbol which is used to replace pOld
  virtual bool resolve(ResolveInfo& __restrict__ pOld,
                       const ResolveInfo& __restrict__ pNew,
                       bool& pOverride,
                       LDSymbol::ValueType pValue) const = 0;

  /// resolveAgain - Can override by derived classes.
  /// @return the pointer to resolved ResolveInfo
  /// @return is the symbol existent?
  virtual void resolveAgain(NamePool& pNamePool,
                            unsigned int pAction,
                            ResolveInfo& __restrict__ pOld,
                            const ResolveInfo& __restrict__ pNew,
                            Result& pResult) const {
    pResult.info = NULL;
    pResult.existent = false;
    pResult.overriden = false;
  }
};

}  // namespace mcld

#endif  // MCLD_LD_RESOLVER_H_
