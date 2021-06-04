/*
 * Copyright 2017, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MODULE_H
#define MODULE_H

#include <iostream>
#include <map>
#include <vector>

#include "core_defs.h"
#include "entity.h"
#include "instructions.h"
#include "stl_util.h"
#include "types_generated.h"
#include "visitor.h"

namespace android {
namespace spirit {

class Builder;
class AnnotationSection;
class CapabilityInst;
class DebugInfoSection;
class ExtensionInst;
class ExtInstImportInst;
class EntryPointInst;
class ExecutionModeInst;
class EntryPointDefinition;
class FunctionDeclaration;
class FunctionDefinition;
class GlobalSection;
class InputWordStream;
class Instruction;
class MemoryModelInst;

union VersionNumber {
  struct {
    uint8_t mLowZero;
    uint8_t mMinorNumber;
    uint8_t mMajorNumber;
    uint8_t mHighZero;
  } mMajorMinor;
  uint8_t mBytes[4];
  uint32_t mWord;
};

class Module : public Entity {
public:
  static Module *getCurrentModule();
  uint32_t nextId() { return mNextId++; }

  Module();

  Module(Builder *b);

  virtual ~Module() {}

  bool DeserializeInternal(InputWordStream &IS) override;

  void Serialize(OutputWordStream &OS) const override;

  void SerializeHeader(OutputWordStream &OS) const;

  void registerId(uint32_t id, Instruction *inst) {
    mIdTable.insert(std::make_pair(id, inst));
  }

  void initialize();

  bool resolveIds();

  void accept(IVisitor *v) override {
    for (auto cap : mCapabilities) {
      v->visit(cap);
    }
    for (auto ext : mExtensions) {
      v->visit(ext);
    }
    for (auto imp : mExtInstImports) {
      v->visit(imp);
    }

    v->visit(mMemoryModel.get());

    for (auto entry : mEntryPoints) {
      v->visit(entry);
    }

    for (auto mode : mExecutionModes) {
      v->visit(mode);
    }

    v->visit(mDebugInfo.get());
    if (mAnnotations) {
      v->visit(mAnnotations.get());
    }
    if (mGlobals) {
      v->visit(mGlobals.get());
    }

    for (auto def : mFunctionDefinitions) {
      v->visit(def);
    }
  }

  static std::ostream &errs() { return std::cerr; }

  Module *addCapability(Capability cap);
  Module *setMemoryModel(AddressingModel am, MemoryModel mm);
  Module *addExtInstImport(const char *extName);
  Module *addSource(SourceLanguage lang, int version);
  Module *addSourceExtension(const char *ext);
  Module *addString(const char *ext);
  Module *addEntryPoint(EntryPointDefinition *entry);

  ExtInstImportInst *getGLExt() const { return mGLExt; }

  const std::string findStringOfPrefix(const char *prefix) const;

  GlobalSection *getGlobalSection();

  Instruction *lookupByName(const char *) const;
  FunctionDefinition *
  getFunctionDefinitionFromInstruction(FunctionInst *) const;
  FunctionDefinition *lookupFunctionDefinitionByName(const char *name) const;

  // Find the name of the instruction, e.g., the name of a function (OpFunction
  // instruction).
  // The returned string is owned by the OpName instruction, whose first operand
  // is the instruction being queried on.
  const char *lookupNameByInstruction(const Instruction *) const;

  VariableInst *getInvocationId();
  VariableInst *getNumWorkgroups();

  // Adds a struct type built somewhere else.
  Module *addStructType(TypeStructInst *structType);
  Module *addVariable(VariableInst *var);

  // Methods to look up types. Create them if not found.
  TypeVoidInst *getVoidType();
  TypeIntInst *getIntType(int bits, bool isSigned = true);
  TypeIntInst *getUnsignedIntType(int bits);
  TypeFloatInst *getFloatType(int bits);
  TypeVectorInst *getVectorType(Instruction *componentType, int width);
  TypePointerInst *getPointerType(StorageClass storage,
                                  Instruction *pointeeType);
  TypeRuntimeArrayInst *getRuntimeArrayType(Instruction *elementType);

  // This implies that struct types are strictly structural equivalent, i.e.,
  // two structs are equivalent i.f.f. their fields are equivalent, recursively.
  TypeStructInst *getStructType(Instruction *fieldType[], int numField);
  TypeStructInst *getStructType(const std::vector<Instruction *> &fieldType);
  TypeStructInst *getStructType(Instruction *field0Type);
  TypeStructInst *getStructType(Instruction *field0Type,
                                Instruction *field1Type);
  TypeStructInst *getStructType(Instruction *field0Type,
                                Instruction *field1Type,
                                Instruction *field2Type);

  // TODO: Can function types of different decorations be considered the same?
  TypeFunctionInst *getFunctionType(Instruction *retType,
                                    Instruction *const argType[],
                                    size_t numArg);
  TypeFunctionInst *getFunctionType(Instruction *retType,
                                    const std::vector<Instruction *> &argTypes);

  size_t getSize(TypeVoidInst *voidTy);
  size_t getSize(TypeIntInst *intTy);
  size_t getSize(TypeFloatInst *fpTy);
  size_t getSize(TypeVectorInst *vTy);
  size_t getSize(TypePointerInst *ptrTy);
  size_t getSize(TypeStructInst *structTy);
  size_t getSize(TypeFunctionInst *funcTy);
  size_t getSize(Instruction *inst);

  ConstantInst *getConstant(TypeIntInst *type, int32_t value);
  ConstantInst *getConstant(TypeIntInst *type, uint32_t value);
  ConstantInst *getConstant(TypeFloatInst *type, float value);

  ConstantCompositeInst *getConstantComposite(TypeVectorInst *type,
                                              ConstantInst *components[],
                                              size_t width);
  ConstantCompositeInst *
  getConstantComposite(Instruction *type,
                       const std::vector<ConstantInst *> &components);
  ConstantCompositeInst *getConstantComposite(Instruction *type,
                                              ConstantInst *comp0,
                                              ConstantInst *comp1);
  ConstantCompositeInst *getConstantComposite(TypeVectorInst *type,
                                              ConstantInst *comp0,
                                              ConstantInst *comp1,
                                              ConstantInst *comp2);
  ConstantCompositeInst *getConstantComposite(TypeVectorInst *type,
                                              ConstantInst *comp0,
                                              ConstantInst *comp1,
                                              ConstantInst *comp2,
                                              ConstantInst *comp3);

  Module *addFunctionDefinition(FunctionDefinition *func);

  void consolidateAnnotations();

private:
  static Module *mInstance;
  uint32_t mNextId;
  std::map<uint32_t, Instruction *> mIdTable;

  uint32_t mMagicNumber;
  VersionNumber mVersion;
  uint32_t mGeneratorMagicNumber;
  uint32_t mBound;
  uint32_t mReserved;

  std::vector<CapabilityInst *> mCapabilities;
  std::vector<ExtensionInst *> mExtensions;
  std::vector<ExtInstImportInst *> mExtInstImports;
  std::unique_ptr<MemoryModelInst> mMemoryModel;
  std::vector<EntryPointInst *> mEntryPointInsts;
  std::vector<ExecutionModeInst *> mExecutionModes;
  std::vector<EntryPointDefinition *> mEntryPoints;
  std::unique_ptr<DebugInfoSection> mDebugInfo;
  std::unique_ptr<AnnotationSection> mAnnotations;
  std::unique_ptr<GlobalSection> mGlobals;
  std::vector<FunctionDefinition *> mFunctionDefinitions;

  ExtInstImportInst *mGLExt;

  ContainerDeleter<std::vector<CapabilityInst *>> mCapabilitiesDeleter;
  ContainerDeleter<std::vector<ExtensionInst *>> mExtensionsDeleter;
  ContainerDeleter<std::vector<ExtInstImportInst *>> mExtInstImportsDeleter;
  ContainerDeleter<std::vector<EntryPointInst *>> mEntryPointInstsDeleter;
  ContainerDeleter<std::vector<ExecutionModeInst *>> mExecutionModesDeleter;
  ContainerDeleter<std::vector<EntryPointDefinition *>> mEntryPointsDeleter;
  ContainerDeleter<std::vector<FunctionDefinition *>>
      mFunctionDefinitionsDeleter;
};

struct Extent3D {
  uint32_t mWidth;
  uint32_t mHeight;
  uint32_t mDepth;
};

class EntryPointDefinition : public Entity {
public:
  EntryPointDefinition() {}
  EntryPointDefinition(Builder *builder, ExecutionModel execModel,
                       FunctionDefinition *func, const char *name);

  virtual ~EntryPointDefinition() {
    // Nothing to do here since ~Module() will delete entities referenced here
  }

  void accept(IVisitor *visitor) override {
    visitor->visit(mEntryPointInst);
    // Do not visit the ExecutionMode instructions here. They are linked here
    // for convinience, and for convinience only. They are all grouped, stored,
    // and serialized directly in the module in a section right after all
    // EntryPoint instructions. Visit them from there.
  }

  bool DeserializeInternal(InputWordStream &IS) override;

  EntryPointDefinition *addToInterface(VariableInst *var);
  EntryPointDefinition *addExecutionMode(ExecutionModeInst *mode) {
    mExecutionModeInsts.push_back(mode);
    return this;
  }
  const std::vector<ExecutionModeInst *> &getExecutionModes() const {
    return mExecutionModeInsts;
  }

  EntryPointDefinition *setLocalSize(uint32_t width, uint32_t height,
                                     uint32_t depth);

  EntryPointDefinition *applyExecutionMode(ExecutionModeInst *mode);

  EntryPointInst *getInstruction() const { return mEntryPointInst; }

private:
  const char *mName;
  FunctionInst *mFunction;
  ExecutionModel mExecutionModel;
  std::vector<VariableInst *> mInterface;
  Extent3D mLocalSize;

  EntryPointInst *mEntryPointInst;
  std::vector<ExecutionModeInst *> mExecutionModeInsts;
};

class DebugInfoSection : public Entity {
public:
  DebugInfoSection() : mSourcesDeleter(mSources), mNamesDeleter(mNames) {}
  DebugInfoSection(Builder *b)
      : Entity(b), mSourcesDeleter(mSources), mNamesDeleter(mNames) {}

  virtual ~DebugInfoSection() {}

  bool DeserializeInternal(InputWordStream &IS) override;

  DebugInfoSection *addSource(SourceLanguage lang, int version);
  DebugInfoSection *addSourceExtension(const char *ext);
  DebugInfoSection *addString(const char *str);

  std::string findStringOfPrefix(const char *prefix);

  Instruction *lookupByName(const char *name) const;
  const char *lookupNameByInstruction(const Instruction *) const;

  void accept(IVisitor *v) override {
    for (auto source : mSources) {
      v->visit(source);
    }
    for (auto name : mNames) {
      v->visit(name);
    }
  }

private:
  // (OpString|OpSource|OpSourceExtension|OpSourceContinued)*
  std::vector<Instruction *> mSources;
  // (OpName|OpMemberName)*
  std::vector<Instruction *> mNames;

  ContainerDeleter<std::vector<Instruction *>> mSourcesDeleter;
  ContainerDeleter<std::vector<Instruction *>> mNamesDeleter;
};

class AnnotationSection : public Entity {
public:
  AnnotationSection();
  AnnotationSection(Builder *b);

  virtual ~AnnotationSection() {}

  bool DeserializeInternal(InputWordStream &IS) override;

  void accept(IVisitor *v) override {
    for (auto inst : mAnnotations) {
      v->visit(inst);
    }
  }

  template <typename T> void addAnnotations(T begin, T end) {
    mAnnotations.insert<T>(std::end(mAnnotations), begin, end);
  }

  std::vector<Instruction *>::const_iterator begin() const {
    return mAnnotations.begin();
  }

  std::vector<Instruction *>::const_iterator end() const {
    return mAnnotations.end();
  }

  void clear() { mAnnotations.clear(); }

private:
  std::vector<Instruction *> mAnnotations; // OpDecorate, etc.

  ContainerDeleter<std::vector<Instruction *>> mAnnotationsDeleter;
};

// Types, constants, and globals
class GlobalSection : public Entity {
public:
  GlobalSection();
  GlobalSection(Builder *builder);

  virtual ~GlobalSection() {}

  bool DeserializeInternal(InputWordStream &IS) override;

  void accept(IVisitor *v) override {
    for (auto inst : mGlobalDefs) {
      v->visit(inst);
    }

    if (mInvocationId) {
      v->visit(mInvocationId.get());
    }

    if (mNumWorkgroups) {
      v->visit(mNumWorkgroups.get());
    }
  }

  ConstantInst *getConstant(TypeIntInst *type, int32_t value);
  ConstantInst *getConstant(TypeIntInst *type, uint32_t value);
  ConstantInst *getConstant(TypeFloatInst *type, float value);
  ConstantCompositeInst *getConstantComposite(TypeVectorInst *type,
                                              ConstantInst *components[],
                                              size_t width);

  // Methods to look up types. Create them if not found.
  TypeVoidInst *getVoidType();
  TypeIntInst *getIntType(int bits, bool isSigned = true);
  TypeFloatInst *getFloatType(int bits);
  TypeVectorInst *getVectorType(Instruction *componentType, int width);
  TypePointerInst *getPointerType(StorageClass storage,
                                  Instruction *pointeeType);
  TypeRuntimeArrayInst *getRuntimeArrayType(Instruction *elementType);

  // This implies that struct types are strictly structural equivalent, i.e.,
  // two structs are equivalent i.f.f. their fields are equivalent, recursively.
  TypeStructInst *getStructType(Instruction *fieldType[], int numField);
  // TypeStructInst *getStructType(const std::vector<Instruction *>
  // &fieldTypes);

  // TODO: Can function types of different decorations be considered the same?
  TypeFunctionInst *getFunctionType(Instruction *retType,
                                    Instruction *const argType[],
                                    size_t numArg);
  // TypeStructInst *addStructType(Instruction *fieldType[], int numField);
  GlobalSection *addStructType(TypeStructInst *structType);
  GlobalSection *addVariable(VariableInst *var);

  VariableInst *getInvocationId();
  VariableInst *getNumWorkgroups();

private:
  // TODO: Add structure to this.
  // Separate types, constants, variables, etc.
  std::vector<Instruction *> mGlobalDefs;
  std::unique_ptr<VariableInst> mInvocationId;
  std::unique_ptr<VariableInst> mNumWorkgroups;

  ContainerDeleter<std::vector<Instruction *>> mGlobalDefsDeleter;
};

class FunctionDeclaration : public Entity {
public:
  virtual ~FunctionDeclaration() {}

  bool DeserializeInternal(InputWordStream &IS) override;

  void accept(IVisitor *v) override {
    v->visit(mFunc);
    for (auto param : mParams) {
      v->visit(param);
    }
    v->visit(mFuncEnd);
  }

private:
  FunctionInst *mFunc;
  std::vector<FunctionParameterInst *> mParams;
  FunctionEndInst *mFuncEnd;
};

class Block : public Entity {
public:
  Block() {}
  Block(Builder *b) : Entity(b) {}

  virtual ~Block() {}

  bool DeserializeInternal(InputWordStream &IS) override;

  void accept(IVisitor *v) override {
    for (auto inst : mInsts) {
      v->visit(inst);
    }
  }

  Block *addInstruction(Instruction *inst) {
    mInsts.push_back(inst);
    return this;
  }

private:
  std::vector<Instruction *> mInsts;
};

class FunctionDefinition : public Entity {
public:
  FunctionDefinition();
  FunctionDefinition(Builder *builder, FunctionInst *func,
                     FunctionEndInst *end);

  virtual ~FunctionDefinition() {}

  bool DeserializeInternal(InputWordStream &IS) override;

  void accept(IVisitor *v) override {
    v->visit(mFunc.get());
    for (auto param : mParams) {
      v->visit(param);
    }
    for (auto block : mBlocks) {
      v->visit(block);
    }
    v->visit(mFuncEnd.get());
  }

  FunctionDefinition *addBlock(Block *b) {
    mBlocks.push_back(b);
    return this;
  }

  FunctionInst *getInstruction() const { return mFunc.get(); }
  FunctionParameterInst *getParameter(uint32_t i) const { return mParams[i]; }

  Instruction *getReturnType() const;

private:
  std::unique_ptr<FunctionInst> mFunc;
  std::vector<FunctionParameterInst *> mParams;
  std::vector<Block *> mBlocks;
  std::unique_ptr<FunctionEndInst> mFuncEnd;

  ContainerDeleter<std::vector<FunctionParameterInst *>> mParamsDeleter;
  ContainerDeleter<std::vector<Block *>> mBlocksDeleter;
};

} // namespace spirit
} // namespace android

#endif // MODULE_H
