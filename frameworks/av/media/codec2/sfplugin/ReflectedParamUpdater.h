/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef REFLECTED_PARAM_BUILDER_H_
#define REFLECTED_PARAM_BUILDER_H_

#include <map>
#include <memory>

#include <C2.h>
#include <C2Param.h>

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/AData.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>

namespace android {

/**
 * Utility class to query and update Codec 2.0 configuration values. Use custom dictionary as
 * AMessage cannot represent all types of Codec 2.0 parameters and C2Value cannot represent
 * all types of SDK values. We want to be precise when setting standard parameters (use C2Value
 * for arithmetic values), but also support int32 and int64 for SDK values specifically for
 * vendor parameters (as SDK API does not allow specifying proper type.) When querying fields,
 * we can use C2Values as they are defined.
 *
 *      Item => Codec 2.0 value mappings:
 *     CValue::type => type
 *     int32 => int32, ctr32 or uint32
 *     int64 => int64, ctr64 or uint64
 *     AString => string
 *     ABuffer => blob
 *     'Rect' => C2RectStruct (not exposed in SDK as a rectangle)
 */
class ReflectedParamUpdater {
public:
    ReflectedParamUpdater() = default;
    ~ReflectedParamUpdater() = default;

    /**
     * Element for values
     */
    struct Value : public AData<C2Value, int32_t, int64_t, AString, sp<ABuffer>>::Basic {
        // allow construction from base types
        Value() = default;
        explicit Value(C2Value i)            { set(i); }
        explicit Value(int32_t i)            { set(i); }
        explicit Value(int64_t i)            { set(i); }
        explicit Value(const AString &i)     { set(i); }
        explicit Value(const sp<ABuffer> &i) { set(i); }
    };

    struct Dict : public std::map<std::string, Value> {
        Dict() = default;
        std::string debugString(size_t indent = 0) const;
    };

    /**
     * Enumerates all fields of the parameter descriptors supplied, so that this opbject can later
     * query and update these.
     *
     * For now only first-level fields are supported. Also, array fields are not supported.
     *
     * \param reflector   C2ParamReflector object for C2Param reflection.
     * \param paramDescs  vector of C2ParamDescriptor objects that this object
     *                    would recognize when building params.
     */
    void addParamDesc(
            const std::shared_ptr<C2ParamReflector> &reflector,
            const std::vector<std::shared_ptr<C2ParamDescriptor>> &paramDescs);

    /**
     * Adds fields of a standard parameter (that may not be supported by the parameter reflector
     * or may not be listed as a supported value by the component). If the parameter name is
     * used for another parameter, this operation is a no-op. (Technically, this is by fields).
     *
     * \param T standard parameter type
     * \param name parameter name
     */
    template<typename T>
    void addStandardParam(const std::string &name, C2ParamDescriptor::attrib_t attrib =
                          C2ParamDescriptor::IS_READ_ONLY) {
        addParamDesc(std::make_shared<C2ParamDescriptor>(
                C2Param::Index(T::PARAM_TYPE), attrib, name.c_str()),
                C2StructDescriptor((T*)nullptr), nullptr /* descriptor */);
    }

    /**
     * Adds fields of a structure (or a parameater) described by the struct descriptor. If
     * reflector is provided, fields of sub-structures are also added. Otherwise, only top-level
     * fundamental typed fields (arithmetic, string and blob) are added.
     *
     * \param paramDesc parameter descriptor
     * \param fieldDesc field descriptor
     * \param path path/name of the structure (field or parent parameter)
     * \param offset offset of the structure in the parameter
     * \param reflector  C2ParamReflector object for C2Param reflection (may be null)
     */
    void addParamStructDesc(
            std::shared_ptr<C2ParamDescriptor> paramDesc, C2String path, size_t offset,
            const C2StructDescriptor &structDesc,
            const std::shared_ptr<C2ParamReflector> &reflector);

    /**
     * Adds fields of a parameter described by the struct descriptor. If reflector is provided,
     * fields of sub-structures are also added. Otherwise, only top-level fundamental typed fields
     * (arithmetic, string and blob) are added.
     *
     * \param paramDesc parameter descriptor
     * \param fieldDesc field descriptor
     * \param reflector  C2ParamReflector object for C2Param reflection (may be null)
     * \param markVendor TEMP if true, prefix vendor parameter names with "vendor."
     */
    void addParamDesc(
            std::shared_ptr<C2ParamDescriptor> paramDesc, const C2StructDescriptor &structDesc,
            const std::shared_ptr<C2ParamReflector> &reflector,
            bool markVendor = true);

    /**
     * Add support for setting a parameter as a binary blob.
     *
     * \param name name of the parameter
     * \param coreIndex parameter (core) index
     */
    void supportWholeParam(std::string name, C2Param::CoreIndex coreIndex);

    /**
     * Returns the name of the parameter for an index.
     */
    std::string getParamName(C2Param::Index index) const;

    /**
     * Get list of param indices from field names and values in AMessage object.
     *
     * TODO: This should be in the order that they are listed by the component.
     *
     * \param params[in]  Dict object with field name to value pairs.
     * \param vec[out]    vector to store the indices from |params|.
     */
    void getParamIndicesFromMessage(
            const Dict &params,
            std::vector<C2Param::Index> *vec /* nonnull */) const;

    /**
     * Get list of param indices from field names (only) in AMessage object.
     *
     * \param params[in]  Vector object with field names.
     * \param vec[out]    vector to store the indices from |params|.
     */
    void getParamIndicesForKeys(
            const std::vector<std::string> &keys,
            std::vector<C2Param::Index> *vec /* nonnull */) const;

    /**
     * Update C2Param objects from field name and value in AMessage object.
     *
     * \param params[in]    Dict object with field name to value pairs.
     * \param vec[in,out]   vector of the C2Param objects to be updated.
     */
    void updateParamsFromMessage(
            const Dict &params,
            std::vector<std::unique_ptr<C2Param>> *vec /* nonnull */) const;

    /**
     * Get fields from C2Param objects in AMessage object.
     *
     * \param params[in]    vector of the C2Param objects to be queried
     * \return a Dict object containing the known parameters
     */
    Dict getParams(
            const std::vector<C2Param*> &params /* nonnull */) const;

    Dict getParams(
            const std::vector<std::unique_ptr<C2Param>> &params /* nonnull */) const;

    /**
     * Clear param descriptors in this object.
     */
    void clear();

private:
    struct FieldDesc {
        std::shared_ptr<C2ParamDescriptor> paramDesc;
        std::unique_ptr<C2FieldDescriptor> fieldDesc;
        size_t offset;
    };
    std::map<std::string, FieldDesc> mMap;
    std::map<C2Param::Index, std::string> mParamNames;
    std::map<std::string, C2Param::CoreIndex> mWholeParams;

    void parseMessageAndDoWork(
            const Dict &params,
            std::function<void(const std::string &, const FieldDesc &, const void *, size_t)> work) const;

    C2_DO_NOT_COPY(ReflectedParamUpdater);
};

}  // namespace android

#endif  // REFLECTED_PARAM_BUILDER_H_
