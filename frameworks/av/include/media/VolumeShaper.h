/*
 * Copyright 2017 The Android Open Source Project
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

#ifndef ANDROID_VOLUME_SHAPER_H
#define ANDROID_VOLUME_SHAPER_H

#include <cmath>
#include <list>
#include <math.h>
#include <sstream>

#include <binder/Parcel.h>
#include <media/Interpolator.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>

#pragma push_macro("LOG_TAG")
#undef LOG_TAG
#define LOG_TAG "VolumeShaper"

// turn on VolumeShaper logging
#define VS_LOGGING 0
#define VS_LOG(...) ALOGD_IF(VS_LOGGING, __VA_ARGS__)

namespace android {

namespace media {

// The native VolumeShaper class mirrors the java VolumeShaper class;
// in addition, the native class contains implementation for actual operation.
//
// VolumeShaper methods are not safe for multiple thread access.
// Use VolumeHandler for thread-safe encapsulation of multiple VolumeShapers.
//
// Classes below written are to avoid naked pointers so there are no
// explicit destructors required.

class VolumeShaper {
public:
    // S and T are like template typenames (matching the Interpolator<S, T>)
    using S = float; // time type
    using T = float; // volume type

// Curve and dimension information
// TODO: member static const or constexpr float initialization not permitted in C++11
#define MIN_CURVE_TIME    0.f  // type S: start of VolumeShaper curve (normalized)
#define MAX_CURVE_TIME    1.f  // type S: end of VolumeShaper curve (normalized)
#define MIN_LINEAR_VOLUME 0.f  // type T: silence / mute audio
#define MAX_LINEAR_VOLUME 1.f  // type T: max volume, unity gain
#define MAX_LOG_VOLUME    0.f  // type T: max volume, unity gain in dBFS

    /* kSystemVolumeShapersMax is the maximum number of system VolumeShapers.
     * Each system VolumeShapers has a predefined Id, which ranges from 0
     * to kSystemVolumeShapersMax - 1 and is unique for its usage.
     *
     * "1" is reserved for system ducking.
     */
    static const int kSystemVolumeShapersMax = 16;

    /* kUserVolumeShapersMax is the maximum number of application
     * VolumeShapers for a player/track.  Application VolumeShapers are
     * assigned on creation by the client, and have Ids ranging
     * from kSystemVolumeShapersMax to INT32_MAX.
     *
     * The number of user/application volume shapers is independent to the
     * system volume shapers. If an application tries to create more than
     * kUserVolumeShapersMax to a player, then the apply() will fail.
     * This prevents exhausting server side resources by a potentially malicious
     * application.
     */
    static const int kUserVolumeShapersMax = 16;

    /* VolumeShaper::Status is equivalent to status_t if negative
     * but if non-negative represents the id operated on.
     * It must be expressible as an int32_t for binder purposes.
     */
    using Status = status_t;

    // Local definition for clamp as std::clamp is included in C++17 only.
    // TODO: use the std::clamp version when Android build uses C++17.
    template<typename R>
    static constexpr const R &clamp(const R &v, const R &lo, const R &hi) {
        return (v < lo) ? lo : (hi < v) ? hi : v;
    }

    /* VolumeShaper.Configuration derives from the Interpolator class and adds
     * parameters relating to the volume shape.
     *
     * This parallels the Java implementation and the enums must match.
     * See "frameworks/base/media/java/android/media/VolumeShaper.java" for
     * details on the Java implementation.
     */
    class Configuration : public Interpolator<S, T>, public RefBase, public Parcelable {
    public:
        // Must match with VolumeShaper.java in frameworks/base.
        enum Type : int32_t {
            TYPE_ID,
            TYPE_SCALE,
        };

        // Must match with VolumeShaper.java in frameworks/base.
        enum OptionFlag : int32_t {
            OPTION_FLAG_NONE           = 0,
            OPTION_FLAG_VOLUME_IN_DBFS = (1 << 0),
            OPTION_FLAG_CLOCK_TIME     = (1 << 1),

            OPTION_FLAG_ALL            = (OPTION_FLAG_VOLUME_IN_DBFS | OPTION_FLAG_CLOCK_TIME),
        };

        // Bring from base class; must match with VolumeShaper.java in frameworks/base.
        using InterpolatorType = Interpolator<S, T>::InterpolatorType;

        Configuration()
            : Interpolator<S, T>()
            , RefBase()
            , mType(TYPE_SCALE)
            , mId(-1)
            , mOptionFlags(OPTION_FLAG_NONE)
            , mDurationMs(1000.) {
        }

        explicit Configuration(const Configuration &configuration)
            : Interpolator<S, T>(*static_cast<const Interpolator<S, T> *>(&configuration))
            , RefBase()
            , mType(configuration.mType)
            , mId(configuration.mId)
            , mOptionFlags(configuration.mOptionFlags)
            , mDurationMs(configuration.mDurationMs) {
        }

        Type getType() const {
            return mType;
        }

        status_t setType(Type type) {
            switch (type) {
            case TYPE_ID:
            case TYPE_SCALE:
                mType = type;
                return NO_ERROR;
            default:
                ALOGE("invalid Type: %d", type);
                return BAD_VALUE;
            }
        }

        OptionFlag getOptionFlags() const {
            return mOptionFlags;
        }

        status_t setOptionFlags(OptionFlag optionFlags) {
            if ((optionFlags & ~OPTION_FLAG_ALL) != 0) {
                ALOGE("optionFlags has invalid bits: %#x", optionFlags);
                return BAD_VALUE;
            }
            mOptionFlags = optionFlags;
            return NO_ERROR;
        }

        double getDurationMs() const {
            return mDurationMs;
        }

        status_t setDurationMs(double durationMs) {
            if (durationMs > 0.) {
                mDurationMs = durationMs;
                return NO_ERROR;
            }
            // zero, negative, or nan. These values not possible from Java.
            return BAD_VALUE;
        }

        int32_t getId() const {
            return mId;
        }

        void setId(int32_t id) {
            // We permit a negative id here (representing invalid).
            mId = id;
        }

        /* Adjust the volume to be in linear range from MIN_LINEAR_VOLUME to MAX_LINEAR_VOLUME
         * and compensate for log dbFS volume as needed.
         */
        T adjustVolume(T volume) const {
            if ((getOptionFlags() & OPTION_FLAG_VOLUME_IN_DBFS) != 0) {
                const T out = powf(10.f, volume / 10.f);
                VS_LOG("in: %f  out: %f", volume, out);
                volume = out;
            }
            return clamp(volume, MIN_LINEAR_VOLUME /* lo */, MAX_LINEAR_VOLUME /* hi */);
        }

        /* Check if the existing curve is valid.
         */
        status_t checkCurve() const {
            if (mType == TYPE_ID) return NO_ERROR;
            if (this->size() < 2) {
                ALOGE("curve must have at least 2 points");
                return BAD_VALUE;
            }
            if (first().first != MIN_CURVE_TIME || last().first != MAX_CURVE_TIME) {
                ALOGE("curve must start at MIN_CURVE_TIME and end at MAX_CURVE_TIME");
                return BAD_VALUE;
            }
            if ((getOptionFlags() & OPTION_FLAG_VOLUME_IN_DBFS) != 0) {
                for (const auto &pt : *this) {
                    if (!(pt.second <= MAX_LOG_VOLUME) /* handle nan */) {
                        ALOGE("positive volume dbFS");
                        return BAD_VALUE;
                    }
                }
            } else {
                for (const auto &pt : *this) {
                    if (!(pt.second >= MIN_LINEAR_VOLUME)
                            || !(pt.second <= MAX_LINEAR_VOLUME) /* handle nan */) {
                        ALOGE("volume < MIN_LINEAR_VOLUME or > MAX_LINEAR_VOLUME");
                        return BAD_VALUE;
                    }
                }
            }
            return NO_ERROR;
        }

        /* Clamps the volume curve in the configuration to
         * the valid range for log or linear scale.
         */
        void clampVolume() {
            if ((mOptionFlags & OPTION_FLAG_VOLUME_IN_DBFS) != 0) {
                for (auto it = this->begin(); it != this->end(); ++it) {
                    if (!(it->second <= MAX_LOG_VOLUME) /* handle nan */) {
                        it->second = MAX_LOG_VOLUME;
                    }
                }
            } else {
                for (auto it = this->begin(); it != this->end(); ++it) {
                    if (!(it->second >= MIN_LINEAR_VOLUME) /* handle nan */) {
                        it->second = MIN_LINEAR_VOLUME;
                    } else if (!(it->second <= MAX_LINEAR_VOLUME)) {
                        it->second = MAX_LINEAR_VOLUME;
                    }
                }
            }
        }

        /* scaleToStartVolume() is used to set the start volume of a
         * new VolumeShaper curve, when replacing one VolumeShaper
         * with another using the "join" (volume match) option.
         *
         * It works best for monotonic volume ramps or ducks.
         */
        void scaleToStartVolume(T volume) {
            if (this->size() < 2) {
                return;
            }
            const T startVolume = first().second;
            const T endVolume = last().second;
            if (endVolume == startVolume) {
                // match with linear ramp
                const T offset = volume - startVolume;
                static const T scale =  1.f / (MAX_CURVE_TIME - MIN_CURVE_TIME); // nominally 1.f
                for (auto it = this->begin(); it != this->end(); ++it) {
                    it->second = it->second + offset * (MAX_CURVE_TIME - it->first) * scale;
                }
            } else {
                const T  scale = (volume - endVolume) / (startVolume - endVolume);
                for (auto it = this->begin(); it != this->end(); ++it) {
                    it->second = scale * (it->second - endVolume) + endVolume;
                }
            }
            clampVolume();
        }

        // The parcel layout must match VolumeShaper.java
        status_t writeToParcel(Parcel *parcel) const override {
            if (parcel == nullptr) return BAD_VALUE;
            return parcel->writeInt32((int32_t)mType)
                    ?: parcel->writeInt32(mId)
                    ?: mType == TYPE_ID
                        ? NO_ERROR
                        : parcel->writeInt32((int32_t)mOptionFlags)
                            ?: parcel->writeDouble(mDurationMs)
                            ?: Interpolator<S, T>::writeToParcel(parcel);
        }

        status_t readFromParcel(const Parcel *parcel) override {
            int32_t type, optionFlags;
            return parcel->readInt32(&type)
                    ?: setType((Type)type)
                    ?: parcel->readInt32(&mId)
                    ?: mType == TYPE_ID
                        ? NO_ERROR
                        : parcel->readInt32(&optionFlags)
                            ?: setOptionFlags((OptionFlag)optionFlags)
                            ?: parcel->readDouble(&mDurationMs)
                            ?: Interpolator<S, T>::readFromParcel(*parcel)
                            ?: checkCurve();
        }

        // Returns a string for debug printing.
        std::string toString() const {
            std::stringstream ss;
            ss << "VolumeShaper::Configuration{mType=" << static_cast<int32_t>(mType);
            ss << ", mId=" << mId;
            if (mType != TYPE_ID) {
                ss << ", mOptionFlags=" << static_cast<int32_t>(mOptionFlags);
                ss << ", mDurationMs=" << mDurationMs;
                ss << ", " << Interpolator<S, T>::toString().c_str();
            }
            ss << "}";
            return ss.str();
        }

    private:
        Type mType;              // type of configuration
        int32_t mId;             // A valid id is >= 0.
        OptionFlag mOptionFlags; // option flags for the configuration.
        double mDurationMs;      // duration, must be > 0; default is 1000 ms.
    }; // Configuration

    /* VolumeShaper::Operation expresses an operation to perform on the
     * configuration (either explicitly specified or an id).
     *
     * This parallels the Java implementation and the enums must match.
     * See "frameworks/base/media/java/android/media/VolumeShaper.java" for
     * details on the Java implementation.
     */
    class Operation : public RefBase, public Parcelable {
    public:
        // Must match with VolumeShaper.java.
        enum Flag : int32_t {
            FLAG_NONE      = 0,
            FLAG_REVERSE   = (1 << 0), // the absence of this indicates "play"
            FLAG_TERMINATE = (1 << 1),
            FLAG_JOIN      = (1 << 2),
            FLAG_DELAY     = (1 << 3),
            FLAG_CREATE_IF_NECESSARY = (1 << 4),

            FLAG_ALL       = (FLAG_REVERSE | FLAG_TERMINATE | FLAG_JOIN | FLAG_DELAY
                            | FLAG_CREATE_IF_NECESSARY),
        };

        Operation()
            : Operation(FLAG_NONE, -1 /* replaceId */) {
        }

        Operation(Flag flags, int replaceId)
            : Operation(flags, replaceId, std::numeric_limits<S>::quiet_NaN() /* xOffset */) {
        }

        explicit Operation(const Operation &operation)
            : Operation(operation.mFlags, operation.mReplaceId, operation.mXOffset) {
        }

        explicit Operation(const sp<Operation> &operation)
            : Operation(*operation.get()) {
        }

        Operation(Flag flags, int replaceId, S xOffset)
            : mFlags(flags)
            , mReplaceId(replaceId)
            , mXOffset(xOffset) {
        }

        int32_t getReplaceId() const {
            return mReplaceId;
        }

        void setReplaceId(int32_t replaceId) {
            mReplaceId = replaceId;
        }

        S getXOffset() const {
            return mXOffset;
        }

        void setXOffset(S xOffset) {
            mXOffset = clamp(xOffset, MIN_CURVE_TIME /* lo */, MAX_CURVE_TIME /* hi */);
        }

        Flag getFlags() const {
            return mFlags;
        }

        /* xOffset is the position on the volume curve and may go backwards
         * if you are in reverse mode. This must be in the range from
         * [MIN_CURVE_TIME, MAX_CURVE_TIME].
         *
         * normalizedTime always increases as time or framecount increases.
         * normalizedTime is nominally from MIN_CURVE_TIME to MAX_CURVE_TIME when
         * running through the curve, but could be outside this range afterwards.
         * If you are reversing, this means the position on the curve, or xOffset,
         * is computed as MAX_CURVE_TIME - normalizedTime, clamped to
         * [MIN_CURVE_TIME, MAX_CURVE_TIME].
         */
        void setNormalizedTime(S normalizedTime) {
            setXOffset((mFlags & FLAG_REVERSE) != 0
                    ? MAX_CURVE_TIME - normalizedTime : normalizedTime);
        }

        status_t setFlags(Flag flags) {
            if ((flags & ~FLAG_ALL) != 0) {
                ALOGE("flags has invalid bits: %#x", flags);
                return BAD_VALUE;
            }
            mFlags = flags;
            return NO_ERROR;
        }

        status_t writeToParcel(Parcel *parcel) const override {
            if (parcel == nullptr) return BAD_VALUE;
            return parcel->writeInt32((int32_t)mFlags)
                    ?: parcel->writeInt32(mReplaceId)
                    ?: parcel->writeFloat(mXOffset);
        }

        status_t readFromParcel(const Parcel *parcel) override {
            int32_t flags;
            return parcel->readInt32(&flags)
                    ?: parcel->readInt32(&mReplaceId)
                    ?: parcel->readFloat(&mXOffset)
                    ?: setFlags((Flag)flags);
        }

        std::string toString() const {
            std::stringstream ss;
            ss << "VolumeShaper::Operation{mFlags=" << static_cast<int32_t>(mFlags) ;
            ss << ", mReplaceId=" << mReplaceId;
            ss << ", mXOffset=" << mXOffset;
            ss << "}";
            return ss.str();
        }

    private:
        Flag mFlags;        // operation to do
        int32_t mReplaceId; // if >= 0 the id to remove in a replace operation.
        S mXOffset;         // position in the curve to set if a valid number (not nan)
    }; // Operation

    /* VolumeShaper.State is returned when requesting the last
     * state of the VolumeShaper.
     *
     * This parallels the Java implementation.
     * See "frameworks/base/media/java/android/media/VolumeShaper.java" for
     * details on the Java implementation.
     */
    class State : public RefBase, public Parcelable {
    public:
        State(T volume, S xOffset)
            : mVolume(volume)
            , mXOffset(xOffset) {
        }

        State()
            : State(NAN, NAN) { }

        T getVolume() const {
            return mVolume;
        }

        void setVolume(T volume) {
            mVolume = volume;
        }

        S getXOffset() const {
            return mXOffset;
        }

        void setXOffset(S xOffset) {
            mXOffset = xOffset;
        }

        status_t writeToParcel(Parcel *parcel) const override {
            if (parcel == nullptr) return BAD_VALUE;
            return parcel->writeFloat(mVolume)
                    ?: parcel->writeFloat(mXOffset);
        }

        status_t readFromParcel(const Parcel *parcel) override {
            return parcel->readFloat(&mVolume)
                     ?: parcel->readFloat(&mXOffset);
        }

        std::string toString() const {
            std::stringstream ss;
            ss << "VolumeShaper::State{mVolume=" << mVolume;
            ss << ", mXOffset=" << mXOffset;
            ss << "}";
            return ss.str();
        }

    private:
        T mVolume;   // linear volume in the range MIN_LINEAR_VOLUME to MAX_LINEAR_VOLUME
        S mXOffset;  // position on curve expressed from MIN_CURVE_TIME to MAX_CURVE_TIME
    }; // State

    // Internal helper class to do an affine transform for time and amplitude scaling.
    template <typename R>
    class Translate {
    public:
        Translate()
            : mOffset(0)
            , mScale(1) {
        }

        R getOffset() const {
            return mOffset;
        }

        void setOffset(R offset) {
            mOffset = offset;
        }

        R getScale() const {
            return mScale;
        }

        void setScale(R scale) {
            mScale = scale;
        }

        R operator()(R in) const {
            return mScale * (in - mOffset);
        }

        std::string toString() const {
            std::stringstream ss;
            ss << "VolumeShaper::Translate{mOffset=" << mOffset;
            ss << ", mScale=" << mScale;
            ss << "}";
            return ss.str();
        }

    private:
        R mOffset;
        R mScale;
    }; // Translate

    static int64_t convertTimespecToUs(const struct timespec &tv)
    {
        return tv.tv_sec * 1000000LL + tv.tv_nsec / 1000;
    }

    // current monotonic time in microseconds.
    static int64_t getNowUs()
    {
        struct timespec tv;
        if (clock_gettime(CLOCK_MONOTONIC, &tv) != 0) {
            return 0; // system is really sick, just return 0 for consistency.
        }
        return convertTimespecToUs(tv);
    }

    /* Native implementation of VolumeShaper.  This is NOT mirrored
     * on the Java side, so we don't need to mimic Java side layout
     * and data; furthermore, this isn't refcounted as a "RefBase" object.
     *
     * Since we pass configuration and operation as shared pointers (like
     * Java) there is a potential risk that the caller may modify
     * these after delivery.
     */
    VolumeShaper(
            const sp<VolumeShaper::Configuration> &configuration,
            const sp<VolumeShaper::Operation> &operation)
        : mConfiguration(configuration) // we do not make a copy
        , mOperation(operation)         // ditto
        , mStartFrame(-1)
        , mLastVolume(T(1))
        , mLastXOffset(MIN_CURVE_TIME)
        , mDelayXOffset(MIN_CURVE_TIME) {
        if (configuration.get() != nullptr
                && (getFlags() & VolumeShaper::Operation::FLAG_DELAY) == 0) {
            mLastVolume = configuration->first().second;
        }
    }

    // We allow a null operation here, though VolumeHandler always provides one.
    VolumeShaper::Operation::Flag getFlags() const {
        return mOperation == nullptr
                ? VolumeShaper::Operation::FLAG_NONE : mOperation->getFlags();
    }

    /* Returns the last volume and xoffset reported to the AudioFlinger.
     * If the VolumeShaper has not been started, compute what the volume
     * should be based on the initial offset specified.
     */
    sp<VolumeShaper::State> getState() const {
        if (!isStarted()) {
            const T volume = computeVolumeFromXOffset(mDelayXOffset);
            VS_LOG("delayed VolumeShaper, using cached offset:%f for volume:%f",
                    mDelayXOffset, volume);
            return new VolumeShaper::State(volume, mDelayXOffset);
        } else {
            return new VolumeShaper::State(mLastVolume, mLastXOffset);
        }
    }

    S getDelayXOffset() const {
        return mDelayXOffset;
    }

    void setDelayXOffset(S xOffset) {
        mDelayXOffset = clamp(xOffset, MIN_CURVE_TIME /* lo */, MAX_CURVE_TIME /* hi */);
    }

    bool isStarted() const {
        return mStartFrame >= 0;
    }

    /* getVolume() updates the last volume/xoffset state so it is not
     * const, even though logically it may be viewed as const.
     */
    std::pair<T /* volume */, bool /* active */> getVolume(
            int64_t trackFrameCount, double trackSampleRate) {
        if ((getFlags() & VolumeShaper::Operation::FLAG_DELAY) != 0) {
            // We haven't had PLAY called yet, so just return the value
            // as if PLAY were called just now.
            VS_LOG("delayed VolumeShaper, using cached offset %f", mDelayXOffset);
            const T volume = computeVolumeFromXOffset(mDelayXOffset);
            return std::make_pair(volume, false);
        }
        const bool clockTime = (mConfiguration->getOptionFlags()
                & VolumeShaper::Configuration::OPTION_FLAG_CLOCK_TIME) != 0;
        const int64_t frameCount = clockTime ? getNowUs() : trackFrameCount;
        const double sampleRate = clockTime ? 1000000 : trackSampleRate;

        if (mStartFrame < 0) {
            updatePosition(frameCount, sampleRate, mDelayXOffset);
            mStartFrame = frameCount;
        }
        VS_LOG("frameCount: %lld", (long long)frameCount);
        const S x = mXTranslate((T)frameCount);
        VS_LOG("translation to normalized time: %f", x);

        std::tuple<T /* volume */, S /* position */, bool /* active */> vt =
                computeStateFromNormalizedTime(x);

        mLastVolume = std::get<0>(vt);
        mLastXOffset = std::get<1>(vt);
        const bool active = std::get<2>(vt);
        VS_LOG("rescaled time:%f  volume:%f  xOffset:%f  active:%s",
                x, mLastVolume, mLastXOffset, active ? "true" : "false");
        return std::make_pair(mLastVolume, active);
    }

    std::string toString() const {
        std::stringstream ss;
        ss << "VolumeShaper{mStartFrame=" << mStartFrame;
        ss << ", mXTranslate=" << mXTranslate.toString().c_str();
        ss << ", mConfiguration=" <<
                (mConfiguration.get() == nullptr
                        ? "nullptr" : mConfiguration->toString().c_str());
        ss << ", mOperation=" <<
                (mOperation.get() == nullptr
                        ? "nullptr" :  mOperation->toString().c_str());
        ss << "}";
        return ss.str();
    }

    Translate<S> mXTranslate; // translation from frames (usec for clock time) to normalized time.
    sp<VolumeShaper::Configuration> mConfiguration;
    sp<VolumeShaper::Operation> mOperation;

private:
    int64_t mStartFrame; // starting frame, non-negative when started (in usec for clock time)
    T mLastVolume;       // last computed interpolated volume (y-axis)
    S mLastXOffset;      // last computed interpolated xOffset/time (x-axis)
    S mDelayXOffset;     // xOffset to use for first invocation of VolumeShaper.

    // Called internally to adjust mXTranslate for first time start.
    void updatePosition(int64_t startFrame, double sampleRate, S xOffset) {
        double scale = (mConfiguration->last().first - mConfiguration->first().first)
                        / (mConfiguration->getDurationMs() * 0.001 * sampleRate);
        const double minScale = 1. / static_cast<double>(INT64_MAX);
        scale = std::max(scale, minScale);
        VS_LOG("update position: scale %lf  frameCount:%lld, sampleRate:%lf, xOffset:%f",
                scale, (long long) startFrame, sampleRate, xOffset);

        S normalizedTime = (getFlags() & VolumeShaper::Operation::FLAG_REVERSE) != 0 ?
                MAX_CURVE_TIME - xOffset : xOffset;
        mXTranslate.setOffset(static_cast<float>(static_cast<double>(startFrame)
                                                 - static_cast<double>(normalizedTime) / scale));
        mXTranslate.setScale(static_cast<float>(scale));
        VS_LOG("translate: %s", mXTranslate.toString().c_str());
    }

    T computeVolumeFromXOffset(S xOffset) const {
        const T unscaledVolume = mConfiguration->findY(xOffset);
        const T volume = mConfiguration->adjustVolume(unscaledVolume); // handle log scale
        VS_LOG("computeVolumeFromXOffset %f -> %f -> %f", xOffset, unscaledVolume, volume);
        return volume;
    }

    std::tuple<T /* volume */, S /* position */, bool /* active */>
    computeStateFromNormalizedTime(S x) const {
        bool active = true;
        // handle reversal of position
        if (getFlags() & VolumeShaper::Operation::FLAG_REVERSE) {
            x = MAX_CURVE_TIME - x;
            VS_LOG("reversing to %f", x);
            if (x < MIN_CURVE_TIME) {
                x = MIN_CURVE_TIME;
                active = false; // at the end
            } else if (x > MAX_CURVE_TIME) {
                x = MAX_CURVE_TIME; //early
            }
        } else {
            if (x < MIN_CURVE_TIME) {
                x = MIN_CURVE_TIME; // early
            } else if (x > MAX_CURVE_TIME) {
                x = MAX_CURVE_TIME;
                active = false; // at end
            }
        }
        const S xOffset = x;
        const T volume = computeVolumeFromXOffset(xOffset);
        return std::make_tuple(volume, xOffset, active);
    }
}; // VolumeShaper

/* VolumeHandler combines the volume factors of multiple VolumeShapers associated
 * with a player.  It is thread safe by synchronizing all public methods.
 *
 * This is a native-only implementation.
 *
 * The server side VolumeHandler is used to maintain a list of volume handlers,
 * keep state, and obtain volume.
 *
 * The client side VolumeHandler is used to maintain a list of volume handlers,
 * keep some partial state, and restore if the server dies.
 */
class VolumeHandler : public RefBase {
public:
    using S = float;
    using T = float;

    // A volume handler which just keeps track of active VolumeShapers does not need sampleRate.
    VolumeHandler()
        : VolumeHandler(0 /* sampleRate */) {
    }

    explicit VolumeHandler(uint32_t sampleRate)
        : mSampleRate((double)sampleRate)
        , mLastFrame(0)
        , mVolumeShaperIdCounter(VolumeShaper::kSystemVolumeShapersMax)
        , mLastVolume(1.f, false) {
    }

    VolumeShaper::Status applyVolumeShaper(
            const sp<VolumeShaper::Configuration> &configuration,
            const sp<VolumeShaper::Operation> &operation_in) {
        // make a local copy of operation, as we modify it.
        sp<VolumeShaper::Operation> operation(new VolumeShaper::Operation(operation_in));
        VS_LOG("applyVolumeShaper:configuration: %s", configuration->toString().c_str());
        VS_LOG("applyVolumeShaper:operation: %s", operation->toString().c_str());
        AutoMutex _l(mLock);
        if (configuration == nullptr) {
            ALOGE("null configuration");
            return VolumeShaper::Status(BAD_VALUE);
        }
        if (operation == nullptr) {
            ALOGE("null operation");
            return VolumeShaper::Status(BAD_VALUE);
        }
        const int32_t id = configuration->getId();
        if (id < 0) {
            ALOGE("negative id: %d", id);
            return VolumeShaper::Status(BAD_VALUE);
        }
        VS_LOG("applyVolumeShaper id: %d", id);

        switch (configuration->getType()) {
        case VolumeShaper::Configuration::TYPE_SCALE: {
            const int replaceId = operation->getReplaceId();
            if (replaceId >= 0) {
                VS_LOG("replacing %d", replaceId);
                auto replaceIt = findId_l(replaceId);
                if (replaceIt == mVolumeShapers.end()) {
                    ALOGW("cannot find replace id: %d", replaceId);
                } else {
                    if ((operation->getFlags() & VolumeShaper::Operation::FLAG_JOIN) != 0) {
                        // For join, we scale the start volume of the current configuration
                        // to match the last-used volume of the replacing VolumeShaper.
                        auto state = replaceIt->getState();
                        ALOGD("join: state:%s", state->toString().c_str());
                        if (state->getXOffset() >= 0) { // valid
                            const T volume = state->getVolume();
                            ALOGD("join: scaling start volume to %f", volume);
                            configuration->scaleToStartVolume(volume);
                        }
                    }
                    (void)mVolumeShapers.erase(replaceIt);
                }
                operation->setReplaceId(-1);
            }
            // check if we have another of the same id.
            auto oldIt = findId_l(id);
            if (oldIt != mVolumeShapers.end()) {
                if ((operation->getFlags()
                        & VolumeShaper::Operation::FLAG_CREATE_IF_NECESSARY) != 0) {
                    // TODO: move the case to a separate function.
                    goto HANDLE_TYPE_ID; // no need to create, take over existing id.
                }
                ALOGW("duplicate id, removing old %d", id);
                (void)mVolumeShapers.erase(oldIt);
            }

            /* Check if too many application VolumeShapers (with id >= kSystemVolumeShapersMax).
             * We check on the server side to ensure synchronization and robustness.
             *
             * This shouldn't fail on a replace command unless the replaced id is
             * already invalid (which *should* be checked in the Java layer).
             */
            if (id >= VolumeShaper::kSystemVolumeShapersMax
                    && numberOfUserVolumeShapers_l() >= VolumeShaper::kUserVolumeShapersMax) {
                ALOGW("Too many app VolumeShapers, cannot add to VolumeHandler");
                return VolumeShaper::Status(INVALID_OPERATION);
            }

            // create new VolumeShaper with default behavior.
            mVolumeShapers.emplace_back(configuration, new VolumeShaper::Operation());
            VS_LOG("after adding, number of volumeShapers:%zu", mVolumeShapers.size());
        }
        // fall through to handle the operation
        HANDLE_TYPE_ID:
        case VolumeShaper::Configuration::TYPE_ID: {
            VS_LOG("trying to find id: %d", id);
            auto it = findId_l(id);
            if (it == mVolumeShapers.end()) {
                VS_LOG("couldn't find id: %d", id);
                return VolumeShaper::Status(INVALID_OPERATION);
            }
            if ((operation->getFlags() & VolumeShaper::Operation::FLAG_TERMINATE) != 0) {
                VS_LOG("terminate id: %d", id);
                mVolumeShapers.erase(it);
                break;
            }
            const bool clockTime = (it->mConfiguration->getOptionFlags()
                    & VolumeShaper::Configuration::OPTION_FLAG_CLOCK_TIME) != 0;
            if ((it->getFlags() & VolumeShaper::Operation::FLAG_REVERSE) !=
                    (operation->getFlags() & VolumeShaper::Operation::FLAG_REVERSE)) {
                if (it->isStarted()) {
                    const int64_t frameCount = clockTime ? VolumeShaper::getNowUs() : mLastFrame;
                    const S x = it->mXTranslate((T)frameCount);
                    VS_LOG("reverse normalizedTime: %f", x);
                    // reflect position
                    S target = MAX_CURVE_TIME - x;
                    if (target < MIN_CURVE_TIME) {
                        VS_LOG("clamp to start - begin immediately");
                        target = MIN_CURVE_TIME;
                    }
                    VS_LOG("reverse normalizedTime target: %f", target);
                    it->mXTranslate.setOffset(it->mXTranslate.getOffset()
                            + (x - target) / it->mXTranslate.getScale());
                }
                // if not started, the delay offset doesn't change.
            }
            const S xOffset = operation->getXOffset();
            if (!std::isnan(xOffset)) {
                if (it->isStarted()) {
                    const int64_t frameCount = clockTime ? VolumeShaper::getNowUs() : mLastFrame;
                    const S x = it->mXTranslate((T)frameCount);
                    VS_LOG("normalizedTime translation: %f", x);
                    const S target =
                            (operation->getFlags() & VolumeShaper::Operation::FLAG_REVERSE) != 0 ?
                                    MAX_CURVE_TIME - xOffset : xOffset;
                    VS_LOG("normalizedTime target x offset: %f", target);
                    it->mXTranslate.setOffset(it->mXTranslate.getOffset()
                            + (x - target) / it->mXTranslate.getScale());
                } else {
                    it->setDelayXOffset(xOffset);
                }
            }
            it->mOperation = operation; // replace the operation
        } break;
        }
        return VolumeShaper::Status(id);
    }

    sp<VolumeShaper::State> getVolumeShaperState(int id) {
        AutoMutex _l(mLock);
        auto it = findId_l(id);
        if (it == mVolumeShapers.end()) {
            VS_LOG("cannot find state for id: %d", id);
            return nullptr;
        }
        return it->getState();
    }

    /* getVolume() is not const, as it updates internal state.
     * Once called, any VolumeShapers not already started begin running.
     */
    std::pair<T /* volume */, bool /* active */> getVolume(int64_t trackFrameCount) {
        AutoMutex _l(mLock);
        mLastFrame = trackFrameCount;
        T volume(1);
        size_t activeCount = 0;
        for (auto it = mVolumeShapers.begin(); it != mVolumeShapers.end();) {
            const std::pair<T, bool> shaperVolume =
                    it->getVolume(trackFrameCount, mSampleRate);
            volume *= shaperVolume.first;
            activeCount += shaperVolume.second;
            ++it;
        }
        mLastVolume = std::make_pair(volume, activeCount != 0);
        VS_LOG("getVolume: <%f, %s>", mLastVolume.first, mLastVolume.second ? "true" : "false");
        return mLastVolume;
    }

    /* Used by a client side VolumeHandler to ensure all the VolumeShapers
     * indicate that they have been started.  Upon a change in audioserver
     * output sink, this information is used for restoration of the server side
     * VolumeHandler.
     */
    void setStarted() {
        (void)getVolume(mLastFrame);  // getVolume() will start the individual VolumeShapers.
    }

    std::pair<T /* volume */, bool /* active */> getLastVolume() const {
        AutoMutex _l(mLock);
        return mLastVolume;
    }

    std::string toString() const {
        AutoMutex _l(mLock);
        std::stringstream ss;
        ss << "VolumeHandler{mSampleRate=" << mSampleRate;
        ss << ", mLastFrame=" << mLastFrame;
        ss << ", mVolumeShapers={";
        bool first = true;
        for (const auto &shaper : mVolumeShapers) {
            if (first) {
                first = false;
            } else {
                ss << ", ";
            }
            ss << shaper.toString().c_str();
        }
        ss << "}}";
        return ss.str();
    }

    void forall(const std::function<VolumeShaper::Status (const VolumeShaper &)> &lambda) {
        AutoMutex _l(mLock);
        VS_LOG("forall: mVolumeShapers.size() %zu", mVolumeShapers.size());
        for (const auto &shaper : mVolumeShapers) {
            VolumeShaper::Status status = lambda(shaper);
            VS_LOG("forall applying lambda on shaper (%p): %d", &shaper, (int)status);
        }
    }

    void reset() {
        AutoMutex _l(mLock);
        mVolumeShapers.clear();
        mLastFrame = 0;
        // keep mVolumeShaperIdCounter as is.
    }

    /* Sets the configuration id if necessary - This is based on the counter
     * internal to the VolumeHandler.
     */
    void setIdIfNecessary(const sp<VolumeShaper::Configuration> &configuration) {
        if (configuration->getType() == VolumeShaper::Configuration::TYPE_SCALE) {
            const int id = configuration->getId();
            if (id == -1) {
                // Reassign to a unique id, skipping system ids.
                AutoMutex _l(mLock);
                while (true) {
                    if (mVolumeShaperIdCounter == INT32_MAX) {
                        mVolumeShaperIdCounter = VolumeShaper::kSystemVolumeShapersMax;
                    } else {
                        ++mVolumeShaperIdCounter;
                    }
                    if (findId_l(mVolumeShaperIdCounter) != mVolumeShapers.end()) {
                        continue; // collision with an existing id.
                    }
                    configuration->setId(mVolumeShaperIdCounter);
                    ALOGD("setting id to %d", mVolumeShaperIdCounter);
                    break;
                }
            }
        }
    }

private:
    std::list<VolumeShaper>::iterator findId_l(int32_t id) {
        std::list<VolumeShaper>::iterator it = mVolumeShapers.begin();
        for (; it != mVolumeShapers.end(); ++it) {
            if (it->mConfiguration->getId() == id) {
                break;
            }
        }
        return it;
    }

    size_t numberOfUserVolumeShapers_l() const {
        size_t count = 0;
        for (const auto &shaper : mVolumeShapers) {
            count += (shaper.mConfiguration->getId() >= VolumeShaper::kSystemVolumeShapersMax);
        }
        return count;
    }

    mutable Mutex mLock;
    double mSampleRate; // in samples (frames) per second
    int64_t mLastFrame; // logging purpose only, 0 on start
    int32_t mVolumeShaperIdCounter; // a counter to return a unique volume shaper id.
    std::pair<T /* volume */, bool /* active */> mLastVolume;
    std::list<VolumeShaper> mVolumeShapers; // list provides stable iterators on erase
}; // VolumeHandler

} // namespace media

} // namespace android

#pragma pop_macro("LOG_TAG")

#endif // ANDROID_VOLUME_SHAPER_H
