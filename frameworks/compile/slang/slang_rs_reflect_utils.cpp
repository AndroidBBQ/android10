/*
 * Copyright 2010-2014, The Android Open Source Project
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

#include "slang_rs_reflect_utils.h"

#include <cstdio>
#include <cstring>
#include <string>
#include <iomanip>

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

#include "os_sep.h"
#include "slang_assert.h"

namespace slang {

using std::string;

string RSSlangReflectUtils::GetFileNameStem(const char *fileName) {
  const char *dot = fileName + strlen(fileName);
  const char *slash = dot - 1;
  while (slash >= fileName) {
    if (*slash == OS_PATH_SEPARATOR) {
      break;
    }
    if ((*slash == '.') && (*dot == 0)) {
      dot = slash;
    }
    --slash;
  }
  ++slash;
  return string(slash, dot - slash);
}

string RSSlangReflectUtils::ComputePackagedPath(const char *prefixPath,
                                                const char *packageName) {
  string packaged_path(prefixPath);
  if (!packaged_path.empty() &&
      (packaged_path[packaged_path.length() - 1] != OS_PATH_SEPARATOR)) {
    packaged_path += OS_PATH_SEPARATOR_STR;
  }
  size_t s = packaged_path.length();
  packaged_path += packageName;
  while (s < packaged_path.length()) {
    if (packaged_path[s] == '.') {
      packaged_path[s] = OS_PATH_SEPARATOR;
    }
    ++s;
  }
  return packaged_path;
}

static string InternalFileNameConvert(const char *rsFileName, bool toLower) {
  const char *dot = rsFileName + strlen(rsFileName);
  const char *slash = dot - 1;
  while (slash >= rsFileName) {
    if (*slash == OS_PATH_SEPARATOR) {
      break;
    }
    if ((*slash == '.') && (*dot == 0)) {
      dot = slash;
    }
    --slash;
  }
  ++slash;
  char ret[256];
  int i = 0;
  for (; (i < 255) && (slash < dot); ++slash) {
    if (isalnum(*slash) || *slash == '_') {
      if (toLower) {
        ret[i] = tolower(*slash);
      } else {
        ret[i] = *slash;
      }
      ++i;
    }
  }
  ret[i] = 0;
  return string(ret);
}

std::string
RSSlangReflectUtils::JavaClassNameFromRSFileName(const char *rsFileName) {
  return InternalFileNameConvert(rsFileName, false);
}

std::string RootNameFromRSFileName(const std::string &rsFileName) {
  return InternalFileNameConvert(rsFileName.c_str(), false);
}

std::string
RSSlangReflectUtils::BCFileNameFromRSFileName(const char *rsFileName) {
  return InternalFileNameConvert(rsFileName, true);
}

std::string RSSlangReflectUtils::JavaBitcodeClassNameFromRSFileName(
    const char *rsFileName) {
  std::string tmp(InternalFileNameConvert(rsFileName, false));
  return tmp.append("BitCode");
}

static bool GenerateAccessorMethod(
    const RSSlangReflectUtils::BitCodeAccessorContext &context,
    int bitwidth, GeneratedFile &out) {
  // the prototype of the accessor method
  out.indent() << "// return byte array representation of the " << bitwidth
               << "-bit bitcode.\n";
  out.indent() << "public static byte[] getBitCode" << bitwidth << "()";
  out.startBlock();
  out.indent() << "return getBitCode" << bitwidth << "Internal();\n";
  out.endBlock(true);
  return true;
}

// Java method size must not exceed 64k,
// so we have to split the bitcode into multiple segments.
static bool GenerateSegmentMethod(const char *buff, int blen, int bitwidth,
                                  int seg_num, GeneratedFile &out) {
  out.indent() << "private static byte[] getSegment" << bitwidth << "_"
               << seg_num << "()";
  out.startBlock();
  out.indent() << "byte[] data = {";
  out.increaseIndent();

  const int kEntriesPerLine = 16;
  int position = kEntriesPerLine;  // We start with a new line and indent.
  for (int written = 0; written < blen; written++) {
    if (++position >= kEntriesPerLine) {
      out << "\n";
      out.indent();
      position = 0;
    } else {
      out << " ";
    }
    out << std::setw(4) << static_cast<int>(buff[written]) << ",";
  }
  out << "\n";

  out.decreaseIndent();
  out.indent() << "};\n";
  out.indent() << "return data;\n";
  out.endBlock();

  return true;
}

static bool GenerateJavaCodeAccessorMethodForBitwidth(
    const RSSlangReflectUtils::BitCodeAccessorContext &context,
    int bitwidth, GeneratedFile &out) {

  std::string filename(context.bc32FileName);
  if (bitwidth == 64) {
    filename = context.bc64FileName;
  }

  FILE *pfin = fopen(filename.c_str(), "rb");
  if (pfin == nullptr) {
    fprintf(stderr, "Error: could not read file %s\n", filename.c_str());
    return false;
  }

  // start the accessor method
  GenerateAccessorMethod(context, bitwidth, out);

  // output the data
  // make sure the generated function for a segment won't break the Javac
  // size limitation (64K).
  static const int SEG_SIZE = 0x2000;
  char *buff = new char[SEG_SIZE];
  int read_length;
  int seg_num = 0;
  int total_length = 0;
  while ((read_length = fread(buff, 1, SEG_SIZE, pfin)) > 0) {
    GenerateSegmentMethod(buff, read_length, bitwidth, seg_num, out);
    ++seg_num;
    total_length += read_length;
  }
  delete[] buff;
  fclose(pfin);

  // output the internal accessor method
  out.indent() << "private static int bitCode" << bitwidth << "Length = "
               << total_length << ";\n\n";
  out.indent() << "private static byte[] getBitCode" << bitwidth
               << "Internal()";
  out.startBlock();
  out.indent() << "byte[] bc = new byte[bitCode" << bitwidth << "Length];\n";
  out.indent() << "int offset = 0;\n";
  out.indent() << "byte[] seg;\n";
  for (int i = 0; i < seg_num; ++i) {
    out.indent() << "seg = getSegment" << bitwidth << "_" << i << "();\n";
    out.indent() << "System.arraycopy(seg, 0, bc, offset, seg.length);\n";
    out.indent() << "offset += seg.length;\n";
  }
  out.indent() << "return bc;\n";
  out.endBlock();

  return true;
}

static bool GenerateJavaCodeAccessorMethod(
    const RSSlangReflectUtils::BitCodeAccessorContext &context,
    GeneratedFile &out) {
  if (!GenerateJavaCodeAccessorMethodForBitwidth(context, 32, out)) {
    slangAssert(false && "Couldn't generate 32-bit embedded bitcode!");
    return false;
  }
  if (!GenerateJavaCodeAccessorMethodForBitwidth(context, 64, out)) {
    slangAssert(false && "Couldn't generate 64-bit embedded bitcode!");
    return false;
  }

  return true;
}

static bool GenerateAccessorClass(
    const RSSlangReflectUtils::BitCodeAccessorContext &context,
    const char *clazz_name, GeneratedFile &out) {
  // begin the class.
  out << "/**\n";
  out << " * @hide\n";
  out << " */\n";
  out << "public class " << clazz_name;
  out.startBlock();

  bool ret = true;
  switch (context.bcStorage) {
  case BCST_APK_RESOURCE:
    slangAssert(false &&
                "Invalid generation of bitcode accessor with resource");
    break;
  case BCST_JAVA_CODE:
    ret = GenerateJavaCodeAccessorMethod(context, out);
    break;
  default:
    ret = false;
  }

  // end the class.
  out.endBlock();

  return ret;
}

bool RSSlangReflectUtils::GenerateJavaBitCodeAccessor(
    const BitCodeAccessorContext &context) {
  string output_path =
      ComputePackagedPath(context.reflectPath, context.packageName);
  if (std::error_code EC = llvm::sys::fs::create_directories(
          llvm::sys::path::parent_path(output_path))) {
    fprintf(stderr, "Error: could not create dir %s: %s\n",
            output_path.c_str(), EC.message().c_str());
    return false;
  }

  string clazz_name(JavaBitcodeClassNameFromRSFileName(context.rsFileName));
  string filename(clazz_name);
  filename += ".java";

  GeneratedFile out;
  if (!out.startFile(output_path, filename, context.rsFileName,
                     context.licenseNote, true, context.verbose)) {
    return false;
  }

  out << "package " << context.packageName << ";\n\n";

  bool ret = GenerateAccessorClass(context, clazz_name.c_str(), out);

  out.closeFile();
  return ret;
}

std::string JoinPath(const std::string &path1, const std::string &path2) {
  if (path1.empty()) {
    return path2;
  }
  if (path2.empty()) {
    return path1;
  }
  std::string fullPath = path1;
  if (fullPath[fullPath.length() - 1] != OS_PATH_SEPARATOR) {
    fullPath += OS_PATH_SEPARATOR;
  }
  if (path2[0] == OS_PATH_SEPARATOR) {
    fullPath += path2.substr(1, string::npos);
  } else {
    fullPath += path2;
  }
  return fullPath;
}

// Replace all instances of "\" with "\\" in a single string to prevent
// formatting errors.  In Java, this can happen even within comments, as
// Java processes \u before the comments are stripped.  E.g. if the generated
// file in Windows contains the note:
//     /* Do not modify!  Generated from \Users\MyName\MyDir\foo.cs */
// Java will think that \U tells of a Unicode character.
static void SanitizeString(std::string *s) {
  size_t p = 0;
  while ((p = s->find('\\', p)) != std::string::npos) {
    s->replace(p, 1, "\\\\");
    p += 2;
  }
}

static const char *const gApacheLicenseNote =
    "/*\n"
    " * Copyright (C) 2011-2014 The Android Open Source Project\n"
    " *\n"
    " * Licensed under the Apache License, Version 2.0 (the \"License\");\n"
    " * you may not use this file except in compliance with the License.\n"
    " * You may obtain a copy of the License at\n"
    " *\n"
    " *      http://www.apache.org/licenses/LICENSE-2.0\n"
    " *\n"
    " * Unless required by applicable law or agreed to in writing, software\n"
    " * distributed under the License is distributed on an \"AS IS\" BASIS,\n"
    " * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or "
    "implied.\n"
    " * See the License for the specific language governing permissions and\n"
    " * limitations under the License.\n"
    " */\n"
    "\n";

bool GeneratedFile::startFile(const string &outDirectory,
                              const string &outFileName,
                              const string &sourceFileName,
                              const string *optionalLicense, bool isJava,
                              bool verbose) {
  if (verbose) {
    printf("Generating %s\n", outFileName.c_str());
  }

  // Create the parent directories.
  if (!outDirectory.empty()) {
    if (std::error_code EC = llvm::sys::fs::create_directories(
            llvm::sys::path::parent_path(outDirectory))) {
      fprintf(stderr, "Error: %s\n", EC.message().c_str());
      return false;
    }
  }

  std::string FilePath = JoinPath(outDirectory, outFileName);

  // Open the file.
  open(FilePath.c_str());
  if (!good()) {
    fprintf(stderr, "Error: could not write file %s\n", outFileName.c_str());
    return false;
  }

  // Write the license.
  if (optionalLicense != nullptr) {
    *this << *optionalLicense;
  } else {
    *this << gApacheLicenseNote;
  }

  // Write a notice that this is a generated file.
  std::string source(sourceFileName);
  if (isJava) {
    SanitizeString(&source);
  }

  *this << "/*\n"
        << " * This file is auto-generated. DO NOT MODIFY!\n"
        << " * The source Renderscript file: " << source << "\n"
        << " */\n\n";

  return true;
}

void GeneratedFile::closeFile() { close(); }

void GeneratedFile::increaseIndent() { mIndent.append("    "); }

void GeneratedFile::decreaseIndent() {
  slangAssert(!mIndent.empty() && "No indent");
  mIndent.erase(0, 4);
}

void GeneratedFile::comment(const std::string &s) {
  indent() << "/* ";
  // +3 for the " * " starting each line.
  std::size_t indentLength = mIndent.length() + 3;
  std::size_t lengthOfCommentOnLine = 0;
  const std::size_t maxPerLine = 80;
  for (std::size_t start = 0, length = s.length(), nextStart = 0;
       start < length; start = nextStart) {
    std::size_t p = s.find_first_of(" \n", start);
    std::size_t toCopy = 1;
    bool forceBreak = false;
    if (p == std::string::npos) {
      toCopy = length - start;
      nextStart = length;
    } else {
      toCopy = p - start;
      nextStart = p + 1;
      forceBreak = s[p] == '\n';
    }
    if (lengthOfCommentOnLine > 0) {
      if (indentLength + lengthOfCommentOnLine + toCopy >= maxPerLine) {
        *this << "\n";
        indent() << " * ";
        lengthOfCommentOnLine = 0;
      } else {
        *this << " ";
      }
    }

    *this << s.substr(start, toCopy);
    if (forceBreak) {
      lengthOfCommentOnLine = maxPerLine;
    } else {
      lengthOfCommentOnLine += toCopy;
    }
  }
  *this << "\n";
  indent() << " */\n";
}

} // namespace slang
