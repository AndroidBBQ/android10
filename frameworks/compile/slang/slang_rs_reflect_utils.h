/*
 * Copyright 2010, The Android Open Source Project
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

#ifndef _FRAMEWORKS_COMPILE_SLANG_SLANG_REFLECT_UTILS_H_ // NOLINT
#define _FRAMEWORKS_COMPILE_SLANG_SLANG_REFLECT_UTILS_H_

#include <fstream>
#include <string>

namespace slang {

// BitCode storage type
enum BitCodeStorageType { BCST_APK_RESOURCE, BCST_JAVA_CODE, BCST_CPP_CODE };

class RSSlangReflectUtils {
public:
  // Encode a binary bitcode file into a Java source file.
  // rsFileName: the original .rs file name (with or without path).
  // bc32FileName: path of the 32-bit bitcode file
  // bc64FileName: path of the 64-bit bitcode file
  // reflectPath: where to output the generated Java file, no package name in
  // it.
  // packageName: the package of the output Java file.
  // verbose: whether or not to print out additional info about compilation.
  // bcStorage: where to emit bitcode to (resource file or embedded).
  struct BitCodeAccessorContext {
    const char *rsFileName;
    const char *bc32FileName;
    const char *bc64FileName;
    const char *reflectPath;
    const char *packageName;
    const std::string *licenseNote;
    bool verbose;
    BitCodeStorageType bcStorage;
  };

  // Return the stem of the file name, i.e., remove the dir and the extension.
  // Eg, foo.ext -> foo
  //     foo.bar.ext -> foo.bar
  //     ./path/foo.ext -> foo
  static std::string GetFileNameStem(const char *fileName);

  // Compute a Java source file path from a given prefixPath and its package.
  // Eg, given prefixPath=./foo/bar and packageName=com.x.y, then it returns
  // ./foo/bar/com/x/y
  static std::string ComputePackagedPath(const char *prefixPath,
                                         const char *packageName);

  // Compute Java class name from a .rs file name.
  // Any non-alnum, non-underscore characters will be discarded.
  // E.g. with rsFileName=./foo/bar/my-Renderscript_file.rs it returns
  // "myRenderscript_file".
  // rsFileName: the input .rs file name (with or without path).
  static std::string JavaClassNameFromRSFileName(const char *rsFileName);

  // Compute a bitcode file name (no extension) from a .rs file name.
  // Because the bitcode file name may be used as Resource ID in the generated
  // class (something like R.raw.<bitcode_filename>), Any non-alnum,
  // non-underscore character will be discarded.
  // The difference from JavaClassNameFromRSFileName() is that the result is
  // converted to lowercase.
  // E.g. with rsFileName=./foo/bar/my-Renderscript_file.rs it returns
  // "myrenderscript_file"
  // rsFileName: the input .rs file name (with or without path).
  static std::string BCFileNameFromRSFileName(const char *rsFileName);

  // Compute the bitcode-containing class name from a .rs filename.
  // Any non-alnum, non-underscore characters will be discarded.
  // E.g. with rsFileName=./foo/bar/my-Renderscript_file.rs it returns
  // "myRenderscript_fileBitCode".
  // rsFileName: the input .rs file name (with or without path).
  static std::string JavaBitcodeClassNameFromRSFileName(const char *rsFileName);

  // Generate the bit code accessor Java source file.
  static bool GenerateJavaBitCodeAccessor(const BitCodeAccessorContext &context);
};

// Joins two sections of a path, inserting a separator if needed.
// E.g. JoinPath("foo/bar", "baz/a.java") returns "foo/bar/baz/a.java",
// JoinPath("foo", "/bar/baz") returns "foo/bar/baz", and
// JoinPath("foo/", "/bar") returns "foo/bar".
std::string JoinPath(const std::string &path1, const std::string &path2);

/* Compute a safe root name from a .rs file name.  Any non-alphanumeric,
 * non-underscore characters will be discarded.
 * E.g. RootNameFromRSFileName("./foo/bar/my-Renderscript_file.rs") returns
 * "myRenderscript_file".
 */
std::string RootNameFromRSFileName(const std::string &rsFileName);

/* This class is used to generate one source file.  There will be one instance
 * for each generated file.
 */
class GeneratedFile : public std::ofstream {
public:
  /* Starts the file by:
   * - creating the parent directories (if needed),
   * - opening the stream,
   * - writing out the license,
   * - writing a message that this file has been auto-generated.
   * If optionalLicense is nullptr, a default license is used.
   */
  bool startFile(const std::string &outPath, const std::string &outFileName,
                 const std::string &sourceFileName,
                 const std::string *optionalLicense, bool isJava, bool verbose);
  void closeFile();

  void increaseIndent(); // Increases the new line indentation by 4.
  void decreaseIndent(); // Decreases the new line indentation by 4.
  void comment(const std::string& s); // Outputs a multiline comment.

  // Starts a control block.  This works both for Java and C++.
  void startBlock() {
    *this << " {\n";
    increaseIndent();
  }

  // Ends a control block.
  void endBlock(bool addSemicolon = false) {
    decreaseIndent();
    indent() << "}" << (addSemicolon ? ";" : "") << "\n\n";
  }

  /* Indents the line.  By returning *this, we can use like this:
   *  mOut.ident() << "a = b;\n";
   */
  std::ofstream &indent() {
    *this << mIndent;
    return *this;
  }

private:
  std::string mIndent; // The correct spacing at the beginning of each line.
};

} // namespace slang

#endif // _FRAMEWORKS_COMPILE_SLANG_SLANG_REFLECT_UTILS_H_  NOLINT
