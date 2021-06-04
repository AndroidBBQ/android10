//===- Stub.cpp -----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Fragment/Stub.h"

#include "mcld/IRBuilder.h"
#include "mcld/Fragment/Relocation.h"
#include "mcld/LD/BranchIsland.h"
#include "mcld/LD/ResolveInfo.h"

#include <cassert>

namespace mcld {

Stub::Stub() : Fragment(Fragment::Stub), m_pSymInfo(NULL) {
}

Stub::~Stub() {
  for (fixup_iterator fixup = fixup_begin(); fixup != fixup_end(); ++fixup)
    delete (*fixup);
}

void Stub::setSymInfo(ResolveInfo* pSymInfo) {
  m_pSymInfo = pSymInfo;
}

void Stub::applyFixup(Relocation& pSrcReloc,
                      IRBuilder& pBuilder,
                      BranchIsland& pIsland) {
  // build a name for stub symbol
  std::string sym_name("__");
  sym_name.append(pSrcReloc.symInfo()->name())
          .append("_")
          .append(name())
          .append("@")
          .append(pIsland.name());

  // create LDSymbol for the stub
  LDSymbol* symbol =
      pBuilder.AddSymbol<IRBuilder::Force, IRBuilder::Unresolve>(
          sym_name,
          ResolveInfo::Function,
          ResolveInfo::Define,
          ResolveInfo::Local,
          size(),
          initSymValue(),
          FragmentRef::Create(*this, initSymValue()),
          ResolveInfo::Default);
  setSymInfo(symbol->resolveInfo());

  // add relocations of this stub (i.e., set the branch target of the stub)
  for (fixup_iterator it = fixup_begin(), ie = fixup_end(); it != ie; ++it) {
    Relocation* reloc =
        Relocation::Create((*it)->type(),
                           *(FragmentRef::Create(*this, (*it)->offset())),
                           (*it)->addend());
    reloc->setSymInfo(pSrcReloc.symInfo());
    pIsland.addRelocation(*reloc);
  }
}

void Stub::applyFixup(FragmentRef& pSrcFragRef,
                      IRBuilder& pBuilder,
                      BranchIsland& pIsland) {
  // If applying fixups is based on the source FragmentRef, each target stub
  // probably should override this function.
  assert(0 && "target stub should override this function");
}

void Stub::addFixup(DWord pOffset, SWord pAddend, Type pType) {
  assert(pOffset < size());
  m_FixupList.push_back(new Fixup(pOffset, pAddend, pType));
}

void Stub::addFixup(const Fixup& pFixup) {
  assert(pFixup.offset() < size());
  m_FixupList.push_back(new Fixup(pFixup));
}

}  // namespace mcld
