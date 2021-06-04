//===- InputFactory.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_MC_INPUTFACTORY_H_
#define MCLD_MC_INPUTFACTORY_H_
#include "mcld/MC/Input.h"
#include "mcld/Support/GCFactory.h"

namespace mcld {

class AttributeProxy;
class AttributeSet;
class LinkerConfig;

/** \class InputFactory
 *  \brief InputFactory controls the production and destruction of
 *  MCLDInput.
 *
 *  All MCLDFiles created by MCLDFileFactory are guaranteed to be destructed
 *  while MCLDFileFactory is destructed.
 *
 *  \see llvm::sys::Path
 */
class InputFactory : public GCFactory<Input, 0> {
 public:
  typedef GCFactory<Input, 0> Alloc;

 public:
  InputFactory(size_t pNum, const LinkerConfig& pConfig);

  ~InputFactory();

  // -----  input  ----- //
  Input* produce(llvm::StringRef pName,
                 const sys::fs::Path& pPath,
                 unsigned int pType = Input::Unknown,
                 off_t pFileOffset = 0);

  Input* produce(llvm::StringRef pName,
                 const char* pPath,
                 unsigned int pType = Input::Unknown,
                 off_t pFileOffset = 0);

  // -----  attributes  ----- //
  /// attr - the last touched attribute.
  const AttributeProxy& attr() const { return *m_pLast; }
  AttributeProxy& attr() { return *m_pLast; }

 private:
  AttributeProxy* m_pLast;
  AttributeSet* m_pAttrSet;
};

}  // namespace mcld

#endif  // MCLD_MC_INPUTFACTORY_H_
