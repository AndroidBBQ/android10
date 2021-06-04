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

#ifndef MEDIA_STAGEFRIGHT_XMLPARSER_H_
#define MEDIA_STAGEFRIGHT_XMLPARSER_H_

#include <sys/types.h>
#include <utils/Errors.h>

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

struct XML_ParserStruct; // from expat library

namespace android {

class MediaCodecsXmlParser {
public:

    // Treblized media codec list will be located in /odm/etc or /vendor/etc.
    static std::vector<std::string> getDefaultSearchDirs() {
            return { "/odm/etc", "/vendor/etc", "/etc" };
    }
    static std::vector<std::string> getDefaultXmlNames() {
            return { "media_codecs.xml", "media_codecs_performance.xml" };
    }
    static constexpr char const* defaultProfilingResultsXmlPath =
            "/data/misc/media/media_codecs_profiling_results.xml";

    MediaCodecsXmlParser();
    ~MediaCodecsXmlParser();

    typedef std::pair<std::string, std::string> Attribute;
    typedef std::map<std::string, std::string> AttributeMap;

    typedef std::pair<std::string, AttributeMap> Type;
    typedef std::map<std::string, AttributeMap> TypeMap;

    typedef std::set<std::string> StringSet;

    /**
     * Properties of a codec (node)
     */
    struct CodecProperties {
        bool isEncoder;    ///< Whether this codec is an encoder or a decoder
        size_t order;      ///< Order of appearance in the file (starting from 0)
        StringSet quirkSet; ///< Set of quirks requested by this codec
        StringSet domainSet; ///< Set of domains this codec is in
        StringSet variantSet; ///< Set of variants this codec is enabled on
        TypeMap typeMap;   ///< Map of types supported by this codec
        std::vector<std::string> aliases; ///< Name aliases for this codec
        std::string rank;  ///< Rank of this codec. This is a numeric string.
    };

    typedef std::pair<std::string, CodecProperties> Codec;
    typedef std::map<std::string, CodecProperties> CodecMap;

    /**
     * Properties of a node (for IOmxStore)
     */
    struct NodeInfo {
        std::string name;
        std::vector<Attribute> attributeList;
        // note: aliases are not exposed here as they are not part of the role map
    };

    /**
     * Properties of a role (for IOmxStore)
     */
    struct RoleProperties {
        std::string type;
        bool isEncoder;
        std::multimap<size_t, NodeInfo> nodeList;
    };

    typedef std::pair<std::string, RoleProperties> Role;
    typedef std::map<std::string, RoleProperties> RoleMap;

    /**
     * Return a map for attributes that are service-specific.
     */
    const AttributeMap& getServiceAttributeMap() const;

    /**
     * Return a map for codecs and their properties.
     */
    const CodecMap& getCodecMap() const;

    /**
     * Return a map for roles and their properties.
     * This map is generated from the CodecMap.
     */
    const RoleMap& getRoleMap() const;

    /**
     * Return a common prefix of all node names.
     *
     * The prefix is not provided in the xml, so it has to be computed by taking
     * the longest common prefix of all node names.
     */
    const char* getCommonPrefix() const;

    status_t getParsingStatus() const;

    /**
     * Parse top level XML files from a group of search directories.
     *
     * @param xmlFiles ordered list of XML file names (no paths)
     * @param searchDirs ordered list of paths to consider
     *
     * @return parsing status
     */
    status_t parseXmlFilesInSearchDirs(
            const std::vector<std::string> &xmlFiles = getDefaultXmlNames(),
            const std::vector<std::string> &searchDirs = getDefaultSearchDirs());


    /**
     * Parse a top level XML file.
     *
     * @param path XML file path
     *
     * @return parsing status
     */
    status_t parseXmlPath(const std::string &path);

private:
    struct Impl;
    std::shared_ptr<Impl> mImpl;

    MediaCodecsXmlParser(const MediaCodecsXmlParser&) = delete;
    MediaCodecsXmlParser& operator=(const MediaCodecsXmlParser&) = delete;
};

} // namespace android

#endif // MEDIA_STAGEFRIGHT_XMLPARSER_H_
