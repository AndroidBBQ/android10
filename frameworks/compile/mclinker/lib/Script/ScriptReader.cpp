//===- ScriptReader.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Script/ScriptReader.h"

#include "mcld/MC/Input.h"
#include "mcld/Script/ScriptFile.h"
#include "mcld/Script/ScriptScanner.h"
#include "mcld/Support/MemoryArea.h"

#include <llvm/ADT/StringRef.h>

#include <istream>
#include <sstream>

namespace mcld {

ScriptReader::ScriptReader(ObjectReader& pObjectReader,
                           ArchiveReader& pArchiveReader,
                           DynObjReader& pDynObjReader,
                           GroupReader& pGroupReader)
    : m_ObjectReader(pObjectReader),
      m_ArchiveReader(pArchiveReader),
      m_DynObjReader(pDynObjReader),
      m_GroupReader(pGroupReader) {
}

ScriptReader::~ScriptReader() {
}

/// isMyFormat
bool ScriptReader::isMyFormat(Input& input, bool& doContinue) const {
  doContinue = true;
  // always return true now
  return true;
}

bool ScriptReader::readScript(const LinkerConfig& pConfig,
                              ScriptFile& pScriptFile) {
  Input& input = pScriptFile.input();
  size_t size = input.memArea()->size();
  llvm::StringRef region = input.memArea()->request(input.fileOffset(), size);
  std::stringbuf buf(region.data());

  std::istream in(&buf);
  ScriptScanner scanner(&in);
  ScriptParser parser(pConfig,
                      pScriptFile,
                      scanner,
                      m_ObjectReader,
                      m_ArchiveReader,
                      m_DynObjReader,
                      m_GroupReader);
  return parser.parse() == 0;
}

}  // namespace mcld
