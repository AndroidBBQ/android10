//===- Relocation.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_LD_RELOCATIONFACTORY_H_
#define MCLD_LD_RELOCATIONFACTORY_H_
#include "mcld/Config/Config.h"
#include "mcld/Fragment/Relocation.h"
#include "mcld/Support/GCFactory.h"

namespace mcld {

class FragmentRef;
class LinkerConfig;

/** \class RelocationFactory
 *  \brief RelocationFactory provides the interface for generating target
 *  relocation
 *
 */
class RelocationFactory
    : public GCFactory<Relocation, MCLD_RELOCATIONS_PER_INPUT> {
 public:
  typedef Relocation::Type Type;
  typedef Relocation::Address Address;
  typedef Relocation::DWord DWord;
  typedef Relocation::SWord SWord;

 public:
  RelocationFactory();

  void setConfig(const LinkerConfig& pConfig);

  // ----- production ----- //
  /// produce - produce a relocation entry
  /// @param pType - the type of the relocation entry
  /// @param pFragRef - the place to apply the relocation
  /// @param pAddend - the addend of the relocation entry
  Relocation* produce(Type pType, FragmentRef& pFragRef, Address pAddend = 0);

  /// produceEmptyEntry - produce an empty relocation which
  /// occupied memory space but all contents set to zero.
  Relocation* produceEmptyEntry();

  void destroy(Relocation* pRelocation);

 private:
  const LinkerConfig* m_pConfig;
};

}  // namespace mcld

#endif  // MCLD_LD_RELOCATIONFACTORY_H_
