/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef C2PARAM_H_
#define C2PARAM_H_

#include <C2.h>

#include <stdbool.h>
#include <stdint.h>

#include <algorithm>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

/// \addtogroup Parameters
/// @{

/// \defgroup internal Internal helpers.

/*!
 * \file
 * PARAMETERS: SETTINGs, TUNINGs, and INFOs
 * ===
 *
 * These represent miscellaneous control and metadata information and are likely copied into
 * kernel space. Therefore, these are C-like structures designed to carry just a small amount of
 * information. We are using C++ to be able to add constructors, as well as non-virtual and class
 * methods.
 *
 * ==Specification details:
 *
 * Restrictions:
 *   - must be POD struct, e.g. no vtable (no virtual destructor)
 *   - must have the same size in 64-bit and 32-bit mode (no size_t)
 *   - as such, no pointer members
 *   - some common member field names are reserved as they are defined as methods for all
 *     parameters:
 *     they are: size, type, kind, index and stream
 *
 * Behavior:
 * - Params can be global (not related to input or output), related to input or output,
 *   or related to an input/output stream.
 * - All params are queried/set using a unique param index, which incorporates a potential stream
 *   index and/or port.
 * - Querying (supported) params MUST never fail.
 * - All params MUST have default values.
 * - If some fields have "unsupported" or "invalid" values during setting, this SHOULD be
 *   communicated to the app.
 *   a) Ideally, this should be avoided.  When setting parameters, in general, component should do
 *     "best effort" to apply all settings. It should change "invalid/unsupported" values to the
 *     nearest supported values.
 *   - This is communicated to the client by changing the source values in tune()/
 *     configure().
 *   b) If falling back to a supported value is absolutely impossible, the component SHALL return
 *     an error for the specific setting, but should continue to apply other settings.
 *     TODO: this currently may result in unintended results.
 *
 * **NOTE:** unlike OMX, params are not versioned. Instead, a new struct with new param index
 * SHALL be added as new versions are required.
 *
 * The proper subtype (Setting, Info or Param) is incorporated into the class type. Define structs
 * to define multiple subtyped versions of related parameters.
 *
 * ==Implementation details:
 *
 * - Use macros to define parameters
 * - All parameters must have a default constructor
 *   - This is only used for instantiating the class in source (e.g. will not be used
 *     when building a parameter by the framework from key/value pairs.)
 */

/// \ingroup internal

/**
 * Parameter base class.
 */
struct C2Param {
    // param index encompasses the following:
    //
    // - kind (setting, tuning, info, struct)
    // - scope
    //   - direction (global, input, output)
    //   - stream flag
    //   - stream ID (usually 0)
    // - and the parameter's type (core index)
    //   - flexible parameter flag
    //   - vendor extension flag
    //   - type index (this includes the vendor extension flag)
    //
    // layout:
    //
    //        kind : <------- scope -------> : <----- core index ----->
    //      +------+-----+---+------+--------+----|------+--------------+
    //      | kind | dir | - |stream|streamID|flex|vendor|  type index  |
    //      +------+-----+---+------+--------+----+------+--------------+
    //  bit: 31..30 29.28       25   24 .. 17  16    15   14    ..     0
    //
public:
    /**
     * C2Param kinds, usable as bitmaps.
     */
    enum kind_t : uint32_t {
        NONE    = 0,
        STRUCT  = (1 << 0),
        INFO    = (1 << 1),
        SETTING = (1 << 2),
        TUNING  = (1 << 3) | SETTING, // tunings are settings
    };

    /**
     * The parameter type index specifies the underlying parameter type of a parameter as
     * an integer value.
     *
     * Parameter types are divided into two groups: platform types and vendor types.
     *
     * Platform types are defined by the platform and are common for all implementations.
     *
     * Vendor types are defined by each vendors, so they may differ between implementations.
     * It is recommended that vendor types be the same for all implementations by a specific
     * vendor.
     */
    typedef uint32_t type_index_t;
    enum : uint32_t {
            TYPE_INDEX_VENDOR_START = 0x00008000, ///< vendor indices SHALL start after this
    };

    /**
     * Core index is the underlying parameter type for a parameter. It is used to describe the
     * layout of the parameter structure regardless of the component or parameter kind/scope.
     *
     * It is used to identify and distinguish global parameters, and also parameters on a given
     * port or stream. They must be unique for the set of global parameters, as well as for the
     * set of parameters on each port or each stream, but the same core index can be used for
     * parameters on different streams or ports, as well as for global parameters and port/stream
     * parameters.
     *
     * Multiple parameter types can share the same layout.
     *
     * \note The layout for all parameters with the same core index across all components must
     * be identical.
     */
    struct CoreIndex {
    //public:
        enum : uint32_t {
            IS_FLEX_FLAG    = 0x00010000,
            IS_REQUEST_FLAG = 0x00020000,
        };

    protected:
        enum : uint32_t {
            KIND_MASK      = 0xC0000000,
            KIND_STRUCT    = 0x00000000,
            KIND_TUNING    = 0x40000000,
            KIND_SETTING   = 0x80000000,
            KIND_INFO      = 0xC0000000,

            DIR_MASK       = 0x30000000,
            DIR_GLOBAL     = 0x20000000,
            DIR_UNDEFINED  = DIR_MASK, // MUST have all bits set
            DIR_INPUT      = 0x00000000,
            DIR_OUTPUT     = 0x10000000,

            IS_STREAM_FLAG  = 0x02000000,
            STREAM_ID_MASK  = 0x01F00000,
            STREAM_ID_SHIFT = 20,
            MAX_STREAM_ID   = STREAM_ID_MASK >> STREAM_ID_SHIFT,
            STREAM_MASK     = IS_STREAM_FLAG | STREAM_ID_MASK,

            IS_VENDOR_FLAG  = 0x00008000,
            TYPE_INDEX_MASK = 0x0000FFFF,
            CORE_MASK       = TYPE_INDEX_MASK | IS_FLEX_FLAG,
        };

    public:
        /// constructor/conversion from uint32_t
        inline CoreIndex(uint32_t index) : mIndex(index) { }

        // no conversion from uint64_t
        inline CoreIndex(uint64_t index) = delete;

        /// returns true iff this is a vendor extension parameter
        inline bool isVendor() const { return mIndex & IS_VENDOR_FLAG; }

        /// returns true iff this is a flexible parameter (with variable size)
        inline bool isFlexible() const { return mIndex & IS_FLEX_FLAG; }

        /// returns the core index
        /// This is the combination of the parameter type index and the flexible flag.
        inline uint32_t coreIndex() const { return mIndex & CORE_MASK; }

        /// returns the parameter type index
        inline type_index_t typeIndex() const { return mIndex & TYPE_INDEX_MASK; }

        DEFINE_FIELD_AND_MASK_BASED_COMPARISON_OPERATORS(CoreIndex, mIndex, CORE_MASK)

    protected:
        uint32_t mIndex;
    };

    /**
     * Type encompasses the parameter's kind (tuning, setting, info), its scope (whether the
     * parameter is global, input or output, and whether it is for a stream) and the its base
     * index (which also determines its layout).
     */
    struct Type : public CoreIndex {
    //public:
        /// returns true iff this is a global parameter (not for input nor output)
        inline bool isGlobal() const { return (mIndex & DIR_MASK) == DIR_GLOBAL; }
        /// returns true iff this is an input or input stream parameter
        inline bool forInput() const { return (mIndex & DIR_MASK) == DIR_INPUT; }
        /// returns true iff this is an output or output stream parameter
        inline bool forOutput() const { return (mIndex & DIR_MASK) == DIR_OUTPUT; }

        /// returns true iff this is a stream parameter
        inline bool forStream() const { return mIndex & IS_STREAM_FLAG; }
        /// returns true iff this is a port (input or output) parameter
        inline bool forPort() const   { return !forStream() && !isGlobal(); }

        /// returns the parameter type: the parameter index without the stream ID
        inline uint32_t type() const { return mIndex & (~STREAM_ID_MASK); }

        /// return the kind (struct, info, setting or tuning) of this param
        inline kind_t kind() const {
            switch (mIndex & KIND_MASK) {
                case KIND_STRUCT: return STRUCT;
                case KIND_INFO: return INFO;
                case KIND_SETTING: return SETTING;
                case KIND_TUNING: return TUNING;
                default: return NONE; // should not happen
            }
        }

        /// constructor/conversion from uint32_t
        inline Type(uint32_t index) : CoreIndex(index) { }

        // no conversion from uint64_t
        inline Type(uint64_t index) = delete;

        DEFINE_FIELD_AND_MASK_BASED_COMPARISON_OPERATORS(Type, mIndex, ~STREAM_ID_MASK)

    private:
        friend struct C2Param;   // for setPort()
        friend struct C2Tuning;  // for KIND_TUNING
        friend struct C2Setting; // for KIND_SETTING
        friend struct C2Info;    // for KIND_INFO
        // for DIR_GLOBAL
        template<typename T, typename S, int I, class F> friend struct C2GlobalParam;
        template<typename T, typename S, int I, class F> friend struct C2PortParam;   // for kDir*
        template<typename T, typename S, int I, class F> friend struct C2StreamParam; // for kDir*
        friend struct _C2ParamInspector; // for testing

        /**
         * Sets the port/stream direction.
         * @return true on success, false if could not set direction (e.g. it is global param).
         */
        inline bool setPort(bool output) {
            if (isGlobal()) {
                return false;
            } else {
                mIndex = (mIndex & ~DIR_MASK) | (output ? DIR_OUTPUT : DIR_INPUT);
                return true;
            }
        }
    };

    /**
     * index encompasses all remaining information: basically the stream ID.
     */
    struct Index : public Type {
        /// returns the index as uint32_t
        inline operator uint32_t() const { return mIndex; }

        /// constructor/conversion from uint32_t
        inline Index(uint32_t index) : Type(index) { }

        /// copy constructor
        inline Index(const Index &index) = default;

        // no conversion from uint64_t
        inline Index(uint64_t index) = delete;

        /// returns the stream ID or ~0 if not a stream
        inline unsigned stream() const {
            return forStream() ? rawStream() : ~0U;
        }

        /// Returns an index with stream field set to given stream.
        inline Index withStream(unsigned stream) const {
            Index ix = mIndex;
            (void)ix.setStream(stream);
            return ix;
        }

        /// sets the port (direction). Returns true iff successful.
        inline Index withPort(bool output) const {
            Index ix = mIndex;
            (void)ix.setPort(output);
            return ix;
        }

        DEFINE_FIELD_BASED_COMPARISON_OPERATORS(Index, mIndex)

    private:
        friend struct C2Param;           // for setStream, MakeStreamId, isValid
        friend struct _C2ParamInspector; // for testing

        /**
         * @return true if the type is valid, e.g. direction is not undefined AND
         * stream is 0 if not a stream param.
         */
        inline bool isValid() const {
            // there is no Type::isValid (even though some of this check could be
            // performed on types) as this is only used on index...
            return (forStream() ? rawStream() < MAX_STREAM_ID : rawStream() == 0)
                    && (mIndex & DIR_MASK) != DIR_UNDEFINED;
        }

        /// returns the raw stream ID field
        inline unsigned rawStream() const {
            return (mIndex & STREAM_ID_MASK) >> STREAM_ID_SHIFT;
        }

        /// returns the streamId bitfield for a given |stream|. If stream is invalid,
        /// returns an invalid bitfield.
        inline static uint32_t MakeStreamId(unsigned stream) {
            // saturate stream ID (max value is invalid)
            if (stream > MAX_STREAM_ID) {
                stream = MAX_STREAM_ID;
            }
            return (stream << STREAM_ID_SHIFT) & STREAM_ID_MASK;
        }

        inline bool convertToStream(bool output, unsigned stream) {
            mIndex = (mIndex & ~DIR_MASK) | IS_STREAM_FLAG;
            (void)setPort(output);
            return setStream(stream);
        }

        inline void convertToPort(bool output) {
            mIndex = (mIndex & ~(DIR_MASK | IS_STREAM_FLAG));
            (void)setPort(output);
        }

        inline void convertToGlobal() {
            mIndex = (mIndex & ~(DIR_MASK | IS_STREAM_FLAG)) | DIR_GLOBAL;
        }

        inline void convertToRequest() {
            mIndex = mIndex | IS_REQUEST_FLAG;
        }

        /**
         * Sets the stream index.
         * \return true on success, false if could not set index (e.g. not a stream param).
         */
        inline bool setStream(unsigned stream) {
            if (forStream()) {
                mIndex = (mIndex & ~STREAM_ID_MASK) | MakeStreamId(stream);
                return this->stream() < MAX_STREAM_ID;
            }
            return false;
        }
    };

public:
    // public getters for Index methods

    /// returns true iff this is a vendor extension parameter
    inline bool isVendor() const { return _mIndex.isVendor(); }
    /// returns true iff this is a flexible parameter
    inline bool isFlexible() const { return _mIndex.isFlexible(); }
    /// returns true iff this is a global parameter (not for input nor output)
    inline bool isGlobal() const { return _mIndex.isGlobal(); }
    /// returns true iff this is an input or input stream parameter
    inline bool forInput() const { return _mIndex.forInput(); }
    /// returns true iff this is an output or output stream parameter
    inline bool forOutput() const { return _mIndex.forOutput(); }

    /// returns true iff this is a stream parameter
    inline bool forStream() const { return _mIndex.forStream(); }
    /// returns true iff this is a port (input or output) parameter
    inline bool forPort() const   { return _mIndex.forPort(); }

    /// returns the stream ID or ~0 if not a stream
    inline unsigned stream() const { return _mIndex.stream(); }

    /// returns the parameter type: the parameter index without the stream ID
    inline Type type() const { return _mIndex.type(); }

    /// returns the index of this parameter
    /// \todo: should we restrict this to C2ParamField?
    inline uint32_t index() const { return (uint32_t)_mIndex; }

    /// returns the core index of this parameter
    inline CoreIndex coreIndex() const { return _mIndex.coreIndex(); }

    /// returns the kind of this parameter
    inline kind_t kind() const { return _mIndex.kind(); }

    /// returns the size of the parameter or 0 if the parameter is invalid
    inline size_t size() const { return _mSize; }

    /// returns true iff the parameter is valid
    inline operator bool() const { return _mIndex.isValid() && _mSize > 0; }

    /// returns true iff the parameter is invalid
    inline bool operator!() const { return !operator bool(); }

    // equality is done by memcmp (use equals() to prevent any overread)
    inline bool operator==(const C2Param &o) const {
        return equals(o) && memcmp(this, &o, _mSize) == 0;
    }
    inline bool operator!=(const C2Param &o) const { return !operator==(o); }

    /// safe(r) type cast from pointer and size
    inline static C2Param* From(void *addr, size_t len) {
        // _mSize must fit into size, but really C2Param must also to be a valid param
        if (len < sizeof(C2Param)) {
            return nullptr;
        }
        // _mSize must match length
        C2Param *param = (C2Param*)addr;
        if (param->_mSize != len) {
            return nullptr;
        }
        return param;
    }

    /// Returns managed clone of |orig| at heap.
    inline static std::unique_ptr<C2Param> Copy(const C2Param &orig) {
        if (orig.size() == 0) {
            return nullptr;
        }
        void *mem = ::operator new (orig.size());
        C2Param *param = new (mem) C2Param(orig.size(), orig._mIndex);
        param->updateFrom(orig);
        return std::unique_ptr<C2Param>(param);
    }

    /// Returns managed clone of |orig| as a stream parameter at heap.
    inline static std::unique_ptr<C2Param> CopyAsStream(
            const C2Param &orig, bool output, unsigned stream) {
        std::unique_ptr<C2Param> copy = Copy(orig);
        if (copy) {
            copy->_mIndex.convertToStream(output, stream);
        }
        return copy;
    }

    /// Returns managed clone of |orig| as a port parameter at heap.
    inline static std::unique_ptr<C2Param> CopyAsPort(const C2Param &orig, bool output) {
        std::unique_ptr<C2Param> copy = Copy(orig);
        if (copy) {
            copy->_mIndex.convertToPort(output);
        }
        return copy;
    }

    /// Returns managed clone of |orig| as a global parameter at heap.
    inline static std::unique_ptr<C2Param> CopyAsGlobal(const C2Param &orig) {
        std::unique_ptr<C2Param> copy = Copy(orig);
        if (copy) {
            copy->_mIndex.convertToGlobal();
        }
        return copy;
    }

    /// Returns managed clone of |orig| as a stream parameter at heap.
    inline static std::unique_ptr<C2Param> CopyAsRequest(const C2Param &orig) {
        std::unique_ptr<C2Param> copy = Copy(orig);
        if (copy) {
            copy->_mIndex.convertToRequest();
        }
        return copy;
    }

#if 0
    template<typename P, class=decltype(C2Param(P()))>
    P *As() { return P::From(this); }
    template<typename P>
    const P *As() const { return const_cast<const P*>(P::From(const_cast<C2Param*>(this))); }
#endif

protected:
    /// sets the stream field. Returns true iff successful.
    inline bool setStream(unsigned stream) {
        return _mIndex.setStream(stream);
    }

    /// sets the port (direction). Returns true iff successful.
    inline bool setPort(bool output) {
        return _mIndex.setPort(output);
    }

public:
    /// invalidate this parameter. There is no recovery from this call; e.g. parameter
    /// cannot be 'corrected' to be valid.
    inline void invalidate() { _mSize = 0; }

    // if other is the same kind of (valid) param as this, copy it into this and return true.
    // otherwise, do not copy anything, and return false.
    inline bool updateFrom(const C2Param &other) {
        if (other._mSize <= _mSize && other._mIndex == _mIndex && _mSize > 0) {
            memcpy(this, &other, other._mSize);
            return true;
        }
        return false;
    }

protected:
    // returns |o| if it is a null ptr, or if can suitably be a param of given |type| (e.g. has
    // same type (ignoring stream ID), and size). Otherwise, returns null. If |checkDir| is false,
    // allow undefined or different direction (e.g. as constructed from C2PortParam() vs.
    // C2PortParam::input), but still require equivalent type (stream, port or global); otherwise,
    // return null.
    inline static const C2Param* IfSuitable(
            const C2Param* o, size_t size, Type type, size_t flexSize = 0, bool checkDir = true) {
        if (o == nullptr || o->_mSize < size || (flexSize && ((o->_mSize - size) % flexSize))) {
            return nullptr;
        } else if (checkDir) {
            return o->_mIndex.type() == type.mIndex ? o : nullptr;
        } else if (o->_mIndex.isGlobal()) {
            return nullptr;
        } else {
            return ((o->_mIndex.type() ^ type.mIndex) & ~Type::DIR_MASK) ? nullptr : o;
        }
    }

    /// base constructor
    inline C2Param(uint32_t paramSize, Index paramIndex)
        : _mSize(paramSize),
          _mIndex(paramIndex) {
        if (paramSize > sizeof(C2Param)) {
            memset(this + 1, 0, paramSize - sizeof(C2Param));
        }
    }

    /// base constructor with stream set
    inline C2Param(uint32_t paramSize, Index paramIndex, unsigned stream)
        : _mSize(paramSize),
          _mIndex(paramIndex | Index::MakeStreamId(stream)) {
        if (paramSize > sizeof(C2Param)) {
            memset(this + 1, 0, paramSize - sizeof(C2Param));
        }
        if (!forStream()) {
            invalidate();
        }
    }

private:
    friend struct _C2ParamInspector; // for testing

    /// returns true iff |o| has the same size and index as this. This performs the
    /// basic check for equality.
    inline bool equals(const C2Param &o) const {
        return _mSize == o._mSize && _mIndex == o._mIndex;
    }

    uint32_t _mSize;
    Index _mIndex;
};

/// \ingroup internal
/// allow C2Params access to private methods, e.g. constructors
#define C2PARAM_MAKE_FRIENDS \
    template<typename U, typename S, int I, class F> friend struct C2GlobalParam; \
    template<typename U, typename S, int I, class F> friend struct C2PortParam; \
    template<typename U, typename S, int I, class F> friend struct C2StreamParam; \

/**
 * Setting base structure for component method signatures. Wrap constructors.
 */
struct C2Setting : public C2Param {
protected:
    template<typename ...Args>
    inline C2Setting(const Args(&... args)) : C2Param(args...) { }
public: // TODO
    enum : uint32_t { PARAM_KIND = Type::KIND_SETTING };
};

/**
 * Tuning base structure for component method signatures. Wrap constructors.
 */
struct C2Tuning : public C2Setting {
protected:
    template<typename ...Args>
    inline C2Tuning(const Args(&... args)) : C2Setting(args...) { }
public: // TODO
    enum : uint32_t { PARAM_KIND = Type::KIND_TUNING };
};

/**
 * Info base structure for component method signatures. Wrap constructors.
 */
struct C2Info : public C2Param {
protected:
    template<typename ...Args>
    inline C2Info(const Args(&... args)) : C2Param(args...) { }
public: // TODO
    enum : uint32_t { PARAM_KIND = Type::KIND_INFO };
};

/**
 * Structure uniquely specifying a field in an arbitrary structure.
 *
 * \note This structure is used differently in C2FieldDescriptor to
 * identify array fields, such that _mSize is the size of each element. This is
 * because the field descriptor contains the array-length, and we want to keep
 * a relevant element size for variable length arrays.
 */
struct _C2FieldId {
//public:
    /**
     * Constructor used for C2FieldDescriptor that removes the array extent.
     *
     * \param[in] offset pointer to the field in an object at address 0.
     */
    template<typename T, class B=typename std::remove_extent<T>::type>
    inline _C2FieldId(T* offset)
        : // offset is from "0" so will fit on 32-bits
          _mOffset((uint32_t)(uintptr_t)(offset)),
          _mSize(sizeof(B)) { }

    /**
     * Direct constructor from offset and size.
     *
     * \param[in] offset offset of the field.
     * \param[in] size size of the field.
     */
    inline _C2FieldId(size_t offset, size_t size)
        : _mOffset(offset), _mSize(size) {}

    /**
     * Constructor used to identify a field in an object.
     *
     * \param U[type] pointer to the object that contains this field. This is needed in case the
     *        field is in an (inherited) base class, in which case T will be that base class.
     * \param pm[im] member pointer to the field
     */
    template<typename R, typename T, typename U, typename B=typename std::remove_extent<R>::type>
    inline _C2FieldId(U *, R T::* pm)
        : _mOffset((uint32_t)(uintptr_t)(&(((U*)256)->*pm)) - 256u),
          _mSize(sizeof(B)) { }

    /**
     * Constructor used to identify a field in an object.
     *
     * \param pm[im] member pointer to the field
     */
    template<typename R, typename T, typename B=typename std::remove_extent<R>::type>
    inline _C2FieldId(R T::* pm)
        : _mOffset((uint32_t)(uintptr_t)(&(((T*)0)->*pm))),
          _mSize(sizeof(B)) { }

    inline bool operator==(const _C2FieldId &other) const {
        return _mOffset == other._mOffset && _mSize == other._mSize;
    }

    inline bool operator<(const _C2FieldId &other) const {
        return _mOffset < other._mOffset ||
            // NOTE: order parent structure before sub field
            (_mOffset == other._mOffset && _mSize > other._mSize);
    }

    DEFINE_OTHER_COMPARISON_OPERATORS(_C2FieldId)

#if 0
    inline uint32_t offset() const { return _mOffset; }
    inline uint32_t size() const { return _mSize; }
#endif

#if defined(FRIEND_TEST)
    friend void PrintTo(const _C2FieldId &d, ::std::ostream*);
#endif

private:
    friend struct _C2ParamInspector;
    friend struct C2FieldDescriptor;

    uint32_t _mOffset; // offset of field
    uint32_t _mSize;   // size of field
};

/**
 * Structure uniquely specifying a 'field' in a configuration. The field
 * can be a field of a configuration, a subfield of a field of a configuration,
 * and even the whole configuration. Moreover, if the field can point to an
 * element in a array field, or to the entire array field.
 *
 * This structure is used for querying supported values for a field, as well
 * as communicating configuration failures and conflicts when trying to change
 * a configuration for a component/interface or a store.
 */
struct C2ParamField {
//public:
    /**
     * Create a field identifier using a configuration parameter (variable),
     * and a pointer to member.
     *
     * ~~~~~~~~~~~~~ (.cpp)
     *
     * struct C2SomeParam {
     *   uint32_t mField;
     *   uint32_t mArray[2];
     *   C2OtherStruct mStruct;
     *   uint32_t mFlexArray[];
     * } *mParam;
     *
     * C2ParamField(mParam, &mParam->mField);
     * C2ParamField(mParam, &mParam->mArray);
     * C2ParamField(mParam, &mParam->mArray[0]);
     * C2ParamField(mParam, &mParam->mStruct.mSubField);
     * C2ParamField(mParam, &mParam->mFlexArray);
     * C2ParamField(mParam, &mParam->mFlexArray[2]);
     *
     * ~~~~~~~~~~~~~
     *
     * \todo fix what this is for T[] (for now size becomes T[1])
     *
     * \note this does not work for 64-bit members as it triggers a
     * 'taking address of packed member' warning.
     *
     * \param param pointer to parameter
     * \param offset member pointer
     */
    template<typename S, typename T>
    inline C2ParamField(S* param, T* offset)
        : _mIndex(param->index()),
          _mFieldId((T*)((uintptr_t)offset - (uintptr_t)param)) {}

    template<typename S, typename T>
    inline static C2ParamField Make(S& param, T& offset) {
        return C2ParamField(param.index(), (uintptr_t)&offset - (uintptr_t)&param, sizeof(T));
    }

    /**
     * Create a field identifier using a configuration parameter (variable),
     * and a member pointer. This method cannot be used to refer to an
     * array element or a subfield.
     *
     * ~~~~~~~~~~~~~ (.cpp)
     *
     * C2SomeParam mParam;
     * C2ParamField(&mParam, &C2SomeParam::mMemberField);
     *
     * ~~~~~~~~~~~~~
     *
     * \param p pointer to parameter
     * \param T member pointer to the field member
     */
    template<typename R, typename T, typename U>
    inline C2ParamField(U *p, R T::* pm) : _mIndex(p->index()), _mFieldId(p, pm) { }

    /**
     * Create a field identifier to a configuration parameter (variable).
     *
     * ~~~~~~~~~~~~~ (.cpp)
     *
     * C2SomeParam mParam;
     * C2ParamField(&mParam);
     *
     * ~~~~~~~~~~~~~
     *
     * \param param pointer to parameter
     */
    template<typename S>
    inline C2ParamField(S* param)
        : _mIndex(param->index()), _mFieldId(0u, param->size()) { }

    /** Copy constructor. */
    inline C2ParamField(const C2ParamField &other) = default;

    /**
     * Equality operator.
     */
    inline bool operator==(const C2ParamField &other) const {
        return _mIndex == other._mIndex && _mFieldId == other._mFieldId;
    }

    /**
     * Ordering operator.
     */
    inline bool operator<(const C2ParamField &other) const {
        return _mIndex < other._mIndex ||
            (_mIndex == other._mIndex && _mFieldId < other._mFieldId);
    }

    DEFINE_OTHER_COMPARISON_OPERATORS(C2ParamField)

protected:
    inline C2ParamField(C2Param::Index index, uint32_t offset, uint32_t size)
        : _mIndex(index), _mFieldId(offset, size) {}

private:
    friend struct _C2ParamInspector;

    C2Param::Index _mIndex; ///< parameter index
    _C2FieldId _mFieldId;   ///< field identifier
};

/**
 * A shared (union) representation of numeric values
 */
class C2Value {
public:
    /// A union of supported primitive types.
    union Primitive {
        // first member is always zero initialized so it must be the largest
        uint64_t    u64;   ///< uint64_t value
        int64_t     i64;   ///< int64_t value
        c2_cntr64_t c64;   ///< c2_cntr64_t value
        uint32_t    u32;   ///< uint32_t value
        int32_t     i32;   ///< int32_t value
        c2_cntr32_t c32;   ///< c2_cntr32_t value
        float       fp;    ///< float value

        // constructors - implicit
        Primitive(uint64_t value)    : u64(value) { }
        Primitive(int64_t value)     : i64(value) { }
        Primitive(c2_cntr64_t value) : c64(value) { }
        Primitive(uint32_t value)    : u32(value) { }
        Primitive(int32_t value)     : i32(value) { }
        Primitive(c2_cntr32_t value) : c32(value) { }
        Primitive(uint8_t value)     : u32(value) { }
        Primitive(char value)        : i32(value) { }
        Primitive(float value)       : fp(value)  { }

        // allow construction from enum type
        template<typename E, typename = typename std::enable_if<std::is_enum<E>::value>::type>
        Primitive(E value)
            : Primitive(static_cast<typename std::underlying_type<E>::type>(value)) { }

        Primitive() : u64(0) { }

        /** gets value out of the union */
        template<typename T> const T &ref() const;

        // verify that we can assume standard aliasing
        static_assert(sizeof(u64) == sizeof(i64), "");
        static_assert(sizeof(u64) == sizeof(c64), "");
        static_assert(sizeof(u32) == sizeof(i32), "");
        static_assert(sizeof(u32) == sizeof(c32), "");
    };
    // verify that we can assume standard aliasing
    static_assert(offsetof(Primitive, u64) == offsetof(Primitive, i64), "");
    static_assert(offsetof(Primitive, u64) == offsetof(Primitive, c64), "");
    static_assert(offsetof(Primitive, u32) == offsetof(Primitive, i32), "");
    static_assert(offsetof(Primitive, u32) == offsetof(Primitive, c32), "");

    enum type_t : uint32_t {
        NO_INIT,
        INT32,
        UINT32,
        CNTR32,
        INT64,
        UINT64,
        CNTR64,
        FLOAT,
    };

    template<typename T, bool = std::is_enum<T>::value>
    inline static constexpr type_t TypeFor() {
        using U = typename std::underlying_type<T>::type;
        return TypeFor<U>();
    }

    // deprectated
    template<typename T, bool B = std::is_enum<T>::value>
    inline static constexpr type_t typeFor() {
        return TypeFor<T, B>();
    }

    // constructors - implicit
    template<typename T>
    C2Value(T value)  : _mType(typeFor<T>()), _mValue(value) { }

    C2Value() : _mType(NO_INIT) { }

    inline type_t type() const { return _mType; }

    template<typename T>
    inline bool get(T *value) const {
        if (_mType == typeFor<T>()) {
            *value = _mValue.ref<T>();
            return true;
        }
        return false;
    }

    /// returns the address of the value
    void *get() const {
        return _mType == NO_INIT ? nullptr : (void*)&_mValue;
    }

    /// returns the size of the contained value
    size_t inline sizeOf() const {
        return SizeFor(_mType);
    }

    static size_t SizeFor(type_t type) {
        switch (type) {
            case INT32:
            case UINT32:
            case CNTR32: return sizeof(_mValue.i32);
            case INT64:
            case UINT64:
            case CNTR64: return sizeof(_mValue.i64);
            case FLOAT: return sizeof(_mValue.fp);
            default: return 0;
        }
    }

private:
    type_t _mType;
    Primitive _mValue;
};

template<> inline const int32_t &C2Value::Primitive::ref<int32_t>() const { return i32; }
template<> inline const int64_t &C2Value::Primitive::ref<int64_t>() const { return i64; }
template<> inline const uint32_t &C2Value::Primitive::ref<uint32_t>() const { return u32; }
template<> inline const uint64_t &C2Value::Primitive::ref<uint64_t>() const { return u64; }
template<> inline const c2_cntr32_t &C2Value::Primitive::ref<c2_cntr32_t>() const { return c32; }
template<> inline const c2_cntr64_t &C2Value::Primitive::ref<c2_cntr64_t>() const { return c64; }
template<> inline const float &C2Value::Primitive::ref<float>() const { return fp; }

// provide types for enums and uint8_t, char even though we don't provide reading as them
template<> constexpr C2Value::type_t C2Value::TypeFor<char, false>() { return INT32; }
template<> constexpr C2Value::type_t C2Value::TypeFor<int32_t, false>() { return INT32; }
template<> constexpr C2Value::type_t C2Value::TypeFor<int64_t, false>() { return INT64; }
template<> constexpr C2Value::type_t C2Value::TypeFor<uint8_t, false>() { return UINT32; }
template<> constexpr C2Value::type_t C2Value::TypeFor<uint32_t, false>() { return UINT32; }
template<> constexpr C2Value::type_t C2Value::TypeFor<uint64_t, false>() { return UINT64; }
template<> constexpr C2Value::type_t C2Value::TypeFor<c2_cntr32_t, false>() { return CNTR32; }
template<> constexpr C2Value::type_t C2Value::TypeFor<c2_cntr64_t, false>() { return CNTR64; }
template<> constexpr C2Value::type_t C2Value::TypeFor<float, false>() { return FLOAT; }

// forward declare easy enum template
template<typename E> struct C2EasyEnum;

/**
 * field descriptor. A field is uniquely defined by an index into a parameter.
 * (Note: Stream-id is not captured as a field.)
 *
 * Ordering of fields is by offset. In case of structures, it is depth first,
 * with a structure taking an index just before and in addition to its members.
 */
struct C2FieldDescriptor {
//public:
    /** field types and flags
     * \note: only 32-bit and 64-bit fields are supported (e.g. no boolean, as that
     * is represented using INT32).
     */
    enum type_t : uint32_t {
        // primitive types
        INT32   = C2Value::INT32,  ///< 32-bit signed integer
        UINT32  = C2Value::UINT32, ///< 32-bit unsigned integer
        CNTR32  = C2Value::CNTR32, ///< 32-bit counter
        INT64   = C2Value::INT64,  ///< 64-bit signed integer
        UINT64  = C2Value::UINT64, ///< 64-bit signed integer
        CNTR64  = C2Value::CNTR64, ///< 64-bit counter
        FLOAT   = C2Value::FLOAT,  ///< 32-bit floating point

        // array types
        STRING = 0x100, ///< fixed-size string (POD)
        BLOB,           ///< blob. Blobs have no sub-elements and can be thought of as byte arrays;
                        ///< however, bytes cannot be individually addressed by clients.

        // complex types
        STRUCT_FLAG = 0x20000, ///< structs. Marked with this flag in addition to their coreIndex.
    };

    typedef std::pair<C2String, C2Value::Primitive> NamedValueType;
    typedef std::vector<NamedValueType> NamedValuesType;
    //typedef std::pair<std::vector<C2String>, std::vector<C2Value::Primitive>> NamedValuesType;

    /**
     * Template specialization that returns the named values for a type.
     *
     * \todo hide from client.
     *
     * \return a vector of name-value pairs.
     */
    template<typename B>
    static NamedValuesType namedValuesFor(const B &);

    /** specialization for easy enums */
    template<typename E>
    inline static NamedValuesType namedValuesFor(const C2EasyEnum<E> &) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
        return namedValuesFor(*(E*)nullptr);
#pragma GCC diagnostic pop
    }

private:
    template<typename B, bool enabled=std::is_arithmetic<B>::value || std::is_enum<B>::value>
    struct C2_HIDE _NamedValuesGetter;

public:
    inline C2FieldDescriptor(uint32_t type, uint32_t extent, C2String name, size_t offset, size_t size)
        : _mType((type_t)type), _mExtent(extent), _mName(name), _mFieldId(offset, size) { }

    inline C2FieldDescriptor(const C2FieldDescriptor &) = default;

    template<typename T, class B=typename std::remove_extent<T>::type>
    inline C2FieldDescriptor(const T* offset, const char *name)
        : _mType(this->GetType((B*)nullptr)),
          _mExtent(std::is_array<T>::value ? std::extent<T>::value : 1),
          _mName(name),
          _mNamedValues(_NamedValuesGetter<B>::getNamedValues()),
          _mFieldId(offset) {}

    /// \deprecated
    template<typename T, typename S, class B=typename std::remove_extent<T>::type>
    inline C2FieldDescriptor(S*, T S::* field, const char *name)
        : _mType(this->GetType((B*)nullptr)),
          _mExtent(std::is_array<T>::value ? std::extent<T>::value : 1),
          _mName(name),
          _mFieldId(&(((S*)0)->*field)) {}

    /// returns the type of this field
    inline type_t type() const { return _mType; }
    /// returns the length of the field in case it is an array. Returns 0 for
    /// T[] arrays, returns 1 for T[1] arrays as well as if the field is not an array.
    inline size_t extent() const { return _mExtent; }
    /// returns the name of the field
    inline C2String name() const { return _mName; }

    const NamedValuesType &namedValues() const { return _mNamedValues; }

#if defined(FRIEND_TEST)
    friend void PrintTo(const C2FieldDescriptor &, ::std::ostream*);
    friend bool operator==(const C2FieldDescriptor &, const C2FieldDescriptor &);
    FRIEND_TEST(C2ParamTest_ParamFieldList, VerifyStruct);
#endif

private:
    /**
     * Construct an offseted field descriptor.
     */
    inline C2FieldDescriptor(const C2FieldDescriptor &desc, size_t offset)
        : _mType(desc._mType), _mExtent(desc._mExtent),
          _mName(desc._mName), _mNamedValues(desc._mNamedValues),
          _mFieldId(desc._mFieldId._mOffset + offset, desc._mFieldId._mSize) { }

    type_t _mType;
    uint32_t _mExtent; // the last member can be arbitrary length if it is T[] array,
                       // extending to the end of the parameter (this is marked with
                       // 0). T[0]-s are not fields.
    C2String _mName;
    NamedValuesType _mNamedValues;

    _C2FieldId _mFieldId;   // field identifier (offset and size)

    // NOTE: We do not capture default value(s) here as that may depend on the component.
    // NOTE: We also do not capture bestEffort, as 1) this should be true for most fields,
    // 2) this is at parameter granularity.

    // type resolution
    inline static type_t GetType(int32_t*)     { return INT32; }
    inline static type_t GetType(uint32_t*)    { return UINT32; }
    inline static type_t GetType(c2_cntr32_t*) { return CNTR32; }
    inline static type_t GetType(int64_t*)     { return INT64; }
    inline static type_t GetType(uint64_t*)    { return UINT64; }
    inline static type_t GetType(c2_cntr64_t*) { return CNTR64; }
    inline static type_t GetType(float*)       { return FLOAT; }
    inline static type_t GetType(char*)        { return STRING; }
    inline static type_t GetType(uint8_t*)     { return BLOB; }

    template<typename T,
             class=typename std::enable_if<std::is_enum<T>::value>::type>
    inline static type_t GetType(T*) {
        typename std::underlying_type<T>::type underlying(0);
        return GetType(&underlying);
    }

    // verify C2Struct by having a FieldList() and a CORE_INDEX.
    template<typename T,
             class=decltype(T::CORE_INDEX + 1), class=decltype(T::FieldList())>
    inline static type_t GetType(T*) {
        static_assert(!std::is_base_of<C2Param, T>::value, "cannot use C2Params as fields");
        return (type_t)(T::CORE_INDEX | STRUCT_FLAG);
    }

    friend struct _C2ParamInspector;
};

// no named values for compound types
template<typename B>
struct C2FieldDescriptor::_NamedValuesGetter<B, false> {
    inline static C2FieldDescriptor::NamedValuesType getNamedValues() {
        return NamedValuesType();
    }
};

template<typename B>
struct C2FieldDescriptor::_NamedValuesGetter<B, true> {
    inline static C2FieldDescriptor::NamedValuesType getNamedValues() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
        return C2FieldDescriptor::namedValuesFor(*(B*)nullptr);
#pragma GCC diagnostic pop
    }
};

#define DEFINE_NO_NAMED_VALUES_FOR(type) \
template<> inline C2FieldDescriptor::NamedValuesType C2FieldDescriptor::namedValuesFor(const type &) { \
    return NamedValuesType(); \
}

// We cannot subtype constructor for enumerated types so insted define no named values for
// non-enumerated integral types.
DEFINE_NO_NAMED_VALUES_FOR(int32_t)
DEFINE_NO_NAMED_VALUES_FOR(uint32_t)
DEFINE_NO_NAMED_VALUES_FOR(c2_cntr32_t)
DEFINE_NO_NAMED_VALUES_FOR(int64_t)
DEFINE_NO_NAMED_VALUES_FOR(uint64_t)
DEFINE_NO_NAMED_VALUES_FOR(c2_cntr64_t)
DEFINE_NO_NAMED_VALUES_FOR(uint8_t)
DEFINE_NO_NAMED_VALUES_FOR(char)
DEFINE_NO_NAMED_VALUES_FOR(float)

/**
 * Describes the fields of a structure.
 */
struct C2StructDescriptor {
public:
    /// Returns the core index of the struct
    inline C2Param::CoreIndex coreIndex() const { return _mType.coreIndex(); }

    // Returns the number of fields in this struct (not counting any recursive fields).
    // Must be at least 1 for valid structs.
    inline size_t numFields() const { return _mFields.size(); }

    // Returns the list of direct fields (not counting any recursive fields).
    typedef std::vector<C2FieldDescriptor>::const_iterator field_iterator;
    inline field_iterator cbegin() const { return _mFields.cbegin(); }
    inline field_iterator cend() const { return _mFields.cend(); }

    // only supplying const iterator - but these names are needed for range based loops
    inline field_iterator begin() const { return _mFields.cbegin(); }
    inline field_iterator end() const { return _mFields.cend(); }

    template<typename T>
    inline C2StructDescriptor(T*)
        : C2StructDescriptor(T::CORE_INDEX, T::FieldList()) { }

    inline C2StructDescriptor(
            C2Param::CoreIndex type,
            const std::vector<C2FieldDescriptor> &fields)
        : _mType(type), _mFields(fields) { }

private:
    friend struct _C2ParamInspector;

    inline C2StructDescriptor(
            C2Param::CoreIndex type,
            std::vector<C2FieldDescriptor> &&fields)
        : _mType(type), _mFields(std::move(fields)) { }

    const C2Param::CoreIndex _mType;
    const std::vector<C2FieldDescriptor> _mFields;
};

/**
 * Describes parameters for a component.
 */
struct C2ParamDescriptor {
public:
    /**
     * Returns whether setting this param is required to configure this component.
     * This can only be true for builtin params for platform-defined components (e.g. video and
     * audio encoders/decoders, video/audio filters).
     * For vendor-defined components, it can be true even for vendor-defined params,
     * but it is not recommended, in case the component becomes platform-defined.
     */
    inline bool isRequired() const { return _mAttrib & IS_REQUIRED; }

    /**
     * Returns whether this parameter is persistent. This is always true for C2Tuning and C2Setting,
     * but may be false for C2Info. If true, this parameter persists across frames and applies to
     * the current and subsequent frames. If false, this C2Info parameter only applies to the
     * current frame and is not assumed to have the same value (or even be present) on subsequent
     * frames, unless it is specified for those frames.
     */
    inline bool isPersistent() const { return _mAttrib & IS_PERSISTENT; }

    inline bool isStrict() const { return _mAttrib & IS_STRICT; }

    inline bool isReadOnly() const { return _mAttrib & IS_READ_ONLY; }

    inline bool isVisible() const { return !(_mAttrib & IS_HIDDEN); }

    inline bool isPublic() const { return !(_mAttrib & IS_INTERNAL); }

    /// Returns the name of this param.
    /// This defaults to the underlying C2Struct's name, but could be altered for a component.
    inline C2String name() const { return _mName; }

    /// Returns the parameter index
    inline C2Param::Index index() const { return _mIndex; }

    /// Returns the indices of parameters that this parameter has a dependency on
    inline const std::vector<C2Param::Index> &dependencies() const { return _mDependencies; }

    /// \deprecated
    template<typename T>
    inline C2ParamDescriptor(bool isRequired, C2StringLiteral name, const T*)
        : _mIndex(T::PARAM_TYPE),
          _mAttrib(IS_PERSISTENT | (isRequired ? IS_REQUIRED : 0)),
          _mName(name) { }

    /// \deprecated
    inline C2ParamDescriptor(
            bool isRequired, C2StringLiteral name, C2Param::Index index)
        : _mIndex(index),
          _mAttrib(IS_PERSISTENT | (isRequired ? IS_REQUIRED : 0)),
          _mName(name) { }

    enum attrib_t : uint32_t {
        // flags that default on
        IS_REQUIRED   = 1u << 0, ///< parameter is required to be specified
        IS_PERSISTENT = 1u << 1, ///< parameter retains its value
        // flags that default off
        IS_STRICT     = 1u << 2, ///< parameter is strict
        IS_READ_ONLY  = 1u << 3, ///< parameter is publicly read-only
        IS_HIDDEN     = 1u << 4, ///< parameter shall not be visible to clients
        IS_INTERNAL   = 1u << 5, ///< parameter shall not be used by framework (other than testing)
        IS_CONST      = 1u << 6 | IS_READ_ONLY, ///< parameter is publicly const (hence read-only)
    };

    inline C2ParamDescriptor(
        C2Param::Index index, attrib_t attrib, C2StringLiteral name)
        : _mIndex(index),
          _mAttrib(attrib),
          _mName(name) { }

    inline C2ParamDescriptor(
        C2Param::Index index, attrib_t attrib, C2String &&name,
        std::vector<C2Param::Index> &&dependencies)
        : _mIndex(index),
          _mAttrib(attrib),
          _mName(name),
          _mDependencies(std::move(dependencies)) { }

private:
    const C2Param::Index _mIndex;
    const uint32_t _mAttrib;
    const C2String _mName;
    std::vector<C2Param::Index> _mDependencies;

    friend struct _C2ParamInspector;
};

DEFINE_ENUM_OPERATORS(::C2ParamDescriptor::attrib_t)


/// \ingroup internal
/// Define a structure without CORE_INDEX.
/// \note _FIELD_LIST is used only during declaration so that C2Struct declarations can end with
/// a simple list of C2FIELD-s and closing bracket. Mark it unused as it is not used in templated
/// structs.
#define DEFINE_BASE_C2STRUCT(name) \
private: \
    const static std::vector<C2FieldDescriptor> _FIELD_LIST __unused; /**< structure fields */ \
public: \
    typedef C2##name##Struct _type; /**< type name shorthand */ \
    static const std::vector<C2FieldDescriptor> FieldList(); /**< structure fields factory */

/// Define a structure with matching CORE_INDEX.
#define DEFINE_C2STRUCT(name) \
public: \
    enum : uint32_t { CORE_INDEX = kParamIndex##name }; \
    DEFINE_BASE_C2STRUCT(name)

/// Define a flexible structure without CORE_INDEX.
#define DEFINE_BASE_FLEX_C2STRUCT(name, flexMember) \
public: \
    FLEX(C2##name##Struct, flexMember) \
    DEFINE_BASE_C2STRUCT(name)

/// Define a flexible structure with matching CORE_INDEX.
#define DEFINE_FLEX_C2STRUCT(name, flexMember) \
public: \
    FLEX(C2##name##Struct, flexMember) \
    enum : uint32_t { CORE_INDEX = kParamIndex##name | C2Param::CoreIndex::IS_FLEX_FLAG }; \
    DEFINE_BASE_C2STRUCT(name)

/// \ingroup internal
/// Describe a structure of a templated structure.
// Use list... as the argument gets resubsitituted and it contains commas. Alternative would be
// to wrap list in an expression, e.g. ({ std::vector<C2FieldDescriptor> list; })) which converts
// it from an initializer list to a vector.
#define DESCRIBE_TEMPLATED_C2STRUCT(strukt, list...) \
    _DESCRIBE_TEMPLATABLE_C2STRUCT(template<>, strukt, __C2_GENERATE_GLOBAL_VARS__, list)

/// \deprecated
/// Describe the fields of a structure using an initializer list.
#define DESCRIBE_C2STRUCT(name, list...) \
    _DESCRIBE_TEMPLATABLE_C2STRUCT(, C2##name##Struct, __C2_GENERATE_GLOBAL_VARS__, list)

/// \ingroup internal
/// Macro layer to get value of enabled that is passed in as a macro variable
#define _DESCRIBE_TEMPLATABLE_C2STRUCT(template, strukt, enabled, list...) \
    __DESCRIBE_TEMPLATABLE_C2STRUCT(template, strukt, enabled, list)

/// \ingroup internal
/// Macro layer to resolve to the specific macro based on macro variable
#define __DESCRIBE_TEMPLATABLE_C2STRUCT(template, strukt, enabled, list...) \
    ___DESCRIBE_TEMPLATABLE_C2STRUCT##enabled(template, strukt, list)

#define ___DESCRIBE_TEMPLATABLE_C2STRUCT(template, strukt, list...) \
    template \
    const std::vector<C2FieldDescriptor> strukt::FieldList() { return list; }

#define ___DESCRIBE_TEMPLATABLE_C2STRUCT__C2_GENERATE_GLOBAL_VARS__(template, strukt, list...)

/**
 * Describe a field of a structure.
 * These must be in order.
 *
 * There are two ways to use this macro:
 *
 *  ~~~~~~~~~~~~~ (.cpp)
 *  struct C2VideoWidthStruct {
 *      int32_t width;
 *      C2VideoWidthStruct() {} // optional default constructor
 *      C2VideoWidthStruct(int32_t _width) : width(_width) {}
 *
 *      DEFINE_AND_DESCRIBE_C2STRUCT(VideoWidth)
 *      C2FIELD(width, "width")
 *  };
 *  ~~~~~~~~~~~~~
 *
 *  ~~~~~~~~~~~~~ (.cpp)
 *  struct C2VideoWidthStruct {
 *      int32_t width;
 *      C2VideoWidthStruct() = default; // optional default constructor
 *      C2VideoWidthStruct(int32_t _width) : width(_width) {}
 *
 *      DEFINE_C2STRUCT(VideoWidth)
 *  } C2_PACK;
 *
 *  DESCRIBE_C2STRUCT(VideoWidth, {
 *      C2FIELD(width, "width")
 *  })
 *  ~~~~~~~~~~~~~
 *
 *  For flexible structures (those ending in T[]), use the flexible macros:
 *
 *  ~~~~~~~~~~~~~ (.cpp)
 *  struct C2VideoFlexWidthsStruct {
 *      int32_t widths[];
 *      C2VideoFlexWidthsStruct(); // must have a default constructor
 *
 *  private:
 *      // may have private constructors taking number of widths as the first argument
 *      // This is used by the C2Param factory methods, e.g.
 *      //   C2VideoFlexWidthsGlobalParam::AllocUnique(size_t, int32_t);
 *      C2VideoFlexWidthsStruct(size_t flexCount, int32_t value) {
 *          for (size_t i = 0; i < flexCount; ++i) {
 *              widths[i] = value;
 *          }
 *      }
 *
 *      // If the last argument is T[N] or std::initializer_list<T>, the flexCount will
 *      // be automatically calculated and passed by the C2Param factory methods, e.g.
 *      //   int widths[] = { 1, 2, 3 };
 *      //   C2VideoFlexWidthsGlobalParam::AllocUnique(widths);
 *      template<unsigned N>
 *      C2VideoFlexWidthsStruct(size_t flexCount, const int32_t(&init)[N]) {
 *          for (size_t i = 0; i < flexCount; ++i) {
 *              widths[i] = init[i];
 *          }
 *      }
 *
 *      DEFINE_AND_DESCRIBE_FLEX_C2STRUCT(VideoFlexWidths, widths)
 *      C2FIELD(widths, "widths")
 *  };
 *  ~~~~~~~~~~~~~
 *
 *  ~~~~~~~~~~~~~ (.cpp)
 *  struct C2VideoFlexWidthsStruct {
 *      int32_t mWidths[];
 *      C2VideoFlexWidthsStruct(); // must have a default constructor
 *
 *      DEFINE_FLEX_C2STRUCT(VideoFlexWidths, mWidths)
 *  } C2_PACK;
 *
 *  DESCRIBE_C2STRUCT(VideoFlexWidths, {
 *      C2FIELD(mWidths, "widths")
 *  })
 *  ~~~~~~~~~~~~~
 *
 */
#define DESCRIBE_C2FIELD(member, name) \
  C2FieldDescriptor(&((_type*)(nullptr))->member, name),

#define C2FIELD(member, name) _C2FIELD(member, name, __C2_GENERATE_GLOBAL_VARS__)
/// \if 0
#define _C2FIELD(member, name, enabled) __C2FIELD(member, name, enabled)
#define __C2FIELD(member, name, enabled) DESCRIBE_C2FIELD##enabled(member, name)
#define DESCRIBE_C2FIELD__C2_GENERATE_GLOBAL_VARS__(member, name)
/// \endif

/// Define a structure with matching CORE_INDEX and start describing its fields.
/// This must be at the end of the structure definition.
#define DEFINE_AND_DESCRIBE_C2STRUCT(name) \
    _DEFINE_AND_DESCRIBE_C2STRUCT(name, DEFINE_C2STRUCT, __C2_GENERATE_GLOBAL_VARS__)

/// Define a base structure (with no CORE_INDEX) and start describing its fields.
/// This must be at the end of the structure definition.
#define DEFINE_AND_DESCRIBE_BASE_C2STRUCT(name) \
    _DEFINE_AND_DESCRIBE_C2STRUCT(name, DEFINE_BASE_C2STRUCT, __C2_GENERATE_GLOBAL_VARS__)

/// Define a flexible structure with matching CORE_INDEX and start describing its fields.
/// This must be at the end of the structure definition.
#define DEFINE_AND_DESCRIBE_FLEX_C2STRUCT(name, flexMember) \
    _DEFINE_AND_DESCRIBE_FLEX_C2STRUCT( \
            name, flexMember, DEFINE_FLEX_C2STRUCT, __C2_GENERATE_GLOBAL_VARS__)

/// Define a flexible base structure (with no CORE_INDEX) and start describing its fields.
/// This must be at the end of the structure definition.
#define DEFINE_AND_DESCRIBE_BASE_FLEX_C2STRUCT(name, flexMember) \
    _DEFINE_AND_DESCRIBE_FLEX_C2STRUCT( \
            name, flexMember, DEFINE_BASE_FLEX_C2STRUCT, __C2_GENERATE_GLOBAL_VARS__)

/// \if 0
/*
   Alternate declaration of field definitions in case no field list is to be generated.
   The specific macro is chosed based on the value of __C2_GENERATE_GLOBAL_VARS__ (whether it is
   defined (to be empty) or not. This requires two level of macro substitution.
   TRICKY: use namespace declaration to handle closing bracket that is normally after
   these macros.
*/

#define _DEFINE_AND_DESCRIBE_C2STRUCT(name, defineMacro, enabled) \
    __DEFINE_AND_DESCRIBE_C2STRUCT(name, defineMacro, enabled)
#define __DEFINE_AND_DESCRIBE_C2STRUCT(name, defineMacro, enabled) \
    ___DEFINE_AND_DESCRIBE_C2STRUCT##enabled(name, defineMacro)
#define ___DEFINE_AND_DESCRIBE_C2STRUCT__C2_GENERATE_GLOBAL_VARS__(name, defineMacro) \
    defineMacro(name) } C2_PACK; namespace {
#define ___DEFINE_AND_DESCRIBE_C2STRUCT(name, defineMacro) \
    defineMacro(name) } C2_PACK; \
    const std::vector<C2FieldDescriptor> C2##name##Struct::FieldList() { return _FIELD_LIST; } \
    const std::vector<C2FieldDescriptor> C2##name##Struct::_FIELD_LIST = {

#define _DEFINE_AND_DESCRIBE_FLEX_C2STRUCT(name, flexMember, defineMacro, enabled) \
    __DEFINE_AND_DESCRIBE_FLEX_C2STRUCT(name, flexMember, defineMacro, enabled)
#define __DEFINE_AND_DESCRIBE_FLEX_C2STRUCT(name, flexMember, defineMacro, enabled) \
    ___DEFINE_AND_DESCRIBE_FLEX_C2STRUCT##enabled(name, flexMember, defineMacro)
#define ___DEFINE_AND_DESCRIBE_FLEX_C2STRUCT__C2_GENERATE_GLOBAL_VARS__(name, flexMember, defineMacro) \
    defineMacro(name, flexMember) } C2_PACK; namespace {
#define ___DEFINE_AND_DESCRIBE_FLEX_C2STRUCT(name, flexMember, defineMacro) \
    defineMacro(name, flexMember) } C2_PACK; \
    const std::vector<C2FieldDescriptor> C2##name##Struct::FieldList() { return _FIELD_LIST; } \
    const std::vector<C2FieldDescriptor> C2##name##Struct::_FIELD_LIST = {
/// \endif


/**
 * Parameter reflector class.
 *
 * This class centralizes the description of parameter structures. This can be shared
 * by multiple components as describing a parameter does not imply support of that
 * parameter. However, each supported parameter and any dependent structures within
 * must be described by the parameter reflector provided by a component.
 */
class C2ParamReflector {
public:
    /**
     *  Describes a parameter structure.
     *
     *  \param[in] coreIndex the core index of the parameter structure containing at least the
     *  core index
     *
     *  \return the description of the parameter structure
     *  \retval nullptr if the parameter is not supported by this reflector
     *
     *  This methods shall not block and return immediately.
     *
     *  \note this class does not take a set of indices because we would then prefer
     *  to also return any dependent structures, and we don't want this logic to be
     *  repeated in each reflector. Alternately, this could just return a map of all
     *  descriptions, but we want to conserve memory if client only wants the description
     *  of a few indices.
     */
    virtual std::unique_ptr<C2StructDescriptor> describe(C2Param::CoreIndex coreIndex) const = 0;

protected:
    virtual ~C2ParamReflector() = default;
};

/**
 * Generic supported values for a field.
 *
 * This can be either a range or a set of values. The range can be a simple range, an arithmetic,
 * geometric or multiply-accumulate series with a clear minimum and maximum value. Values can
 * be discrete values, or can optionally represent flags to be or-ed.
 *
 * \note Do not use flags to represent bitfields. Use individual values or separate fields instead.
 */
struct C2FieldSupportedValues {
//public:
    enum type_t {
        EMPTY,      ///< no supported values
        RANGE,      ///< a numeric range that can be continuous or discrete
        VALUES,     ///< a list of values
        FLAGS       ///< a list of flags that can be OR-ed
    };

    type_t type; /** Type of values for this field. */

    typedef C2Value::Primitive Primitive;

    /**
     * Range specifier for supported value. Used if type is RANGE.
     *
     * If step is 0 and num and denom are both 1, the supported values are any value, for which
     * min <= value <= max.
     *
     * Otherwise, the range represents a geometric/arithmetic/multiply-accumulate series, where
     * successive supported values can be derived from previous values (starting at min), using the
     * following formula:
     *  v[0] = min
     *  v[i] = v[i-1] * num / denom + step for i >= 1, while min < v[i] <= max.
     */
    struct {
        /** Lower end of the range (inclusive). */
        Primitive min;
        /** Upper end of the range (inclusive if permitted by series). */
        Primitive max;
        /** Step between supported values. */
        Primitive step;
        /** Numerator of a geometric series. */
        Primitive num;
        /** Denominator of a geometric series. */
        Primitive denom;
    } range;

    /**
     * List of values. Used if type is VALUES or FLAGS.
     *
     * If type is VALUES, this is the list of supported values in decreasing preference.
     *
     * If type is FLAGS, this vector contains { min-mask, flag1, flag2... }. Basically, the first
     * value is the required set of flags to be set, and the rest of the values are flags that can
     * be set independently. FLAGS is only supported for integral types. Supported flags should
     * not overlap, as it can make validation non-deterministic. The standard validation method
     * is that starting from the original value, if each flag is removed when fully present (the
     * min-mask must be fully present), we shall arrive at 0.
     */
    std::vector<Primitive> values;

    C2FieldSupportedValues()
        : type(EMPTY) {
    }

    template<typename T>
    C2FieldSupportedValues(T min, T max, T step = T(std::is_floating_point<T>::value ? 0 : 1))
        : type(RANGE),
          range{min, max, step, (T)1, (T)1} { }

    template<typename T>
    C2FieldSupportedValues(T min, T max, T num, T den) :
        type(RANGE),
        range{min, max, (T)0, num, den} { }

    template<typename T>
    C2FieldSupportedValues(T min, T max, T step, T num, T den)
        : type(RANGE),
          range{min, max, step, num, den} { }

    /// \deprecated
    template<typename T>
    C2FieldSupportedValues(bool flags, std::initializer_list<T> list)
        : type(flags ? FLAGS : VALUES),
          range{(T)0, (T)0, (T)0, (T)0, (T)0} {
        for (T value : list) {
            values.emplace_back(value);
        }
    }

    /// \deprecated
    template<typename T>
    C2FieldSupportedValues(bool flags, const std::vector<T>& list)
        : type(flags ? FLAGS : VALUES),
          range{(T)0, (T)0, (T)0, (T)0, (T)0} {
        for(T value : list) {
            values.emplace_back(value);
        }
    }

    /// \internal
    /// \todo: create separate values vs. flags initializer as for flags we want
    /// to list both allowed and required flags
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
    template<typename T, typename E=decltype(C2FieldDescriptor::namedValuesFor(*(T*)nullptr))>
    C2FieldSupportedValues(bool flags, const T*)
        : type(flags ? FLAGS : VALUES),
          range{(T)0, (T)0, (T)0, (T)0, (T)0} {
              C2FieldDescriptor::NamedValuesType named = C2FieldDescriptor::namedValuesFor(*(T*)nullptr);
        if (flags) {
            values.emplace_back(0); // min-mask defaults to 0
        }
        for (const C2FieldDescriptor::NamedValueType &item : named){
            values.emplace_back(item.second);
        }
    }
};
#pragma GCC diagnostic pop

/**
 * Supported values for a specific field.
 *
 * This is a pair of the field specifier together with an optional supported values object.
 * This structure is used when reporting parameter configuration failures and conflicts.
 */
struct C2ParamFieldValues {
    C2ParamField paramOrField; ///< the field or parameter
    /// optional supported values for the field if paramOrField specifies an actual field that is
    /// numeric (non struct, blob or string). Supported values for arrays (including string and
    /// blobs) describe the supported values for each element (character for string, and bytes for
    /// blobs). It is optional for read-only strings and blobs.
    std::unique_ptr<C2FieldSupportedValues> values;

    // This struct is meant to be move constructed.
    C2_DEFAULT_MOVE(C2ParamFieldValues);

    // Copy constructor/assignment is also provided as this object may get copied.
    C2ParamFieldValues(const C2ParamFieldValues &other)
        : paramOrField(other.paramOrField),
          values(other.values ? std::make_unique<C2FieldSupportedValues>(*other.values) : nullptr) { }

    C2ParamFieldValues& operator=(const C2ParamFieldValues &other) {
        paramOrField = other.paramOrField;
        values = other.values ? std::make_unique<C2FieldSupportedValues>(*other.values) : nullptr;
        return *this;
    }


    /**
     * Construct with no values.
     */
    C2ParamFieldValues(const C2ParamField &paramOrField_)
        : paramOrField(paramOrField_) { }

    /**
     * Construct with values.
     */
    C2ParamFieldValues(const C2ParamField &paramOrField_, const C2FieldSupportedValues &values_)
        : paramOrField(paramOrField_),
          values(std::make_unique<C2FieldSupportedValues>(values_)) { }

    /**
     * Construct from fields.
     */
    C2ParamFieldValues(const C2ParamField &paramOrField_, std::unique_ptr<C2FieldSupportedValues> &&values_)
        : paramOrField(paramOrField_),
          values(std::move(values_)) { }
};

/// @}

// include debug header for C2Params.h if C2Debug.h was already included
#ifdef C2UTILS_DEBUG_H_
#include <util/C2Debug-param.h>
#endif

#endif  // C2PARAM_H_
