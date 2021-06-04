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

#include <algorithm>
#include <iostream>
#include <sstream>

#include "Generator.h"
#include "Specification.h"
#include "Utilities.h"

using namespace std;

struct DetailedFunctionEntry {
    VersionInfo info;
    string htmlDeclaration;
};

static const char OVERVIEW_HTML_FILE_NAME[] = "overview.html";
static const char INDEX_HTML_FILE_NAME[] = "index.html";

static void writeHeader(GeneratedFile* file, const string& title,
                        const SpecFile& specFile) {
    // Generate DevSite markups
    *file
        << "<html devsite>\n"
           "<!-- " << AUTO_GENERATED_WARNING << "-->\n"
           "<head>\n"
           "  <title>RenderScript " << title << "</title>\n"
           "  <meta name=\"top_category\" value=\"develop\" />\n"
           "  <meta name=\"subcategory\" value=\"guide\" />\n"
           "  <meta name=\"book_path\" value=\"/guide/_book.yaml\" />\n"
           "  <meta name=\"project_path\" value=\"/guide/_project.yaml\" />\n";
    auto desc = specFile.getFullDescription();
    if (desc.size()) {
        *file << "  <meta name=\"description\" content=\"";
        // Output only the first two lines. Assuming there's no other HTML
        // markups there
        // TODO: escape/remove markups
        for (unsigned int i = 0; i < std::min(desc.size(), 2UL); ++i) {
            if (i) *file << " ";
            *file << desc[i];
        }
        *file << "…\">\n";
    }
    *file << "</head>\n\n"
             "<body>\n\n";
    *file << "<div class='renderscript'>\n";
}

static void writeFooter(GeneratedFile* file) {
    *file << "</div>\n";
    *file << "\n\n</body>\n";
    *file << "</html>\n";
}

// If prefix starts input, copy it to stream and remove it from input.
static void skipPrefix(ostringstream* stream, string* input, const string& prefix) {
    size_t size = prefix.size();
    if (input->compare(0, size, prefix) != 0) {
        return;
    }
    input->erase(0, size);
    *stream << prefix;
}

// Merge b into a.  Returns true if successful
static bool mergeVersionInfo(VersionInfo* a, const VersionInfo& b) {
    if (a->intSize != b.intSize) {
        cerr << "Error.  We don't currently support versions that differ based on int size\n";
        return false;
    }
    if (b.minVersion != 0 && a->maxVersion == b.minVersion - 1) {
        a->maxVersion = b.maxVersion;
    } else if (b.maxVersion != 0 && a->minVersion == b.maxVersion + 1) {
        a->minVersion = b.minVersion;
    } else {
        cerr << "Error.  This code currently assume that all versions are contiguous.  Don't know "
                "how to merge versions (" << a->minVersion << " - " << a->maxVersion << ") and ("
             << b.minVersion << " - " << b.maxVersion << ")\n";
        return false;
    }
    return true;
}

static string getHtmlStringForType(const ParameterDefinition& parameter) {
    string s = parameter.rsType;
    ostringstream stream;
    skipPrefix(&stream, &s, "const ");
    skipPrefix(&stream, &s, "volatile ");
    bool endsWithAsterisk = s.size() > 0 && s[s.size() - 1] == '*';
    if (endsWithAsterisk) {
        s.erase(s.size() - 1, 1);
    }

    string anchor = systemSpecification.getHtmlAnchor(s);
    if (anchor.empty()) {
        // Not a RenderScript specific type.
        return parameter.rsType;
    } else {
        stream << anchor;
    }
    if (endsWithAsterisk) {
        stream << "*";
    }
    return stream.str();
}

static string getDetailedHtmlDeclaration(const FunctionPermutation& permutation) {
    ostringstream stream;
    auto ret = permutation.getReturn();
    if (ret) {
        stream << getHtmlStringForType(*ret);
    } else {
        stream << "void";
    }
    stream << " " << permutation.getName() << "(";
    bool needComma = false;
    for (auto p : permutation.getParams()) {
        if (needComma) {
            stream << ", ";
        }
        stream << getHtmlStringForType(*p);
        if (p->isOutParameter) {
            stream << "*";
        }
        if (!p->specName.empty()) {
            stream << " " << p->specName;
        }
        needComma = true;
    }
    stream << ");\n";
    return stream.str();
}

/* Some functions (like max) have changed implementations but not their
 * declaration.  We need to unify these so that we don't end up with entries
 * like:
 *   char max(char a, char b);  Removed from API level 20
 *   char max(char a, char b);  Added to API level 20
 */
static bool getUnifiedFunctionPrototypes(Function* function,
                                         map<string, DetailedFunctionEntry>* entries) {
    for (auto f : function->getSpecifications()) {
        DetailedFunctionEntry entry;
        entry.info = f->getVersionInfo();
        for (auto p : f->getPermutations()) {
            entry.htmlDeclaration = getDetailedHtmlDeclaration(*p);
            const string s = stripHtml(entry.htmlDeclaration);
            auto i = entries->find(s);
            if (i == entries->end()) {
                entries->insert(pair<string, DetailedFunctionEntry>(s, entry));
            } else {
                if (!mergeVersionInfo(&i->second.info, entry.info)) {
                    return false;
                }
            }
        }
    }
    return true;
}

// Convert words starting with @ into HTML references.  Returns false if error.
static bool convertDocumentationRefences(string* s) {
    bool success = true;
    size_t end = 0;
    for (;;) {
        size_t start = s->find('@', end);
        if (start == string::npos) {
            break;
        }
        // Find the end of the identifier
        end = start;
        char c;
        do {
            c = (*s)[++end];
        } while (isalnum(c) || c == '_');

        const string id = s->substr(start + 1, end - start - 1);
        string anchor = systemSpecification.getHtmlAnchor(id);
        if (anchor.empty()) {
            cerr << "Error:  Can't convert the documentation reference @" << id << "\n";
            success = false;
        }
        s->replace(start, end - start, anchor);
    }
    return success;
}

static bool generateHtmlParagraphs(GeneratedFile* file, const vector<string>& description) {
    bool inParagraph = false;
    for (auto s : description) {
        // Empty lines in the .spec marks paragraphs.
        if (s.empty()) {
            if (inParagraph) {
                *file << "</p>\n";
                inParagraph = false;
            }
        } else {
            if (!inParagraph) {
                *file << "<p> ";
                inParagraph = true;
            }
        }
        if (!convertDocumentationRefences(&s)) {
            return false;
        }
        *file << s << "\n";
    }
    if (inParagraph) {
        *file << "</p>\n";
    }
    return true;
}

static void writeSummaryTableStart(GeneratedFile* file, const string& label, bool labelIsHeading) {
    if (labelIsHeading) {
        *file << "<h2 style='margin-bottom: 0px;'>" << label << "</h2>\n";
    }
    *file << "<table class='jd-sumtable'><tbody>\n";
    if (!labelIsHeading) {
        *file << "  <tr><th colspan='2'>" << label << "</th></tr>\n";
    }
}

static void writeSummaryTableEnd(GeneratedFile* file) {
    *file << "</tbody></table>\n";
}

enum DeprecatedSelector {
    DEPRECATED_ONLY,
    NON_DEPRECATED_ONLY,
    ALL,
};

static void writeSummaryTableEntry(ostream* stream, Definition* definition,
                                   DeprecatedSelector deprecatedSelector) {
    if (definition->hidden()) {
        return;
    }
    const bool deprecated = definition->deprecated();
    if ((deprecatedSelector == DEPRECATED_ONLY && !deprecated) ||
        (deprecatedSelector == NON_DEPRECATED_ONLY && deprecated)) {
        return;
    }

    *stream << "  <tr class='alt-color api apilevel-1'>\n";
    *stream << "    <td class='jd-linkcol'>\n";
    *stream << "      <a href='" << definition->getUrl() << "'>" << definition->getName()
            << "</a>\n";
    *stream << "    </td>\n";
    *stream << "    <td class='jd-descrcol' width='100%'>\n";
    *stream << "      ";
    if (deprecated) {
        *stream << "<b>Deprecated</b>.  ";
    }
    *stream << definition->getSummary() << "\n";
    *stream << "    </td>\n";
    *stream << "  </tr>\n";
}

static void writeSummaryTable(GeneratedFile* file, const ostringstream* entries, const char* name,
                              DeprecatedSelector deprecatedSelector, bool labelAsHeader) {
    string s = entries->str();
    if (!s.empty()) {
        string prefix;
        if (deprecatedSelector == DEPRECATED_ONLY) {
            prefix = "Deprecated ";
        }
        writeSummaryTableStart(file, prefix + name, labelAsHeader);
        *file << s;
        writeSummaryTableEnd(file);
    }
}

static void writeSummaryTables(GeneratedFile* file, const map<string, Constant*>& constants,
                               const map<string, Type*>& types,
                               const map<string, Function*>& functions,
                               DeprecatedSelector deprecatedSelector, bool labelAsHeader) {
    ostringstream constantStream;
    for (auto e : constants) {
        writeSummaryTableEntry(&constantStream, e.second, deprecatedSelector);
    }
    writeSummaryTable(file, &constantStream, "Constants", deprecatedSelector, labelAsHeader);

    ostringstream typeStream;
    for (auto e : types) {
        writeSummaryTableEntry(&typeStream, e.second, deprecatedSelector);
    }
    writeSummaryTable(file, &typeStream, "Types", deprecatedSelector, labelAsHeader);

    ostringstream functionStream;
    for (auto e : functions) {
        writeSummaryTableEntry(&functionStream, e.second, deprecatedSelector);
    }
    writeSummaryTable(file, &functionStream, "Functions", deprecatedSelector, labelAsHeader);
}

static void writeHtmlVersionTag(GeneratedFile* file, VersionInfo info,
                                bool addSpacing) {
    ostringstream stream;
    if (info.intSize == 32) {
        stream << "When compiling for 32 bits. ";
    } else if (info.intSize == 64) {
        stream << "When compiling for 64 bits. ";
    }

    if (info.minVersion > 1 || info.maxVersion) {
        const char* mid =
                    "<a "
                    "href='http://developer.android.com/guide/topics/manifest/"
                    "uses-sdk-element.html#ApiLevels'>API level ";
        if (info.minVersion <= 1) {
            // No minimum
            if (info.maxVersion > 0) {
                stream << "Removed from " << mid << info.maxVersion + 1 << " and higher";
            }
        } else {
            if (info.maxVersion == 0) {
                // No maximum
                stream << "Added in " << mid << info.minVersion;
            } else {
                stream << mid << info.minVersion << " - " << info.maxVersion;
            }
        }
        stream << "</a>";
    }
    string s = stream.str();
    // Remove any trailing whitespace
    while (s.back() == ' ') {
        s.pop_back();
    }
    if (!s.empty()) {
        *file << (addSpacing ? "    " : "") << s << "\n";
    }
}

static void writeDetailedTypeSpecification(GeneratedFile* file, const TypeSpecification* spec) {
    switch (spec->getKind()) {
        case SIMPLE: {
            Type* type = spec->getType();
            *file << "<p>A typedef of: " << spec->getSimpleType()
                  << makeAttributeTag(spec->getAttribute(), "", type->getDeprecatedApiLevel(),
                                      type->getDeprecatedMessage())
                  << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
            writeHtmlVersionTag(file, spec->getVersionInfo(), false);
            *file << "</p>\n";
            break;
        }
        case RS_OBJECT: {
            *file << "<p>";
            writeHtmlVersionTag(file, spec->getVersionInfo(), false);
            *file << "</p>\n";
            break;
        }
        case ENUM: {
            *file << "<p>An enum with the following values:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n";
            writeHtmlVersionTag(file, spec->getVersionInfo(), false);
            *file << "</p>\n";

            *file << "  <table class='jd-tagtable'><tbody>\n";
            const vector<string>& values = spec->getValues();
            const vector<string>& valueComments = spec->getValueComments();
            for (size_t i = 0; i < values.size(); i++) {
                *file << "    <tr><th>" << values[i] << "</th><td>";
                if (valueComments.size() > i) {
                    *file << valueComments[i];
                }
                *file << "</td></tr>\n";
            }
            *file << "  </tbody></table><br/>\n";
            break;
        }
        case STRUCT: {
            *file << "<p>A structure with the following fields:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
            writeHtmlVersionTag(file, spec->getVersionInfo(), false);
            *file << "</p>\n";

            *file << "  <table class='jd-tagtable'><tbody>\n";
            const vector<string>& fields = spec->getFields();
            const vector<string>& fieldComments = spec->getFieldComments();
            for (size_t i = 0; i < fields.size(); i++) {
                *file << "    <tr><th>" << fields[i] << "</th><td>";
                if (fieldComments.size() > i && !fieldComments[i].empty()) {
                    *file << fieldComments[i];
                }
                *file << "</td></tr>\n";
            }
            *file << "  </tbody></table><br/>\n";
            break;
        }
    }
}

static void writeDetailedConstantSpecification(GeneratedFile* file, ConstantSpecification* c) {
    *file << "      <tr><td>";
    *file << "Value: " << c->getValue() << "\n";
    writeHtmlVersionTag(file, c->getVersionInfo(), true);
    *file << "      </td></tr>\n";
    *file << "<br/>\n";
}

static bool writeOverviewForFile(GeneratedFile* file, const SpecFile& specFile) {
    bool success = true;
    *file << "<h2>" << specFile.getBriefDescription() << "</h2>\n";
    if (!generateHtmlParagraphs(file, specFile.getFullDescription())) {
        success = false;
    }

    // Write the summary tables.
    // file << "<h2>Summary</h2>\n";
    writeSummaryTables(file, specFile.getDocumentedConstants(), specFile.getDocumentedTypes(),
                       specFile.getDocumentedFunctions(), NON_DEPRECATED_ONLY, false);

    return success;
}

static bool generateOverview(const string& directory) {
    GeneratedFile file;
    if (!file.start(directory, OVERVIEW_HTML_FILE_NAME)) {
        return false;
    }
    bool success = true;

    // Take the description from the first spec file (rs_core.spec, based on how
    // currently this generator is called)
    writeHeader(&file, "Runtime API Reference",
                *(systemSpecification.getSpecFiles()[0]));

    for (auto specFile : systemSpecification.getSpecFiles()) {
        if (!writeOverviewForFile(&file, *specFile)) {
            success = false;
        }
    }

    writeFooter(&file);
    file.close();
    return success;
}

static bool generateAlphabeticalIndex(const string& directory) {
    GeneratedFile file;
    if (!file.start(directory, INDEX_HTML_FILE_NAME)) {
        return false;
    }
    writeHeader(&file, "Index", SpecFile(""));

    writeSummaryTables(&file, systemSpecification.getConstants(), systemSpecification.getTypes(),
                       systemSpecification.getFunctions(), NON_DEPRECATED_ONLY, true);

    writeSummaryTables(&file, systemSpecification.getConstants(), systemSpecification.getTypes(),
                       systemSpecification.getFunctions(), DEPRECATED_ONLY, true);

    writeFooter(&file);
    file.close();
    return true;
}

static void writeDeprecatedWarning(GeneratedFile* file, Definition* definition) {
    if (definition->deprecated()) {
        *file << "    <p><b>Deprecated.</b>  ";
        string s = definition->getDeprecatedMessage();
        convertDocumentationRefences(&s);
        if (!s.empty()) {
            *file << s;
        } else {
            *file << "Do not use.";
        }
        *file << "</p>\n";
    }
}

static bool writeDetailedConstant(GeneratedFile* file, Constant* constant) {
    if (constant->hidden()) {
        return true;
    }
    const string& name = constant->getName();

    *file << "<a name='android_rs:" << name << "'></a>\n";
    *file << "<div class='jd-details'>\n";
    *file << "  <h4 class='jd-details-title'>\n";
    *file << "    <span class='sympad'>" << name << "</span>\n";
    *file << "    <span class='normal'>: " << constant->getSummary() << "</span>\n";
    *file << "  </h4>\n";

    *file << "  <div class='jd-details-descr'>\n";
    *file << "    <table class='jd-tagtable'><tbody>\n";
    auto specifications = constant->getSpecifications();
    bool addSeparator = specifications.size() > 1;
    for (auto spec : specifications) {
        if (addSeparator) {
            *file << "    <h5 class='jd-tagtitle'>Variant:</h5>\n";
        }
        writeDetailedConstantSpecification(file, spec);
    }
    *file << "    </tbody></table>\n";
    *file << "  </div>\n";

    *file << "    <div class='jd-tagdata jd-tagdescr'>\n";

    writeDeprecatedWarning(file, constant);
    if (!generateHtmlParagraphs(file, constant->getDescription())) {
        return false;
    }
    *file << "    </div>\n";

    *file << "</div>\n";
    *file << "\n";
    return true;
}

static bool writeDetailedType(GeneratedFile* file, Type* type) {
    if (type->hidden()) {
        return true;
    }
    const string& name = type->getName();

    *file << "<a name='android_rs:" << name << "'></a>\n";
    *file << "<div class='jd-details'>\n";
    *file << "  <h4 class='jd-details-title'>\n";
    *file << "    <span class='sympad'>" << name << "</span>\n";
    *file << "    <span class='normal'>: " << type->getSummary() << "</span>\n";
    *file << "  </h4>\n";

    *file << "  <div class='jd-details-descr'>\n";
    for (auto spec : type->getSpecifications()) {
        writeDetailedTypeSpecification(file, spec);
    }

    writeDeprecatedWarning(file, type);
    if (!generateHtmlParagraphs(file, type->getDescription())) {
        return false;
    }

    *file << "  </div>\n";
    *file << "</div>\n";
    *file << "\n";
    return true;
}

static bool writeDetailedFunction(GeneratedFile* file, Function* function) {
    if (function->hidden()) {
        return true;
    }
    const string& name = function->getName();

    *file << "<a name='android_rs:" << name << "'></a>\n";
    *file << "<div class='jd-details'>\n";
    *file << "  <h4 class='jd-details-title'>\n";
    *file << "    <span class='sympad'>" << name << "</span>\n";
    *file << "    <span class='normal'>: " << function->getSummary() << "</span>\n";
    *file << "  </h4>\n";

    *file << "  <div class='jd-details-descr'>\n";
    map<string, DetailedFunctionEntry> entries;
    if (!getUnifiedFunctionPrototypes(function, &entries)) {
        return false;
    }
    *file << "    <table class='jd-tagtable'><tbody>\n";
    for (auto i : entries) {
        *file << "      <tr>\n";
        *file << "        <td>" << i.second.htmlDeclaration << "</td>\n";
        *file << "        <td>";
        writeHtmlVersionTag(file, i.second.info, true);
        *file << "        </td>\n";
        *file << "      </tr>\n";
    }
    *file << "    </tbody></table>\n";
    *file << "  </div>\n";

    if (function->someParametersAreDocumented()) {
        *file << "  <div class='jd-tagdata'>";
        *file << "    <h5 class='jd-tagtitle'>Parameters</h5>\n";
        *file << "    <table class='jd-tagtable'><tbody>\n";
        for (ParameterEntry* p : function->getParameters()) {
            *file << "    <tr><th>" << p->name << "</th><td>" << p->documentation << "</td></tr>\n";
        }
        *file << "    </tbody></table>\n";
        *file << "  </div>\n";
    }

    string ret = function->getReturnDocumentation();
    if (!ret.empty()) {
        *file << "  <div class='jd-tagdata'>";
        *file << "    <h5 class='jd-tagtitle'>Returns</h5>\n";
        *file << "    <table class='jd-tagtable'><tbody>\n";
        *file << "    <tr><td>" << ret << "</td></tr>\n";
        *file << "    </tbody></table>\n";
        *file << "  </div>\n";
    }

    *file << "  <div class='jd-tagdata jd-tagdescr'>\n";
    writeDeprecatedWarning(file, function);
    if (!generateHtmlParagraphs(file, function->getDescription())) {
        return false;
    }
    *file << "  </div>\n";

    *file << "</div>\n";
    *file << "\n";
    return true;
}

static bool writeDetailedDocumentationFile(const string& directory,
                                           const SpecFile& specFile) {
    if (!specFile.hasSpecifications()) {
        // This is true for rs_core.spec
        return true;
    }

    GeneratedFile file;
    const string fileName = stringReplace(specFile.getSpecFileName(), ".spec",
                                          ".html");
    if (!file.start(directory, fileName)) {
        return false;
    }
    bool success = true;

    string title = specFile.getBriefDescription();
    writeHeader(&file, title, specFile);

    file << "<h2>Overview</h2>\n";
    if (!generateHtmlParagraphs(&file, specFile.getFullDescription())) {
        success = false;
    }

    // Write the summary tables.
    file << "<h2>Summary</h2>\n";
    const auto& constants = specFile.getDocumentedConstants();
    const auto& types = specFile.getDocumentedTypes();
    const auto& functions = specFile.getDocumentedFunctions();

    writeSummaryTables(&file, constants, types, functions, NON_DEPRECATED_ONLY, false);
    writeSummaryTables(&file, constants, types, functions, DEPRECATED_ONLY, false);

    // Write the full details of each constant, type, and function.
    if (!constants.empty()) {
        file << "<h2>Constants</h2>\n";
        for (auto i : constants) {
            if (!writeDetailedConstant(&file, i.second)) {
                success = false;
            }
        }
    }
    if (!types.empty()) {
        file << "<h2>Types</h2>\n";
        for (auto i : types) {
            if (!writeDetailedType(&file, i.second)) {
                success = false;
            }
        }
    }
    if (!functions.empty()) {
        file << "<h2>Functions</h2>\n";
        for (auto i : functions) {
            if (!writeDetailedFunction(&file, i.second)) {
                success = false;
            }
        }
    }

    writeFooter(&file);
    file.close();

    if (!success) {
        // If in error, write a final message to make it easier to figure out which file failed.
        cerr << fileName << ": Failed due to errors.\n";
    }
    return success;
}

static void generateSnippet(GeneratedFile* file, const string& fileName, const string& title) {
    const char offset[] = "                  ";
    *file << offset << "<li><a href=\"<?cs var:toroot ?>guide/topics/renderscript/reference/"
          << fileName << "\">\n";
    *file << offset << "  <span class=\"en\">" << title << "</span>\n";
    *file << offset << "</a></li>\n";
}

/* Generate a partial file of links that should be cut & pasted into the proper section of the
 * guide_toc.cs file.
 */
static bool generateAndroidTableOfContentSnippet(const string& directory) {
    GeneratedFile file;
    if (!file.start(directory, "guide_toc.cs")) {
        return false;
    }
    file << "<!-- Copy and paste the following lines into the RenderScript section of\n";
    file << "     platform/frameworks/base/docs/html/guide/guide_toc.cs\n\n";

    const char offset[] = "              ";
    file << offset << "<li class=\"nav-section\">\n";
    file << offset << "  <div class=\"nav-section-header\">\n";
    file << offset << "    <a href=\"<?cs var:toroot ?>guide/topics/renderscript/reference/" <<
            OVERVIEW_HTML_FILE_NAME << "\">\n";
    file << offset << "      <span class=\"en\">Runtime API Reference</span>\n";
    file << offset << "    </a></div>\n";
    file << offset << "  <ul>\n";

    for (auto specFile : systemSpecification.getSpecFiles()) {
        if (specFile->hasSpecifications()) {
            const string fileName = stringReplace(specFile->getSpecFileName(), ".spec", ".html");
            generateSnippet(&file, fileName, specFile->getBriefDescription());
        }
    }
    generateSnippet(&file, INDEX_HTML_FILE_NAME, "Index");

    file << offset << "  </ul>\n";
    file << offset << "</li>\n";

    return true;
}

bool generateDocumentation(const string& directory) {
    bool success = generateOverview(directory) &&
                   generateAlphabeticalIndex(directory) &&
                   generateAndroidTableOfContentSnippet(directory);
    for (auto specFile : systemSpecification.getSpecFiles()) {
        if (!writeDetailedDocumentationFile(directory, *specFile)) {
            success = false;
        }
    }
    return success;
}
