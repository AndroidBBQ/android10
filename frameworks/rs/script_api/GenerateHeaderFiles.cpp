/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <sstream>

#include "Generator.h"
#include "Specification.h"
#include "Utilities.h"

using namespace std;

// Convert a file name into a string that can be used to guard the include file with #ifdef...
static string makeGuardString(const string& filename) {
    string s;
    s.resize(15 + filename.size());
    s = "RENDERSCRIPT_";
    for (char c : filename) {
        if (c == '.') {
            s += '_';
        } else {
            s += toupper(c);
        }
    }
    return s;
}

/* Write #ifdef's that ensure that the specified version is present.  If we're at the final version,
 * add a check on a flag that can be set for internal builds.  This enables us to keep supporting
 * old APIs in the runtime code.
 */
static void writeVersionGuardStart(GeneratedFile* file, VersionInfo info, unsigned int finalVersion) {
    if (info.intSize == 32) {
        *file << "#ifndef __LP64__\n";
    } else if (info.intSize == 64) {
        *file << "#ifdef __LP64__\n";
    }

    ostringstream checkMaxVersion;
    if (info.maxVersion > 0) {
        checkMaxVersion << "(";
        if (info.maxVersion == finalVersion) {
            checkMaxVersion << "defined(RS_DECLARE_EXPIRED_APIS) || ";
        }
        checkMaxVersion << "RS_VERSION <= " << info.maxVersion << ")";
    }

    if (info.minVersion <= 1) {
        // No minimum
        if (info.maxVersion > 0) {
            *file << "#if !defined(RS_VERSION) || " << checkMaxVersion.str() << "\n";
        }
    } else {
        *file << "#if (defined(RS_VERSION) && (RS_VERSION >= " << info.minVersion << ")";
        if (info.maxVersion > 0) {
            *file << " && " << checkMaxVersion.str();
        }
        *file << ")\n";
    }
}

static void writeVersionGuardEnd(GeneratedFile* file, VersionInfo info) {
    if (info.minVersion > 1 || info.maxVersion != 0) {
        *file << "#endif\n";
    }
    if (info.intSize != 0) {
        *file << "#endif\n";
    }
}

static void writeComment(GeneratedFile* file, const string& name, const string& briefComment,
                         const vector<string>& comment, bool addDeprecatedWarning,
                         bool closeBlock) {
    if (briefComment.empty() && comment.size() == 0) {
        return;
    }
    *file << "/*\n";
    if (!briefComment.empty()) {
        *file << " * " << name << ": " << briefComment << "\n";
        *file << " *\n";
    }
    if (addDeprecatedWarning) {
        *file << " * DEPRECATED.  Do not use.\n";
        *file << " *\n";
    }
    for (size_t ct = 0; ct < comment.size(); ct++) {
        string s = stripHtml(comment[ct]);
        s = stringReplace(s, "@", "");
        if (!s.empty()) {
            *file << " * " << s << "\n";
        } else {
            *file << " *\n";
        }
    }
    if (closeBlock) {
        *file << " */\n";
    }
}

static void writeConstantComment(GeneratedFile* file, const Constant& constant) {
    const string name = constant.getName();
    writeComment(file, name, constant.getSummary(), constant.getDescription(),
                 constant.deprecated(), true);
}

static void writeConstantSpecification(GeneratedFile* file, const ConstantSpecification& spec) {
    const Constant* constant = spec.getConstant();
    VersionInfo info = spec.getVersionInfo();
    writeVersionGuardStart(file, info, constant->getFinalVersion());
    *file << "static const " << spec.getType() << " " << constant->getName()
          << " = " << spec.getValue() << ";\n\n";
    writeVersionGuardEnd(file, info);
}

static void writeTypeSpecification(GeneratedFile* file, const TypeSpecification& spec) {
    const Type* type = spec.getType();
    const string& typeName = type->getName();
    const VersionInfo info = spec.getVersionInfo();
    writeVersionGuardStart(file, info, type->getFinalVersion());

    const string attribute =
                makeAttributeTag(spec.getAttribute(), "", type->getDeprecatedApiLevel(),
                                 type->getDeprecatedMessage());
    *file << "typedef ";
    switch (spec.getKind()) {
        case SIMPLE:
            *file << spec.getSimpleType() << attribute;
            break;
        case RS_OBJECT:
            *file << "struct " << typeName << " _RS_OBJECT_DECL" << attribute;
            break;
        case ENUM: {
            *file << "enum" << attribute << " ";
            const string name = spec.getEnumName();
            if (!name.empty()) {
                *file << name << " ";
            }
            *file << "{\n";

            const vector<string>& values = spec.getValues();
            const vector<string>& valueComments = spec.getValueComments();
            const size_t last = values.size() - 1;
            for (size_t i = 0; i <= last; i++) {
                *file << "    " << values[i];
                if (i != last) {
                    *file << ",";
                }
                if (valueComments.size() > i && !valueComments[i].empty()) {
                    *file << " // " << valueComments[i];
                }
                *file << "\n";
            }
            *file << "}";
            break;
        }
        case STRUCT: {
            *file << "struct" << attribute << " ";
            const string name = spec.getStructName();
            if (!name.empty()) {
                *file << name << " ";
            }
            *file << "{\n";

            const vector<string>& fields = spec.getFields();
            const vector<string>& fieldComments = spec.getFieldComments();
            for (size_t i = 0; i < fields.size(); i++) {
                *file << "    " << fields[i] << ";";
                if (fieldComments.size() > i && !fieldComments[i].empty()) {
                    *file << " // " << fieldComments[i];
                }
                *file << "\n";
            }
            *file << "}";
            break;
        }
    }
    *file << " " << typeName << ";\n";

    writeVersionGuardEnd(file, info);
    *file << "\n";
}

static void writeTypeComment(GeneratedFile* file, const Type& type) {
    const string name = type.getName();
    writeComment(file, name, type.getSummary(), type.getDescription(), type.deprecated(), true);
}

static void writeFunctionPermutation(GeneratedFile* file, const FunctionSpecification& spec,
                                     const FunctionPermutation& permutation) {
    Function* function = spec.getFunction();
    writeVersionGuardStart(file, spec.getVersionInfo(), function->getFinalVersion());

    // Write linkage info.
    const auto inlineCodeLines = permutation.getInline();
    if (inlineCodeLines.size() > 0) {
        *file << "static inline ";
    } else {
        *file << "extern ";
    }

    // Write the return type.
    auto ret = permutation.getReturn();
    if (ret) {
        *file << ret->rsType;
    } else {
        *file << "void";
    }

    *file << makeAttributeTag(spec.getAttribute(), spec.isOverloadable() ? "overloadable" : "",
                              function->getDeprecatedApiLevel(), function->getDeprecatedMessage());
    *file << "\n";

    // Write the function name.
    *file << "    " << permutation.getName() << "(";
    const int offset = 4 + permutation.getName().size() + 1;  // Size of above

    // Write the arguments.  We wrap on mulitple lines if a line gets too long.
    int charsOnLine = offset;
    bool hasGenerated = false;
    for (auto p : permutation.getParams()) {
        if (hasGenerated) {
            *file << ",";
            charsOnLine++;
        }
        ostringstream ps;
        ps << p->rsType;
        if (p->isOutParameter) {
            ps << "*";
        }
        if (!p->specName.empty() && p->rsType != "...") {
            ps << " " << p->specName;
        }
        const string s = ps.str();
        if (charsOnLine + s.size() >= 100) {
            *file << "\n" << string(offset, ' ');
            charsOnLine = offset;
        } else if (hasGenerated) {
            *file << " ";
            charsOnLine++;
        }
        *file << s;
        charsOnLine += s.size();
        hasGenerated = true;
    }
    // In C, if no parameters, we need to output void, e.g. fn(void).
    if (!hasGenerated) {
        *file << "void";
    }
    *file << ")";

    // Write the inline code, if any.
    if (inlineCodeLines.size() > 0) {
        *file << " {\n";
        for (size_t ct = 0; ct < inlineCodeLines.size(); ct++) {
            if (inlineCodeLines[ct].empty()) {
                *file << "\n";
            } else {
                *file << "    " << inlineCodeLines[ct] << "\n";
            }
        }
        *file << "}\n";
    } else {
        *file << ";\n";
    }

    writeVersionGuardEnd(file, spec.getVersionInfo());
    *file << "\n";
}

static void writeFunctionComment(GeneratedFile* file, const Function& function) {
    // Write the generic documentation.
    writeComment(file, function.getName(), function.getSummary(), function.getDescription(),
                 function.deprecated(), false);

    // Comment the parameters.
    if (function.someParametersAreDocumented()) {
        *file << " *\n";
        *file << " * Parameters:\n";
        for (auto p : function.getParameters()) {
            if (!p->documentation.empty()) {
                *file << " *   " << p->name << ": " << p->documentation << "\n";
            }
        }
    }

    // Comment the return type.
    const string returnDoc = function.getReturnDocumentation();
    if (!returnDoc.empty()) {
        *file << " *\n";
        *file << " * Returns: " << returnDoc << "\n";
    }

    *file << " */\n";
}

static void writeFunctionSpecification(GeneratedFile* file, const FunctionSpecification& spec) {
    // Write all the variants.
    for (auto permutation : spec.getPermutations()) {
        writeFunctionPermutation(file, spec, *permutation);
    }
}

static bool writeHeaderFile(const string& directory, const SpecFile& specFile) {
    const string headerFileName = specFile.getHeaderFileName();

    // We generate one header file for each spec file.
    GeneratedFile file;
    if (!file.start(directory, headerFileName)) {
        return false;
    }

    // Write the comments that start the file.
    file.writeNotices();
    writeComment(&file, headerFileName, specFile.getBriefDescription(),
                 specFile.getFullDescription(), false, true);
    file << "\n";

    // Write the ifndef that prevents the file from being included twice.
    const string guard = makeGuardString(headerFileName);
    file << "#ifndef " << guard << "\n";
    file << "#define " << guard << "\n\n";

    // Add lines that need to be put in "as is".
    if (specFile.getVerbatimInclude().size() > 0) {
        for (auto s : specFile.getVerbatimInclude()) {
            file << s << "\n";
        }
        file << "\n";
    }

    /* Write the constants, types, and functions in the same order as
     * encountered in the spec file.
     */
    set<Constant*> documentedConstants;
    for (auto spec : specFile.getConstantSpecifications()) {
        Constant* constant = spec->getConstant();
        if (documentedConstants.find(constant) == documentedConstants.end()) {
            documentedConstants.insert(constant);
            writeConstantComment(&file, *constant);
        }
        writeConstantSpecification(&file, *spec);
    }
    set<Type*> documentedTypes;
    for (auto spec : specFile.getTypeSpecifications()) {
        Type* type = spec->getType();
        if (documentedTypes.find(type) == documentedTypes.end()) {
            documentedTypes.insert(type);
            writeTypeComment(&file, *type);
        }
        writeTypeSpecification(&file, *spec);
    }

    set<Function*> documentedFunctions;
    for (auto spec : specFile.getFunctionSpecifications()) {
        // Do not include internal APIs in the header files.
        if (spec->isInternal()) {
            continue;
        }
        Function* function = spec->getFunction();
        if (documentedFunctions.find(function) == documentedFunctions.end()) {
            documentedFunctions.insert(function);
            writeFunctionComment(&file, *function);
        }
        writeFunctionSpecification(&file, *spec);
    }

    file << "#endif // " << guard << "\n";
    file.close();
    return true;
}

bool generateHeaderFiles(const string& directory) {
    bool success = true;
    for (auto specFile : systemSpecification.getSpecFiles()) {
        if (!writeHeaderFile(directory, *specFile)) {
            success = false;
        }
    }
    return success;
}
