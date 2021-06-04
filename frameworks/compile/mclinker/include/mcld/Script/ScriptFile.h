//===- ScriptFile.h -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_SCRIPT_SCRIPTFILE_H_
#define MCLD_SCRIPT_SCRIPTFILE_H_

#include "mcld/Script/Assignment.h"
#include "mcld/Script/InputSectDesc.h"
#include "mcld/Script/OutputSectDesc.h"

#include <string>
#include <vector>

namespace mcld {

class ArchiveReader;
class DynObjReader;
class GroupReader;
class Input;
class InputBuilder;
class InputTree;
class LinkerConfig;
class Module;
class ObjectReader;
class ScriptCommand;
class StringList;
class RpnExpr;

/** \class ScriptFile
 *  \brief This class defines the interfaces to a linker script file.
 */

class ScriptFile {
 public:
  enum Kind {
    LDScript,       // -T
    Expression,     // --defsym
    VersionScript,  // --version-script
    DynamicList,    // --dynamic-list
    Unknown
  };

  typedef std::vector<ScriptCommand*> CommandQueue;
  typedef CommandQueue::const_iterator const_iterator;
  typedef CommandQueue::iterator iterator;
  typedef CommandQueue::const_reference const_reference;
  typedef CommandQueue::reference reference;

 public:
  ScriptFile(Kind pKind, Input& pInput, InputBuilder& pBuilder);
  ~ScriptFile();

  const_iterator begin() const { return m_CommandQueue.begin(); }
  iterator begin() { return m_CommandQueue.begin(); }
  const_iterator end() const { return m_CommandQueue.end(); }
  iterator end() { return m_CommandQueue.end(); }

  const_reference front() const { return m_CommandQueue.front(); }
  reference front() { return m_CommandQueue.front(); }
  const_reference back() const { return m_CommandQueue.back(); }
  reference back() { return m_CommandQueue.back(); }

  const Input& input() const { return m_Input; }
  Input& input() { return m_Input; }

  size_t size() const { return m_CommandQueue.size(); }

  Kind getKind() const { return m_Kind; }

  const InputTree& inputs() const { return *m_pInputTree; }
  InputTree& inputs() { return *m_pInputTree; }

  const std::string& name() const { return m_Name; }
  std::string& name() { return m_Name; }

  void dump() const;
  void activate(Module& pModule);

  /// ENTRY(symbol)
  void addEntryPoint(const std::string& pSymbol);

  /// OUTPUT_FORMAT(bfdname)
  /// OUTPUT_FORMAT(default, big, little)
  void addOutputFormatCmd(const std::string& pFormat);
  void addOutputFormatCmd(const std::string& pDefault,
                          const std::string& pBig,
                          const std::string& pLittle);

  /// INPUT(file, file, ...)
  /// INPUT(file file ...)
  void addInputCmd(StringList& pStringList,
                   ObjectReader& pObjectReader,
                   ArchiveReader& pArchiveReader,
                   DynObjReader& pDynObjReader,
                   const LinkerConfig& pConfig);

  /// GROUP(file, file, ...)
  /// GROUP(file file ...)
  void addGroupCmd(StringList& pStringList,
                   GroupReader& pGroupReader,
                   const LinkerConfig& pConfig);

  /// OUTPUT(filename)
  void addOutputCmd(const std::string& pFileName);

  /// SEARCH_DIR(path)
  void addSearchDirCmd(const std::string& pPath);

  /// OUTPUT_ARCH(bfdarch)
  void addOutputArchCmd(const std::string& pArch);

  /// ASSERT(exp, message)
  void addAssertCmd(RpnExpr& pRpnExpr, const std::string& pMessage);

  /// assignment
  void addAssignment(const std::string& pSymbol,
                     RpnExpr& pRpnExpr,
                     Assignment::Type pType = Assignment::DEFAULT);

  bool hasSectionsCmd() const;

  void enterSectionsCmd();

  void leaveSectionsCmd();

  void enterOutputSectDesc(const std::string& pName,
                           const OutputSectDesc::Prolog& pProlog);

  void leaveOutputSectDesc(const OutputSectDesc::Epilog& pEpilog);

  void addInputSectDesc(InputSectDesc::KeepPolicy pPolicy,
                        const InputSectDesc::Spec& pSpec);

  RpnExpr* createRpnExpr();
  const RpnExpr* getCurrentRpnExpr() const { return m_pRpnExpr; }
  RpnExpr* getCurrentRpnExpr() { return m_pRpnExpr; }

  StringList* createStringList();
  const StringList* getCurrentStringList() const { return m_pStringList; }
  StringList* getCurrentStringList() { return m_pStringList; }

  void setAsNeeded(bool pEnable = true);
  bool asNeeded() const { return m_bAsNeeded; }

  static const std::string& createParserStr(const char* pText, size_t pLength);

  static void clearParserStrPool();

 private:
  Kind m_Kind;
  Input& m_Input;
  std::string m_Name;
  InputTree* m_pInputTree;
  InputBuilder& m_Builder;
  CommandQueue m_CommandQueue;
  bool m_bHasSectionsCmd;
  bool m_bInSectionsCmd;
  bool m_bInOutputSectDesc;
  RpnExpr* m_pRpnExpr;
  StringList* m_pStringList;
  bool m_bAsNeeded;
};

}  // namespace mcld

#endif  // MCLD_SCRIPT_SCRIPTFILE_H_
