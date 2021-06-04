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

#ifndef STAGEFRIGHT_FOUNDATION_A_DATA_H_
#define STAGEFRIGHT_FOUNDATION_A_DATA_H_

#include <memory> // for std::shared_ptr, weak_ptr and unique_ptr
#include <type_traits> // for std::aligned_union

#include <utils/StrongPointer.h> // for android::sp and wp

#include <media/stagefright/foundation/TypeTraits.h>
#include <media/stagefright/foundation/Flagged.h>

#undef HIDE
#define HIDE __attribute__((visibility("hidden")))

namespace android {

/**
 * AData is a flexible union type that supports non-POD members. It supports arbitrary types as long
 * as they are either moveable or copyable.
 *
 * Internally, AData is using AUnion - a structure providing the union support. AUnion should not
 * be used by generic code as it is very unsafe - it opens type aliasing errors where an object of
 * one type can be easily accessed as an object of another type. AData prevents this.
 *
 * AData allows a custom type flagger to be used for future extensions (e.g. allowing automatic
 * type conversion). A strict and a relaxed flagger are provided as internal types.
 *
 * Use as follows:
 *
 * AData<int, float>::Basic data; // strict type support
 * int i = 1;
 * float f = 7.0f;
 *
 * data.set(5);
 * EXPECT_TRUE(data.find(&i));
 * EXPECT_FALSE(data.find(&f));
 * EXPECT_EQ(i, 5);
 *
 * data.set(6.0f);
 * EXPECT_FALSE(data.find(&i));
 * EXPECT_TRUE(data.find(&f));
 * EXPECT_EQ(f, 6.0f);
 *
 * AData<int, sp<RefBase>>::RelaxedBasic objdata; // relaxed type support
 * sp<ABuffer> buf = new ABuffer(16), buf2;
 * sp<RefBase> obj;
 *
 * objdata.set(buf);
 * EXPECT_TRUE(objdata.find(&buf2));
 * EXPECT_EQ(buf, buf2);
 * EXPECT_FALSE(objdata.find(&i));
 * EXPECT_TRUE(objdata.find(&obj));
 * EXPECT_TRUE(obj == buf);
 *
 * obj = buf;
 * objdata.set(obj); // storing as sp<RefBase>
 * EXPECT_FALSE(objdata.find(&buf2));  // not stored as ABuffer(!)
 * EXPECT_TRUE(objdata.find(&obj));
 */

/// \cond Internal

/**
 * Helper class to call constructor and destructor for a specific type in AUnion.
 * This class is needed as member function specialization is not allowed for a
 * templated class.
 */
struct HIDE _AUnion_impl {
    /**
     * Calls placement constuctor for type T with arbitrary arguments for a storage at an address.
     * Storage MUST be large enough to contain T.
     * Also clears the slack space after type T. \todo This is not technically needed, so we may
     * choose to do this just for debugging.
     *
     * \param totalSize size of the storage
     * \param addr      pointer to where object T should be constructed
     * \param args      arbitrary arguments for constructor
     */
    template<typename T, typename ...Args>
    inline static void emplace(size_t totalSize, T *addr, Args&&... args) {
        new(addr)T(std::forward<Args>(args)...);
        // clear slack space - this is not technically required
        constexpr size_t size = sizeof(T);
        memset(reinterpret_cast<uint8_t*>(addr) + size, 0, totalSize - size);
    }

    /**
     * Calls destuctor for an object of type T located at a specific address.
     *
     * \note we do not clear the storage in this case as the storage should not be used
     * until another object is placed there, at which case the storage will be cleared.
     *
     * \param addr    pointer to where object T is stored
     */
    template<typename T>
    inline static void del(T *addr) {
        addr->~T();
    }
};

/** Constructor specialization for void type */
template<>
HIDE inline void _AUnion_impl::emplace<void>(size_t totalSize, void *addr) {
    memset(addr, 0, totalSize);
}

/** Destructor specialization for void type */
template<>
HIDE inline void _AUnion_impl::del<void>(void *) {
}

/// \endcond

/**
 * A templated union class that can contain specific types of data, and provides
 * constructors, destructor and access methods strictly for those types.
 *
 * \note This class is VERY UNSAFE compared to a union, but it allows non-POD unions.
 * In particular care must be taken that methods are called in a careful order to
 * prevent accessing objects of one type as another type. This class provides no
 * facilities to help with this ordering. This is meant to be wrapped by safer
 * utility classes that do that.
 *
 * \param Ts types stored in this union.
 */
template<typename ...Ts>
struct AUnion {
private:
    using _type = typename std::aligned_union<0, Ts...>::type; ///< storage type
    _type mValue;                                              ///< storage

public:
    /**
     * Constructs an object of type T with arbitrary arguments in this union. After this call,
     * this union will contain this object.
     *
     * This method MUST be called only when either 1) no object or 2) a void object (equivalent to
     * no object) is contained in this union.
     *
     * \param T     type of object to be constructed. This must be one of the template parameters of
     *              the union class with the same cv-qualification, or void.
     * \param args  arbitrary arguments for the constructor
     */
    template<
            typename T, typename ...Args,
            typename=typename std::enable_if<is_one_of<T, void, Ts...>::value>::type>
    inline void emplace(Args&&... args) {
        _AUnion_impl::emplace(
                sizeof(_type), reinterpret_cast<T*>(&mValue), std::forward<Args>(args)...);
    }

    /**
     * Destructs an object of type T in this union. After this call, this union will contain no
     * object.
     *
     * This method MUST be called only when this union contains an object of type T.
     *
     * \param T     type of object to be destructed. This must be one of the template parameters of
     *              the union class with the same cv-qualification, or void.
     */
    template<
            typename T,
            typename=typename std::enable_if<is_one_of<T, void, Ts...>::value>::type>
    inline void del() {
        _AUnion_impl::del(reinterpret_cast<T*>(&mValue));
    }

    /**
     * Returns a const reference to the object of type T in this union.
     *
     * This method MUST be called only when this union contains an object of type T.
     *
     * \param T     type of object to be returned. This must be one of the template parameters of
     *              the union class with the same cv-qualification.
     */
    template<
            typename T,
            typename=typename std::enable_if<is_one_of<T, Ts...>::value>::type>
    inline const T &get() const {
        return *reinterpret_cast<const T*>(&mValue);
    }

    /**
     * Returns a reference to the object of type T in this union.
     *
     * This method MUST be called only when this union contains an object of type T.
     *
     * \param T     type of object to be returned. This must be one of the template parameters of
     *              the union class with the same cv-qualification.
     */
    template<typename T>
    inline T &get() {
        return *reinterpret_cast<T*>(&mValue);
    }
};

/**
 * Helper utility class that copies an object of type T to a destination.
 *
 * T must be copy assignable or copy constructible.
 *
 * It provides:
 *
 * void assign(T*, const U&) // for copiable types - this leaves the source unchanged, hence const.
 *
 * \param T type of object to assign to
 */
template<
        typename T,
        bool=std::is_copy_assignable<T>::value>
struct HIDE _AData_copier {
    static_assert(std::is_copy_assignable<T>::value, "T must be copy assignable here");

    /**
     * Copies src to data without modifying data.
     *
     * \param data pointer to destination
     * \param src source object
     */
    inline static void assign(T *data, const T &src) {
        *data = src;
    }

    template<typename U>
    using enable_if_T_is_same_as = typename std::enable_if<std::is_same<U, T>::value>::type;

    /**
     * Downcast specializations for sp<>, shared_ptr<> and weak_ptr<>
     */
    template<typename Tp, typename U, typename=enable_if_T_is_same_as<sp<Tp>>>
    inline static void assign(sp<Tp> *data, const sp<U> &src) {
        *data = static_cast<Tp*>(src.get());
    }

    template<typename Tp, typename U, typename=enable_if_T_is_same_as<wp<Tp>>>
    inline static void assign(wp<Tp> *data, const wp<U> &src) {
        sp<U> __tmp = src.promote();
        *data = static_cast<Tp*>(__tmp.get());
    }

    template<typename Tp, typename U, typename=enable_if_T_is_same_as<sp<Tp>>>
    inline static void assign(sp<Tp> *data, sp<U> &&src) {
        sp<U> __tmp = std::move(src); // move src out as get cannot
        *data = static_cast<Tp*>(__tmp.get());
    }

    template<typename Tp, typename U, typename=enable_if_T_is_same_as<std::shared_ptr<Tp>>>
    inline static void assign(std::shared_ptr<Tp> *data, const std::shared_ptr<U> &src) {
        *data = std::static_pointer_cast<Tp>(src);
    }

    template<typename Tp, typename U, typename=enable_if_T_is_same_as<std::shared_ptr<Tp>>>
    inline static void assign(std::shared_ptr<Tp> *data, std::shared_ptr<U> &&src) {
        std::shared_ptr<U> __tmp = std::move(src); // move src out as static_pointer_cast cannot
        *data = std::static_pointer_cast<Tp>(__tmp);
    }

    template<typename Tp, typename U, typename=enable_if_T_is_same_as<std::weak_ptr<Tp>>>
    inline static void assign(std::weak_ptr<Tp> *data, const std::weak_ptr<U> &src) {
        *data = std::static_pointer_cast<Tp>(src.lock());
    }

    // shared_ptrs are implicitly convertible to weak_ptrs but not vice versa, but picking the
    // first compatible type in Ts requires having shared_ptr types before weak_ptr types, so that
    // they are stored as shared_ptrs.
    /**
     * Provide sensible error message if encountering shared_ptr/weak_ptr ambiguity. This method
     * is not enough to detect this, only if someone is trying to find the shared_ptr.
     */
    template<typename Tp, typename U>
    inline static void assign(std::shared_ptr<Tp> *, const std::weak_ptr<U> &) {
        static_assert(std::is_same<Tp, void>::value,
                      "shared/weak pointer ambiguity. move shared ptr types before weak_ptrs");
    }
};

/**
 * Template specialization for non copy assignable, but copy constructible types.
 *
 * \todo Test this. No basic classes are copy constructible but not assignable.
 *
 */
template<typename T>
struct HIDE _AData_copier<T, false> {
    static_assert(!std::is_copy_assignable<T>::value, "T must not be copy assignable here");
    static_assert(std::is_copy_constructible<T>::value, "T must be copy constructible here");

    inline static void copy(T *data, const T &src) {
        data->~T();
        new(data)T(src);
    }
};

/**
 * Helper utility class that moves an object of type T to a destination.
 *
 * T must be move assignable or move constructible.
 *
 * It provides multiple methods:
 *
 * void assign(T*, T&&)
 *
 * \param T type of object to assign
 */
template<
        typename T,
        bool=std::is_move_assignable<T>::value>
struct HIDE _AData_mover {
    static_assert(std::is_move_assignable<T>::value, "T must be move assignable here");

    /**
     * Moves src to data while likely modifying it.
     *
     * \param data pointer to destination
     * \param src source object
     */
    inline static void assign(T *data, T &&src) {
        *data = std::move(src);
    }

    template<typename U>
    using enable_if_T_is_same_as = typename std::enable_if<std::is_same<U, T>::value>::type;

    /**
     * Downcast specializations for sp<>, shared_ptr<> and weak_ptr<>
     */
    template<typename Tp, typename U, typename=enable_if_T_is_same_as<sp<Tp>>>
    inline static void assign(sp<Tp> *data, sp<U> &&src) {
        sp<U> __tmp = std::move(src); // move src out as get cannot
        *data = static_cast<Tp*>(__tmp.get());
    }

    template<typename Tp, typename U, typename=enable_if_T_is_same_as<std::shared_ptr<Tp>>>
    inline static void assign(std::shared_ptr<Tp> *data, std::shared_ptr<U> &&src) {
        std::shared_ptr<U> __tmp = std::move(src); // move src out as static_pointer_cast cannot
        *data = std::static_pointer_cast<Tp>(__tmp);
    }

    template<typename Tp, typename Td, typename U, typename Ud,
            typename=enable_if_T_is_same_as<std::unique_ptr<Tp, Td>>>
    inline static void assign(std::unique_ptr<Tp, Td> *data, std::unique_ptr<U, Ud> &&src) {
        *data = std::unique_ptr<Tp, Td>(static_cast<Tp*>(src.release()));
    }

    // shared_ptrs are implicitly convertible to weak_ptrs but not vice versa, but picking the
    // first compatible type in Ts requires having shared_ptr types before weak_ptr types, so that
    // they are stored as shared_ptrs.
    /**
     * Provide sensible error message if encountering shared_ptr/weak_ptr ambiguity. This method
     * is not enough to detect this, only if someone is trying to remove the shared_ptr.
     */
    template<typename Tp, typename U>
    inline static void assign(std::shared_ptr<Tp> *, std::weak_ptr<U> &&) {
        static_assert(std::is_same<Tp, void>::value,
                      "shared/weak pointer ambiguity. move shared ptr types before weak_ptrs");
    }

    // unique_ptrs are implicitly convertible to shared_ptrs but not vice versa, but picking the
    // first compatible type in Ts requires having unique_ptrs types before shared_ptrs types, so
    // that they are stored as unique_ptrs.
    /**
     * Provide sensible error message if encountering shared_ptr/unique_ptr ambiguity. This method
     * is not enough to detect this, only if someone is trying to remove the unique_ptr.
     */
    template<typename Tp, typename U>
    inline static void assign(std::unique_ptr<Tp> *, std::shared_ptr<U> &&) {
        static_assert(std::is_same<Tp, void>::value,
                      "unique/shared pointer ambiguity. move unique ptr types before shared_ptrs");
    }
};

/**
 * Template specialization for non move assignable, but move constructible types.
 *
 * \todo Test this. No basic classes are move constructible but not assignable.
 *
 */
template<typename T>
struct HIDE _AData_mover<T, false> {
    static_assert(!std::is_move_assignable<T>::value, "T must not be move assignable here");
    static_assert(std::is_move_constructible<T>::value, "T must be move constructible here");

    inline static void assign(T *data, T &&src) {
        data->~T();
        new(data)T(std::move(src));
    }
};

/**
 * Helper template that deletes an object of a specific type (member) in an AUnion.
 *
 * \param Flagger type flagger class (see AData)
 * \param U AUnion object in which the member should be deleted
 * \param Ts types to consider for the member
 */
template<typename Flagger, typename U, typename ...Ts>
struct HIDE _AData_deleter;

/**
 * Template specialization when there are still types to consider (T and rest)
 */
template<typename Flagger, typename U, typename T, typename ...Ts>
struct HIDE _AData_deleter<Flagger, U, T, Ts...> {
    static bool del(typename Flagger::type flags, U &data) {
        if (Flagger::canDeleteAs(flags, Flagger::flagFor((T*)0))) {
            data.template del<T>();
            return true;
        }
        return _AData_deleter<Flagger, U, Ts...>::del(flags, data);
    }
};

/**
 * Template specialization when there are no more types to consider.
 */
template<typename Flagger, typename U>
struct HIDE _AData_deleter<Flagger, U> {
    inline static bool del(typename Flagger::type, U &) {
        return false;
    }
};

/**
 * Helper template that copy assigns an object of a specific type (member) in an
 * AUnion.
 *
 * \param Flagger type flagger class (see AData)
 * \param U AUnion object in which the member should be copy assigned
 * \param Ts types to consider for the member
 */
template<typename Flagger, typename U, typename ...Ts>
struct HIDE _AData_copy_assigner;

/**
 * Template specialization when there are still types to consider (T and rest)
 */
template<typename Flagger, typename U, typename T, typename ...Ts>
struct HIDE _AData_copy_assigner<Flagger, U, T, Ts...> {
    static bool assign(typename Flagger::type flags, U &dst, const U &src) {
        static_assert(std::is_copy_constructible<T>::value, "T must be copy constructible");
        // if we can delete as, we can also assign as
        if (Flagger::canDeleteAs(flags, Flagger::flagFor((T*)0))) {
            dst.template emplace<T>(src.template get<T>());
            return true;
        }
        return _AData_copy_assigner<Flagger, U, Ts...>::assign(flags, dst, src);
    }
};

/**
 * Template specialization when there are no more types to consider.
 */
template<typename Flagger, typename U>
struct HIDE _AData_copy_assigner<Flagger, U> {
    inline static bool assign(typename Flagger::type, U &, const U &) {
        return false;
    }
};

/**
 * Helper template that move assigns an object of a specific type (member) in an
 * AUnion.
 *
 * \param Flagger type flagger class (see AData)
 * \param U AUnion object in which the member should be copy assigned
 * \param Ts types to consider for the member
 */
template<typename Flagger, typename U, typename ...Ts>
struct HIDE _AData_move_assigner;

/**
 * Template specialization when there are still types to consider (T and rest)
 */
template<typename Flagger, typename U, typename T, typename ...Ts>
struct HIDE _AData_move_assigner<Flagger, U, T, Ts...> {
    template<typename V = T>
    static typename std::enable_if<std::is_move_constructible<V>::value, bool>::type
    assign(typename Flagger::type flags, U &dst, U &src) {
        // if we can delete as, we can also assign as
        if (Flagger::canDeleteAs(flags, Flagger::flagFor((T*)0))) {
            dst.template emplace<T>(std::move(src.template get<T>()));
            return true;
        }
        return _AData_move_assigner<Flagger, U, Ts...>::assign(flags, dst, src);
    }

    // Fall back to copy construction if T is not move constructible
    template<typename V = T>
    static typename std::enable_if<!std::is_move_constructible<V>::value, bool>::type
    assign(typename Flagger::type flags, U &dst, U &src) {
        static_assert(std::is_copy_constructible<T>::value, "T must be copy constructible");
        // if we can delete as, we can also assign as
        if (Flagger::canDeleteAs(flags, Flagger::flagFor((T*)0))) {
            dst.template emplace<T>(src.template get<T>());
            return true;
        }
        return _AData_move_assigner<Flagger, U, Ts...>::assign(flags, dst, src);
    }
};

/**
 * Template specialization when there are no more types to consider.
 */
template<typename Flagger, typename U>
struct HIDE _AData_move_assigner<Flagger, U> {
    inline static bool assign(typename Flagger::type, U &, U &) {
        return false;
    }
};

/**
 * Container that can store an arbitrary object of a set of specified types.
 *
 * This struct is an outer class that contains various inner classes based on desired type
 * strictness. The following inner classes are supported:
 *
 * AData<types...>::Basic   - strict type support using uint32_t flag.
 *
 * AData<types...>::Strict<Flag> - strict type support using custom flag.
 * AData<types...>::Relaxed<Flag, MaxSize, Align>
 *                          - relaxed type support with compatible (usually derived) class support
 *                            for pointer types with added size checking for minimal additional
 *                            safety.
 *
 * AData<types...>::RelaxedBasic - strict type support using uint32_t flag.
 *
 * AData<types...>::Custom<flagger> - custom type support (flaggers determine the supported types
 *                                    and the base type to use for each)
 *
 */
template<typename ...Ts>
struct AData {
private:
    static_assert(are_unique<Ts...>::value, "types must be unique");

    static constexpr size_t num_types = sizeof...(Ts); ///< number of types to support

public:
    /**
     * Default (strict) type flagger provided.
     *
     * The default flagger simply returns the index of the type within Ts, or 0 for void.
     *
     * Type flaggers return a flag for a supported type.
     *
     * They must provide:
     *
     * - a flagFor(T*) method for supported types _and_ for T=void. T=void is used to mark that no
     *   object is stored in the container. For this, an arbitrary unique value may be returned.
     * - a mask field that contains the flag mask.
     * - a canDeleteAs(Flag, Flag) flag comparison method that checks if a type of a flag can be
     *   deleted as another type.
     *
     * \param Flag the underlying unsigned integral to use for the flags.
     */
    template<typename Flag>
    struct flagger {
    private:
        static_assert(std::is_unsigned<Flag>::value, "Flag must be unsigned");
        static_assert(std::is_integral<Flag>::value, "Flag must be an integral type");

        static constexpr Flag count = num_types + 1;

    public:
        typedef Flag type; ///< flag type

        static constexpr Flag mask = _Flagged_helper::minMask<Flag>(count); ///< flag mask

        /**
         * Return the stored type for T. This is itself.
         */
        template<typename T>
        struct store {
            typedef T as_type; ///< the base type that T is stored as
        };

        /**
         * Constexpr method that returns if two flags are compatible for deletion.
         *
         * \param objectFlag flag for object to be deleted
         * \param deleteFlag flag for type that object is to be deleted as
         */
        static constexpr bool canDeleteAs(Flag objectFlag, Flag deleteFlag) {
            // default flagger requires strict type equality
            return objectFlag == deleteFlag;
        }

        /**
         * Constexpr method that returns the flag to use for a given type.
         *
         * Function overload for void*.
         */
        static constexpr Flag flagFor(void*) {
            return 0u;
        }

        /**
         * Constexpr method that returns the flag to use for a given supported type (T).
         */
        template<typename T, typename=typename std::enable_if<is_one_of<T, Ts...>::value>::type>
        static constexpr Flag flagFor(T*) {
            return find_first<T, Ts...>::index;
        }
    };

    /**
     * Relaxed flagger returns the index of the type within Ts. However, for pointers T* it returns
     * the first type in Ts that T* can be converted into (this is normally a base type, but also
     * works for sp<>, shared_ptr<> or unique_ptr<>). For a bit more strictness, the flag also
     * contains the size of the class to avoid finding objects that were stored as a different
     * derived class of the same base class.
     *
     * Flag is basically the index of the (base) type in Ts multiplied by the max size stored plus
     * the size of the type (divided by alignment) for derived pointer types.
     *
     * \param MaxSize max supported size for derived class pointers
     * \param Align alignment to assume for derived class pointers
     */
    template<typename Flag, size_t MaxSize=1024, size_t Align=4>
    struct relaxed_flagger {
    private:
        static_assert(std::is_unsigned<Flag>::value, "Flag must be unsigned");
        static_assert(std::is_integral<Flag>::value, "Flag must be an integral type");

        static constexpr Flag count = num_types + 1;
        static_assert(std::numeric_limits<Flag>::max() / count > (MaxSize / Align),
                      "not enough bits to fit into flag");

        static constexpr Flag max_size_stored = MaxSize / Align + 1;

        // T can be converted if it's size is <= MaxSize and it can be converted to one of the Ts
        template<typename T, size_t size>
        using enable_if_can_be_converted = typename std::enable_if<
                (size / Align < max_size_stored
                        && find_first_convertible_to<T, Ts...>::index)>::type;


        template<typename W, typename T, typename=enable_if_can_be_converted<W, sizeof(T)>>
        static constexpr Flag relaxedFlagFor(W*, T*) {
            return find_first_convertible_to<W, Ts...>::index * max_size_stored
                    + (is_one_of<W, Ts...>::value ? 0 : (sizeof(T) / Align));
        }

    public:
        typedef Flag type; ///< flag type

        static constexpr Flag mask =
            _Flagged_helper::minMask<Flag>(count * max_size_stored); ///< flag mask

        /**
         * Constexpr method that returns if two flags are compatible for deletion.
         *
         * \param objectFlag flag for object to be deleted
         * \param deleteFlag flag for type that object is to be deleted as
         */
        static constexpr bool canDeleteAs(Flag objectFlag, Flag deleteFlag) {
            // can delete if objects have the same base type
            return
                objectFlag / max_size_stored == deleteFlag / max_size_stored &&
                (deleteFlag % max_size_stored) == 0;
        }

        /**
         * Constexpr method that returns the flag to use for a given type.
         *
         * Function overload for void*.
         */
        static constexpr Flag flagFor(void*) {
            return 0u;
        }

        /**
         * Constexpr method that returns the flag to use for a given supported type (T).
         *
         * This is a member method to enable both overloading as well as template specialization.
         */
        template<typename T, typename=typename std::enable_if<is_one_of<T, Ts...>::value>::type>
        static constexpr Flag flagFor(T*) {
            return find_first<T, Ts...>::index * max_size_stored;
        }

        /**
         * For precaution, we only consider converting pointers to their base classes.
         */

        /**
         * Template specialization for derived class pointers and managed pointers.
         */
        template<typename T>
        static constexpr Flag flagFor(T**p) { return relaxedFlagFor(p, (T*)0); }
        template<typename T>
        static constexpr Flag flagFor(std::shared_ptr<T>*p) { return relaxedFlagFor(p, (T*)0); }
        template<typename T>
        static constexpr Flag flagFor(std::unique_ptr<T>*p) { return relaxedFlagFor(p, (T*)0); }
        template<typename T>
        static constexpr Flag flagFor(std::weak_ptr<T>*p) { return relaxedFlagFor(p, (T*)0); }
        template<typename T>
        static constexpr Flag flagFor(sp<T>*p) { return relaxedFlagFor(p, (T*)0); }
        template<typename T>
        static constexpr Flag flagFor(wp<T>*p) { return relaxedFlagFor(p, (T*)0); }

        /**
         * Type support template that provodes the stored type for T.
         * This is itself if it is one of Ts, or the first type in Ts that T is convertible to.
         *
         * NOTE: This template may provide a base class for an unsupported type. Support is
         * determined by flagFor().
         */
        template<typename T>
        struct store {
            typedef typename std::conditional<
                    is_one_of<T, Ts...>::value,
                    T,
                    typename find_first_convertible_to<T, Ts...>::type>::type as_type;
        };
    };

    /**
     * Implementation of AData.
     */
    template<typename Flagger>
    struct Custom : protected Flagged<AUnion<Ts...>, typename Flagger::type, Flagger::mask> {
        using data_t = AUnion<Ts...>;
        using base_t = Flagged<AUnion<Ts...>, typename Flagger::type, Flagger::mask>;

        /**
         * Constructor. Initializes this to a container that does not contain any object.
         */
        Custom() : base_t(Flagger::flagFor((void*)0)) { }

        /**
         * Copy assignment operator.
         */
        Custom& operator=(const Custom &o) {
            if (&o != this) {
                if (this->used() && !this->clear()) {
                    __builtin_trap();
                }
                if (o.used()) {
                    if (_AData_copy_assigner<Flagger, data_t, Ts...>::assign(
                            o.flags(), this->get(), o.get())) {
                        this->setFlags(o.flags());
                    } else {
                        __builtin_trap();
                    }
                }
            }
            return *this;
        }

        /**
         * Copy constructor.
         */
        Custom(const Custom &o) : Custom() {
            *this = o;
        }

        /**
         * Move assignment operator.
         */
        Custom& operator=(Custom &&o) noexcept {
            if (&o != this) {
                if (this->used() && !this->clear()) {
                    __builtin_trap();
                }
                if (o.used()) {
                    if (_AData_move_assigner<Flagger, data_t, Ts...>::assign(
                            o.flags(), this->get(), o.get())) {
                        this->setFlags(o.flags());
                        o.clear();
                    } else {
                        __builtin_trap();
                    }
                }
            }
            return *this;
        }

        /**
         * Move constructor.
         */
        Custom(Custom &&o) noexcept : Custom() {
            *this = std::move(o);
        }

        /**
         * Removes the contained object, if any.
         */
        ~Custom() {
            if (!this->clear()) {
                __builtin_trap();
                // std::cerr << "could not delete data of type " << this->flags() << std::endl;
            }
        }

        /**
         * Returns whether there is any object contained.
         */
        inline bool used() const {
            return this->flags() != Flagger::flagFor((void*)0);
        }

        /**
         * Removes the contained object, if any. Returns true if there are no objects contained,
         * or false on any error (this is highly unexpected).
         */
        bool clear() {
            if (this->used()) {
                if (_AData_deleter<Flagger, data_t, Ts...>::del(this->flags(), this->get())) {
                    this->setFlags(Flagger::flagFor((void*)0));
                    return true;
                }
                return false;
            }
            return true;
        }

        template<typename T>
        using is_supported_by_flagger =
            typename std::enable_if<Flagger::flagFor((T*)0) != Flagger::flagFor((void*)0)>::type;

        /**
         * Checks if there is a copiable object of type T in this container. If there is, it copies
         * that object into the provided address and returns true. Otherwise, it does nothing and
         * returns false.
         *
         * This method normally requires a flag equality between the stored and retrieved types.
         * However, it also allows retrieving the stored object as the stored type
         * (usually base type).
         *
         * \param T type of object to sought
         * \param data address at which the object should be retrieved
         *
         * \return true if the object was retrieved. false if it was not.
         */
        template<
                typename T,
                typename=is_supported_by_flagger<T>>
        bool find(T *data) const {
            using B = typename Flagger::template store<T>::as_type;
            if (this->flags() == Flagger::flagFor((T*)0) ||
                Flagger::canDeleteAs(this->flags(), Flagger::flagFor((T*)0))) {
                _AData_copier<T>::assign(data, this->get().template get<B>());
                return true;
            }
            return false;
        }

        /**
         * Checks if there is an object of type T in this container. If there is, it moves that
         * object into the provided address and returns true. Otherwise, it does nothing and returns
         * false.
         *
         * This method normally requires a flag equality between the stored and retrieved types.
         * However, it also allows retrieving the stored object as the stored type
         * (usually base type).
         *
         * \param T type of object to sought
         * \param data address at which the object should be retrieved.
         *
         * \return true if the object was retrieved. false if it was not.
         */
        template<
                typename T,
                typename=is_supported_by_flagger<T>>
        bool remove(T *data) {
            using B = typename Flagger::template store<T>::as_type;
            if (this->flags() == Flagger::flagFor((T*)0) ||
                Flagger::canDeleteAs(this->flags(), Flagger::flagFor((T*)0))) {
                _AData_mover<T>::assign(data, std::move(this->get().template get<B>()));
                return true;
            }
            return false;
        }

        /**
         * Stores an object into this container by copying. If it was successful, returns true.
         * Otherwise, (e.g. it could not destroy the already stored object) it returns false. This
         * latter would be highly unexpected.
         *
         * \param T type of object to store
         * \param data object to store
         *
         * \return true if the object was stored. false if it was not.
         */
        template<
                typename T,
                typename=is_supported_by_flagger<T>,
                typename=typename std::enable_if<
                        std::is_copy_constructible<T>::value ||
                        (std::is_default_constructible<T>::value &&
                                std::is_copy_assignable<T>::value)>::type>
        bool set(const T &data) {
            using B = typename Flagger::template store<T>::as_type;

            // if already contains an object of this type, simply assign
            if (this->flags() == Flagger::flagFor((T*)0) && std::is_same<T, B>::value) {
                _AData_copier<B>::assign(&this->get().template get<B>(), data);
                return true;
            } else if (this->used()) {
                // destroy previous object
                if (!this->clear()) {
                    return false;
                }
            }
            this->get().template emplace<B>(data);
            this->setFlags(Flagger::flagFor((T *)0));
            return true;
        }

        /**
         * Moves an object into this container. If it was successful, returns true. Otherwise,
         * (e.g. it could not destroy the already stored object) it returns false. This latter
         * would be highly unexpected.
         *
         * \param T type of object to store
         * \param data object to store
         *
         * \return true if the object was stored. false if it was not.
         */
        template<
                typename T,
                typename=is_supported_by_flagger<T>>
        bool set(T &&data) {
            using B = typename Flagger::template store<T>::as_type;

            // if already contains an object of this type, simply assign
            if (this->flags() == Flagger::flagFor((T*)0) && std::is_same<T, B>::value) {
                _AData_mover<B>::assign(&this->get().template get<B>(), std::forward<T&&>(data));
                return true;
            } else if (this->used()) {
                // destroy previous object
                if (!this->clear()) {
                    return false;
                }
            }
            this->get().template emplace<B>(std::forward<T&&>(data));
            this->setFlags(Flagger::flagFor((T *)0));
            return true;
        }
    };

    /**
     * Basic AData using the default type flagger and requested flag type.
     *
     * \param Flag desired flag type to use. Must be an unsigned and std::integral type.
     */
    template<typename Flag>
    using Strict = Custom<flagger<Flag>>;

    /**
     * Basic AData using the default type flagger and uint32_t flag.
     */
    using Basic = Strict<uint32_t>;

    /**
     * AData using the relaxed type flagger for max size and requested flag type.
     *
     * \param Flag desired flag type to use. Must be an unsigned and std::integral type.
     */
    template<typename Flag, size_t MaxSize = 1024, size_t Align = 4>
    using Relaxed = Custom<relaxed_flagger<Flag, MaxSize, Align>>;

    /**
     * Basic AData using the relaxed type flagger and uint32_t flag.
     */
    using RelaxedBasic = Relaxed<uint32_t>;
};

}  // namespace android

#endif  // STAGEFRIGHT_FOUNDATION_A_DATA_H_

