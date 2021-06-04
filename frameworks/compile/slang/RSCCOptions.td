/*
 * Copyright 2010-2012, The Android Open Source Project
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

//===----------------------------------------------------------------------===//
//
//  This file defines the options accepted by llvm-rs-cc.
//
//===----------------------------------------------------------------------===//

// Include the common option parsing interfaces.
include "llvm/Option/OptParser.td"

//===----------------------------------------------------------------------===//
// Target Options
//===----------------------------------------------------------------------===//

def target_api : Separate<["-"], "target-api">,
  HelpText<"Specify target API level (e.g. 14)">;
def target_api_EQ : Joined<["-"], "target-api=">, Alias<target_api>;

//===----------------------------------------------------------------------===//
// Header Search Options
//===----------------------------------------------------------------------===//

def I : JoinedOrSeparate<["-"], "I">, MetaVarName<"<directory>">,
  HelpText<"Add directory to include search path">;
def _I : Separate<["-", "--"], "include-path">, MetaVarName<"<directory>">, Alias<I>;

//===----------------------------------------------------------------------===//
// Frontend Options
//===----------------------------------------------------------------------===//

def o : Separate<["-"], "o">, MetaVarName<"<directory>">,
  HelpText<"Specify output directory">;

def Output_Type_Group : OptionGroup<"<output type group>">;
let Group = Output_Type_Group in {

def emit_asm : Flag<["-"], "emit-asm">,
  HelpText<"Emit target assembly files">;
def _emit_asm : Flag<["-"], "S">, Alias<emit_asm>;
def emit_llvm : Flag<["-"], "emit-llvm">,
  HelpText<"Build ASTs then convert to LLVM, emit .ll file">;
def emit_bc : Flag<["-"], "emit-bc">,
  HelpText<"Build ASTs then convert to LLVM, emit .bc file">;
def emit_nothing : Flag<["-"], "emit-nothing">,
  HelpText<"Build ASTs then convert to LLVM, but emit nothing">;
}

def m32 : Flag<["-"], "m32">, HelpText<"Emit 32-bit code (only for C++, unless eng build)">;
def m64 : Flag<["-"], "m64">, HelpText<"Emit 64-bit code (only for C++, unless eng build)">;

def emit_g : Flag<["-"], "g">,
  HelpText<"Emit LLVM Debug Metadata">;

def optimization_level : JoinedOrSeparate<["-"], "O">, MetaVarName<"<optimization-level>">,
  HelpText<"<optimization-level> can be one of '0' or '3' (default)">;

def allow_rs_prefix : Flag<["-"], "allow-rs-prefix">,
  HelpText<"Allow user-defined function prefixed with 'rs'">;

def java_reflection_path_base : Separate<["-"], "java-reflection-path-base">,
  MetaVarName<"<directory>">,
  HelpText<"Base directory for output reflected Java files">;
def _java_reflection_path_base : Separate<["-"], "p">,
  Alias<java_reflection_path_base>;
def java_reflection_package_name : Separate<["-"], "java-reflection-package-name">,
  HelpText<"Specify the package name that reflected Java files belong to">;
def _java_reflection_package_name : Separate<["-"], "j">,
  Alias<java_reflection_package_name>;

def bitcode_storage : Separate<["-"], "bitcode-storage">,
  MetaVarName<"<value>">, HelpText<"<value> should be 'ar' or 'jc'">;
def _bitcode_storage : Separate<["-"], "s">, Alias<bitcode_storage>;

def rs_package_name : Separate<["-"], "rs-package-name">,
  MetaVarName<"<package_name>">,
  HelpText<"package name for referencing RS classes">;
def rs_package_name_EQ : Joined<["-"], "rs-package-name=">, Alias<rs_package_name>;

def W : Joined<["-"], "W">, HelpText<"Enable the specified warning">;
def w : Flag<["-"], "w">, HelpText<"Suppress all warnings">;

//===----------------------------------------------------------------------===//
// Dependency Output Options
//===----------------------------------------------------------------------===//

def M_Group : OptionGroup<"M group>">;
let Group = M_Group in {

  def MD : Flag<["-"], "MD">, HelpText<"Emit .d dependency files">;
  def MP : Flag<["-"], "MP">, HelpText<"Also emit phony target for dependency files">;

  def M : Flag<["-"], "M">;
  def emit_dep : Flag<["-"], "emit-dep">, Alias<M>;
}

def output_dep_dir : Separate<["-"], "output-dep-dir">, MetaVarName<"<directory>">,
  HelpText<"Specify output directory for dependencies output">;
def _output_dep_dir : Separate<["-"], "d">, Alias<output_dep_dir>;

def additional_dep_target: Separate<["-"], "additional-dep-target">,
  HelpText<"Additional targets to show up in dependencies output">;
def _additional_dep_target : Separate<["-"], "a">, Alias<additional_dep_target>;

//===----------------------------------------------------------------------===//
// Reflection Options
//===----------------------------------------------------------------------===//

def reflect_cpp : Flag<["-"], "reflect-c++">,
  HelpText<"Reflect C++ classes">;

//===----------------------------------------------------------------------===//
// Diagnostic Options
//===----------------------------------------------------------------------===//

def ast_print : Flag<["-"], "ast-print">,
  HelpText<"Print clang AST prior to llvm IR generation">;

def debug : Flag<["-"], "debug">,
  HelpText<"Enable debug output">;

def print_after_all : Flag<["-"], "print-after-all">,
  HelpText<"Print llvm IR after each pass">;
def print_before_all : Flag<["-"], "print-before-all">,
  HelpText<"Print llvm IR before each pass">;

//===----------------------------------------------------------------------===//
// Misc Options
//===----------------------------------------------------------------------===//

def verbose : Flag<["-"], "v">,
  HelpText<"Display verbose information during the compilation">;
def _verbose : Flag<["-"], "verbose">, Alias<verbose>;
def __verbose : Flag<["--"], "verbose">, Alias<verbose>;

def help : Flag<["-"], "help">,
  HelpText<"Print this help text">;
def _help : Flag<["--"], "help">, Alias<help>;
def __help : Flag<["-"], "h">, Alias<help>;

def version : Flag<["-"], "version">,
  HelpText<"Print the assembler version">;
def _version : Flag<["--"], "version">, Alias<version>;

// Compatible with old slang
def no_link : Flag<["-"], "no-link">;  // currently no effect
