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

#ifndef STAGEFRIGHT_FOUNDATION_FLAGGED_H_
#define STAGEFRIGHT_FOUNDATION_FLAGGED_H_

#include <media/stagefright/foundation/TypeTraits.h>

namespace android {

/**
 * Flagged<T, Flag> is basically a specialized std::pair<Flag, T> that automatically optimizes out
 * the flag if the wrapped type T is already flagged and we can combine the outer and inner flags.
 *
 * Flags can be queried/manipulated via flags() an setFlags(Flags). The wrapped value can be
 * accessed via get(). This template is meant to be inherited by other utility/wrapper classes
 * that need to store integral information along with the value.
 *
 * Users must specify the used bits (MASK) in the flags. Flag getters and setters will enforce this
 * mask. _Flagged_helper::minMask<Flag> is provided to easily calculate a mask for a max value.
 *
 * E.g. adding a safe flag can be achieved like this:
 *
 *
 * enum SafeFlags : uint32_t {
 *   kUnsafe,
 *   kSafe,
 *   kSafeMask = _Flagged_helper::minMask(kSafe),
 * };
 * typedef Flagged<int32_t, SafeFlags, kSafeMask> safeInt32;
 *
 * safeInt32 a;
 * a.setFlags(kSafe);
 * a.get() = 15;
 * EXPECT_EQ(a.flags(), kSafe);
 * EXPECT_EQ(a.get(), 15);
 *
 *
 * Flagged also supports lazy or calculated wrapping of already flagged types. Lazy wrapping is
 * provided automatically (flags are automatically shared if possible, e.g. mask is shifted
 * automatically to not overlap with used bits of the wrapped type's flags, and fall back to
 * unshared version of the template.):
 *
 * enum OriginFlags : uint32_t {
 *   kUnknown,
 *   kConst,
 *   kCalculated,
 *   kComponent,
 *   kApplication,
 *   kFile,
 *   kBinder,
 *   kOriginMask = _Flagged_helper::minMask(kBinder),
 * };
 * typedef Flagged<safeInt32, OriginFlags, kOriginMask>
 *          trackedSafeInt32;
 *
 * static_assert(sizeof(trackedSafeInt32) == sizeof(safeInt32), "");
 *
 * trackedSafeInt32 b(kConst, kSafe, 1);
 * EXPECT_EQ(b.flags(), kConst);
 * EXPECT_EQ(b.get().flags(), kSafe);
 * EXPECT_EQ(b.get().get(), 1);
 * b.setFlags(kCalculated);
 * b.get().setFlags(overflow ? kUnsafe : kSafe);
 *
 * One can also choose to share some flag-bits with the wrapped class:
 *
 * enum ValidatedFlags : uint32_t {
 *   kUnsafeV = kUnsafe,
 *   kSafeV = kSafe,
 *   kValidated = kSafe | 2,
 *   kSharedMaskV = kSafeMask,
 *   kValidatedMask = _Flagged_helper::minMask(kValidated),
 * };
 * typedef Flagged<safeInt32, ValidatedFlags, kValidatedMask, kSharedMaskV> validatedInt32;
 *
 * validatedInt32 v(kUnsafeV, kSafe, 10);
 * EXPECT_EQ(v.flags(), kUnsafeV);
 * EXPECT_EQ(v.get().flags(), kUnsafe);  // !kUnsafeV overrides kSafe
 * EXPECT_EQ(v.get().get(), 10);
 * v.setFlags(kValidated);
 * EXPECT_EQ(v.flags(), kValidated);
 * EXPECT_EQ(v.get().flags(), kSafe);
 * v.get().setFlags(kUnsafe);
 * EXPECT_EQ(v.flags(), 2);  // NOTE: sharing masks with enums allows strange situations to occur
 */

/**
 * Helper class for Flagged support. Encapsulates common utilities used by all
 * templated classes.
 */
struct _Flagged_helper {
    /**
     * Calculates the value with a given number of top-most bits set.
     *
     * This method may be called with a signed flag.
     *
     * \param num number of bits to set. This must be between 0 and the number of bits in Flag.
     *
     * \return the value where only the given number of top-most bits are set.
     */
    template<typename Flag>
    static constexpr Flag topBits(int num) {
        return Flag(num > 0 ?
                    ~((Flag(1) << (sizeof(Flag) * 8 - is_signed_integral<Flag>::value - num)) - 1) :
                    0);
    }

    /**
     * Calculates the minimum mask required to cover a value. Used with the maximum enum value for
     * an unsigned flag.
     *
     * \param maxValue maximum value to cover
     * \param shift DO NO USE. used internally
     *
     * \return mask that can be used that covers the maximum value.
     */
    template<typename Flag>
    static constexpr Flag minMask(Flag maxValue, int shift=sizeof(Flag) * 4) {
        static_assert(is_unsigned_integral<Flag>::value,
                      "this method only makes sense for unsigned flags");
        return shift ? minMask<Flag>(Flag(maxValue | (maxValue >> shift)), shift >> 1) : maxValue;
    }

    /**
     * Returns a value left-shifted by an argument as a potential constexpr.
     *
     * This method helps around the C-language limitation, when left-shift of a negative value with
     * even 0 cannot be a constexpr.
     *
     * \param value value to shift
     * \param shift amount of shift
     * \returns the shifted value as an integral type
     */
    template<typename Flag, typename IntFlag = typename underlying_integral_type<Flag>::type>
    static constexpr IntFlag lshift(Flag value, int shift) {
        return shift ? value << shift : value;
    }

private:

    /**
     * Determines whether mask can be combined with base-mask for a given left shift.
     *
     * \param mask desired mask
     * \param baseMask mask used by T or 0 if T is not flagged by Flag
     * \param sharedMask desired shared mask (if this is non-0, this must be mask & baseMask)
     * \param shift desired left shift to be used for mask
     * \param baseShift left shift used by T or 0 if T is not flagged by Flag
     * \param effectiveMask effective mask used by T or 0 if T is not flagged by Flag
     *
     * \return bool whether mask can be combined with baseMask using the desired values.
     */
    template<typename Flag, typename IntFlag=typename underlying_integral_type<Flag>::type>
    static constexpr bool canCombine(
            Flag mask, IntFlag baseMask, Flag sharedMask, int shift,
            int baseShift, IntFlag effectiveMask) {
        return
            // verify that shift is valid and mask can be shifted
            shift >= 0 && (mask & topBits<Flag>(shift)) == 0 &&

            // verify that base mask is part of effective mask (sanity check on arguments)
            (baseMask & ~(effectiveMask >> baseShift)) == 0 &&

            // if sharing masks, shift must be the base's shift.
            // verify that shared mask is the overlap of base mask and mask
            (sharedMask ?
                    ((sharedMask ^ (baseMask & mask)) == 0 &&
                     shift == baseShift) :


            // otherwise, verify that there is no overlap between mask and base's effective mask
                    (mask & (effectiveMask >> shift)) == 0);
    }


    /**
     * Calculates the minimum (left) shift required to combine a mask with the mask of an
     * underlying type (T, also flagged by Flag).
     *
     * \param mask desired mask
     * \param baseMask mask used by T or 0 if T is not flagged by Flag
     * \param sharedMask desired shared mask (if this is non-0, this must be mask & baseMask)
     * \param baseShift left shift used by T
     * \param effectiveMask effective mask used by T
     *
     * \return a non-negative minimum left shift value if mask can be combined with baseMask,
     *         or -1 if the masks cannot be combined. -2 if the input is invalid.
     */
    template<typename Flag,
             typename IntFlag = typename underlying_integral_type<Flag>::type>
    static constexpr int getShift(
            Flag mask, IntFlag baseMask, Flag sharedMask, int baseShift, IntFlag effectiveMask) {
        return
            // baseMask must be part of the effective mask
            (baseMask & ~(effectiveMask >> baseShift)) ? -2 :

            // if sharing masks, shift must be base's shift. verify that shared mask is part of
            // base mask and mask, and that desired mask still fits with base's shift value
            sharedMask ?
                    (canCombine(mask, baseMask, sharedMask, baseShift /* shift */,
                                baseShift, effectiveMask) ? baseShift : -1) :

            // otherwise, see if 0-shift works
            ((mask & effectiveMask) == 0) ? 0 :

            // otherwise, verify that mask can be shifted up
            ((mask & topBits<Flag>(1)) || (mask < 0)) ? -1 :

            incShift(getShift(Flag(mask << 1), baseMask /* unused */, sharedMask /* 0 */,
                              baseShift /* unused */, effectiveMask));
    }

    /**
     * Helper method that increments a non-negative (shift) value.
     *
     * This method is used to make it easier to create a constexpr for getShift.
     *
     * \param shift (shift) value to increment
     *
     * \return original shift if it was negative; otherwise, the shift incremented by one.
     */
    static constexpr int incShift(int shift) {
        return shift + (shift >= 0);
    }

#ifdef FRIEND_TEST
    FRIEND_TEST(FlaggedTest, _Flagged_helper_Test);
#endif

public:
    /**
     * Base class for all Flagged<T, Flag> classes.
     *
     * \note flagged types do not have a member variable for the mask used by the type. As such,
     * they should be be cast to this base class.
     *
     * \todo can we replace this base class check with a static member check to remove possibility
     * of cast?
     */
    template<typename Flag>
    struct base {};

    /**
     * Type support utility that retrieves the mask of a class (T) if it is a type flagged by
     * Flag (e.g. Flagged<T, Flag>).
     *
     * \note This retrieves 0 if T is a flagged class, that is not flagged by Flag or an equivalent
     * underlying type.
     *
     * Generic implementation for a non-flagged class.
     */
    template<
        typename T, typename Flag,
        bool=std::is_base_of<base<typename underlying_integral_type<Flag>::type>, T>::value>
    struct mask_of {
        using IntFlag = typename underlying_integral_type<Flag>::type;
        static constexpr IntFlag value = Flag(0); ///< mask of a potentially flagged class
        static constexpr int shift = 0; ///<left shift of flags in a potentially flagged class
        static constexpr IntFlag effective_value = IntFlag(0); ///<effective mask of flagged class
    };

    /**
     * Type support utility that calculates the minimum (left) shift required to combine a mask
     * with the mask of an underlying type T also flagged by Flag.
     *
     * \note if T is not flagged, not flagged by Flag, or the masks cannot be combined due to
     * incorrect sharing or the flags not having enough bits, the minimum is -1.
     *
     * \param MASK desired mask
     * \param SHARED_MASK desired shared mask (if this is non-0, T must be an type flagged by
     *        Flag with a mask that has exactly these bits common with MASK)
     */
    template<typename T, typename Flag, Flag MASK, Flag SHARED_MASK>
    struct min_shift {
        /// minimum (left) shift required, or -1 if masks cannot be combined
        static constexpr int value =
            getShift(MASK, mask_of<T, Flag>::value, SHARED_MASK,
                     mask_of<T, Flag>::shift, mask_of<T, Flag>::effective_value);
    };

    /**
     * Type support utility that calculates whether the flags of T can be combined with MASK.
     *
     * \param MASK desired mask
     * \param SHARED_MASK desired shared mask (if this is non-0, T MUST be an type flagged by
     *        Flag with a mask that has exactly these bits common with MASK)
     */
    template<
            typename T, typename Flag, Flag MASK,
            Flag SHARED_MASK=Flag(0),
            int SHIFT=min_shift<T, Flag, MASK, SHARED_MASK>::value>
    struct can_combine {
        using IntFlag = typename underlying_integral_type<Flag>::type;
        /// true if this mask can be combined with T's existing flag. false otherwise.
        static constexpr bool value =
                std::is_base_of<base<IntFlag>, T>::value
                        && canCombine(MASK, mask_of<T, Flag>::value, SHARED_MASK, SHIFT,
                                      mask_of<T, Flag>::shift, mask_of<T, Flag>::effective_value);
    };
};

/**
 * Template specialization for the case when T is flagged by Flag or a compatible type.
 */
template<typename T, typename Flag>
struct _Flagged_helper::mask_of<T, Flag, true> {
    using IntType = typename underlying_integral_type<Flag>::type;
    static constexpr IntType value = T::sFlagMask;
    static constexpr int shift = T::sFlagShift;
    static constexpr IntType effective_value = T::sEffectiveMask;
};

/**
 * Main Flagged template that adds flags to an object of another type (in essence, creates a pair)
 *
 * Flag must be an integral type (enums are allowed).
 *
 * \note We could make SHARED_MASK be a boolean as it must be either 0 or MASK & base's mask, but we
 * want it to be spelled out for safety.
 *
 * \param T type of object wrapped
 * \param Flag type of flag
 * \param MASK mask for the bits used in flag (before any shift)
 * \param SHARED_MASK optional mask to be shared with T (if this is not zero, SHIFT must be 0, and
 *        it must equal to MASK & T's mask)
 * \param SHIFT optional left shift for MASK to combine with T's mask (or -1, if masks should not
 *        be combined.)
 */
template<
        typename T, typename Flag, Flag MASK, Flag SHARED_MASK=(Flag)0,
        int SHIFT=_Flagged_helper::min_shift<T, Flag, MASK, SHARED_MASK>::value,
        typename IntFlag=typename underlying_integral_type<Flag>::type,
        bool=_Flagged_helper::can_combine<T, IntFlag, MASK, SHARED_MASK, SHIFT>::value>
class Flagged : public _Flagged_helper::base<IntFlag> {
    static_assert(SHARED_MASK == 0,
                  "shared mask can only be used with common flag types "
                  "and must be part of mask and mask of base type");
    static_assert((_Flagged_helper::topBits<Flag>(SHIFT) & MASK) == 0, "SHIFT overflows MASK");

    static constexpr Flag sFlagMask = MASK;  ///< the mask
    static constexpr int sFlagShift = SHIFT > 0 ? SHIFT : 0; ///< the left shift applied to flags

    friend struct _Flagged_helper;
#ifdef FRIEND_TEST
    static constexpr bool sFlagCombined = false;
    FRIEND_TEST(FlaggedTest, _Flagged_helper_Test);
#endif

    T       mValue; ///< wrapped value
    IntFlag mFlags; ///< flags

protected:
    /// The effective combined mask used by this class and any wrapped classes if the flags are
    /// combined.
    static constexpr IntFlag sEffectiveMask = _Flagged_helper::lshift(MASK, sFlagShift);

    /**
     * Helper method used by subsequent flagged wrappers to query flags. Returns the
     * flags for a particular mask and left shift.
     *
     * \param mask bitmask to use
     * \param shift left shifts to use
     *
     * \return the requested flags
     */
    inline constexpr IntFlag getFlagsHelper(IntFlag mask, int shift) const {
        return (mFlags >> shift) & mask;
    }

    /**
     * Helper method used by subsequent flagged wrappers to apply combined flags. Sets the flags
     * in the bitmask using a particulare left shift.
     *
     * \param mask bitmask to use
     * \param shift left shifts to use
     * \param flags flags to update (any flags within the bitmask are updated to their value in this
     *        argument)
     */
    inline void setFlagsHelper(IntFlag mask, int shift, IntFlag flags) {
        mFlags = Flag((mFlags & ~(mask << shift)) | ((flags & mask) << shift));
    }

public:
    /**
     * Wrapper around base class constructor. These take the flags as their first
     * argument and pass the rest of the arguments to the base class constructor.
     *
     * \param flags initial flags
     */
    template<typename ...Args>
    constexpr Flagged(Flag flags, Args... args)
        : mValue(std::forward<Args>(args)...),
          mFlags(Flag(_Flagged_helper::lshift(flags & sFlagMask, sFlagShift))) { }

    /** Gets the wrapped value as const. */
    inline constexpr const T &get() const { return mValue; }

    /** Gets the wrapped value. */
    inline T &get() { return mValue; }

    /** Gets the flags. */
    constexpr Flag flags() const {
        return Flag(getFlagsHelper(sFlagMask, sFlagShift));
    }

    /** Sets the flags. */
    void setFlags(Flag flags) {
        setFlagsHelper(sFlagMask, sFlagShift, flags);
    }
};

/*
 * TRICKY: we cannot implement the specialization as:
 *
 * class Flagged : base<Flag> {
 *    T value;
 * };
 *
 * Because T also inherits from base<Flag> and this runs into a compiler bug where
 * sizeof(Flagged) > sizeof(T).
 *
 * Instead, we must inherit directly from the wrapped class
 *
 */
#if 0
template<
        typename T, typename Flag, Flag MASK, Flag SHARED_MASK, int SHIFT>
class Flagged<T, Flag, MASK, SHARED_MASK, SHIFT, true> : public _Flagged_helper::base<Flag> {
private:
    T mValue;
};
#else
/**
 * Specialization for the case when T is derived from Flagged<U, Flag> and flags can be combined.
 */
template<
        typename T, typename Flag, Flag MASK, Flag SHARED_MASK, int SHIFT, typename IntFlag>
class Flagged<T, Flag, MASK, SHARED_MASK, SHIFT, IntFlag, true> : private T {
    static_assert(is_integral_or_enum<Flag>::value, "flag must be integer or enum");

    static_assert(SHARED_MASK == 0 || SHIFT == 0, "cannot overlap masks when using SHIFT");
    static_assert((SHARED_MASK & ~MASK) == 0, "shared mask must be part of the mask");
    static_assert((SHARED_MASK & ~T::sEffectiveMask) == 0,
                  "shared mask must be part of the base mask");
    static_assert(SHARED_MASK == 0 || (~SHARED_MASK & (MASK & T::sEffectiveMask)) == 0,
                  "mask and base mask can only overlap in shared mask");

    static constexpr Flag sFlagMask = MASK;  ///< the mask
    static constexpr int sFlagShift = SHIFT;  ///< the left shift applied to the flags

#ifdef FRIEND_TEST
    const static bool sFlagCombined = true;
    FRIEND_TEST(FlaggedTest, _Flagged_helper_Test);
#endif

protected:
    /// The effective combined mask used by this class and any wrapped classes if the flags are
    /// combined.
    static constexpr IntFlag sEffectiveMask = Flag((MASK << SHIFT) | T::sEffectiveMask);
    friend struct _Flagged_helper;

public:
    /**
     * Wrapper around base class constructor. These take the flags as their first
     * argument and pass the rest of the arguments to the base class constructor.
     *
     * \param flags initial flags
     */
    template<typename ...Args>
    constexpr Flagged(Flag flags, Args... args)
        : T(std::forward<Args>(args)...) {
        // we construct the base class first and apply the flags afterwards as
        // base class may not have a constructor that takes flags even if it is derived from
        // Flagged<U, Flag>
        setFlags(flags);
    }

    /** Gets the wrapped value as const. */
    inline constexpr T &get() const { return *this; }

    /** Gets the wrapped value. */
    inline T &get() { return *this; }

    /** Gets the flags. */
    Flag constexpr flags() const {
        return Flag(this->getFlagsHelper(sFlagMask, sFlagShift));
    }

    /** Sets the flags. */
    void setFlags(Flag flags) {
        this->setFlagsHelper(sFlagMask, sFlagShift, flags);
    }
};
#endif

}  // namespace android

#endif  // STAGEFRIGHT_FOUNDATION_FLAGGED_H_

