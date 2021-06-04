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

//#define LOG_NDEBUG 0
#define LOG_TAG "MediaCodecsXmlParser"

#include <media/stagefright/xmlparser/MediaCodecsXmlParser.h>

#include <android-base/logging.h>
#include <android-base/macros.h>
#include <utils/Log.h>

#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/omx/OMXUtils.h>

#include <expat.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace android {

using MCXP = MediaCodecsXmlParser;

namespace {

bool fileExists(const std::string &path) {
    struct stat fileStat;
    return stat(path.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode);
}

/**
 * Search for a file in a list of search directories.
 *
 * For each string `searchDir` in `searchDirs`, `searchDir/fileName` will be
 * tested whether it is a valid file name or not. If it is a valid file name,
 * the concatenated name (`searchDir/fileName`) will be stored in the output
 * variable `outPath`, and the function will return `true`. Otherwise, the
 * search continues until the `nullptr` element in `searchDirs` is reached, at
 * which point the function returns `false`.
 *
 * \param[in] searchDirs array of search paths.
 * \param[in] fileName Name of the file to search.
 * \param[out] outPath Full path of the file. `outPath` will hold a valid file
 * name if the return value of this function is `true`.
 * \return `true` if some element in `searchDirs` combined with `fileName` is a
 * valid file name; `false` otherwise.
 */
bool findFileInDirs(
        const std::vector<std::string> &searchDirs,
        const std::string &fileName,
        std::string *outPath) {
    for (const std::string &searchDir : searchDirs) {
        std::string path = searchDir + "/" + fileName;
        if (fileExists(path)) {
            *outPath = path;
            return true;
        }
    }
    return false;
}

bool strnEq(const char* s1, const char* s2, size_t count) {
    return strncmp(s1, s2, count) == 0;
}

bool strEq(const char* s1, const char* s2) {
    return strcmp(s1, s2) == 0;
}

bool striEq(const char* s1, const char* s2) {
    return strcasecmp(s1, s2) == 0;
}

bool strHasPrefix(const char* test, const char* prefix) {
    return strnEq(test, prefix, strlen(prefix));
}

bool parseBoolean(const char* s) {
    return striEq(s, "y") ||
            striEq(s, "yes") ||
            striEq(s, "enabled") ||
            striEq(s, "t") ||
            striEq(s, "true") ||
            striEq(s, "1");
}


status_t combineStatus(status_t a, status_t b) {
    if (a == NO_INIT) {
        return b;
    } else if ((a == OK && (b == NAME_NOT_FOUND || b == ALREADY_EXISTS || b == NO_INIT))
            || (b == OK && (a == NAME_NOT_FOUND || a == ALREADY_EXISTS))) {
        // ignore NAME_NOT_FOUND and ALREADY_EXIST errors as long as the other error is OK
        // also handle OK + NO_INIT here
        return OK;
    } else {
        // prefer the first error result
        return a ? : b;
    }
}

MCXP::StringSet parseCommaSeparatedStringSet(const char *s) {
    MCXP::StringSet result;
    for (const char *ptr = s ? : ""; *ptr; ) {
        const char *end = strchrnul(ptr, ',');
        if (ptr != end) { // skip empty values
            result.emplace(ptr, end - ptr);
        }
        ptr = end + ('\0' != *end);
    }
    return result;
}

#define PLOGD(msg, ...) \
        ALOGD(msg " at line %zu of %s", ##__VA_ARGS__, \
                (size_t)::XML_GetCurrentLineNumber(mParser.get()), mPath.c_str());

}  // unnamed namespace

struct MediaCodecsXmlParser::Impl {
    // status + error message
    struct Result {
    private:
        status_t mStatus;
        std::string mError;

    public:
        Result(status_t s, std::string error = "")
            : mStatus(s),
              mError(error) {
            if (error.empty() && s) {
                mError = "Failed (" + std::string(asString(s)) + ")";
            }
        }
        operator status_t() const { return mStatus; }
        std::string error() const { return mError; }
    };


    // Parsed data
    struct Data {
        // Service attributes
        AttributeMap mServiceAttributeMap;
        CodecMap mCodecMap;
        Result addGlobal(std::string key, std::string value, bool updating);
    };

    enum Section {
        SECTION_TOPLEVEL,
        SECTION_SETTINGS,
        SECTION_DECODERS,
        SECTION_DECODER,
        SECTION_DECODER_TYPE,
        SECTION_ENCODERS,
        SECTION_ENCODER,
        SECTION_ENCODER_TYPE,
        SECTION_INCLUDE,
        SECTION_VARIANT,
        SECTION_UNKNOWN,
    };

    // XML parsing state
    struct State {
    private:
        Data *mData;

        // current codec and/or type, plus whether we are updating
        struct CodecAndType {
            std::string mName;
            CodecMap::iterator mCodec;
            TypeMap::iterator mType;
            bool mUpdating;
        };

        // using vectors as we need to reset their sizes
        std::vector<std::string> mIncludeStack;
        std::vector<Section> mSectionStack;
        std::vector<StringSet> mVariantsStack;
        std::vector<CodecAndType> mCurrent;

    public:
        State(Data *data);

        Data &data() { return *mData; }

        // used to restore parsing state at XML include boundaries, in case parsing the included
        // file fails.
        struct RestorePoint {
            size_t numIncludes;
            size_t numSections;
            size_t numVariantSets;
            size_t numCodecAndTypes;
        };

        // method manipulating restore points (all state stacks)
        RestorePoint createRestorePoint() const {
            return {
                mIncludeStack.size(), mSectionStack.size(), mVariantsStack.size(), mCurrent.size()
            };
        }

        void restore(RestorePoint rp) {
            CHECK_GE(mIncludeStack.size(), rp.numIncludes);
            CHECK_GE(mSectionStack.size(), rp.numSections);
            CHECK_GE(mVariantsStack.size(), rp.numVariantSets);
            CHECK_GE(mCurrent.size(), rp.numCodecAndTypes);

            mIncludeStack.resize(rp.numIncludes);
            mSectionStack.resize(rp.numSections);
            mVariantsStack.resize(rp.numVariantSets);
            mCurrent.resize(rp.numCodecAndTypes);
        }

        // methods manipulating the include stack
        Result enterInclude(const std::string &path);
        void exitInclude() {
            mIncludeStack.pop_back();
        }

        // methods manipulating the codec/type stack/state
        bool inCodec() const {
            return !mCurrent.empty() && mCurrent.back().mCodec != mData->mCodecMap.end();
        }

        bool inType() const {
            return inCodec()
                    && mCurrent.back().mType != mCurrent.back().mCodec->second.typeMap.end();
        }

        Result enterMediaCodec(bool encoder, const char *name, const char *type, bool update);
        Result enterType(const char *name, bool update);
        void exitCodecOrType() {
            mCurrent.pop_back();
        }

        // can only be called when inCodec()
        MediaCodecsXmlParser::CodecProperties &codec() {
            return mCurrent.back().mCodec->second;
        }
        // can only be called when inCodec()
        std::string codecName() const {
            return mCurrent.back().mName;
        }
        // can only be called when inCodec()
        bool updating() const {
            return mCurrent.back().mUpdating;
        }
        // can only be called when inType()
        MediaCodecsXmlParser::AttributeMap &type() {
            return mCurrent.back().mType->second;
        }

        // methods manipulating the section stack
        Section section() const {
            return mSectionStack.back();
        }
        Section lastNonIncludeSection() const;
        void enterSection(Section s) {
            mSectionStack.push_back(s);
        }
        void exitSection() {
            mSectionStack.pop_back();
            CHECK(!mSectionStack.empty());
        }

        // methods manipulating the variants stack
        StringSet variants() const {
            return mVariantsStack.back();
        }
        void enterVariants(StringSet variants) {
            mVariantsStack.push_back(variants);
        }
        void exitVariants() {
            mVariantsStack.pop_back();
        }

        // utility methods

        // updates rank, domains, variants and enabledness on the current codec/type
        Result updateCodec(
                const char *rank, StringSet domains, StringSet variants, const char *enabled);
        // adds a key-value attribute detail to the current type of the current codec
        void addDetail(const std::string &key, const std::string &value);
    };

    /** XML Parser (state) */
    struct Parser {
        State *mState;

        Parser(State *state, std::string path);

        // keep track of the parser state
        std::shared_ptr<XML_ParserStruct> mParser;
        std::string mPath;
        std::string mHrefBase;
        status_t mStatus;

        void parseXmlFile();

        // XML parser callbacks
        static void StartElementHandlerWrapper(void *me, const char *name, const char **attrs);
        static void EndElementHandlerWrapper(void *me, const char *name);

        void startElementHandler(const char *name, const char **attrs);
        void endElementHandler(const char *name);

        void updateStatus(status_t status);
        void logAnyErrors(const Result &status) const;
        status_t getStatus() const { return mStatus; }

        status_t addAlias(const char **attrs);
        status_t addFeature(const char **attrs);
        status_t addLimit(const char **attrs);
        status_t addQuirk(const char **attrs, const char *prefix = nullptr);
        status_t addSetting(const char **attrs, const char *prefix = nullptr);
        status_t enterMediaCodec(const char **attrs, bool encoder);
        status_t enterType(const char **attrs);
        status_t includeXmlFile(const char **attrs);
        status_t limitVariants(const char **attrs);

        status_t updateMediaCodec(
                const char *rank, const StringSet &domain, const StringSet &variants,
                const char *enabled);
    };

    status_t parseXmlFilesInSearchDirs(
        const std::vector<std::string> &fileNames,
        const std::vector<std::string> &searchDirs);

    status_t parseXmlPath(const std::string &path);

    // Computed longest common prefix
    Data mData;
    State mState;

    // Role map
    mutable std::string mCommonPrefix;
    mutable RoleMap mRoleMap;
    mutable std::mutex mLock;

    status_t mParsingStatus;

    Impl()
        : mState(&mData),
          mParsingStatus(NO_INIT) {
    }

    void generateRoleMap() const;
    void generateCommonPrefix() const;

    const AttributeMap& getServiceAttributeMap() const {
        std::lock_guard<std::mutex> guard(mLock);
        return mData.mServiceAttributeMap;
    }

    const CodecMap& getCodecMap() const {
        std::lock_guard<std::mutex> guard(mLock);
        return mData.mCodecMap;
    }

    const RoleMap& getRoleMap() const;
    const char* getCommonPrefix() const;

    status_t getParsingStatus() const {
        std::lock_guard<std::mutex> guard(mLock);
        return mParsingStatus;
    }
};

constexpr char const* MediaCodecsXmlParser::defaultProfilingResultsXmlPath;

MediaCodecsXmlParser::MediaCodecsXmlParser()
    : mImpl(new Impl()) {
}

status_t MediaCodecsXmlParser::parseXmlFilesInSearchDirs(
        const std::vector<std::string> &fileNames,
        const std::vector<std::string> &searchDirs) {
    return mImpl->parseXmlFilesInSearchDirs(fileNames, searchDirs);
}

status_t MediaCodecsXmlParser::parseXmlPath(const std::string &path) {
    return mImpl->parseXmlPath(path);
}

status_t MediaCodecsXmlParser::Impl::parseXmlFilesInSearchDirs(
        const std::vector<std::string> &fileNames,
        const std::vector<std::string> &searchDirs) {
    status_t res = NO_INIT;
    for (const std::string fileName : fileNames) {
        status_t err = NO_INIT;
        std::string path;
        if (findFileInDirs(searchDirs, fileName, &path)) {
            err = parseXmlPath(path);
        } else {
            ALOGD("Cannot find %s", path.c_str());
        }
        res = combineStatus(res, err);
    }
    return res;
}

status_t MediaCodecsXmlParser::Impl::parseXmlPath(const std::string &path) {
    std::lock_guard<std::mutex> guard(mLock);
    if (!fileExists(path)) {
        ALOGD("Cannot find %s", path.c_str());
        mParsingStatus = combineStatus(mParsingStatus, NAME_NOT_FOUND);
        return NAME_NOT_FOUND;
    }

    // save state (even though we should always be at toplevel here)
    State::RestorePoint rp = mState.createRestorePoint();
    Parser parser(&mState, path);
    parser.parseXmlFile();
    mState.restore(rp);

    if (parser.getStatus() != OK) {
        ALOGD("parseXmlPath(%s) failed with %s", path.c_str(), asString(parser.getStatus()));
    }
    mParsingStatus = combineStatus(mParsingStatus, parser.getStatus());
    return parser.getStatus();
}

MediaCodecsXmlParser::~MediaCodecsXmlParser() {
}

MediaCodecsXmlParser::Impl::State::State(MediaCodecsXmlParser::Impl::Data *data)
    : mData(data) {
    mSectionStack.emplace_back(SECTION_TOPLEVEL);
}

MediaCodecsXmlParser::Impl::Section
MediaCodecsXmlParser::Impl::State::lastNonIncludeSection() const {
    for (auto it = mSectionStack.end(); it != mSectionStack.begin(); --it) {
        if (it[-1] != SECTION_INCLUDE) {
            return it[-1];
        }
    }
    TRESPASS("must have non-include section");
}

void MediaCodecsXmlParser::Impl::Parser::updateStatus(status_t status) {
    mStatus = combineStatus(mStatus, status);
}

void MediaCodecsXmlParser::Impl::Parser::logAnyErrors(const Result &status) const {
    if (status) {
        if (status.error().empty()) {
            PLOGD("error %s", asString((status_t)status));
        } else {
            PLOGD("%s", status.error().c_str());
        }
    }
}

MediaCodecsXmlParser::Impl::Parser::Parser(State *state, std::string path)
    : mState(state),
      mPath(path),
      mStatus(NO_INIT) {
    // determine href_base
    std::string::size_type end = path.rfind("/");
    if (end != std::string::npos) {
        mHrefBase = path.substr(0, end + 1);
    }
}

void MediaCodecsXmlParser::Impl::Parser::parseXmlFile() {
    const char *path = mPath.c_str();
    ALOGD("parsing %s...", path);
    FILE *file = fopen(path, "r");

    if (file == nullptr) {
        ALOGD("unable to open media codecs configuration xml file: %s", path);
        mStatus = NAME_NOT_FOUND;
        return;
    }

    mParser = std::shared_ptr<XML_ParserStruct>(
        ::XML_ParserCreate(nullptr),
        [](XML_ParserStruct *parser) { ::XML_ParserFree(parser); });
    LOG_FATAL_IF(!mParser, "XML_MediaCodecsXmlParserCreate() failed.");

    ::XML_SetUserData(mParser.get(), this);
    ::XML_SetElementHandler(mParser.get(), StartElementHandlerWrapper, EndElementHandlerWrapper);

    static constexpr int BUFF_SIZE = 512;
    // updateStatus(OK);
    if (mStatus == NO_INIT) {
        mStatus = OK;
    }
    while (mStatus == OK) {
        void *buff = ::XML_GetBuffer(mParser.get(), BUFF_SIZE);
        if (buff == nullptr) {
            ALOGD("failed in call to XML_GetBuffer()");
            mStatus = UNKNOWN_ERROR;
            break;
        }

        int bytes_read = ::fread(buff, 1, BUFF_SIZE, file);
        if (bytes_read < 0) {
            ALOGD("failed in call to read");
            mStatus = ERROR_IO;
            break;
        }

        XML_Status status = ::XML_ParseBuffer(mParser.get(), bytes_read, bytes_read == 0);
        if (status != XML_STATUS_OK) {
            PLOGD("malformed (%s)", ::XML_ErrorString(::XML_GetErrorCode(mParser.get())));
            mStatus = ERROR_MALFORMED;
            break;
        }

        if (bytes_read == 0) {
            break;
        }
    }

    mParser.reset();

    fclose(file);
    file = nullptr;
}

// static
void MediaCodecsXmlParser::Impl::Parser::StartElementHandlerWrapper(
        void *me, const char *name, const char **attrs) {
    static_cast<MediaCodecsXmlParser::Impl::Parser*>(me)->startElementHandler(name, attrs);
}

// static
void MediaCodecsXmlParser::Impl::Parser::EndElementHandlerWrapper(void *me, const char *name) {
    static_cast<MediaCodecsXmlParser::Impl::Parser*>(me)->endElementHandler(name);
}

status_t MediaCodecsXmlParser::Impl::Parser::includeXmlFile(const char **attrs) {
    const char *href = nullptr;
    size_t i = 0;
    while (attrs[i] != nullptr) {
        CHECK((i & 1) == 0);
        if (attrs[i + 1] == nullptr) {
            PLOGD("Include: attribute '%s' is null", attrs[i]);
            return BAD_VALUE;
        }

        if (strEq(attrs[i], "href")) {
            href = attrs[++i];
        } else {
            PLOGD("Include: ignoring unrecognized attribute '%s'", attrs[i]);
            ++i;
        }
        ++i;
    }

    if (href == nullptr) {
        PLOGD("Include with no 'href' attribute");
        return BAD_VALUE;
    }

    // For security reasons and for simplicity, file names can only contain
    // [a-zA-Z0-9_.] and must start with  media_codecs_ and end with .xml
    for (i = 0; href[i] != '\0'; i++) {
        if (href[i] == '.' || href[i] == '_' ||
                (href[i] >= '0' && href[i] <= '9') ||
                (href[i] >= 'A' && href[i] <= 'Z') ||
                (href[i] >= 'a' && href[i] <= 'z')) {
            continue;
        }
        PLOGD("invalid include file name: %s", href);
        return BAD_VALUE;
    }

    std::string filename = href;
    if (filename.compare(0, 13, "media_codecs_") != 0 ||
            filename.compare(filename.size() - 4, 4, ".xml") != 0) {
        PLOGD("invalid include file name: %s", href);
        return BAD_VALUE;
    }
    filename.insert(0, mHrefBase);

    Result res = mState->enterInclude(filename);
    if (res) {
        logAnyErrors(res);
        return res;
    }

    // save state so that we can resume even if XML parsing of the included file failed midway
    State::RestorePoint rp = mState->createRestorePoint();
    Parser parser(mState, filename);
    parser.parseXmlFile();
    mState->restore(rp);
    mState->exitInclude();
    return parser.getStatus();
}

MediaCodecsXmlParser::Impl::Result
MediaCodecsXmlParser::Impl::State::enterInclude(const std::string &fileName) {
    if (std::find(mIncludeStack.begin(), mIncludeStack.end(), fileName)
            != mIncludeStack.end()) {
        return { BAD_VALUE, "recursive include chain" };
    }
    mIncludeStack.emplace_back(fileName);
    return OK;
}

void MediaCodecsXmlParser::Impl::Parser::startElementHandler(
        const char *name, const char **attrs) {
    bool inType = true;
    Result err = NO_INIT;

    Section section = mState->section();

    // handle include at any level
    if (strEq(name, "Include")) {
        mState->enterSection(SECTION_INCLUDE);
        updateStatus(includeXmlFile(attrs));
        return;
    }

    // handle include section (top level)
    if (section == SECTION_INCLUDE) {
        if (strEq(name, "Included")) {
            return;
        }
        // imitate prior level
        section = mState->lastNonIncludeSection();
    }

    switch (section) {
        case SECTION_TOPLEVEL:
        {
            Section nextSection;
            if (strEq(name, "Decoders")) {
                nextSection = SECTION_DECODERS;
            } else if (strEq(name, "Encoders")) {
                nextSection = SECTION_ENCODERS;
            } else if (strEq(name, "Settings")) {
                nextSection = SECTION_SETTINGS;
            } else if (strEq(name, "MediaCodecs") || strEq(name, "Included")) {
                return;
            } else {
                break;
            }
            mState->enterSection(nextSection);
            return;
        }

        case SECTION_SETTINGS:
        {
            if (strEq(name, "Setting")) {
                err = addSetting(attrs);
            } else if (strEq(name, "Variant")) {
                err = addSetting(attrs, "variant-");
            } else if (strEq(name, "Domain")) {
                err = addSetting(attrs, "domain-");
            } else {
                break;
            }
            updateStatus(err);
            return;
        }

        case SECTION_DECODERS:
        case SECTION_ENCODERS:
        {
            if (strEq(name, "MediaCodec")) {
                err = enterMediaCodec(attrs, section == SECTION_ENCODERS);
                updateStatus(err);
                if (err != OK) { // skip this element on error
                    mState->enterSection(SECTION_UNKNOWN);
                } else {
                    mState->enterVariants(mState->codec().variantSet);
                    mState->enterSection(
                            section == SECTION_DECODERS ? SECTION_DECODER : SECTION_ENCODER);
                }
                return;
            }
            break;
        }

        case SECTION_DECODER:
        case SECTION_ENCODER:
        {
            if (strEq(name, "Quirk")) {
                err = addQuirk(attrs, "quirk::");
            } else if (strEq(name, "Attribute")) {
                err = addQuirk(attrs, "attribute::");
            } else if (strEq(name, "Alias")) {
                err = addAlias(attrs);
            } else if (strEq(name, "Type")) {
                err = enterType(attrs);
                if (err != OK) { // skip this element on error
                    mState->enterSection(SECTION_UNKNOWN);
                } else {
                    mState->enterSection(
                            section == SECTION_DECODER
                                    ? SECTION_DECODER_TYPE : SECTION_ENCODER_TYPE);
                }
            }
        }
        inType = false;
        FALLTHROUGH_INTENDED;

        case SECTION_DECODER_TYPE:
        case SECTION_ENCODER_TYPE:
        case SECTION_VARIANT:
        {
            // ignore limits and features specified outside of type
            if (!mState->inType()
                    && (strEq(name, "Limit") || strEq(name, "Feature") || strEq(name, "Variant"))) {
                PLOGD("ignoring %s specified outside of a Type", name);
                return;
            } else if (strEq(name, "Limit")) {
                err = addLimit(attrs);
            } else if (strEq(name, "Feature")) {
                err = addFeature(attrs);
            } else if (strEq(name, "Variant") && section != SECTION_VARIANT) {
                err = limitVariants(attrs);
                mState->enterSection(err == OK ? SECTION_VARIANT : SECTION_UNKNOWN);
            } else if (inType
                    && (strEq(name, "Alias") || strEq(name, "Attribute") || strEq(name, "Quirk"))) {
                PLOGD("ignoring %s specified not directly in a MediaCodec", name);
                return;
            } else if (err == NO_INIT) {
                break;
            }
            updateStatus(err);
            return;
        }

        default:
            break;
    }

    if (section != SECTION_UNKNOWN) {
        PLOGD("Ignoring unrecognized tag <%s>", name);
    }
    mState->enterSection(SECTION_UNKNOWN);
}

void MediaCodecsXmlParser::Impl::Parser::endElementHandler(const char *name) {
    // XMLParser handles tag matching, so we really just need to handle the section state here
    Section section = mState->section();
    switch (section) {
        case SECTION_INCLUDE:
        {
            // this could also be any of: Included, MediaCodecs
            if (strEq(name, "Include")) {
                mState->exitSection();
                return;
            }
            break;
        }

        case SECTION_SETTINGS:
        {
            // this could also be any of: Domain, Variant, Setting
            if (strEq(name, "Settings")) {
                mState->exitSection();
            }
            break;
        }

        case SECTION_DECODERS:
        case SECTION_ENCODERS:
        case SECTION_UNKNOWN:
        {
            mState->exitSection();
            break;
        }

        case SECTION_DECODER_TYPE:
        case SECTION_ENCODER_TYPE:
        {
            // this could also be any of: Alias, Limit, Feature
            if (strEq(name, "Type")) {
                mState->exitSection();
                mState->exitCodecOrType();
            }
            break;
        }

        case SECTION_DECODER:
        case SECTION_ENCODER:
        {
            // this could also be any of: Alias, Limit, Quirk, Variant
            if (strEq(name, "MediaCodec")) {
                mState->exitSection();
                mState->exitCodecOrType();
                mState->exitVariants();
            }
            break;
        }

        case SECTION_VARIANT:
        {
            // this could also be any of: Alias, Limit, Quirk
            if (strEq(name, "Variant")) {
                mState->exitSection();
                mState->exitVariants();
                return;
            }
            break;
        }

        default:
            break;
    }
}

status_t MediaCodecsXmlParser::Impl::Parser::addSetting(const char **attrs, const char *prefix) {
    const char *a_name = nullptr;
    const char *a_value = nullptr;
    const char *a_update = nullptr;
    bool isBoolean = false;

    size_t i = 0;
    while (attrs[i] != nullptr) {
        CHECK((i & 1) == 0);
        if (attrs[i + 1] == nullptr) {
            PLOGD("Setting: attribute '%s' is null", attrs[i]);
            return BAD_VALUE;
        }

        if (strEq(attrs[i], "name")) {
            a_name = attrs[++i];
        } else if (strEq(attrs[i], "value") || strEq(attrs[i], "enabled")) {
            if (a_value) {
                PLOGD("Setting: redundant attribute '%s'", attrs[i]);
                return BAD_VALUE;
            }
            isBoolean = strEq(attrs[i], "enabled");
            a_value = attrs[++i];
        } else if (strEq(attrs[i], "update")) {
            a_update = attrs[++i];
        } else {
            PLOGD("Setting: ignoring unrecognized attribute '%s'", attrs[i]);
            ++i;
        }
        ++i;
    }

    if (a_name == nullptr || a_value == nullptr) {
        PLOGD("Setting with no 'name' or 'value' attribute");
        return BAD_VALUE;
    }

    // Boolean values are converted to "0" or "1".
    if (strHasPrefix(a_name, "supports-") || isBoolean) {
        a_value = parseBoolean(a_value) ? "1" : "0";
    }

    bool update = (a_update != nullptr) && parseBoolean(a_update);
    Result res = mState->data().addGlobal(std::string(prefix ? : "") + a_name, a_value, update);
    if (res != OK) {
        PLOGD("Setting: %s", res.error().c_str());
    }
    return res;
}

MediaCodecsXmlParser::Impl::Result MediaCodecsXmlParser::Impl::Data::addGlobal(
        std::string key, std::string value, bool updating) {
    auto attribute = mServiceAttributeMap.find(key);
    if (attribute == mServiceAttributeMap.end()) { // New attribute name
        if (updating) {
            return { NAME_NOT_FOUND, "cannot update non-existing setting" };
        }
        mServiceAttributeMap.insert(Attribute(key, value));
    } else { // Existing attribute name
        attribute->second = value;
        if (!updating) {
            return { ALREADY_EXISTS, "updating existing setting" };
        }
    }

    return OK;
}

status_t MediaCodecsXmlParser::Impl::Parser::enterMediaCodec(
        const char **attrs, bool encoder) {
    const char *a_name = nullptr;
    const char *a_type = nullptr;
    const char *a_update = nullptr;
    const char *a_rank = nullptr;
    const char *a_domain = nullptr;
    const char *a_variant = nullptr;
    const char *a_enabled = nullptr;

    size_t i = 0;
    while (attrs[i] != nullptr) {
        CHECK((i & 1) == 0);
        if (attrs[i + 1] == nullptr) {
            PLOGD("MediaCodec: attribute '%s' is null", attrs[i]);
            return BAD_VALUE;
        }

        if (strEq(attrs[i], "name")) {
            a_name = attrs[++i];
        } else if (strEq(attrs[i], "type")) {
            a_type = attrs[++i];
        } else if (strEq(attrs[i], "update")) {
            a_update = attrs[++i];
        } else if (strEq(attrs[i], "rank")) {
            a_rank = attrs[++i];
        } else if (strEq(attrs[i], "domain")) {
            a_domain = attrs[++i];
        } else if (strEq(attrs[i], "variant")) {
            a_variant = attrs[++i];
        } else if (strEq(attrs[i], "enabled")) {
            a_enabled = attrs[++i];
        } else {
            PLOGD("MediaCodec: ignoring unrecognized attribute '%s'", attrs[i]);
            ++i;
        }
        ++i;
    }

    if (a_name == nullptr) {
        PLOGD("MediaCodec with no 'name' attribute");
        return BAD_VALUE;
    }

    bool update = (a_update != nullptr) && parseBoolean(a_update);
    if (a_domain != nullptr) {
        // disable codecs with domain by default (unless updating)
        if (!a_enabled && !update) {
            a_enabled = "false";
        }
    }

    Result res = mState->enterMediaCodec(encoder, a_name, a_type, update);
    if (res != OK) {
        logAnyErrors(res);
        return res;
    }

    return updateMediaCodec(
            a_rank, parseCommaSeparatedStringSet(a_domain),
            parseCommaSeparatedStringSet(a_variant), a_enabled);
}

MediaCodecsXmlParser::Impl::Result
MediaCodecsXmlParser::Impl::State::enterMediaCodec(
        bool encoder, const char *name, const char *type, bool updating) {
    // store name even in case of an error
    CodecMap::iterator codecIt = mData->mCodecMap.find(name);
    TypeMap::iterator typeIt;
    if (codecIt == mData->mCodecMap.end()) { // New codec name
        if (updating) {
            return { NAME_NOT_FOUND, "MediaCodec: cannot update non-existing codec" };
        }
        // Create a new codec in mCodecMap
        codecIt = mData->mCodecMap.insert(Codec(name, CodecProperties())).first;
        if (type != nullptr) {
            typeIt = codecIt->second.typeMap.insert(Type(type, AttributeMap())).first;
        } else {
            typeIt = codecIt->second.typeMap.end();
        }
        codecIt->second.isEncoder = encoder;
        codecIt->second.order = mData->mCodecMap.size();
    } else { // Existing codec name
        if (!updating) {
            return { ALREADY_EXISTS, "MediaCodec: cannot add existing codec" };
        }
        if (type != nullptr) {
            typeIt = codecIt->second.typeMap.find(type);
            if (typeIt == codecIt->second.typeMap.end()) {
                return { NAME_NOT_FOUND, "MediaCodec: cannot update non-existing type for codec" };
            }
        } else {
            // This should happen only when the codec has at most one type.
            typeIt = codecIt->second.typeMap.begin();
            if (typeIt == codecIt->second.typeMap.end()
                    || codecIt->second.typeMap.size() != 1) {
                return { BAD_VALUE, "MediaCodec: cannot update codec without type specified" };
            }
        }
    }
    mCurrent.emplace_back(CodecAndType{name, codecIt, typeIt, updating});
    return OK;
}

status_t MediaCodecsXmlParser::Impl::Parser::updateMediaCodec(
        const char *rank, const StringSet &domains, const StringSet &variants,
        const char *enabled) {
    CHECK(mState->inCodec());
    CodecProperties &codec = mState->codec();

    if (rank != nullptr) {
        ALOGD_IF(!codec.rank.empty() && codec.rank != rank,
                "codec '%s' rank changed from '%s' to '%s'",
                mState->codecName().c_str(), codec.rank.c_str(), rank);
        codec.rank = rank;
    }

    codec.variantSet = variants;

    for (const std::string &domain : domains) {
        if (domain.size() && domain.at(0) == '!') {
            codec.domainSet.erase(domain.substr(1));
        } else {
            codec.domainSet.emplace(domain);
        }
    }

    if (enabled != nullptr) {
        if (parseBoolean(enabled)) {
            codec.quirkSet.erase("attribute::disabled");
            ALOGD("enabling %s", mState->codecName().c_str());
        } else {
            codec.quirkSet.emplace("attribute::disabled");
            ALOGD("disabling %s", mState->codecName().c_str());
        }
    }
    return OK;
}

status_t MediaCodecsXmlParser::Impl::Parser::addQuirk(const char **attrs, const char *prefix) {
    CHECK(mState->inCodec());
    const char *a_name = nullptr;

    size_t i = 0;
    while (attrs[i] != nullptr) {
        CHECK((i & 1) == 0);
        if (attrs[i + 1] == nullptr) {
            PLOGD("Quirk: attribute '%s' is null", attrs[i]);
            return BAD_VALUE;
        }

        if (strEq(attrs[i], "name")) {
            a_name = attrs[++i];
        } else {
            PLOGD("Quirk: ignoring unrecognized attribute '%s'", attrs[i]);
            ++i;
        }
        ++i;
    }

    if (a_name == nullptr) {
        PLOGD("Quirk with no 'name' attribute");
        return BAD_VALUE;
    }

    std::string key = std::string(prefix ? : "") + a_name;
    mState->codec().quirkSet.emplace(key);
    ALOGV("adding %s to %s", key.c_str(), mState->codecName().c_str());
    return OK;
}

status_t MediaCodecsXmlParser::Impl::Parser::enterType(const char **attrs) {
    CHECK(mState->inCodec());

    const char *a_name = nullptr;
    const char *a_update = nullptr;

    size_t i = 0;
    while (attrs[i] != nullptr) {
        CHECK((i & 1) == 0);
        if (attrs[i + 1] == nullptr) {
            PLOGD("Type: attribute '%s' is null", attrs[i]);
            return BAD_VALUE;
        }

        if (strEq(attrs[i], "name")) {
            a_name = attrs[++i];
        } else if (strEq(attrs[i], "update")) {
            a_update = attrs[++i];
        } else {
            PLOGD("Type: ignoring unrecognized attribute '%s'", attrs[i]);
            ++i;
        }
        ++i;
    }

    if (a_name == nullptr) {
        PLOGD("Type with no 'name' attribute");
        return BAD_VALUE;
    }

    bool update = (a_update != nullptr) && parseBoolean(a_update);
    return mState->enterType(a_name, update);
}

MediaCodecsXmlParser::Impl::Result
MediaCodecsXmlParser::Impl::State::enterType(const char *name, bool update) {
    update = update || updating(); // handle parent

    CodecMap::iterator codecIt = mCurrent.back().mCodec;
    TypeMap::iterator typeIt = codecIt->second.typeMap.find(name);
    if (!update) {
        if (typeIt != codecIt->second.typeMap.end()) {
            return { ALREADY_EXISTS, "trying to update existing type '" + std::string(name) + "'" };
        }
        typeIt = codecIt->second.typeMap.insert(Type(name, AttributeMap())).first;
    } else if (typeIt == codecIt->second.typeMap.end()) {
        return { NAME_NOT_FOUND, "addType: updating non-existing type" };
    }
    mCurrent.push_back({ codecName(), codecIt, typeIt, update });
    CHECK(inType());
    return OK;
}

status_t MediaCodecsXmlParser::Impl::Parser::addLimit(const char **attrs) {
    CHECK(mState->inType());
    const char* a_name = nullptr;
    const char* a_default = nullptr;
    const char* a_in = nullptr;
    const char* a_max = nullptr;
    const char* a_min = nullptr;
    const char* a_range = nullptr;
    const char* a_ranges = nullptr;
    const char* a_scale = nullptr;
    const char* a_value = nullptr;

    size_t i = 0;
    while (attrs[i] != nullptr) {
        CHECK((i & 1) == 0);
        if (attrs[i + 1] == nullptr) {
            PLOGD("Limit: attribute '%s' is null", attrs[i]);
            return BAD_VALUE;
        }

        if (strEq(attrs[i], "name")) {
            a_name = attrs[++i];
        } else if (strEq(attrs[i], "default")) {
            a_default = attrs[++i];
        } else if (strEq(attrs[i], "in")) {
            a_in = attrs[++i];
        } else if (strEq(attrs[i], "max")) {
            a_max = attrs[++i];
        } else if (strEq(attrs[i], "min")) {
            a_min = attrs[++i];
        } else if (strEq(attrs[i], "range")) {
            a_range = attrs[++i];
        } else if (strEq(attrs[i], "ranges")) {
            a_ranges = attrs[++i];
        } else if (strEq(attrs[i], "scale")) {
            a_scale = attrs[++i];
        } else if (strEq(attrs[i], "value")) {
            a_value = attrs[++i];
        } else {
            PLOGD("Limit: ignoring unrecognized limit: %s", attrs[i]);
            ++i;
        }
        ++i;
    }

    if (a_name == nullptr) {
        PLOGD("Limit with no 'name' attribute");
        return BAD_VALUE;
    }

    // size, blocks, bitrate, frame-rate, blocks-per-second, aspect-ratio,
    // measured-frame-rate, measured-blocks-per-second: range
    // quality: range + default + [scale]
    // complexity: range + default
    std::string key = a_name, value;

    // don't allow specifying more than one of value, range or min/max
    if ((a_value != nullptr) + (a_range != nullptr) + (a_ranges != nullptr)
            + (a_min != nullptr || a_max != nullptr) > 1) {
        PLOGD("Limit '%s' has multiple 'min'/'max', 'range', 'ranges' or 'value' attributes",
                a_name);
        return BAD_VALUE;
    }

    // Min/max limits (only containing min or max attribute)
    //
    // Current "max" limits are "channel-count", "concurrent-instances".
    // There are no current "min" limits
    //
    // Range limits. "range" is specified in exactly one of the following forms:
    // 1) min-max
    // 2) value-value
    // 3) range
    //
    // Current range limits are "aspect-ratio", "bitrate", "block-count", "blocks-per-second",
    // "complexity", "frame-rate", "quality", "size", "measured-blocks-per-second",
    // "performance-point-*", "measured-frame-rate-*"
    //
    // Other limits (containing only value or ranges)
    //
    // Current ranges limit is "sample-rate"
    if ((a_min != nullptr) ^ (a_max != nullptr)) {
        // min/max limit
        if (a_max != nullptr) {
            key = "max-" + key;
            value = a_max;
        } else if (a_min != nullptr) {
            key = "min-" + key;
            value = a_min;
        }
    } else if (a_min != nullptr && a_max != nullptr) {
        // min-max
        key += "-range";
        value = a_min + std::string("-") + a_max;
    } else if (a_value != nullptr) {
        // value-value or value
        value = a_value;
        if (strEq(a_name, "aspect-ratio") ||
            strEq(a_name, "bitrate") ||
            strEq(a_name, "block-count") ||
            strEq(a_name, "blocks-per-second") ||
            strEq(a_name, "complexity") ||
            strEq(a_name, "frame-rate") ||
            strEq(a_name, "quality") ||
            strEq(a_name, "size") ||
            strEq(a_name, "measured-blocks-per-second") ||
            strHasPrefix(a_name, "performance-point-") ||
            strHasPrefix(a_name, "measured-frame-rate-")) {
            key += "-range";
            value += std::string("-") + a_value;
        }
    } else if (a_range != nullptr) {
        // range
        key += "-range";
        value = a_range;
    } else if (a_ranges != nullptr) {
        // ranges
        key += "-ranges";
        value = a_ranges;
    } else {
        PLOGD("Limit '%s' with no 'range', 'value' or 'min'/'max' attributes", a_name);
        return BAD_VALUE;
    }

    // handle 'in' attribute - this changes the key
    if (a_in != nullptr) {
        // Currently "aspect-ratio" uses in attribute
        const size_t a_in_len = strlen(a_in);
        key = std::string(a_in, a_in_len - a_in[a_in_len] == 's') + '-' + key;
    }

    // handle 'scale' attribute - this adds a new detail
    if (a_scale != nullptr) {
        mState->addDetail(a_name + std::string("-scale"), a_scale);
    } else if (strEq(a_name, "quality")) {
        // The default value of "quality-scale" is "linear" even if unspecified.
        mState->addDetail(a_name + std::string("-scale"), "linear");
    }

    // handle 'default' attribute - this adds a new detail
    if (a_default != nullptr) {
        mState->addDetail(a_name + std::string("-default"), a_default);
    }

    mState->addDetail(key, value);
    return OK;
}

void MediaCodecsXmlParser::Impl::State::addDetail(
        const std::string &key, const std::string &value) {
    CHECK(inType());
    ALOGI("limit: %s = %s", key.c_str(), value.c_str());
    const StringSet &variants = mVariantsStack.back();
    if (variants.empty()) {
        type()[key] = value;
    } else {
        for (const std::string &variant : variants) {
            type()[variant + ":::" + key] = value;
        }
    }
}

status_t MediaCodecsXmlParser::Impl::Parser::limitVariants(const char **attrs) {
    const char* a_name = nullptr;

    size_t i = 0;
    while (attrs[i] != nullptr) {
        CHECK((i & 1) == 0);
        if (attrs[i + 1] == nullptr) {
            PLOGD("Variant: attribute '%s' is null", attrs[i]);
            return BAD_VALUE;
        }
        if (strEq(attrs[i], "name")) {
            a_name = attrs[++i];
        } else {
            PLOGD("Variant: ignoring unrecognized attribute: %s", attrs[i]);
            ++i;
        }
        ++i;
    }

    if (a_name == nullptr || *a_name == '\0') {
        PLOGD("Variant with no or empty 'name' attribute");
        return BAD_VALUE;
    }

    StringSet variants;
    for (const std::string &variant : parseCommaSeparatedStringSet(a_name)) {
        if (mState->variants().count(variant)) {
            variants.emplace(variant);
        } else {
            PLOGD("Variant: variant '%s' not in parent variants", variant.c_str());
            return BAD_VALUE;
        }
    }
    mState->enterVariants(variants);
    return OK;
}

status_t MediaCodecsXmlParser::Impl::Parser::addFeature(const char **attrs) {
    CHECK(mState->inType());
    size_t i = 0;
    const char *a_name = nullptr;
    int32_t optional = -1;
    int32_t required = -1;
    const char *a_value = nullptr;

    while (attrs[i] != nullptr) {
        CHECK((i & 1) == 0);
        if (attrs[i + 1] == nullptr) {
            PLOGD("Feature: attribute '%s' is null", attrs[i]);
            return BAD_VALUE;
        }

        if (strEq(attrs[i], "name")) {
            a_name = attrs[++i];
        } else if (strEq(attrs[i], "optional")) {
            optional = parseBoolean(attrs[++i]) ? 1 : 0;
        } else if (strEq(attrs[i], "required")) {
            required = parseBoolean(attrs[++i]) ? 1 : 0;
        } else if (strEq(attrs[i], "value")) {
            a_value = attrs[++i];
        } else {
            PLOGD("Feature: ignoring unrecognized attribute '%s'", attrs[i]);
            ++i;
        }
        ++i;
    }

    // Every feature must have a name.
    if (a_name == nullptr) {
        PLOGD("Feature with no 'name' attribute");
        return BAD_VALUE;
    }

    if (a_value != nullptr) {
        if (optional != -1 || required != -1) {
            PLOGD("Feature '%s' has both value and optional/required attributes", a_name);
            return BAD_VALUE;
        }
    } else {
        if (optional == required && optional != -1) {
            PLOGD("Feature '%s' is both/neither optional and required", a_name);
            return BAD_VALUE;
        }
        a_value = (required == 1 || optional == 0) ? "1" : "0";
    }

    mState->addDetail(std::string("feature-") + a_name, a_value ? : "0");
    return OK;
}

status_t MediaCodecsXmlParser::Impl::Parser::addAlias(const char **attrs) {
    CHECK(mState->inCodec());
    size_t i = 0;
    const char *a_name = nullptr;

    while (attrs[i] != nullptr) {
        CHECK((i & 1) == 0);
        if (attrs[i + 1] == nullptr) {
            PLOGD("Alias: attribute '%s' is null", attrs[i]);
            return BAD_VALUE;
        }

        if (strEq(attrs[i], "name")) {
            a_name = attrs[++i];
        } else {
            PLOGD("Alias: ignoring unrecognized attribute '%s'", attrs[i]);
            ++i;
        }
        ++i;
    }

    // Every feature must have a name.
    if (a_name == nullptr) {
        PLOGD("Alias with no 'name' attribute");
        return BAD_VALUE;
    }

    mState->codec().aliases.emplace_back(a_name);
    return OK;
}

const MediaCodecsXmlParser::AttributeMap&
MediaCodecsXmlParser::getServiceAttributeMap() const {
    return mImpl->getServiceAttributeMap();
}

const MediaCodecsXmlParser::CodecMap&
MediaCodecsXmlParser::getCodecMap() const {
    return mImpl->getCodecMap();
}

const MediaCodecsXmlParser::RoleMap&
MediaCodecsXmlParser::getRoleMap() const {
    return mImpl->getRoleMap();
}

const MediaCodecsXmlParser::RoleMap&
MediaCodecsXmlParser::Impl::getRoleMap() const {
    std::lock_guard<std::mutex> guard(mLock);
    if (mRoleMap.empty()) {
        generateRoleMap();
    }
    return mRoleMap;
}

const char* MediaCodecsXmlParser::getCommonPrefix() const {
    return mImpl->getCommonPrefix();
}

const char* MediaCodecsXmlParser::Impl::getCommonPrefix() const {
    std::lock_guard<std::mutex> guard(mLock);
    if (mCommonPrefix.empty()) {
        generateCommonPrefix();
    }
    return mCommonPrefix.data();
}

status_t MediaCodecsXmlParser::getParsingStatus() const {
    return mImpl->getParsingStatus();
}

void MediaCodecsXmlParser::Impl::generateRoleMap() const {
    for (const auto& codec : mData.mCodecMap) {
        const auto &codecName = codec.first;
        if (codecName == "<dummy>") {
            continue;
        }
        bool isEncoder = codec.second.isEncoder;
        size_t order = codec.second.order;
        std::string rank = codec.second.rank;
        const auto& typeMap = codec.second.typeMap;
        for (const auto& type : typeMap) {
            const auto& typeName = type.first;
            const char* roleName = GetComponentRole(isEncoder, typeName.data());
            if (roleName == nullptr) {
                ALOGE("Cannot find the role for %s of type %s",
                        isEncoder ? "an encoder" : "a decoder",
                        typeName.data());
                continue;
            }
            const auto& typeAttributeMap = type.second;

            auto roleIterator = mRoleMap.find(roleName);
            std::multimap<size_t, NodeInfo>* nodeList;
            if (roleIterator == mRoleMap.end()) {
                RoleProperties roleProperties;
                roleProperties.type = typeName;
                roleProperties.isEncoder = isEncoder;
                auto insertResult = mRoleMap.insert(
                        std::make_pair(roleName, roleProperties));
                if (!insertResult.second) {
                    ALOGE("Cannot add role %s", roleName);
                    continue;
                }
                nodeList = &insertResult.first->second.nodeList;
            } else {
                if (roleIterator->second.type != typeName) {
                    ALOGE("Role %s has mismatching types: %s and %s",
                            roleName,
                            roleIterator->second.type.data(),
                            typeName.data());
                    continue;
                }
                if (roleIterator->second.isEncoder != isEncoder) {
                    ALOGE("Role %s cannot be both an encoder and a decoder",
                            roleName);
                    continue;
                }
                nodeList = &roleIterator->second.nodeList;
            }

            NodeInfo nodeInfo;
            nodeInfo.name = codecName;
            // NOTE: no aliases are exposed in role info
            // attribute quirks are exposed as node attributes
            nodeInfo.attributeList.reserve(typeAttributeMap.size());
            for (const auto& attribute : typeAttributeMap) {
                nodeInfo.attributeList.push_back(
                        Attribute{attribute.first, attribute.second});
            }
            for (const std::string &quirk : codec.second.quirkSet) {
                if (strHasPrefix(quirk.c_str(), "attribute::")) {
                    nodeInfo.attributeList.push_back(Attribute{quirk, "present"});
                }
            }
            if (!rank.empty()) {
                nodeInfo.attributeList.push_back(Attribute{"rank", rank});
            }
            nodeList->insert(std::make_pair(
                    std::move(order), std::move(nodeInfo)));
        }
    }
}

void MediaCodecsXmlParser::Impl::generateCommonPrefix() const {
    if (mData.mCodecMap.empty()) {
        return;
    }
    auto i = mData.mCodecMap.cbegin();
    auto first = i->first.cbegin();
    auto last = i->first.cend();
    for (++i; i != mData.mCodecMap.cend(); ++i) {
        last = std::mismatch(
                first, last, i->first.cbegin(), i->first.cend()).first;
    }
    mCommonPrefix.insert(mCommonPrefix.begin(), first, last);
}

} // namespace android
