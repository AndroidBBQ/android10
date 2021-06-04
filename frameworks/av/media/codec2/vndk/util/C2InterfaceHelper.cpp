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

#include <C2Debug.h>
#include <C2ParamInternal.h>
#include <util/C2InterfaceHelper.h>

#include <android-base/stringprintf.h>

using ::android::base::StringPrintf;

/* --------------------------------- ReflectorHelper --------------------------------- */

void C2ReflectorHelper::addStructDescriptors(
        std::vector<C2StructDescriptor> &structs, _Tuple<> *) {
    std::lock_guard<std::mutex> lock(_mMutex);
    for (C2StructDescriptor &strukt : structs) {
        // TODO: check if structure descriptions conflict with existing ones
        addStructDescriptor(std::move(strukt));
    }
}

std::unique_ptr<C2StructDescriptor>
C2ReflectorHelper::describe(C2Param::CoreIndex paramIndex) const {
    std::lock_guard<std::mutex> lock(_mMutex);
    auto it = _mStructs.find(paramIndex);
    if (it == _mStructs.end()) {
        return nullptr;
    } else {
        return std::make_unique<C2StructDescriptor>(it->second);
    }
};

void C2ReflectorHelper::addStructDescriptor(C2StructDescriptor &&strukt) {
    if (_mStructs.find(strukt.coreIndex()) != _mStructs.end()) {
        // already added
        // TODO: validate that descriptor matches stored descriptor
    }
    // validate that all struct fields are known to this reflector
    for (const C2FieldDescriptor &fd : strukt) {
        if (fd.type() & C2FieldDescriptor::STRUCT_FLAG) {
            C2Param::CoreIndex coreIndex = fd.type() &~ C2FieldDescriptor::STRUCT_FLAG;
            if (_mStructs.find(coreIndex) == _mStructs.end()) {
                C2_LOG(INFO) << "missing struct descriptor #" << coreIndex << " for field "
                        << fd.name() << " of struct #" << strukt.coreIndex();
            }
        }
    }
    _mStructs.emplace(strukt.coreIndex(), strukt);
}


/* ---------------------------- ParamHelper ---------------------------- */

class C2InterfaceHelper::ParamHelper::Impl {
public:
    Impl(ParamRef param, C2StringLiteral name, C2StructDescriptor &&strukt)
        : mParam(param), mName(name), _mStruct(strukt) { }

    Impl(Impl&&) = default;

    void addDownDependency(C2Param::Index index) {
        mDownDependencies.push_back(index);
    }

    C2InterfaceHelper::ParamHelper::attrib_t& attrib() {
        return mAttrib;
    }

    void build() {
        // move dependencies into descriptor
        mDescriptor = std::make_shared<C2ParamDescriptor>(
                index(), (C2ParamDescriptor::attrib_t)mAttrib,
                std::move(mName), std::move(mDependencies));
    }

    void createFieldsAndSupportedValues(const std::shared_ptr<C2ParamReflector> &reflector) {
        for (const C2FieldUtils::Info &f :
                C2FieldUtils::enumerateFields(*mDefaultValue, reflector)) {
            if (!f.isArithmetic()) {
                continue;
            }
            std::unique_ptr<C2FieldSupportedValues> fsvPointer;

            // create a breakable structure
            do {
                C2FieldSupportedValues fsv;
                switch (f.type()) {
                    case C2FieldDescriptor::INT32:  fsv = C2SupportedRange<int32_t>::Any(); break;
                    case C2FieldDescriptor::UINT32: fsv = C2SupportedRange<uint32_t>::Any(); break;
                    case C2FieldDescriptor::INT64:  fsv = C2SupportedRange<int64_t>::Any(); break;
                    case C2FieldDescriptor::UINT64: fsv = C2SupportedRange<uint64_t>::Any(); break;
                    case C2FieldDescriptor::FLOAT:  fsv = C2SupportedRange<float>::Any(); break;
                    case C2FieldDescriptor::BLOB:   fsv = C2SupportedRange<uint8_t>::Any(); break;
                    case C2FieldDescriptor::STRING: fsv = C2SupportedRange<char>::Any(); break;
                default:
                    continue; // break out of do {} while
                }
                fsvPointer = std::make_unique<C2FieldSupportedValues>(fsv);
            } while (false);

            mFields.emplace_hint(
                    mFields.end(),
                    _C2FieldId(f.offset(), f.size()),
                    std::make_shared<FieldHelper>(
                            mParam, _C2FieldId(f.offset(), f.size()), std::move(fsvPointer)));
        }
    }

    /**
     * Finds a field descriptor.
     */
    std::shared_ptr<FieldHelper> findField(size_t baseOffs, size_t baseSize) const {
        auto it = mFields.find(_C2FieldId(baseOffs, baseSize));
        if (it == mFields.end()) {
            return nullptr;
        }
        return it->second;
    }

    const std::vector<ParamRef> getDependenciesAsRefs() const {
        return mDependenciesAsRefs;
    }

    std::shared_ptr<const C2ParamDescriptor> getDescriptor() const {
        return mDescriptor;
    }

    const std::vector<C2Param::Index> getDownDependencies() const {
        return mDownDependencies;
    }

    C2Param::Index index() const {
        if (!mDefaultValue) {
            fprintf(stderr, "%s missing default value\n", mName.c_str());
        }
        return mDefaultValue->index();
    }

    C2String name() const {
        return mName;
    }

    const ParamRef ref() const {
        return mParam;
    }

    C2StructDescriptor retrieveStructDescriptor() {
        return std::move(_mStruct);
    }

    void setDefaultValue(std::shared_ptr<C2Param> default_) {
        mDefaultValue = default_;
    }

    void setDependencies(std::vector<C2Param::Index> indices, std::vector<ParamRef> refs) {
        mDependencies = indices;
        mDependenciesAsRefs = refs;
    }

    void setFields(std::vector<C2ParamFieldValues> &&fields) {
        // do not allow adding fields multiple times, or to const values
        if (!mFields.empty()) {
            C2_LOG(FATAL) << "Trying to add fields to param " << mName << " multiple times";
        } else if (mAttrib & attrib_t::IS_CONST) {
            C2_LOG(FATAL) << "Trying to add fields to const param " << mName;
        }

        for (C2ParamFieldValues &pfv : fields) {
            mFields.emplace_hint(
                    mFields.end(),
                    // _C2FieldId constructor
                    _C2ParamInspector::GetField(pfv.paramOrField),
                    // Field constructor
                    std::make_shared<FieldHelper>(mParam,
                                            _C2ParamInspector::GetField(pfv.paramOrField),
                                            std::move(pfv.values)));
        }
    }

    void setGetter(std::function<std::shared_ptr<C2Param>(bool)> getter) {
        mGetter = getter;
    }

    void setSetter(std::function<C2R(const C2Param *, bool, bool *, Factory &)> setter) {
        mSetter = setter;
    }

    c2_status_t trySet(
            const C2Param *value, bool mayBlock, bool *changed, Factory &f,
            std::vector<std::unique_ptr<C2SettingResult>>* const failures) {
        C2R result = mSetter(value, mayBlock, changed, f);
        return result.retrieveFailures(failures);
    }

    c2_status_t validate(const std::shared_ptr<C2ParamReflector> &reflector) {
        if (!mSetter && mFields.empty()) {
            C2_LOG(WARNING) << "Param " << mName << " has no setter, making it const";
            // dependencies are empty in this case
            mAttrib |= attrib_t::IS_CONST;
        } else if (!mSetter) {
            C2_LOG(FATAL) << "Param " << mName << " has no setter";
        }

        if (mAttrib & attrib_t::IS_CONST) {
            createFieldsAndSupportedValues(reflector);
        } else {
            // TODO: update default based on setter and verify that FSV covers the values
        }

        if (mFields.empty()) {
            C2_LOG(FATAL) << "Param " << mName << " has no fields";
        }

        return C2_OK;
    }

    std::shared_ptr<C2Param> value() {
        return mParam.get();
    }

    std::shared_ptr<const C2Param> value() const {
        return mParam.get();
    }

private:
    typedef _C2ParamInspector::attrib_t attrib_t;
    ParamRef mParam;
    C2String mName;
    C2StructDescriptor _mStruct;
    std::shared_ptr<C2Param> mDefaultValue;
    attrib_t mAttrib;
    std::function<C2R(const C2Param *, bool, bool *, Factory &)> mSetter;
    std::function<std::shared_ptr<C2Param>(bool)> mGetter;
    std::vector<C2Param::Index> mDependencies;
    std::vector<ParamRef> mDependenciesAsRefs;
    std::vector<C2Param::Index> mDownDependencies; // TODO: this does not work for stream dependencies
    std::map<_C2FieldId, std::shared_ptr<FieldHelper>> mFields;
    std::shared_ptr<C2ParamDescriptor> mDescriptor;
};

C2InterfaceHelper::ParamHelper::ParamHelper(
        ParamRef param, C2StringLiteral name, C2StructDescriptor &&strukt)
    : mImpl(std::make_unique<C2InterfaceHelper::ParamHelper::Impl>(
            param, name, std::move(strukt))) { }

C2InterfaceHelper::ParamHelper::ParamHelper(C2InterfaceHelper::ParamHelper &&) = default;

C2InterfaceHelper::ParamHelper::~ParamHelper() = default;

void C2InterfaceHelper::ParamHelper::addDownDependency(C2Param::Index index) {
    return mImpl->addDownDependency(index);
}

C2InterfaceHelper::ParamHelper::attrib_t& C2InterfaceHelper::ParamHelper::attrib() {
    return mImpl->attrib();
}

std::shared_ptr<C2InterfaceHelper::ParamHelper> C2InterfaceHelper::ParamHelper::build() {
    mImpl->build();
    return std::make_shared<C2InterfaceHelper::ParamHelper>(std::move(*this));
}

std::shared_ptr<C2InterfaceHelper::FieldHelper>
C2InterfaceHelper::ParamHelper::findField(size_t baseOffs, size_t baseSize) const {
    return mImpl->findField(baseOffs, baseSize);
}

const std::vector<C2InterfaceHelper::ParamRef>
C2InterfaceHelper::ParamHelper::getDependenciesAsRefs() const {
    return mImpl->getDependenciesAsRefs();
}

std::shared_ptr<const C2ParamDescriptor>
C2InterfaceHelper::ParamHelper::getDescriptor() const {
    return mImpl->getDescriptor();
}

const std::vector<C2Param::Index> C2InterfaceHelper::ParamHelper::getDownDependencies() const {
    return mImpl->getDownDependencies();
}

C2Param::Index C2InterfaceHelper::ParamHelper::index() const {
    return mImpl->index();
}

C2String C2InterfaceHelper::ParamHelper::name() const {
    return mImpl->name();
}

const C2InterfaceHelper::ParamRef C2InterfaceHelper::ParamHelper::ref() const {
    return mImpl->ref();
}

C2StructDescriptor C2InterfaceHelper::ParamHelper::retrieveStructDescriptor() {
    return mImpl->retrieveStructDescriptor();
}

void C2InterfaceHelper::ParamHelper::setDefaultValue(std::shared_ptr<C2Param> default_) {
    mImpl->setDefaultValue(default_);
}

void C2InterfaceHelper::ParamHelper::setDependencies(
        std::vector<C2Param::Index> indices, std::vector<ParamRef> refs) {
    mImpl->setDependencies(indices, refs);
}

void C2InterfaceHelper::ParamHelper::setFields(std::vector<C2ParamFieldValues> &&fields) {
    return mImpl->setFields(std::move(fields));
}

void C2InterfaceHelper::ParamHelper::setGetter(
        std::function<std::shared_ptr<C2Param>(bool)> getter) {
    mImpl->setGetter(getter);
}

void C2InterfaceHelper::ParamHelper::setSetter(
        std::function<C2R(const C2Param *, bool, bool *, Factory &)> setter) {
    mImpl->setSetter(setter);
}

c2_status_t C2InterfaceHelper::ParamHelper::trySet(
        const C2Param *value, bool mayBlock, bool *changed, Factory &f,
        std::vector<std::unique_ptr<C2SettingResult>>* const failures) {
    return mImpl->trySet(value, mayBlock, changed, f, failures);
}

c2_status_t C2InterfaceHelper::ParamHelper::validate(
        const std::shared_ptr<C2ParamReflector> &reflector) {
    return mImpl->validate(reflector);
}

std::shared_ptr<C2Param> C2InterfaceHelper::ParamHelper::value() {
    return mImpl->value();
}

std::shared_ptr<const C2Param> C2InterfaceHelper::ParamHelper::value() const {
    return mImpl->value();
}

/* ---------------------------- FieldHelper ---------------------------- */

C2ParamField C2InterfaceHelper::FieldHelper::makeParamField(C2Param::Index index) const {
    return _C2ParamInspector::CreateParamField(index, mFieldId);
}

C2InterfaceHelper::FieldHelper::FieldHelper(const ParamRef &param, const _C2FieldId &field,
            std::unique_ptr<C2FieldSupportedValues> &&values)
    : mParam(param),
      mFieldId(field),
      mPossible(std::move(values)) {
    C2_LOG(VERBOSE) << "Creating field helper " << field << " "
            << C2FieldSupportedValuesHelper<uint32_t>(*mPossible);
}

void C2InterfaceHelper::FieldHelper::setSupportedValues(
        std::unique_ptr<C2FieldSupportedValues> &&values) {
    mSupported = std::move(values);
}

const C2FieldSupportedValues *C2InterfaceHelper::FieldHelper::getSupportedValues() const {
    return (mSupported ? mSupported : mPossible).get();
}

const C2FieldSupportedValues *C2InterfaceHelper::FieldHelper::getPossibleValues() const {
    return mPossible.get();
}


/* ---------------------------- Field ---------------------------- */

/**
 * Wrapper around field-supported-values builder that gets stored in the
 * field helper when the builder goes out of scope.
 */
template<typename T>
struct SupportedValuesBuilder : C2ParamFieldValuesBuilder<T> {
    SupportedValuesBuilder(
            C2ParamField &field, std::shared_ptr<C2InterfaceHelper::FieldHelper> helper)
        : C2ParamFieldValuesBuilder<T>(field), _mHelper(helper), _mField(field) {
    }

    /**
     * Save builder values on destruction.
     */
    virtual ~SupportedValuesBuilder() override {
        _mHelper->setSupportedValues(std::move(C2ParamFieldValues(*this).values));
    }

private:
    std::shared_ptr<C2InterfaceHelper::FieldHelper> _mHelper;
    C2ParamField _mField;
};


template<typename T>
C2ParamFieldValuesBuilder<T> C2InterfaceHelper::Field<T>::shouldBe() const {
    return C2ParamFieldValuesBuilder<T>(_mField);
}

template<typename T>
C2ParamFieldValuesBuilder<T> C2InterfaceHelper::Field<T>::mustBe() {
    return SupportedValuesBuilder<T>(_mField, _mHelper);
}

/*
template<typename T> C2SettingResultsBuilder C2InterfaceHelper::Field<T>::validatePossible(T &value)
const {
    /// TODO
    return C2SettingResultsBuilder::Ok();
}
*/

template<typename T>
C2InterfaceHelper::Field<T>::Field(std::shared_ptr<FieldHelper> helper, C2Param::Index index)
    : _mHelper(helper), _mField(helper->makeParamField(index)) { }

template struct C2InterfaceHelper::Field<uint8_t>;
template struct C2InterfaceHelper::Field<char>;
template struct C2InterfaceHelper::Field<int32_t>;
template struct C2InterfaceHelper::Field<uint32_t>;
//template struct C2InterfaceHelper::Field<c2_cntr32_t>;
template struct C2InterfaceHelper::Field<int64_t>;
template struct C2InterfaceHelper::Field<uint64_t>;
//template struct C2InterfaceHelper::Field<c2_cntr64_t>;
template struct C2InterfaceHelper::Field<float>;

/* --------------------------------- Factory --------------------------------- */

struct C2InterfaceHelper::FactoryImpl : public C2InterfaceHelper::Factory {
    virtual std::shared_ptr<C2ParamReflector> getReflector() const override {
        return _mReflector;
    }

    virtual std::shared_ptr<ParamHelper>
    getParamHelper(const ParamRef &param) const override {
        return _mParams.find(param)->second;
    }

public:
    FactoryImpl(std::shared_ptr<C2ParamReflector> reflector)
        : _mReflector(reflector) { }

    virtual ~FactoryImpl() = default;

    void addParam(std::shared_ptr<ParamHelper> param) {
        _mParams.insert({ param->ref(), param });
        _mIndexToHelper.insert({param->index(), param});

        // add down-dependencies (and validate dependencies as a result)
        size_t ix = 0;
        for (const ParamRef &ref : param->getDependenciesAsRefs()) {
            // dependencies must already be defined
            if (!_mParams.count(ref)) {
                C2_LOG(FATAL) << "Parameter " << param->name() << " has a dependency at index "
                        << ix << " that is not yet defined";
            }
            _mParams.find(ref)->second->addDownDependency(param->index());
            ++ix;
        }

        _mDependencyIndex.emplace(param->index(), _mDependencyIndex.size());
    }

    std::shared_ptr<ParamHelper> getParam(C2Param::Index ix) const {
        // TODO: handle streams separately
        const auto it = _mIndexToHelper.find(ix);
        if (it == _mIndexToHelper.end()) {
            return nullptr;
        }
        return it->second;
    }

    /**
     * TODO: this could return a copy using proper pointer cast.
     */
    std::shared_ptr<C2Param> getParamValue(C2Param::Index ix) const {
        std::shared_ptr<ParamHelper> helper = getParam(ix);
        return helper ? helper->value() : nullptr;
    }

    c2_status_t querySupportedParams(
            std::vector<std::shared_ptr<C2ParamDescriptor>> *const params) const {
        for (const auto &it : _mParams) {
            // TODO: change querySupportedParams signature?
            params->push_back(
                    std::const_pointer_cast<C2ParamDescriptor>(it.second->getDescriptor()));
        }
        // TODO: handle errors
        return C2_OK;
    }

    size_t getDependencyIndex(C2Param::Index ix) {
        // in this version of the helper there is only a single stream so
        // we can look up directly by index
        auto it = _mDependencyIndex.find(ix);
        return it == _mDependencyIndex.end() ? SIZE_MAX : it->second;
    }

private:
    std::map<ParamRef, std::shared_ptr<ParamHelper>> _mParams;
    std::map<C2Param::Index, std::shared_ptr<ParamHelper>> _mIndexToHelper;
    std::shared_ptr<C2ParamReflector> _mReflector;
    std::map<C2Param::Index, size_t> _mDependencyIndex;
};

/* --------------------------------- Helper --------------------------------- */

namespace {

static std::string asString(C2Param *p) {
    char addr[20];
    sprintf(addr, "%p:[", p);
    std::string v = addr;
    for (size_t i = 0; i < p->size(); ++i) {
        char d[4];
        sprintf(d, " %02x", *(((uint8_t *)p) + i));
        v += d + (i == 0);
    }
    return v + "]";
}

}

C2InterfaceHelper::C2InterfaceHelper(std::shared_ptr<C2ReflectorHelper> reflector)
    : mReflector(reflector),
      _mFactory(std::make_shared<FactoryImpl>(reflector)) { }


size_t C2InterfaceHelper::GetBaseOffset(const std::shared_ptr<C2ParamReflector> &reflector,
        C2Param::CoreIndex index, size_t offset) {
    std::unique_ptr<C2StructDescriptor> param = reflector->describe(index);
    if (param == nullptr) {
        return ~(size_t)0; // param structure not described
    }

    for (const C2FieldDescriptor &field : *param) {
        size_t fieldOffset = _C2ParamInspector::GetOffset(field);
        size_t fieldSize = _C2ParamInspector::GetSize(field);
        size_t fieldExtent = field.extent();
        if (offset < fieldOffset) {
            return ~(size_t)0; // not found
        }
        if (offset == fieldOffset) {
            // exact match
            return offset;
        }
        if (field.extent() == 0 || offset < fieldOffset + fieldSize * fieldExtent) {
            // reduce to first element in case of array
            offset = fieldOffset + (offset - fieldOffset) % fieldSize;
            if (field.type() >= C2FieldDescriptor::STRUCT_FLAG) {
                // this offset is within a field
                offset = GetBaseOffset(
                        reflector, field.type() & ~C2FieldDescriptor::STRUCT_FLAG,
                        offset - fieldOffset);
                return ~offset ? fieldOffset + offset : offset;
            }
        }
    }
    return ~(size_t)0; // not found
}

void C2InterfaceHelper::addParameter(std::shared_ptr<ParamHelper> param) {
    std::lock_guard<std::mutex> lock(mMutex);
    mReflector->addStructDescriptor(param->retrieveStructDescriptor());
    c2_status_t err = param->validate(mReflector);
    if (err != C2_CORRUPTED) {
        _mFactory->addParam(param);

        // run setter to ensure correct values
        bool changed = false;
        std::vector<std::unique_ptr<C2SettingResult>> failures;
        (void)param->trySet(param->value().get(), C2_MAY_BLOCK, &changed, *_mFactory, &failures);
    }
}

c2_status_t C2InterfaceHelper::config(
       const std::vector<C2Param*> &params, c2_blocking_t mayBlock,
       std::vector<std::unique_ptr<C2SettingResult>>* const failures, bool updateParams,
       std::vector<std::shared_ptr<C2Param>> *changes __unused /* TODO */) {
    std::lock_guard<std::mutex> lock(mMutex);
    bool paramWasInvalid = false; // TODO is this the same as bad value?
    bool paramNotFound = false;
    bool paramBadValue = false;
    bool paramNoMemory = false;
    bool paramBlocking = false;
    bool paramTimedOut = false;
    bool paramCorrupted = false;

    // dependencies
    // down dependencies are marked dirty, but params set are not immediately
    // marked dirty (unless they become down dependency) so that we can
    // avoid setting them if they did not change

    // TODO: there could be multiple indices for the same dependency index
    // { depIx, paramIx } may be a suitable key
    std::map<size_t, std::pair<C2Param::Index, bool>> dependencies;

    std::vector<std::unique_ptr<C2Param>> paramRequests;
    std::vector<C2Param*> lateReadParams;

    // we cannot determine the last valid parameter, so add an extra
    // loop iteration after the last parameter
    for (size_t p_ix = 0; p_ix <= params.size(); ++p_ix) {
        C2Param *p = nullptr;
        C2Param::Index paramIx = 0u;
        size_t paramDepIx = SIZE_MAX;
        bool last = p_ix == params.size();
        if (!last) {
            p = params[p_ix];
            if (!*p) {
                paramWasInvalid = true;
                p->invalidate();
                continue;
            }

            paramIx = p->index();

            // convert parameter to request in case this is a split parameter
            C2Param::Index requestParamIx = paramIx | C2Param::CoreIndex::IS_REQUEST_FLAG;

            // setting a request directly is handled as normal
            if (paramIx != requestParamIx) {
                paramDepIx = getDependencyIndex_l(requestParamIx);
                if (paramDepIx == SIZE_MAX) {
                    // not a split parameter, handle it normally
                    paramDepIx = getDependencyIndex_l(paramIx);
                } else {
                    // split parameter - replace with setting for the request - and queue to
                    // read back actual value
                    // TODO: read late params at the right time
                    lateReadParams.emplace_back(p);
                    std::unique_ptr<C2Param> request(C2Param::CopyAsRequest(*p));
                    p = request.get();
                    paramRequests.emplace_back(std::move(request));
                }
            }

            if (paramDepIx == SIZE_MAX) {
                // unsupported parameter
                paramNotFound = true;
                continue;
            }

            //
            // first insert - mark not dirty
            // it may have been marked dirty by a dependency update
            // this does not overrwrite(!)
            (void)dependencies.insert({ paramDepIx, { paramIx, false /* dirty */ }});
            auto it = dependencies.find(paramDepIx);
            C2_LOG(VERBOSE) << "marking dependency for setting at #" << paramDepIx << ": "
                    << it->second.first << ", update "
                    << (it->second.second ? "always (dirty)" : "only if changed");
        } else {
            // process any remaining dependencies
            if (dependencies.empty()) {
                continue;
            }
            C2_LOG(VERBOSE) << "handling dirty down dependencies after last setting";
        }

        // process any dirtied down-dependencies until the next param
        while (dependencies.size() && dependencies.begin()->first <= paramDepIx) {
            auto min = dependencies.begin();
            C2Param::Index ix = min->second.first;
            bool dirty = min->second.second;
            dependencies.erase(min);

            std::shared_ptr<ParamHelper> param = _mFactory->getParam(ix);
            C2_LOG(VERBOSE) << "old value " << asString(param->value().get());
            if (!last) {
                C2_LOG(VERBOSE) << "new value " << asString(p);
            }
            if (!last && !dirty && ix == paramIx && *param->value() == *p) {
                // no change in value - and dependencies were not updated
                // no need to update
                C2_LOG(VERBOSE) << "ignoring setting unchanged param " << ix;
                continue;
            }

            // apply setting
            bool changed = false;
            C2_LOG(VERBOSE) << "setting param " << ix;
            std::shared_ptr<C2Param> oldValue = param->value();
            c2_status_t res = param->trySet(
                    (!last && paramIx == ix) ? p : param->value().get(), mayBlock,
                    &changed, *_mFactory, failures);
            std::shared_ptr<C2Param> newValue = param->value();
            C2_CHECK_EQ(oldValue == newValue, *oldValue == *newValue);
            switch (res) {
                case C2_OK: break;
                case C2_BAD_VALUE: paramBadValue = true; break;
                case C2_NO_MEMORY: paramNoMemory = true; break;
                case C2_TIMED_OUT: paramTimedOut = true; break;
                case C2_BLOCKING:  paramBlocking = true; break;
                case C2_CORRUPTED: paramCorrupted = true; break;
                default: ;// TODO fatal
            }

            // copy back result for configured values (or invalidate if it does not fit or match)
            if (updateParams && !last && paramIx == ix) {
                if (!p->updateFrom(*param->value())) {
                    p->invalidate();
                }
            }

            // compare ptrs as params are copy on write
            if (changed) {
                C2_LOG(VERBOSE) << "param " << ix << " value changed";
                // value changed update down-dependencies and mark them dirty
                for (const C2Param::Index ix : param->getDownDependencies()) {
                    C2_LOG(VERBOSE) << 1;
                    auto insert_res = dependencies.insert(
                            { getDependencyIndex_l(ix), { ix, true /* dirty */ }});
                    if (!insert_res.second) {
                        (*insert_res.first).second.second = true; // mark dirty
                    }

                    auto it = dependencies.find(getDependencyIndex_l(ix));
                    C2_CHECK(it->second.second);
                    C2_LOG(VERBOSE) << "marking down dependencies to update at #"
                            << getDependencyIndex_l(ix) << ": " << it->second.first;
                }
            }
        }
    }

    // get late read parameters
    for (C2Param *p : lateReadParams) {
        std::shared_ptr<C2Param> value = _mFactory->getParamValue(p->index());
        if (value) {
            p->updateFrom(*value);
        } else {
            p->invalidate();
        }
    }

    return (paramCorrupted ? C2_CORRUPTED :
            paramBlocking ? C2_BLOCKING :
            paramTimedOut ? C2_TIMED_OUT :
            paramNoMemory ? C2_NO_MEMORY :
            (paramBadValue || paramWasInvalid) ? C2_BAD_VALUE :
            paramNotFound ? C2_BAD_INDEX : C2_OK);
}

size_t C2InterfaceHelper::getDependencyIndex_l(C2Param::Index ix) const {
    return _mFactory->getDependencyIndex(ix);
}

c2_status_t C2InterfaceHelper::query(
        const std::vector<C2Param*> &stackParams,
        const std::vector<C2Param::Index> &heapParamIndices,
        c2_blocking_t mayBlock __unused /* TODO */,
        std::vector<std::unique_ptr<C2Param>>* const heapParams) const {
    std::lock_guard<std::mutex> lock(mMutex);
    bool paramWasInvalid = false;
    bool paramNotFound = false;
    bool paramDidNotFit = false;
    bool paramNoMemory = false;

    for (C2Param* const p : stackParams) {
        if (!*p) {
            paramWasInvalid = true;
            p->invalidate();
        } else {
            std::shared_ptr<C2Param> value = _mFactory->getParamValue(p->index());
            if (!value) {
                paramNotFound = true;
                p->invalidate();
            } else if (!p->updateFrom(*value)) {
                paramDidNotFit = true;
                p->invalidate();
            }
        }
    }

    for (const C2Param::Index ix : heapParamIndices) {
        std::shared_ptr<C2Param> value = _mFactory->getParamValue(ix);
        if (value) {
            std::unique_ptr<C2Param> p = C2Param::Copy(*value);
            if (p != nullptr) {
                heapParams->push_back(std::move(p));
            } else {
                heapParams->push_back(nullptr);
                paramNoMemory = true;
            }
        } else {
            heapParams->push_back(nullptr);
            paramNotFound = true;
        }
    }

    return paramNoMemory ? C2_NO_MEMORY :
           paramNotFound ? C2_BAD_INDEX :
           // the following errors are not marked in the return value
           paramDidNotFit ? C2_OK :
           paramWasInvalid ? C2_OK : C2_OK;
}

c2_status_t C2InterfaceHelper::querySupportedParams(
        std::vector<std::shared_ptr<C2ParamDescriptor>> *const params) const {
    std::lock_guard<std::mutex> lock(mMutex);
    return _mFactory->querySupportedParams(params);
}


c2_status_t C2InterfaceHelper::querySupportedValues(
        std::vector<C2FieldSupportedValuesQuery> &fields, c2_blocking_t mayBlock __unused) const {
    std::lock_guard<std::mutex> lock(mMutex);
    for (C2FieldSupportedValuesQuery &query : fields) {
        C2_LOG(VERBOSE) << "querying field " << query.field();
        C2Param::Index ix = _C2ParamInspector::GetIndex(query.field());
        std::shared_ptr<ParamHelper> param = _mFactory->getParam(ix);
        if (!param) {
            C2_LOG(VERBOSE) << "bad param";
            query.status = C2_BAD_INDEX;
            continue;
        }
        size_t offs = GetBaseOffset(
                mReflector, ix,
                _C2ParamInspector::GetOffset(query.field()) - sizeof(C2Param));
        if (~offs == 0) {
            C2_LOG(VERBOSE) << "field could not be found";
            query.status = C2_NOT_FOUND;
            continue;
        }
        offs += sizeof(C2Param);
        C2_LOG(VERBOSE) << "field resolved to "
                << StringPrintf("@%02zx+%02x", offs, _C2ParamInspector::GetSize(query.field()));
        std::shared_ptr<FieldHelper> field =
            param->findField(offs, _C2ParamInspector::GetSize(query.field()));
        if (!field) {
            C2_LOG(VERBOSE) << "bad field";
            query.status = C2_NOT_FOUND;
            continue;
        }

        const C2FieldSupportedValues *values = nullptr;
        switch (query.type()) {
        case C2FieldSupportedValuesQuery::CURRENT:
            values = field->getSupportedValues();
            break;
        case C2FieldSupportedValuesQuery::POSSIBLE:
            values = field->getPossibleValues();
            break;
        default:
            C2_LOG(VERBOSE) << "bad query type: " << query.type();
            query.status = C2_BAD_VALUE;
        }
        if (values) {
            query.values = *values;
            query.status = C2_OK;
        } else {
            C2_LOG(DEBUG) << "no values published by component";
            query.status = C2_CORRUPTED;
        }
    }
    return C2_OK;
}

std::unique_lock<std::mutex> C2InterfaceHelper::lock() const {
    return std::unique_lock<std::mutex>(mMutex);
}
