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

#include "module.h"

#include <set>

#include "builder.h"
#include "core_defs.h"
#include "instructions.h"
#include "types_generated.h"
#include "word_stream.h"

namespace android {
namespace spirit {

Module *Module::mInstance = nullptr;

Module *Module::getCurrentModule() {
  if (mInstance == nullptr) {
    return mInstance = new Module();
  }
  return mInstance;
}

Module::Module()
    : mNextId(1), mCapabilitiesDeleter(mCapabilities),
      mExtensionsDeleter(mExtensions), mExtInstImportsDeleter(mExtInstImports),
      mEntryPointInstsDeleter(mEntryPointInsts),
      mExecutionModesDeleter(mExecutionModes),
      mEntryPointsDeleter(mEntryPoints),
      mFunctionDefinitionsDeleter(mFunctionDefinitions) {
  mInstance = this;
}

Module::Module(Builder *b)
    : Entity(b), mNextId(1), mCapabilitiesDeleter(mCapabilities),
      mExtensionsDeleter(mExtensions), mExtInstImportsDeleter(mExtInstImports),
      mEntryPointInstsDeleter(mEntryPointInsts),
      mExecutionModesDeleter(mExecutionModes),
      mEntryPointsDeleter(mEntryPoints),
      mFunctionDefinitionsDeleter(mFunctionDefinitions) {
  mInstance = this;
}

bool Module::resolveIds() {
  auto &table = mIdTable;

  std::unique_ptr<IVisitor> v0(
      CreateInstructionVisitor([&table](Instruction *inst) {
        if (inst->hasResult()) {
          table.insert(std::make_pair(inst->getId(), inst));
        }
      }));
  v0->visit(this);

  mNextId = mIdTable.rbegin()->first + 1;

  int err = 0;
  std::unique_ptr<IVisitor> v(
      CreateInstructionVisitor([&table, &err](Instruction *inst) {
        for (auto ref : inst->getAllIdRefs()) {
          if (ref) {
            auto it = table.find(ref->mId);
            if (it != table.end()) {
              ref->mInstruction = it->second;
            } else {
              std::cout << "Found no instruction for id " << ref->mId
                        << std::endl;
              err++;
            }
          }
        }
      }));
  v->visit(this);
  return err == 0;
}

bool Module::DeserializeInternal(InputWordStream &IS) {
  if (IS.empty()) {
    return false;
  }

  IS >> &mMagicNumber;
  if (mMagicNumber != 0x07230203) {
    errs() << "Wrong Magic Number: " << mMagicNumber;
    return false;
  }

  if (IS.empty()) {
    return false;
  }

  IS >> &mVersion.mWord;
  if (mVersion.mBytes[0] != 0 || mVersion.mBytes[3] != 0) {
    return false;
  }

  if (IS.empty()) {
    return false;
  }

  IS >> &mGeneratorMagicNumber >> &mBound >> &mReserved;

  DeserializeZeroOrMore<CapabilityInst>(IS, mCapabilities);
  DeserializeZeroOrMore<ExtensionInst>(IS, mExtensions);
  DeserializeZeroOrMore<ExtInstImportInst>(IS, mExtInstImports);

  mMemoryModel.reset(Deserialize<MemoryModelInst>(IS));
  if (!mMemoryModel) {
    errs() << "Missing memory model specification.\n";
    return false;
  }

  DeserializeZeroOrMore<EntryPointDefinition>(IS, mEntryPoints);
  DeserializeZeroOrMore<ExecutionModeInst>(IS, mExecutionModes);
  for (auto entry : mEntryPoints) {
    mEntryPointInsts.push_back(entry->getInstruction());
    for (auto mode : mExecutionModes) {
      entry->applyExecutionMode(mode);
    }
  }

  mDebugInfo.reset(Deserialize<DebugInfoSection>(IS));
  mAnnotations.reset(Deserialize<AnnotationSection>(IS));
  mGlobals.reset(Deserialize<GlobalSection>(IS));

  DeserializeZeroOrMore<FunctionDefinition>(IS, mFunctionDefinitions);

  if (mFunctionDefinitions.empty()) {
    errs() << "Missing function definitions.\n";
    for (int i = 0; i < 4; i++) {
      uint32_t w;
      IS >> &w;
      std::cout << std::hex << w << " ";
    }
    std::cout << std::endl;
    return false;
  }

  return true;
}

void Module::initialize() {
  mMagicNumber = 0x07230203;
  mVersion.mMajorMinor = {.mMinorNumber = 1, .mMajorNumber = 1};
  mGeneratorMagicNumber = 0x00070000;
  mBound = 0;
  mReserved = 0;
  mAnnotations.reset(new AnnotationSection());
}

void Module::SerializeHeader(OutputWordStream &OS) const {
  OS << mMagicNumber;
  OS << mVersion.mWord << mGeneratorMagicNumber;
  if (mBound == 0) {
    OS << mIdTable.end()->first + 1;
  } else {
    OS << std::max(mBound, mNextId);
  }
  OS << mReserved;
}

void Module::Serialize(OutputWordStream &OS) const {
  SerializeHeader(OS);
  Entity::Serialize(OS);
}

Module *Module::addCapability(Capability cap) {
  mCapabilities.push_back(mBuilder->MakeCapability(cap));
  return this;
}

Module *Module::setMemoryModel(AddressingModel am, MemoryModel mm) {
  mMemoryModel.reset(mBuilder->MakeMemoryModel(am, mm));
  return this;
}

Module *Module::addExtInstImport(const char *extName) {
  ExtInstImportInst *extInst = mBuilder->MakeExtInstImport(extName);
  mExtInstImports.push_back(extInst);
  if (strcmp(extName, "GLSL.std.450") == 0) {
    mGLExt = extInst;
  }
  return this;
}

Module *Module::addSource(SourceLanguage lang, int version) {
  if (!mDebugInfo) {
    mDebugInfo.reset(mBuilder->MakeDebugInfoSection());
  }
  mDebugInfo->addSource(lang, version);
  return this;
}

Module *Module::addSourceExtension(const char *ext) {
  if (!mDebugInfo) {
    mDebugInfo.reset(mBuilder->MakeDebugInfoSection());
  }
  mDebugInfo->addSourceExtension(ext);
  return this;
}

Module *Module::addString(const char *str) {
  if (!mDebugInfo) {
    mDebugInfo.reset(mBuilder->MakeDebugInfoSection());
  }
  mDebugInfo->addString(str);
  return this;
}

Module *Module::addEntryPoint(EntryPointDefinition *entry) {
  mEntryPoints.push_back(entry);
  auto newModes = entry->getExecutionModes();
  mExecutionModes.insert(mExecutionModes.end(), newModes.begin(),
                         newModes.end());
  return this;
}

const std::string Module::findStringOfPrefix(const char *prefix) const {
  if (!mDebugInfo) {
    return std::string();
  }
  return mDebugInfo->findStringOfPrefix(prefix);
}

GlobalSection *Module::getGlobalSection() {
  if (!mGlobals) {
    mGlobals.reset(new GlobalSection());
  }
  return mGlobals.get();
}

ConstantInst *Module::getConstant(TypeIntInst *type, int32_t value) {
  return getGlobalSection()->getConstant(type, value);
}

ConstantInst *Module::getConstant(TypeIntInst *type, uint32_t value) {
  return getGlobalSection()->getConstant(type, value);
}

ConstantInst *Module::getConstant(TypeFloatInst *type, float value) {
  return getGlobalSection()->getConstant(type, value);
}

ConstantCompositeInst *Module::getConstantComposite(TypeVectorInst *type,
                                                    ConstantInst *components[],
                                                    size_t width) {
  return getGlobalSection()->getConstantComposite(type, components, width);
}

ConstantCompositeInst *Module::getConstantComposite(TypeVectorInst *type,
                                                    ConstantInst *comp0,
                                                    ConstantInst *comp1,
                                                    ConstantInst *comp2) {
  // TODO: verify that component types are the same and consistent with the
  // resulting vector type
  ConstantInst *comps[] = {comp0, comp1, comp2};
  return getConstantComposite(type, comps, 3);
}

ConstantCompositeInst *Module::getConstantComposite(TypeVectorInst *type,
                                                    ConstantInst *comp0,
                                                    ConstantInst *comp1,
                                                    ConstantInst *comp2,
                                                    ConstantInst *comp3) {
  // TODO: verify that component types are the same and consistent with the
  // resulting vector type
  ConstantInst *comps[] = {comp0, comp1, comp2, comp3};
  return getConstantComposite(type, comps, 4);
}

TypeVoidInst *Module::getVoidType() {
  return getGlobalSection()->getVoidType();
}

TypeIntInst *Module::getIntType(int bits, bool isSigned) {
  return getGlobalSection()->getIntType(bits, isSigned);
}

TypeIntInst *Module::getUnsignedIntType(int bits) {
  return getIntType(bits, false);
}

TypeFloatInst *Module::getFloatType(int bits) {
  return getGlobalSection()->getFloatType(bits);
}

TypeVectorInst *Module::getVectorType(Instruction *componentType, int width) {
  return getGlobalSection()->getVectorType(componentType, width);
}

TypePointerInst *Module::getPointerType(StorageClass storage,
                                        Instruction *pointeeType) {
  return getGlobalSection()->getPointerType(storage, pointeeType);
}

TypeRuntimeArrayInst *Module::getRuntimeArrayType(Instruction *elementType) {
  return getGlobalSection()->getRuntimeArrayType(elementType);
}

TypeStructInst *Module::getStructType(Instruction *fieldType[], int numField) {
  return getGlobalSection()->getStructType(fieldType, numField);
}

TypeStructInst *Module::getStructType(Instruction *fieldType) {
  return getStructType(&fieldType, 1);
}

TypeFunctionInst *Module::getFunctionType(Instruction *retType,
                                          Instruction *const argType[],
                                          size_t numArg) {
  return getGlobalSection()->getFunctionType(retType, argType, numArg);
}

TypeFunctionInst *
Module::getFunctionType(Instruction *retType,
                        const std::vector<Instruction *> &argTypes) {
  return getGlobalSection()->getFunctionType(retType, argTypes.data(),
                                             argTypes.size());
}

size_t Module::getSize(TypeVoidInst *) { return 0; }

size_t Module::getSize(TypeIntInst *intTy) { return intTy->mOperand1 / 8; }

size_t Module::getSize(TypeFloatInst *fpTy) { return fpTy->mOperand1 / 8; }

size_t Module::getSize(TypeVectorInst *vTy) {
  return getSize(vTy->mOperand1.mInstruction) * vTy->mOperand2;
}

size_t Module::getSize(TypePointerInst *) {
  return 4; // TODO: or 8?
}

size_t Module::getSize(TypeStructInst *structTy) {
  size_t sz = 0;
  for (auto ty : structTy->mOperand1) {
    sz += getSize(ty.mInstruction);
  }
  return sz;
}

size_t Module::getSize(TypeFunctionInst *) {
  return 4; // TODO: or 8? Is this just the size of a pointer?
}

size_t Module::getSize(Instruction *inst) {
  switch (inst->getOpCode()) {
  case OpTypeVoid:
    return getSize(static_cast<TypeVoidInst *>(inst));
  case OpTypeInt:
    return getSize(static_cast<TypeIntInst *>(inst));
  case OpTypeFloat:
    return getSize(static_cast<TypeFloatInst *>(inst));
  case OpTypeVector:
    return getSize(static_cast<TypeVectorInst *>(inst));
  case OpTypeStruct:
    return getSize(static_cast<TypeStructInst *>(inst));
  case OpTypeFunction:
    return getSize(static_cast<TypeFunctionInst *>(inst));
  default:
    return 0;
  }
}

Module *Module::addFunctionDefinition(FunctionDefinition *func) {
  mFunctionDefinitions.push_back(func);
  return this;
}

Instruction *Module::lookupByName(const char *name) const {
  return mDebugInfo->lookupByName(name);
}

FunctionDefinition *
Module::getFunctionDefinitionFromInstruction(FunctionInst *inst) const {
  for (auto fdef : mFunctionDefinitions) {
    if (fdef->getInstruction() == inst) {
      return fdef;
    }
  }
  return nullptr;
}

FunctionDefinition *
Module::lookupFunctionDefinitionByName(const char *name) const {
  FunctionInst *inst = static_cast<FunctionInst *>(lookupByName(name));
  return getFunctionDefinitionFromInstruction(inst);
}

const char *Module::lookupNameByInstruction(const Instruction *inst) const {
  return mDebugInfo->lookupNameByInstruction(inst);
}

VariableInst *Module::getInvocationId() {
  return getGlobalSection()->getInvocationId();
}

VariableInst *Module::getNumWorkgroups() {
  return getGlobalSection()->getNumWorkgroups();
}

Module *Module::addStructType(TypeStructInst *structType) {
  getGlobalSection()->addStructType(structType);
  return this;
}

Module *Module::addVariable(VariableInst *var) {
  getGlobalSection()->addVariable(var);
  return this;
}

void Module::consolidateAnnotations() {
  std::vector<Instruction *> annotations(mAnnotations->begin(),
                                      mAnnotations->end());
  std::unique_ptr<IVisitor> v(
      CreateInstructionVisitor([&annotations](Instruction *inst) -> void {
        const auto &ann = inst->getAnnotations();
        annotations.insert(annotations.end(), ann.begin(), ann.end());
      }));
  v->visit(this);
  mAnnotations->clear();
  mAnnotations->addAnnotations(annotations.begin(), annotations.end());
}

EntryPointDefinition::EntryPointDefinition(Builder *builder,
                                           ExecutionModel execModel,
                                           FunctionDefinition *func,
                                           const char *name)
    : Entity(builder), mFunction(func->getInstruction()),
      mExecutionModel(execModel) {
  mName = strndup(name, strlen(name));
  mEntryPointInst = mBuilder->MakeEntryPoint(execModel, mFunction, mName);
  (void)mExecutionModel; // suppress unused private field warning
}

bool EntryPointDefinition::DeserializeInternal(InputWordStream &IS) {
  if (IS.empty()) {
    return false;
  }

  if ((mEntryPointInst = Deserialize<EntryPointInst>(IS))) {
    return true;
  }

  return false;
}

EntryPointDefinition *
EntryPointDefinition::applyExecutionMode(ExecutionModeInst *mode) {
  if (mode->mOperand1.mInstruction == mFunction) {
    addExecutionMode(mode);
  }
  return this;
}

EntryPointDefinition *EntryPointDefinition::addToInterface(VariableInst *var) {
  mInterface.push_back(var);
  mEntryPointInst->mOperand4.push_back(var);
  return this;
}

EntryPointDefinition *EntryPointDefinition::setLocalSize(uint32_t width,
                                                         uint32_t height,
                                                         uint32_t depth) {
  mLocalSize.mWidth = width;
  mLocalSize.mHeight = height;
  mLocalSize.mDepth = depth;

  auto mode = mBuilder->MakeExecutionMode(mFunction, ExecutionMode::LocalSize);
  mode->addExtraOperand(width)->addExtraOperand(height)->addExtraOperand(depth);

  addExecutionMode(mode);

  return this;
}

bool DebugInfoSection::DeserializeInternal(InputWordStream &IS) {
  while (true) {
    if (auto str = Deserialize<StringInst>(IS)) {
      mSources.push_back(str);
    } else if (auto src = Deserialize<SourceInst>(IS)) {
      mSources.push_back(src);
    } else if (auto srcExt = Deserialize<SourceExtensionInst>(IS)) {
      mSources.push_back(srcExt);
    } else if (auto srcCont = Deserialize<SourceContinuedInst>(IS)) {
      mSources.push_back(srcCont);
    } else {
      break;
    }
  }

  while (true) {
    if (auto name = Deserialize<NameInst>(IS)) {
      mNames.push_back(name);
    } else if (auto memName = Deserialize<MemberNameInst>(IS)) {
      mNames.push_back(memName);
    } else {
      break;
    }
  }

  return true;
}

DebugInfoSection *DebugInfoSection::addSource(SourceLanguage lang,
                                              int version) {
  SourceInst *source = mBuilder->MakeSource(lang, version);
  mSources.push_back(source);
  return this;
}

DebugInfoSection *DebugInfoSection::addSourceExtension(const char *ext) {
  SourceExtensionInst *inst = mBuilder->MakeSourceExtension(ext);
  mSources.push_back(inst);
  return this;
}

DebugInfoSection *DebugInfoSection::addString(const char *str) {
  StringInst *source = mBuilder->MakeString(str);
  mSources.push_back(source);
  return this;
}

std::string DebugInfoSection::findStringOfPrefix(const char *prefix) {
  auto it = std::find_if(
      mSources.begin(), mSources.end(), [prefix](Instruction *inst) -> bool {
        if (inst->getOpCode() != OpString) {
          return false;
        }
        const StringInst *strInst = static_cast<const StringInst *>(inst);
        const std::string &str = strInst->mOperand1;
        return str.find(prefix) == 0;
      });
  if (it == mSources.end()) {
    return "";
  }
  StringInst *strInst = static_cast<StringInst *>(*it);
  return strInst->mOperand1;
}

Instruction *DebugInfoSection::lookupByName(const char *name) const {
  for (auto inst : mNames) {
    if (inst->getOpCode() == OpName) {
      NameInst *nameInst = static_cast<NameInst *>(inst);
      if (nameInst->mOperand2.compare(name) == 0) {
        return nameInst->mOperand1.mInstruction;
      }
    }
    // Ignore member names
  }
  return nullptr;
}

const char *
DebugInfoSection::lookupNameByInstruction(const Instruction *target) const {
  for (auto inst : mNames) {
    if (inst->getOpCode() == OpName) {
      NameInst *nameInst = static_cast<NameInst *>(inst);
      if (nameInst->mOperand1.mInstruction == target) {
        return nameInst->mOperand2.c_str();
      }
    }
    // Ignore member names
  }
  return nullptr;
}

AnnotationSection::AnnotationSection() : mAnnotationsDeleter(mAnnotations) {}

AnnotationSection::AnnotationSection(Builder *b)
    : Entity(b), mAnnotationsDeleter(mAnnotations) {}

bool AnnotationSection::DeserializeInternal(InputWordStream &IS) {
  while (true) {
    if (auto decor = Deserialize<DecorateInst>(IS)) {
      mAnnotations.push_back(decor);
    } else if (auto decor = Deserialize<MemberDecorateInst>(IS)) {
      mAnnotations.push_back(decor);
    } else if (auto decor = Deserialize<GroupDecorateInst>(IS)) {
      mAnnotations.push_back(decor);
    } else if (auto decor = Deserialize<GroupMemberDecorateInst>(IS)) {
      mAnnotations.push_back(decor);
    } else if (auto decor = Deserialize<DecorationGroupInst>(IS)) {
      mAnnotations.push_back(decor);
    } else {
      break;
    }
  }
  return true;
}

GlobalSection::GlobalSection() : mGlobalDefsDeleter(mGlobalDefs) {}

GlobalSection::GlobalSection(Builder *builder)
    : Entity(builder), mGlobalDefsDeleter(mGlobalDefs) {}

namespace {

template <typename T>
T *findOrCreate(std::function<bool(T *)> criteria, std::function<T *()> factory,
                std::vector<Instruction *> *globals) {
  T *derived;
  for (auto inst : *globals) {
    if (inst->getOpCode() == T::mOpCode) {
      T *derived = static_cast<T *>(inst);
      if (criteria(derived)) {
        return derived;
      }
    }
  }
  derived = factory();
  globals->push_back(derived);
  return derived;
}

} // anonymous namespace

bool GlobalSection::DeserializeInternal(InputWordStream &IS) {
  while (true) {
#define HANDLE_INSTRUCTION(OPCODE, INST_CLASS)                                 \
  if (auto typeInst = Deserialize<INST_CLASS>(IS)) {                           \
    mGlobalDefs.push_back(typeInst);                                           \
    continue;                                                                  \
  }
#include "const_inst_dispatches_generated.h"
#include "type_inst_dispatches_generated.h"
#undef HANDLE_INSTRUCTION

    if (auto globalInst = Deserialize<VariableInst>(IS)) {
      // Check if this is function scoped
      if (globalInst->mOperand1 == StorageClass::Function) {
        Module::errs() << "warning: Variable (id = " << globalInst->mResult;
        Module::errs() << ") has function scope in global section.\n";
        // Khronos LLVM-SPIRV convertor emits "Function" storage-class globals.
        // As a workaround, accept such SPIR-V code here, and fix it up later
        // in the rs2spirv compiler by correcting the storage class.
        // In a stricter deserializer, such code should be rejected, and we
        // should return false here.
      }
      mGlobalDefs.push_back(globalInst);
      continue;
    }

    if (auto globalInst = Deserialize<UndefInst>(IS)) {
      mGlobalDefs.push_back(globalInst);
      continue;
    }
    break;
  }
  return true;
}

ConstantInst *GlobalSection::getConstant(TypeIntInst *type, int32_t value) {
  return findOrCreate<ConstantInst>(
      [=](ConstantInst *c) { return c->mOperand1.intValue == value; },
      [=]() -> ConstantInst * {
        LiteralContextDependentNumber cdn = {.intValue = value};
        return mBuilder->MakeConstant(type, cdn);
      },
      &mGlobalDefs);
}

ConstantInst *GlobalSection::getConstant(TypeIntInst *type, uint32_t value) {
  return findOrCreate<ConstantInst>(
      [=](ConstantInst *c) { return c->mOperand1.intValue == (int)value; },
      [=]() -> ConstantInst * {
        LiteralContextDependentNumber cdn = {.intValue = (int)value};
        return mBuilder->MakeConstant(type, cdn);
      },
      &mGlobalDefs);
}

ConstantInst *GlobalSection::getConstant(TypeFloatInst *type, float value) {
  return findOrCreate<ConstantInst>(
      [=](ConstantInst *c) { return c->mOperand1.floatValue == value; },
      [=]() -> ConstantInst * {
        LiteralContextDependentNumber cdn = {.floatValue = value};
        return mBuilder->MakeConstant(type, cdn);
      },
      &mGlobalDefs);
}

ConstantCompositeInst *
GlobalSection::getConstantComposite(TypeVectorInst *type,
                                    ConstantInst *components[], size_t width) {
  return findOrCreate<ConstantCompositeInst>(
      [=](ConstantCompositeInst *c) {
        if (c->mOperand1.size() != width) {
          return false;
        }
        for (size_t i = 0; i < width; i++) {
          if (c->mOperand1[i].mInstruction != components[i]) {
            return false;
          }
        }
        return true;
      },
      [=]() -> ConstantCompositeInst * {
        ConstantCompositeInst *c = mBuilder->MakeConstantComposite(type);
        for (size_t i = 0; i < width; i++) {
          c->mOperand1.push_back(components[i]);
        }
        return c;
      },
      &mGlobalDefs);
}

TypeVoidInst *GlobalSection::getVoidType() {
  return findOrCreate<TypeVoidInst>(
      [=](TypeVoidInst *) -> bool { return true; },
      [=]() -> TypeVoidInst * { return mBuilder->MakeTypeVoid(); },
      &mGlobalDefs);
}

TypeIntInst *GlobalSection::getIntType(int bits, bool isSigned) {
  if (isSigned) {
    switch (bits) {
#define HANDLE_INT_SIZE(INT_TYPE, BITS, SIGNED)                                \
  case BITS: {                                                                 \
    return findOrCreate<TypeIntInst>(                                          \
        [=](TypeIntInst *intTy) -> bool {                                      \
          return intTy->mOperand1 == BITS && intTy->mOperand2 == SIGNED;       \
        },                                                                     \
        [=]() -> TypeIntInst * {                                               \
          return mBuilder->MakeTypeInt(BITS, SIGNED);                          \
        },                                                                     \
        &mGlobalDefs);                                                         \
  }
      HANDLE_INT_SIZE(Int, 8, 1);
      HANDLE_INT_SIZE(Int, 16, 1);
      HANDLE_INT_SIZE(Int, 32, 1);
      HANDLE_INT_SIZE(Int, 64, 1);
    default:
      Module::errs() << "unexpected int type";
    }
  } else {
    switch (bits) {
      HANDLE_INT_SIZE(UInt, 8, 0);
      HANDLE_INT_SIZE(UInt, 16, 0);
      HANDLE_INT_SIZE(UInt, 32, 0);
      HANDLE_INT_SIZE(UInt, 64, 0);
    default:
      Module::errs() << "unexpected int type";
    }
  }
#undef HANDLE_INT_SIZE
  return nullptr;
}

TypeFloatInst *GlobalSection::getFloatType(int bits) {
  switch (bits) {
#define HANDLE_FLOAT_SIZE(BITS)                                                \
  case BITS: {                                                                 \
    return findOrCreate<TypeFloatInst>(                                        \
        [=](TypeFloatInst *floatTy) -> bool {                                  \
          return floatTy->mOperand1 == BITS;                                   \
        },                                                                     \
        [=]() -> TypeFloatInst * { return mBuilder->MakeTypeFloat(BITS); },    \
        &mGlobalDefs);                                                         \
  }
    HANDLE_FLOAT_SIZE(16);
    HANDLE_FLOAT_SIZE(32);
    HANDLE_FLOAT_SIZE(64);
  default:
    Module::errs() << "unexpeced floating point type";
  }
#undef HANDLE_FLOAT_SIZE
  return nullptr;
}

TypeVectorInst *GlobalSection::getVectorType(Instruction *componentType,
                                             int width) {
  // TODO: verify that componentType is basic numeric types

  return findOrCreate<TypeVectorInst>(
      [=](TypeVectorInst *vecTy) -> bool {
        return vecTy->mOperand1.mInstruction == componentType &&
               vecTy->mOperand2 == width;
      },
      [=]() -> TypeVectorInst * {
        return mBuilder->MakeTypeVector(componentType, width);
      },
      &mGlobalDefs);
}

TypePointerInst *GlobalSection::getPointerType(StorageClass storage,
                                               Instruction *pointeeType) {
  return findOrCreate<TypePointerInst>(
      [=](TypePointerInst *type) -> bool {
        return type->mOperand1 == storage &&
               type->mOperand2.mInstruction == pointeeType;
      },
      [=]() -> TypePointerInst * {
        return mBuilder->MakeTypePointer(storage, pointeeType);
      },
      &mGlobalDefs);
}

TypeRuntimeArrayInst *
GlobalSection::getRuntimeArrayType(Instruction *elemType) {
  return findOrCreate<TypeRuntimeArrayInst>(
      [=](TypeRuntimeArrayInst * /*type*/) -> bool {
        // return type->mOperand1.mInstruction == elemType;
        return false;
      },
      [=]() -> TypeRuntimeArrayInst * {
        return mBuilder->MakeTypeRuntimeArray(elemType);
      },
      &mGlobalDefs);
}

TypeStructInst *GlobalSection::getStructType(Instruction *fieldType[],
                                             int numField) {
  TypeStructInst *structTy = mBuilder->MakeTypeStruct();
  for (int i = 0; i < numField; i++) {
    structTy->mOperand1.push_back(fieldType[i]);
  }
  mGlobalDefs.push_back(structTy);
  return structTy;
}

TypeFunctionInst *GlobalSection::getFunctionType(Instruction *retType,
                                                 Instruction *const argType[],
                                                 size_t numArg) {
  return findOrCreate<TypeFunctionInst>(
      [=](TypeFunctionInst *type) -> bool {
        if (type->mOperand1.mInstruction != retType ||
            type->mOperand2.size() != numArg) {
          return false;
        }
        for (size_t i = 0; i < numArg; i++) {
          if (type->mOperand2[i].mInstruction != argType[i]) {
            return false;
          }
        }
        return true;
      },
      [=]() -> TypeFunctionInst * {
        TypeFunctionInst *funcTy = mBuilder->MakeTypeFunction(retType);
        for (size_t i = 0; i < numArg; i++) {
          funcTy->mOperand2.push_back(argType[i]);
        }
        return funcTy;
      },
      &mGlobalDefs);
}

GlobalSection *GlobalSection::addStructType(TypeStructInst *structType) {
  mGlobalDefs.push_back(structType);
  return this;
}

GlobalSection *GlobalSection::addVariable(VariableInst *var) {
  mGlobalDefs.push_back(var);
  return this;
}

VariableInst *GlobalSection::getInvocationId() {
  if (mInvocationId) {
    return mInvocationId.get();
  }

  TypeIntInst *UIntTy = getIntType(32, false);
  TypeVectorInst *V3UIntTy = getVectorType(UIntTy, 3);
  TypePointerInst *V3UIntPtrTy = getPointerType(StorageClass::Input, V3UIntTy);

  VariableInst *InvocationId =
      mBuilder->MakeVariable(V3UIntPtrTy, StorageClass::Input);
  InvocationId->decorate(Decoration::BuiltIn)
      ->addExtraOperand(static_cast<uint32_t>(BuiltIn::GlobalInvocationId));

  mInvocationId.reset(InvocationId);

  return InvocationId;
}

VariableInst *GlobalSection::getNumWorkgroups() {
  if (mNumWorkgroups) {
    return mNumWorkgroups.get();
  }

  TypeIntInst *UIntTy = getIntType(32, false);
  TypeVectorInst *V3UIntTy = getVectorType(UIntTy, 3);
  TypePointerInst *V3UIntPtrTy = getPointerType(StorageClass::Input, V3UIntTy);

  VariableInst *GNum = mBuilder->MakeVariable(V3UIntPtrTy, StorageClass::Input);
  GNum->decorate(Decoration::BuiltIn)
      ->addExtraOperand(static_cast<uint32_t>(BuiltIn::NumWorkgroups));

  mNumWorkgroups.reset(GNum);

  return GNum;
}

bool FunctionDeclaration::DeserializeInternal(InputWordStream &IS) {
  if (!(mFunc = Deserialize<FunctionInst>(IS))) {
    return false;
  }

  DeserializeZeroOrMore<FunctionParameterInst>(IS, mParams);

  if (!(mFuncEnd = Deserialize<FunctionEndInst>(IS))) {
    return false;
  }

  return true;
}

template <> Instruction *Deserialize(InputWordStream &IS) {
  Instruction *inst;

  switch ((*IS) & 0xFFFF) {
#define HANDLE_INSTRUCTION(OPCODE, INST_CLASS)                                 \
  case OPCODE:                                                                 \
    inst = Deserialize<INST_CLASS>(IS);                                        \
    break;
#include "instruction_dispatches_generated.h"
#undef HANDLE_INSTRUCTION
  default:
    Module::errs() << "unrecognized instruction";
    inst = nullptr;
  }

  return inst;
}

bool Block::DeserializeInternal(InputWordStream &IS) {
  Instruction *inst;
  while (((*IS) & 0xFFFF) != OpFunctionEnd &&
         (inst = Deserialize<Instruction>(IS))) {
    mInsts.push_back(inst);
    if (inst->getOpCode() == OpBranch ||
        inst->getOpCode() == OpBranchConditional ||
        inst->getOpCode() == OpSwitch || inst->getOpCode() == OpKill ||
        inst->getOpCode() == OpReturn || inst->getOpCode() == OpReturnValue ||
        inst->getOpCode() == OpUnreachable) {
      break;
    }
  }
  return !mInsts.empty();
}

FunctionDefinition::FunctionDefinition()
    : mParamsDeleter(mParams), mBlocksDeleter(mBlocks) {}

FunctionDefinition::FunctionDefinition(Builder *builder, FunctionInst *func,
                                       FunctionEndInst *end)
    : Entity(builder), mFunc(func), mFuncEnd(end), mParamsDeleter(mParams),
      mBlocksDeleter(mBlocks) {}

bool FunctionDefinition::DeserializeInternal(InputWordStream &IS) {
  mFunc.reset(Deserialize<FunctionInst>(IS));
  if (!mFunc) {
    return false;
  }

  DeserializeZeroOrMore<FunctionParameterInst>(IS, mParams);
  DeserializeZeroOrMore<Block>(IS, mBlocks);

  mFuncEnd.reset(Deserialize<FunctionEndInst>(IS));
  if (!mFuncEnd) {
    return false;
  }

  return true;
}

Instruction *FunctionDefinition::getReturnType() const {
  return mFunc->mResultType.mInstruction;
}

} // namespace spirit
} // namespace android
