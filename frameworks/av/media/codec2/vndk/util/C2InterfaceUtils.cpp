/*
 * Copyright (C) 2017 The Android Open Source Project
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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-value"

#define C2_LOG_VERBOSE

#include <C2Debug.h>
#include <C2Param.h>
#include <C2ParamDef.h>
#include <C2ParamInternal.h>
#include <util/C2InterfaceUtils.h>

#include <cmath>
#include <limits>
#include <map>
#include <type_traits>

#include <android-base/stringprintf.h>

std::ostream& operator<<(std::ostream& os, const _C2FieldId &i);

std::ostream& operator<<(std::ostream& os, const C2ParamField &i);

/* ---------------------------- C2SupportedRange ---------------------------- */

/**
 * Helper class for supported values range calculations.
 */
template<typename T, bool FP=std::is_floating_point<T>::value>
struct _C2TypedSupportedRangeHelper {
    /**
     * type of range size: a - b if a >= b and a and b are of type T
     */
    typedef typename std::make_unsigned<T>::type DiffType;

    /**
     * calculate (high - low) mod step
     */
    static DiffType mod(T low, T high, T step) {
        return DiffType(high - low) % DiffType(step);
    }
};

template<typename T>
struct _C2TypedSupportedRangeHelper<T, true> {
    typedef T DiffType;

    static DiffType mod(T low, T high, T step) {
        return fmod(high - low, step);
    }
};

template<typename T>
C2SupportedRange<T>::C2SupportedRange(const C2FieldSupportedValues &values) {
    if (values.type == C2FieldSupportedValues::RANGE) {
        _mMin = values.range.min.ref<ValueType>();
        _mMax = values.range.max.ref<ValueType>();
        _mStep = values.range.step.ref<ValueType>();
        _mNum = values.range.num.ref<ValueType>();
        _mDenom = values.range.denom.ref<ValueType>();
    } else {
        _mMin = MAX_VALUE;
        _mMax = MIN_VALUE;
        _mStep = MIN_STEP;
        _mNum = 0;
        _mDenom = 0;
    }
}

template<typename T>
bool C2SupportedRange<T>::contains(T value) const {
    // value must fall between min and max
    if (value < _mMin || value > _mMax) {
        return false;
    }
    // simple ranges contain all values between min and max
    if (isSimpleRange()) {
        return true;
    }
    // min is always part of the range
    if (value == _mMin) {
        return true;
    }
    // stepped ranges require (val - min) % step to be zero
    if (isArithmeticSeries()) {
        return _C2TypedSupportedRangeHelper<T>::mod(_mMin, value, _mStep) == 0;
    }
    // pure geometric series require (val / min) to be integer multiple of (num/denom)
    if (isGeometricSeries()) {
        if (value <= 0) {
            return false;
        }
        double log2base = log2(_mNum / _mDenom);
        double power = llround(log2(value / double(_mMin)) / log2base);
        // TODO: validate that result falls within precision (other than round)
        return value == T(_mMin * pow(_mNum / _mDenom, power) + MIN_STEP / 2);
    }
    // multiply-accumulate series require validating by walking through the series
    if (isMacSeries()) {
        double lastValue = _mMin;
        double base = _mNum / _mDenom;
        while (true) {
            // this cast is safe as _mMin <= lastValue <= _mMax
            if (T(lastValue + MIN_STEP / 2) == value) {
                return true;
            }
            double nextValue = fma(lastValue, base, _mStep);
            if (nextValue <= lastValue || nextValue > _mMax) {
                return false; // series is no longer monotonic or within range
            }
            lastValue = nextValue;
        };
    }
    // if we are here, this must be an invalid range
    return false;
}

template<typename T>
C2SupportedRange<T> C2SupportedRange<T>::limitedTo(const C2SupportedRange<T> &limit) const {
    // TODO - this only works for simple ranges
    return C2SupportedRange(std::max(_mMin, limit._mMin), std::min(_mMax, limit._mMax),
                                 std::max(_mStep, limit._mStep));
}

template class C2SupportedRange<uint8_t>;
template class C2SupportedRange<char>;
template class C2SupportedRange<int32_t>;
template class C2SupportedRange<uint32_t>;
//template class C2SupportedRange<c2_cntr32_t>;
template class C2SupportedRange<int64_t>;
template class C2SupportedRange<uint64_t>;
//template class C2SupportedRange<c2_cntr64_t>;
template class C2SupportedRange<float>;

/* -------------------------- C2SupportedFlags -------------------------- */

/**
 * Ordered supported flag set for a field of a given type.
 */
// float flags are not supported, but define a few methods to support generic supported values code
template<>
bool C2SupportedFlags<float>::contains(float value) const {
    return false;
}

template<>
const std::vector<float> C2SupportedFlags<float>::flags() const {
    return std::vector<float>();
}

template<>
C2SupportedFlags<float> C2SupportedFlags<float>::limitedTo(const C2SupportedFlags<float> &limit) const {
    std::vector<C2Value::Primitive> values;
    return C2SupportedFlags(std::move(values));
}

template<>
float C2SupportedFlags<float>::min() const {
    return 0;
}

template<typename T>
bool C2SupportedFlags<T>::contains(T value) const {
    // value must contain the minimal mask
    T minMask = min();
    if (~value & minMask) {
        return false;
    }
    value &= ~minMask;
    // otherwise, remove flags from value and see if we arrive at 0
    for (const C2Value::Primitive &v : _mValues) {
        if (value == 0) {
            break;
        }
        if ((~value & v.ref<ValueType>()) == 0) {
            value &= ~v.ref<ValueType>();
        }
    }
    return value == 0;
}

template<typename T>
const std::vector<T> C2SupportedFlags<T>::flags() const {
    std::vector<T> vals(c2_max(_mValues.size(), 1u) - 1);
    if (!_mValues.empty()) {
        std::transform(_mValues.cbegin() + 1, _mValues.cend(), vals.begin(),
                       [](const C2Value::Primitive &p)->T {
            return p.ref<ValueType>();
        });
    }
    return vals;
}

template<typename T>
C2SupportedFlags<T> C2SupportedFlags<T>::limitedTo(const C2SupportedFlags<T> &limit) const {
    std::vector<C2Value::Primitive> values = _mValues; // make a copy
    T minMask = min() | limit.min();
    // minimum mask must be covered by both this and other
    if (limit.contains(minMask) && contains(minMask)) {
        values[0] = minMask;
        // keep only flags that are covered by limit
        std::remove_if(values.begin(), values.end(), [&limit, minMask](const C2Value::Primitive &v) -> bool {
            T value = v.ref<ValueType>() | minMask;
            return value == minMask || !limit.contains(value); });
        // we also need to do it vice versa
        for (const C2Value::Primitive &v : _mValues) {
            T value = v.ref<ValueType>() | minMask;
            if (value != minMask && contains(value)) {
                values.emplace_back((ValueType)value);
            }
        }
    }
    return C2SupportedFlags(std::move(values));
}

template<typename T>
T C2SupportedFlags<T>::min() const {
    if (!_mValues.empty()) {
        return _mValues.front().template ref<ValueType>();
    } else {
        return T(0);
    }
}

template class C2SupportedFlags<uint8_t>;
template class C2SupportedFlags<char>;
template class C2SupportedFlags<int32_t>;
template class C2SupportedFlags<uint32_t>;
//template class C2SupportedFlags<c2_cntr32_t>;
template class C2SupportedFlags<int64_t>;
template class C2SupportedFlags<uint64_t>;
//template class C2SupportedFlags<c2_cntr64_t>;

/* -------------------------- C2SupportedValueSet -------------------------- */

/**
 * Ordered supported value set for a field of a given type.
 */
template<typename T>
bool C2SupportedValueSet<T>::contains(T value) const {
    return std::find_if(_mValues.cbegin(), _mValues.cend(),
            [value](const C2Value::Primitive &p) -> bool {
                return value == p.ref<ValueType>();
            }) != _mValues.cend();
}

template<typename T>
C2SupportedValueSet<T> C2SupportedValueSet<T>::limitedTo(const C2SupportedValueSet<T> &limit) const {
    std::vector<C2Value::Primitive> values = _mValues; // make a copy
    std::remove_if(values.begin(), values.end(), [&limit](const C2Value::Primitive &v) -> bool {
        return !limit.contains(v.ref<ValueType>()); });
    return C2SupportedValueSet(std::move(values));
}

template<typename T>
C2SupportedValueSet<T> C2SupportedValueSet<T>::limitedTo(const C2SupportedRange<T> &limit) const {
    std::vector<C2Value::Primitive> values = _mValues; // make a copy
    std::remove_if(values.begin(), values.end(), [&limit](const C2Value::Primitive &v) -> bool {
        return !limit.contains(v.ref<ValueType>()); });
    return C2SupportedValueSet(std::move(values));
}

template<typename T>
C2SupportedValueSet<T> C2SupportedValueSet<T>::limitedTo(const C2SupportedFlags<T> &limit) const {
    std::vector<C2Value::Primitive> values = _mValues; // make a copy
    std::remove_if(values.begin(), values.end(), [&limit](const C2Value::Primitive &v) -> bool {
        return !limit.contains(v.ref<ValueType>()); });
    return C2SupportedValueSet(std::move(values));
}

template<typename T>
const std::vector<T> C2SupportedValueSet<T>::values() const {
    std::vector<T> vals(_mValues.size());
    std::transform(_mValues.cbegin(), _mValues.cend(), vals.begin(), [](const C2Value::Primitive &p) -> T {
        return p.ref<ValueType>();
    });
    return vals;
}

template class C2SupportedValueSet<uint8_t>;
template class C2SupportedValueSet<char>;
template class C2SupportedValueSet<int32_t>;
template class C2SupportedValueSet<uint32_t>;
//template class C2SupportedValueSet<c2_cntr32_t>;
template class C2SupportedValueSet<int64_t>;
template class C2SupportedValueSet<uint64_t>;
//template class C2SupportedValueSet<c2_cntr64_t>;
template class C2SupportedValueSet<float>;

/* ---------------------- C2FieldSupportedValuesHelper ---------------------- */

template<typename T>
struct C2FieldSupportedValuesHelper<T>::Impl {
    Impl(const C2FieldSupportedValues &values)
        : _mType(values.type),
          _mRange(values),
          _mValues(values),
          _mFlags(values) { }

    bool supports(T value) const;

private:
    typedef typename _C2FieldValueHelper<T>::ValueType ValueType;
    C2FieldSupportedValues::type_t _mType;
    C2SupportedRange<ValueType> _mRange;
    C2SupportedValueSet<ValueType> _mValues;
    C2SupportedValueSet<ValueType> _mFlags;

//    friend std::ostream& operator<< <T>(std::ostream& os, const C2FieldSupportedValuesHelper<T>::Impl &i);
//    friend std::ostream& operator<<(std::ostream& os, const Impl &i);
    std::ostream& streamOut(std::ostream& os) const;
};

template<typename T>
bool C2FieldSupportedValuesHelper<T>::Impl::supports(T value) const {
    switch (_mType) {
        case C2FieldSupportedValues::RANGE: return _mRange.contains(value);
        case C2FieldSupportedValues::VALUES: return _mValues.contains(value);
        case C2FieldSupportedValues::FLAGS: return _mFlags.contains(value);
        default: return false;
    }
}

template<typename T>
C2FieldSupportedValuesHelper<T>::C2FieldSupportedValuesHelper(const C2FieldSupportedValues &values)
    : _mImpl(std::make_unique<C2FieldSupportedValuesHelper<T>::Impl>(values)) { }

template<typename T>
C2FieldSupportedValuesHelper<T>::~C2FieldSupportedValuesHelper() = default;

template<typename T>
bool C2FieldSupportedValuesHelper<T>::supports(T value) const {
    return _mImpl->supports(value);
}

template class C2FieldSupportedValuesHelper<uint8_t>;
template class C2FieldSupportedValuesHelper<char>;
template class C2FieldSupportedValuesHelper<int32_t>;
template class C2FieldSupportedValuesHelper<uint32_t>;
//template class C2FieldSupportedValuesHelper<c2_cntr32_t>;
template class C2FieldSupportedValuesHelper<int64_t>;
template class C2FieldSupportedValuesHelper<uint64_t>;
//template class C2FieldSupportedValuesHelper<c2_cntr64_t>;
template class C2FieldSupportedValuesHelper<float>;

/* ----------------------- C2ParamFieldValuesBuilder ----------------------- */

template<typename T>
struct C2ParamFieldValuesBuilder<T>::Impl {
    Impl(const C2ParamField &field)
        : _mParamField(field),
          _mType(type_t::RANGE),
          _mDefined(false),
          _mRange(C2SupportedRange<T>::Any()),
          _mValues(C2SupportedValueSet<T>::None()),
          _mFlags(C2SupportedFlags<T>::None()) { }

    /**
     * Get C2ParamFieldValues from this builder.
     */
    operator C2ParamFieldValues() const {
        if (!_mDefined) {
            return C2ParamFieldValues(_mParamField);
        }
        switch (_mType) {
        case type_t::EMPTY:
        case type_t::VALUES:
            return C2ParamFieldValues(_mParamField, (C2FieldSupportedValues)_mValues);
        case type_t::RANGE:
            return C2ParamFieldValues(_mParamField, (C2FieldSupportedValues)_mRange);
        case type_t::FLAGS:
            return C2ParamFieldValues(_mParamField, (C2FieldSupportedValues)_mFlags);
        default:
            // TRESPASS
            // should never get here
            return C2ParamFieldValues(_mParamField);
        }
    }

    /** Define the supported values as the currently supported values of this builder. */
    void any() {
        _mDefined = true;
    }

    /** Restrict (and thus define) the supported values to none. */
    void none() {
        _mDefined = true;
        _mType = type_t::VALUES;
        _mValues.clear();
    }

    /** Restrict (and thus define) the supported values to |value| alone. */
    void equalTo(T value) {
         return limitTo(C2SupportedValueSet<T>::OneOf({value}));
    }

    /** Restrict (and thus define) the supported values to a value set. */
    void limitTo(const C2SupportedValueSet<T> &limit) {
        if (!_mDefined) {
            C2_LOG(VERBOSE) << "NA.limitTo(" << C2FieldSupportedValuesHelper<T>(limit) << ")";

            // shortcut for first limit applied
            _mDefined = true;
            _mValues = limit;
            _mType = _mValues.isEmpty() ? type_t::EMPTY : type_t::VALUES;
        } else {
            switch (_mType) {
            case type_t::EMPTY:
            case type_t::VALUES:
                C2_LOG(VERBOSE) << "(" << C2FieldSupportedValuesHelper<T>(_mValues) << ").limitTo("
                        << C2FieldSupportedValuesHelper<T>(limit) << ")";

                _mValues = _mValues.limitedTo(limit);
                _mType = _mValues.isEmpty() ? type_t::EMPTY : type_t::VALUES;
                break;
            case type_t::RANGE:
                C2_LOG(VERBOSE) << "(" << C2FieldSupportedValuesHelper<T>(_mRange) << ").limitTo("
                        << C2FieldSupportedValuesHelper<T>(limit) << ")";

                _mValues = limit.limitedTo(_mRange);
                _mType = _mValues.isEmpty() ? type_t::EMPTY : type_t::VALUES;
                break;
            case type_t::FLAGS:
                C2_LOG(VERBOSE) << "(" << C2FieldSupportedValuesHelper<T>(_mRange) << ").limitTo("
                        << C2FieldSupportedValuesHelper<T>(limit) << ")";

                _mValues = limit.limitedTo(_mFlags);
                _mType = _mValues.isEmpty() ? type_t::EMPTY : type_t::VALUES;
                break;
            default:
                C2_LOG(FATAL); // should not be here
            }
            // TODO: support flags
        }
        C2_LOG(VERBOSE) << " = " << _mType << ":" << C2FieldSupportedValuesHelper<T>(_mValues);
    }

    /** Restrict (and thus define) the supported values to a flag set. */
    void limitTo(const C2SupportedFlags<T> &limit) {
        if (!_mDefined) {
            C2_LOG(VERBOSE) << "NA.limitTo(" << C2FieldSupportedValuesHelper<T>(limit) << ")";

            // shortcut for first limit applied
            _mDefined = true;
            _mFlags = limit;
            _mType = _mFlags.isEmpty() ? type_t::EMPTY : type_t::FLAGS;
        } else {
            switch (_mType) {
            case type_t::EMPTY:
            case type_t::VALUES:
                C2_LOG(VERBOSE) << "(" << C2FieldSupportedValuesHelper<T>(_mValues) << ").limitTo("
                        << C2FieldSupportedValuesHelper<T>(limit) << ")";

                _mValues = _mValues.limitedTo(limit);
                _mType = _mValues.isEmpty() ? type_t::EMPTY : type_t::VALUES;
                C2_LOG(VERBOSE) << " = " << _mType << ":" << C2FieldSupportedValuesHelper<T>(_mValues);
                break;
            case type_t::FLAGS:
                C2_LOG(VERBOSE) << "(" << C2FieldSupportedValuesHelper<T>(_mFlags) << ").limitTo("
                        << C2FieldSupportedValuesHelper<T>(limit) << ")";

                _mFlags = _mFlags.limitedTo(limit);
                _mType = _mFlags.isEmpty() ? type_t::EMPTY : type_t::FLAGS;
                C2_LOG(VERBOSE) << " = " << _mType << ":" << C2FieldSupportedValuesHelper<T>(_mFlags);
                break;
            case type_t::RANGE:
                C2_LOG(FATAL) << "limiting ranges to flags is not supported";
                _mType = type_t::EMPTY;
                break;
            default:
                C2_LOG(FATAL); // should not be here
            }
        }
    }

    void limitTo(const C2SupportedRange<T> &limit) {
        if (!_mDefined) {
            C2_LOG(VERBOSE) << "NA.limitTo(" << C2FieldSupportedValuesHelper<T>(limit) << ")";

            // shortcut for first limit applied
            _mDefined = true;
            _mRange = limit;
            _mType = _mRange.isEmpty() ? type_t::EMPTY : type_t::RANGE;
            C2_LOG(VERBOSE) << " = " << _mType << ":" << C2FieldSupportedValuesHelper<T>(_mRange);
        } else {
            switch (_mType) {
            case type_t::EMPTY:
            case type_t::VALUES:
                C2_LOG(VERBOSE) << "(" << C2FieldSupportedValuesHelper<T>(_mValues) << ").limitTo("
                        << C2FieldSupportedValuesHelper<T>(limit) << ")";
                _mValues = _mValues.limitedTo(limit);
                _mType = _mValues.isEmpty() ? type_t::EMPTY : type_t::VALUES;
                C2_LOG(VERBOSE) << " = " << _mType << ":" << C2FieldSupportedValuesHelper<T>(_mValues);
                break;
            case type_t::FLAGS:
                C2_LOG(FATAL) << "limiting flags to ranges is not supported";
                _mType = type_t::EMPTY;
                break;
            case type_t::RANGE:
                C2_LOG(VERBOSE) << "(" << C2FieldSupportedValuesHelper<T>(_mRange) << ").limitTo("
                        << C2FieldSupportedValuesHelper<T>(limit) << ")";
                _mRange = _mRange.limitedTo(limit);
                C2_DCHECK(_mValues.isEmpty());
                _mType = _mRange.isEmpty() ? type_t::EMPTY : type_t::RANGE;
                C2_LOG(VERBOSE) << " = " << _mType << ":" << C2FieldSupportedValuesHelper<T>(_mRange);
                break;
            default:
                C2_LOG(FATAL); // should not be here
            }
        }
    }

private:
    void instantiate() __unused {
        (void)_mValues.values(); // instantiate non-const values()
    }

    void instantiate() const __unused {
        (void)_mValues.values(); // instantiate const values()
    }

    typedef C2FieldSupportedValues::type_t type_t;

    C2ParamField _mParamField;
    type_t _mType;
    bool _mDefined;
    C2SupportedRange<T> _mRange;
    C2SupportedValueSet<T> _mValues;
    C2SupportedFlags<T> _mFlags;

};

template<typename T>
C2ParamFieldValuesBuilder<T>::operator C2ParamFieldValues() const {
    return (C2ParamFieldValues)(*_mImpl.get());
}

template<typename T>
C2ParamFieldValuesBuilder<T>::C2ParamFieldValuesBuilder(const C2ParamField &field)
    : _mImpl(std::make_unique<C2ParamFieldValuesBuilder<T>::Impl>(field)) { }

template<typename T>
C2ParamFieldValuesBuilder<T> &C2ParamFieldValuesBuilder<T>::any() {
    _mImpl->any();
    return *this;
}

template<typename T>
C2ParamFieldValuesBuilder<T> &C2ParamFieldValuesBuilder<T>::none() {
    _mImpl->none();
    return *this;
}

template<typename T>
C2ParamFieldValuesBuilder<T> &C2ParamFieldValuesBuilder<T>::equalTo(T value) {
    _mImpl->equalTo(value);
    return *this;
}

template<typename T>
C2ParamFieldValuesBuilder<T> &C2ParamFieldValuesBuilder<T>::limitTo(const C2SupportedValueSet<T> &limit) {
    _mImpl->limitTo(limit);
    return *this;
}

template<typename T>
C2ParamFieldValuesBuilder<T> &C2ParamFieldValuesBuilder<T>::limitTo(const C2SupportedFlags<T> &limit) {
    _mImpl->limitTo(limit);
    return *this;
}

template<typename T>
C2ParamFieldValuesBuilder<T> &C2ParamFieldValuesBuilder<T>::limitTo(const C2SupportedRange<T> &limit) {
    _mImpl->limitTo(limit);
    return *this;
}

template<typename T>
C2ParamFieldValuesBuilder<T>::C2ParamFieldValuesBuilder(const C2ParamFieldValuesBuilder<T> &other)
    : _mImpl(std::make_unique<C2ParamFieldValuesBuilder<T>::Impl>(*other._mImpl.get())) { }

template<typename T>
C2ParamFieldValuesBuilder<T> &C2ParamFieldValuesBuilder<T>::operator=(
        const C2ParamFieldValuesBuilder<T> &other) {
    _mImpl = std::make_unique<C2ParamFieldValuesBuilder<T>::Impl>(*other._mImpl.get());
    return *this;
}

template<typename T>
C2ParamFieldValuesBuilder<T>::~C2ParamFieldValuesBuilder() = default;

template class C2ParamFieldValuesBuilder<uint8_t>;
template class C2ParamFieldValuesBuilder<char>;
template class C2ParamFieldValuesBuilder<int32_t>;
template class C2ParamFieldValuesBuilder<uint32_t>;
//template class C2ParamFieldValuesBuilder<c2_cntr32_t>;
template class C2ParamFieldValuesBuilder<int64_t>;
template class C2ParamFieldValuesBuilder<uint64_t>;
//template class C2ParamFieldValuesBuilder<c2_cntr64_t>;
template class C2ParamFieldValuesBuilder<float>;

/* ------------------------- C2SettingResultBuilder ------------------------- */

C2SettingConflictsBuilder::C2SettingConflictsBuilder() : _mConflicts() { }

C2SettingConflictsBuilder::C2SettingConflictsBuilder(C2ParamFieldValues &&conflict) {
    _mConflicts.emplace_back(std::move(conflict));
}

C2SettingConflictsBuilder& C2SettingConflictsBuilder::with(C2ParamFieldValues &&conflict) {
    _mConflicts.emplace_back(std::move(conflict));
    return *this;
}

std::vector<C2ParamFieldValues> C2SettingConflictsBuilder::retrieveConflicts() {
    return std::move(_mConflicts);
}

/* ------------------------- C2SettingResult/sBuilder ------------------------- */

C2SettingResult C2SettingResultBuilder::ReadOnly(const C2ParamField &param) {
    return C2SettingResult { C2SettingResult::READ_ONLY, { param }, { } };
}

C2SettingResult C2SettingResultBuilder::BadValue(const C2ParamField &paramField, bool isInfo) {
    return { isInfo ? C2SettingResult::INFO_BAD_VALUE : C2SettingResult::BAD_VALUE,
             { paramField }, { } };
}

C2SettingResult C2SettingResultBuilder::Conflict(
        C2ParamFieldValues &&paramFieldValues, C2SettingConflictsBuilder &conflicts, bool isInfo) {
    C2_CHECK(!conflicts.empty());
    if (isInfo) {
        return C2SettingResult {
            C2SettingResult::INFO_CONFLICT,
            std::move(paramFieldValues), conflicts.retrieveConflicts()
        };
    } else {
        return C2SettingResult {
            C2SettingResult::CONFLICT,
            std::move(paramFieldValues), conflicts.retrieveConflicts()
        };
    }
}

C2SettingResultsBuilder::C2SettingResultsBuilder(C2SettingResult &&result)
        : _mStatus(C2_BAD_VALUE) {
    _mResults.emplace_back(new C2SettingResult(std::move(result)));
}

C2SettingResultsBuilder C2SettingResultsBuilder::plus(C2SettingResultsBuilder&& results) {
    for (std::unique_ptr<C2SettingResult> &r : results._mResults) {
        _mResults.emplace_back(std::move(r));
    }
    results._mResults.clear();
    // TODO: mStatus
    return std::move(*this);
}

c2_status_t C2SettingResultsBuilder::retrieveFailures(
        std::vector<std::unique_ptr<C2SettingResult>>* const failures) {
    for (std::unique_ptr<C2SettingResult> &r : _mResults) {
        failures->emplace_back(std::move(r));
    }
    _mResults.clear();
    return _mStatus;
}

C2SettingResultsBuilder::C2SettingResultsBuilder(c2_status_t status) : _mStatus(status) {
    // status must be one of OK, BAD_STATE, TIMED_OUT or CORRUPTED
    // mainly: BLOCKING, BAD_INDEX, BAD_VALUE and NO_MEMORY requires a setting attempt
}

#pragma clang diagnostic pop

/* ------------------------- C2FieldUtils ------------------------- */

struct C2_HIDE C2FieldUtils::_Inspector {
    /// returns the implementation object
    inline static std::shared_ptr<Info::Impl> GetImpl(const Info &info) {
        return info._mImpl;
    }
};

/* ------------------------- C2FieldUtils::Info ------------------------- */

struct C2_HIDE C2FieldUtils::Info::Impl {
    C2FieldDescriptor field;
    std::shared_ptr<Impl> parent;
    uint32_t index;
    uint32_t depth;
    uint32_t baseFieldOffset;
    uint32_t arrayOffset;
    uint32_t usedExtent;

    /// creates a copy of this object including copies of its parent chain
    Impl clone() const;

    /// creates a copy of a shared pointer to an object
    static std::shared_ptr<Impl> Clone(const std::shared_ptr<Impl> &);

    Impl(const C2FieldDescriptor &field_, std::shared_ptr<Impl> parent_,
            uint32_t index_, uint32_t depth_, uint32_t baseFieldOffset_,
            uint32_t arrayOffset_, uint32_t usedExtent_)
        : field(field_), parent(parent_), index(index_), depth(depth_),
          baseFieldOffset(baseFieldOffset_), arrayOffset(arrayOffset_), usedExtent(usedExtent_) { }
};

std::shared_ptr<C2FieldUtils::Info::Impl> C2FieldUtils::Info::Impl::Clone(const std::shared_ptr<Impl> &info) {
    if (info) {
        return std::make_shared<Impl>(info->clone());
    }
    return nullptr;
}

C2FieldUtils::Info::Impl C2FieldUtils::Info::Impl::clone() const {
    Impl res = Impl(*this);
    res.parent = Clone(res.parent);
    return res;
}

C2FieldUtils::Info::Info(std::shared_ptr<Impl> impl)
    : _mImpl(impl) { }

size_t C2FieldUtils::Info::arrayOffset() const {
    return _mImpl->arrayOffset;
}

size_t C2FieldUtils::Info::arraySize() const {
    return extent() * size();
}

size_t C2FieldUtils::Info::baseFieldOffset() const {
    return _mImpl->baseFieldOffset;
};

size_t C2FieldUtils::Info::depth() const {
    return _mImpl->depth;
}

size_t C2FieldUtils::Info::extent() const {
    return _mImpl->usedExtent;
}

size_t C2FieldUtils::Info::index() const {
    return _mImpl->index;
}

bool C2FieldUtils::Info::isArithmetic() const {
    switch (_mImpl->field.type()) {
    case C2FieldDescriptor::BLOB:
    case C2FieldDescriptor::CNTR32:
    case C2FieldDescriptor::CNTR64:
    case C2FieldDescriptor::FLOAT:
    case C2FieldDescriptor::INT32:
    case C2FieldDescriptor::INT64:
    case C2FieldDescriptor::STRING:
    case C2FieldDescriptor::UINT32:
    case C2FieldDescriptor::UINT64:
        return true;
    default:
        return false;
    }
}

bool C2FieldUtils::Info::isFlexible() const {
    return _mImpl->field.extent() == 0;
}

C2String C2FieldUtils::Info::name() const {
    return _mImpl->field.name();
}

const C2FieldUtils::Info::NamedValuesType &C2FieldUtils::Info::namedValues() const {
    return _mImpl->field.namedValues();
}

size_t C2FieldUtils::Info::offset() const {
    return _C2ParamInspector::GetOffset(_mImpl->field);
}

C2FieldUtils::Info C2FieldUtils::Info::parent() const {
    return Info(_mImpl->parent);
};

size_t C2FieldUtils::Info::size() const {
    return _C2ParamInspector::GetSize(_mImpl->field);
}

C2FieldUtils::Info::type_t C2FieldUtils::Info::type() const {
    return _mImpl->field.type();
}

/* ------------------------- C2FieldUtils::Iterator ------------------------- */

struct C2_HIDE C2FieldUtils::Iterator::Impl : public _C2ParamInspector {
    Impl() = default;

    virtual ~Impl() = default;

    /// implements object equality
    virtual bool equals(const std::shared_ptr<Impl> &other) const {
        return other != nullptr && mHead == other->mHead;
    };

    /// returns the info pointed to by this iterator
    virtual value_type get() const {
        return Info(mHead);
    }

    /// increments this iterator
    virtual void increment() {
        // note: this cannot be abstract as we instantiate this for List::end(). increment to end()
        // instead.
        mHead.reset();
    }

protected:
    Impl(std::shared_ptr<C2FieldUtils::Info::Impl> head)
        : mHead(head) { }

    std::shared_ptr<Info::Impl> mHead; ///< current field
};

C2FieldUtils::Iterator::Iterator(std::shared_ptr<Impl> impl)
    : mImpl(impl) { }

C2FieldUtils::Iterator::value_type C2FieldUtils::Iterator::operator*() const {
    return mImpl->get();
}

C2FieldUtils::Iterator& C2FieldUtils::Iterator::operator++() {
    mImpl->increment();
    return *this;
}

bool C2FieldUtils::Iterator::operator==(const Iterator &other) const {
    return mImpl->equals(other.mImpl);
}

/* ------------------------- C2FieldUtils::List ------------------------- */

struct C2_HIDE C2FieldUtils::List::Impl {
    virtual std::shared_ptr<Iterator::Impl> begin() const = 0;

    /// returns an iterator to the end of the list
    virtual std::shared_ptr<Iterator::Impl> end() const {
        return std::make_shared<Iterator::Impl>();
    }

    virtual ~Impl() = default;
};

C2FieldUtils::List::List(std::shared_ptr<Impl> impl)
    : mImpl(impl) { }

C2FieldUtils::Iterator C2FieldUtils::List::begin() const {
    return C2FieldUtils::Iterator(mImpl->begin());
}

C2FieldUtils::Iterator C2FieldUtils::List::end() const {
    return C2FieldUtils::Iterator(mImpl->end());
}

/* ------------------------- C2FieldUtils::enumerateFields ------------------------- */

namespace {

/**
 * Iterator base class helper that allows descending into the field hierarchy.
 */
struct C2FieldUtilsFieldsIteratorHelper : public C2FieldUtils::Iterator::Impl {
    virtual ~C2FieldUtilsFieldsIteratorHelper() override = default;

    /// returns the base-field's offset of the parent field (or the param offset if no parent)
    static inline uint32_t GetParentBaseFieldOffset(
            const std::shared_ptr<C2FieldUtils::Info::Impl> parent) {
        return parent == nullptr ? sizeof(C2Param) : parent->baseFieldOffset;
    }

    /// returns the offset of the parent field (or the param)
    static inline uint32_t GetParentOffset(const std::shared_ptr<C2FieldUtils::Info::Impl> parent) {
        return parent == nullptr ? sizeof(C2Param) : GetOffset(parent->field);
    }

protected:
    C2FieldUtilsFieldsIteratorHelper(
            std::shared_ptr<C2ParamReflector> reflector,
            uint32_t paramSize,
            std::shared_ptr<C2FieldUtils::Info::Impl> head = nullptr)
        : C2FieldUtils::Iterator::Impl(head),
          mParamSize(paramSize),
          mReflector(reflector) { }

    /// returns a leaf info object at a specific index for a child field
    std::shared_ptr<C2FieldUtils::Info::Impl> makeLeaf(
            const C2FieldDescriptor &field, uint32_t index) {
        uint32_t parentOffset = GetParentOffset(mHead);
        uint32_t arrayOffset = parentOffset + GetOffset(field);
        uint32_t usedExtent = field.extent() ? :
                (std::max(arrayOffset, mParamSize) - arrayOffset) / GetSize(field);

        return std::make_shared<C2FieldUtils::Info::Impl>(
                OffsetFieldDescriptor(field, parentOffset + index * GetSize(field)),
                mHead /* parent */, index, mHead == nullptr ? 0 : mHead->depth + 1,
                GetParentBaseFieldOffset(mHead) + GetOffset(field),
                arrayOffset, usedExtent);
    }

    /// returns whether this struct index have been traversed to get to this field
    bool visited(C2Param::CoreIndex index) const {
        for (const std::shared_ptr<C2StructDescriptor> &sd : mHistory) {
            if (sd->coreIndex() == index) {
                return true;
            }
        }
        return false;
    }

    uint32_t mParamSize;
    std::shared_ptr<C2ParamReflector> mReflector;
    std::vector<std::shared_ptr<C2StructDescriptor>> mHistory; // structure types visited
};

/**
 * Iterator implementing enumerateFields() that visits each base field.
 */
struct C2FieldUtilsFieldsIterator : public C2FieldUtilsFieldsIteratorHelper {
    /// enumerate base fields of a parameter
    C2FieldUtilsFieldsIterator(const C2Param &param, std::shared_ptr<C2ParamReflector> reflector)
        : C2FieldUtilsFieldsIteratorHelper(reflector, param.size()) {
        descendInto(param.coreIndex());
    }

    /// enumerate base fields of a field
    C2FieldUtilsFieldsIterator(std::shared_ptr<C2FieldUtilsFieldsIterator> impl)
        : C2FieldUtilsFieldsIteratorHelper(impl->mReflector, impl->mParamSize, impl->mHead) {
        mHistory = impl->mHistory;
        if (mHead->field.type() & C2FieldDescriptor::STRUCT_FLAG) {
            C2Param::CoreIndex index = { mHead->field.type() &~C2FieldDescriptor::STRUCT_FLAG };
            if (!visited(index)) {
                descendInto(index);
            }
        }
    }

    virtual ~C2FieldUtilsFieldsIterator() override = default;

    /// Increments this iterator by visiting each base field.
    virtual void increment() override {
        // don't go past end
        if (mHead == nullptr || _mFields.empty()) {
            return;
        }

        // descend into structures
        if (mHead->field.type() & C2FieldDescriptor::STRUCT_FLAG) {
            C2Param::CoreIndex index = { mHead->field.type() &~C2FieldDescriptor::STRUCT_FLAG };
            // do not recurse into the same structs
            if (!visited(index) && descendInto(index)) {
                return;
            }
        }

        // ascend after the last field in the current struct
        while (!mHistory.empty() && _mFields.back() == mHistory.back()->end()) {
            mHead = mHead->parent;
            mHistory.pop_back();
            _mFields.pop_back();
        }

        // done if history is now empty
        if (_mFields.empty()) {
            // we could be traversing a sub-tree so clear head
            mHead.reset();
            return;
        }

        // move to the next field in the current struct
        C2StructDescriptor::field_iterator next = _mFields.back();
        mHead->field = OffsetFieldDescriptor(*next, GetParentOffset(mHead->parent));
        mHead->index = 0; // reset index just in case for correctness
        mHead->baseFieldOffset = GetParentBaseFieldOffset(mHead->parent) + GetOffset(*next);
        mHead->arrayOffset = GetOffset(mHead->field);
        mHead->usedExtent = mHead->field.extent() ? :
                (std::max(mHead->arrayOffset, mParamSize) - mHead->arrayOffset)
                        / GetSize(mHead->field);
        ++_mFields.back();
    }

private:
    /// If the current field is a known, valid (untraversed) structure, it modifies this iterator
    /// to point to the first field of the structure and returns true. Otherwise, it does not
    /// modify this iterator and returns false.
    bool descendInto(C2Param::CoreIndex index) {
        std::unique_ptr<C2StructDescriptor> descUnique = mReflector->describe(index);
        // descend into known structs (as long as they have at least one field)
        if (descUnique && descUnique->begin() != descUnique->end()) {
            std::shared_ptr<C2StructDescriptor> desc(std::move(descUnique));
            mHistory.emplace_back(desc);
            C2StructDescriptor::field_iterator first = desc->begin();
            mHead = makeLeaf(*first, 0 /* index */);
            _mFields.emplace_back(++first);
            return true;
        }
        return false;
    }

    /// next field pointers for each depth.
    /// note: _mFields may be shorted than mHistory, if iterating at a depth
    std::vector<C2StructDescriptor::field_iterator> _mFields;
};

/**
 * Iterable implementing enumerateFields().
 */
struct C2FieldUtilsFieldIterable : public C2FieldUtils::List::Impl {
    /// returns an iterator to the beginning of the list
    virtual std::shared_ptr<C2FieldUtils::Iterator::Impl> begin() const override {
        return std::make_shared<C2FieldUtilsFieldsIterator>(*_mParam, _mReflector);
    };

    C2FieldUtilsFieldIterable(const C2Param &param, std::shared_ptr<C2ParamReflector> reflector)
        : _mParam(&param), _mReflector(reflector) { }

private:
    const C2Param *_mParam;
    std::shared_ptr<C2ParamReflector> _mReflector;
};

}

C2FieldUtils::List C2FieldUtils::enumerateFields(
        const C2Param &param, const std::shared_ptr<C2ParamReflector> &reflector) {
    return C2FieldUtils::List(std::make_shared<C2FieldUtilsFieldIterable>(param, reflector));
}

/* ------------------------- C2FieldUtils::enumerate siblings ------------------------- */

namespace {

struct C2FieldUtilsCousinsIterator : public C2FieldUtils::Iterator::Impl {
    C2FieldUtilsCousinsIterator(
                const std::shared_ptr<C2FieldUtils::Info::Impl> &info, size_t level)
          // clone info chain as this iterator will change it
        : C2FieldUtils::Iterator::Impl(C2FieldUtils::Info::Impl::Clone(info)) {
        if (level == 0) {
            return;
        }

        // store parent chain (up to level) for quick access
        std::shared_ptr<C2FieldUtils::Info::Impl> node = mHead;
        size_t ix = 0;
        for (; ix < level && node; ++ix) {
            node->index = 0;
            _mPath.emplace_back(node);
            node = node->parent;
        }
        setupPath(ix);
    }

    virtual ~C2FieldUtilsCousinsIterator() override = default;

    /// Increments this iterator by visiting each index.
    virtual void increment() override {
        size_t ix = 0;
        while (ix < _mPath.size()) {
            if (++_mPath[ix]->index < _mPath[ix]->usedExtent) {
                setupPath(ix + 1);
                return;
            }
            _mPath[ix++]->index = 0;
        }
        mHead.reset();
    }

private:
    /// adjusts field offsets along the path up to the specific level - 1.
    /// This in-fact has to be done down the path from parent to child as child fields must
    /// fall within parent fields.
    void setupPath(size_t level) {
        C2_CHECK_LE(level, _mPath.size());
        uint32_t oldArrayOffset = level ? _mPath[level - 1]->arrayOffset : 0 /* unused */;
        while (level) {
            --level;
            C2FieldUtils::Info::Impl &path = *_mPath[level];
            uint32_t size = GetSize(path.field);
            uint32_t offset = path.arrayOffset + size * path.index;
            SetOffset(path.field, offset);
            if (level) {
                // reset child's array offset to fall within current index, but hold onto the
                // original value of the arrayOffset so that we can adjust subsequent children.
                // This is because the modulo is only defined within the current array.
                uint32_t childArrayOffset =
                    offset + (_mPath[level - 1]->arrayOffset - oldArrayOffset) % size;
                oldArrayOffset = _mPath[level - 1]->arrayOffset;
                _mPath[level - 1]->arrayOffset = childArrayOffset;
            }
        }
    }

    std::vector<std::shared_ptr<C2FieldUtils::Info::Impl>> _mPath;
};

/**
 * Iterable implementing enumerateFields().
 */
struct C2FieldUtilsCousinsIterable : public C2FieldUtils::List::Impl {
    /// returns an iterator to the beginning of the list
    virtual std::shared_ptr<C2FieldUtils::Iterator::Impl> begin() const override {
        return std::make_shared<C2FieldUtilsCousinsIterator>(_mHead, _mLevel);
    };

    C2FieldUtilsCousinsIterable(const C2FieldUtils::Info &field, uint32_t level)
        : _mHead(C2FieldUtils::_Inspector::GetImpl(field)), _mLevel(level) { }

private:
    std::shared_ptr<C2FieldUtils::Info::Impl> _mHead;
    size_t _mLevel;
};

}

C2FieldUtils::List C2FieldUtils::enumerateCousins(const C2FieldUtils::Info &field, uint32_t level) {
    return C2FieldUtils::List(std::make_shared<C2FieldUtilsCousinsIterable>(field, level));
}

/* ------------------------- C2FieldUtils::locateField ------------------------- */

namespace {

/**
 * Iterator implementing locateField().
 */
struct C2FieldUtilsFieldLocator : public C2FieldUtilsFieldsIteratorHelper {
    C2FieldUtilsFieldLocator(
            C2Param::CoreIndex index, const _C2FieldId &field, uint32_t paramSize,
            std::shared_ptr<C2ParamReflector> reflector)
        : C2FieldUtilsFieldsIteratorHelper(reflector, paramSize),
          _mField(field) {
        while (descendInto(index)) {
            if ((mHead->field.type() & C2FieldDescriptor::STRUCT_FLAG) == 0) {
                break;
            }
            index = C2Param::CoreIndex(mHead->field.type() &~ C2FieldDescriptor::STRUCT_FLAG);
        }
    }

    void increment() {
        mHead = _mTail;
        _mTail = nullptr;
    }

private:
    /// If the current field is a known, valid (untraversed) structure, it modifies this iterator
    /// to point to the field at the beginning/end of the given field of the structure and returns
    /// true. Otherwise, including if no such field exists in the structure, it does not modify this
    /// iterator and returns false.
    bool descendInto(C2Param::CoreIndex index) {
        // check that the boundaries of the field to be located are still within the same parent
        // field
        if (mHead != _mTail) {
            return false;
        }

        std::unique_ptr<C2StructDescriptor> descUnique = mReflector->describe(index);
        // descend into known structs (as long as they have at least one field)
        if (descUnique && descUnique->begin() != descUnique->end()) {
            std::shared_ptr<C2StructDescriptor> desc(std::move(descUnique));
            mHistory.emplace_back(desc);

            uint32_t parentOffset = GetParentOffset(mHead);

            // locate field using a dummy field descriptor
            C2FieldDescriptor dummy = {
                C2FieldDescriptor::BLOB, 1 /* extent */, "name",
                GetOffset(_mField) - parentOffset, GetSize(_mField)
            };

            // locate first field where offset is greater than dummy offset (which is one past)
            auto it = std::upper_bound(
                    desc->cbegin(), desc->cend(), dummy,
                    [](const C2FieldDescriptor &a, const C2FieldDescriptor &b) -> bool {
                return _C2ParamInspector::GetOffset(a) < _C2ParamInspector::GetOffset(b);
            });
            if (it == desc->begin()) {
                // field is prior to first field
                return false;
            }
            --it;
            const C2FieldDescriptor &field = *it;

            // check that dummy end-offset is within this field
            uint32_t structSize = std::max(mParamSize, parentOffset) - parentOffset;
            if (GetEndOffset(dummy) > GetEndOffset(field, structSize)) {
                return false;
            }

            uint32_t startIndex = (GetOffset(dummy) - GetOffset(field)) / GetSize(field);
            uint32_t endIndex =
                (GetEndOffset(dummy) - GetOffset(field) + GetSize(field) - 1) / GetSize(field);
            if (endIndex > startIndex) {
                // Field size could be zero, in which case end index is still on start index.
                // However, for all other cases, endIndex was rounded up to the next index, so
                // decrement it.
                --endIndex;
            }
            std::shared_ptr<C2FieldUtils::Info::Impl> startLeaf =
                makeLeaf(field, startIndex);
            if (endIndex == startIndex) {
                _mTail = startLeaf;
                mHead = startLeaf;
            } else {
                _mTail = makeLeaf(field, endIndex);
                mHead = startLeaf;
            }
            return true;
        }
        return false;
    }

    _C2FieldId _mField;
    std::shared_ptr<C2FieldUtils::Info::Impl> _mTail;
};

/**
 * Iterable implementing locateField().
 */
struct C2FieldUtilsFieldLocation : public C2FieldUtils::List::Impl {
    /// returns an iterator to the beginning of the list
    virtual std::shared_ptr<C2FieldUtils::Iterator::Impl> begin() const override {
        return std::make_shared<C2FieldUtilsFieldLocator>(
                _mIndex, _mField, _mParamSize, _mReflector);
    };

    C2FieldUtilsFieldLocation(
            const C2ParamField &pf, std::shared_ptr<C2ParamReflector> reflector)
        : _mIndex(C2Param::CoreIndex(_C2ParamInspector::GetIndex(pf))),
          _mField(_C2ParamInspector::GetField(pf)),
          _mParamSize(0),
          _mReflector(reflector) { }


    C2FieldUtilsFieldLocation(
            const C2Param &param, const _C2FieldId &field,
            std::shared_ptr<C2ParamReflector> reflector)
        : _mIndex(param.coreIndex()),
          _mField(field),
          _mParamSize(param.size()),
          _mReflector(reflector) { }

private:
    C2Param::CoreIndex _mIndex;
    _C2FieldId _mField;
    uint32_t _mParamSize;
    std::shared_ptr<C2ParamReflector> _mReflector;
};

}

std::vector<C2FieldUtils::Info> C2FieldUtils::locateField(
        const C2ParamField &pf, const std::shared_ptr<C2ParamReflector> &reflector) {
    C2FieldUtils::List location = { std::make_shared<C2FieldUtilsFieldLocation>(pf, reflector) };
    return std::vector<Info>(location.begin(), location.end());
}

std::vector<C2FieldUtils::Info> C2FieldUtils::locateField(
        const C2Param &param, const _C2FieldId &field,
        const std::shared_ptr<C2ParamReflector> &reflector) {
    C2FieldUtils::List location = {
        std::make_shared<C2FieldUtilsFieldLocation>(param, field, reflector)
    };
    return std::vector<Info>(location.begin(), location.end());
}

