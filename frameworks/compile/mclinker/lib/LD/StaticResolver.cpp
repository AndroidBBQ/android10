//===- StaticResolver.cpp -------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/LD/StaticResolver.h"

#include "mcld/LD/LDSymbol.h"
#include "mcld/Support/Demangle.h"
#include "mcld/Support/MsgHandling.h"

namespace mcld {

//==========================
// StaticResolver
StaticResolver::~StaticResolver() {
}

bool StaticResolver::resolve(ResolveInfo& __restrict__ pOld,
                             const ResolveInfo& __restrict__ pNew,
                             bool& pOverride,
                             LDSymbol::ValueType pValue) const {
  /* The state table itself.
   * The first index is a link_row and the second index is a bfd_link_hash_type.
   *
   * Cs -> all rest kind of common (d_C, wd_C)
   * Is -> all kind of indirect
   */
  static const enum LinkAction link_action[LAST_ORD][LAST_ORD] = {
    /* new\old  U       w_U     d_U    wd_U   D      w_D    d_D    wd_D   C      w_C,   Cs,    Is   */  // NOLINT
    /* U    */ {NOACT,  UND,    UND,   UND,   NOACT, NOACT, DUND,  DUND,  NOACT, NOACT, NOACT, REFC },  // NOLINT
    /* w_U  */ {NOACT,  NOACT,  NOACT, WEAK,  NOACT, NOACT, DUNDW, DUNDW, NOACT, NOACT, NOACT, REFC },  // NOLINT
    /* d_U  */ {NOACT,  NOACT,  NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, REFC },  // NOLINT
    /* wd_U */ {NOACT,  NOACT,  NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, REFC },  // NOLINT
    /* D    */ {DEF,    DEF,    DEF,   DEF,   MDEF,  DEF,   DEF,   DEF,   CDEF,  CDEF,  CDEF,  MDEF },  // NOLINT
    /* w_D  */ {DEFW,   DEFW,   DEFW,  DEFW,  NOACT, NOACT, DEFW,  DEFW,  NOACT, NOACT, NOACT, NOACT},  // NOLINT
    /* d_D  */ {MDEFD,  MDEFD,  DEFD,  DEFD,  NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, MDEF },  // NOLINT
    /* wd_D */ {MDEFWD, MDEFWD, DEFWD, DEFWD, NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, NOACT},  // NOLINT
    /* C    */ {COM,    COM,    COM,   COM,   CREF,  COM,   COM,   COM,   MBIG,  COM,   BIG,   REFC },  // NOLINT
    /* w_C  */ {COM,    COM,    COM,   COM,   NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, NOACT, REFC },  // NOLINT
    /* Cs   */ {COM,    COM,    COM,   COM,   NOACT, NOACT, NOACT, NOACT, MBIG,  MBIG,  MBIG,  REFC },  // NOLINT
    /* Is   */ {IND,    IND,    IND,   IND,   MDEF,  IND,   IND,   IND,   CIND,  CIND,  CIND,  MIND }   // NOLINT
  };

  // Special cases:
  // * when a dynamic defined symbol meets a dynamic weak defined symbol, act
  //   noting.
  // * when a undefined symbol meets a dynamic defined symbol, override by
  //   dynamic defined first, then recover back to undefined symbol later.
  // * when a dynamic defined symbol meets a undefined symbol or a weak
  //   undefined symbol, do not override, instead of marking.
  // * When a undefined symbol meets a dynamic defined symbol or a weak
  //   undefined symbol meets a dynamic defined symbol, should override.
  // * When a common symbol meets a weak common symbol, adjust the size of
  //   common symbol.

  unsigned int row = getOrdinate(pNew);
  unsigned int col = getOrdinate(pOld);

  bool cycle = false;
  pOverride = false;
  ResolveInfo* old = &pOld;
  LinkAction action;
  do {
    cycle = false;
    action = link_action[row][col];

    switch (action) {
      case FAIL: { /* abort.  */
        fatal(diag::fail_sym_resolution) << __FILE__ << __LINE__
                                         << "mclinker@googlegroups.com";
        return false;
      }
      case NOACT: { /* no action.  */
        pOverride = false;
        old->overrideVisibility(pNew);
        break;
      }
      case UND:   /* override by symbol undefined symbol.  */
      case WEAK:  /* override by symbol weak undefined.  */
      case DEF:   /* override by symbol defined.  */
      case DEFW:  /* override by symbol weak defined.  */
      case DEFD:  /* override by symbol dynamic defined.  */
      case DEFWD: /* override by symbol dynamic weak defined. */
      case COM: { /* override by symbol common defined.  */
        pOverride = true;
        old->override(pNew);
        break;
      }
      case MDEFD:    /* mark symbol dynamic defined.  */
      case MDEFWD: { /* mark symbol dynamic weak defined.  */
        uint32_t binding = old->binding();
        old->override(pNew);
        old->setBinding(binding);
        ignore(diag::mark_dynamic_defined) << old->name();
        pOverride = true;
        break;
      }
      case DUND:
      case DUNDW: {
        old->overrideVisibility(pNew);
        old->setDynamic();
        pOverride = false;
        break;
      }
      case CREF: { /* Possibly warn about common reference to defined symbol. */
        // A common symbol does not override a definition.
        ignore(diag::comm_refer_to_define) << old->name();
        pOverride = false;
        break;
      }
      case CDEF: { /* redefine existing common symbol.  */
        // We've seen a common symbol and now we see a definition.  The
        // definition overrides.
        //
        // NOTE: m_Mesg uses 'name' instead of `name' for being compatible to
        // GNU ld.
        ignore(diag::redefine_common) << old->name();
        old->override(pNew);
        pOverride = true;
        break;
      }
      case BIG: { /* override by symbol common using largest size.  */
        if (old->size() < pNew.size())
          old->setSize(pNew.size());
        old->overrideAttributes(pNew);
        old->overrideVisibility(pNew);
        pOverride = true;
        break;
      }
      case MBIG: { /* mark common symbol by larger size. */
        if (old->size() < pNew.size())
          old->setSize(pNew.size());
        old->overrideVisibility(pNew);
        pOverride = false;
        break;
      }
      case CIND: { /* mark indirect symbol from existing common symbol.  */
        ignore(diag::indirect_refer_to_common) << old->name();
      }
      /* Fall through */
      case IND: { /* override by indirect symbol.  */
        if (pNew.link() == NULL) {
          fatal(diag::indirect_refer_to_inexist) << pNew.name();
          break;
        }

        /** Should detect the loop of indirect symbol during file reading **/
        // if (pNew.link()->isIndirect() && pNew.link()->link() == &pNew) {
        //  m_Mesg = "indirect symbol `"+pNew.name()+"' to
        //  `"+pNew.link()->name()+"' is a loop.";
        //  return Resolver::Abort;
        //}

        // change the old symbol to the indirect symbol
        old->setLink(pNew.link());
        pOverride = true;
        break;
      }
      case MIND: { /* multiple indirect symbols.  */
        // it is OK if they both point to the same symbol
        if (old->link() == pNew.link()) {
          pOverride = false;
          break;
        }
      }
      /* Fall through */
      case MDEF: { /* multiple definition error.  */
        if (pOld.isDefine() && pNew.isDefine() && pOld.isAbsolute() &&
            pNew.isAbsolute() &&
            (pOld.desc() == pNew.desc() || pOld.desc() == ResolveInfo::NoType ||
             pNew.desc() == ResolveInfo::NoType)) {
          if (pOld.outSymbol()->value() == pValue) {
            pOverride = true;
            old->override(pNew);
            break;
          } else {
            error(diag::multiple_absolute_definitions)
                << demangleName(pNew.name()) << pOld.outSymbol()->value()
                << pValue;
            break;
          }
        }

        error(diag::multiple_definitions) << demangleName(pNew.name());
        break;
      }
      case REFC: { /* Mark indirect symbol referenced and then CYCLE.  */
        if (old->link() == NULL) {
          fatal(diag::indirect_refer_to_inexist) << old->name();
          break;
        }

        old = old->link();
        col = getOrdinate(*old);
        cycle = true;
        break;
      }
      default: {
        error(diag::undefined_situation) << action << old->name()
                                         << pNew.name();
        return false;
      }
    }  // end of the big switch (action)
  } while (cycle);
  return true;
}

}  // namespace mcld
