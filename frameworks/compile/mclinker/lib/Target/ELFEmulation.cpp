//===- ELFEmulation.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "mcld/Target/ELFEmulation.h"
#include "mcld/LinkerScript.h"
#include "mcld/LinkerConfig.h"
#include "mcld/Script/InputSectDesc.h"

#include <llvm/Support/Host.h>

namespace mcld {

struct NameMap {
  const char* from;  ///< the prefix of the input string. (match FROM*)
  const char* to;    ///< the output string.
  InputSectDesc::KeepPolicy policy;  /// mark whether the input is kept in GC
};

static const NameMap map[] = {
    {".text*", ".text", InputSectDesc::NoKeep},
    {".rodata*", ".rodata", InputSectDesc::NoKeep},
    {".data.rel.ro.local*", ".data.rel.ro.local", InputSectDesc::NoKeep},
    {".data.rel.ro*", ".data.rel.ro", InputSectDesc::NoKeep},
    {".data*", ".data", InputSectDesc::NoKeep},
    {".bss*", ".bss", InputSectDesc::NoKeep},
    {".tdata*", ".tdata", InputSectDesc::NoKeep},
    {".tbss*", ".tbss", InputSectDesc::NoKeep},
    {".init", ".init", InputSectDesc::Keep},
    {".fini", ".fini", InputSectDesc::Keep},
    {".preinit_array*", ".preinit_array", InputSectDesc::Keep},
    {".init_array*", ".init_array", InputSectDesc::Keep},
    {".fini_array*", ".fini_array", InputSectDesc::Keep},
    // TODO: Support DT_INIT_ARRAY for all constructors?
    {".ctors*", ".ctors", InputSectDesc::Keep},
    {".dtors*", ".dtors", InputSectDesc::Keep},
    {".jcr", ".jcr", InputSectDesc::Keep},
    // FIXME: in GNU ld, if we are creating a shared object .sdata2 and .sbss2
    // sections would be handled differently.
    {".sdata2*", ".sdata", InputSectDesc::NoKeep},
    {".sbss2*", ".sbss", InputSectDesc::NoKeep},
    {".sdata*", ".sdata", InputSectDesc::NoKeep},
    {".sbss*", ".sbss", InputSectDesc::NoKeep},
    {".lrodata*", ".lrodata", InputSectDesc::NoKeep},
    {".ldata*", ".ldata", InputSectDesc::NoKeep},
    {".lbss*", ".lbss", InputSectDesc::NoKeep},
    {".gcc_except_table*", ".gcc_except_table", InputSectDesc::Keep},
    {".gnu.linkonce.d.rel.ro.local*", ".data.rel.ro.local", InputSectDesc::NoKeep},  // NOLINT
    {".gnu.linkonce.d.rel.ro*", ".data.rel.ro", InputSectDesc::NoKeep},
    {".gnu.linkonce.r*", ".rodata", InputSectDesc::NoKeep},
    {".gnu.linkonce.d*", ".data", InputSectDesc::NoKeep},
    {".gnu.linkonce.b*", ".bss", InputSectDesc::NoKeep},
    {".gnu.linkonce.sb2*", ".sbss", InputSectDesc::NoKeep},
    {".gnu.linkonce.sb*", ".sbss", InputSectDesc::NoKeep},
    {".gnu.linkonce.s2*", ".sdata", InputSectDesc::NoKeep},
    {".gnu.linkonce.s*", ".sdata", InputSectDesc::NoKeep},
    {".gnu.linkonce.wi*", ".debug_info", InputSectDesc::NoKeep},
    {".gnu.linkonce.td*", ".tdata", InputSectDesc::NoKeep},
    {".gnu.linkonce.tb*", ".tbss", InputSectDesc::NoKeep},
    {".gnu.linkonce.t*", ".text", InputSectDesc::NoKeep},
    {".gnu.linkonce.lr*", ".lrodata", InputSectDesc::NoKeep},
    {".gnu.linkonce.lb*", ".lbss", InputSectDesc::NoKeep},
    {".gnu.linkonce.l*", ".ldata", InputSectDesc::NoKeep},
};

// FIXME: LinkerConfig& pConfig should be constant
bool MCLDEmulateELF(LinkerScript& pScript, LinkerConfig& pConfig) {
  // set up section map
  if (pConfig.options().getScriptList().empty() &&
      pConfig.codeGenType() != LinkerConfig::Object) {
    const unsigned int map_size = (sizeof(map) / sizeof(map[0]));
    for (unsigned int i = 0; i < map_size; ++i) {
      std::pair<SectionMap::mapping, bool> res =
          pScript.sectionMap().insert(map[i].from, map[i].to, map[i].policy);
      if (!res.second)
        return false;
    }
  } else {
    // FIXME: this is the hack to help assignment processing in current
    // implementation.
    pScript.sectionMap().insert("", "");
  }

  if (!pConfig.options().nostdlib()) {
    // TODO: check if user sets the default search path instead via -Y option
    // set up default search path
    switch (pConfig.targets().triple().getOS()) {
      case llvm::Triple::NetBSD:
        pScript.directories().insert("=/usr/lib");
        break;
      case llvm::Triple::Win32:
        pScript.directories().insert("=/mingw/lib");
        break;
      default:
        pScript.directories().insert("=/lib");
        pScript.directories().insert("=/usr/lib");
        break;
    }
  }
  return true;
}

}  // namespace mcld
