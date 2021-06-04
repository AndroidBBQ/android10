//===- BitReader_2_7.h - Internal BitcodeReader 2.7 impl --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header defines the BitcodeReader class.
//
//===----------------------------------------------------------------------===//

#ifndef BITREADER_2_7_H
#define BITREADER_2_7_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/Bitcode/BitstreamReader.h"
#include "llvm/Bitcode/LLVMBitCodes.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/GVMaterializer.h"
#include "llvm/IR/OperandTraits.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/Support/ErrorOr.h"
#include <string>

namespace llvm {
  class LLVMContext;
  class MemoryBuffer;
  class MemoryBufferRef;
  class Module;
} // End llvm namespace

namespace llvm_2_7 {

using llvm::DiagnosticHandlerFunction;
using llvm::LLVMContext;
using llvm::MemoryBuffer;
using llvm::MemoryBufferRef;


  /// Read the header of the specified bitcode buffer and prepare for lazy
  /// deserialization of function bodies.  If successful, this moves Buffer. On
  /// error, this *does not* move Buffer.
  llvm::ErrorOr<llvm::Module *>
  getLazyBitcodeModule(std::unique_ptr<MemoryBuffer> &&Buffer,
                       LLVMContext &Context,
                       const DiagnosticHandlerFunction &DiagnosticHandler = nullptr);

  /// Read the header of the specified bitcode buffer and extract just the
  /// triple information. If successful, this returns a string. On error, this
  /// returns "".
  std::string
  getBitcodeTargetTriple(MemoryBufferRef Buffer, LLVMContext &Context,
                         DiagnosticHandlerFunction DiagnosticHandler = nullptr);

  /// Read the specified bitcode file, returning the module.
  llvm::ErrorOr<llvm::Module *>
  parseBitcodeFile(MemoryBufferRef Buffer, LLVMContext &Context,
                   const DiagnosticHandlerFunction &DiagnosticHandler = nullptr);
} // End llvm_2_7 namespace

#endif
