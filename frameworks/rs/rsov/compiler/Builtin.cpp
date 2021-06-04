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

#include "Builtin.h"

#include "cxxabi.h"
#include "spirit.h"
#include "transformer.h"

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

namespace android {
namespace spirit {

namespace {

Instruction *translateClampVector(const char *name,
                                  const FunctionCallInst *call, Transformer *tr,
                                  Builder *b, Module *m) {
  int width = name[10] - '0';
  if (width < 2 || width > 4) {
    return nullptr;
  }

  uint32_t extOpCode = 0;
  switch (name[strlen(name) - 1]) {
  case 'f':
    extOpCode = 43;
    break; // FClamp
  // TODO: Do we get _Z5clampDV_uuu at all? Does LLVM convert u into i?
  case 'u':
    extOpCode = 44;
    break; // UClamp
  case 'i':
    extOpCode = 45;
    break; // SClamp
  default:
    return nullptr;
  }

  std::vector<IdRef> minConstituents(width, call->mOperand2[1]);
  std::unique_ptr<Instruction> min(
      b->MakeCompositeConstruct(call->mResultType, minConstituents));
  tr->insert(min.get());

  std::vector<IdRef> maxConstituents(width, call->mOperand2[2]);
  std::unique_ptr<Instruction> max(
      b->MakeCompositeConstruct(call->mResultType, maxConstituents));
  tr->insert(max.get());

  std::vector<IdRef> extOpnds = {call->mOperand2[0], min.get(), max.get()};
  return b->MakeExtInst(call->mResultType, m->getGLExt(), extOpCode, extOpnds);
}

Instruction *translateExtInst(const uint32_t extOpCode,
                              const FunctionCallInst *call, Builder *b,
                              Module *m) {
  return b->MakeExtInst(call->mResultType, m->getGLExt(), extOpCode,
                        {call->mOperand2[0]});
}

} // anonymous namespace

typedef std::function<Instruction *(const char *, const FunctionCallInst *,
                                    Transformer *, Builder *, Module *)>
    InstTrTy;

class BuiltinLookupTable {
public:
  BuiltinLookupTable() {
    for (sNameCode const *p = &mFPMathFuncOpCode[0]; p->name; p++) {
      const char *name = p->name;
      const uint32_t extOpCode = p->code;
      addMapping(name, {"*"}, {{"float+"}}, {1, 2, 3, 4},
                 [extOpCode](const char *, const FunctionCallInst *call,
                             Transformer *, Builder *b, Module *m) {
                   return translateExtInst(extOpCode, call, b, m);
                 });
    }

    addMapping("abs", {"*"}, {{"int+"}, {"char+"}}, {1, 2, 3, 4},
               [](const char *, const FunctionCallInst *call, Transformer *,
                  Builder *b, Module *m) {
                 return translateExtInst(5, call, b, m); // SAbs
               });

    addMapping("clamp", {"*"},
               {{"int+", "int", "int"}, {"float+", "float", "float"}},
               {1, 2, 3, 4}, [](const char *name, const FunctionCallInst *call,
                                Transformer *tr, Builder *b, Module *m) {
                 return translateClampVector(name, call, tr, b, m);
               });

    addMapping("convert", {"char+", "int+", "uchar+", "uint+"},
               {{"char+"}, {"int+"}, {"uchar+"}, {"uint+"}}, {1, 2, 3, 4},
               [](const char *, const FunctionCallInst *call, Transformer *,
                  Builder *b, Module *) -> Instruction * {
                 return b->MakeUConvert(call->mResultType, call->mOperand2[0]);
               });

    addMapping(
        "convert", {"char+", "int+", "uchar+", "uint+"}, {{"float+"}},
        {1, 2, 3, 4}, [](const char *, const FunctionCallInst *call,
                         Transformer *, Builder *b, Module *) -> Instruction * {
          return b->MakeConvertFToU(call->mResultType, call->mOperand2[0]);
        });

    addMapping(
        "convert", {"float+"}, {{"char+"}, {"int+"}, {"uchar+"}, {"uint+"}},
        {1, 2, 3, 4}, [](const char *, const FunctionCallInst *call,
                         Transformer *, Builder *b, Module *) {
          return b->MakeConvertUToF(call->mResultType, call->mOperand2[0]);
        });

    addMapping("dot", {"*"}, {{"float+"}}, {1, 2, 3, 4},
               [](const char *, const FunctionCallInst *call, Transformer *,
                  Builder *b, Module *) {
                 return b->MakeDot(call->mResultType, call->mOperand2[0],
                                   call->mOperand2[1]);
               });

    addMapping("min", {"*"}, {{"uint+"}, {"uchar+"}}, {1, 2, 3, 4},
               [](const char *, const FunctionCallInst *call, Transformer *,
                  Builder *b, Module *m) {
                 return translateExtInst(38, call, b, m); // UMin
               });

    addMapping("min", {"*"}, {{"int+"}, {"char+"}}, {1, 2, 3, 4},
               [](const char *, const FunctionCallInst *call, Transformer *,
                  Builder *b, Module *m) {
                 return translateExtInst(39, call, b, m); // SMin
               });

    addMapping("max", {"*"}, {{"uint+"}, {"uchar+"}}, {1, 2, 3, 4},
               [](const char *, const FunctionCallInst *call, Transformer *,
                  Builder *b, Module *m) {
                 return translateExtInst(41, call, b, m); // UMax
               });

    addMapping("max", {"*"}, {{"int+"}, {"char+"}}, {1, 2, 3, 4},
               [](const char *, const FunctionCallInst *call, Transformer *,
                  Builder *b, Module *m) {
                 return translateExtInst(42, call, b, m); // SMax
               });

    addMapping("rsUnpackColor8888", {"*"}, {{"uchar+"}}, {4},
               [](const char *, const FunctionCallInst *call, Transformer *,
                  Builder *b, Module *m) {
                 auto cast = b->MakeBitcast(m->getUnsignedIntType(32),
                                            call->mOperand2[0]);
                 return b->MakeExtInst(call->mResultType, m->getGLExt(), 64,
                                       {cast}); // UnpackUnorm4x8
               });

    addMapping("rsPackColorTo8888", {"*"}, {{"float+"}}, {4},
               [](const char *, const FunctionCallInst *call, Transformer *,
                  Builder *b, Module *m) {
                 // PackUnorm4x8
                 auto packed = b->MakeExtInst(call->mResultType, m->getGLExt(),
                                              55, {call->mOperand2[0]});
                 return b->MakeBitcast(
                     m->getVectorType(m->getUnsignedIntType(8), 4), packed);
               });
  }

  static const BuiltinLookupTable &getInstance() {
    static BuiltinLookupTable table;
    return table;
  }

  void addMapping(const char *funcName,
                  const std::vector<std::string> &retTypes,
                  const std::vector<std::vector<std::string>> &argTypes,
                  const std::vector<uint8_t> &vecWidths, InstTrTy fp) {
    for (auto width : vecWidths) {
      for (auto retType : retTypes) {
        std::string suffixed(funcName);
        if (retType != "*") {
          if (retType.back() == '+') {
            retType.pop_back();
            if (width > 1) {
              retType.append(1, '0' + width);
            }
          }
          suffixed.append("_").append(retType);
        }

        for (auto argList : argTypes) {
          std::string args("(");
          bool first = true;
          for (auto argType : argList) {
            if (first) {
              first = false;
            } else {
              args.append(", ");
            }
            if (argType.front() == 'u') {
              argType.replace(0, 1, "unsigned ");
            }
            if (argType.back() == '+') {
              argType.pop_back();
              if (width > 1) {
                argType.append(" vector[");
                argType.append(1, '0' + width);
                argType.append("]");
              }
            }
            args.append(argType);
          }
          args.append(")");
          mFuncNameMap[suffixed + args] = fp;
        }
      }
    }
  }

  InstTrTy lookupTranslation(const char *mangled) const {
    const char *demangled =
        __cxxabiv1::__cxa_demangle(mangled, nullptr, nullptr, nullptr);

    if (!demangled) {
      // All RS runtime/builtin functions are overloaded, therefore
      // name-mangled.
      return nullptr;
    }

    std::string strDemangled(demangled);

    auto it = mFuncNameMap.find(strDemangled);
    if (it == mFuncNameMap.end()) {
      return nullptr;
    }
    return it->second;
  }

private:
  std::map<std::string, InstTrTy> mFuncNameMap;

  struct sNameCode {
    const char *name;
    uint32_t code;
  };

  static sNameCode constexpr mFPMathFuncOpCode[] = {
      {"abs", 4},        {"sin", 13},   {"cos", 14},   {"tan", 15},
      {"asin", 16},      {"acos", 17},  {"atan", 18},  {"sinh", 19},
      {"cosh", 20},      {"tanh", 21},  {"asinh", 22}, {"acosh", 23},
      {"atanh", 24},     {"atan2", 25}, {"pow", 26},   {"exp", 27},
      {"log", 28},       {"exp2", 29},  {"log2", 30},  {"sqrt", 31},
      {"modf", 35},      {"min", 37},   {"max", 40},   {"length", 66},
      {"normalize", 69}, {nullptr, 0},
  };

}; // BuiltinLookupTable

BuiltinLookupTable::sNameCode constexpr BuiltinLookupTable::mFPMathFuncOpCode[];

class BuiltinTransformer : public Transformer {
public:
  // BEGIN: cleanup unrelated to builtin functions, but necessary for LLVM-SPIRV
  // converter generated code.

  // TODO: Move these in its own pass

  std::vector<uint32_t> runAndSerialize(Module *module, int *error) override {
    module->addExtInstImport("GLSL.std.450");
    return Transformer::runAndSerialize(module, error);
  }

  Instruction *transform(CapabilityInst *inst) override {
    // Remove capabilities Address, Linkage, and Kernel.
    if (inst->mOperand1 == Capability::Addresses ||
        inst->mOperand1 == Capability::Linkage ||
        inst->mOperand1 == Capability::Kernel) {
      return nullptr;
    }
    return inst;
  }

  Instruction *transform(ExtInstImportInst *inst) override {
    if (inst->mOperand1.compare("OpenCL.std") == 0) {
      return nullptr;
    }
    return inst;
  }

  Instruction *transform(InBoundsPtrAccessChainInst *inst) override {
    // Transform any OpInBoundsPtrAccessChain instruction to an
    // OpInBoundsAccessChain instruction, since the former is not allowed by
    // the Vulkan validation rules.
    auto newInst = mBuilder.MakeInBoundsAccessChain(inst->mResultType,
                                                    inst->mOperand1,
                                                    inst->mOperand3);
    newInst->setId(inst->getId());
    return newInst;
  }

  Instruction *transform(SourceInst *inst) override {
    if (inst->mOperand1 == SourceLanguage::Unknown) {
      return nullptr;
    }
    return inst;
  }

  Instruction *transform(DecorateInst *inst) override {
    if (inst->mOperand2 == Decoration::LinkageAttributes ||
        inst->mOperand2 == Decoration::Alignment) {
      return nullptr;
    }
    return inst;
  }

  // END: cleanup unrelated to builtin functions

  Instruction *transform(FunctionCallInst *call) {
    FunctionInst *func =
        static_cast<FunctionInst *>(call->mOperand1.mInstruction);
    // TODO: attach name to the instruction to avoid linear search in the debug
    // section, i.e.,
    // const char *name = func->getName();
    const char *name = getModule()->lookupNameByInstruction(func);
    if (!name) {
      return call;
    }

    // Maps name into a SPIR-V instruction
    auto fpTranslate =
        BuiltinLookupTable::getInstance().lookupTranslation(name);
    if (!fpTranslate) {
      return call;
    }
    Instruction *inst = fpTranslate(name, call, this, &mBuilder, getModule());

    if (inst) {
      inst->setId(call->getId());
    }

    return inst;
  }

private:
  Builder mBuilder;
};

} // namespace spirit
} // namespace android

namespace rs2spirv {

android::spirit::Pass *CreateBuiltinPass() {
  return new android::spirit::BuiltinTransformer();
}

} // namespace rs2spirv

